#ifndef _HELLO_CUSTOM_WIDGETS_CONTENT_PRESENTER_H_
#define _HELLO_CUSTOM_WIDGETS_CONTENT_PRESENTER_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct egui_view_content_presenter egui_view_content_presenter_t;
struct egui_view_content_presenter
{
    egui_view_group_t base;
    egui_view_t *child;
    egui_color_t surface_color;
    egui_color_t guide_color;
    egui_color_t accent_color;
    egui_dim_t corner_radius;
    egui_dim_t guide_width;
    uint8_t content_align_type;
    uint8_t compact_mode;
    uint8_t read_only_mode;
};

void egui_view_content_presenter_init(egui_view_t *self);
void egui_view_content_presenter_set_child(egui_view_t *self, egui_view_t *child);
egui_view_t *egui_view_content_presenter_get_child(egui_view_t *self);
void egui_view_content_presenter_layout_child(egui_view_t *self);
void egui_view_content_presenter_set_padding(egui_view_t *self, egui_dim_margin_padding_t left, egui_dim_margin_padding_t right,
                                             egui_dim_margin_padding_t top, egui_dim_margin_padding_t bottom);
void egui_view_content_presenter_set_content_align_type(egui_view_t *self, uint8_t align_type);
uint8_t egui_view_content_presenter_get_content_align_type(egui_view_t *self);
void egui_view_content_presenter_set_corner_radius(egui_view_t *self, egui_dim_t radius);
egui_dim_t egui_view_content_presenter_get_corner_radius(egui_view_t *self);
void egui_view_content_presenter_set_guide_width(egui_view_t *self, egui_dim_t width);
egui_dim_t egui_view_content_presenter_get_guide_width(egui_view_t *self);
void egui_view_content_presenter_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t guide_color,
                                             egui_color_t accent_color);
void egui_view_content_presenter_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_content_presenter_get_compact_mode(egui_view_t *self);
void egui_view_content_presenter_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_content_presenter_get_read_only_mode(egui_view_t *self);
void egui_view_content_presenter_apply_standard_style(egui_view_t *self);
void egui_view_content_presenter_apply_template_style(egui_view_t *self);
void egui_view_content_presenter_apply_compact_style(egui_view_t *self);
void egui_view_content_presenter_apply_read_only_style(egui_view_t *self);
void egui_view_content_presenter_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_CONTENT_PRESENTER_H_ */
