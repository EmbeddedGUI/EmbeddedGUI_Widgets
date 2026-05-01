#ifndef _HELLO_CUSTOM_WIDGETS_SHAPE_LINE_H_
#define _HELLO_CUSTOM_WIDGETS_SHAPE_LINE_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct egui_view_shape_line egui_view_shape_line_t;
struct egui_view_shape_line
{
    egui_view_t base;
    egui_color_t stroke_color;
    egui_color_t accent_color;
    egui_dim_t stroke_width;
    uint8_t x1_percent;
    uint8_t y1_percent;
    uint8_t x2_percent;
    uint8_t y2_percent;
};

void egui_view_shape_line_init(egui_view_t *self);
void egui_view_shape_line_set_palette(egui_view_t *self, egui_color_t stroke_color, egui_color_t accent_color);
void egui_view_shape_line_set_stroke_width(egui_view_t *self, egui_dim_t stroke_width);
egui_dim_t egui_view_shape_line_get_stroke_width(egui_view_t *self);
void egui_view_shape_line_set_points(egui_view_t *self, uint8_t x1_percent, uint8_t y1_percent, uint8_t x2_percent, uint8_t y2_percent);
void egui_view_shape_line_get_points(egui_view_t *self, uint8_t *x1_percent, uint8_t *y1_percent, uint8_t *x2_percent, uint8_t *y2_percent);
void egui_view_shape_line_apply_standard_style(egui_view_t *self);
void egui_view_shape_line_apply_accent_style(egui_view_t *self);
void egui_view_shape_line_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_SHAPE_LINE_H_ */
