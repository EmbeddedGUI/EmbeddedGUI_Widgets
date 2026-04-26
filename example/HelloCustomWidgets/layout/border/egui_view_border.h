#ifndef _HELLO_CUSTOM_WIDGETS_BORDER_H_
#define _HELLO_CUSTOM_WIDGETS_BORDER_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct egui_view_border egui_view_border_t;
struct egui_view_border
{
    egui_view_group_t base;
    egui_view_t *child;
    egui_color_t background_color;
    egui_color_t border_color;
    egui_color_t accent_color;
    egui_dim_t corner_radius;
    egui_dim_t border_width;
    uint8_t compact_mode;
    uint8_t read_only_mode;
};

void egui_view_border_init(egui_view_t *self);
void egui_view_border_set_child(egui_view_t *self, egui_view_t *child);
egui_view_t *egui_view_border_get_child(egui_view_t *self);
void egui_view_border_layout_child(egui_view_t *self);
void egui_view_border_set_padding(egui_view_t *self, egui_dim_margin_padding_t left, egui_dim_margin_padding_t right,
                                  egui_dim_margin_padding_t top, egui_dim_margin_padding_t bottom);
void egui_view_border_set_corner_radius(egui_view_t *self, egui_dim_t radius);
egui_dim_t egui_view_border_get_corner_radius(egui_view_t *self);
void egui_view_border_set_border_width(egui_view_t *self, egui_dim_t width);
egui_dim_t egui_view_border_get_border_width(egui_view_t *self);
void egui_view_border_set_palette(egui_view_t *self, egui_color_t background_color, egui_color_t border_color,
                                  egui_color_t accent_color);
void egui_view_border_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_border_get_compact_mode(egui_view_t *self);
void egui_view_border_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_border_get_read_only_mode(egui_view_t *self);
void egui_view_border_apply_standard_style(egui_view_t *self);
void egui_view_border_apply_accent_style(egui_view_t *self);
void egui_view_border_apply_compact_style(egui_view_t *self);
void egui_view_border_apply_read_only_style(egui_view_t *self);
void egui_view_border_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_BORDER_H_ */
