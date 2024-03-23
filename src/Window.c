#include <Window.h>

#include <stdio.h>
#include <sys/time.h>

#include <Renderer/Renderer.h>
#include <Simulation.h>

void tmp_window_init(struct tmp_window *w)
{
}

void tmp_window_tick(struct tmp_window *w, float delta)
{
    tmp_simulation_tick(w->simulation, 1);

    struct tmp_renderer_context_state state;
    tmp_renderer_context_state_init(w->renderer, &state);
    // identity matrix
    state.transform_matrix[0] = 1;
    state.transform_matrix[4] = 1;
    {
        tmp_renderer_scale(w->renderer, 800.0f / TMP_MAP_SIZE);
        tmp_simulation_render(w->simulation, w->renderer);
    }
    tmp_renderer_context_state_free(w->renderer, &state);

    tmp_renderer_execute_instructions();
    tmp_renderer_reset_instruction_queue();
}
