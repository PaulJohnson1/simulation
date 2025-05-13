#include <GL/gl.h>
#include <SDL3/SDL.h>
#include <fenv.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <Simulation.h>
#include <Utilities.h>
#include <Window.h>

// #include <Ball.c>
// #include <Simulation.c>
// #include <SpatialHash.c>
// #include <Utilities.c>
// #include <Vector.c>
// #include <Window.c>

int main()
{
#if !defined(WASM_BUILD) && !defined(NDEBUG)
    feenableexcept(FE_INVALID | FE_DIVBYZERO);
#endif
    static struct tmp_window window = {0};
    struct tmp_simulation *simulation =
        (struct tmp_simulation *)malloc(sizeof *simulation);

    tmp_window_init(&window);
    tmp_simulation_init(simulation);

    window.simulation = simulation;

#ifdef NRENDER
    int const seconds = 10;
    int64_t const steps = 10;
    struct timeval start;
    gettimeofday(&start, NULL);
    uint64_t ticks = 0;
    while (1)
    {
        tmp_spatial_hash_optimize(&simulation->grid);
        for (int64_t i = 0; i < steps; i++)
        {
            tmp_simulation_subtick(simulation, 1.0f / (float)steps);
            ticks++;
        }
        struct timeval now;
        gettimeofday(&now, NULL);
        if (now.tv_sec - start.tv_sec > seconds)
        {
            printf("did %lu ticks in %d seconds\n", ticks, seconds);
            break;
        }
    }
    return 0;
#endif

    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

    SDL_Window *gwindow = SDL_CreateWindow("lol", TMP_WINDOW_SIZE,
                                           TMP_WINDOW_SIZE, SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(gwindow);
    SDL_GL_MakeCurrent(gwindow, context);

    glViewport(0, 0, TMP_WINDOW_SIZE, TMP_WINDOW_SIZE);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    SDL_Event event;
    int running = 1;
    while (running)
    {
        while (SDL_PollEvent(&event))
            if (event.type == SDL_EVENT_QUIT)
                running = 0;

        tmp_window_tick(&window, 0.016);
        SDL_GL_SwapWindow(gwindow);
    }

    SDL_Quit();

    tmp_window_tick(&window, 0.5);

    return 0;
}
