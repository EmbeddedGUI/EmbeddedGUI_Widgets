#include "egui_view_command_bar_flyout.h"

#define EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_RADIUS        10
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_TRIGGER_H     28
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_SECTION_GAP   5
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_PANEL_PAD     7
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_HEADER_H      12
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_RAIL_H        24
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_HELPER_H      10
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_ROW_H         18
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_ROW_GAP       1
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_ITEM_GAP      4
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_ITEM_MIN_W    30
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_ITEM_MAX_W    52
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_ITEM_BASE_W   12
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_ITEM_CHAR_W   5
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_GLYPH_W       12
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_GLYPH_H       12
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_EYEBROW_BASE  16
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_EYEBROW_CHARW 4

#define EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_RADIUS        8
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_TRIGGER_H     20
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_SECTION_GAP   4
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_PANEL_PAD     5
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_HEADER_H      10
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_RAIL_H        18
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_ROW_H         15
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_ROW_GAP       1
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_ITEM_GAP      3
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_ITEM_MIN_W    18
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_ITEM_MAX_W    22
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_ITEM_BASE_W   18
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_EYEBROW_BASE  12
#define EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_EYEBROW_CHARW 3

typedef struct egui_view_command_bar_flyout_metrics egui_view_command_bar_flyout_metrics_t;
struct egui_view_command_bar_flyout_metrics
{
    egui_region_t content_region;
    egui_region_t trigger_region;
    egui_region_t panel_region;
    egui_region_t header_region;
    egui_region_t rail_region;
    egui_region_t helper_region;
    egui_region_t primary_regions[EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_PRIMARY_ITEMS];
    egui_region_t secondary_regions[EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_SECONDARY_ITEMS];
    uint8_t show_panel;
    uint8_t primary_count;
    uint8_t secondary_count;
};

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_command_bar_flyout_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event);
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_command_bar_flyout_on_static_key_event(egui_view_t *self, egui_key_event_t *event);
#endif

static uint8_t egui_view_command_bar_flyout_clamp_snapshot_count(uint8_t count)
{
    if (count > EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_SNAPSHOTS)
    {
        return EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_SNAPSHOTS;
    }

    return count;
}

static uint8_t egui_view_command_bar_flyout_clamp_primary_count(uint8_t count)
{
    if (count > EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_PRIMARY_ITEMS)
    {
        return EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_PRIMARY_ITEMS;
    }

    return count;
}

static uint8_t egui_view_command_bar_flyout_clamp_secondary_count(uint8_t count)
{
    if (count > EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_SECONDARY_ITEMS)
    {
        return EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_SECONDARY_ITEMS;
    }

    return count;
}

static uint8_t egui_view_command_bar_flyout_text_len(const char *text)
{
    uint8_t length = 0;

    if (text == NULL)
    {
        return 0;
    }

    while (text[length] != '\0')
    {
        length++;
    }

    return length;
}

static uint8_t egui_view_command_bar_flyout_part_is_primary(uint8_t part)
{
    return part >= EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_PRIMARY_BASE &&
           part < EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_PRIMARY_BASE + EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_PRIMARY_ITEMS;
}

static uint8_t egui_view_command_bar_flyout_part_is_secondary(uint8_t part)
{
    return part >= EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_SECONDARY_BASE &&
           part < EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_SECONDARY_BASE + EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_SECONDARY_ITEMS;
}

static uint8_t egui_view_command_bar_flyout_primary_index(uint8_t part)
{
    return (uint8_t)(part - EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_PRIMARY_BASE);
}

static uint8_t egui_view_command_bar_flyout_secondary_index(uint8_t part)
{
    return (uint8_t)(part - EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_SECONDARY_BASE);
}

static egui_color_t egui_view_command_bar_flyout_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 68);
}

static egui_color_t egui_view_command_bar_flyout_tone_color(egui_view_command_bar_flyout_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_SUCCESS:
        return local->success_color;
    case EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_WARNING:
        return local->warning_color;
    case EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_DANGER:
        return local->danger_color;
    case EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL:
        return local->neutral_color;
    default:
        return local->accent_color;
    }
}

static const egui_view_command_bar_flyout_snapshot_t *egui_view_command_bar_flyout_get_snapshot(egui_view_command_bar_flyout_t *local)
{
    if (local->snapshots == NULL || local->snapshot_count == 0 || local->current_snapshot >= local->snapshot_count)
    {
        return NULL;
    }

    return &local->snapshots[local->current_snapshot];
}

static uint8_t egui_view_command_bar_flyout_clear_pressed_state(egui_view_t *self, egui_view_command_bar_flyout_t *local)
{
    uint8_t was_pressed = self->is_pressed || local->pressed_part != EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE;

    if (!was_pressed)
    {
        return 0;
    }

    local->pressed_part = EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE;
    egui_view_set_pressed(self, 0);
    return 1;
}

static uint8_t egui_view_command_bar_flyout_part_exists(const egui_view_command_bar_flyout_snapshot_t *snapshot, uint8_t open_state, uint8_t part)
{
    uint8_t primary_count;
    uint8_t secondary_count;

    if (snapshot == NULL)
    {
        return 0;
    }

    if (part == EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER)
    {
        return 1;
    }

    if (!open_state)
    {
        return 0;
    }

    primary_count = egui_view_command_bar_flyout_clamp_primary_count(snapshot->primary_count);
    secondary_count = egui_view_command_bar_flyout_clamp_secondary_count(snapshot->secondary_count);

    if (egui_view_command_bar_flyout_part_is_primary(part))
    {
        return egui_view_command_bar_flyout_primary_index(part) < primary_count;
    }

    if (egui_view_command_bar_flyout_part_is_secondary(part))
    {
        return egui_view_command_bar_flyout_secondary_index(part) < secondary_count;
    }

    return 0;
}

static uint8_t egui_view_command_bar_flyout_part_data_enabled(const egui_view_command_bar_flyout_snapshot_t *snapshot, uint8_t open_state, uint8_t part)
{
    if (!egui_view_command_bar_flyout_part_exists(snapshot, open_state, part))
    {
        return 0;
    }

    if (part == EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER)
    {
        return 1;
    }

    if (egui_view_command_bar_flyout_part_is_primary(part))
    {
        return snapshot->primary_items[egui_view_command_bar_flyout_primary_index(part)].enabled ? 1 : 0;
    }

    return snapshot->secondary_items[egui_view_command_bar_flyout_secondary_index(part)].enabled ? 1 : 0;
}

static uint8_t egui_view_command_bar_flyout_part_is_interactive(egui_view_command_bar_flyout_t *local, egui_view_t *self,
                                                                const egui_view_command_bar_flyout_snapshot_t *snapshot, uint8_t part)
{
    if (snapshot == NULL || local->compact_mode || local->disabled_mode || !egui_view_get_enable(self))
    {
        return 0;
    }

    return egui_view_command_bar_flyout_part_data_enabled(snapshot, local->open_state, part);
}

static uint8_t egui_view_command_bar_flyout_find_primary_part(const egui_view_command_bar_flyout_snapshot_t *snapshot, uint8_t open_state, uint8_t reverse,
                                                              uint8_t enabled_only)
{
    uint8_t count = snapshot == NULL ? 0 : egui_view_command_bar_flyout_clamp_primary_count(snapshot->primary_count);
    int16_t index = reverse ? (int16_t)count - 1 : 0;
    int16_t step = reverse ? -1 : 1;

    while (index >= 0 && index < count)
    {
        uint8_t part = EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART((uint8_t)index);

        if (!enabled_only || egui_view_command_bar_flyout_part_data_enabled(snapshot, open_state, part))
        {
            return part;
        }
        index += step;
    }

    return EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE;
}

static uint8_t egui_view_command_bar_flyout_find_secondary_part(const egui_view_command_bar_flyout_snapshot_t *snapshot, uint8_t open_state, uint8_t reverse,
                                                                uint8_t enabled_only)
{
    uint8_t count = snapshot == NULL ? 0 : egui_view_command_bar_flyout_clamp_secondary_count(snapshot->secondary_count);
    int16_t index = reverse ? (int16_t)count - 1 : 0;
    int16_t step = reverse ? -1 : 1;

    while (index >= 0 && index < count)
    {
        uint8_t part = EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART((uint8_t)index);

        if (!enabled_only || egui_view_command_bar_flyout_part_data_enabled(snapshot, open_state, part))
        {
            return part;
        }
        index += step;
    }

    return EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE;
}

static uint8_t egui_view_command_bar_flyout_resolve_default_part(egui_view_command_bar_flyout_t *local,
                                                                 const egui_view_command_bar_flyout_snapshot_t *snapshot)
{
    if (snapshot == NULL)
    {
        return EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE;
    }

    if (!local->open_state)
    {
        return EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER;
    }

    if (snapshot->focus_part != EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER &&
        egui_view_command_bar_flyout_part_exists(snapshot, local->open_state, snapshot->focus_part))
    {
        if (egui_view_command_bar_flyout_part_data_enabled(snapshot, local->open_state, snapshot->focus_part))
        {
            return snapshot->focus_part;
        }
    }

    if (egui_view_command_bar_flyout_find_primary_part(snapshot, local->open_state, 0, 1) != EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE)
    {
        return egui_view_command_bar_flyout_find_primary_part(snapshot, local->open_state, 0, 1);
    }

    if (egui_view_command_bar_flyout_find_secondary_part(snapshot, local->open_state, 0, 1) != EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE)
    {
        return egui_view_command_bar_flyout_find_secondary_part(snapshot, local->open_state, 0, 1);
    }

    if (egui_view_command_bar_flyout_find_primary_part(snapshot, local->open_state, 0, 0) != EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE)
    {
        return egui_view_command_bar_flyout_find_primary_part(snapshot, local->open_state, 0, 0);
    }

    if (egui_view_command_bar_flyout_find_secondary_part(snapshot, local->open_state, 0, 0) != EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE)
    {
        return egui_view_command_bar_flyout_find_secondary_part(snapshot, local->open_state, 0, 0);
    }

    return EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER;
}

static void egui_view_command_bar_flyout_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                                   egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (text == NULL || text[0] == '\0' || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, self->alpha);
}

static egui_dim_t egui_view_command_bar_flyout_measure_eyebrow_width(uint8_t compact_mode, const char *text, egui_dim_t max_w)
{
    egui_dim_t width;

    if (text == NULL || text[0] == '\0')
    {
        return 0;
    }

    width = (compact_mode ? EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_EYEBROW_BASE : EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_EYEBROW_BASE) +
            egui_view_command_bar_flyout_text_len(text) *
                    (compact_mode ? EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_EYEBROW_CHARW : EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_EYEBROW_CHARW);
    if (width > max_w)
    {
        width = max_w;
    }

    return width;
}

static egui_dim_t egui_view_command_bar_flyout_measure_primary_width(uint8_t compact_mode,
                                                                     const egui_view_command_bar_flyout_primary_item_t *item)
{
    egui_dim_t width;

    if (item == NULL)
    {
        return 0;
    }

    if (compact_mode)
    {
        width = EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_ITEM_BASE_W + (item->emphasized ? 2 : 0);
        if (width < EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_ITEM_MIN_W)
        {
            width = EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_ITEM_MIN_W;
        }
        if (width > EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_ITEM_MAX_W)
        {
            width = EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_ITEM_MAX_W;
        }
        return width;
    }

    width = EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_ITEM_BASE_W + EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_GLYPH_W + 4 +
            egui_view_command_bar_flyout_text_len(item->label) * EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_ITEM_CHAR_W + (item->emphasized ? 4 : 0);
    if (width < EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_ITEM_MIN_W)
    {
        width = EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_ITEM_MIN_W;
    }
    if (width > EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_ITEM_MAX_W)
    {
        width = EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_ITEM_MAX_W;
    }

    return width;
}

static void egui_view_command_bar_flyout_reset_metrics(egui_view_command_bar_flyout_metrics_t *metrics)
{
    uint8_t i;

    metrics->show_panel = 0;
    metrics->primary_count = 0;
    metrics->secondary_count = 0;
    for (i = 0; i < EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_PRIMARY_ITEMS; ++i)
    {
        metrics->primary_regions[i].size.width = 0;
        metrics->primary_regions[i].size.height = 0;
    }
    for (i = 0; i < EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_SECONDARY_ITEMS; ++i)
    {
        metrics->secondary_regions[i].size.width = 0;
        metrics->secondary_regions[i].size.height = 0;
    }
}

static void egui_view_command_bar_flyout_get_metrics(egui_view_command_bar_flyout_t *local, egui_view_t *self,
                                                     const egui_view_command_bar_flyout_snapshot_t *snapshot,
                                                     egui_view_command_bar_flyout_metrics_t *metrics)
{
    egui_region_t region;
    egui_dim_t trigger_h = local->compact_mode ? EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_TRIGGER_H : EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_TRIGGER_H;
    egui_dim_t section_gap = local->compact_mode ? EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_SECTION_GAP : EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_SECTION_GAP;
    egui_dim_t panel_pad = local->compact_mode ? EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_PANEL_PAD : EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_PANEL_PAD;
    egui_dim_t header_h = local->compact_mode ? EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_HEADER_H : EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_HEADER_H;
    egui_dim_t rail_h = local->compact_mode ? EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_RAIL_H : EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_RAIL_H;
    egui_dim_t row_h = local->compact_mode ? EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_ROW_H : EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_ROW_H;
    egui_dim_t row_gap = local->compact_mode ? EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_ROW_GAP : EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_ROW_GAP;
    egui_dim_t item_gap = local->compact_mode ? EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_ITEM_GAP : EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_ITEM_GAP;
    egui_dim_t helper_h = (!local->compact_mode && snapshot != NULL && snapshot->panel_helper != NULL && snapshot->panel_helper[0] != '\0')
                                  ? EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_HELPER_H
                                  : 0;
    egui_dim_t cursor_x;
    egui_dim_t cursor_y;
    egui_dim_t limit_y;
    egui_dim_t available_w;
    egui_dim_t widths[EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_PRIMARY_ITEMS];
    egui_dim_t total_w = 0;
    uint8_t primary_count = snapshot == NULL ? 0 : egui_view_command_bar_flyout_clamp_primary_count(snapshot->primary_count);
    uint8_t secondary_count = snapshot == NULL ? 0 : egui_view_command_bar_flyout_clamp_secondary_count(snapshot->secondary_count);
    uint8_t i;

    egui_view_get_work_region(self, &region);
    egui_view_command_bar_flyout_reset_metrics(metrics);
    metrics->content_region = region;
    metrics->trigger_region = region;
    metrics->trigger_region.size.height = trigger_h;

    if (region.size.width <= 0 || region.size.height <= 0)
    {
        return;
    }

    if (metrics->trigger_region.size.height > region.size.height)
    {
        metrics->trigger_region.size.height = region.size.height;
    }

    if (snapshot == NULL || !local->open_state)
    {
        return;
    }

    metrics->panel_region.location.x = region.location.x;
    metrics->panel_region.location.y = metrics->trigger_region.location.y + metrics->trigger_region.size.height + section_gap;
    metrics->panel_region.size.width = region.size.width;
    metrics->panel_region.size.height = region.size.height - metrics->trigger_region.size.height - section_gap;
    if (metrics->panel_region.size.height <= 0)
    {
        metrics->panel_region.size.height = 0;
        return;
    }

    metrics->show_panel = 1;
    metrics->primary_count = primary_count;

    metrics->header_region.location.x = metrics->panel_region.location.x + panel_pad;
    metrics->header_region.location.y = metrics->panel_region.location.y + panel_pad;
    metrics->header_region.size.width = metrics->panel_region.size.width - panel_pad * 2;
    metrics->header_region.size.height = (snapshot->panel_title != NULL && snapshot->panel_title[0] != '\0') ||
                                                 (snapshot->eyebrow != NULL && snapshot->eyebrow[0] != '\0')
                                         ? header_h
                                         : 0;

    cursor_y = metrics->header_region.location.y + metrics->header_region.size.height;
    if (metrics->header_region.size.height > 0)
    {
        cursor_y += section_gap - 1;
    }

    metrics->rail_region.location.x = metrics->panel_region.location.x + panel_pad;
    metrics->rail_region.location.y = cursor_y;
    metrics->rail_region.size.width = metrics->panel_region.size.width - panel_pad * 2;
    metrics->rail_region.size.height = primary_count > 0 ? rail_h : 0;

    if (helper_h > 0)
    {
        metrics->helper_region.location.x = metrics->panel_region.location.x + panel_pad;
        metrics->helper_region.location.y = metrics->panel_region.location.y + metrics->panel_region.size.height - panel_pad - helper_h;
        metrics->helper_region.size.width = metrics->panel_region.size.width - panel_pad * 2;
        metrics->helper_region.size.height = helper_h;
        limit_y = metrics->helper_region.location.y - 1;
    }
    else
    {
        metrics->helper_region.size.width = 0;
        metrics->helper_region.size.height = 0;
        limit_y = metrics->panel_region.location.y + metrics->panel_region.size.height - panel_pad;
    }

    if (metrics->rail_region.size.height > 0 && metrics->rail_region.size.width > 0)
    {
        egui_dim_t min_w = local->compact_mode ? EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_ITEM_MIN_W : EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_ITEM_MIN_W;

        for (i = 0; i < primary_count; ++i)
        {
            widths[i] = egui_view_command_bar_flyout_measure_primary_width(local->compact_mode, &snapshot->primary_items[i]);
            total_w += widths[i];
        }

        available_w = metrics->rail_region.size.width - (primary_count > 0 ? (primary_count - 1) * item_gap : 0);
        if (available_w < 0)
        {
            available_w = 0;
        }
        if (primary_count > 0 && total_w > available_w && available_w > 0)
        {
            egui_dim_t shared_w = available_w / primary_count;

            if (shared_w < min_w)
            {
                shared_w = min_w;
            }

            for (i = 0; i < primary_count; ++i)
            {
                widths[i] = shared_w;
            }
        }

        cursor_x = metrics->rail_region.location.x;
        for (i = 0; i < primary_count; ++i)
        {
            metrics->primary_regions[i].location.x = cursor_x;
            metrics->primary_regions[i].location.y = metrics->rail_region.location.y + 1;
            metrics->primary_regions[i].size.width = widths[i];
            metrics->primary_regions[i].size.height = metrics->rail_region.size.height - 2;
            if (metrics->primary_regions[i].size.height < 0)
            {
                metrics->primary_regions[i].size.height = 0;
            }
            cursor_x += widths[i] + item_gap;
        }
    }

    cursor_y = metrics->rail_region.location.y + metrics->rail_region.size.height;
    if (metrics->rail_region.size.height > 0)
    {
        cursor_y += section_gap - 1;
    }

    for (i = 0; i < secondary_count; ++i)
    {
        if (snapshot->secondary_items[i].separator_before)
        {
            cursor_y += 2;
        }

        if (cursor_y + row_h > limit_y)
        {
            break;
        }

        metrics->secondary_regions[i].location.x = metrics->panel_region.location.x + panel_pad;
        metrics->secondary_regions[i].location.y = cursor_y;
        metrics->secondary_regions[i].size.width = metrics->panel_region.size.width - panel_pad * 2;
        metrics->secondary_regions[i].size.height = row_h;
        cursor_y += row_h + row_gap;
        metrics->secondary_count++;
    }
}

static uint8_t egui_view_command_bar_flyout_resolve_hit(egui_view_command_bar_flyout_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    egui_view_command_bar_flyout_metrics_t metrics;
    const egui_view_command_bar_flyout_snapshot_t *snapshot = egui_view_command_bar_flyout_get_snapshot(local);
    uint8_t i;

    egui_view_command_bar_flyout_get_metrics(local, self, snapshot, &metrics);
    if (metrics.trigger_region.size.width > 0 && egui_region_pt_in_rect(&metrics.trigger_region, x, y))
    {
        return EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER;
    }

    if (!metrics.show_panel)
    {
        return EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE;
    }

    for (i = 0; i < metrics.primary_count; ++i)
    {
        if (metrics.primary_regions[i].size.width > 0 && egui_region_pt_in_rect(&metrics.primary_regions[i], x, y))
        {
            return EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(i);
        }
    }

    for (i = 0; i < metrics.secondary_count; ++i)
    {
        if (metrics.secondary_regions[i].size.width > 0 && egui_region_pt_in_rect(&metrics.secondary_regions[i], x, y))
        {
            return EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART(i);
        }
    }

    return EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE;
}

static void egui_view_command_bar_flyout_sync_current_part(egui_view_command_bar_flyout_t *local)
{
    const egui_view_command_bar_flyout_snapshot_t *snapshot = egui_view_command_bar_flyout_get_snapshot(local);

    if (!egui_view_command_bar_flyout_part_exists(snapshot, local->open_state, local->current_part))
    {
        local->current_part = egui_view_command_bar_flyout_resolve_default_part(local, snapshot);
    }
}

static void egui_view_command_bar_flyout_set_current_part_inner(egui_view_t *self, uint8_t part, uint8_t clear_pressed)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    const egui_view_command_bar_flyout_snapshot_t *snapshot = egui_view_command_bar_flyout_get_snapshot(local);
    uint8_t had_pressed = 0;

    if (clear_pressed)
    {
        had_pressed = egui_view_command_bar_flyout_clear_pressed_state(self, local);
    }

    if (!egui_view_command_bar_flyout_part_exists(snapshot, local->open_state, part))
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    if (local->current_part == part)
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_part = part;
    egui_view_invalidate(self);
}

static uint8_t egui_view_command_bar_flyout_get_active_parts(egui_view_command_bar_flyout_t *local,
                                                             const egui_view_command_bar_flyout_snapshot_t *snapshot, uint8_t *parts)
{
    uint8_t count = 0;
    uint8_t i;

    if (snapshot == NULL)
    {
        return 0;
    }

    parts[count++] = EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER;

    if (!local->open_state)
    {
        return count;
    }

    for (i = 0; i < egui_view_command_bar_flyout_clamp_primary_count(snapshot->primary_count); ++i)
    {
        uint8_t part = EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(i);

        if (egui_view_command_bar_flyout_part_data_enabled(snapshot, local->open_state, part))
        {
            parts[count++] = part;
        }
    }

    for (i = 0; i < egui_view_command_bar_flyout_clamp_secondary_count(snapshot->secondary_count); ++i)
    {
        uint8_t part = EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART(i);

        if (egui_view_command_bar_flyout_part_data_enabled(snapshot, local->open_state, part))
        {
            parts[count++] = part;
        }
    }

    return count;
}

static void egui_view_command_bar_flyout_move_linear(egui_view_t *self, int8_t step, uint8_t wrap)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    const egui_view_command_bar_flyout_snapshot_t *snapshot = egui_view_command_bar_flyout_get_snapshot(local);
    uint8_t parts[1 + EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_PRIMARY_ITEMS + EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_SECONDARY_ITEMS];
    uint8_t count = egui_view_command_bar_flyout_get_active_parts(local, snapshot, parts);
    int16_t current = 0;
    int16_t next;

    if (count == 0)
    {
        return;
    }

    while (current < count && parts[current] != local->current_part)
    {
        current++;
    }

    if (current >= count)
    {
        local->current_part = parts[0];
        egui_view_invalidate(self);
        return;
    }

    next = current + step;
    if (wrap)
    {
        if (next < 0)
        {
            next = count - 1;
        }
        else if (next >= count)
        {
            next = 0;
        }
    }
    else if (next < 0 || next >= count)
    {
        return;
    }

    if (parts[next] != local->current_part)
    {
        local->current_part = parts[next];
        egui_view_invalidate(self);
    }
}

static void egui_view_command_bar_flyout_move_vertical(egui_view_t *self, int8_t direction)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    const egui_view_command_bar_flyout_snapshot_t *snapshot = egui_view_command_bar_flyout_get_snapshot(local);
    uint8_t target = local->current_part;
    uint8_t first_primary = egui_view_command_bar_flyout_find_primary_part(snapshot, local->open_state, 0, 1);
    uint8_t last_primary = egui_view_command_bar_flyout_find_primary_part(snapshot, local->open_state, 1, 1);
    uint8_t first_secondary = egui_view_command_bar_flyout_find_secondary_part(snapshot, local->open_state, 0, 1);

    if (snapshot == NULL || !local->open_state)
    {
        return;
    }

    if (direction > 0)
    {
        if (local->current_part == EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER)
        {
            target = first_primary != EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE ? first_primary : first_secondary;
        }
        else if (egui_view_command_bar_flyout_part_is_primary(local->current_part))
        {
            target = first_secondary != EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE ? first_secondary : local->current_part;
        }
        else if (egui_view_command_bar_flyout_part_is_secondary(local->current_part) &&
                 egui_view_command_bar_flyout_part_exists(snapshot, local->open_state, local->current_part + 1) &&
                 egui_view_command_bar_flyout_part_data_enabled(snapshot, local->open_state, local->current_part + 1))
        {
            target = local->current_part + 1;
        }
    }
    else
    {
        if (egui_view_command_bar_flyout_part_is_secondary(local->current_part))
        {
            if (local->current_part > EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_SECONDARY_BASE &&
                egui_view_command_bar_flyout_part_data_enabled(snapshot, local->open_state, local->current_part - 1))
            {
                target = local->current_part - 1;
            }
            else if (last_primary != EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE)
            {
                target = last_primary;
            }
            else
            {
                target = EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER;
            }
        }
        else if (egui_view_command_bar_flyout_part_is_primary(local->current_part))
        {
            target = EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER;
        }
    }

    if (target != local->current_part && target != EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE)
    {
        local->current_part = target;
        egui_view_invalidate(self);
    }
}

static void egui_view_command_bar_flyout_draw_trigger(egui_view_t *self, egui_view_command_bar_flyout_t *local,
                                                      const egui_view_command_bar_flyout_snapshot_t *snapshot,
                                                      const egui_view_command_bar_flyout_metrics_t *metrics)
{
    egui_region_t text_region;
    egui_color_t tone = local->accent_color;
    egui_color_t fill = egui_rgb_mix(local->surface_color, tone, local->open_state ? 10 : 4);
    egui_color_t border = egui_rgb_mix(local->border_color, tone, local->current_part == EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER ? 24 : 14);
    egui_color_t title = local->text_color;
    egui_color_t meta = egui_rgb_mix(local->muted_text_color, tone, 10);
    egui_dim_t eyebrow_w = egui_view_command_bar_flyout_measure_eyebrow_width(local->compact_mode, snapshot->eyebrow,
                                                                              metrics->trigger_region.size.width / 3);
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_RADIUS : EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_RADIUS;
    egui_dim_t chevron_x = metrics->trigger_region.location.x + metrics->trigger_region.size.width - (local->compact_mode ? 12 : 14);
    egui_dim_t chevron_y = metrics->trigger_region.location.y + metrics->trigger_region.size.height / 2;

    if (local->pressed_part == EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER &&
        egui_view_command_bar_flyout_part_data_enabled(snapshot, local->open_state, EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER))
    {
        fill = egui_rgb_mix(fill, tone, 14);
    }

    if (local->disabled_mode)
    {
        fill = egui_rgb_mix(fill, local->surface_color, 32);
        border = egui_rgb_mix(border, local->neutral_color, 20);
        title = egui_rgb_mix(title, local->muted_text_color, 18);
        meta = egui_rgb_mix(meta, local->muted_text_color, 20);
    }

    if (!egui_view_get_enable(self))
    {
        fill = egui_view_command_bar_flyout_mix_disabled(fill);
        border = egui_view_command_bar_flyout_mix_disabled(border);
        title = egui_view_command_bar_flyout_mix_disabled(title);
        meta = egui_view_command_bar_flyout_mix_disabled(meta);
    }

    egui_canvas_draw_round_rectangle_fill(metrics->trigger_region.location.x, metrics->trigger_region.location.y, metrics->trigger_region.size.width,
                                          metrics->trigger_region.size.height, radius, fill, egui_color_alpha_mix(self->alpha, 100));
    egui_canvas_draw_round_rectangle(metrics->trigger_region.location.x, metrics->trigger_region.location.y, metrics->trigger_region.size.width,
                                     metrics->trigger_region.size.height, radius, 1, border, egui_color_alpha_mix(self->alpha, 42));

    if (eyebrow_w > 0)
    {
        egui_dim_t pill_h = local->compact_mode ? 8 : 10;
        egui_dim_t pill_y = metrics->trigger_region.location.y + (metrics->trigger_region.size.height - pill_h) / 2;
        egui_dim_t pill_x = chevron_x - eyebrow_w - (local->compact_mode ? 6 : 8);

        egui_canvas_draw_round_rectangle_fill(pill_x, pill_y, eyebrow_w, pill_h, pill_h / 2, egui_rgb_mix(fill, tone, 20),
                                              egui_color_alpha_mix(self->alpha, 96));
        text_region.location.x = pill_x;
        text_region.location.y = pill_y;
        text_region.size.width = eyebrow_w;
        text_region.size.height = pill_h;
        egui_view_command_bar_flyout_draw_text(local->meta_font, self, snapshot->eyebrow, &text_region, EGUI_ALIGN_CENTER, meta);
    }

    text_region.location.x = metrics->trigger_region.location.x + (local->compact_mode ? 7 : 9);
    text_region.location.y = metrics->trigger_region.location.y;
    text_region.size.width = metrics->trigger_region.size.width - (local->compact_mode ? 24 : 28) - (eyebrow_w > 0 ? eyebrow_w + 8 : 0);
    text_region.size.height = metrics->trigger_region.size.height;
    egui_view_command_bar_flyout_draw_text(local->font, self, snapshot->trigger_label, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title);

    egui_canvas_draw_line(chevron_x - 3, chevron_y - (local->open_state ? -1 : 1), chevron_x, chevron_y + (local->open_state ? -2 : 2), 1, meta,
                          egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_line(chevron_x, chevron_y + (local->open_state ? -2 : 2), chevron_x + 3, chevron_y - (local->open_state ? -1 : 1), 1, meta,
                          egui_color_alpha_mix(self->alpha, 96));
}

static void egui_view_command_bar_flyout_draw_primary_item(egui_view_t *self, egui_view_command_bar_flyout_t *local,
                                                           const egui_view_command_bar_flyout_primary_item_t *item,
                                                           const egui_region_t *item_region, uint8_t part)
{
    egui_region_t text_region;
    egui_color_t tone = egui_view_command_bar_flyout_tone_color(local, item->tone);
    egui_color_t fill = item->emphasized ? tone : egui_rgb_mix(local->surface_color, tone, part == local->current_part ? 14 : 8);
    egui_color_t border = egui_rgb_mix(local->border_color, tone, part == local->current_part ? 22 : 16);
    egui_color_t text = item->emphasized ? EGUI_COLOR_WHITE : (item->tone == EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_DANGER ? tone : local->text_color);

    if (local->pressed_part == part && item->enabled)
    {
        fill = egui_rgb_mix(fill, tone, 16);
    }

    if (!item->enabled || local->disabled_mode)
    {
        fill = egui_rgb_mix(fill, local->surface_color, 28);
        border = egui_rgb_mix(border, local->muted_text_color, 26);
        text = egui_rgb_mix(text, local->muted_text_color, 32);
    }

    if (!egui_view_get_enable(self))
    {
        fill = egui_view_command_bar_flyout_mix_disabled(fill);
        border = egui_view_command_bar_flyout_mix_disabled(border);
        text = egui_view_command_bar_flyout_mix_disabled(text);
    }

    egui_canvas_draw_round_rectangle_fill(item_region->location.x, item_region->location.y, item_region->size.width, item_region->size.height,
                                          local->compact_mode ? 5 : 6, fill, egui_color_alpha_mix(self->alpha, 100));
    egui_canvas_draw_round_rectangle(item_region->location.x, item_region->location.y, item_region->size.width, item_region->size.height,
                                     local->compact_mode ? 5 : 6, 1, border, egui_color_alpha_mix(self->alpha, 30));

    if (!local->compact_mode && item->glyph != NULL && item->glyph[0] != '\0' && item_region->size.width > 24)
    {
        text_region.location.x = item_region->location.x + 4;
        text_region.location.y = item_region->location.y + (item_region->size.height - EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_GLYPH_H) / 2;
        text_region.size.width = EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_GLYPH_W;
        text_region.size.height = EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_GLYPH_H;
        egui_view_command_bar_flyout_draw_text(local->meta_font, self, item->glyph, &text_region, EGUI_ALIGN_CENTER, text);

        text_region.location.x += EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_GLYPH_W + 4;
        text_region.location.y = item_region->location.y;
        text_region.size.width = item_region->size.width - (EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_GLYPH_W + 12);
        text_region.size.height = item_region->size.height;
        egui_view_command_bar_flyout_draw_text(local->font, self, item->label, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text);
    }
    else
    {
        text_region.location.x = item_region->location.x + 3;
        text_region.location.y = item_region->location.y;
        text_region.size.width = item_region->size.width - 6;
        text_region.size.height = item_region->size.height;
        egui_view_command_bar_flyout_draw_text(local->font, self, item->label, &text_region, EGUI_ALIGN_CENTER, text);
    }
}

static void egui_view_command_bar_flyout_draw_secondary_row(egui_view_t *self, egui_view_command_bar_flyout_t *local,
                                                            const egui_view_command_bar_flyout_secondary_item_t *item,
                                                            const egui_region_t *row_region, uint8_t part)
{
    egui_region_t text_region;
    egui_color_t tone = egui_view_command_bar_flyout_tone_color(local, item->tone);
    egui_color_t row_fill = egui_rgb_mix(local->surface_color, tone, part == local->current_part ? 8 : (item->emphasized ? 6 : 3));
    egui_color_t title = item->tone == EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_DANGER ? egui_rgb_mix(local->text_color, tone, 44) : local->text_color;
    egui_color_t meta = egui_rgb_mix(local->muted_text_color, tone, 8);
    egui_color_t icon_fill = egui_rgb_mix(local->section_color, tone, 10);
    egui_dim_t icon_size = local->compact_mode ? 9 : 11;
    egui_dim_t icon_x = row_region->location.x + (local->compact_mode ? 5 : 7);
    egui_dim_t icon_y = row_region->location.y + (row_region->size.height - icon_size) / 2;
    egui_dim_t meta_w = row_region->size.width / 3;

    if (local->pressed_part == part && item->enabled)
    {
        row_fill = egui_rgb_mix(row_fill, tone, 12);
    }

    if (!item->enabled || local->disabled_mode)
    {
        row_fill = egui_rgb_mix(row_fill, local->surface_color, 34);
        title = egui_rgb_mix(title, local->muted_text_color, 32);
        meta = egui_rgb_mix(meta, local->muted_text_color, 38);
        icon_fill = egui_rgb_mix(icon_fill, local->surface_color, 34);
    }

    if (!egui_view_get_enable(self))
    {
        row_fill = egui_view_command_bar_flyout_mix_disabled(row_fill);
        title = egui_view_command_bar_flyout_mix_disabled(title);
        meta = egui_view_command_bar_flyout_mix_disabled(meta);
        icon_fill = egui_view_command_bar_flyout_mix_disabled(icon_fill);
    }

    if (item->separator_before)
    {
        egui_canvas_draw_line(row_region->location.x + 4, row_region->location.y - 1, row_region->location.x + row_region->size.width - 4,
                              row_region->location.y - 1, 1, egui_rgb_mix(local->border_color, local->surface_color, 22), egui_color_alpha_mix(self->alpha, 36));
    }

    egui_canvas_draw_round_rectangle_fill(row_region->location.x, row_region->location.y, row_region->size.width, row_region->size.height,
                                          local->compact_mode ? 5 : 6, row_fill, egui_color_alpha_mix(self->alpha, part == local->current_part ? 100 : 82));
    egui_canvas_draw_round_rectangle_fill(icon_x, icon_y, icon_size, icon_size, 3, icon_fill, egui_color_alpha_mix(self->alpha, 96));

    text_region.location.x = icon_x;
    text_region.location.y = icon_y;
    text_region.size.width = icon_size;
    text_region.size.height = icon_size;
    egui_view_command_bar_flyout_draw_text(local->meta_font, self, item->icon_text, &text_region, EGUI_ALIGN_CENTER, title);

    text_region.location.x = icon_x + icon_size + (local->compact_mode ? 4 : 6);
    text_region.location.y = row_region->location.y;
    text_region.size.width = row_region->size.width - (text_region.location.x - row_region->location.x) - meta_w - (local->compact_mode ? 4 : 6);
    text_region.size.height = row_region->size.height;
    egui_view_command_bar_flyout_draw_text(local->font, self, item->title, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title);

    text_region.location.x = row_region->location.x + row_region->size.width - meta_w;
    text_region.location.y = row_region->location.y;
    text_region.size.width = meta_w - 2;
    text_region.size.height = row_region->size.height;
    egui_view_command_bar_flyout_draw_text(local->meta_font, self, item->meta, &text_region, EGUI_ALIGN_RIGHT | EGUI_ALIGN_VCENTER, meta);
}

static void egui_view_command_bar_flyout_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    const egui_view_command_bar_flyout_snapshot_t *snapshot = egui_view_command_bar_flyout_get_snapshot(local);
    egui_view_command_bar_flyout_metrics_t metrics;
    egui_region_t text_region;
    egui_color_t panel_fill;
    egui_color_t panel_border;
    egui_color_t rail_fill;
    egui_color_t rail_border;
    egui_color_t header_text;
    egui_color_t helper_text;
    egui_dim_t eyebrow_w;
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_COMMAND_BAR_FLYOUT_COMPACT_RADIUS : EGUI_VIEW_COMMAND_BAR_FLYOUT_STANDARD_RADIUS;
    uint8_t i;

    if (snapshot == NULL)
    {
        return;
    }

    egui_view_command_bar_flyout_sync_current_part(local);
    egui_view_command_bar_flyout_get_metrics(local, self, snapshot, &metrics);
    egui_view_command_bar_flyout_draw_trigger(self, local, snapshot, &metrics);
    if (!metrics.show_panel)
    {
        return;
    }

    panel_fill = local->surface_color;
    panel_border = local->border_color;
    rail_fill = egui_rgb_mix(local->section_color, local->accent_color, 5);
    rail_border = egui_rgb_mix(local->border_color, local->accent_color, 10);
    header_text = local->text_color;
    helper_text = local->muted_text_color;

    if (local->disabled_mode)
    {
        panel_fill = egui_rgb_mix(panel_fill, EGUI_COLOR_HEX(0xF4F7FA), 38);
        panel_border = egui_rgb_mix(panel_border, local->neutral_color, 18);
        rail_fill = egui_rgb_mix(rail_fill, panel_fill, 42);
        rail_border = egui_rgb_mix(rail_border, local->neutral_color, 18);
        header_text = egui_rgb_mix(header_text, helper_text, 18);
        helper_text = egui_rgb_mix(helper_text, local->neutral_color, 22);
    }

    if (!egui_view_get_enable(self))
    {
        panel_fill = egui_view_command_bar_flyout_mix_disabled(panel_fill);
        panel_border = egui_view_command_bar_flyout_mix_disabled(panel_border);
        rail_fill = egui_view_command_bar_flyout_mix_disabled(rail_fill);
        rail_border = egui_view_command_bar_flyout_mix_disabled(rail_border);
        header_text = egui_view_command_bar_flyout_mix_disabled(header_text);
        helper_text = egui_view_command_bar_flyout_mix_disabled(helper_text);
    }

    egui_canvas_draw_round_rectangle_fill(metrics.panel_region.location.x, metrics.panel_region.location.y + 2, metrics.panel_region.size.width,
                                          metrics.panel_region.size.height, radius, egui_rgb_mix(local->shadow_color, EGUI_COLOR_HEX(0x0F172A), 12),
                                          egui_color_alpha_mix(self->alpha, local->compact_mode ? 8 : 11));
    egui_canvas_draw_round_rectangle_fill(metrics.panel_region.location.x, metrics.panel_region.location.y, metrics.panel_region.size.width,
                                          metrics.panel_region.size.height, radius, panel_fill, egui_color_alpha_mix(self->alpha, 100));
    egui_canvas_draw_round_rectangle(metrics.panel_region.location.x, metrics.panel_region.location.y, metrics.panel_region.size.width,
                                     metrics.panel_region.size.height, radius, 1, panel_border, egui_color_alpha_mix(self->alpha, 42));

    if (metrics.header_region.size.height > 0)
    {
        eyebrow_w = egui_view_command_bar_flyout_measure_eyebrow_width(local->compact_mode, snapshot->eyebrow, metrics.header_region.size.width / 3);
        if (eyebrow_w > 0)
        {
            egui_dim_t pill_h = local->compact_mode ? 8 : 10;
            egui_dim_t pill_y = metrics.header_region.location.y + (metrics.header_region.size.height - pill_h) / 2;
            egui_dim_t pill_x = metrics.header_region.location.x + metrics.header_region.size.width - eyebrow_w;

            egui_canvas_draw_round_rectangle_fill(pill_x, pill_y, eyebrow_w, pill_h, pill_h / 2,
                                                  egui_rgb_mix(local->section_color, local->accent_color, 16), egui_color_alpha_mix(self->alpha, 96));
            text_region.location.x = pill_x;
            text_region.location.y = pill_y;
            text_region.size.width = eyebrow_w;
            text_region.size.height = pill_h;
            egui_view_command_bar_flyout_draw_text(local->meta_font, self, snapshot->eyebrow, &text_region, EGUI_ALIGN_CENTER,
                                                   egui_rgb_mix(local->muted_text_color, local->accent_color, 24));
        }
        else
        {
            eyebrow_w = 0;
        }

        text_region.location.x = metrics.header_region.location.x;
        text_region.location.y = metrics.header_region.location.y;
        text_region.size.width = metrics.header_region.size.width - (eyebrow_w > 0 ? eyebrow_w + 6 : 0);
        text_region.size.height = metrics.header_region.size.height;
        egui_view_command_bar_flyout_draw_text(local->font, self, snapshot->panel_title, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, header_text);
    }

    if (metrics.rail_region.size.height > 0)
    {
        egui_canvas_draw_round_rectangle_fill(metrics.rail_region.location.x, metrics.rail_region.location.y, metrics.rail_region.size.width,
                                              metrics.rail_region.size.height, local->compact_mode ? 6 : 7, rail_fill,
                                              egui_color_alpha_mix(self->alpha, 92));
        egui_canvas_draw_round_rectangle(metrics.rail_region.location.x, metrics.rail_region.location.y, metrics.rail_region.size.width,
                                         metrics.rail_region.size.height, local->compact_mode ? 6 : 7, 1, rail_border,
                                         egui_color_alpha_mix(self->alpha, 34));
        for (i = 0; i < metrics.primary_count; ++i)
        {
            if (metrics.primary_regions[i].size.width > 0)
            {
                egui_view_command_bar_flyout_draw_primary_item(self, local, &snapshot->primary_items[i], &metrics.primary_regions[i],
                                                               EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(i));
            }
        }
    }

    for (i = 0; i < metrics.secondary_count; ++i)
    {
        if (metrics.secondary_regions[i].size.width > 0)
        {
            egui_view_command_bar_flyout_draw_secondary_row(self, local, &snapshot->secondary_items[i], &metrics.secondary_regions[i],
                                                            EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART(i));
        }
    }

    if (metrics.helper_region.size.height > 0)
    {
        text_region.location.x = metrics.helper_region.location.x;
        text_region.location.y = metrics.helper_region.location.y;
        text_region.size.width = metrics.helper_region.size.width;
        text_region.size.height = metrics.helper_region.size.height;
        egui_view_command_bar_flyout_draw_text(local->meta_font, self, snapshot->panel_helper, &text_region,
                                               EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, helper_text);
    }
}

void egui_view_command_bar_flyout_set_snapshots(egui_view_t *self, const egui_view_command_bar_flyout_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    const egui_view_command_bar_flyout_snapshot_t *snapshot;

    egui_view_command_bar_flyout_clear_pressed_state(self, local);
    local->snapshots = snapshots;
    local->snapshot_count = egui_view_command_bar_flyout_clamp_snapshot_count(snapshot_count);
    if (local->current_snapshot >= local->snapshot_count)
    {
        local->current_snapshot = 0;
    }

    snapshot = egui_view_command_bar_flyout_get_snapshot(local);
    local->open_state = snapshot != NULL && snapshot->open ? 1 : 0;
    local->current_part = egui_view_command_bar_flyout_resolve_default_part(local, snapshot);
    egui_view_invalidate(self);
}

void egui_view_command_bar_flyout_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    const egui_view_command_bar_flyout_snapshot_t *snapshot;
    uint8_t had_pressed = egui_view_command_bar_flyout_clear_pressed_state(self, local);

    if (local->snapshot_count == 0 || snapshot_index >= local->snapshot_count)
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    if (local->current_snapshot == snapshot_index)
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_snapshot = snapshot_index;
    snapshot = egui_view_command_bar_flyout_get_snapshot(local);
    local->open_state = snapshot != NULL && snapshot->open ? 1 : 0;
    local->current_part = egui_view_command_bar_flyout_resolve_default_part(local, snapshot);
    egui_view_invalidate(self);
}

uint8_t egui_view_command_bar_flyout_get_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    return local->current_snapshot;
}

void egui_view_command_bar_flyout_set_open(egui_view_t *self, uint8_t open)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    const egui_view_command_bar_flyout_snapshot_t *snapshot = egui_view_command_bar_flyout_get_snapshot(local);
    uint8_t had_pressed = egui_view_command_bar_flyout_clear_pressed_state(self, local);

    open = open ? 1 : 0;
    if (snapshot == NULL)
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    if (local->open_state == open)
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->open_state = open;
    local->current_part = egui_view_command_bar_flyout_resolve_default_part(local, snapshot);
    egui_view_invalidate(self);
}

uint8_t egui_view_command_bar_flyout_get_open(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    return local->open_state;
}

void egui_view_command_bar_flyout_set_current_part(egui_view_t *self, uint8_t part)
{
    egui_view_command_bar_flyout_set_current_part_inner(self, part, 1);
}

uint8_t egui_view_command_bar_flyout_get_current_part(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    egui_view_command_bar_flyout_sync_current_part(local);
    return local->current_part;
}

uint8_t egui_view_command_bar_flyout_activate_current_part(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    const egui_view_command_bar_flyout_snapshot_t *snapshot = egui_view_command_bar_flyout_get_snapshot(local);

    egui_view_command_bar_flyout_sync_current_part(local);
    if (!egui_view_command_bar_flyout_part_is_interactive(local, self, snapshot, local->current_part))
    {
        return 0;
    }

    if (local->current_part == EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER)
    {
        local->open_state = local->open_state ? 0 : 1;
        local->current_part = egui_view_command_bar_flyout_resolve_default_part(local, snapshot);
        egui_view_invalidate(self);
        return 1;
    }

    if (local->on_action != NULL)
    {
        local->on_action(self, local->current_snapshot, local->current_part);
    }
    egui_view_invalidate(self);
    return 1;
}

void egui_view_command_bar_flyout_set_on_action_listener(egui_view_t *self, egui_view_on_command_bar_flyout_action_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    local->on_action = listener;
}

void egui_view_command_bar_flyout_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    uint8_t had_pressed = egui_view_command_bar_flyout_clear_pressed_state(self, local);

    local->font = font ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_command_bar_flyout_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    uint8_t had_pressed = egui_view_command_bar_flyout_clear_pressed_state(self, local);

    local->meta_font = font ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_command_bar_flyout_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    uint8_t had_pressed = egui_view_command_bar_flyout_clear_pressed_state(self, local);

    compact_mode = compact_mode ? 1 : 0;
    if (local->compact_mode != compact_mode)
    {
        local->compact_mode = compact_mode;
        egui_view_invalidate(self);
        return;
    }

    if (had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_command_bar_flyout_set_disabled_mode(egui_view_t *self, uint8_t disabled_mode)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    uint8_t had_pressed = egui_view_command_bar_flyout_clear_pressed_state(self, local);

    disabled_mode = disabled_mode ? 1 : 0;
    if (local->disabled_mode != disabled_mode)
    {
        local->disabled_mode = disabled_mode;
        egui_view_invalidate(self);
        return;
    }

    if (had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_command_bar_flyout_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                                              egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                              egui_color_t success_color, egui_color_t warning_color, egui_color_t danger_color,
                                              egui_color_t neutral_color, egui_color_t shadow_color)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    uint8_t had_pressed = egui_view_command_bar_flyout_clear_pressed_state(self, local);

    local->surface_color = surface_color;
    local->section_color = section_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->danger_color = danger_color;
    local->neutral_color = neutral_color;
    local->shadow_color = shadow_color;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

uint8_t egui_view_command_bar_flyout_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    egui_view_command_bar_flyout_metrics_t metrics;
    const egui_view_command_bar_flyout_snapshot_t *snapshot = egui_view_command_bar_flyout_get_snapshot(local);
    uint8_t index;

    if (region == NULL)
    {
        return 0;
    }

    egui_view_command_bar_flyout_get_metrics(local, self, snapshot, &metrics);
    if (part == EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER)
    {
        *region = metrics.trigger_region;
        return metrics.trigger_region.size.width > 0 ? 1 : 0;
    }

    if (egui_view_command_bar_flyout_part_is_primary(part))
    {
        index = egui_view_command_bar_flyout_primary_index(part);
        if (index < metrics.primary_count && metrics.primary_regions[index].size.width > 0)
        {
            *region = metrics.primary_regions[index];
            return 1;
        }
        return 0;
    }

    if (egui_view_command_bar_flyout_part_is_secondary(part))
    {
        index = egui_view_command_bar_flyout_secondary_index(part);
        if (index < metrics.secondary_count && metrics.secondary_regions[index].size.width > 0)
        {
            *region = metrics.secondary_regions[index];
            return 1;
        }
        return 0;
    }

    return 0;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_command_bar_flyout_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    const egui_view_command_bar_flyout_snapshot_t *snapshot = egui_view_command_bar_flyout_get_snapshot(local);
    uint8_t hit_part;

    if (snapshot == NULL || local->compact_mode || local->disabled_mode || !egui_view_get_enable(self))
    {
        if (egui_view_command_bar_flyout_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_part = egui_view_command_bar_flyout_resolve_hit(local, self, event->location.x, event->location.y);
        if (!egui_view_command_bar_flyout_part_is_interactive(local, self, snapshot, hit_part))
        {
            if (egui_view_command_bar_flyout_clear_pressed_state(self, local))
            {
                egui_view_invalidate(self);
            }
            return 0;
        }
        local->pressed_part = hit_part;
        local->current_part = hit_part;
        egui_view_set_pressed(self, 1);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_part == EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE)
        {
            return 0;
        }
        hit_part = egui_view_command_bar_flyout_resolve_hit(local, self, event->location.x, event->location.y);
        egui_view_set_pressed(self, hit_part == local->pressed_part &&
                                            egui_view_command_bar_flyout_part_is_interactive(local, self, snapshot, local->pressed_part));
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
    {
        uint8_t handled;

        hit_part = egui_view_command_bar_flyout_resolve_hit(local, self, event->location.x, event->location.y);
        if (hit_part == local->pressed_part && egui_view_command_bar_flyout_part_is_interactive(local, self, snapshot, hit_part))
        {
            local->current_part = hit_part;
            egui_view_command_bar_flyout_activate_current_part(self);
        }
        handled = egui_view_command_bar_flyout_clear_pressed_state(self, local);
        if (handled)
        {
            egui_view_invalidate(self);
        }
        return handled || hit_part != EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE;
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (egui_view_command_bar_flyout_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}

static int egui_view_command_bar_flyout_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    EGUI_UNUSED(event);

    if (egui_view_command_bar_flyout_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_command_bar_flyout_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    const egui_view_command_bar_flyout_snapshot_t *snapshot = egui_view_command_bar_flyout_get_snapshot(local);

    if (snapshot == NULL || local->compact_mode || local->disabled_mode || !egui_view_get_enable(self))
    {
        if (egui_view_command_bar_flyout_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    if (event->key_code == EGUI_KEY_CODE_ENTER || event->key_code == EGUI_KEY_CODE_SPACE)
    {
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            if (!egui_view_command_bar_flyout_part_is_interactive(local, self, snapshot, local->current_part))
            {
                return 0;
            }
            local->pressed_part = local->current_part;
            egui_view_set_pressed(self, 1);
            egui_view_invalidate(self);
            return 1;
        }

        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            uint8_t handled = 0;

            if (local->pressed_part != EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE && local->pressed_part == local->current_part &&
                egui_view_command_bar_flyout_part_is_interactive(local, self, snapshot, local->pressed_part))
            {
                handled = egui_view_command_bar_flyout_activate_current_part(self);
            }
            if (egui_view_command_bar_flyout_clear_pressed_state(self, local))
            {
                egui_view_invalidate(self);
            }
            return handled;
        }

        return 0;
    }

    if (egui_view_command_bar_flyout_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }

    if (event->type != EGUI_KEY_EVENT_ACTION_UP)
    {
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_LEFT:
        egui_view_command_bar_flyout_move_linear(self, -1, 0);
        return 1;
    case EGUI_KEY_CODE_RIGHT:
        egui_view_command_bar_flyout_move_linear(self, 1, 0);
        return 1;
    case EGUI_KEY_CODE_TAB:
        egui_view_command_bar_flyout_move_linear(self, 1, 1);
        return 1;
    case EGUI_KEY_CODE_UP:
        egui_view_command_bar_flyout_move_vertical(self, -1);
        return 1;
    case EGUI_KEY_CODE_DOWN:
        egui_view_command_bar_flyout_move_vertical(self, 1);
        return 1;
    case EGUI_KEY_CODE_HOME:
        egui_view_command_bar_flyout_set_current_part_inner(self, EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER, 0);
        return 1;
    case EGUI_KEY_CODE_END:
    {
        uint8_t target = local->open_state ? egui_view_command_bar_flyout_find_secondary_part(snapshot, local->open_state, 1, 1) :
                                             EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE;

        if (target == EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE)
        {
            target = local->open_state ? egui_view_command_bar_flyout_find_primary_part(snapshot, local->open_state, 1, 1) :
                                         EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE;
        }
        if (target == EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE)
        {
            target = EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER;
        }
        egui_view_command_bar_flyout_set_current_part_inner(self, target, 0);
        return 1;
    }
    case EGUI_KEY_CODE_ESCAPE:
        if (local->open_state)
        {
            egui_view_command_bar_flyout_set_open(self, 0);
            return 1;
        }
        return 0;
    default:
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_command_bar_flyout_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_command_bar_flyout_t);
    EGUI_UNUSED(event);

    if (egui_view_command_bar_flyout_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_command_bar_flyout_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_command_bar_flyout_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_command_bar_flyout_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_command_bar_flyout_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_command_bar_flyout_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_command_bar_flyout_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_command_bar_flyout_on_key_event,
#endif
};

void egui_view_command_bar_flyout_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_command_bar_flyout_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_command_bar_flyout_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 1);
#endif

    local->snapshots = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->on_action = NULL;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->section_color = EGUI_COLOR_HEX(0xF4F7FA);
    local->border_color = EGUI_COLOR_HEX(0xD6DEE7);
    local->text_color = EGUI_COLOR_HEX(0x18222D);
    local->muted_text_color = EGUI_COLOR_HEX(0x6C7B89);
    local->accent_color = EGUI_COLOR_HEX(0x2563EB);
    local->success_color = EGUI_COLOR_HEX(0x178454);
    local->warning_color = EGUI_COLOR_HEX(0xB77719);
    local->danger_color = EGUI_COLOR_HEX(0xBA3C36);
    local->neutral_color = EGUI_COLOR_HEX(0x7B8897);
    local->shadow_color = EGUI_COLOR_HEX(0xD0D8E0);
    local->snapshot_count = 0;
    local->current_snapshot = 0;
    local->current_part = EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER;
    local->open_state = 0;
    local->compact_mode = 0;
    local->disabled_mode = 0;
    local->pressed_part = EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE;

    egui_view_set_view_name(self, "egui_view_command_bar_flyout");
}
