#ifndef _HELLO_CUSTOM_WIDGETS_INFO_LABEL_H_
#define _HELLO_CUSTOM_WIDGETS_INFO_LABEL_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HCW_INFO_LABEL_PART_NONE   0
#define HCW_INFO_LABEL_PART_ICON   1
#define HCW_INFO_LABEL_PART_BUBBLE 2

typedef void (*hcw_info_label_on_open_changed_listener_t)(egui_view_t *self, uint8_t is_open);

typedef struct hcw_info_label hcw_info_label_t;
struct hcw_info_label
{
    egui_view_t base;
    const char *label;
    const char *info_title;
    const char *info_body;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    const egui_font_t *icon_font;
    hcw_info_label_on_open_changed_listener_t on_open_changed;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t bubble_surface_color;
    egui_color_t shadow_color;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t open;
    uint8_t pressed_part;
};

void hcw_info_label_init(egui_view_t *self);
void hcw_info_label_apply_standard_style(egui_view_t *self);
void hcw_info_label_apply_compact_style(egui_view_t *self);
void hcw_info_label_apply_read_only_style(egui_view_t *self);
void hcw_info_label_set_text(egui_view_t *self, const char *label);
void hcw_info_label_set_info_title(egui_view_t *self, const char *title);
void hcw_info_label_set_info_body(egui_view_t *self, const char *body);
void hcw_info_label_set_font(egui_view_t *self, const egui_font_t *font);
void hcw_info_label_set_meta_font(egui_view_t *self, const egui_font_t *font);
void hcw_info_label_set_icon_font(egui_view_t *self, const egui_font_t *font);
void hcw_info_label_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t bubble_surface_color,
                                egui_color_t shadow_color);
void hcw_info_label_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void hcw_info_label_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void hcw_info_label_set_open(egui_view_t *self, uint8_t is_open);
uint8_t hcw_info_label_get_open(egui_view_t *self);
void hcw_info_label_set_on_open_changed_listener(egui_view_t *self, hcw_info_label_on_open_changed_listener_t listener);
uint8_t hcw_info_label_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region);
void hcw_info_label_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_INFO_LABEL_H_ */
