#ifndef _HELLO_CUSTOM_WIDGETS_ELLIPSE_H_
#define _HELLO_CUSTOM_WIDGETS_ELLIPSE_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct egui_view_ellipse egui_view_ellipse_t;
struct egui_view_ellipse
{
    egui_view_t base;
    egui_color_t fill_color;
    egui_color_t stroke_color;
    egui_color_t accent_color;
    egui_dim_t stroke_width;
    uint8_t fill_enabled;
    uint8_t circle_mode;
};

void egui_view_ellipse_init(egui_view_t *self);
void egui_view_ellipse_set_palette(egui_view_t *self, egui_color_t fill_color, egui_color_t stroke_color, egui_color_t accent_color);
void egui_view_ellipse_set_stroke_width(egui_view_t *self, egui_dim_t stroke_width);
egui_dim_t egui_view_ellipse_get_stroke_width(egui_view_t *self);
void egui_view_ellipse_set_circle_mode(egui_view_t *self, uint8_t circle_mode);
uint8_t egui_view_ellipse_get_circle_mode(egui_view_t *self);
void egui_view_ellipse_set_fill_enabled(egui_view_t *self, uint8_t fill_enabled);
uint8_t egui_view_ellipse_get_fill_enabled(egui_view_t *self);
void egui_view_ellipse_apply_standard_style(egui_view_t *self);
void egui_view_ellipse_apply_accent_style(egui_view_t *self);
void egui_view_ellipse_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_ELLIPSE_H_ */
