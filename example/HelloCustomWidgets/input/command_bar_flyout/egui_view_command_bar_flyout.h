#ifndef _EGUI_VIEW_COMMAND_BAR_FLYOUT_H_
#define _EGUI_VIEW_COMMAND_BAR_FLYOUT_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_SNAPSHOTS       6
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_PRIMARY_ITEMS   4
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_SECONDARY_ITEMS 6

#define EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_ACCENT  0
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_SUCCESS 1
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_WARNING 2
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_DANGER  3
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL 4

#define EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER        0
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_PRIMARY_BASE   1
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_SECONDARY_BASE 16
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE           0xFF

#define EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(index)   ((uint8_t)(EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_PRIMARY_BASE + (index)))
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART(index) ((uint8_t)(EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_SECONDARY_BASE + (index)))

typedef struct egui_view_command_bar_flyout_primary_item egui_view_command_bar_flyout_primary_item_t;
struct egui_view_command_bar_flyout_primary_item
{
    const char *glyph;
    const char *label;
    uint8_t tone;
    uint8_t emphasized;
    uint8_t enabled;
};

typedef struct egui_view_command_bar_flyout_secondary_item egui_view_command_bar_flyout_secondary_item_t;
struct egui_view_command_bar_flyout_secondary_item
{
    const char *icon_text;
    const char *title;
    const char *meta;
    uint8_t tone;
    uint8_t emphasized;
    uint8_t enabled;
    uint8_t separator_before;
};

typedef struct egui_view_command_bar_flyout_snapshot egui_view_command_bar_flyout_snapshot_t;
struct egui_view_command_bar_flyout_snapshot
{
    const char *eyebrow;
    const char *trigger_label;
    const char *panel_title;
    const char *panel_helper;
    const egui_view_command_bar_flyout_primary_item_t *primary_items;
    uint8_t primary_count;
    const egui_view_command_bar_flyout_secondary_item_t *secondary_items;
    uint8_t secondary_count;
    uint8_t open;
    uint8_t focus_part;
};

typedef void (*egui_view_on_command_bar_flyout_action_listener_t)(egui_view_t *self, uint8_t snapshot_index, uint8_t part);

typedef struct egui_view_command_bar_flyout egui_view_command_bar_flyout_t;
struct egui_view_command_bar_flyout
{
    egui_view_t base;
    const egui_view_command_bar_flyout_snapshot_t *snapshots;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_command_bar_flyout_action_listener_t on_action;
    egui_color_t surface_color;
    egui_color_t section_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t success_color;
    egui_color_t warning_color;
    egui_color_t danger_color;
    egui_color_t neutral_color;
    egui_color_t shadow_color;
    uint8_t snapshot_count;
    uint8_t current_snapshot;
    uint8_t current_part;
    uint8_t open_state;
    uint8_t compact_mode;
    uint8_t disabled_mode;
    uint8_t pressed_part;
};

void egui_view_command_bar_flyout_init(egui_view_t *self);
void egui_view_command_bar_flyout_set_snapshots(egui_view_t *self, const egui_view_command_bar_flyout_snapshot_t *snapshots, uint8_t snapshot_count);
void egui_view_command_bar_flyout_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index);
uint8_t egui_view_command_bar_flyout_get_current_snapshot(egui_view_t *self);
void egui_view_command_bar_flyout_set_open(egui_view_t *self, uint8_t open);
uint8_t egui_view_command_bar_flyout_get_open(egui_view_t *self);
void egui_view_command_bar_flyout_set_current_part(egui_view_t *self, uint8_t part);
uint8_t egui_view_command_bar_flyout_get_current_part(egui_view_t *self);
uint8_t egui_view_command_bar_flyout_activate_current_part(egui_view_t *self);
void egui_view_command_bar_flyout_set_on_action_listener(egui_view_t *self, egui_view_on_command_bar_flyout_action_listener_t listener);
void egui_view_command_bar_flyout_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_command_bar_flyout_set_meta_font(egui_view_t *self, const egui_font_t *font);
void egui_view_command_bar_flyout_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_command_bar_flyout_set_disabled_mode(egui_view_t *self, uint8_t disabled_mode);
void egui_view_command_bar_flyout_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                                              egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                              egui_color_t success_color, egui_color_t warning_color, egui_color_t danger_color,
                                              egui_color_t neutral_color, egui_color_t shadow_color);
uint8_t egui_view_command_bar_flyout_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region);
void egui_view_command_bar_flyout_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_COMMAND_BAR_FLYOUT_H_ */
