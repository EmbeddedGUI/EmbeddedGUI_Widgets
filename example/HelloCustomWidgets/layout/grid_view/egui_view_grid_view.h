#ifndef _HELLO_CUSTOM_WIDGETS_GRID_VIEW_REFERENCE_H_
#define _HELLO_CUSTOM_WIDGETS_GRID_VIEW_REFERENCE_H_

#include "../items_repeater/egui_view_items_repeater.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HCW_GRID_VIEW_MAX_SNAPSHOTS EGUI_VIEW_ITEMS_REPEATER_MAX_SNAPSHOTS
#define HCW_GRID_VIEW_MAX_ITEMS     EGUI_VIEW_ITEMS_REPEATER_MAX_ITEMS
#define HCW_GRID_VIEW_ITEM_NONE     EGUI_VIEW_ITEMS_REPEATER_ITEM_NONE

#define HCW_GRID_VIEW_TONE_ACCENT  EGUI_VIEW_ITEMS_REPEATER_TONE_ACCENT
#define HCW_GRID_VIEW_TONE_SUCCESS EGUI_VIEW_ITEMS_REPEATER_TONE_SUCCESS
#define HCW_GRID_VIEW_TONE_WARNING EGUI_VIEW_ITEMS_REPEATER_TONE_WARNING
#define HCW_GRID_VIEW_TONE_NEUTRAL EGUI_VIEW_ITEMS_REPEATER_TONE_NEUTRAL

#define HCW_GRID_VIEW_LAYOUT_STACK EGUI_VIEW_ITEMS_REPEATER_LAYOUT_STACK
#define HCW_GRID_VIEW_LAYOUT_WRAP  EGUI_VIEW_ITEMS_REPEATER_LAYOUT_WRAP
#define HCW_GRID_VIEW_LAYOUT_STRIP EGUI_VIEW_ITEMS_REPEATER_LAYOUT_STRIP

#define HCW_GRID_VIEW_WIDTH_COMPACT   EGUI_VIEW_ITEMS_REPEATER_WIDTH_COMPACT
#define HCW_GRID_VIEW_WIDTH_BALANCED  EGUI_VIEW_ITEMS_REPEATER_WIDTH_BALANCED
#define HCW_GRID_VIEW_WIDTH_PROMINENT EGUI_VIEW_ITEMS_REPEATER_WIDTH_PROMINENT

typedef egui_view_items_repeater_item_t hcw_grid_view_item_t;
typedef egui_view_items_repeater_snapshot_t hcw_grid_view_snapshot_t;
typedef egui_view_on_items_repeater_action_listener_t hcw_on_grid_view_action_listener_t;
typedef egui_view_items_repeater_t hcw_grid_view_t;

void hcw_grid_view_init(egui_view_t *self);
void hcw_grid_view_set_snapshots(egui_view_t *self, const hcw_grid_view_snapshot_t *snapshots, uint8_t snapshot_count);
void hcw_grid_view_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index);
uint8_t hcw_grid_view_get_current_snapshot(egui_view_t *self);
void hcw_grid_view_set_current_item(egui_view_t *self, uint8_t item_index);
uint8_t hcw_grid_view_get_current_item(egui_view_t *self);
void hcw_grid_view_set_layout_mode(egui_view_t *self, uint8_t layout_mode);
uint8_t hcw_grid_view_get_layout_mode(egui_view_t *self);
uint8_t hcw_grid_view_activate_current_item(egui_view_t *self);
void hcw_grid_view_set_on_action_listener(egui_view_t *self, hcw_on_grid_view_action_listener_t listener);
void hcw_grid_view_set_font(egui_view_t *self, const egui_font_t *font);
void hcw_grid_view_set_meta_font(egui_view_t *self, const egui_font_t *font);
void hcw_grid_view_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void hcw_grid_view_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void hcw_grid_view_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                               egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                               egui_color_t success_color, egui_color_t warning_color, egui_color_t neutral_color);
uint8_t hcw_grid_view_get_item_region(egui_view_t *self, uint8_t item_index, egui_region_t *region);
uint8_t hcw_grid_view_handle_navigation_key(egui_view_t *self, uint8_t key_code);
void hcw_grid_view_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_GRID_VIEW_REFERENCE_H_ */
