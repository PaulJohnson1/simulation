#pragma once

#include <stdint.h>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#else
// #include <include/core/SkSurface.h>
// #include <include/core/SkCanvas.h>
#endif

#include <Bitset.h>

// reason this is here is because this header is also included in the native.cc
// file and in the wasm.c file for the target specific function implementations.
#ifdef __cplusplus
extern "C"
{
#endif

    struct tmp_input_touch
    {
        float touch_x;
        float touch_y;
        uint8_t identifier;
        uint8_t active;
    };

    struct tmp_input_data
    {
        char *clipboard;
        uint8_t keys_pressed[BS_ROUND(256)];
        // c version of addeventlistener("keydown")
        uint8_t keys_pressed_this_tick[BS_ROUND(256)];
        uint8_t keys_released_this_tick[BS_ROUND(256)];

        uint32_t keycodes_pressed_this_tick[16];
        uint32_t keycodes_length;
        struct tmp_input_touch touches[16];
        uint8_t touch_length;
        uint8_t mouse_buttons;
        uint8_t mouse_state;
        // c version of addeventlistener("mouse")
        uint8_t mouse_state_this_tick;
        uint8_t mouse_buttons_up_this_tick;
        uint8_t mouse_buttons_down_this_tick;
        float mouse_x;
        float mouse_y;
        float prev_mouse_x;
        float prev_mouse_y;
        float scroll_delta;
    };

    void tmp_input_data_init(struct tmp_input_data *);

#define INSTRUCTION_QUEUE_MAX_SIZE (1 << 14)

    struct tmp_renderer_paint
    {
        uint8_t alpha;
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };

    struct tmp_renderer_filter
    {
        uint32_t color;
        float amount;
    };

    struct tmp_renderer_context_state
    {
        float transform_matrix[6];
        struct tmp_renderer_filter filter;
        float global_alpha;
#ifndef WASM_BUILD
        void *path;
#endif
    };

    struct tmp_renderer
    {
#ifndef WASM_BUILD
        void *surface;
        void *canvas;
#else
    uint32_t context_id;
#endif
        struct tmp_renderer_context_state state;
        float width;
        float height;
        float scale;

        uint8_t matrix_moddified;
    };

    struct tmp_sprite_bounds
    {
        float x;
        float y;
        float w;
        float h;
        void (*render)(struct tmp_renderer *);
    };

    struct tmp_renderer_spritesheet
    {
        struct tmp_renderer renderer;
        struct tmp_sprite_bounds sprites[16];
    };

#ifndef WASM_BUILD
    struct tmp_window;
    void __tmp_renderer_init_native(struct tmp_window *twindow, void *glfw_window);
    void tmp_renderer_circle(struct tmp_renderer *, float, float, float);
#endif

    void tmp_renderer_init(struct tmp_renderer *);
    void tmp_renderer_set_dimensions(struct tmp_renderer *, float, float);

    void tmp_renderer_spritesheet_init(struct tmp_renderer_spritesheet *, void (*)(struct tmp_renderer *), ...);
    void render_sprite_from_cache(struct tmp_renderer *, struct tmp_renderer_spritesheet *, uint32_t);

    void tmp_renderer_context_state_init(struct tmp_renderer *,
                                         struct tmp_renderer_context_state *);
    void tmp_renderer_context_state_free(struct tmp_renderer *,
                                         struct tmp_renderer_context_state *);

    void tmp_renderer_add_color_filter(struct tmp_renderer *, uint32_t, float);
    void tmp_renderer_reset_color_filter(struct tmp_renderer *);

    void tmp_renderer_set_fill(struct tmp_renderer *, uint32_t);
    void tmp_renderer_set_stroke(struct tmp_renderer *, uint32_t);

    void tmp_renderer_set_line_width(struct tmp_renderer *, float);
    void tmp_renderer_set_text_size(struct tmp_renderer *, float);
    void tmp_renderer_set_global_alpha(struct tmp_renderer *, float);

    void tmp_renderer_set_line_cap(struct tmp_renderer *, uint8_t);
    void tmp_renderer_set_line_join(struct tmp_renderer *, uint8_t);
    void tmp_renderer_set_text_align(struct tmp_renderer *, uint8_t);
    void tmp_renderer_set_text_baseline(struct tmp_renderer *, uint8_t);

    void tmp_renderer_update_transform(struct tmp_renderer *);
    void tmp_renderer_set_transform(struct tmp_renderer *, float, float, float,
                                    float, float, float);
    void tmp_renderer_translate(struct tmp_renderer *, float, float);
    void tmp_renderer_rotate(struct tmp_renderer *, float);
    void tmp_renderer_scale(struct tmp_renderer *, float);
    void tmp_renderer_scale2(struct tmp_renderer *, float, float);

    void tmp_renderer_save(struct tmp_renderer *);
    void tmp_renderer_restore(struct tmp_renderer *);

    void tmp_renderer_begin_path(struct tmp_renderer *);
    void tmp_renderer_move_to(struct tmp_renderer *, float, float);
    void tmp_renderer_line_to(struct tmp_renderer *, float, float);
    void tmp_renderer_quadratic_curve_to(struct tmp_renderer *, float, float,
                                         float, float);
    void tmp_renderer_bezier_curve_to(struct tmp_renderer *, float, float, float,
                                      float, float, float);
    void tmp_renderer_arc(struct tmp_renderer *, float, float, float);
    void tmp_renderer_reverse_arc(struct tmp_renderer *, float, float, float);
    void tmp_renderer_partial_arc(struct tmp_renderer *, float, float, float,
                                  float, float, uint8_t);
    void tmp_renderer_ellipse(struct tmp_renderer *, float, float, float, float);
    void tmp_renderer_rect(struct tmp_renderer *, float, float, float, float);
    void tmp_renderer_round_rect(struct tmp_renderer *, float, float, float,
                                 float, float);

    void tmp_renderer_draw_translated_image(struct tmp_renderer *,
                                            struct tmp_renderer *, float, float);
    void tmp_renderer_draw_clipped_image(struct tmp_renderer *,
                                         struct tmp_renderer *, float, float,
                                         float, float, float, float);
    void tmp_renderer_draw_image(struct tmp_renderer *, struct tmp_renderer *);
    void tmp_renderer_draw_svg(struct tmp_renderer *, char *svg, float x,
                               float y);

    void tmp_renderer_fill_rect(struct tmp_renderer *, float, float, float,
                                float);
    void tmp_renderer_stroke_rect(struct tmp_renderer *, float, float, float,
                                  float);
    void tmp_renderer_fill(struct tmp_renderer *);
    void tmp_renderer_stroke(struct tmp_renderer *);
    void tmp_renderer_clip(struct tmp_renderer *);
    void tmp_renderer_clip2(struct tmp_renderer *);

    void tmp_renderer_fill_text(struct tmp_renderer *, char const *, float,
                                float);
    void tmp_renderer_stroke_text(struct tmp_renderer *, char const *, float,
                                  float);

    float tmp_renderer_get_text_size(char const *);

    void tmp_renderer_execute_instructions();
    uint32_t tmp_renderer_get_op_size();
    void tmp_renderer_reset_instruction_queue();
#ifdef __cplusplus
}
#endif
