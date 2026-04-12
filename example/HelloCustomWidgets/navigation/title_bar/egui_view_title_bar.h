#ifndef _EGUI_VIEW_TITLE_BAR_H_
#define _EGUI_VIEW_TITLE_BAR_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_TITLE_BAR_MAX_SNAPSHOTS 4

#define EGUI_VIEW_TITLE_BAR_PART_BACK             0
#define EGUI_VIEW_TITLE_BAR_PART_PANE_TOGGLE      1
#define EGUI_VIEW_TITLE_BAR_PART_PRIMARY_ACTION   2
#define EGUI_VIEW_TITLE_BAR_PART_SECONDARY_ACTION 3
#define EGUI_VIEW_TITLE_BAR_PART_NONE             0xFF

typedef struct egui_view_title_bar_snapshot egui_view_title_bar_snapshot_t;
struct egui_view_title_bar_snapshot
{
    const char *leading_icon;
    const char *leading_header;
    const char *title;
    const char *subtitle;
    const char *trailing_header;
    const char *primary_action;
    const char *secondary_action;
    uint8_t show_back_button;
    uint8_t show_pane_toggle;
    uint8_t emphasize_primary_action;
};

typedef void (*egui_view_on_title_bar_action_listener_t)(egui_view_t *self, uint8_t part, uint8_t snapshot_index);

typedef struct egui_view_title_bar egui_view_title_bar_t;
struct egui_view_title_bar
{
    egui_view_t base;
    const egui_view_title_bar_snapshot_t *snapshots;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    const egui_font_t *icon_font;
    egui_view_on_title_bar_action_listener_t on_action;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t subtle_fill_color;
    egui_color_t subtle_border_color;
    egui_color_t shadow_color;
    uint8_t snapshot_count;
    uint8_t current_snapshot;
    uint8_t current_part;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_part;
};

void egui_view_title_bar_init(egui_view_t *self);
void egui_view_title_bar_set_snapshots(egui_view_t *self, const egui_view_title_bar_snapshot_t *snapshots, uint8_t snapshot_count);
void egui_view_title_bar_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index);
uint8_t egui_view_title_bar_get_current_snapshot(egui_view_t *self);
void egui_view_title_bar_set_current_part(egui_view_t *self, uint8_t part);
uint8_t egui_view_title_bar_get_current_part(egui_view_t *self);
void egui_view_title_bar_activate_current_part(egui_view_t *self);
void egui_view_title_bar_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_title_bar_set_meta_font(egui_view_t *self, const egui_font_t *font);
void egui_view_title_bar_set_icon_font(egui_view_t *self, const egui_font_t *font);
void egui_view_title_bar_set_on_action_listener(egui_view_t *self, egui_view_on_title_bar_action_listener_t listener);
void egui_view_title_bar_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_title_bar_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void egui_view_title_bar_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                     egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t subtle_fill_color,
                                     egui_color_t subtle_border_color, egui_color_t shadow_color);
void egui_view_title_bar_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);
uint8_t egui_view_title_bar_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_TITLE_BAR_H_ */
