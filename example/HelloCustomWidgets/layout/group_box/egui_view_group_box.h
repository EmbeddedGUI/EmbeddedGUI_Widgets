#ifndef _HELLO_CUSTOM_WIDGETS_GROUP_BOX_H_
#define _HELLO_CUSTOM_WIDGETS_GROUP_BOX_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct egui_view_group_box egui_view_group_box_t;
struct egui_view_group_box
{
    egui_view_group_t base;
    egui_view_t *header;
    egui_view_t *content;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t header_surface_color;
    egui_color_t content_surface_color;
    egui_color_t accent_color;
    egui_dim_t corner_radius;
    egui_dim_t border_width;
    egui_dim_t header_gap;
    egui_dim_t header_indent;
    uint8_t header_align_type;
    uint8_t content_align_type;
    uint8_t compact_mode;
    uint8_t read_only_mode;
};

void egui_view_group_box_init(egui_view_t *self);
void egui_view_group_box_set_header(egui_view_t *self, egui_view_t *header);
egui_view_t *egui_view_group_box_get_header(egui_view_t *self);
void egui_view_group_box_set_content(egui_view_t *self, egui_view_t *content);
egui_view_t *egui_view_group_box_get_content(egui_view_t *self);
void egui_view_group_box_layout_childs(egui_view_t *self);
void egui_view_group_box_set_padding(egui_view_t *self, egui_dim_margin_padding_t left, egui_dim_margin_padding_t right,
                                     egui_dim_margin_padding_t top, egui_dim_margin_padding_t bottom);
void egui_view_group_box_set_header_gap(egui_view_t *self, egui_dim_t gap);
egui_dim_t egui_view_group_box_get_header_gap(egui_view_t *self);
void egui_view_group_box_set_header_indent(egui_view_t *self, egui_dim_t indent);
egui_dim_t egui_view_group_box_get_header_indent(egui_view_t *self);
void egui_view_group_box_set_header_align_type(egui_view_t *self, uint8_t align_type);
uint8_t egui_view_group_box_get_header_align_type(egui_view_t *self);
void egui_view_group_box_set_content_align_type(egui_view_t *self, uint8_t align_type);
uint8_t egui_view_group_box_get_content_align_type(egui_view_t *self);
void egui_view_group_box_set_corner_radius(egui_view_t *self, egui_dim_t radius);
egui_dim_t egui_view_group_box_get_corner_radius(egui_view_t *self);
void egui_view_group_box_set_border_width(egui_view_t *self, egui_dim_t width);
egui_dim_t egui_view_group_box_get_border_width(egui_view_t *self);
void egui_view_group_box_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color,
                                     egui_color_t header_surface_color, egui_color_t content_surface_color, egui_color_t accent_color);
void egui_view_group_box_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_group_box_get_compact_mode(egui_view_t *self);
void egui_view_group_box_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_group_box_get_read_only_mode(egui_view_t *self);
void egui_view_group_box_apply_standard_style(egui_view_t *self);
void egui_view_group_box_apply_accent_style(egui_view_t *self);
void egui_view_group_box_apply_compact_style(egui_view_t *self);
void egui_view_group_box_apply_read_only_style(egui_view_t *self);
void egui_view_group_box_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_GROUP_BOX_H_ */
