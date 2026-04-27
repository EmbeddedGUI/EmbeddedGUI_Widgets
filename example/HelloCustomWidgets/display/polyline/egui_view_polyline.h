#ifndef _HELLO_CUSTOM_WIDGETS_POLYLINE_H_
#define _HELLO_CUSTOM_WIDGETS_POLYLINE_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_POLYLINE_MAX_POINTS 8

typedef struct egui_view_polyline egui_view_polyline_t;
struct egui_view_polyline
{
    egui_view_t base;
    egui_color_t stroke_color;
    egui_color_t accent_color;
    egui_dim_t stroke_width;
    uint8_t point_count;
    uint8_t points_percent[EGUI_VIEW_POLYLINE_MAX_POINTS * 2];
    uint8_t compact_mode;
    uint8_t read_only_mode;
};

void egui_view_polyline_init(egui_view_t *self);
void egui_view_polyline_set_palette(egui_view_t *self, egui_color_t stroke_color, egui_color_t accent_color);
void egui_view_polyline_set_stroke_width(egui_view_t *self, egui_dim_t stroke_width);
egui_dim_t egui_view_polyline_get_stroke_width(egui_view_t *self);
void egui_view_polyline_set_points(egui_view_t *self, const uint8_t *points_percent, uint8_t point_count);
uint8_t egui_view_polyline_get_point_count(egui_view_t *self);
void egui_view_polyline_get_point(egui_view_t *self, uint8_t index, uint8_t *x_percent, uint8_t *y_percent);
void egui_view_polyline_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_polyline_get_compact_mode(egui_view_t *self);
void egui_view_polyline_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_polyline_get_read_only_mode(egui_view_t *self);
void egui_view_polyline_apply_standard_style(egui_view_t *self);
void egui_view_polyline_apply_accent_style(egui_view_t *self);
void egui_view_polyline_apply_compact_style(egui_view_t *self);
void egui_view_polyline_apply_read_only_style(egui_view_t *self);
void egui_view_polyline_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_POLYLINE_H_ */
