#ifndef _EGUI_VIEW_RELATIVE_PANEL_H_
#define _EGUI_VIEW_RELATIVE_PANEL_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_RELATIVE_PANEL_MAX_SNAPSHOTS 6
#define EGUI_VIEW_RELATIVE_PANEL_MAX_ITEMS     6
#define EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE     0xFF

#define EGUI_VIEW_RELATIVE_PANEL_TONE_ACCENT  0
#define EGUI_VIEW_RELATIVE_PANEL_TONE_SUCCESS 1
#define EGUI_VIEW_RELATIVE_PANEL_TONE_WARNING 2
#define EGUI_VIEW_RELATIVE_PANEL_TONE_NEUTRAL 3

typedef struct egui_view_relative_panel_item egui_view_relative_panel_item_t;
struct egui_view_relative_panel_item
{
    const char *badge;
    const char *title;
    const char *meta;
    const char *rule;
    egui_dim_t origin_x;
    egui_dim_t origin_y;
    egui_dim_t width;
    egui_dim_t height;
    uint8_t tone;
    uint8_t emphasized;
    uint8_t anchor_to;
};

typedef struct egui_view_relative_panel_snapshot egui_view_relative_panel_snapshot_t;
struct egui_view_relative_panel_snapshot
{
    const char *header;
    const char *title;
    const char *summary;
    const char *footer;
    const egui_view_relative_panel_item_t *items;
    uint8_t item_count;
    uint8_t selected_item;
    egui_dim_t layout_width;
    egui_dim_t layout_height;
};

typedef void (*egui_view_on_relative_panel_action_listener_t)(egui_view_t *self, uint8_t snapshot_index, uint8_t item_index);

typedef struct egui_view_relative_panel egui_view_relative_panel_t;
struct egui_view_relative_panel
{
    egui_view_t base;
    const egui_view_relative_panel_snapshot_t *snapshots;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_relative_panel_action_listener_t on_action;
    egui_color_t surface_color;
    egui_color_t section_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t preview_color;
    uint8_t snapshot_count;
    uint8_t current_snapshot;
    uint8_t current_item;
    uint8_t focus_on_rule;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_item;
    uint8_t pressed_rule;
};

void egui_view_relative_panel_init(egui_view_t *self);
void egui_view_relative_panel_set_snapshots(egui_view_t *self, const egui_view_relative_panel_snapshot_t *snapshots, uint8_t snapshot_count);
void egui_view_relative_panel_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index);
uint8_t egui_view_relative_panel_get_current_snapshot(egui_view_t *self);
void egui_view_relative_panel_set_current_item(egui_view_t *self, uint8_t item_index);
uint8_t egui_view_relative_panel_get_current_item(egui_view_t *self);
uint8_t egui_view_relative_panel_activate_current_item(egui_view_t *self);
void egui_view_relative_panel_set_on_action_listener(egui_view_t *self, egui_view_on_relative_panel_action_listener_t listener);
void egui_view_relative_panel_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_relative_panel_set_meta_font(egui_view_t *self, const egui_font_t *font);
void egui_view_relative_panel_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_relative_panel_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void egui_view_relative_panel_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                                          egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                          egui_color_t preview_color);
uint8_t egui_view_relative_panel_get_item_region(egui_view_t *self, uint8_t item_index, egui_region_t *region);
uint8_t egui_view_relative_panel_handle_navigation_key(egui_view_t *self, uint8_t key_code);
void egui_view_relative_panel_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_RELATIVE_PANEL_H_ */
