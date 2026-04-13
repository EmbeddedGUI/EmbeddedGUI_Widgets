#ifndef _EGUI_VIEW_ITEMS_REPEATER_H_
#define _EGUI_VIEW_ITEMS_REPEATER_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_ITEMS_REPEATER_MAX_SNAPSHOTS 6
#define EGUI_VIEW_ITEMS_REPEATER_MAX_ITEMS     8
#define EGUI_VIEW_ITEMS_REPEATER_ITEM_NONE     0xFF

#define EGUI_VIEW_ITEMS_REPEATER_TONE_ACCENT  0
#define EGUI_VIEW_ITEMS_REPEATER_TONE_SUCCESS 1
#define EGUI_VIEW_ITEMS_REPEATER_TONE_WARNING 2
#define EGUI_VIEW_ITEMS_REPEATER_TONE_NEUTRAL 3

#define EGUI_VIEW_ITEMS_REPEATER_LAYOUT_STACK 0
#define EGUI_VIEW_ITEMS_REPEATER_LAYOUT_WRAP  1
#define EGUI_VIEW_ITEMS_REPEATER_LAYOUT_STRIP 2

#define EGUI_VIEW_ITEMS_REPEATER_WIDTH_COMPACT   0
#define EGUI_VIEW_ITEMS_REPEATER_WIDTH_BALANCED  1
#define EGUI_VIEW_ITEMS_REPEATER_WIDTH_PROMINENT 2

typedef struct egui_view_items_repeater_item egui_view_items_repeater_item_t;
struct egui_view_items_repeater_item
{
    const char *badge;
    const char *title;
    const char *meta;
    uint8_t tone;
    uint8_t emphasized;
    uint8_t width_class;
};

typedef struct egui_view_items_repeater_snapshot egui_view_items_repeater_snapshot_t;
struct egui_view_items_repeater_snapshot
{
    const char *header;
    const char *title;
    const char *summary;
    const char *footer;
    const egui_view_items_repeater_item_t *items;
    uint8_t item_count;
    uint8_t selected_item;
    uint8_t layout_mode;
};

typedef void (*egui_view_on_items_repeater_action_listener_t)(egui_view_t *self, uint8_t snapshot_index, uint8_t item_index);

typedef struct egui_view_items_repeater egui_view_items_repeater_t;
struct egui_view_items_repeater
{
    egui_view_t base;
    const egui_view_items_repeater_snapshot_t *snapshots;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_items_repeater_action_listener_t on_action;
    egui_color_t surface_color;
    egui_color_t section_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t success_color;
    egui_color_t warning_color;
    egui_color_t neutral_color;
    uint8_t snapshot_count;
    uint8_t current_snapshot;
    uint8_t current_item;
    uint8_t current_layout_mode;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_item;
};

void egui_view_items_repeater_init(egui_view_t *self);
void egui_view_items_repeater_set_snapshots(egui_view_t *self, const egui_view_items_repeater_snapshot_t *snapshots, uint8_t snapshot_count);
void egui_view_items_repeater_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index);
uint8_t egui_view_items_repeater_get_current_snapshot(egui_view_t *self);
void egui_view_items_repeater_set_current_item(egui_view_t *self, uint8_t item_index);
uint8_t egui_view_items_repeater_get_current_item(egui_view_t *self);
void egui_view_items_repeater_set_layout_mode(egui_view_t *self, uint8_t layout_mode);
uint8_t egui_view_items_repeater_get_layout_mode(egui_view_t *self);
uint8_t egui_view_items_repeater_activate_current_item(egui_view_t *self);
void egui_view_items_repeater_set_on_action_listener(egui_view_t *self, egui_view_on_items_repeater_action_listener_t listener);
void egui_view_items_repeater_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_items_repeater_set_meta_font(egui_view_t *self, const egui_font_t *font);
void egui_view_items_repeater_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_items_repeater_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void egui_view_items_repeater_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                                      egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                      egui_color_t success_color, egui_color_t warning_color, egui_color_t neutral_color);
uint8_t egui_view_items_repeater_get_item_region(egui_view_t *self, uint8_t item_index, egui_region_t *region);
void egui_view_items_repeater_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_ITEMS_REPEATER_H_ */

