#ifndef _HELLO_CUSTOM_WIDGETS_RESIZE_GRIP_H_
#define _HELLO_CUSTOM_WIDGETS_RESIZE_GRIP_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    EGUI_VIEW_RESIZE_GRIP_CORNER_BOTTOM_RIGHT = 0,
    EGUI_VIEW_RESIZE_GRIP_CORNER_BOTTOM_LEFT,
} egui_view_resize_grip_corner_t;

typedef struct egui_view_resize_grip egui_view_resize_grip_t;
struct egui_view_resize_grip
{
    egui_view_t base;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t dot_color;
    egui_color_t accent_color;
    egui_dim_t grip_size;
    egui_dim_t dot_size;
    egui_dim_t dot_gap;
    egui_dim_t corner_radius;
    uint8_t corner;
    uint8_t compact_mode;
    uint8_t disabled_mode;
    uint8_t read_only_mode;
};

void egui_view_resize_grip_init(egui_view_t *self);
void egui_view_resize_grip_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color,
                                       egui_color_t dot_color, egui_color_t accent_color);
void egui_view_resize_grip_set_metrics(egui_view_t *self, egui_dim_t grip_size, egui_dim_t dot_size, egui_dim_t dot_gap);
egui_dim_t egui_view_resize_grip_get_grip_size(egui_view_t *self);
egui_dim_t egui_view_resize_grip_get_dot_size(egui_view_t *self);
egui_dim_t egui_view_resize_grip_get_dot_gap(egui_view_t *self);
void egui_view_resize_grip_set_corner_radius(egui_view_t *self, egui_dim_t radius);
egui_dim_t egui_view_resize_grip_get_corner_radius(egui_view_t *self);
void egui_view_resize_grip_set_corner(egui_view_t *self, uint8_t corner);
uint8_t egui_view_resize_grip_get_corner(egui_view_t *self);
void egui_view_resize_grip_get_grip_region(egui_view_t *self, egui_region_t *grip_region);
void egui_view_resize_grip_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_resize_grip_get_compact_mode(egui_view_t *self);
void egui_view_resize_grip_set_disabled_mode(egui_view_t *self, uint8_t disabled_mode);
uint8_t egui_view_resize_grip_get_disabled_mode(egui_view_t *self);
void egui_view_resize_grip_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_resize_grip_get_read_only_mode(egui_view_t *self);
void egui_view_resize_grip_apply_standard_style(egui_view_t *self);
void egui_view_resize_grip_apply_accent_style(egui_view_t *self);
void egui_view_resize_grip_apply_compact_style(egui_view_t *self);
void egui_view_resize_grip_apply_disabled_style(egui_view_t *self);
void egui_view_resize_grip_apply_read_only_style(egui_view_t *self);
void egui_view_resize_grip_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_RESIZE_GRIP_H_ */
