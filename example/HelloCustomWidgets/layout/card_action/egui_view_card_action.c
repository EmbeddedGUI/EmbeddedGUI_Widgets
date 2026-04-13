#include "egui_view_card_action.h"

#define egui_view_card_control_snapshot egui_view_card_action_backing_snapshot
#define egui_view_card_control_snapshot_t egui_view_card_action_backing_snapshot_t
#define egui_view_on_card_control_action_listener_t egui_view_on_card_action_backing_listener_t
#define egui_view_card_control egui_view_card_action_backing
#define egui_view_card_control_t egui_view_card_action_backing_t
#define egui_view_card_control_t_api_table egui_view_card_action_backing_t_api_table
#define egui_view_card_control_init egui_view_card_action_backing_init
#define egui_view_card_control_set_snapshots egui_view_card_action_backing_set_snapshots
#define egui_view_card_control_set_current_snapshot egui_view_card_action_backing_set_current_snapshot
#define egui_view_card_control_get_current_snapshot egui_view_card_action_backing_get_current_snapshot
#define egui_view_card_control_set_current_part egui_view_card_action_backing_set_current_part
#define egui_view_card_control_get_current_part egui_view_card_action_backing_get_current_part
#define egui_view_card_control_activate_current_part egui_view_card_action_backing_activate_current_part
#define egui_view_card_control_set_on_action_listener egui_view_card_action_backing_set_on_action_listener
#define egui_view_card_control_set_font egui_view_card_action_backing_set_font
#define egui_view_card_control_set_meta_font egui_view_card_action_backing_set_meta_font
#define egui_view_card_control_set_compact_mode egui_view_card_action_backing_set_compact_mode
#define egui_view_card_control_set_read_only_mode egui_view_card_action_backing_set_read_only_mode
#define egui_view_card_control_set_palette egui_view_card_action_backing_set_palette
#define egui_view_card_control_get_part_region egui_view_card_action_backing_get_part_region
#define egui_view_card_control_override_static_preview_api egui_view_card_action_backing_override_static_preview_api

#include "../card_control/egui_view_card_control.c"

#undef egui_view_card_control_snapshot
#undef egui_view_card_control_snapshot_t
#undef egui_view_on_card_control_action_listener_t
#undef egui_view_card_control
#undef egui_view_card_control_t
#undef egui_view_card_control_t_api_table
#undef egui_view_card_control_init
#undef egui_view_card_control_set_snapshots
#undef egui_view_card_control_set_current_snapshot
#undef egui_view_card_control_get_current_snapshot
#undef egui_view_card_control_set_current_part
#undef egui_view_card_control_get_current_part
#undef egui_view_card_control_activate_current_part
#undef egui_view_card_control_set_on_action_listener
#undef egui_view_card_control_set_font
#undef egui_view_card_control_set_meta_font
#undef egui_view_card_control_set_compact_mode
#undef egui_view_card_control_set_read_only_mode
#undef egui_view_card_control_set_palette
#undef egui_view_card_control_get_part_region
#undef egui_view_card_control_override_static_preview_api

#define EGUI_VIEW_CARD_ACTION_BACKING_CONTROL_NONE    0
#define EGUI_VIEW_CARD_ACTION_BACKING_CONTROL_CHEVRON 4

static uint8_t egui_view_card_action_clamp_snapshot_count(uint8_t count)
{
    return count > EGUI_VIEW_CARD_ACTION_MAX_SNAPSHOTS ? EGUI_VIEW_CARD_ACTION_MAX_SNAPSHOTS : count;
}

static void egui_view_card_action_translate_snapshots(egui_view_card_action_t *local, const egui_view_card_action_snapshot_t *snapshots,
                                                      uint8_t snapshot_count)
{
    uint8_t i;
    uint8_t resolved_count = egui_view_card_action_clamp_snapshot_count(snapshot_count);

    local->source_snapshots = snapshots;
    for (i = 0; i < resolved_count; ++i)
    {
        local->resolved_snapshots[i].header = snapshots[i].header;
        local->resolved_snapshots[i].icon_text = snapshots[i].icon_text;
        local->resolved_snapshots[i].title = snapshots[i].title;
        local->resolved_snapshots[i].body = snapshots[i].body;
        local->resolved_snapshots[i].control_text = NULL;
        local->resolved_snapshots[i].meta = snapshots[i].meta;
        local->resolved_snapshots[i].tone = snapshots[i].tone;
        local->resolved_snapshots[i].emphasized = snapshots[i].emphasized;
        local->resolved_snapshots[i].control_kind =
                snapshots[i].chevron_visible ? EGUI_VIEW_CARD_ACTION_BACKING_CONTROL_CHEVRON : EGUI_VIEW_CARD_ACTION_BACKING_CONTROL_NONE;
    }
}

static void egui_view_card_action_on_action_bridge(egui_view_t *self, uint8_t snapshot_index, uint8_t part)
{
    EGUI_LOCAL_INIT(egui_view_card_action_t);

    if (local->source_on_action != NULL)
    {
        local->source_on_action(self, snapshot_index, part);
    }
}

void egui_view_card_action_init(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_card_action_t);

    egui_view_card_action_backing_init(self);
    local->source_snapshots = NULL;
    local->source_on_action = NULL;
    egui_view_set_view_name(self, "egui_view_card_action");
}

void egui_view_card_action_set_snapshots(egui_view_t *self, const egui_view_card_action_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_card_action_t);
    uint8_t resolved_count = snapshots == NULL ? 0 : egui_view_card_action_clamp_snapshot_count(snapshot_count);

    if (snapshots != NULL && resolved_count > 0)
    {
        egui_view_card_action_translate_snapshots(local, snapshots, resolved_count);
        egui_view_card_action_backing_set_snapshots(self, (const egui_view_card_action_backing_snapshot_t *)local->resolved_snapshots, resolved_count);
        return;
    }

    local->source_snapshots = NULL;
    egui_view_card_action_backing_set_snapshots(self, NULL, 0);
}

void egui_view_card_action_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    egui_view_card_action_backing_set_current_snapshot(self, snapshot_index);
}

uint8_t egui_view_card_action_get_current_snapshot(egui_view_t *self)
{
    return egui_view_card_action_backing_get_current_snapshot(self);
}

void egui_view_card_action_set_current_part(egui_view_t *self, uint8_t part)
{
    egui_view_card_action_backing_set_current_part(self, part);
}

uint8_t egui_view_card_action_get_current_part(egui_view_t *self)
{
    return egui_view_card_action_backing_get_current_part(self);
}

uint8_t egui_view_card_action_activate_current_part(egui_view_t *self)
{
    return egui_view_card_action_backing_activate_current_part(self);
}

void egui_view_card_action_set_on_action_listener(egui_view_t *self, egui_view_on_card_action_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_card_action_t);

    local->source_on_action = listener;
    egui_view_card_action_backing_set_on_action_listener(self, listener == NULL ? NULL : egui_view_card_action_on_action_bridge);
}

void egui_view_card_action_set_font(egui_view_t *self, const egui_font_t *font)
{
    egui_view_card_action_backing_set_font(self, font);
}

void egui_view_card_action_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    egui_view_card_action_backing_set_meta_font(self, font);
}

void egui_view_card_action_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    egui_view_card_action_backing_set_compact_mode(self, compact_mode);
}

void egui_view_card_action_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    egui_view_card_action_backing_set_read_only_mode(self, read_only_mode);
}

void egui_view_card_action_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                                       egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                       egui_color_t success_color, egui_color_t warning_color, egui_color_t neutral_color)
{
    egui_view_card_action_backing_set_palette(self, surface_color, section_color, border_color, text_color, muted_text_color, accent_color,
                                              success_color, warning_color, neutral_color);
}

uint8_t egui_view_card_action_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region)
{
    return egui_view_card_action_backing_get_part_region(self, part, region);
}

void egui_view_card_action_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_card_action_backing_override_static_preview_api(self, api);
}
