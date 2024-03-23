#include <Renderer/Renderer.h>

#include <cstring>
#include <unistd.h>
#include <iostream>
#include <sys/time.h>

#include <GLFW/glfw3.h>
#define SK_GL
void SkDebugf(const char format[], ...)
{
}
#include <include/core/SkCanvas.h>
#include <include/core/SkData.h>
#include <include/core/SkImage.h>
#include <include/core/SkPath.h>
#include <include/core/SkGraphics.h>
#include <include/core/SkImageEncoder.h>
#include <include/core/SkString.h>
#include <include/core/SkSurface.h>
#include <include/core/SkStream.h>
#include <include/core/SkTextBlob.h>
#include <include/core/SkColorSpace.h>
#include <include/gpu/GrBackendSurface.h>
#include <include/gpu/GrDirectContext.h>
#include <include/gpu/gl/GrGLInterface.h>

#include <Window.h>

void __tmp_renderer_init_native(struct tmp_window *twindow, void *_glfw_window)
{
    GLFWwindow *window = (GLFWwindow *)_glfw_window;
    sk_sp<GrGLInterface const> interface = GrGLMakeNativeInterface();

    GrDirectContext *context = GrDirectContext::MakeGL(interface).release();
    GrGLFramebufferInfo framebufferInfo;
    framebufferInfo.fFBOID = 0;
    framebufferInfo.fFormat = GL_RGBA8;
    SkColorType colorType = kRGBA_8888_SkColorType;
    GrBackendRenderTarget backendRenderTarget(800, 800,
                                              0, // sample count
                                              0, // stencil bits
                                              framebufferInfo);

    SkSurface *surface = SkSurface::MakeFromBackendRenderTarget(context, backendRenderTarget, kBottomLeft_GrSurfaceOrigin, colorType, nullptr, nullptr).release();
    SkCanvas *canvas = surface->getCanvas();
    twindow->renderer->state.path = new SkPath;
    twindow->renderer->surface = (void *)surface;
    twindow->renderer->canvas = (void *)canvas;

    struct timeval start;
    struct timeval end;
    while (!glfwWindowShouldClose(window))
    {
        context->flush();
        canvas->clear(-1);
        gettimeofday(&start, NULL);
        tmp_window_tick(twindow, 0.606);
        gettimeofday(&end, NULL);
        uint64_t elapsed_time = (end.tv_sec - start.tv_sec) * 1000000 +
                                (end.tv_usec - start.tv_usec);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

#define RETRIEVE_CANVAS                     \
    SkCanvas *c = (SkCanvas *)r->canvas;    \
    SkSurface *s = (SkSurface *)r->surface; \
    SkPath *p = (SkPath *)r->state.path;

void tmp_renderer_init(struct tmp_renderer *r) {}
void tmp_renderer_set_dimensions(struct tmp_renderer *r, float, float) {}

void tmp_renderer_set_fill(struct tmp_renderer *r, uint32_t) {}
void tmp_renderer_set_stroke(struct tmp_renderer *r, uint32_t) {}

void tmp_renderer_set_line_width(struct tmp_renderer *r, float) {}
void tmp_renderer_set_text_size(struct tmp_renderer *r, float) {}
void tmp_renderer_set_global_alpha(struct tmp_renderer *r, float) {}

void tmp_renderer_set_line_cap(struct tmp_renderer *r, uint8_t) {}
void tmp_renderer_set_line_join(struct tmp_renderer *r, uint8_t) {}
void tmp_renderer_set_text_align(struct tmp_renderer *r, uint8_t) {}
void tmp_renderer_set_text_baseline(struct tmp_renderer *r, uint8_t) {}

void tmp_renderer_update_transform(struct tmp_renderer *r)
{
    RETRIEVE_CANVAS
    SkMatrix m;
    m.setAll(
        r->state.transform_matrix[0],
        r->state.transform_matrix[1],
        r->state.transform_matrix[2],
        r->state.transform_matrix[3],
        r->state.transform_matrix[4],
        r->state.transform_matrix[5], 0, 0, 1);
    c->setMatrix(m);
}

void tmp_renderer_circle(struct tmp_renderer *r, float x, float y, float rad)
{
    RETRIEVE_CANVAS
    SkPaint paint;
    paint.setColor4f(SkColor4f{0, 0, 0, 1});
    paint.setAntiAlias(true);
    c->drawCircle(x, y, rad, paint);
}

void tmp_renderer_save(struct tmp_renderer *r)
{
    RETRIEVE_CANVAS
    c->save();
}
void tmp_renderer_restore(struct tmp_renderer *r)
{
    RETRIEVE_CANVAS
    c->restore();
}

void tmp_renderer_begin_path(struct tmp_renderer *r)
{
    RETRIEVE_CANVAS
    p->~SkPath();
    new (p) SkPath();
}
void tmp_renderer_move_to(struct tmp_renderer *r, float, float) {}
void tmp_renderer_line_to(struct tmp_renderer *r, float, float) {}
void tmp_renderer_quadratic_curve_to(struct tmp_renderer *r, float, float,
                                     float, float) {}
void tmp_renderer_bezier_curve_to(struct tmp_renderer *r, float, float, float,
                                  float, float, float) {}
void tmp_renderer_partial_arc(struct tmp_renderer *r, float x, float y, float rad,
                              float start, float end, uint8_t ccw)
{
    RETRIEVE_CANVAS
    p->addCircle(x, y, rad, ccw ? SkPathDirection::kCCW : SkPathDirection::kCW);
}
void tmp_renderer_ellipse(struct tmp_renderer *r, float, float, float, float) {}
void tmp_renderer_rect(struct tmp_renderer *r, float, float, float, float) {}

void tmp_renderer_draw_translated_image(struct tmp_renderer *r,
                                        struct tmp_renderer *, float, float) {}
void tmp_renderer_draw_clipped_image(struct tmp_renderer *r,
                                     struct tmp_renderer *, float, float,
                                     float, float, float, float) {}
void tmp_renderer_draw_image(struct tmp_renderer *r, struct tmp_renderer *) {}
void tmp_renderer_draw_svg(struct tmp_renderer *r, char *svg, float x,
                           float y) {}

void tmp_renderer_fill_rect(struct tmp_renderer *r, float, float, float,
                            float) {}
void tmp_renderer_stroke_rect(struct tmp_renderer *r, float, float, float,
                              float) {}
void tmp_renderer_fill(struct tmp_renderer *r)
{
    RETRIEVE_CANVAS

    SkPaint paint;
    paint.setColor4f(SkColor4f{0, 0, 0, 1});
    paint.setAntiAlias(true);
    c->drawPath(*p, paint);
}
void tmp_renderer_stroke(struct tmp_renderer *r) {}
void tmp_renderer_clip(struct tmp_renderer *r) {}
void tmp_renderer_clip2(struct tmp_renderer *r) {}

void tmp_renderer_fill_text(struct tmp_renderer *r, char const *, float,
                            float) {}
void tmp_renderer_stroke_text(struct tmp_renderer *r, char const *, float,
                              float) {}

float tmp_renderer_get_text_size(char const *) { return 0; }

void tmp_renderer_execute_instructions() {}
uint32_t tmp_renderer_get_op_size() { return 0; }
void tmp_renderer_reset_instruction_queue() {}
