#include "egui_view_virtualizing_stack_panel.h"

#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_RADIUS       10
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_OUTER_PAD_X  6
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_OUTER_PAD_Y  6
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_INNER_PAD_X  6
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_INNER_PAD_Y  5
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_BADGE_H      8
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_BADGE_GAP    3
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_TITLE_H      10
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_SUMMARY_H    8
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_TITLE_GAP    1
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_SHELL_GAP    2
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_SHELL_RADIUS 8
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_SHELL_PAD_X  4
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_SHELL_PAD_Y  3
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_FOOTER_H     8
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_FOOTER_GAP   0
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_ROW_GAP      4
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_ROW_H        18
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_ROW_RADIUS   6
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_TRACK_W      4
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_TRACK_GAP    4
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_THUMB_MIN    14

#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_RADIUS       8
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_OUTER_PAD_X  5
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_OUTER_PAD_Y  5
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_INNER_PAD_X  5
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_INNER_PAD_Y  4
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_BADGE_H      7
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_BADGE_GAP    3
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_TITLE_H      9
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_SUMMARY_H    0
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_TITLE_GAP    0
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_SHELL_GAP    3
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_SHELL_RADIUS 6
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_SHELL_PAD_X  4
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_SHELL_PAD_Y  4
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_FOOTER_H     0
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_FOOTER_GAP   0
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_ROW_GAP      3
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_ROW_H        15
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_ROW_RADIUS   5
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_TRACK_W      3
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_TRACK_GAP    3
#define EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_THUMB_MIN    10

typedef struct egui_view_virtualizing_stack_panel_metrics egui_view_virtualizing_stack_panel_metrics_t;
struct egui_view_virtualizing_stack_panel_metrics
{
    egui_region_t content_region;
    egui_region_t badge_region;
    egui_region_t title_region;
    egui_region_t summary_region;
    egui_region_t shell_region;
    egui_region_t footer_region;
    egui_region_t track_region;
    egui_region_t thumb_region;
    egui_region_t item_regions[EGUI_VIEW_VIRTUALIZING_STACK_PANEL_MAX_ITEMS];
    uint8_t first_visible_item;
    uint8_t last_visible_item;
    uint8_t visible_count;
};

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_virtualizing_stack_panel_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event);
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_virtualizing_stack_panel_on_static_key_event(egui_view_t *self, egui_key_event_t *event);
#endif

static void virtualizing_stack_panel_reset_region(egui_region_t *region)
{
    region->location.x = 0;
    region->location.y = 0;
    region->size.width = 0;
    region->size.height = 0;
}

static uint8_t virtualizing_stack_panel_region_has_size(const egui_region_t *region)
{
    return region != NULL && region->size.width > 0 && region->size.height > 0 ? 1 : 0;
}

static void virtualizing_stack_panel_reset_metrics(egui_view_virtualizing_stack_panel_metrics_t *metrics)
{
    uint8_t item_index;

    virtualizing_stack_panel_reset_region(&metrics->content_region);
    virtualizing_stack_panel_reset_region(&metrics->badge_region);
    virtualizing_stack_panel_reset_region(&metrics->title_region);
    virtualizing_stack_panel_reset_region(&metrics->summary_region);
    virtualizing_stack_panel_reset_region(&metrics->shell_region);
    virtualizing_stack_panel_reset_region(&metrics->footer_region);
    virtualizing_stack_panel_reset_region(&metrics->track_region);
    virtualizing_stack_panel_reset_region(&metrics->thumb_region);
    for (item_index = 0; item_index < EGUI_VIEW_VIRTUALIZING_STACK_PANEL_MAX_ITEMS; ++item_index)
    {
        virtualizing_stack_panel_reset_region(&metrics->item_regions[item_index]);
    }
    metrics->first_visible_item = EGUI_VIEW_VIRTUALIZING_STACK_PANEL_ITEM_NONE;
    metrics->last_visible_item = EGUI_VIEW_VIRTUALIZING_STACK_PANEL_ITEM_NONE;
    metrics->visible_count = 0;
}

static uint8_t virtualizing_stack_panel_clamp_snapshot_count(uint8_t count)
{
    return count > EGUI_VIEW_VIRTUALIZING_STACK_PANEL_MAX_SNAPSHOTS ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_MAX_SNAPSHOTS : count;
}

static uint8_t virtualizing_stack_panel_clamp_item_count(uint8_t count)
{
    return count > EGUI_VIEW_VIRTUALIZING_STACK_PANEL_MAX_ITEMS ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_MAX_ITEMS : count;
}

static uint8_t virtualizing_stack_panel_text_len(const char *text)
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

static uint8_t virtualizing_stack_panel_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static egui_color_t virtualizing_stack_panel_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 68);
}

static const egui_view_virtualizing_stack_panel_snapshot_t *virtualizing_stack_panel_get_snapshot(egui_view_virtualizing_stack_panel_t *local)
{
    if (local->snapshots == NULL || local->snapshot_count == 0 || local->current_snapshot >= local->snapshot_count)
    {
        return NULL;
    }
    return &local->snapshots[local->current_snapshot];
}

static uint8_t virtualizing_stack_panel_get_item_count(const egui_view_virtualizing_stack_panel_snapshot_t *snapshot)
{
    if (snapshot == NULL || snapshot->items == NULL)
    {
        return 0;
    }
    return virtualizing_stack_panel_clamp_item_count(snapshot->item_count);
}

static const egui_view_virtualizing_stack_panel_item_t *virtualizing_stack_panel_get_item(const egui_view_virtualizing_stack_panel_snapshot_t *snapshot,
                                                                                           uint8_t item_index)
{
    if (item_index >= virtualizing_stack_panel_get_item_count(snapshot))
    {
        return NULL;
    }
    return &snapshot->items[item_index];
}

static egui_color_t virtualizing_stack_panel_tone_color(egui_view_virtualizing_stack_panel_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_SUCCESS:
        return local->success_color;
    case EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_WARNING:
        return local->warning_color;
    case EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_NEUTRAL:
        return local->neutral_color;
    case EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_ACCENT:
    default:
        return local->accent_color;
    }
}

static uint8_t virtualizing_stack_panel_clear_pressed_state(egui_view_t *self, egui_view_virtualizing_stack_panel_t *local)
{
    uint8_t had_pressed = self->is_pressed || local->pressed_item != EGUI_VIEW_VIRTUALIZING_STACK_PANEL_ITEM_NONE;

    local->pressed_item = EGUI_VIEW_VIRTUALIZING_STACK_PANEL_ITEM_NONE;
    egui_view_set_pressed(self, false);
    return had_pressed;
}

static uint8_t virtualizing_stack_panel_item_exists(const egui_view_virtualizing_stack_panel_snapshot_t *snapshot, uint8_t item_index)
{
    return item_index < virtualizing_stack_panel_get_item_count(snapshot) ? 1 : 0;
}

static uint8_t virtualizing_stack_panel_resolve_default_item(const egui_view_virtualizing_stack_panel_snapshot_t *snapshot)
{
    uint8_t item_count = virtualizing_stack_panel_get_item_count(snapshot);

    if (item_count == 0)
    {
        return EGUI_VIEW_VIRTUALIZING_STACK_PANEL_ITEM_NONE;
    }
    return snapshot->selected_item < item_count ? snapshot->selected_item : 0;
}

static uint8_t virtualizing_stack_panel_normalize_anchor(const egui_view_virtualizing_stack_panel_snapshot_t *snapshot, uint8_t anchor)
{
    uint8_t item_count = virtualizing_stack_panel_get_item_count(snapshot);

    if (item_count == 0)
    {
        return 0;
    }
    return anchor < item_count ? anchor : (item_count - 1);
}

static uint8_t virtualizing_stack_panel_resolve_default_anchor(const egui_view_virtualizing_stack_panel_snapshot_t *snapshot)
{
    uint8_t item_count = virtualizing_stack_panel_get_item_count(snapshot);

    if (item_count == 0)
    {
        return 0;
    }
    if (snapshot->window_anchor < item_count)
    {
        return snapshot->window_anchor;
    }
    if (snapshot->selected_item < item_count)
    {
        return snapshot->selected_item;
    }
    return 0;
}

static uint8_t virtualizing_stack_panel_item_is_interactive(egui_view_virtualizing_stack_panel_t *local, egui_view_t *self,
                                                            const egui_view_virtualizing_stack_panel_snapshot_t *snapshot, uint8_t item_index)
{
    if (snapshot == NULL || local->read_only_mode || !egui_view_get_enable(self))
    {
        return 0;
    }
    return virtualizing_stack_panel_item_exists(snapshot, item_index);
}

static egui_dim_t virtualizing_stack_panel_pill_width(const char *text, uint8_t compact_mode, egui_dim_t min_width, egui_dim_t max_width)
{
    egui_dim_t width = min_width;

    if (virtualizing_stack_panel_has_text(text))
    {
        width += virtualizing_stack_panel_text_len(text) * (compact_mode ? 4 : 5);
    }
    if (width > max_width)
    {
        width = max_width;
    }
    return width;
}

static void virtualizing_stack_panel_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                               egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (font == NULL || !virtualizing_stack_panel_has_text(text) || !virtualizing_stack_panel_region_has_size(region))
    {
        return;
    }
    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, self->alpha);
}

static void virtualizing_stack_panel_get_metrics(egui_view_virtualizing_stack_panel_t *local, egui_view_t *self,
                                                 const egui_view_virtualizing_stack_panel_snapshot_t *snapshot, uint8_t anchor,
                                                 egui_view_virtualizing_stack_panel_metrics_t *metrics)
{
    egui_region_t work_region;
    egui_dim_t outer_pad_x = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_OUTER_PAD_X : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_OUTER_PAD_X;
    egui_dim_t outer_pad_y = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_OUTER_PAD_Y : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_OUTER_PAD_Y;
    egui_dim_t inner_pad_x = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_INNER_PAD_X : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_INNER_PAD_X;
    egui_dim_t inner_pad_y = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_INNER_PAD_Y : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_INNER_PAD_Y;
    egui_dim_t badge_h = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_BADGE_H : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_BADGE_H;
    egui_dim_t badge_gap = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_BADGE_GAP : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_BADGE_GAP;
    egui_dim_t title_h = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_TITLE_H : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_TITLE_H;
    egui_dim_t summary_h = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_SUMMARY_H : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_SUMMARY_H;
    egui_dim_t title_gap = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_TITLE_GAP : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_TITLE_GAP;
    egui_dim_t shell_gap = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_SHELL_GAP : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_SHELL_GAP;
    egui_dim_t shell_pad_x = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_SHELL_PAD_X : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_SHELL_PAD_X;
    egui_dim_t shell_pad_y = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_SHELL_PAD_Y : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_SHELL_PAD_Y;
    egui_dim_t footer_h = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_FOOTER_H : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_FOOTER_H;
    egui_dim_t footer_gap = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_FOOTER_GAP : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_FOOTER_GAP;
    egui_dim_t row_gap = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_ROW_GAP : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_ROW_GAP;
    egui_dim_t row_h = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_ROW_H : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_ROW_H;
    egui_dim_t track_w = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_TRACK_W : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_TRACK_W;
    egui_dim_t track_gap = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_TRACK_GAP : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_TRACK_GAP;
    egui_dim_t thumb_min = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_THUMB_MIN : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_THUMB_MIN;
    egui_dim_t inner_x;
    egui_dim_t inner_y;
    egui_dim_t inner_w;
    egui_dim_t inner_h;
    egui_dim_t cursor_y;
    egui_dim_t footer_y;
    egui_dim_t items_x;
    egui_dim_t items_y;
    egui_dim_t items_w;
    egui_dim_t items_h;
    egui_dim_t thumb_h;
    egui_dim_t thumb_y;
    egui_dim_t scroll_span;
    uint8_t item_count = virtualizing_stack_panel_get_item_count(snapshot);
    uint8_t normalized_anchor = virtualizing_stack_panel_normalize_anchor(snapshot, anchor);
    uint8_t visible_count = 0;
    uint8_t max_start = 0;
    uint8_t item_index;

    virtualizing_stack_panel_reset_metrics(metrics);
    egui_view_get_work_region(self, &work_region);
    metrics->content_region.location.x = work_region.location.x + outer_pad_x;
    metrics->content_region.location.y = work_region.location.y + outer_pad_y;
    metrics->content_region.size.width = work_region.size.width - outer_pad_x * 2;
    metrics->content_region.size.height = work_region.size.height - outer_pad_y * 2;

    if (!virtualizing_stack_panel_region_has_size(&metrics->content_region) || snapshot == NULL)
    {
        return;
    }

    inner_x = metrics->content_region.location.x + inner_pad_x;
    inner_y = metrics->content_region.location.y + inner_pad_y;
    inner_w = metrics->content_region.size.width - inner_pad_x * 2;
    inner_h = metrics->content_region.size.height - inner_pad_y * 2;
    if (inner_w <= 0 || inner_h <= 0)
    {
        return;
    }

    cursor_y = inner_y;
    if (virtualizing_stack_panel_has_text(snapshot->header))
    {
        metrics->badge_region.location.x = inner_x;
        metrics->badge_region.location.y = cursor_y;
        metrics->badge_region.size.width = virtualizing_stack_panel_pill_width(snapshot->header, local->compact_mode, local->compact_mode ? 18 : 24, inner_w);
        metrics->badge_region.size.height = badge_h;
        cursor_y += badge_h + badge_gap;
    }

    metrics->title_region.location.x = inner_x;
    metrics->title_region.location.y = cursor_y;
    metrics->title_region.size.width = inner_w;
    metrics->title_region.size.height = title_h;
    cursor_y += title_h;

    if (summary_h > 0 && virtualizing_stack_panel_has_text(snapshot->summary))
    {
        cursor_y += title_gap;
        metrics->summary_region.location.x = inner_x;
        metrics->summary_region.location.y = cursor_y;
        metrics->summary_region.size.width = inner_w;
        metrics->summary_region.size.height = summary_h;
        cursor_y += summary_h;
    }

    cursor_y += shell_gap;
    footer_y = inner_y + inner_h;
    if (footer_h > 0 && virtualizing_stack_panel_has_text(snapshot->footer))
    {
        footer_y -= footer_h;
        metrics->footer_region.location.x = inner_x;
        metrics->footer_region.location.y = footer_y;
        metrics->footer_region.size.width =
                virtualizing_stack_panel_pill_width(snapshot->footer, local->compact_mode, local->compact_mode ? 18 : 22, inner_w);
        metrics->footer_region.size.height = footer_h;
        footer_y -= footer_gap;
    }

    metrics->shell_region.location.x = inner_x;
    metrics->shell_region.location.y = cursor_y;
    metrics->shell_region.size.width = inner_w;
    metrics->shell_region.size.height = footer_y - cursor_y;
    if (!virtualizing_stack_panel_region_has_size(&metrics->shell_region) || item_count == 0)
    {
        return;
    }

    items_x = metrics->shell_region.location.x + shell_pad_x;
    items_y = metrics->shell_region.location.y + shell_pad_y;
    items_w = metrics->shell_region.size.width - shell_pad_x * 2;
    items_h = metrics->shell_region.size.height - shell_pad_y * 2;
    if (items_w <= 0 || items_h <= 0)
    {
        return;
    }

    if (items_h + row_gap >= row_h + row_gap)
    {
        visible_count = (uint8_t)((items_h + row_gap) / (row_h + row_gap));
    }
    if (visible_count == 0)
    {
        visible_count = 1;
    }
    if (visible_count > item_count)
    {
        visible_count = item_count;
    }

    if (item_count > visible_count && items_w > track_w + track_gap + 36)
    {
        metrics->track_region.location.x = items_x + items_w - track_w;
        metrics->track_region.location.y = items_y;
        metrics->track_region.size.width = track_w;
        metrics->track_region.size.height = items_h;
        items_w -= track_w + track_gap;
    }

    max_start = item_count > visible_count ? (item_count - visible_count) : 0;
    if (normalized_anchor > max_start)
    {
        normalized_anchor = max_start;
    }

    metrics->first_visible_item = normalized_anchor;
    metrics->last_visible_item = (uint8_t)(normalized_anchor + visible_count - 1);
    metrics->visible_count = visible_count;

    for (item_index = 0; item_index < visible_count; ++item_index)
    {
        uint8_t visible_item = (uint8_t)(normalized_anchor + item_index);

        metrics->item_regions[visible_item].location.x = items_x;
        metrics->item_regions[visible_item].location.y = items_y + item_index * (row_h + row_gap);
        metrics->item_regions[visible_item].size.width = items_w;
        metrics->item_regions[visible_item].size.height = row_h;
    }

    if (virtualizing_stack_panel_region_has_size(&metrics->track_region) && item_count > visible_count)
    {
        thumb_h = (egui_dim_t)((metrics->track_region.size.height * visible_count) / item_count);
        if (thumb_h < thumb_min)
        {
            thumb_h = thumb_min;
        }
        if (thumb_h > metrics->track_region.size.height)
        {
            thumb_h = metrics->track_region.size.height;
        }

        scroll_span = item_count - visible_count;
        thumb_y = metrics->track_region.location.y;
        if (scroll_span > 0 && metrics->track_region.size.height > thumb_h)
        {
            thumb_y += (egui_dim_t)(((metrics->track_region.size.height - thumb_h) * normalized_anchor) / scroll_span);
        }

        metrics->thumb_region.location.x = metrics->track_region.location.x;
        metrics->thumb_region.location.y = thumb_y;
        metrics->thumb_region.size.width = metrics->track_region.size.width;
        metrics->thumb_region.size.height = thumb_h;
    }
}

static void virtualizing_stack_panel_sync_window_state(egui_view_virtualizing_stack_panel_t *local, egui_view_t *self)
{
    const egui_view_virtualizing_stack_panel_snapshot_t *snapshot = virtualizing_stack_panel_get_snapshot(local);
    egui_view_virtualizing_stack_panel_metrics_t metrics;
    uint8_t item_count = virtualizing_stack_panel_get_item_count(snapshot);
    uint8_t visible_count;
    uint8_t max_start;

    if (item_count == 0)
    {
        local->current_item = EGUI_VIEW_VIRTUALIZING_STACK_PANEL_ITEM_NONE;
        local->window_anchor = 0;
        return;
    }

    if (!virtualizing_stack_panel_item_exists(snapshot, local->current_item))
    {
        local->current_item = virtualizing_stack_panel_resolve_default_item(snapshot);
    }
    local->window_anchor = virtualizing_stack_panel_normalize_anchor(snapshot, local->window_anchor);

    virtualizing_stack_panel_get_metrics(local, self, snapshot, local->window_anchor, &metrics);
    visible_count = metrics.visible_count > 0 ? metrics.visible_count : 1;
    max_start = item_count > visible_count ? (item_count - visible_count) : 0;
    if (local->window_anchor > max_start)
    {
        local->window_anchor = max_start;
    }
    if (local->current_item < local->window_anchor)
    {
        local->window_anchor = local->current_item;
    }
    else if (local->current_item >= local->window_anchor + visible_count)
    {
        local->window_anchor = (uint8_t)(local->current_item - visible_count + 1);
        if (local->window_anchor > max_start)
        {
            local->window_anchor = max_start;
        }
    }
}

static void virtualizing_stack_panel_set_current_snapshot_inner(egui_view_t *self, uint8_t snapshot_index, uint8_t invalidate)
{
    EGUI_LOCAL_INIT(egui_view_virtualizing_stack_panel_t);
    const egui_view_virtualizing_stack_panel_snapshot_t *snapshot;

    if (local->snapshots == NULL || local->snapshot_count == 0 || snapshot_index >= local->snapshot_count)
    {
        return;
    }
    if (local->current_snapshot == snapshot_index)
    {
        if (virtualizing_stack_panel_clear_pressed_state(self, local) && invalidate)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_snapshot = snapshot_index;
    snapshot = virtualizing_stack_panel_get_snapshot(local);
    local->current_item = virtualizing_stack_panel_resolve_default_item(snapshot);
    local->window_anchor = virtualizing_stack_panel_resolve_default_anchor(snapshot);
    virtualizing_stack_panel_sync_window_state(local, self);
    virtualizing_stack_panel_clear_pressed_state(self, local);
    if (invalidate)
    {
        egui_view_invalidate(self);
    }
}

static void virtualizing_stack_panel_set_current_item_inner(egui_view_t *self, uint8_t item_index, uint8_t invalidate)
{
    EGUI_LOCAL_INIT(egui_view_virtualizing_stack_panel_t);
    const egui_view_virtualizing_stack_panel_snapshot_t *snapshot = virtualizing_stack_panel_get_snapshot(local);

    if (!virtualizing_stack_panel_item_exists(snapshot, item_index))
    {
        return;
    }
    if (local->current_item == item_index)
    {
        if (virtualizing_stack_panel_clear_pressed_state(self, local) && invalidate)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_item = item_index;
    virtualizing_stack_panel_sync_window_state(local, self);
    virtualizing_stack_panel_clear_pressed_state(self, local);
    if (invalidate)
    {
        egui_view_invalidate(self);
    }
}

static void virtualizing_stack_panel_set_window_anchor_inner(egui_view_t *self, uint8_t item_index, uint8_t invalidate)
{
    EGUI_LOCAL_INIT(egui_view_virtualizing_stack_panel_t);
    const egui_view_virtualizing_stack_panel_snapshot_t *snapshot = virtualizing_stack_panel_get_snapshot(local);

    if (!virtualizing_stack_panel_item_exists(snapshot, item_index))
    {
        return;
    }
    if (local->window_anchor == item_index)
    {
        if (virtualizing_stack_panel_clear_pressed_state(self, local) && invalidate)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->window_anchor = item_index;
    local->current_item = item_index;
    virtualizing_stack_panel_sync_window_state(local, self);
    virtualizing_stack_panel_clear_pressed_state(self, local);
    if (invalidate)
    {
        egui_view_invalidate(self);
    }
}

static void virtualizing_stack_panel_draw_item(egui_view_t *self, egui_view_virtualizing_stack_panel_t *local,
                                               const egui_view_virtualizing_stack_panel_item_t *item, const egui_region_t *region, uint8_t item_index)
{
    egui_region_t badge_region;
    egui_region_t title_region;
    egui_region_t meta_region;
    egui_region_t value_region;
    egui_color_t tone_color = virtualizing_stack_panel_tone_color(local, item->tone);
    egui_color_t row_fill = egui_rgb_mix(local->surface_color, tone_color, item_index == local->current_item ? 12 : (local->compact_mode ? 4 : 7));
    egui_color_t row_border = egui_rgb_mix(local->border_color, tone_color, item_index == local->current_item ? 22 : (local->compact_mode ? 10 : 14));
    egui_color_t badge_fill = egui_rgb_mix(local->surface_color, tone_color, item_index == local->current_item ? 20 : 12);
    egui_color_t badge_text = item_index == local->current_item ? tone_color : egui_rgb_mix(local->text_color, tone_color, 18);
    egui_color_t title_color = item_index == local->current_item ? egui_rgb_mix(local->text_color, tone_color, 8) : local->text_color;
    egui_color_t meta_color = egui_rgb_mix(local->muted_text_color, tone_color, item_index == local->current_item ? 20 : 12);
    egui_color_t value_fill = egui_rgb_mix(local->surface_color, tone_color, item_index == local->current_item ? 14 : 10);
    egui_color_t value_border = egui_rgb_mix(local->border_color, tone_color, item_index == local->current_item ? 24 : 18);
    egui_color_t value_text = item_index == local->current_item ? tone_color : egui_rgb_mix(local->muted_text_color, tone_color, 20);
    egui_dim_t pad_x = local->compact_mode ? 5 : 6;
    egui_dim_t row_radius = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_ROW_RADIUS : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_ROW_RADIUS;
    egui_dim_t badge_h = local->compact_mode ? 8 : 9;
    egui_dim_t title_h = local->compact_mode ? 9 : 10;
    egui_dim_t meta_h = local->compact_mode ? 0 : 8;
    egui_dim_t value_h = local->compact_mode ? 9 : 10;
    egui_dim_t indicator_w = local->compact_mode ? 2 : 3;
    egui_dim_t text_left = region->location.x + pad_x;
    egui_dim_t text_right = region->location.x + region->size.width - pad_x;
    egui_dim_t vertical_gap = local->compact_mode ? 0 : 1;

    if (!virtualizing_stack_panel_region_has_size(region))
    {
        return;
    }

    if (item->emphasized)
    {
        row_fill = egui_rgb_mix(row_fill, tone_color, 8);
        row_border = egui_rgb_mix(row_border, tone_color, 8);
    }
    if (item_index == local->pressed_item && self->is_pressed)
    {
        row_fill = egui_rgb_mix(row_fill, tone_color, 14);
    }
    if (local->read_only_mode)
    {
        row_fill = egui_rgb_mix(row_fill, local->surface_color, 26);
        row_border = egui_rgb_mix(row_border, local->muted_text_color, 22);
        badge_fill = egui_rgb_mix(badge_fill, local->surface_color, 26);
        badge_text = egui_rgb_mix(badge_text, local->muted_text_color, 28);
        title_color = egui_rgb_mix(title_color, local->muted_text_color, 14);
        meta_color = egui_rgb_mix(meta_color, local->muted_text_color, 20);
        value_fill = egui_rgb_mix(value_fill, local->surface_color, 24);
        value_border = egui_rgb_mix(value_border, local->muted_text_color, 20);
        value_text = egui_rgb_mix(value_text, local->muted_text_color, 28);
    }
    if (!egui_view_get_enable(self))
    {
        row_fill = virtualizing_stack_panel_mix_disabled(row_fill);
        row_border = virtualizing_stack_panel_mix_disabled(row_border);
        badge_fill = virtualizing_stack_panel_mix_disabled(badge_fill);
        badge_text = virtualizing_stack_panel_mix_disabled(badge_text);
        title_color = virtualizing_stack_panel_mix_disabled(title_color);
        meta_color = virtualizing_stack_panel_mix_disabled(meta_color);
        value_fill = virtualizing_stack_panel_mix_disabled(value_fill);
        value_border = virtualizing_stack_panel_mix_disabled(value_border);
        value_text = virtualizing_stack_panel_mix_disabled(value_text);
    }

    egui_canvas_draw_round_rectangle_fill(region->location.x, region->location.y, region->size.width, region->size.height, row_radius, row_fill,
                                          egui_color_alpha_mix(self->alpha, local->compact_mode ? 84 : 94));
    egui_canvas_draw_round_rectangle(region->location.x, region->location.y, region->size.width, region->size.height, row_radius, 1, row_border,
                                     egui_color_alpha_mix(self->alpha, item_index == local->current_item ? 44 : 28));

    if (item_index == local->current_item)
    {
        egui_canvas_draw_round_rectangle_fill(region->location.x + 1, region->location.y + 2, indicator_w, region->size.height - 4, 1, tone_color,
                                              egui_color_alpha_mix(self->alpha, 100));
    }

    if (virtualizing_stack_panel_has_text(item->badge))
    {
        badge_region.location.x = text_left;
        badge_region.location.y = region->location.y + (region->size.height - badge_h) / 2;
        badge_region.size.width = virtualizing_stack_panel_pill_width(item->badge, local->compact_mode, local->compact_mode ? 18 : 22, region->size.width / 3);
        badge_region.size.height = badge_h;
        egui_canvas_draw_round_rectangle_fill(badge_region.location.x, badge_region.location.y, badge_region.size.width, badge_region.size.height,
                                              badge_region.size.height / 2, badge_fill, egui_color_alpha_mix(self->alpha, 94));
        egui_canvas_draw_round_rectangle(badge_region.location.x, badge_region.location.y, badge_region.size.width, badge_region.size.height,
                                         badge_region.size.height / 2, 1, row_border, egui_color_alpha_mix(self->alpha, 24));
        virtualizing_stack_panel_draw_text(local->meta_font, self, item->badge, &badge_region, EGUI_ALIGN_CENTER, badge_text);
        text_left = badge_region.location.x + badge_region.size.width + 5;
    }

    if (virtualizing_stack_panel_has_text(item->value))
    {
        value_region.size.width = virtualizing_stack_panel_pill_width(item->value, local->compact_mode, local->compact_mode ? 16 : 20, region->size.width / 3);
        value_region.size.height = value_h;
        value_region.location.x = text_right - value_region.size.width;
        value_region.location.y = region->location.y + (region->size.height - value_h) / 2;

        if (local->compact_mode)
        {
            virtualizing_stack_panel_draw_text(local->meta_font, self, item->value, &value_region, EGUI_ALIGN_RIGHT | EGUI_ALIGN_VCENTER, value_text);
        }
        else
        {
            egui_canvas_draw_round_rectangle_fill(value_region.location.x, value_region.location.y, value_region.size.width, value_region.size.height,
                                                  value_region.size.height / 2, value_fill, egui_color_alpha_mix(self->alpha, 96));
            egui_canvas_draw_round_rectangle(value_region.location.x, value_region.location.y, value_region.size.width, value_region.size.height,
                                             value_region.size.height / 2, 1, value_border, egui_color_alpha_mix(self->alpha, 34));
            virtualizing_stack_panel_draw_text(local->meta_font, self, item->value, &value_region, EGUI_ALIGN_CENTER, value_text);
        }
        text_right = value_region.location.x - 5;
    }

    if (text_right <= text_left)
    {
        return;
    }

    title_region.location.x = text_left;
    title_region.size.width = text_right - text_left;
    title_region.size.height = title_h;
    if (meta_h > 0 && virtualizing_stack_panel_has_text(item->meta))
    {
        title_region.location.y = region->location.y + (region->size.height - (title_h + vertical_gap + meta_h)) / 2;
        meta_region.location.x = text_left;
        meta_region.location.y = title_region.location.y + title_h + vertical_gap;
        meta_region.size.width = text_right - text_left;
        meta_region.size.height = meta_h;
    }
    else
    {
        title_region.location.y = region->location.y + (region->size.height - title_h) / 2;
        virtualizing_stack_panel_reset_region(&meta_region);
    }
    virtualizing_stack_panel_draw_text(local->font, self, item->title, &title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);
    virtualizing_stack_panel_draw_text(local->meta_font, self, item->meta, &meta_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, meta_color);
}

static uint8_t virtualizing_stack_panel_hit_item(egui_view_virtualizing_stack_panel_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    const egui_view_virtualizing_stack_panel_snapshot_t *snapshot = virtualizing_stack_panel_get_snapshot(local);
    egui_view_virtualizing_stack_panel_metrics_t metrics;
    uint8_t item_count = virtualizing_stack_panel_get_item_count(snapshot);
    uint8_t item_index;

    if (snapshot == NULL)
    {
        return EGUI_VIEW_VIRTUALIZING_STACK_PANEL_ITEM_NONE;
    }

    virtualizing_stack_panel_get_metrics(local, self, snapshot, local->window_anchor, &metrics);
    for (item_index = 0; item_index < item_count; ++item_index)
    {
        if (!virtualizing_stack_panel_region_has_size(&metrics.item_regions[item_index]))
        {
            continue;
        }
        if (egui_region_pt_in_rect(&metrics.item_regions[item_index], x, y))
        {
            return item_index;
        }
    }
    return EGUI_VIEW_VIRTUALIZING_STACK_PANEL_ITEM_NONE;
}

static void egui_view_virtualizing_stack_panel_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_virtualizing_stack_panel_t);
    const egui_view_virtualizing_stack_panel_snapshot_t *snapshot = virtualizing_stack_panel_get_snapshot(local);
    const egui_view_virtualizing_stack_panel_item_t *current_item = virtualizing_stack_panel_get_item(snapshot, local->current_item);
    egui_view_virtualizing_stack_panel_metrics_t metrics;
    egui_color_t card_fill;
    egui_color_t card_border;
    egui_color_t shell_fill;
    egui_color_t shell_border;
    egui_color_t badge_fill;
    egui_color_t badge_border;
    egui_color_t badge_text;
    egui_color_t title_color;
    egui_color_t summary_color;
    egui_color_t footer_fill;
    egui_color_t footer_border;
    egui_color_t footer_text;
    egui_color_t track_fill;
    egui_color_t thumb_fill;
    egui_color_t thumb_border;
    egui_color_t tone_color = current_item != NULL ? virtualizing_stack_panel_tone_color(local, current_item->tone) : local->accent_color;
    egui_dim_t card_radius = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_RADIUS : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_RADIUS;
    egui_dim_t shell_radius = local->compact_mode ? EGUI_VIEW_VIRTUALIZING_STACK_PANEL_COMPACT_SHELL_RADIUS : EGUI_VIEW_VIRTUALIZING_STACK_PANEL_STANDARD_SHELL_RADIUS;
    uint8_t item_count;
    uint8_t item_index;

    if (snapshot == NULL)
    {
        return;
    }

    virtualizing_stack_panel_sync_window_state(local, self);
    virtualizing_stack_panel_get_metrics(local, self, snapshot, local->window_anchor, &metrics);
    if (!virtualizing_stack_panel_region_has_size(&metrics.content_region))
    {
        return;
    }

    card_fill = egui_rgb_mix(local->surface_color, local->section_color, local->compact_mode ? 6 : 8);
    card_border = egui_rgb_mix(local->border_color, tone_color, local->compact_mode ? 12 : 16);
    shell_fill = egui_rgb_mix(local->section_color, local->surface_color, local->compact_mode ? 14 : 10);
    shell_border = egui_rgb_mix(local->border_color, tone_color, local->compact_mode ? 10 : 14);
    badge_fill = egui_rgb_mix(local->surface_color, tone_color, local->compact_mode ? 18 : 14);
    badge_border = egui_rgb_mix(local->border_color, tone_color, 20);
    badge_text = tone_color;
    title_color = local->text_color;
    summary_color = egui_rgb_mix(local->muted_text_color, tone_color, 14);
    footer_fill = egui_rgb_mix(local->surface_color, tone_color, local->compact_mode ? 8 : 14);
    footer_border = egui_rgb_mix(local->border_color, tone_color, local->compact_mode ? 12 : 18);
    footer_text = local->compact_mode ? egui_rgb_mix(local->muted_text_color, tone_color, 20) : tone_color;
    track_fill = egui_rgb_mix(local->section_color, local->border_color, 22);
    thumb_fill = egui_rgb_mix(local->surface_color, tone_color, local->compact_mode ? 24 : 32);
    thumb_border = egui_rgb_mix(local->border_color, tone_color, 28);

    if (local->read_only_mode)
    {
        card_fill = egui_rgb_mix(card_fill, local->surface_color, 24);
        card_border = egui_rgb_mix(card_border, local->muted_text_color, 20);
        shell_fill = egui_rgb_mix(shell_fill, local->surface_color, 18);
        shell_border = egui_rgb_mix(shell_border, local->muted_text_color, 18);
        badge_fill = egui_rgb_mix(badge_fill, local->surface_color, 24);
        badge_border = egui_rgb_mix(badge_border, local->muted_text_color, 20);
        badge_text = egui_rgb_mix(badge_text, local->muted_text_color, 24);
        title_color = egui_rgb_mix(title_color, local->muted_text_color, 12);
        summary_color = egui_rgb_mix(summary_color, local->muted_text_color, 18);
        footer_fill = egui_rgb_mix(footer_fill, local->surface_color, 24);
        footer_border = egui_rgb_mix(footer_border, local->muted_text_color, 20);
        footer_text = egui_rgb_mix(footer_text, local->muted_text_color, 24);
        track_fill = egui_rgb_mix(track_fill, local->muted_text_color, 18);
        thumb_fill = egui_rgb_mix(thumb_fill, local->surface_color, 18);
        thumb_border = egui_rgb_mix(thumb_border, local->muted_text_color, 18);
    }
    if (!egui_view_get_enable(self))
    {
        card_fill = virtualizing_stack_panel_mix_disabled(card_fill);
        card_border = virtualizing_stack_panel_mix_disabled(card_border);
        shell_fill = virtualizing_stack_panel_mix_disabled(shell_fill);
        shell_border = virtualizing_stack_panel_mix_disabled(shell_border);
        badge_fill = virtualizing_stack_panel_mix_disabled(badge_fill);
        badge_border = virtualizing_stack_panel_mix_disabled(badge_border);
        badge_text = virtualizing_stack_panel_mix_disabled(badge_text);
        title_color = virtualizing_stack_panel_mix_disabled(title_color);
        summary_color = virtualizing_stack_panel_mix_disabled(summary_color);
        footer_fill = virtualizing_stack_panel_mix_disabled(footer_fill);
        footer_border = virtualizing_stack_panel_mix_disabled(footer_border);
        footer_text = virtualizing_stack_panel_mix_disabled(footer_text);
        track_fill = virtualizing_stack_panel_mix_disabled(track_fill);
        thumb_fill = virtualizing_stack_panel_mix_disabled(thumb_fill);
        thumb_border = virtualizing_stack_panel_mix_disabled(thumb_border);
    }

    egui_canvas_draw_round_rectangle_fill(metrics.content_region.location.x - 2, metrics.content_region.location.y - 2, metrics.content_region.size.width + 4,
                                          metrics.content_region.size.height + 4, card_radius, card_fill, egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_round_rectangle(metrics.content_region.location.x - 2, metrics.content_region.location.y - 2, metrics.content_region.size.width + 4,
                                     metrics.content_region.size.height + 4, card_radius, 1, card_border, egui_color_alpha_mix(self->alpha, 54));

    if (virtualizing_stack_panel_region_has_size(&metrics.badge_region))
    {
        egui_canvas_draw_round_rectangle_fill(metrics.badge_region.location.x, metrics.badge_region.location.y, metrics.badge_region.size.width,
                                              metrics.badge_region.size.height, metrics.badge_region.size.height / 2, badge_fill,
                                              egui_color_alpha_mix(self->alpha, 96));
        egui_canvas_draw_round_rectangle(metrics.badge_region.location.x, metrics.badge_region.location.y, metrics.badge_region.size.width,
                                         metrics.badge_region.size.height, metrics.badge_region.size.height / 2, 1, badge_border,
                                         egui_color_alpha_mix(self->alpha, 28));
        virtualizing_stack_panel_draw_text(local->meta_font, self, snapshot->header, &metrics.badge_region, EGUI_ALIGN_CENTER, badge_text);
    }

    virtualizing_stack_panel_draw_text(local->font, self, snapshot->title, &metrics.title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);
    virtualizing_stack_panel_draw_text(local->meta_font, self, snapshot->summary, &metrics.summary_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, summary_color);

    if (virtualizing_stack_panel_region_has_size(&metrics.shell_region))
    {
        egui_canvas_draw_round_rectangle_fill(metrics.shell_region.location.x, metrics.shell_region.location.y, metrics.shell_region.size.width,
                                              metrics.shell_region.size.height, shell_radius, shell_fill, egui_color_alpha_mix(self->alpha, 94));
        egui_canvas_draw_round_rectangle(metrics.shell_region.location.x, metrics.shell_region.location.y, metrics.shell_region.size.width,
                                         metrics.shell_region.size.height, shell_radius, 1, shell_border, egui_color_alpha_mix(self->alpha, 26));
    }

    if (virtualizing_stack_panel_region_has_size(&metrics.track_region))
    {
        egui_canvas_draw_round_rectangle_fill(metrics.track_region.location.x, metrics.track_region.location.y, metrics.track_region.size.width,
                                              metrics.track_region.size.height, metrics.track_region.size.width / 2, track_fill,
                                              egui_color_alpha_mix(self->alpha, 70));
    }
    if (virtualizing_stack_panel_region_has_size(&metrics.thumb_region))
    {
        egui_canvas_draw_round_rectangle_fill(metrics.thumb_region.location.x, metrics.thumb_region.location.y, metrics.thumb_region.size.width,
                                              metrics.thumb_region.size.height, metrics.thumb_region.size.width / 2, thumb_fill,
                                              egui_color_alpha_mix(self->alpha, 96));
        egui_canvas_draw_round_rectangle(metrics.thumb_region.location.x, metrics.thumb_region.location.y, metrics.thumb_region.size.width,
                                         metrics.thumb_region.size.height, metrics.thumb_region.size.width / 2, 1, thumb_border,
                                         egui_color_alpha_mix(self->alpha, 34));
    }

    item_count = virtualizing_stack_panel_get_item_count(snapshot);
    for (item_index = 0; item_index < item_count; ++item_index)
    {
        const egui_view_virtualizing_stack_panel_item_t *item = virtualizing_stack_panel_get_item(snapshot, item_index);

        if (item == NULL || !virtualizing_stack_panel_region_has_size(&metrics.item_regions[item_index]))
        {
            continue;
        }
        virtualizing_stack_panel_draw_item(self, local, item, &metrics.item_regions[item_index], item_index);
    }

    if (virtualizing_stack_panel_region_has_size(&metrics.footer_region))
    {
        egui_canvas_draw_round_rectangle_fill(metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                              metrics.footer_region.size.height, metrics.footer_region.size.height / 2, footer_fill,
                                              egui_color_alpha_mix(self->alpha, 96));
        egui_canvas_draw_round_rectangle(metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                         metrics.footer_region.size.height, metrics.footer_region.size.height / 2, 1, footer_border,
                                         egui_color_alpha_mix(self->alpha, 34));
        virtualizing_stack_panel_draw_text(local->meta_font, self, snapshot->footer, &metrics.footer_region, EGUI_ALIGN_CENTER, footer_text);
    }
}

void egui_view_virtualizing_stack_panel_set_snapshots(egui_view_t *self, const egui_view_virtualizing_stack_panel_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_virtualizing_stack_panel_t);

    virtualizing_stack_panel_clear_pressed_state(self, local);
    local->snapshots = snapshots;
    local->snapshot_count = snapshots == NULL ? 0 : virtualizing_stack_panel_clamp_snapshot_count(snapshot_count);
    if (local->current_snapshot >= local->snapshot_count)
    {
        local->current_snapshot = 0;
    }
    local->current_item = virtualizing_stack_panel_resolve_default_item(virtualizing_stack_panel_get_snapshot(local));
    local->window_anchor = virtualizing_stack_panel_resolve_default_anchor(virtualizing_stack_panel_get_snapshot(local));
    virtualizing_stack_panel_sync_window_state(local, self);
    egui_view_invalidate(self);
}

void egui_view_virtualizing_stack_panel_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    virtualizing_stack_panel_set_current_snapshot_inner(self, snapshot_index, 1);
}

uint8_t egui_view_virtualizing_stack_panel_get_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_virtualizing_stack_panel_t);
    return local->current_snapshot;
}

void egui_view_virtualizing_stack_panel_set_current_item(egui_view_t *self, uint8_t item_index)
{
    virtualizing_stack_panel_set_current_item_inner(self, item_index, 1);
}

uint8_t egui_view_virtualizing_stack_panel_get_current_item(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_virtualizing_stack_panel_t);

    virtualizing_stack_panel_sync_window_state(local, self);
    return local->current_item;
}

void egui_view_virtualizing_stack_panel_set_window_anchor(egui_view_t *self, uint8_t item_index)
{
    virtualizing_stack_panel_set_window_anchor_inner(self, item_index, 1);
}

uint8_t egui_view_virtualizing_stack_panel_get_window_anchor(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_virtualizing_stack_panel_t);

    virtualizing_stack_panel_sync_window_state(local, self);
    return local->window_anchor;
}

uint8_t egui_view_virtualizing_stack_panel_activate_current_item(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_virtualizing_stack_panel_t);
    const egui_view_virtualizing_stack_panel_snapshot_t *snapshot = virtualizing_stack_panel_get_snapshot(local);

    virtualizing_stack_panel_sync_window_state(local, self);
    if (!virtualizing_stack_panel_item_is_interactive(local, self, snapshot, local->current_item))
    {
        return 0;
    }

    if (local->on_action != NULL)
    {
        local->on_action(self, local->current_snapshot, local->current_item);
    }
    egui_view_invalidate(self);
    return 1;
}

void egui_view_virtualizing_stack_panel_set_on_action_listener(egui_view_t *self, egui_view_on_virtualizing_stack_panel_action_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_virtualizing_stack_panel_t);
    local->on_action = listener;
}

void egui_view_virtualizing_stack_panel_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_virtualizing_stack_panel_t);

    local->font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    virtualizing_stack_panel_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_virtualizing_stack_panel_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_virtualizing_stack_panel_t);

    local->meta_font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    virtualizing_stack_panel_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_virtualizing_stack_panel_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_virtualizing_stack_panel_t);

    local->compact_mode = compact_mode ? 1 : 0;
    virtualizing_stack_panel_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_virtualizing_stack_panel_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_virtualizing_stack_panel_t);

    local->read_only_mode = read_only_mode ? 1 : 0;
    virtualizing_stack_panel_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_virtualizing_stack_panel_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                                                    egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                                    egui_color_t success_color, egui_color_t warning_color, egui_color_t neutral_color)
{
    EGUI_LOCAL_INIT(egui_view_virtualizing_stack_panel_t);

    local->surface_color = surface_color;
    local->section_color = section_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->neutral_color = neutral_color;
    virtualizing_stack_panel_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

uint8_t egui_view_virtualizing_stack_panel_get_item_region(egui_view_t *self, uint8_t item_index, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_virtualizing_stack_panel_t);
    const egui_view_virtualizing_stack_panel_snapshot_t *snapshot = virtualizing_stack_panel_get_snapshot(local);
    egui_view_virtualizing_stack_panel_metrics_t metrics;

    if (region == NULL || !virtualizing_stack_panel_item_exists(snapshot, item_index))
    {
        return 0;
    }

    virtualizing_stack_panel_sync_window_state(local, self);
    virtualizing_stack_panel_get_metrics(local, self, snapshot, local->window_anchor, &metrics);
    if (!virtualizing_stack_panel_region_has_size(&metrics.item_regions[item_index]))
    {
        return 0;
    }
    *region = metrics.item_regions[item_index];
    return 1;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_virtualizing_stack_panel_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_virtualizing_stack_panel_t);
    const egui_view_virtualizing_stack_panel_snapshot_t *snapshot = virtualizing_stack_panel_get_snapshot(local);
    uint8_t hit_item;

    virtualizing_stack_panel_sync_window_state(local, self);
    if (snapshot == NULL || virtualizing_stack_panel_get_item_count(snapshot) == 0 || local->read_only_mode || !egui_view_get_enable(self))
    {
        if (virtualizing_stack_panel_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_item = virtualizing_stack_panel_hit_item(local, self, event->location.x, event->location.y);
        if (!virtualizing_stack_panel_item_is_interactive(local, self, snapshot, hit_item))
        {
            if (virtualizing_stack_panel_clear_pressed_state(self, local))
            {
                egui_view_invalidate(self);
            }
            return 0;
        }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        local->pressed_item = hit_item;
        egui_view_set_pressed(self, true);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_item == EGUI_VIEW_VIRTUALIZING_STACK_PANEL_ITEM_NONE)
        {
            return 0;
        }
        hit_item = virtualizing_stack_panel_hit_item(local, self, event->location.x, event->location.y);
        egui_view_set_pressed(self, hit_item == local->pressed_item);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
    {
        uint8_t handled = local->pressed_item != EGUI_VIEW_VIRTUALIZING_STACK_PANEL_ITEM_NONE ? 1 : 0;

        hit_item = virtualizing_stack_panel_hit_item(local, self, event->location.x, event->location.y);
        if (hit_item == local->pressed_item && virtualizing_stack_panel_item_is_interactive(local, self, snapshot, hit_item))
        {
            virtualizing_stack_panel_set_current_item_inner(self, hit_item, 0);
            egui_view_virtualizing_stack_panel_activate_current_item(self);
        }
        if (virtualizing_stack_panel_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return handled || hit_item != EGUI_VIEW_VIRTUALIZING_STACK_PANEL_ITEM_NONE;
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (virtualizing_stack_panel_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}

static int egui_view_virtualizing_stack_panel_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_virtualizing_stack_panel_t);
    EGUI_UNUSED(event);

    if (virtualizing_stack_panel_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_virtualizing_stack_panel_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_virtualizing_stack_panel_t);
    const egui_view_virtualizing_stack_panel_snapshot_t *snapshot = virtualizing_stack_panel_get_snapshot(local);
    egui_view_virtualizing_stack_panel_metrics_t metrics;
    uint8_t item_count;
    uint8_t next_item;
    uint8_t next_snapshot;
    uint8_t page_step;

    virtualizing_stack_panel_sync_window_state(local, self);
    if (snapshot == NULL || local->read_only_mode || !egui_view_get_enable(self))
    {
        if (virtualizing_stack_panel_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    item_count = virtualizing_stack_panel_get_item_count(snapshot);
    if (item_count == 0)
    {
        return 0;
    }

    if (event->key_code == EGUI_KEY_CODE_ENTER || event->key_code == EGUI_KEY_CODE_SPACE)
    {
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            if (!virtualizing_stack_panel_item_is_interactive(local, self, snapshot, local->current_item))
            {
                return 0;
            }
            local->pressed_item = local->current_item;
            egui_view_set_pressed(self, true);
            egui_view_invalidate(self);
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            uint8_t handled = 0;

            if (local->pressed_item != EGUI_VIEW_VIRTUALIZING_STACK_PANEL_ITEM_NONE && local->pressed_item == local->current_item &&
                virtualizing_stack_panel_item_is_interactive(local, self, snapshot, local->pressed_item))
            {
                handled = egui_view_virtualizing_stack_panel_activate_current_item(self);
            }
            if (virtualizing_stack_panel_clear_pressed_state(self, local))
            {
                egui_view_invalidate(self);
            }
            return handled;
        }
        return 0;
    }

    if (virtualizing_stack_panel_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    if (event->type != EGUI_KEY_EVENT_ACTION_UP)
    {
        return 0;
    }

    virtualizing_stack_panel_get_metrics(local, self, snapshot, local->window_anchor, &metrics);
    page_step = metrics.visible_count > 0 ? metrics.visible_count : 1;

    /* The current SDK key enum has no PageUp/PageDown, so Ctrl+Up/Ctrl+Down is used for page jumps. */
    if (event->is_ctrl && event->key_code == EGUI_KEY_CODE_UP)
    {
        next_item = local->current_item > page_step ? (local->current_item - page_step) : 0;
        virtualizing_stack_panel_set_current_item_inner(self, next_item, 0);
        return 1;
    }
    if (event->is_ctrl && event->key_code == EGUI_KEY_CODE_DOWN)
    {
        next_item = local->current_item + page_step < item_count ? (local->current_item + page_step) : (item_count - 1);
        virtualizing_stack_panel_set_current_item_inner(self, next_item, 0);
        return 1;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_UP:
    case EGUI_KEY_CODE_LEFT:
        next_item = local->current_item > 0 ? (local->current_item - 1) : 0;
        virtualizing_stack_panel_set_current_item_inner(self, next_item, 0);
        return 1;
    case EGUI_KEY_CODE_DOWN:
    case EGUI_KEY_CODE_RIGHT:
        next_item = local->current_item + 1 < item_count ? (local->current_item + 1) : (item_count - 1);
        virtualizing_stack_panel_set_current_item_inner(self, next_item, 0);
        return 1;
    case EGUI_KEY_CODE_HOME:
        virtualizing_stack_panel_set_current_item_inner(self, 0, 0);
        return 1;
    case EGUI_KEY_CODE_END:
        virtualizing_stack_panel_set_current_item_inner(self, item_count - 1, 0);
        return 1;
    case EGUI_KEY_CODE_TAB:
        next_item = (uint8_t)(local->current_item + 1);
        if (next_item < item_count)
        {
            virtualizing_stack_panel_set_current_item_inner(self, next_item, 0);
            return 1;
        }
        next_snapshot = (uint8_t)(local->current_snapshot + 1);
        if (next_snapshot >= local->snapshot_count)
        {
            next_snapshot = 0;
        }
        virtualizing_stack_panel_set_current_snapshot_inner(self, next_snapshot, 0);
        return 1;
    default:
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_virtualizing_stack_panel_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_virtualizing_stack_panel_t);
    EGUI_UNUSED(event);

    if (virtualizing_stack_panel_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_virtualizing_stack_panel_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_virtualizing_stack_panel_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_virtualizing_stack_panel_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_virtualizing_stack_panel_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_virtualizing_stack_panel_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_virtualizing_stack_panel_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_virtualizing_stack_panel_on_key_event,
#endif
};

void egui_view_virtualizing_stack_panel_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_virtualizing_stack_panel_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_virtualizing_stack_panel_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif

    local->snapshots = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->on_action = NULL;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->section_color = EGUI_COLOR_HEX(0xF6F8FA);
    local->border_color = EGUI_COLOR_HEX(0xD4DDE6);
    local->text_color = EGUI_COLOR_HEX(0x1B2834);
    local->muted_text_color = EGUI_COLOR_HEX(0x6D7C8A);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->success_color = EGUI_COLOR_HEX(0x127A43);
    local->warning_color = EGUI_COLOR_HEX(0xA15D00);
    local->neutral_color = EGUI_COLOR_HEX(0x7A8796);
    local->snapshot_count = 0;
    local->current_snapshot = 0;
    local->current_item = EGUI_VIEW_VIRTUALIZING_STACK_PANEL_ITEM_NONE;
    local->window_anchor = 0;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_item = EGUI_VIEW_VIRTUALIZING_STACK_PANEL_ITEM_NONE;

    egui_view_set_view_name(self, "egui_view_virtualizing_stack_panel");
}
