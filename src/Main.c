#include <Renderer/Renderer.h>
#include <Window.h>
#include <Bitset.h>
#include <Simulation.h>
#include <Utilities.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#ifndef WASM_BUILD
#include <GLFW/glfw3.h>
#include <fenv.h>
#endif

#ifdef WASM_BUILD
#include <emscripten.h>
void tmp_key_event(struct tmp_window *this, uint8_t type, uint32_t which,
                   uint32_t key)
{
    if (type == 1)
    {
        bs_bitset_set(this->input_data->keys_pressed, which);
        bs_bitset_set(this->input_data->keys_pressed_this_tick, which);
        if (this->input_data->keycodes_length < 16 && key != 0)
            this->input_data->keycodes_pressed_this_tick[this->input_data->keycodes_length++] = key;
    }
    else
    {
        bs_bitset_unset(this->input_data->keys_pressed, which);
        bs_bitset_unset(this->input_data->keys_released_this_tick, which);
    }
}

void tmp_mouse_event(struct tmp_window *this, float x, float y, uint8_t state,
                     uint8_t button)
{
    this->input_data->mouse_x = x;
    this->input_data->mouse_y = y;
    this->input_data->mouse_state = state;
    if (state == 1) // press down
    {
        this->input_data->mouse_buttons |= (1 << button);
        this->input_data->mouse_buttons_down_this_tick |= (1 << button);
    }
    else if (state == 0)
    {
        this->input_data->mouse_buttons &= ~(1 << button);
        this->input_data->mouse_buttons_up_this_tick |= (1 << button);
    }
}

void tmp_touch_event(struct tmp_window *this, float x, float y, uint8_t state, uint8_t identifier)
{
    struct tmp_input_touch *touch = &this->input_data->touches[identifier % 16];
    touch->touch_x = x;
    touch->touch_y = y;
    touch->identifier = identifier;
    touch->active = state;
}

void tmp_wheel_event(struct tmp_window *this, float delta)
{
    this->input_data->scroll_delta = delta;
}

void tmp_paste_event(struct tmp_window *this, char *buf)
{
    if (this->input_data->clipboard != NULL)
        free(this->input_data->clipboard);
    this->input_data->clipboard = buf;
}

#else
#endif

void tmp_main_loop(struct tmp_window *this)
{
    puts("int main\n");
#ifdef EMSCRIPTEN
    EM_ASM(
        {
            Module.canvas = document.createElement("canvas");
            Module.canvas.id = "canvas";
            Module.canvas.oncontextmenu = function() { return false; };
            window.onbeforeunload = function(e) { return "Are you sure?"; };
            document.body.appendChild(Module.canvas);
            Module.ctxs = [Module.canvas.getContext('2d')];
            Module.availableCtxs =
                new Array(256).fill(0).map(function(_, i) { return i; });
            window.onkeydown = function(e)
            {
                _tmp_key_event(
                    $0, 1, e.which, e.key ? (!e.ctrlKey && !e.metaKey && e.key.length == 1) * e.key.charCodeAt() : 0);
            };
            window.onkeyup = function(e)
            {
                _tmp_key_event(
                    $0, 0, e.which, e.key ? (!e.ctrlKey && !e.metaKey && e.key.length == 1) * e.key.charCodeAt() : 0);
            };
            window.addEventListener(
                "mousedown", async function(e) {
                    const clientX = e.clientX;
                    const clientY = e.clientY;
                    const button = e.button;
                    _tmp_mouse_event($0, clientX * devicePixelRatio,
                                     clientY * devicePixelRatio, 1, +!!button);
                });
            window.addEventListener(
                "mousemove", async function(e) {
                    const clientX = e.clientX;
                    const clientY = e.clientY;
                    const button = e.button;
                    _tmp_mouse_event($0, clientX * devicePixelRatio,
                                     clientY * devicePixelRatio, 2, +!!button);
                });
            window.addEventListener(
                "mouseup", async function(e) {
                    const clientX = e.clientX;
                    const clientY = e.clientY;
                    const button = e.button;
                    _tmp_mouse_event($0, clientX * devicePixelRatio,
                                     clientY * devicePixelRatio, 0, +!!button);
                });
            window.addEventListener("touchstart", function(e) {
                e.preventDefault();
                e.stopPropagation();
                if (!e.changedTouches.length)
                    return;
                const touch = e.changedTouches[0];
                _tmp_mouse_event(
                    $0, touch.clientX * devicePixelRatio, touch.clientY * devicePixelRatio,
                    1, 0);
                for (const t of e.changedTouches)
                    _tmp_touch_event($0, t.clientX * devicePixelRatio, t.clientY * devicePixelRatio, 1, t.identifier);
            },
                                    {passive : false});
            window.addEventListener("touchmove", function(e) {
                e.preventDefault();
                e.stopPropagation();
                if (!e.changedTouches.length)
                    return;
                const touch = e.changedTouches[0];
                _tmp_mouse_event(
                    $0, touch.clientX * devicePixelRatio, touch.clientY * devicePixelRatio,
                    2, 0);
                for (const t of e.changedTouches)
                    _tmp_touch_event($0, t.clientX * devicePixelRatio, t.clientY * devicePixelRatio, 1, t.identifier);
            },
                                    {passive : false});
            window.addEventListener("touchend", function(e) {
                e.preventDefault();
                e.stopPropagation();
                if (!e.changedTouches.length)
                    return;
                const touch = e.changedTouches[0];
                _tmp_mouse_event(
                    $0, touch.clientX * devicePixelRatio, touch.clientY * devicePixelRatio,
                    0, 0);
                for (const t of e.changedTouches)
                    _tmp_touch_event($0, t.clientX * devicePixelRatio, t.clientY * devicePixelRatio, 0, t.identifier);
            },
                                    {passive : false});
            window.addEventListener(
                "wheel",
                async function({deltaY}) { _tmp_wheel_event($0, deltaY); });
            document.body.addEventListener(
                "paste", async function(e) {
                    const buf = new TextEncoder().encode(e.clipboardData.getData("text/plain"));
                    const $a = _malloc(buf.length + 1);
                    HEAPU8.set(buf, $a);
                    HEAPU8[$a + buf.length] = 0;
                    _tmp_paste_event($0, $a);
                });
            Module.addCtx = function()
            {
                if (Module.availableCtxs.length)
                {
                    const index = Module.availableCtxs.shift();
                    if (index == 0)
                        return 0; // used for the main ctx, because that has
                                  // special behavior
                    const ocanvas = new OffscreenCanvas(1, 1);
                    Module.ctxs[index] = ocanvas.getContext('2d');
                    return index;
                }
                throw new Error(
                    'Out of Contexts: Can be fixed by allowing more contexts');
                return -1;
            };
            Module.removeCtx = function(index)
            {
                if (index == 0)
                    throw new Error('Tried to delete the main context');
                Module.ctxs[index] = null;
                Module.availableCtxs.push(index);
            };

            function loop(time)
            {
                if (window.start == null)
                    window.start = time + 1;
                const delta = Math.min(0.5, (time - window.start) / 1000);
                window.start = time;
                Module.canvas.width = innerWidth * devicePixelRatio;
                Module.canvas.height = innerHeight * devicePixelRatio;
                _tmp_renderer_main_loop($0, delta, Module.canvas.width,
                                        Module.canvas.height,
                                        devicePixelRatio);
                requestAnimationFrame(loop);
            };
            requestAnimationFrame(loop);
        },
        this, &this->input_data->mouse_x, &this->input_data->mouse_state);
#endif
}

void tmp_renderer_main_loop(struct tmp_window *this, float delta, float width,
                            float height, float device_pixel_ratio)
{
    float a = height / 1080;
    float b = width / 1920;

    float scale = (this->renderer->scale = b < a ? a : b) * device_pixel_ratio;
    this->renderer->width = width;
    this->renderer->height = height;
    tmp_window_tick(this, delta);
    this->input_data->scroll_delta = 0;
}

int main()
{
#if !defined(WASM_BUILD) && !defined(NDEBUG)
    feenableexcept(FE_INVALID | FE_DIVBYZERO);
#endif
    tmp_init_lookup_tables();

    static struct tmp_window window = {0};
    static struct tmp_renderer renderer = {0};
    static struct tmp_input_data input_data = {0};
    struct tmp_simulation *simulation = malloc(sizeof *simulation);
    tmp_main_loop(&window);

    tmp_renderer_init(&renderer);
    tmp_window_init(&window);
    tmp_simulation_init(simulation);

    window.renderer = &renderer;
    window.input_data = &input_data;
    window.simulation = simulation;

#ifndef WASM_BUILD
    glfwInit();
    GLFWwindow *gwindow = glfwCreateWindow(800, 800, "lol", NULL, NULL);
    glfwMakeContextCurrent(gwindow);
    __tmp_renderer_init_native(&window, gwindow);
#endif

    tmp_window_tick(&window, 0.5);

    return 0;
}
