#ifndef _EGUI_VIEW_CARD_ACTION_H_
#define _EGUI_VIEW_CARD_ACTION_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_CARD_ACTION_MAX_SNAPSHOTS 6

#define EGUI_VIEW_CARD_ACTION_PART_CARD 0
#define EGUI_VIEW_CARD_ACTION_PART_NONE 0xFF

#define EGUI_VIEW_CARD_ACTION_TONE_ACCENT  0
#define EGUI_VIEW_CARD_ACTION_TONE_SUCCESS 1
#define EGUI_VIEW_CARD_ACTION_TONE_WARNING 2
#define EGUI_VIEW_CARD_ACTION_TONE_NEUTRAL 3

typedef struct egui_view_card_action_snapshot egui_view_card_action_snapshot_t;
struct egui_view_card_action_snapshot
{
    const char *header;
    const char *icon_text;
    const char *title;
    const char *body;
    const char *meta;
    uint8_t tone;
    uint8_t emphasized;
    uint8_t chevron_visible;
};

typedef struct egui_view_card_action_resolved_snapshot egui_view_card_action_resolved_snapshot_t;
struct egui_view_card_action_resolved_snapshot
{
    const char *header;
    const char *icon_text;
    const char *title;
    const char *body;
    const char *control_text;
    const char *meta;
    uint8_t tone;
    uint8_t emphasized;
    uint8_t control_kind;
};

typedef void (*egui_view_on_card_action_listener_t)(egui_view_t *self, uint8_t snapshot_index, uint8_t part);

typedef struct egui_view_card_action egui_view_card_action_t;
struct egui_view_card_action
{
    egui_view_t base;
    const egui_view_card_action_resolved_snapshot_t *snapshots;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_card_action_listener_t on_action;
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
    uint8_t current_part;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_part;
    const egui_view_card_action_snapshot_t *source_snapshots;
    egui_view_on_card_action_listener_t source_on_action;
    egui_view_card_action_resolved_snapshot_t resolved_snapshots[EGUI_VIEW_CARD_ACTION_MAX_SNAPSHOTS];
};

void egui_view_card_action_init(egui_view_t *self);
void egui_view_card_action_set_snapshots(egui_view_t *self, const egui_view_card_action_snapshot_t *snapshots, uint8_t snapshot_count);
void egui_view_card_action_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index);
uint8_t egui_view_card_action_get_current_snapshot(egui_view_t *self);
void egui_view_card_action_set_current_part(egui_view_t *self, uint8_t part);
uint8_t egui_view_card_action_get_current_part(egui_view_t *self);
uint8_t egui_view_card_action_activate_current_part(egui_view_t *self);
void egui_view_card_action_set_on_action_listener(egui_view_t *self, egui_view_on_card_action_listener_t listener);
void egui_view_card_action_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_card_action_set_meta_font(egui_view_t *self, const egui_font_t *font);
void egui_view_card_action_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_card_action_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void egui_view_card_action_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                                       egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                       egui_color_t success_color, egui_color_t warning_color, egui_color_t neutral_color);
uint8_t egui_view_card_action_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region);
void egui_view_card_action_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_CARD_ACTION_H_ */
