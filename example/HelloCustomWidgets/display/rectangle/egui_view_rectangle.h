#ifndef _HELLO_CUSTOM_WIDGETS_RECTANGLE_H_
#define _HELLO_CUSTOM_WIDGETS_RECTANGLE_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct egui_view_rectangle egui_view_rectangle_t;
struct egui_view_rectangle
{
    egui_view_t base;
    egui_color_t fill_color;
    egui_color_t stroke_color;
    egui_color_t accent_color;
    egui_dim_t stroke_width;
    egui_dim_t corner_radius;
    uint8_t fill_enabled;
};

void egui_view_rectangle_init(egui_view_t *self);
void egui_view_rectangle_set_palette(egui_view_t *self, egui_color_t fill_color, egui_color_t stroke_color, egui_color_t accent_color);
void egui_view_rectangle_set_stroke_width(egui_view_t *self, egui_dim_t stroke_width);
egui_dim_t egui_view_rectangle_get_stroke_width(egui_view_t *self);
void egui_view_rectangle_set_corner_radius(egui_view_t *self, egui_dim_t corner_radius);
egui_dim_t egui_view_rectangle_get_corner_radius(egui_view_t *self);
void egui_view_rectangle_set_fill_enabled(egui_view_t *self, uint8_t fill_enabled);
uint8_t egui_view_rectangle_get_fill_enabled(egui_view_t *self);
void egui_view_rectangle_apply_standard_style(egui_view_t *self);
void egui_view_rectangle_apply_accent_style(egui_view_t *self);
void egui_view_rectangle_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_RECTANGLE_H_ */
