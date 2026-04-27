#ifndef _HELLO_CUSTOM_WIDGETS_ADORNER_DECORATOR_H_
#define _HELLO_CUSTOM_WIDGETS_ADORNER_DECORATOR_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_ADORNER_DECORATOR_ADORNER_FOCUS      0x01
#define EGUI_VIEW_ADORNER_DECORATOR_ADORNER_VALIDATION 0x02
#define EGUI_VIEW_ADORNER_DECORATOR_ADORNER_RESIZE     0x04

typedef struct egui_view_adorner_decorator egui_view_adorner_decorator_t;
struct egui_view_adorner_decorator
{
    egui_view_group_t base;
    egui_view_t *child;
    egui_color_t surface_color;
    egui_color_t child_surface_color;
    egui_color_t child_border_color;
    egui_color_t focus_color;
    egui_color_t validation_color;
    egui_color_t resize_color;
    egui_dim_t corner_radius;
    egui_dim_t layer_inset;
    uint8_t adorner_flags;
    uint8_t compact_mode;
    uint8_t read_only_mode;
};

void egui_view_adorner_decorator_init(egui_view_t *self);
void egui_view_adorner_decorator_set_child(egui_view_t *self, egui_view_t *child);
egui_view_t *egui_view_adorner_decorator_get_child(egui_view_t *self);
void egui_view_adorner_decorator_layout_child(egui_view_t *self);
void egui_view_adorner_decorator_set_padding(egui_view_t *self, egui_dim_margin_padding_t left, egui_dim_margin_padding_t right, egui_dim_margin_padding_t top,
                                             egui_dim_margin_padding_t bottom);
void egui_view_adorner_decorator_set_adorner_flags(egui_view_t *self, uint8_t adorner_flags);
uint8_t egui_view_adorner_decorator_get_adorner_flags(egui_view_t *self);
void egui_view_adorner_decorator_set_corner_radius(egui_view_t *self, egui_dim_t corner_radius);
egui_dim_t egui_view_adorner_decorator_get_corner_radius(egui_view_t *self);
void egui_view_adorner_decorator_set_layer_inset(egui_view_t *self, egui_dim_t layer_inset);
egui_dim_t egui_view_adorner_decorator_get_layer_inset(egui_view_t *self);
void egui_view_adorner_decorator_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t child_surface_color, egui_color_t child_border_color,
                                             egui_color_t focus_color, egui_color_t validation_color, egui_color_t resize_color);
void egui_view_adorner_decorator_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_adorner_decorator_get_compact_mode(egui_view_t *self);
void egui_view_adorner_decorator_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_adorner_decorator_get_read_only_mode(egui_view_t *self);
void egui_view_adorner_decorator_apply_standard_style(egui_view_t *self);
void egui_view_adorner_decorator_apply_validation_style(egui_view_t *self);
void egui_view_adorner_decorator_apply_resize_style(egui_view_t *self);
void egui_view_adorner_decorator_apply_compact_style(egui_view_t *self);
void egui_view_adorner_decorator_apply_read_only_style(egui_view_t *self);
void egui_view_adorner_decorator_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_ADORNER_DECORATOR_H_ */
