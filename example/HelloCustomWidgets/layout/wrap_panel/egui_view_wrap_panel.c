#include "egui_view_wrap_panel.h"

#define EGUI_VIEW_WRAP_PANEL_STANDARD_RADIUS       10
#define EGUI_VIEW_WRAP_PANEL_STANDARD_OUTER_PAD_X  6
#define EGUI_VIEW_WRAP_PANEL_STANDARD_OUTER_PAD_Y  6
#define EGUI_VIEW_WRAP_PANEL_STANDARD_INNER_PAD_X  6
#define EGUI_VIEW_WRAP_PANEL_STANDARD_INNER_PAD_Y  5
#define EGUI_VIEW_WRAP_PANEL_STANDARD_BADGE_H      8
#define EGUI_VIEW_WRAP_PANEL_STANDARD_BADGE_GAP    3
#define EGUI_VIEW_WRAP_PANEL_STANDARD_TITLE_H      10
#define EGUI_VIEW_WRAP_PANEL_STANDARD_SUMMARY_H    8
#define EGUI_VIEW_WRAP_PANEL_STANDARD_TITLE_GAP    1
#define EGUI_VIEW_WRAP_PANEL_STANDARD_SHELL_GAP    4
#define EGUI_VIEW_WRAP_PANEL_STANDARD_SHELL_RADIUS 8
#define EGUI_VIEW_WRAP_PANEL_STANDARD_SHELL_PAD_X  4
#define EGUI_VIEW_WRAP_PANEL_STANDARD_SHELL_PAD_Y  4
#define EGUI_VIEW_WRAP_PANEL_STANDARD_FOOTER_H     8
#define EGUI_VIEW_WRAP_PANEL_STANDARD_FOOTER_GAP   3
#define EGUI_VIEW_WRAP_PANEL_STANDARD_ITEM_GAP_X   4
#define EGUI_VIEW_WRAP_PANEL_STANDARD_ITEM_GAP_Y   4
#define EGUI_VIEW_WRAP_PANEL_STANDARD_ITEM_H       18
#define EGUI_VIEW_WRAP_PANEL_STANDARD_ITEM_RADIUS  6

#define EGUI_VIEW_WRAP_PANEL_COMPACT_RADIUS       8
#define EGUI_VIEW_WRAP_PANEL_COMPACT_OUTER_PAD_X  5
#define EGUI_VIEW_WRAP_PANEL_COMPACT_OUTER_PAD_Y  5
#define EGUI_VIEW_WRAP_PANEL_COMPACT_INNER_PAD_X  5
#define EGUI_VIEW_WRAP_PANEL_COMPACT_INNER_PAD_Y  4
#define EGUI_VIEW_WRAP_PANEL_COMPACT_BADGE_H      7
#define EGUI_VIEW_WRAP_PANEL_COMPACT_BADGE_GAP    3
#define EGUI_VIEW_WRAP_PANEL_COMPACT_TITLE_H      9
#define EGUI_VIEW_WRAP_PANEL_COMPACT_SUMMARY_H    0
#define EGUI_VIEW_WRAP_PANEL_COMPACT_TITLE_GAP    0
#define EGUI_VIEW_WRAP_PANEL_COMPACT_SHELL_GAP    3
#define EGUI_VIEW_WRAP_PANEL_COMPACT_SHELL_RADIUS 6
#define EGUI_VIEW_WRAP_PANEL_COMPACT_SHELL_PAD_X  4
#define EGUI_VIEW_WRAP_PANEL_COMPACT_SHELL_PAD_Y  4
#define EGUI_VIEW_WRAP_PANEL_COMPACT_FOOTER_H     0
#define EGUI_VIEW_WRAP_PANEL_COMPACT_FOOTER_GAP   0
#define EGUI_VIEW_WRAP_PANEL_COMPACT_ITEM_GAP_X   3
#define EGUI_VIEW_WRAP_PANEL_COMPACT_ITEM_GAP_Y   3
#define EGUI_VIEW_WRAP_PANEL_COMPACT_ITEM_H       15
#define EGUI_VIEW_WRAP_PANEL_COMPACT_ITEM_RADIUS  5

typedef struct egui_view_wrap_panel_metrics egui_view_wrap_panel_metrics_t;
struct egui_view_wrap_panel_metrics
{
    egui_region_t content_region;
    egui_region_t badge_region;
    egui_region_t title_region;
    egui_region_t summary_region;
    egui_region_t shell_region;
    egui_region_t footer_region;
    egui_region_t item_regions[EGUI_VIEW_WRAP_PANEL_MAX_ITEMS];
};

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_wrap_panel_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event);
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_wrap_panel_on_static_key_event(egui_view_t *self, egui_key_event_t *event);
#endif

static void wrap_panel_reset_region(egui_region_t *region)
{
    region->location.x = 0;
    region->location.y = 0;
    region->size.width = 0;
    region->size.height = 0;
}

static uint8_t wrap_panel_region_has_size(const egui_region_t *region)
{
    return region != NULL && region->size.width > 0 && region->size.height > 0 ? 1 : 0;
}

static uint8_t wrap_panel_clamp_snapshot_count(uint8_t count)
{
    return count > EGUI_VIEW_WRAP_PANEL_MAX_SNAPSHOTS ? EGUI_VIEW_WRAP_PANEL_MAX_SNAPSHOTS : count;
}

static uint8_t wrap_panel_clamp_item_count(uint8_t count)
{
    return count > EGUI_VIEW_WRAP_PANEL_MAX_ITEMS ? EGUI_VIEW_WRAP_PANEL_MAX_ITEMS : count;
}

static uint8_t wrap_panel_text_len(const char *text)
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

static uint8_t wrap_panel_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static egui_color_t wrap_panel_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 68);
}

static const egui_view_wrap_panel_snapshot_t *wrap_panel_get_snapshot(egui_view_wrap_panel_t *local)
{
    if (local->snapshots == NULL || local->snapshot_count == 0 || local->current_snapshot >= local->snapshot_count)
    {
        return NULL;
    }
    return &local->snapshots[local->current_snapshot];
}

static uint8_t wrap_panel_get_item_count(const egui_view_wrap_panel_snapshot_t *snapshot)
{
    if (snapshot == NULL || snapshot->items == NULL)
    {
        return 0;
    }
    return wrap_panel_clamp_item_count(snapshot->item_count);
}

static const egui_view_wrap_panel_item_t *wrap_panel_get_item(const egui_view_wrap_panel_snapshot_t *snapshot, uint8_t item_index)
{
    if (item_index >= wrap_panel_get_item_count(snapshot))
    {
        return NULL;
    }
    return &snapshot->items[item_index];
}

static egui_color_t wrap_panel_tone_color(egui_view_wrap_panel_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_WRAP_PANEL_TONE_SUCCESS:
        return local->success_color;
    case EGUI_VIEW_WRAP_PANEL_TONE_WARNING:
        return local->warning_color;
    case EGUI_VIEW_WRAP_PANEL_TONE_NEUTRAL:
        return local->neutral_color;
    case EGUI_VIEW_WRAP_PANEL_TONE_ACCENT:
    default:
        return local->accent_color;
    }
}

static uint8_t wrap_panel_clear_pressed_state(egui_view_t *self, egui_view_wrap_panel_t *local)
{
    uint8_t had_pressed = self->is_pressed || local->pressed_item != EGUI_VIEW_WRAP_PANEL_ITEM_NONE;

    local->pressed_item = EGUI_VIEW_WRAP_PANEL_ITEM_NONE;
    egui_view_set_pressed(self, false);
    return had_pressed;
}

static uint8_t wrap_panel_item_exists(const egui_view_wrap_panel_snapshot_t *snapshot, uint8_t item_index)
{
    return item_index < wrap_panel_get_item_count(snapshot) ? 1 : 0;
}

static uint8_t wrap_panel_resolve_default_item(const egui_view_wrap_panel_snapshot_t *snapshot)
{
    uint8_t item_count = wrap_panel_get_item_count(snapshot);

    if (item_count == 0)
    {
        return EGUI_VIEW_WRAP_PANEL_ITEM_NONE;
    }
    return snapshot->selected_item < item_count ? snapshot->selected_item : 0;
}

static void wrap_panel_sync_current_item(egui_view_wrap_panel_t *local)
{
    const egui_view_wrap_panel_snapshot_t *snapshot = wrap_panel_get_snapshot(local);

    if (!wrap_panel_item_exists(snapshot, local->current_item))
    {
        local->current_item = wrap_panel_resolve_default_item(snapshot);
    }
}

static uint8_t wrap_panel_item_is_interactive(egui_view_wrap_panel_t *local, egui_view_t *self, const egui_view_wrap_panel_snapshot_t *snapshot,
                                              uint8_t item_index)
{
    if (snapshot == NULL || local->read_only_mode || !egui_view_get_enable(self))
    {
        return 0;
    }
    return wrap_panel_item_exists(snapshot, item_index);
}

static egui_dim_t wrap_panel_pill_width(const char *text, uint8_t compact_mode, egui_dim_t min_width, egui_dim_t max_width)
{
    egui_dim_t width = min_width;

    if (wrap_panel_has_text(text))
    {
        width += wrap_panel_text_len(text) * (compact_mode ? 4 : 5);
    }
    if (width > max_width)
    {
        width = max_width;
    }
    return width;
}

static void wrap_panel_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                 egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (font == NULL || !wrap_panel_has_text(text) || !wrap_panel_region_has_size(region))
    {
        return;
    }
    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, self->alpha);
}

static egui_dim_t wrap_panel_item_width(egui_view_wrap_panel_t *local, const egui_view_wrap_panel_item_t *item, egui_dim_t max_width)
{
    egui_dim_t width = local->compact_mode ? 14 : 16;
    egui_dim_t min_width = local->compact_mode ? 24 : 32;

    if (item == NULL)
    {
        return width;
    }

    if (wrap_panel_has_text(item->badge))
    {
        width += 4 + wrap_panel_text_len(item->badge) * 2;
    }
    if (wrap_panel_has_text(item->title))
    {
        width += 6 + wrap_panel_text_len(item->title) * 3;
    }
    if (wrap_panel_has_text(item->meta))
    {
        width += 4 + wrap_panel_text_len(item->meta) * 2;
    }

    switch (item->width_class)
    {
    case EGUI_VIEW_WRAP_PANEL_WIDTH_PROMINENT:
        width += local->compact_mode ? 8 : 12;
        break;
    case EGUI_VIEW_WRAP_PANEL_WIDTH_BALANCED:
        width += local->compact_mode ? 4 : 6;
        break;
    case EGUI_VIEW_WRAP_PANEL_WIDTH_COMPACT:
    default:
        break;
    }

    if (item->emphasized)
    {
        width += local->compact_mode ? 2 : 4;
    }

    if (width < min_width)
    {
        width = min_width;
    }
    if (width > max_width)
    {
        width = max_width;
    }
    return width;
}

static void wrap_panel_get_metrics(egui_view_wrap_panel_t *local, egui_view_t *self, const egui_view_wrap_panel_snapshot_t *snapshot,
                                   egui_view_wrap_panel_metrics_t *metrics)
{
    egui_region_t work_region;
    egui_dim_t outer_pad_x = local->compact_mode ? EGUI_VIEW_WRAP_PANEL_COMPACT_OUTER_PAD_X : EGUI_VIEW_WRAP_PANEL_STANDARD_OUTER_PAD_X;
    egui_dim_t outer_pad_y = local->compact_mode ? EGUI_VIEW_WRAP_PANEL_COMPACT_OUTER_PAD_Y : EGUI_VIEW_WRAP_PANEL_STANDARD_OUTER_PAD_Y;
    egui_dim_t inner_pad_x = local->compact_mode ? EGUI_VIEW_WRAP_PANEL_COMPACT_INNER_PAD_X : EGUI_VIEW_WRAP_PANEL_STANDARD_INNER_PAD_X;
    egui_dim_t inner_pad_y = local->compact_mode ? EGUI_VIEW_WRAP_PANEL_COMPACT_INNER_PAD_Y : EGUI_VIEW_WRAP_PANEL_STANDARD_INNER_PAD_Y;
    egui_dim_t badge_h = local->compact_mode ? EGUI_VIEW_WRAP_PANEL_COMPACT_BADGE_H : EGUI_VIEW_WRAP_PANEL_STANDARD_BADGE_H;
    egui_dim_t badge_gap = local->compact_mode ? EGUI_VIEW_WRAP_PANEL_COMPACT_BADGE_GAP : EGUI_VIEW_WRAP_PANEL_STANDARD_BADGE_GAP;
    egui_dim_t title_h = local->compact_mode ? EGUI_VIEW_WRAP_PANEL_COMPACT_TITLE_H : EGUI_VIEW_WRAP_PANEL_STANDARD_TITLE_H;
    egui_dim_t summary_h = local->compact_mode ? EGUI_VIEW_WRAP_PANEL_COMPACT_SUMMARY_H : EGUI_VIEW_WRAP_PANEL_STANDARD_SUMMARY_H;
    egui_dim_t title_gap = local->compact_mode ? EGUI_VIEW_WRAP_PANEL_COMPACT_TITLE_GAP : EGUI_VIEW_WRAP_PANEL_STANDARD_TITLE_GAP;
    egui_dim_t shell_gap = local->compact_mode ? EGUI_VIEW_WRAP_PANEL_COMPACT_SHELL_GAP : EGUI_VIEW_WRAP_PANEL_STANDARD_SHELL_GAP;
    egui_dim_t shell_pad_x = local->compact_mode ? EGUI_VIEW_WRAP_PANEL_COMPACT_SHELL_PAD_X : EGUI_VIEW_WRAP_PANEL_STANDARD_SHELL_PAD_X;
    egui_dim_t shell_pad_y = local->compact_mode ? EGUI_VIEW_WRAP_PANEL_COMPACT_SHELL_PAD_Y : EGUI_VIEW_WRAP_PANEL_STANDARD_SHELL_PAD_Y;
    egui_dim_t footer_h = local->compact_mode ? EGUI_VIEW_WRAP_PANEL_COMPACT_FOOTER_H : EGUI_VIEW_WRAP_PANEL_STANDARD_FOOTER_H;
    egui_dim_t footer_gap = local->compact_mode ? EGUI_VIEW_WRAP_PANEL_COMPACT_FOOTER_GAP : EGUI_VIEW_WRAP_PANEL_STANDARD_FOOTER_GAP;
    egui_dim_t item_gap_x = local->compact_mode ? EGUI_VIEW_WRAP_PANEL_COMPACT_ITEM_GAP_X : EGUI_VIEW_WRAP_PANEL_STANDARD_ITEM_GAP_X;
    egui_dim_t item_gap_y = local->compact_mode ? EGUI_VIEW_WRAP_PANEL_COMPACT_ITEM_GAP_Y : EGUI_VIEW_WRAP_PANEL_STANDARD_ITEM_GAP_Y;
    egui_dim_t item_h = local->compact_mode ? EGUI_VIEW_WRAP_PANEL_COMPACT_ITEM_H : EGUI_VIEW_WRAP_PANEL_STANDARD_ITEM_H;
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
    egui_dim_t row_start_x;
    egui_dim_t row_end_x;
    egui_dim_t item_w;
    egui_dim_t cursor_x;
    uint8_t item_count = wrap_panel_get_item_count(snapshot);
    uint8_t item_index;

    egui_view_get_work_region(self, &work_region);
    metrics->content_region.location.x = work_region.location.x + outer_pad_x;
    metrics->content_region.location.y = work_region.location.y + outer_pad_y;
    metrics->content_region.size.width = work_region.size.width - outer_pad_x * 2;
    metrics->content_region.size.height = work_region.size.height - outer_pad_y * 2;
    wrap_panel_reset_region(&metrics->badge_region);
    wrap_panel_reset_region(&metrics->title_region);
    wrap_panel_reset_region(&metrics->summary_region);
    wrap_panel_reset_region(&metrics->shell_region);
    wrap_panel_reset_region(&metrics->footer_region);
    for (item_index = 0; item_index < EGUI_VIEW_WRAP_PANEL_MAX_ITEMS; ++item_index)
    {
        wrap_panel_reset_region(&metrics->item_regions[item_index]);
    }

    if (!wrap_panel_region_has_size(&metrics->content_region) || snapshot == NULL)
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
    if (wrap_panel_has_text(snapshot->header))
    {
        metrics->badge_region.location.x = inner_x;
        metrics->badge_region.location.y = cursor_y;
        metrics->badge_region.size.width = wrap_panel_pill_width(snapshot->header, local->compact_mode, local->compact_mode ? 18 : 24, inner_w);
        metrics->badge_region.size.height = badge_h;
        cursor_y += badge_h + badge_gap;
    }

    metrics->title_region.location.x = inner_x;
    metrics->title_region.location.y = cursor_y;
    metrics->title_region.size.width = inner_w;
    metrics->title_region.size.height = title_h;
    cursor_y += title_h;

    if (summary_h > 0 && wrap_panel_has_text(snapshot->summary))
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
    if (footer_h > 0 && wrap_panel_has_text(snapshot->footer))
    {
        footer_y -= footer_h;
        metrics->footer_region.location.x = inner_x;
        metrics->footer_region.location.y = footer_y;
        metrics->footer_region.size.width = wrap_panel_pill_width(snapshot->footer, local->compact_mode, local->compact_mode ? 18 : 22, inner_w);
        metrics->footer_region.size.height = footer_h;
        footer_y -= footer_gap;
    }

    metrics->shell_region.location.x = inner_x;
    metrics->shell_region.location.y = cursor_y;
    metrics->shell_region.size.width = inner_w;
    metrics->shell_region.size.height = footer_y - cursor_y;
    if (!wrap_panel_region_has_size(&metrics->shell_region) || item_count == 0)
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

    row_start_x = items_x;
    row_end_x = items_x + items_w;
    cursor_x = row_start_x;
    cursor_y = items_y;
    for (item_index = 0; item_index < item_count; ++item_index)
    {
        const egui_view_wrap_panel_item_t *item = wrap_panel_get_item(snapshot, item_index);

        item_w = wrap_panel_item_width(local, item, items_w);
        if (cursor_x > row_start_x && cursor_x + item_w > row_end_x)
        {
            cursor_x = row_start_x;
            cursor_y += item_h + item_gap_y;
        }
        if (cursor_y + item_h > items_y + items_h)
        {
            break;
        }

        metrics->item_regions[item_index].location.x = cursor_x;
        metrics->item_regions[item_index].location.y = cursor_y;
        metrics->item_regions[item_index].size.width = item_w;
        metrics->item_regions[item_index].size.height = item_h;
        cursor_x += item_w + item_gap_x;
    }
}

static uint8_t wrap_panel_hit_item(egui_view_wrap_panel_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    egui_view_wrap_panel_metrics_t metrics;
    const egui_view_wrap_panel_snapshot_t *snapshot = wrap_panel_get_snapshot(local);
    uint8_t item_count = wrap_panel_get_item_count(snapshot);
    uint8_t item_index;

    wrap_panel_get_metrics(local, self, snapshot, &metrics);
    for (item_index = 0; item_index < item_count; ++item_index)
    {
        if (egui_region_pt_in_rect(&metrics.item_regions[item_index], x, y))
        {
            return item_index;
        }
    }
    return EGUI_VIEW_WRAP_PANEL_ITEM_NONE;
}

static uint8_t wrap_panel_find_vertical_neighbor(egui_view_wrap_panel_t *local, egui_view_t *self, uint8_t current_item, int8_t direction)
{
    egui_view_wrap_panel_metrics_t metrics;
    const egui_view_wrap_panel_snapshot_t *snapshot = wrap_panel_get_snapshot(local);
    egui_dim_t current_center_x;
    egui_dim_t current_center_y;
    egui_dim_t best_dy = 0;
    egui_dim_t best_dx = 0;
    uint8_t item_count = wrap_panel_get_item_count(snapshot);
    uint8_t best_item = current_item;
    uint8_t item_index;
    uint8_t found = 0;

    if (!wrap_panel_item_exists(snapshot, current_item))
    {
        return current_item;
    }

    wrap_panel_get_metrics(local, self, snapshot, &metrics);
    if (!wrap_panel_region_has_size(&metrics.item_regions[current_item]))
    {
        return current_item;
    }

    current_center_x = metrics.item_regions[current_item].location.x + metrics.item_regions[current_item].size.width / 2;
    current_center_y = metrics.item_regions[current_item].location.y + metrics.item_regions[current_item].size.height / 2;

    for (item_index = 0; item_index < item_count; ++item_index)
    {
        egui_dim_t candidate_center_x;
        egui_dim_t candidate_center_y;
        egui_dim_t dy;
        egui_dim_t dx;

        if (item_index == current_item || !wrap_panel_region_has_size(&metrics.item_regions[item_index]))
        {
            continue;
        }

        candidate_center_x = metrics.item_regions[item_index].location.x + metrics.item_regions[item_index].size.width / 2;
        candidate_center_y = metrics.item_regions[item_index].location.y + metrics.item_regions[item_index].size.height / 2;
        if (direction < 0)
        {
            if (candidate_center_y >= current_center_y)
            {
                continue;
            }
            dy = current_center_y - candidate_center_y;
        }
        else
        {
            if (candidate_center_y <= current_center_y)
            {
                continue;
            }
            dy = candidate_center_y - current_center_y;
        }
        dx = candidate_center_x >= current_center_x ? (candidate_center_x - current_center_x) : (current_center_x - candidate_center_x);

        if (!found || dy < best_dy || (dy == best_dy && dx < best_dx) || (dy == best_dy && dx == best_dx && item_index < best_item))
        {
            best_item = item_index;
            best_dy = dy;
            best_dx = dx;
            found = 1;
        }
    }

    return found ? best_item : current_item;
}

static void wrap_panel_set_current_snapshot_inner(egui_view_t *self, uint8_t snapshot_index, uint8_t invalidate_on_clear)
{
    EGUI_LOCAL_INIT(egui_view_wrap_panel_t);
    uint8_t had_pressed = wrap_panel_clear_pressed_state(self, local);
    uint8_t next_snapshot = 0;

    if (local->snapshot_count == 0)
    {
        local->current_snapshot = 0;
        local->current_item = EGUI_VIEW_WRAP_PANEL_ITEM_NONE;
        if (had_pressed && invalidate_on_clear)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    next_snapshot = snapshot_index >= local->snapshot_count ? (local->snapshot_count - 1) : snapshot_index;
    if (local->current_snapshot == next_snapshot)
    {
        if (had_pressed && invalidate_on_clear)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_snapshot = next_snapshot;
    local->current_item = wrap_panel_resolve_default_item(wrap_panel_get_snapshot(local));
    egui_view_invalidate(self);
}

static void wrap_panel_set_current_item_inner(egui_view_t *self, uint8_t item_index, uint8_t invalidate_on_clear)
{
    EGUI_LOCAL_INIT(egui_view_wrap_panel_t);
    const egui_view_wrap_panel_snapshot_t *snapshot = wrap_panel_get_snapshot(local);
    uint8_t had_pressed = wrap_panel_clear_pressed_state(self, local);

    if (!wrap_panel_item_exists(snapshot, item_index))
    {
        if (had_pressed && invalidate_on_clear)
        {
            egui_view_invalidate(self);
        }
        return;
    }
    if (local->current_item == item_index)
    {
        if (had_pressed && invalidate_on_clear)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_item = item_index;
    egui_view_invalidate(self);
}

static void wrap_panel_draw_item(egui_view_t *self, egui_view_wrap_panel_t *local, const egui_view_wrap_panel_item_t *item, const egui_region_t *region,
                                 uint8_t item_index)
{
    egui_color_t tone_color = wrap_panel_tone_color(local, item->tone);
    egui_color_t fill_color =
            egui_rgb_mix(local->surface_color, tone_color,
                         self->is_pressed && local->pressed_item == item_index ? 22 : (item_index == local->current_item ? 14 : (item->emphasized ? 10 : 5)));
    egui_color_t border_color =
            egui_rgb_mix(local->border_color, tone_color,
                         self->is_pressed && local->pressed_item == item_index ? 38 : (item_index == local->current_item ? 26 : 12));
    egui_color_t stripe_color = egui_rgb_mix(local->section_color, tone_color, item->emphasized ? 38 : 22);
    egui_color_t badge_fill = egui_rgb_mix(local->section_color, tone_color, item_index == local->current_item ? 32 : 18);
    egui_color_t badge_text = egui_rgb_mix(local->text_color, tone_color, 24);
    egui_color_t title_color = egui_rgb_mix(local->text_color, tone_color, item_index == local->current_item ? 14 : 6);
    egui_color_t meta_color = egui_rgb_mix(local->muted_text_color, tone_color, item->emphasized ? 20 : 8);
    egui_color_t focus_color = tone_color;
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_WRAP_PANEL_COMPACT_ITEM_RADIUS : EGUI_VIEW_WRAP_PANEL_STANDARD_ITEM_RADIUS;
    egui_dim_t pad_x = local->compact_mode ? 5 : 7;
    egui_dim_t badge_h = local->compact_mode ? 8 : 10;
    egui_dim_t title_h = local->compact_mode ? 8 : 10;
    egui_dim_t gap = local->compact_mode ? 4 : 5;
    egui_region_t stripe_region;
    egui_region_t badge_region;
    egui_region_t title_region;
    egui_region_t meta_region;
    egui_dim_t meta_w = 0;
    egui_dim_t badge_w = 0;
    egui_dim_t title_left;
    egui_dim_t title_right;

    if (!wrap_panel_region_has_size(region))
    {
        return;
    }

    if (local->read_only_mode)
    {
        fill_color = egui_rgb_mix(fill_color, local->surface_color, 22);
        border_color = egui_rgb_mix(border_color, local->muted_text_color, 18);
        stripe_color = egui_rgb_mix(stripe_color, local->muted_text_color, 26);
        badge_fill = egui_rgb_mix(badge_fill, local->surface_color, 20);
        badge_text = egui_rgb_mix(badge_text, local->muted_text_color, 16);
        title_color = egui_rgb_mix(title_color, local->muted_text_color, 16);
        meta_color = egui_rgb_mix(meta_color, local->muted_text_color, 18);
        focus_color = egui_rgb_mix(focus_color, local->muted_text_color, 16);
    }
    if (!egui_view_get_enable(self))
    {
        fill_color = wrap_panel_mix_disabled(fill_color);
        border_color = wrap_panel_mix_disabled(border_color);
        stripe_color = wrap_panel_mix_disabled(stripe_color);
        badge_fill = wrap_panel_mix_disabled(badge_fill);
        badge_text = wrap_panel_mix_disabled(badge_text);
        title_color = wrap_panel_mix_disabled(title_color);
        meta_color = wrap_panel_mix_disabled(meta_color);
        focus_color = wrap_panel_mix_disabled(focus_color);
    }

    egui_canvas_draw_round_rectangle_fill(region->location.x, region->location.y, region->size.width, region->size.height, radius, fill_color,
                                          egui_color_alpha_mix(self->alpha, 98));
    egui_canvas_draw_round_rectangle(region->location.x, region->location.y, region->size.width, region->size.height, radius, 1, border_color,
                                     egui_color_alpha_mix(self->alpha, 56));

    stripe_region.location.x = region->location.x + 1;
    stripe_region.location.y = region->location.y + 1;
    stripe_region.size.width = region->size.width - 2;
    stripe_region.size.height = local->compact_mode ? 2 : 3;
    if (wrap_panel_region_has_size(&stripe_region))
    {
        egui_canvas_draw_round_rectangle_fill(stripe_region.location.x, stripe_region.location.y, stripe_region.size.width, stripe_region.size.height, radius,
                                              stripe_color, egui_color_alpha_mix(self->alpha, item->emphasized ? 78 : 64));
    }

    wrap_panel_reset_region(&badge_region);
    wrap_panel_reset_region(&title_region);
    wrap_panel_reset_region(&meta_region);

    if (wrap_panel_has_text(item->badge))
    {
        badge_w = wrap_panel_pill_width(item->badge, local->compact_mode, local->compact_mode ? 12 : 16, region->size.width / 2);
        badge_region.location.x = region->location.x + pad_x;
        badge_region.location.y = region->location.y + region->size.height / 2 - badge_h / 2;
        badge_region.size.width = badge_w;
        badge_region.size.height = badge_h;
        egui_canvas_draw_round_rectangle_fill(badge_region.location.x, badge_region.location.y, badge_region.size.width, badge_region.size.height,
                                              badge_region.size.height / 2, badge_fill, egui_color_alpha_mix(self->alpha, 98));
        wrap_panel_draw_text(local->meta_font, self, item->badge, &badge_region, EGUI_ALIGN_CENTER, badge_text);
    }

    if (wrap_panel_has_text(item->meta))
    {
        meta_w = 8 + wrap_panel_text_len(item->meta) * (local->compact_mode ? 3 : 4);
        if (meta_w > region->size.width / 2)
        {
            meta_w = region->size.width / 2;
        }
        meta_region.location.x = region->location.x + region->size.width - pad_x - meta_w;
        meta_region.location.y = region->location.y + region->size.height / 2 - title_h / 2;
        meta_region.size.width = meta_w;
        meta_region.size.height = title_h;
    }

    title_left = region->location.x + pad_x;
    if (wrap_panel_region_has_size(&badge_region))
    {
        title_left = badge_region.location.x + badge_region.size.width + gap;
    }
    title_right = region->location.x + region->size.width - pad_x;
    if (wrap_panel_region_has_size(&meta_region))
    {
        title_right = meta_region.location.x - gap;
    }
    if (title_right > title_left)
    {
        title_region.location.x = title_left;
        title_region.location.y = region->location.y + region->size.height / 2 - title_h / 2;
        title_region.size.width = title_right - title_left;
        title_region.size.height = title_h;
    }

    wrap_panel_draw_text(local->font, self, item->title, &title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);
    wrap_panel_draw_text(local->meta_font, self, item->meta, &meta_region, EGUI_ALIGN_RIGHT | EGUI_ALIGN_VCENTER, meta_color);

    if (item_index == local->current_item && egui_view_get_enable(self) && !local->read_only_mode)
    {
        egui_canvas_draw_round_rectangle(region->location.x, region->location.y, region->size.width, region->size.height, radius, 1, focus_color,
                                         egui_color_alpha_mix(self->alpha, 82));
    }
}

static void egui_view_wrap_panel_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_wrap_panel_t);
    const egui_view_wrap_panel_snapshot_t *snapshot = wrap_panel_get_snapshot(local);
    egui_view_wrap_panel_metrics_t metrics;
    egui_color_t card_fill = local->surface_color;
    egui_color_t card_border = local->border_color;
    egui_color_t badge_fill = egui_rgb_mix(local->section_color, local->accent_color, 24);
    egui_color_t badge_text = egui_rgb_mix(local->text_color, local->accent_color, 26);
    egui_color_t title_color = local->text_color;
    egui_color_t summary_color = local->muted_text_color;
    egui_color_t shell_fill = egui_rgb_mix(local->section_color, local->accent_color, 8);
    egui_color_t shell_border = egui_rgb_mix(local->border_color, local->accent_color, 10);
    egui_color_t footer_fill = egui_rgb_mix(local->surface_color, local->accent_color, 16);
    egui_color_t footer_border = egui_rgb_mix(local->border_color, local->accent_color, 18);
    egui_color_t footer_text = egui_rgb_mix(local->muted_text_color, local->accent_color, 16);
    egui_color_t focus_color = local->accent_color;
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_WRAP_PANEL_COMPACT_RADIUS : EGUI_VIEW_WRAP_PANEL_STANDARD_RADIUS;
    egui_dim_t shell_radius = local->compact_mode ? EGUI_VIEW_WRAP_PANEL_COMPACT_SHELL_RADIUS : EGUI_VIEW_WRAP_PANEL_STANDARD_SHELL_RADIUS;
    uint8_t item_count;
    uint8_t item_index;

    wrap_panel_get_metrics(local, self, snapshot, &metrics);
    if (!wrap_panel_region_has_size(&metrics.content_region))
    {
        return;
    }

    if (local->read_only_mode)
    {
        badge_fill = egui_rgb_mix(badge_fill, local->surface_color, 24);
        badge_text = egui_rgb_mix(badge_text, local->muted_text_color, 18);
        title_color = egui_rgb_mix(title_color, local->muted_text_color, 12);
        summary_color = egui_rgb_mix(summary_color, local->muted_text_color, 18);
        shell_fill = egui_rgb_mix(shell_fill, local->surface_color, 22);
        shell_border = egui_rgb_mix(shell_border, local->muted_text_color, 16);
        footer_fill = egui_rgb_mix(footer_fill, local->surface_color, 18);
        footer_border = egui_rgb_mix(footer_border, local->muted_text_color, 16);
        footer_text = egui_rgb_mix(footer_text, local->muted_text_color, 18);
        focus_color = egui_rgb_mix(focus_color, local->muted_text_color, 16);
    }
    if (!egui_view_get_enable(self))
    {
        card_fill = wrap_panel_mix_disabled(card_fill);
        card_border = wrap_panel_mix_disabled(card_border);
        badge_fill = wrap_panel_mix_disabled(badge_fill);
        badge_text = wrap_panel_mix_disabled(badge_text);
        title_color = wrap_panel_mix_disabled(title_color);
        summary_color = wrap_panel_mix_disabled(summary_color);
        shell_fill = wrap_panel_mix_disabled(shell_fill);
        shell_border = wrap_panel_mix_disabled(shell_border);
        footer_fill = wrap_panel_mix_disabled(footer_fill);
        footer_border = wrap_panel_mix_disabled(footer_border);
        footer_text = wrap_panel_mix_disabled(footer_text);
        focus_color = wrap_panel_mix_disabled(focus_color);
    }

    egui_canvas_draw_round_rectangle_fill(metrics.content_region.location.x, metrics.content_region.location.y, metrics.content_region.size.width,
                                          metrics.content_region.size.height, radius, card_fill, egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_round_rectangle(metrics.content_region.location.x, metrics.content_region.location.y, metrics.content_region.size.width,
                                     metrics.content_region.size.height, radius, 1, card_border, egui_color_alpha_mix(self->alpha, 54));

    if (self->is_focused && egui_view_get_enable(self) && !local->read_only_mode)
    {
        egui_canvas_draw_round_rectangle(metrics.content_region.location.x, metrics.content_region.location.y, metrics.content_region.size.width,
                                         metrics.content_region.size.height, radius, 2, focus_color, egui_color_alpha_mix(self->alpha, 54));
    }

    if (snapshot != NULL && wrap_panel_region_has_size(&metrics.badge_region))
    {
        egui_canvas_draw_round_rectangle_fill(metrics.badge_region.location.x, metrics.badge_region.location.y, metrics.badge_region.size.width,
                                              metrics.badge_region.size.height, metrics.badge_region.size.height / 2, badge_fill,
                                              egui_color_alpha_mix(self->alpha, 98));
        wrap_panel_draw_text(local->meta_font, self, snapshot->header, &metrics.badge_region, EGUI_ALIGN_CENTER, badge_text);
    }

    if (snapshot != NULL)
    {
        wrap_panel_draw_text(local->font, self, snapshot->title, &metrics.title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);
        wrap_panel_draw_text(local->meta_font, self, snapshot->summary, &metrics.summary_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, summary_color);
    }

    if (wrap_panel_region_has_size(&metrics.shell_region))
    {
        egui_canvas_draw_round_rectangle_fill(metrics.shell_region.location.x, metrics.shell_region.location.y, metrics.shell_region.size.width,
                                              metrics.shell_region.size.height, shell_radius, shell_fill, egui_color_alpha_mix(self->alpha, 98));
        egui_canvas_draw_round_rectangle(metrics.shell_region.location.x, metrics.shell_region.location.y, metrics.shell_region.size.width,
                                         metrics.shell_region.size.height, shell_radius, 1, shell_border, egui_color_alpha_mix(self->alpha, 40));
    }

    item_count = wrap_panel_get_item_count(snapshot);
    for (item_index = 0; item_index < item_count; ++item_index)
    {
        const egui_view_wrap_panel_item_t *item = wrap_panel_get_item(snapshot, item_index);

        if (item == NULL || !wrap_panel_region_has_size(&metrics.item_regions[item_index]))
        {
            continue;
        }
        wrap_panel_draw_item(self, local, item, &metrics.item_regions[item_index], item_index);
    }

    if (snapshot != NULL && wrap_panel_region_has_size(&metrics.footer_region))
    {
        egui_canvas_draw_round_rectangle_fill(metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                              metrics.footer_region.size.height, metrics.footer_region.size.height / 2, footer_fill,
                                              egui_color_alpha_mix(self->alpha, 96));
        egui_canvas_draw_round_rectangle(metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                         metrics.footer_region.size.height, metrics.footer_region.size.height / 2, 1, footer_border,
                                         egui_color_alpha_mix(self->alpha, 36));
        wrap_panel_draw_text(local->meta_font, self, snapshot->footer, &metrics.footer_region, EGUI_ALIGN_CENTER, footer_text);
    }
}

void egui_view_wrap_panel_set_snapshots(egui_view_t *self, const egui_view_wrap_panel_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_wrap_panel_t);

    wrap_panel_clear_pressed_state(self, local);
    local->snapshots = snapshots;
    local->snapshot_count = snapshots == NULL ? 0 : wrap_panel_clamp_snapshot_count(snapshot_count);
    if (local->current_snapshot >= local->snapshot_count)
    {
        local->current_snapshot = 0;
    }
    local->current_item = wrap_panel_resolve_default_item(wrap_panel_get_snapshot(local));
    egui_view_invalidate(self);
}

void egui_view_wrap_panel_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    wrap_panel_set_current_snapshot_inner(self, snapshot_index, 1);
}

uint8_t egui_view_wrap_panel_get_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_wrap_panel_t);
    return local->current_snapshot;
}

void egui_view_wrap_panel_set_current_item(egui_view_t *self, uint8_t item_index)
{
    wrap_panel_set_current_item_inner(self, item_index, 1);
}

uint8_t egui_view_wrap_panel_get_current_item(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_wrap_panel_t);

    wrap_panel_sync_current_item(local);
    return local->current_item;
}

uint8_t egui_view_wrap_panel_activate_current_item(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_wrap_panel_t);
    const egui_view_wrap_panel_snapshot_t *snapshot = wrap_panel_get_snapshot(local);

    wrap_panel_sync_current_item(local);
    if (!wrap_panel_item_is_interactive(local, self, snapshot, local->current_item))
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

void egui_view_wrap_panel_set_on_action_listener(egui_view_t *self, egui_view_on_wrap_panel_action_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_wrap_panel_t);
    local->on_action = listener;
}

void egui_view_wrap_panel_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_wrap_panel_t);

    local->font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    wrap_panel_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_wrap_panel_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_wrap_panel_t);

    local->meta_font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    wrap_panel_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_wrap_panel_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_wrap_panel_t);

    local->compact_mode = compact_mode ? 1 : 0;
    wrap_panel_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_wrap_panel_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_wrap_panel_t);

    local->read_only_mode = read_only_mode ? 1 : 0;
    wrap_panel_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_wrap_panel_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                                      egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                      egui_color_t success_color, egui_color_t warning_color, egui_color_t neutral_color)
{
    EGUI_LOCAL_INIT(egui_view_wrap_panel_t);

    local->surface_color = surface_color;
    local->section_color = section_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->neutral_color = neutral_color;
    wrap_panel_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

uint8_t egui_view_wrap_panel_get_item_region(egui_view_t *self, uint8_t item_index, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_wrap_panel_t);
    const egui_view_wrap_panel_snapshot_t *snapshot = wrap_panel_get_snapshot(local);
    egui_view_wrap_panel_metrics_t metrics;

    if (region == NULL || !wrap_panel_item_exists(snapshot, item_index))
    {
        return 0;
    }

    wrap_panel_get_metrics(local, self, snapshot, &metrics);
    if (!wrap_panel_region_has_size(&metrics.item_regions[item_index]))
    {
        return 0;
    }
    *region = metrics.item_regions[item_index];
    return 1;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_wrap_panel_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_wrap_panel_t);
    const egui_view_wrap_panel_snapshot_t *snapshot = wrap_panel_get_snapshot(local);
    uint8_t hit_item;

    if (snapshot == NULL || wrap_panel_get_item_count(snapshot) == 0 || local->read_only_mode || !egui_view_get_enable(self))
    {
        if (wrap_panel_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_item = wrap_panel_hit_item(local, self, event->location.x, event->location.y);
        if (!wrap_panel_item_is_interactive(local, self, snapshot, hit_item))
        {
            if (wrap_panel_clear_pressed_state(self, local))
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
        if (local->pressed_item == EGUI_VIEW_WRAP_PANEL_ITEM_NONE)
        {
            return 0;
        }
        hit_item = wrap_panel_hit_item(local, self, event->location.x, event->location.y);
        egui_view_set_pressed(self, hit_item == local->pressed_item);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
    {
        uint8_t handled = local->pressed_item != EGUI_VIEW_WRAP_PANEL_ITEM_NONE ? 1 : 0;

        hit_item = wrap_panel_hit_item(local, self, event->location.x, event->location.y);
        if (hit_item == local->pressed_item && wrap_panel_item_is_interactive(local, self, snapshot, hit_item))
        {
            local->current_item = hit_item;
            egui_view_wrap_panel_activate_current_item(self);
        }
        if (wrap_panel_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return handled || hit_item != EGUI_VIEW_WRAP_PANEL_ITEM_NONE;
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (wrap_panel_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}

static int egui_view_wrap_panel_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_wrap_panel_t);
    EGUI_UNUSED(event);

    if (wrap_panel_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_wrap_panel_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_wrap_panel_t);
    const egui_view_wrap_panel_snapshot_t *snapshot = wrap_panel_get_snapshot(local);
    uint8_t item_count;
    uint8_t next_item;
    uint8_t next_snapshot;

    if (snapshot == NULL || local->read_only_mode || !egui_view_get_enable(self))
    {
        if (wrap_panel_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    item_count = wrap_panel_get_item_count(snapshot);
    if (item_count == 0)
    {
        return 0;
    }

    wrap_panel_sync_current_item(local);
    if (event->key_code == EGUI_KEY_CODE_ENTER || event->key_code == EGUI_KEY_CODE_SPACE)
    {
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            if (!wrap_panel_item_is_interactive(local, self, snapshot, local->current_item))
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

            if (local->pressed_item != EGUI_VIEW_WRAP_PANEL_ITEM_NONE && local->pressed_item == local->current_item &&
                wrap_panel_item_is_interactive(local, self, snapshot, local->pressed_item))
            {
                handled = egui_view_wrap_panel_activate_current_item(self);
            }
            if (wrap_panel_clear_pressed_state(self, local))
            {
                egui_view_invalidate(self);
            }
            return handled;
        }
        return 0;
    }

    if (wrap_panel_clear_pressed_state(self, local))
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
        next_item = local->current_item > 0 ? (local->current_item - 1) : 0;
        wrap_panel_set_current_item_inner(self, next_item, 0);
        return 1;
    case EGUI_KEY_CODE_RIGHT:
        next_item = local->current_item + 1 < item_count ? (local->current_item + 1) : (item_count - 1);
        wrap_panel_set_current_item_inner(self, next_item, 0);
        return 1;
    case EGUI_KEY_CODE_UP:
        next_item = wrap_panel_find_vertical_neighbor(local, self, local->current_item, -1);
        wrap_panel_set_current_item_inner(self, next_item, 0);
        return 1;
    case EGUI_KEY_CODE_DOWN:
        next_item = wrap_panel_find_vertical_neighbor(local, self, local->current_item, 1);
        wrap_panel_set_current_item_inner(self, next_item, 0);
        return 1;
    case EGUI_KEY_CODE_HOME:
        wrap_panel_set_current_item_inner(self, 0, 0);
        return 1;
    case EGUI_KEY_CODE_END:
        wrap_panel_set_current_item_inner(self, item_count - 1, 0);
        return 1;
    case EGUI_KEY_CODE_TAB:
        next_item = (uint8_t)(local->current_item + 1);
        if (next_item < item_count)
        {
            wrap_panel_set_current_item_inner(self, next_item, 0);
            return 1;
        }
        next_snapshot = (uint8_t)(local->current_snapshot + 1);
        if (next_snapshot >= local->snapshot_count)
        {
            next_snapshot = 0;
        }
        if (next_snapshot != local->current_snapshot)
        {
            local->current_snapshot = next_snapshot;
            local->current_item = wrap_panel_resolve_default_item(wrap_panel_get_snapshot(local));
            egui_view_invalidate(self);
        }
        return 1;
    default:
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_wrap_panel_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_wrap_panel_t);
    EGUI_UNUSED(event);

    if (wrap_panel_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_wrap_panel_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_wrap_panel_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_wrap_panel_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_wrap_panel_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_wrap_panel_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_wrap_panel_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_wrap_panel_on_key_event,
#endif
};

void egui_view_wrap_panel_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_wrap_panel_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_wrap_panel_t);
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
    local->current_item = EGUI_VIEW_WRAP_PANEL_ITEM_NONE;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_item = EGUI_VIEW_WRAP_PANEL_ITEM_NONE;

    egui_view_set_view_name(self, "egui_view_wrap_panel");
}
