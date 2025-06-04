#include <Window.h>

#include <stdio.h>
#include <sys/time.h>

#include <SDL3/SDL_opengl.h>

#include <Simulation.h>

void tmp_window_init(struct tmp_window *w) { (void)w; }

void tmp_window_tick(struct tmp_window *w, float delta)
{
    (void)w;
    (void)delta;
    tmp_simulation_tick(w->simulation);
#ifdef NRENDER
    return;
#endif

    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, TMP_MAP_SIZE, TMP_MAP_SIZE, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);
    glScalef(2.0f / TMP_MAP_SIZE, 2.0f / TMP_MAP_SIZE, 1);
    glTranslatef(-TMP_MAP_SIZE / 2, -TMP_MAP_SIZE / 2, 0);
    tmp_simulation_render(w->simulation);
    glPopMatrix();
}
