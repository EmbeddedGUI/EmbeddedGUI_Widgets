#ifndef _HELLO_CUSTOM_WIDGETS_BLOCK_UI_CONTAINER_H_
#define _HELLO_CUSTOM_WIDGETS_BLOCK_UI_CONTAINER_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_BLOCK_UI_CONTAINER_MAX_TEXT_LEN 39

typedef struct egui_view_block_ui_container egui_view_block_ui_container_t;
struct egui_view_block_ui_container
{
    egui_view_group_t base;
    egui_view_t *child;
    const egui_font_t *text_font;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t host_surface_color;
    egui_color_t host_border_color;
    egui_color_t accent_color;
    char leading_text[EGUI_VIEW_BLOCK_UI_CONTAINER_MAX_TEXT_LEN + 1];
    char trailing_text[EGUI_VIEW_BLOCK_UI_CONTAINER_MAX_TEXT_LEN + 1];
    egui_dim_t host_padding_x;
    egui_dim_t host_padding_y;
    egui_dim_t block_gap;
    egui_dim_t corner_radius;
    uint8_t compact_mode;
    uint8_t read_only_mode;
};

void egui_view_block_ui_container_init(egui_view_t *self);
void egui_view_block_ui_container_set_child(egui_view_t *self, egui_view_t *child);
egui_view_t *egui_view_block_ui_container_get_child(egui_view_t *self);
void egui_view_block_ui_container_layout_child(egui_view_t *self);
void egui_view_block_ui_container_set_text(egui_view_t *self, const char *leading_text, const char *trailing_text);
const char *egui_view_block_ui_container_get_leading_text(egui_view_t *self);
const char *egui_view_block_ui_container_get_trailing_text(egui_view_t *self);
void egui_view_block_ui_container_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_block_ui_container_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                              egui_color_t host_surface_color, egui_color_t host_border_color, egui_color_t accent_color);
void egui_view_block_ui_container_set_metrics(egui_view_t *self, egui_dim_t host_padding_x, egui_dim_t host_padding_y, egui_dim_t block_gap,
                                              egui_dim_t corner_radius);
egui_dim_t egui_view_block_ui_container_get_host_padding_x(egui_view_t *self);
egui_dim_t egui_view_block_ui_container_get_host_padding_y(egui_view_t *self);
egui_dim_t egui_view_block_ui_container_get_block_gap(egui_view_t *self);
egui_dim_t egui_view_block_ui_container_get_corner_radius(egui_view_t *self);
void egui_view_block_ui_container_get_regions(egui_view_t *self, egui_region_t *leading_region, egui_region_t *host_region, egui_region_t *trailing_region);
void egui_view_block_ui_container_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_block_ui_container_get_compact_mode(egui_view_t *self);
void egui_view_block_ui_container_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_block_ui_container_get_read_only_mode(egui_view_t *self);
void egui_view_block_ui_container_apply_standard_style(egui_view_t *self);
void egui_view_block_ui_container_apply_accent_style(egui_view_t *self);
void egui_view_block_ui_container_apply_compact_style(egui_view_t *self);
void egui_view_block_ui_container_apply_read_only_style(egui_view_t *self);
void egui_view_block_ui_container_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_BLOCK_UI_CONTAINER_H_ */
