#include "egui_view_relative_panel.h"

#define RP_STANDARD_RADIUS      10
#define RP_STANDARD_OUTER_PAD_X 7
#define RP_STANDARD_OUTER_PAD_Y 7
#define RP_STANDARD_INNER_PAD_X 8
#define RP_STANDARD_INNER_PAD_Y 7
#define RP_STANDARD_BADGE_H     8
#define RP_STANDARD_BADGE_GAP   4
#define RP_STANDARD_RULE_H      10
#define RP_STANDARD_TITLE_H     12
#define RP_STANDARD_TITLE_GAP   3
#define RP_STANDARD_SUMMARY_H   10
#define RP_STANDARD_BOARD_GAP   6
#define RP_STANDARD_BOARD_PAD   5
#define RP_STANDARD_FOOTER_H    9

#define RP_COMPACT_RADIUS      8
#define RP_COMPACT_OUTER_PAD_X 5
#define RP_COMPACT_OUTER_PAD_Y 5
#define RP_COMPACT_INNER_PAD_X 6
#define RP_COMPACT_INNER_PAD_Y 5
#define RP_COMPACT_BADGE_H     7
#define RP_COMPACT_BADGE_GAP   3
#define RP_COMPACT_RULE_H      8
#define RP_COMPACT_TITLE_H     10
#define RP_COMPACT_TITLE_GAP   0
#define RP_COMPACT_SUMMARY_H   0
#define RP_COMPACT_BOARD_GAP   4
#define RP_COMPACT_BOARD_PAD   4
#define RP_COMPACT_FOOTER_H    0

typedef struct egui_view_relative_panel_metrics egui_view_relative_panel_metrics_t;
struct egui_view_relative_panel_metrics
{
    egui_region_t content_region;
    egui_region_t badge_region;
    egui_region_t rule_region;
    egui_region_t title_region;
    egui_region_t summary_region;
    egui_region_t board_region;
    egui_region_t board_inner_region;
    egui_region_t footer_region;
    egui_region_t item_regions[EGUI_VIEW_RELATIVE_PANEL_MAX_ITEMS];
};

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_relative_panel_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event);
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_relative_panel_on_static_key_event(egui_view_t *self, egui_key_event_t *event);
#endif

static egui_view_relative_panel_t *relative_panel_local(egui_view_t *self)
{
    return (egui_view_relative_panel_t *)self;
}

static void relative_panel_reset_region(egui_region_t *region)
{
    region->location.x = 0;
    region->location.y = 0;
    region->size.width = 0;
    region->size.height = 0;
}

static uint8_t relative_panel_region_has_size(const egui_region_t *region)
{
    return region != NULL && region->size.width > 0 && region->size.height > 0 ? 1 : 0;
}

static uint8_t relative_panel_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static uint8_t relative_panel_text_len(const char *text)
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

static uint8_t relative_panel_clamp_snapshot_count(uint8_t count)
{
    return count > EGUI_VIEW_RELATIVE_PANEL_MAX_SNAPSHOTS ? EGUI_VIEW_RELATIVE_PANEL_MAX_SNAPSHOTS : count;
}

static uint8_t relative_panel_clamp_item_count(uint8_t count)
{
    return count > EGUI_VIEW_RELATIVE_PANEL_MAX_ITEMS ? EGUI_VIEW_RELATIVE_PANEL_MAX_ITEMS : count;
}

static egui_color_t relative_panel_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 68);
}

static egui_dim_t relative_panel_pill_width(const char *text, uint8_t compact_mode, egui_dim_t min_width, egui_dim_t max_width)
{
    egui_dim_t width = min_width;

    if (relative_panel_has_text(text))
    {
        width += relative_panel_text_len(text) * (compact_mode ? 4 : 5);
    }
    if (width > max_width)
    {
        width = max_width;
    }
    return width;
}

static const egui_view_relative_panel_snapshot_t *relative_panel_get_snapshot(egui_view_relative_panel_t *local)
{
    if (local->snapshots == NULL || local->snapshot_count == 0 || local->current_snapshot >= local->snapshot_count)
    {
        return NULL;
    }
    return &local->snapshots[local->current_snapshot];
}

static uint8_t relative_panel_get_item_count(const egui_view_relative_panel_snapshot_t *snapshot)
{
    if (snapshot == NULL || snapshot->items == NULL)
    {
        return 0;
    }
    return relative_panel_clamp_item_count(snapshot->item_count);
}

static uint8_t relative_panel_item_exists(const egui_view_relative_panel_snapshot_t *snapshot, uint8_t item_index)
{
    return item_index < relative_panel_get_item_count(snapshot) ? 1 : 0;
}

static const egui_view_relative_panel_item_t *relative_panel_get_item(const egui_view_relative_panel_snapshot_t *snapshot, uint8_t item_index)
{
    if (!relative_panel_item_exists(snapshot, item_index))
    {
        return NULL;
    }
    return &snapshot->items[item_index];
}

static uint8_t relative_panel_resolve_default_item(const egui_view_relative_panel_snapshot_t *snapshot)
{
    uint8_t item_count = relative_panel_get_item_count(snapshot);

    if (item_count == 0)
    {
        return EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE;
    }
    return snapshot->selected_item < item_count ? snapshot->selected_item : 0;
}

static void relative_panel_sync_current_item(egui_view_relative_panel_t *local)
{
    const egui_view_relative_panel_snapshot_t *snapshot = relative_panel_get_snapshot(local);

    if (!relative_panel_item_exists(snapshot, local->current_item))
    {
        local->current_item = relative_panel_resolve_default_item(snapshot);
    }
}

static uint8_t relative_panel_clear_pressed_state(egui_view_t *self, egui_view_relative_panel_t *local)
{
    uint8_t had_pressed = self->is_pressed || local->pressed_item != EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE || local->pressed_rule;

    local->pressed_item = EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE;
    local->pressed_rule = 0;
    egui_view_set_pressed(self, false);
    return had_pressed;
}

static const char *relative_panel_current_rule_text(egui_view_relative_panel_t *local)
{
    const egui_view_relative_panel_snapshot_t *snapshot = relative_panel_get_snapshot(local);
    const egui_view_relative_panel_item_t *item;

    if (snapshot == NULL)
    {
        return "";
    }

    relative_panel_sync_current_item(local);
    item = relative_panel_get_item(snapshot, local->current_item);
    if (item != NULL && relative_panel_has_text(item->rule))
    {
        return item->rule;
    }
    if (relative_panel_has_text(snapshot->footer))
    {
        return snapshot->footer;
    }
    return "";
}

static egui_color_t relative_panel_tone_color(egui_view_relative_panel_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_RELATIVE_PANEL_TONE_SUCCESS:
        return egui_rgb_mix(EGUI_COLOR_HEX(0x0F9D58), local->preview_color, 18);
    case EGUI_VIEW_RELATIVE_PANEL_TONE_WARNING:
        return egui_rgb_mix(EGUI_COLOR_HEX(0xF59E0B), local->accent_color, 10);
    case EGUI_VIEW_RELATIVE_PANEL_TONE_NEUTRAL:
        return egui_rgb_mix(local->border_color, local->preview_color, 18);
    case EGUI_VIEW_RELATIVE_PANEL_TONE_ACCENT:
    default:
        return local->accent_color;
    }
}

static void relative_panel_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                     egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (font == NULL || !relative_panel_has_text(text) || !relative_panel_region_has_size(region))
    {
        return;
    }
    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, self->alpha);
}

static void relative_panel_draw_focus(egui_view_t *self, const egui_region_t *region, egui_dim_t radius, egui_color_t color, egui_alpha_t alpha)
{
    egui_canvas_draw_round_rectangle(region->location.x - 1, region->location.y - 1, region->size.width + 2, region->size.height + 2, radius, 1, color,
                                     egui_color_alpha_mix(self->alpha, alpha));
}

static egui_dim_t relative_panel_resolve_layout_span(egui_dim_t defined_span, egui_dim_t fallback_span)
{
    if (defined_span > 0)
    {
        return defined_span;
    }
    return fallback_span > 0 ? fallback_span : 1;
}

static egui_dim_t relative_panel_scale_dim(egui_dim_t value, egui_dim_t source_span, egui_dim_t target_span)
{
    if (source_span <= 0 || target_span <= 0)
    {
        return value;
    }
    return (egui_dim_t)((value * target_span) / source_span);
}

static void relative_panel_get_metrics(egui_view_relative_panel_t *local, egui_view_t *self, const egui_view_relative_panel_snapshot_t *snapshot,
                                       egui_view_relative_panel_metrics_t *metrics)
{
    egui_region_t work_region;
    egui_dim_t outer_pad_x = local->compact_mode ? RP_COMPACT_OUTER_PAD_X : RP_STANDARD_OUTER_PAD_X;
    egui_dim_t outer_pad_y = local->compact_mode ? RP_COMPACT_OUTER_PAD_Y : RP_STANDARD_OUTER_PAD_Y;
    egui_dim_t inner_pad_x = local->compact_mode ? RP_COMPACT_INNER_PAD_X : RP_STANDARD_INNER_PAD_X;
    egui_dim_t inner_pad_y = local->compact_mode ? RP_COMPACT_INNER_PAD_Y : RP_STANDARD_INNER_PAD_Y;
    egui_dim_t badge_h = local->compact_mode ? RP_COMPACT_BADGE_H : RP_STANDARD_BADGE_H;
    egui_dim_t badge_gap = local->compact_mode ? RP_COMPACT_BADGE_GAP : RP_STANDARD_BADGE_GAP;
    egui_dim_t rule_h = local->compact_mode ? RP_COMPACT_RULE_H : RP_STANDARD_RULE_H;
    egui_dim_t title_h = local->compact_mode ? RP_COMPACT_TITLE_H : RP_STANDARD_TITLE_H;
    egui_dim_t title_gap = local->compact_mode ? RP_COMPACT_TITLE_GAP : RP_STANDARD_TITLE_GAP;
    egui_dim_t summary_h = local->compact_mode ? RP_COMPACT_SUMMARY_H : RP_STANDARD_SUMMARY_H;
    egui_dim_t board_gap = local->compact_mode ? RP_COMPACT_BOARD_GAP : RP_STANDARD_BOARD_GAP;
    egui_dim_t board_pad = local->compact_mode ? RP_COMPACT_BOARD_PAD : RP_STANDARD_BOARD_PAD;
    egui_dim_t footer_h = local->compact_mode ? RP_COMPACT_FOOTER_H : RP_STANDARD_FOOTER_H;
    egui_dim_t inner_x;
    egui_dim_t inner_y;
    egui_dim_t inner_w;
    egui_dim_t inner_h;
    egui_dim_t cursor_y;
    egui_dim_t top_row_h = 0;
    egui_dim_t board_bottom;
    egui_dim_t layout_width;
    egui_dim_t layout_height;
    uint8_t item_count = relative_panel_get_item_count(snapshot);
    uint8_t item_index;

    egui_view_get_work_region(self, &work_region);
    metrics->content_region.location.x = work_region.location.x + outer_pad_x;
    metrics->content_region.location.y = work_region.location.y + outer_pad_y;
    metrics->content_region.size.width = work_region.size.width - outer_pad_x * 2;
    metrics->content_region.size.height = work_region.size.height - outer_pad_y * 2;
    relative_panel_reset_region(&metrics->badge_region);
    relative_panel_reset_region(&metrics->rule_region);
    relative_panel_reset_region(&metrics->title_region);
    relative_panel_reset_region(&metrics->summary_region);
    relative_panel_reset_region(&metrics->board_region);
    relative_panel_reset_region(&metrics->board_inner_region);
    relative_panel_reset_region(&metrics->footer_region);
    for (item_index = 0; item_index < EGUI_VIEW_RELATIVE_PANEL_MAX_ITEMS; ++item_index)
    {
        relative_panel_reset_region(&metrics->item_regions[item_index]);
    }

    if (!relative_panel_region_has_size(&metrics->content_region) || snapshot == NULL)
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

    if (relative_panel_has_text(snapshot->header))
    {
        metrics->badge_region.location.x = inner_x;
        metrics->badge_region.location.y = inner_y;
        metrics->badge_region.size.width = relative_panel_pill_width(snapshot->header, local->compact_mode, local->compact_mode ? 18 : 24, inner_w / 2);
        metrics->badge_region.size.height = badge_h;
        top_row_h = badge_h;
    }

    if (relative_panel_has_text(relative_panel_current_rule_text(local)))
    {
        metrics->rule_region.size.width = relative_panel_pill_width(relative_panel_current_rule_text(local), local->compact_mode, local->compact_mode ? 22 : 28,
                                                                    inner_w - 12);
        metrics->rule_region.size.height = rule_h;
        metrics->rule_region.location.x = inner_x + inner_w - metrics->rule_region.size.width;
        metrics->rule_region.location.y = inner_y;
        if (top_row_h < rule_h)
        {
            top_row_h = rule_h;
        }
    }

    cursor_y = inner_y;
    if (top_row_h > 0)
    {
        cursor_y += top_row_h + badge_gap;
    }

    metrics->title_region.location.x = inner_x;
    metrics->title_region.location.y = cursor_y;
    metrics->title_region.size.width = inner_w;
    metrics->title_region.size.height = title_h;
    cursor_y += title_h;

    if (summary_h > 0 && relative_panel_has_text(snapshot->summary))
    {
        cursor_y += title_gap;
        metrics->summary_region.location.x = inner_x;
        metrics->summary_region.location.y = cursor_y;
        metrics->summary_region.size.width = inner_w;
        metrics->summary_region.size.height = summary_h;
        cursor_y += summary_h;
    }

    cursor_y += board_gap;
    board_bottom = inner_y + inner_h;
    if (footer_h > 0 && relative_panel_has_text(snapshot->footer))
    {
        metrics->footer_region.location.x = inner_x;
        metrics->footer_region.location.y = board_bottom - footer_h;
        metrics->footer_region.size.width = inner_w;
        metrics->footer_region.size.height = footer_h;
        board_bottom = metrics->footer_region.location.y - board_gap;
    }

    metrics->board_region.location.x = inner_x;
    metrics->board_region.location.y = cursor_y;
    metrics->board_region.size.width = inner_w;
    metrics->board_region.size.height = board_bottom - cursor_y;
    if (metrics->board_region.size.width <= 0 || metrics->board_region.size.height <= 0)
    {
        relative_panel_reset_region(&metrics->board_region);
        return;
    }

    metrics->board_inner_region.location.x = metrics->board_region.location.x + board_pad;
    metrics->board_inner_region.location.y = metrics->board_region.location.y + board_pad;
    metrics->board_inner_region.size.width = metrics->board_region.size.width - board_pad * 2;
    metrics->board_inner_region.size.height = metrics->board_region.size.height - board_pad * 2;
    if (metrics->board_inner_region.size.width <= 0 || metrics->board_inner_region.size.height <= 0)
    {
        relative_panel_reset_region(&metrics->board_inner_region);
        return;
    }

    layout_width = relative_panel_resolve_layout_span(snapshot->layout_width, metrics->board_inner_region.size.width);
    layout_height = relative_panel_resolve_layout_span(snapshot->layout_height, metrics->board_inner_region.size.height);
    for (item_index = 0; item_index < item_count; ++item_index)
    {
        const egui_view_relative_panel_item_t *item = relative_panel_get_item(snapshot, item_index);
        egui_region_t *region = &metrics->item_regions[item_index];
        egui_dim_t min_w = local->compact_mode ? 20 : 26;
        egui_dim_t min_h = local->compact_mode ? 14 : 20;

        if (item == NULL)
        {
            continue;
        }

        region->location.x = metrics->board_inner_region.location.x + relative_panel_scale_dim(item->origin_x, layout_width, metrics->board_inner_region.size.width);
        region->location.y = metrics->board_inner_region.location.y + relative_panel_scale_dim(item->origin_y, layout_height, metrics->board_inner_region.size.height);
        region->size.width = relative_panel_scale_dim(item->width, layout_width, metrics->board_inner_region.size.width);
        region->size.height = relative_panel_scale_dim(item->height, layout_height, metrics->board_inner_region.size.height);
        if (region->size.width < min_w)
        {
            region->size.width = min_w;
        }
        if (region->size.height < min_h)
        {
            region->size.height = min_h;
        }
        if (region->location.x + region->size.width > metrics->board_inner_region.location.x + metrics->board_inner_region.size.width)
        {
            region->location.x = metrics->board_inner_region.location.x + metrics->board_inner_region.size.width - region->size.width;
        }
        if (region->location.y + region->size.height > metrics->board_inner_region.location.y + metrics->board_inner_region.size.height)
        {
            region->location.y = metrics->board_inner_region.location.y + metrics->board_inner_region.size.height - region->size.height;
        }
        if (region->location.x < metrics->board_inner_region.location.x)
        {
            region->location.x = metrics->board_inner_region.location.x;
        }
        if (region->location.y < metrics->board_inner_region.location.y)
        {
            region->location.y = metrics->board_inner_region.location.y;
        }
    }
}

static uint8_t relative_panel_hit_item(egui_view_relative_panel_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    egui_view_relative_panel_metrics_t metrics;
    const egui_view_relative_panel_snapshot_t *snapshot = relative_panel_get_snapshot(local);
    uint8_t item_count = relative_panel_get_item_count(snapshot);
    uint8_t item_index;

    relative_panel_get_metrics(local, self, snapshot, &metrics);
    for (item_index = 0; item_index < item_count; ++item_index)
    {
        if (egui_region_pt_in_rect(&metrics.item_regions[item_index], x, y))
        {
            return item_index;
        }
    }
    return EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE;
}

static uint8_t relative_panel_find_neighbor(egui_view_relative_panel_t *local, egui_view_t *self, uint8_t current_item, int8_t horizontal, int8_t vertical)
{
    egui_view_relative_panel_metrics_t metrics;
    const egui_view_relative_panel_snapshot_t *snapshot = relative_panel_get_snapshot(local);
    uint8_t item_count = relative_panel_get_item_count(snapshot);
    egui_dim_t current_center_x;
    egui_dim_t current_center_y;
    uint32_t best_score = 0xFFFFFFFFu;
    uint8_t best_item = current_item;
    uint8_t item_index;

    if (!relative_panel_item_exists(snapshot, current_item))
    {
        return relative_panel_resolve_default_item(snapshot);
    }

    relative_panel_get_metrics(local, self, snapshot, &metrics);
    if (!relative_panel_region_has_size(&metrics.item_regions[current_item]))
    {
        return current_item;
    }
    current_center_x = metrics.item_regions[current_item].location.x + metrics.item_regions[current_item].size.width / 2;
    current_center_y = metrics.item_regions[current_item].location.y + metrics.item_regions[current_item].size.height / 2;

    for (item_index = 0; item_index < item_count; ++item_index)
    {
        egui_dim_t candidate_center_x;
        egui_dim_t candidate_center_y;
        egui_dim_t delta_x;
        egui_dim_t delta_y;
        egui_dim_t primary;
        egui_dim_t secondary;
        uint32_t score;

        if (item_index == current_item || !relative_panel_region_has_size(&metrics.item_regions[item_index]))
        {
            continue;
        }

        candidate_center_x = metrics.item_regions[item_index].location.x + metrics.item_regions[item_index].size.width / 2;
        candidate_center_y = metrics.item_regions[item_index].location.y + metrics.item_regions[item_index].size.height / 2;
        delta_x = candidate_center_x - current_center_x;
        delta_y = candidate_center_y - current_center_y;

        if (horizontal < 0 && delta_x >= 0)
        {
            continue;
        }
        if (horizontal > 0 && delta_x <= 0)
        {
            continue;
        }
        if (vertical < 0 && delta_y >= 0)
        {
            continue;
        }
        if (vertical > 0 && delta_y <= 0)
        {
            continue;
        }

        primary = horizontal != 0 ? (delta_x < 0 ? -delta_x : delta_x) : (delta_y < 0 ? -delta_y : delta_y);
        secondary = horizontal != 0 ? (delta_y < 0 ? -delta_y : delta_y) : (delta_x < 0 ? -delta_x : delta_x);
        score = (uint32_t)(secondary * 16 + primary);
        if (score < best_score)
        {
            best_score = score;
            best_item = item_index;
        }
    }

    return best_item;
}

static uint8_t relative_panel_set_current_item_inner(egui_view_t *self, uint8_t item_index, uint8_t focus_on_rule)
{
    egui_view_relative_panel_t *local = relative_panel_local(self);
    const egui_view_relative_panel_snapshot_t *snapshot = relative_panel_get_snapshot(local);
    uint8_t next_item = item_index;
    uint8_t had_pressed = relative_panel_clear_pressed_state(self, local);

    if (!relative_panel_item_exists(snapshot, next_item))
    {
        next_item = relative_panel_resolve_default_item(snapshot);
    }

    if (local->current_item == next_item && local->focus_on_rule == focus_on_rule)
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return relative_panel_item_exists(snapshot, next_item);
    }

    local->current_item = next_item;
    local->focus_on_rule = focus_on_rule ? 1 : 0;
    egui_view_invalidate(self);
    return relative_panel_item_exists(snapshot, next_item);
}

static void relative_panel_set_current_snapshot_inner(egui_view_t *self, uint8_t snapshot_index)
{
    egui_view_relative_panel_t *local = relative_panel_local(self);

    relative_panel_clear_pressed_state(self, local);
    if (local->snapshots == NULL || local->snapshot_count == 0)
    {
        local->current_snapshot = 0;
        local->current_item = EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE;
        local->focus_on_rule = 0;
        egui_view_invalidate(self);
        return;
    }

    if (snapshot_index >= local->snapshot_count)
    {
        snapshot_index = 0;
    }
    local->current_snapshot = snapshot_index;
    local->current_item = relative_panel_resolve_default_item(relative_panel_get_snapshot(local));
    local->focus_on_rule = 0;
    egui_view_invalidate(self);
}

static uint8_t relative_panel_cycle_snapshot(egui_view_t *self)
{
    egui_view_relative_panel_t *local = relative_panel_local(self);
    uint8_t next_snapshot;

    if (local->snapshot_count == 0)
    {
        return 0;
    }

    next_snapshot = (uint8_t)(local->current_snapshot + 1);
    if (next_snapshot >= local->snapshot_count)
    {
        next_snapshot = 0;
    }
    relative_panel_set_current_snapshot_inner(self, next_snapshot);
    return 1;
}

static void relative_panel_draw_item(egui_view_t *self, egui_view_relative_panel_t *local, const egui_view_relative_panel_item_t *item,
                                     const egui_region_t *region, uint8_t item_index)
{
    egui_color_t tone_color = relative_panel_tone_color(local, item->tone);
    egui_color_t fill_color = egui_rgb_mix(local->surface_color, tone_color, item->emphasized ? 18 : 10);
    egui_color_t border_color = egui_rgb_mix(local->border_color, tone_color, 26);
    egui_color_t accent_color = egui_rgb_mix(tone_color, EGUI_COLOR_WHITE, 16);
    egui_color_t title_color = local->text_color;
    egui_color_t meta_color = egui_rgb_mix(local->text_color, local->muted_text_color, 36);
    egui_region_t badge_region;
    egui_region_t title_region;
    egui_region_t meta_region;
    uint8_t active_item = local->current_item == item_index ? 1 : 0;
    uint8_t pressed_item = local->pressed_item == item_index && self->is_pressed ? 1 : 0;
    egui_dim_t radius = local->compact_mode ? 5 : 7;

    if (pressed_item)
    {
        fill_color = egui_rgb_mix(fill_color, tone_color, 22);
        border_color = egui_rgb_mix(border_color, tone_color, 44);
    }
    else if (active_item)
    {
        fill_color = egui_rgb_mix(fill_color, tone_color, 14);
        border_color = egui_rgb_mix(border_color, tone_color, 36);
    }

    if (!egui_view_get_enable(self) || local->read_only_mode)
    {
        fill_color = relative_panel_mix_disabled(fill_color);
        border_color = relative_panel_mix_disabled(border_color);
        title_color = relative_panel_mix_disabled(title_color);
        meta_color = relative_panel_mix_disabled(meta_color);
        accent_color = relative_panel_mix_disabled(accent_color);
    }

    egui_canvas_draw_round_rectangle_fill(region->location.x, region->location.y, region->size.width, region->size.height, radius, fill_color,
                                          egui_color_alpha_mix(self->alpha, 98));
    egui_canvas_draw_round_rectangle(region->location.x, region->location.y, region->size.width, region->size.height, radius, active_item ? 2 : 1,
                                     border_color, egui_color_alpha_mix(self->alpha, active_item ? 74 : 52));
    egui_canvas_draw_rectangle_fill(region->location.x + 2, region->location.y + 2, local->compact_mode ? 2 : 3, region->size.height - 4, accent_color,
                                    egui_color_alpha_mix(self->alpha, item->emphasized ? 88 : 64));

    badge_region.location.x = region->location.x + 7;
    badge_region.location.y = region->location.y + 5;
    badge_region.size.width = relative_panel_pill_width(item->badge, local->compact_mode, local->compact_mode ? 16 : 18,
                                                        region->size.width - 16);
    badge_region.size.height = local->compact_mode ? 7 : 8;
    if (relative_panel_has_text(item->badge) && badge_region.size.width > 0)
    {
        egui_canvas_draw_round_rectangle_fill(badge_region.location.x, badge_region.location.y, badge_region.size.width, badge_region.size.height,
                                              badge_region.size.height / 2, egui_rgb_mix(accent_color, EGUI_COLOR_WHITE, 12),
                                              egui_color_alpha_mix(self->alpha, 94));
        relative_panel_draw_text(local->meta_font, self, item->badge, &badge_region, EGUI_ALIGN_CENTER, EGUI_COLOR_HEX(0xFFFFFF));
    }

    title_region.location.x = region->location.x + 7;
    title_region.location.y = region->location.y + (local->compact_mode ? 16 : 18);
    title_region.size.width = region->size.width - 14;
    title_region.size.height = local->compact_mode ? 9 : 10;

    meta_region.location.x = region->location.x + 7;
    meta_region.location.y = region->location.y + region->size.height - (local->compact_mode ? 13 : 15);
    meta_region.size.width = region->size.width - 14;
    meta_region.size.height = local->compact_mode ? 8 : 9;

    relative_panel_draw_text(local->font, self, item->title, &title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);
    relative_panel_draw_text(local->meta_font, self, item->meta, &meta_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, meta_color);
}

static void relative_panel_draw_connectors(egui_view_t *self, egui_view_relative_panel_t *local, const egui_view_relative_panel_snapshot_t *snapshot,
                                           const egui_view_relative_panel_metrics_t *metrics)
{
    uint8_t item_count = relative_panel_get_item_count(snapshot);
    uint8_t item_index;

    for (item_index = 0; item_index < item_count; ++item_index)
    {
        const egui_view_relative_panel_item_t *item = relative_panel_get_item(snapshot, item_index);
        egui_color_t tone_color;
        egui_color_t connector_color;
        egui_region_t anchor_region;
        egui_region_t target_region;
        egui_dim_t start_x;
        egui_dim_t start_y;
        egui_dim_t end_x;
        egui_dim_t end_y;
        egui_dim_t mid_x;

        if (item == NULL || item->anchor_to == EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE || !relative_panel_item_exists(snapshot, item->anchor_to) ||
            !relative_panel_region_has_size(&metrics->item_regions[item_index]) || !relative_panel_region_has_size(&metrics->item_regions[item->anchor_to]))
        {
            continue;
        }

        anchor_region = metrics->item_regions[item->anchor_to];
        target_region = metrics->item_regions[item_index];
        tone_color = relative_panel_tone_color(local, item->tone);
        connector_color = egui_rgb_mix(local->border_color, tone_color, local->current_item == item_index ? 34 : 20);
        if (!egui_view_get_enable(self) || local->read_only_mode)
        {
            connector_color = relative_panel_mix_disabled(connector_color);
        }

        if (target_region.location.x >= anchor_region.location.x + anchor_region.size.width)
        {
            start_x = anchor_region.location.x + anchor_region.size.width;
            end_x = target_region.location.x;
            start_y = anchor_region.location.y + anchor_region.size.height / 2;
            end_y = target_region.location.y + target_region.size.height / 2;
        }
        else
        {
            start_x = anchor_region.location.x + anchor_region.size.width / 2;
            end_x = target_region.location.x + target_region.size.width / 2;
            start_y = anchor_region.location.y + anchor_region.size.height;
            end_y = target_region.location.y;
        }

        mid_x = (start_x + end_x) / 2;
        egui_canvas_draw_line(start_x, start_y, mid_x, start_y, 1, connector_color, egui_color_alpha_mix(self->alpha, 54));
        egui_canvas_draw_line(mid_x, start_y, mid_x, end_y, 1, connector_color, egui_color_alpha_mix(self->alpha, 54));
        egui_canvas_draw_line(mid_x, end_y, end_x, end_y, local->current_item == item_index ? 2 : 1, connector_color,
                              egui_color_alpha_mix(self->alpha, local->current_item == item_index ? 76 : 54));
    }
}

static void relative_panel_draw_board(egui_view_t *self, egui_view_relative_panel_t *local, const egui_view_relative_panel_snapshot_t *snapshot,
                                      const egui_view_relative_panel_metrics_t *metrics)
{
    const egui_region_t *prev_clip = egui_canvas_get_extra_clip();
    const egui_region_t *active_clip = NULL;
    egui_region_t screen_clip_region;
    egui_region_t clip_region;
    egui_color_t board_fill = egui_rgb_mix(local->section_color, local->surface_color, local->compact_mode ? 18 : 10);
    egui_color_t board_border = egui_rgb_mix(local->border_color, local->preview_color, 12);
    egui_color_t guide_color = egui_rgb_mix(local->border_color, local->preview_color, 18);
    uint8_t item_count = relative_panel_get_item_count(snapshot);
    uint8_t item_index;

    if (!egui_view_get_enable(self) || local->read_only_mode)
    {
        board_fill = relative_panel_mix_disabled(board_fill);
        board_border = relative_panel_mix_disabled(board_border);
        guide_color = relative_panel_mix_disabled(guide_color);
    }

    egui_canvas_draw_round_rectangle_fill(metrics->board_region.location.x, metrics->board_region.location.y, metrics->board_region.size.width,
                                          metrics->board_region.size.height, local->compact_mode ? 7 : 9, board_fill,
                                          egui_color_alpha_mix(self->alpha, 98));
    egui_canvas_draw_round_rectangle(metrics->board_region.location.x, metrics->board_region.location.y, metrics->board_region.size.width,
                                     metrics->board_region.size.height, local->compact_mode ? 7 : 9, 1, board_border,
                                     egui_color_alpha_mix(self->alpha, 56));

    egui_canvas_draw_line(metrics->board_inner_region.location.x, metrics->board_inner_region.location.y + metrics->board_inner_region.size.height / 2,
                          metrics->board_inner_region.location.x + metrics->board_inner_region.size.width,
                          metrics->board_inner_region.location.y + metrics->board_inner_region.size.height / 2, 1, guide_color,
                          egui_color_alpha_mix(self->alpha, 22));
    egui_canvas_draw_line(metrics->board_inner_region.location.x + metrics->board_inner_region.size.width / 2, metrics->board_inner_region.location.y,
                          metrics->board_inner_region.location.x + metrics->board_inner_region.size.width / 2,
                          metrics->board_inner_region.location.y + metrics->board_inner_region.size.height, 1, guide_color,
                          egui_color_alpha_mix(self->alpha, 18));

    screen_clip_region = metrics->board_inner_region;
    screen_clip_region.location.x += self->region_screen.location.x;
    screen_clip_region.location.y += self->region_screen.location.y;
    active_clip = &screen_clip_region;
    if (prev_clip != NULL)
    {
        egui_region_intersect(&screen_clip_region, prev_clip, &clip_region);
        active_clip = &clip_region;
    }
    egui_canvas_set_extra_clip(active_clip);

    relative_panel_draw_connectors(self, local, snapshot, metrics);
    for (item_index = 0; item_index < item_count; ++item_index)
    {
        const egui_view_relative_panel_item_t *item = relative_panel_get_item(snapshot, item_index);

        if (item == NULL || !relative_panel_region_has_size(&metrics->item_regions[item_index]))
        {
            continue;
        }
        relative_panel_draw_item(self, local, item, &metrics->item_regions[item_index], item_index);
    }
    if (item_count == 0)
    {
        relative_panel_draw_text(local->meta_font, self, "No layout", &metrics->board_inner_region, EGUI_ALIGN_CENTER, local->muted_text_color);
    }

    if (prev_clip != NULL)
    {
        egui_canvas_set_extra_clip(prev_clip);
    }
    else
    {
        egui_canvas_clear_extra_clip();
    }
}

static void egui_view_relative_panel_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_relative_panel_t);
    const egui_view_relative_panel_snapshot_t *snapshot = relative_panel_get_snapshot(local);
    egui_view_relative_panel_metrics_t metrics;
    egui_color_t card_fill = local->surface_color;
    egui_color_t card_border = local->border_color;
    egui_color_t title_color = local->text_color;
    egui_color_t summary_color = egui_rgb_mix(local->text_color, local->muted_text_color, 38);
    egui_color_t badge_fill = egui_rgb_mix(local->accent_color, EGUI_COLOR_WHITE, 18);
    egui_color_t badge_text = EGUI_COLOR_HEX(0xFFFFFF);
    egui_color_t rule_fill = egui_rgb_mix(local->preview_color, local->surface_color, 14);
    egui_color_t rule_border = egui_rgb_mix(local->preview_color, local->border_color, 32);
    egui_color_t rule_text = egui_rgb_mix(local->accent_color, local->text_color, 22);
    egui_color_t focus_color = egui_rgb_mix(local->accent_color, EGUI_COLOR_WHITE, 10);
    egui_color_t footer_color = egui_rgb_mix(local->text_color, local->muted_text_color, 46);

    relative_panel_sync_current_item(local);
    relative_panel_get_metrics(local, self, snapshot, &metrics);

    if (!egui_view_get_enable(self) || local->read_only_mode)
    {
        card_fill = relative_panel_mix_disabled(card_fill);
        card_border = relative_panel_mix_disabled(card_border);
        title_color = relative_panel_mix_disabled(title_color);
        summary_color = relative_panel_mix_disabled(summary_color);
        badge_fill = relative_panel_mix_disabled(badge_fill);
        badge_text = relative_panel_mix_disabled(badge_text);
        rule_fill = relative_panel_mix_disabled(rule_fill);
        rule_border = relative_panel_mix_disabled(rule_border);
        rule_text = relative_panel_mix_disabled(rule_text);
        focus_color = relative_panel_mix_disabled(focus_color);
        footer_color = relative_panel_mix_disabled(footer_color);
    }

    if (!relative_panel_region_has_size(&metrics.content_region))
    {
        return;
    }

    egui_canvas_draw_round_rectangle_fill(metrics.content_region.location.x, metrics.content_region.location.y, metrics.content_region.size.width,
                                          metrics.content_region.size.height, local->compact_mode ? RP_COMPACT_RADIUS : RP_STANDARD_RADIUS, card_fill,
                                          egui_color_alpha_mix(self->alpha, 98));
    egui_canvas_draw_round_rectangle(metrics.content_region.location.x, metrics.content_region.location.y, metrics.content_region.size.width,
                                     metrics.content_region.size.height, local->compact_mode ? RP_COMPACT_RADIUS : RP_STANDARD_RADIUS, 1, card_border,
                                     egui_color_alpha_mix(self->alpha, 58));

    if (relative_panel_region_has_size(&metrics.badge_region))
    {
        egui_canvas_draw_round_rectangle_fill(metrics.badge_region.location.x, metrics.badge_region.location.y, metrics.badge_region.size.width,
                                              metrics.badge_region.size.height, metrics.badge_region.size.height / 2, badge_fill,
                                              egui_color_alpha_mix(self->alpha, 96));
        relative_panel_draw_text(local->meta_font, self, snapshot->header, &metrics.badge_region, EGUI_ALIGN_CENTER, badge_text);
    }

    if (relative_panel_region_has_size(&metrics.rule_region))
    {
        egui_canvas_draw_round_rectangle_fill(metrics.rule_region.location.x, metrics.rule_region.location.y, metrics.rule_region.size.width,
                                              metrics.rule_region.size.height, metrics.rule_region.size.height / 2, rule_fill,
                                              egui_color_alpha_mix(self->alpha, 96));
        egui_canvas_draw_round_rectangle(metrics.rule_region.location.x, metrics.rule_region.location.y, metrics.rule_region.size.width,
                                         metrics.rule_region.size.height, metrics.rule_region.size.height / 2, local->focus_on_rule ? 2 : 1, rule_border,
                                         egui_color_alpha_mix(self->alpha, local->focus_on_rule ? 80 : 44));
        relative_panel_draw_text(local->meta_font, self, relative_panel_current_rule_text(local), &metrics.rule_region, EGUI_ALIGN_CENTER, rule_text);
    }

    if (snapshot != NULL)
    {
        relative_panel_draw_text(local->font, self, snapshot->title, &metrics.title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);
        relative_panel_draw_text(local->meta_font, self, snapshot->summary, &metrics.summary_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, summary_color);
    }

    if (relative_panel_region_has_size(&metrics.board_region))
    {
        relative_panel_draw_board(self, local, snapshot, &metrics);
    }

    if (relative_panel_region_has_size(&metrics.footer_region))
    {
        relative_panel_draw_text(local->meta_font, self, snapshot->footer, &metrics.footer_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, footer_color);
    }

    if (self->is_focused && egui_view_get_enable(self) && !local->read_only_mode && !local->compact_mode)
    {
        if (local->focus_on_rule && relative_panel_region_has_size(&metrics.rule_region))
        {
            relative_panel_draw_focus(self, &metrics.rule_region, metrics.rule_region.size.height / 2, focus_color, 62);
        }
        else if (relative_panel_item_exists(snapshot, local->current_item) && relative_panel_region_has_size(&metrics.item_regions[local->current_item]))
        {
            relative_panel_draw_focus(self, &metrics.item_regions[local->current_item], 8, focus_color, 58);
        }
    }
}

void egui_view_relative_panel_set_snapshots(egui_view_t *self, const egui_view_relative_panel_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_relative_panel_t);

    relative_panel_clear_pressed_state(self, local);
    local->snapshots = snapshots;
    local->snapshot_count = snapshots == NULL ? 0 : relative_panel_clamp_snapshot_count(snapshot_count);
    local->current_snapshot = 0;
    local->current_item = relative_panel_resolve_default_item(relative_panel_get_snapshot(local));
    local->focus_on_rule = 0;
    egui_view_invalidate(self);
}

void egui_view_relative_panel_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    relative_panel_set_current_snapshot_inner(self, snapshot_index);
}

uint8_t egui_view_relative_panel_get_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_relative_panel_t);
    return local->current_snapshot;
}

void egui_view_relative_panel_set_current_item(egui_view_t *self, uint8_t item_index)
{
    relative_panel_set_current_item_inner(self, item_index, 0);
}

uint8_t egui_view_relative_panel_get_current_item(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_relative_panel_t);
    relative_panel_sync_current_item(local);
    return local->current_item;
}

uint8_t egui_view_relative_panel_activate_current_item(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_relative_panel_t);
    const egui_view_relative_panel_snapshot_t *snapshot = relative_panel_get_snapshot(local);

    relative_panel_sync_current_item(local);
    if (!relative_panel_item_exists(snapshot, local->current_item) || local->read_only_mode || !egui_view_get_enable(self))
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

void egui_view_relative_panel_set_on_action_listener(egui_view_t *self, egui_view_on_relative_panel_action_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_relative_panel_t);
    local->on_action = listener;
}

void egui_view_relative_panel_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_relative_panel_t);

    relative_panel_clear_pressed_state(self, local);
    local->font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_invalidate(self);
}

void egui_view_relative_panel_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_relative_panel_t);

    relative_panel_clear_pressed_state(self, local);
    local->meta_font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_invalidate(self);
}

void egui_view_relative_panel_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_relative_panel_t);
    uint8_t next_compact_mode = compact_mode ? 1 : 0;
    uint8_t had_pressed = relative_panel_clear_pressed_state(self, local);

    if (local->compact_mode == next_compact_mode)
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->compact_mode = next_compact_mode;
    local->focus_on_rule = 0;
    egui_view_invalidate(self);
}

void egui_view_relative_panel_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_relative_panel_t);
    uint8_t next_read_only_mode = read_only_mode ? 1 : 0;
    uint8_t had_pressed = relative_panel_clear_pressed_state(self, local);

    if (local->read_only_mode == next_read_only_mode)
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->read_only_mode = next_read_only_mode;
    local->focus_on_rule = 0;
    egui_view_invalidate(self);
}

void egui_view_relative_panel_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                                          egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                          egui_color_t preview_color)
{
    EGUI_LOCAL_INIT(egui_view_relative_panel_t);

    relative_panel_clear_pressed_state(self, local);
    local->surface_color = surface_color;
    local->section_color = section_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->preview_color = preview_color;
    egui_view_invalidate(self);
}

uint8_t egui_view_relative_panel_get_item_region(egui_view_t *self, uint8_t item_index, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_relative_panel_t);
    const egui_view_relative_panel_snapshot_t *snapshot = relative_panel_get_snapshot(local);
    egui_view_relative_panel_metrics_t metrics;

    if (region == NULL || !relative_panel_item_exists(snapshot, item_index))
    {
        return 0;
    }

    relative_panel_get_metrics(local, self, snapshot, &metrics);
    if (!relative_panel_region_has_size(&metrics.item_regions[item_index]))
    {
        return 0;
    }
    *region = metrics.item_regions[item_index];
    return 1;
}

uint8_t egui_view_relative_panel_handle_navigation_key(egui_view_t *self, uint8_t key_code)
{
    EGUI_LOCAL_INIT(egui_view_relative_panel_t);
    const egui_view_relative_panel_snapshot_t *snapshot = relative_panel_get_snapshot(local);
    uint8_t item_count;

    if (relative_panel_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }

    if (!egui_view_get_enable(self) || local->read_only_mode || local->compact_mode || snapshot == NULL)
    {
        return 0;
    }

    item_count = relative_panel_get_item_count(snapshot);
    if (item_count == 0)
    {
        return 0;
    }
    relative_panel_sync_current_item(local);

    switch (key_code)
    {
    case EGUI_KEY_CODE_LEFT:
        relative_panel_set_current_item_inner(self, relative_panel_find_neighbor(local, self, local->current_item, -1, 0), 0);
        return 1;
    case EGUI_KEY_CODE_RIGHT:
        relative_panel_set_current_item_inner(self, relative_panel_find_neighbor(local, self, local->current_item, 1, 0), 0);
        return 1;
    case EGUI_KEY_CODE_UP:
        relative_panel_set_current_item_inner(self, relative_panel_find_neighbor(local, self, local->current_item, 0, -1), 0);
        return 1;
    case EGUI_KEY_CODE_DOWN:
        relative_panel_set_current_item_inner(self, relative_panel_find_neighbor(local, self, local->current_item, 0, 1), 0);
        return 1;
    case EGUI_KEY_CODE_HOME:
        relative_panel_set_current_item_inner(self, 0, 0);
        return 1;
    case EGUI_KEY_CODE_END:
        relative_panel_set_current_item_inner(self, (uint8_t)(item_count - 1), 0);
        return 1;
    case EGUI_KEY_CODE_TAB:
        local->focus_on_rule = local->focus_on_rule ? 0 : 1;
        egui_view_invalidate(self);
        return 1;
    case EGUI_KEY_CODE_ENTER:
    case EGUI_KEY_CODE_SPACE:
        if (local->focus_on_rule)
        {
            return relative_panel_cycle_snapshot(self);
        }
        return egui_view_relative_panel_activate_current_item(self);
    default:
        return 0;
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_relative_panel_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_relative_panel_t);
    const egui_view_relative_panel_snapshot_t *snapshot = relative_panel_get_snapshot(local);
    uint8_t hit_item;

    if (snapshot == NULL || relative_panel_get_item_count(snapshot) == 0 || local->read_only_mode || local->compact_mode || !egui_view_get_enable(self))
    {
        if (relative_panel_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_item = relative_panel_hit_item(local, self, event->location.x, event->location.y);
        if (!relative_panel_item_exists(snapshot, hit_item))
        {
            if (relative_panel_clear_pressed_state(self, local))
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
        local->pressed_rule = 0;
        local->focus_on_rule = 0;
        egui_view_set_pressed(self, true);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_item == EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE)
        {
            return 0;
        }
        hit_item = relative_panel_hit_item(local, self, event->location.x, event->location.y);
        egui_view_set_pressed(self, hit_item == local->pressed_item);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
    {
        uint8_t handled = local->pressed_item != EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE ? 1 : 0;

        hit_item = relative_panel_hit_item(local, self, event->location.x, event->location.y);
        if (hit_item == local->pressed_item && relative_panel_item_exists(snapshot, hit_item))
        {
            local->current_item = hit_item;
            local->focus_on_rule = 0;
            egui_view_relative_panel_activate_current_item(self);
        }
        if (relative_panel_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return handled || relative_panel_item_exists(snapshot, hit_item);
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (relative_panel_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}

static int egui_view_relative_panel_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_relative_panel_t);
    EGUI_UNUSED(event);

    if (relative_panel_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_relative_panel_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_relative_panel_t);
    const egui_view_relative_panel_snapshot_t *snapshot = relative_panel_get_snapshot(local);
    uint8_t item_count;

    if (snapshot == NULL || local->read_only_mode || local->compact_mode || !egui_view_get_enable(self))
    {
        if (relative_panel_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    item_count = relative_panel_get_item_count(snapshot);
    if (item_count == 0)
    {
        return 0;
    }
    relative_panel_sync_current_item(local);

    if (event->key_code == EGUI_KEY_CODE_ENTER || event->key_code == EGUI_KEY_CODE_SPACE)
    {
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            if (!local->focus_on_rule && !relative_panel_item_exists(snapshot, local->current_item))
            {
                return 0;
            }
            local->pressed_rule = local->focus_on_rule ? 1 : 0;
            local->pressed_item = local->focus_on_rule ? EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE : local->current_item;
            egui_view_set_pressed(self, true);
            egui_view_invalidate(self);
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            uint8_t handled = 0;

            if (local->pressed_rule && local->focus_on_rule)
            {
                handled = relative_panel_cycle_snapshot(self);
            }
            else if (local->pressed_item != EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE && local->pressed_item == local->current_item)
            {
                handled = egui_view_relative_panel_activate_current_item(self);
            }
            if (relative_panel_clear_pressed_state(self, local))
            {
                egui_view_invalidate(self);
            }
            return handled;
        }
        return 0;
    }

    if (relative_panel_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    if (event->type != EGUI_KEY_EVENT_ACTION_UP)
    {
        switch (event->key_code)
        {
        case EGUI_KEY_CODE_LEFT:
        case EGUI_KEY_CODE_RIGHT:
        case EGUI_KEY_CODE_UP:
        case EGUI_KEY_CODE_DOWN:
        case EGUI_KEY_CODE_HOME:
        case EGUI_KEY_CODE_END:
        case EGUI_KEY_CODE_TAB:
            return 1;
        default:
            return 0;
        }
    }

    if (egui_view_relative_panel_handle_navigation_key(self, event->key_code))
    {
        return 1;
    }
    return egui_view_on_key_event(self, event);
}

static int egui_view_relative_panel_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_relative_panel_t);
    EGUI_UNUSED(event);

    if (relative_panel_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static void egui_view_relative_panel_on_focus_change(egui_view_t *self, int is_focused)
{
    EGUI_LOCAL_INIT(egui_view_relative_panel_t);

    if (is_focused)
    {
        return;
    }
    if (relative_panel_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
}
#endif

void egui_view_relative_panel_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_relative_panel_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_relative_panel_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_relative_panel_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_relative_panel_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_relative_panel_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_relative_panel_on_key_event,
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        .on_focus_changed = egui_view_relative_panel_on_focus_change,
#endif
};

void egui_view_relative_panel_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_relative_panel_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_relative_panel_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif

    local->snapshots = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->on_action = NULL;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->section_color = EGUI_COLOR_HEX(0xF7F9FB);
    local->border_color = EGUI_COLOR_HEX(0xD5DEE6);
    local->text_color = EGUI_COLOR_HEX(0x1B2834);
    local->muted_text_color = EGUI_COLOR_HEX(0x6D7C8A);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->preview_color = EGUI_COLOR_HEX(0x6AA8FF);
    local->snapshot_count = 0;
    local->current_snapshot = 0;
    local->current_item = EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE;
    local->focus_on_rule = 0;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_item = EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE;
    local->pressed_rule = 0;

    egui_view_set_view_name(self, "egui_view_relative_panel");
}
