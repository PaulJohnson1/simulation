#include <Renderer/Renderer.h>

#include <math.h>
#include <stdarg.h>
#include <string.h>

#include <Utilities.h>

void tmp_renderer_spritesheet_init(struct tmp_renderer_spritesheet *spritesheet, void (*setup)(struct tmp_renderer *), ...)
{
    struct tmp_renderer *renderer = &spritesheet->renderer;
    tmp_renderer_init(renderer);
    if (setup != NULL)
        setup(renderer);
    va_list args;
    va_start(args, setup);
    float cutmp_x = 0;
    float cutmp_y = 0;
    uint32_t size = 0;
    while (1)
    {
        float width = va_arg(args, int);
        if (width == 0)
            break;
        float height = va_arg(args, int);
        struct tmp_sprite_bounds *bounds = &spritesheet->sprites[size++];
        bounds->x = cutmp_x + width / 2;
        bounds->y = 0 + height / 2;
        bounds->w = width;
        bounds->h = height;
        cutmp_x += width;
        bounds->render = va_arg(args, void *);
        if (cutmp_y < height)
            cutmp_y = height;
    }
    tmp_renderer_set_dimensions(renderer, cutmp_x, cutmp_y);
    struct tmp_renderer_context_state state;
    for (uint32_t i = 0; i < size; ++i)
    {
        struct tmp_sprite_bounds *bounds = &spritesheet->sprites[i];
        tmp_renderer_set_transform(renderer, 1, 0, bounds->x, 0, 1, bounds->y);
        tmp_renderer_context_state_init(renderer, &state);
        bounds->render(renderer);
        tmp_renderer_context_state_free(renderer, &state);
    }
}

void render_sprite_from_cache(struct tmp_renderer *renderer, struct tmp_renderer_spritesheet *spritesheet, uint32_t pos)
{
    struct tmp_sprite_bounds *bounds = &spritesheet->sprites[pos];
    tmp_renderer_draw_clipped_image(renderer, &spritesheet->renderer, bounds->x, bounds->y, bounds->w, bounds->h, 0, 0);
}

void tmp_renderer_context_state_init(struct tmp_renderer *this,
                                    struct tmp_renderer_context_state *state)
{
    memcpy(state, &this->state, sizeof *state);
    tmp_renderer_save(this);
}

void tmp_renderer_context_state_free(struct tmp_renderer *this,
                                    struct tmp_renderer_context_state *state)
{
    memcpy(&this->state, state, sizeof *state);
    tmp_renderer_restore(this);
}

void tmp_renderer_set_transform(struct tmp_renderer *this, float a, float b,
                               float c, float d, float e, float f)
{
    this->state.transform_matrix[0] = a;
    this->state.transform_matrix[1] = b;
    this->state.transform_matrix[2] = c;
    this->state.transform_matrix[3] = d;
    this->state.transform_matrix[4] = e;
    this->state.transform_matrix[5] = f;
    tmp_renderer_update_transform(this);
}

void tmp_renderer_translate(struct tmp_renderer *this, float x, float y)
{
    this->state.transform_matrix[2] += x * this->state.transform_matrix[0] +
                                       y * this->state.transform_matrix[3];
    this->state.transform_matrix[5] += x * this->state.transform_matrix[1] +
                                       y * this->state.transform_matrix[4];
    tmp_renderer_update_transform(this);
}

void tmp_renderer_rotate(struct tmp_renderer *this, float a)
{
    float cos_a = cosf(a);
    float sin_a = sinf(a);
    float original0 = this->state.transform_matrix[0];
    float original1 = this->state.transform_matrix[1];
    float original3 = this->state.transform_matrix[3];
    float original4 = this->state.transform_matrix[4];
    this->state.transform_matrix[0] = original0 * cos_a + original1 * -sin_a;
    this->state.transform_matrix[1] = original0 * sin_a + original1 * cos_a;
    this->state.transform_matrix[3] = original3 * cos_a + original4 * -sin_a;
    this->state.transform_matrix[4] = original3 * sin_a + original4 * cos_a;
    tmp_renderer_update_transform(this);
}

void tmp_renderer_scale2(struct tmp_renderer *this, float x, float y)
{
    this->state.transform_matrix[0] *= x;
    this->state.transform_matrix[1] *= x;
    this->state.transform_matrix[3] *= y;
    this->state.transform_matrix[4] *= y;
    tmp_renderer_update_transform(this);
}

void tmp_renderer_scale(struct tmp_renderer *this, float s)
{
    tmp_renderer_scale2(this, s, s);
}

void tmp_renderer_arc(struct tmp_renderer *this, float x, float y, float r)
{
    tmp_renderer_partial_arc(this, x, y, r, 0, 2 * M_PI, 0);
}

void tmp_renderer_reverse_arc(struct tmp_renderer *this, float x, float y,
                             float r)
{
    tmp_renderer_partial_arc(this, x, y, r, 2 * M_PI, 0, 1);
}

void tmp_renderer_round_rect(struct tmp_renderer *this, float x, float y, float w,
                            float h, float r)
{
    tmp_renderer_begin_path(this);
    tmp_renderer_move_to(this, x + r, y);
    tmp_renderer_line_to(this, x + w - r, y);
    tmp_renderer_quadratic_curve_to(this, x + w, y, x + w, y + r);
    tmp_renderer_line_to(this, x + w, y + h - r);
    tmp_renderer_quadratic_curve_to(this, x + w, y + h, x + w - r, y + h);
    tmp_renderer_line_to(this, x + r, y + h);
    tmp_renderer_quadratic_curve_to(this, x, y + h, x, y + h - r);
    tmp_renderer_line_to(this, x, y + r);
    tmp_renderer_quadratic_curve_to(this, x, y, x + r, y);
}

void tmp_renderer_add_color_filter(struct tmp_renderer *this, uint32_t color,
                                  float amount)
{
    if (this->state.filter.amount == 0 || (this->state.filter.color << 8) == 0)
    {
        this->state.filter.color = color;
        this->state.filter.amount = 1 - (1 - this->state.filter.amount) * (1 - amount);
        return;
    }
    float alpha = this->state.filter.amount;
    uint32_t c1 = this->state.filter.color;
    float new_amount = 1 - (1 - alpha) * (1 - amount);
    uint8_t old_red = (c1 >> 16) & 255;
    uint8_t c_red = (color >> 16) & 255;
    uint8_t new_red = tmp_fclamp(
        (old_red * alpha * (1 - amount) + c_red * amount) / (new_amount), 0,
        255);
    uint8_t old_green = (c1 >> 8) & 255;
    uint8_t c_green = (color >> 8) & 255;
    uint8_t new_green = tmp_fclamp(
        (old_green * alpha * (1 - amount) + c_green * amount) / (new_amount), 0,
        255);
    uint8_t old_blue = (c1)&255;
    uint8_t c_blue = (color)&255;
    uint8_t new_blue = tmp_fclamp(
        (old_blue * alpha * (1 - amount) + c_blue * amount) / (new_amount), 0,
        255);
    this->state.filter.color =
        (255 << 24) | (new_red << 16) | (new_green << 8) | new_blue;
    this->state.filter.amount = new_amount;
}

void tmp_renderer_reset_color_filter(struct tmp_renderer *this)
{
    this->state.filter.color = 0x00000000;
    this->state.filter.amount = 0;
}
