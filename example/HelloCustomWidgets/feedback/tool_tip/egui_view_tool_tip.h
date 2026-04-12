#ifndef _EGUI_VIEW_TOOL_TIP_H_
#define _EGUI_VIEW_TOOL_TIP_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_TOOL_TIP_MAX_SNAPSHOTS 4

#define EGUI_VIEW_TOOL_TIP_PART_TARGET 0
#define EGUI_VIEW_TOOL_TIP_PART_BUBBLE 1
#define EGUI_VIEW_TOOL_TIP_PART_NONE   0xFF

#define EGUI_VIEW_TOOL_TIP_TONE_ACCENT  0
#define EGUI_VIEW_TOOL_TIP_TONE_WARNING 1
#define EGUI_VIEW_TOOL_TIP_TONE_NEUTRAL 2

#define EGUI_VIEW_TOOL_TIP_PLACEMENT_BOTTOM 0
#define EGUI_VIEW_TOOL_TIP_PLACEMENT_TOP    1

typedef struct egui_view_tool_tip_snapshot egui_view_tool_tip_snapshot_t;
struct egui_view_tool_tip_snapshot
{
    const char *target_label;
    const char *hint;
    const char *title;
    const char *body;
    uint8_t tone;
    uint8_t placement;
    int16_t target_offset_x;
};

typedef struct egui_view_tool_tip egui_view_tool_tip_t;
struct egui_view_tool_tip
{
    egui_view_t base;
    const egui_view_tool_tip_snapshot_t *snapshots;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_timer_t show_timer;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t warning_color;
    egui_color_t neutral_color;
    egui_color_t shadow_color;
    egui_color_t target_fill_color;
    egui_color_t target_border_color;
    uint16_t show_delay_ms;
    uint8_t snapshot_count;
    uint8_t current_snapshot;
    uint8_t current_part;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t open;
    uint8_t timer_started;
    uint8_t pending_show;
    uint8_t touch_active;
    uint8_t key_active;
    uint8_t toggle_on_release;
};

void egui_view_tool_tip_init(egui_view_t *self);
void egui_view_tool_tip_set_snapshots(egui_view_t *self, const egui_view_tool_tip_snapshot_t *snapshots, uint8_t snapshot_count);
void egui_view_tool_tip_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index);
uint8_t egui_view_tool_tip_get_current_snapshot(egui_view_t *self);
void egui_view_tool_tip_set_open(egui_view_t *self, uint8_t is_open);
uint8_t egui_view_tool_tip_get_open(egui_view_t *self);
void egui_view_tool_tip_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_tool_tip_set_meta_font(egui_view_t *self, const egui_font_t *font);
void egui_view_tool_tip_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_tool_tip_get_compact_mode(egui_view_t *self);
void egui_view_tool_tip_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_tool_tip_get_read_only_mode(egui_view_t *self);
void egui_view_tool_tip_set_show_delay(egui_view_t *self, uint16_t show_delay_ms);
uint16_t egui_view_tool_tip_get_show_delay(egui_view_t *self);
void egui_view_tool_tip_begin_show_delay(egui_view_t *self);
void egui_view_tool_tip_cancel_show_delay(egui_view_t *self);
void egui_view_tool_tip_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                    egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t warning_color, egui_color_t neutral_color,
                                    egui_color_t shadow_color, egui_color_t target_fill_color, egui_color_t target_border_color);
void egui_view_tool_tip_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);
uint8_t egui_view_tool_tip_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_TOOL_TIP_H_ */
