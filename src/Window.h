#pragma once

struct tmp_simulation;
struct tmp_input_data;

struct tmp_window
{
    struct tmp_simulation *simulation;
    struct tmp_input_data *input_data;
};

#ifdef __cplusplus
extern "C" {
#endif

extern void tmp_window_init(struct tmp_window *);
extern void tmp_window_tick(struct tmp_window *, float delta);

#ifdef __cplusplus
}
#endif
