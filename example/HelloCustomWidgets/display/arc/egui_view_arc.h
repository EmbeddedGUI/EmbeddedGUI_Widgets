#ifndef _EGUI_VIEW_ARC_H_
#define _EGUI_VIEW_ARC_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct egui_view_arc egui_view_arc_t;
struct egui_view_arc
{
    egui_view_t base;
    uint8_t value;
    int16_t start_angle;
    int16_t sweep_angle;
    egui_dim_t stroke_width;
    egui_color_t track_color;
    egui_color_t active_color;
};

void egui_view_arc_init(egui_view_t *self);
void egui_view_arc_apply_standard_style(egui_view_t *self);
void egui_view_arc_apply_subtle_style(egui_view_t *self);
void egui_view_arc_apply_attention_style(egui_view_t *self);
void egui_view_arc_set_value(egui_view_t *self, uint8_t value);
uint8_t egui_view_arc_get_value(egui_view_t *self);
void egui_view_arc_set_palette(egui_view_t *self, egui_color_t track_color, egui_color_t active_color);
void egui_view_arc_set_angles(egui_view_t *self, int16_t start_angle, int16_t sweep_angle);
void egui_view_arc_set_stroke_width(egui_view_t *self, egui_dim_t stroke_width);
void egui_view_arc_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_ARC_H_ */
