#include "egui_view_scroll_viewer.h"

#include "utils/egui_sprintf.h"

#define SV_STANDARD_RADIUS         10
#define SV_STANDARD_PAD_X          10
#define SV_STANDARD_PAD_Y          8
#define SV_STANDARD_EYEBROW_H      8
#define SV_STANDARD_EYEBROW_GAP    4
#define SV_STANDARD_TITLE_H        12
#define SV_STANDARD_TITLE_GAP      3
#define SV_STANDARD_SUMMARY_H      10
#define SV_STANDARD_SUMMARY_GAP    5
#define SV_STANDARD_BODY_GAP       6
#define SV_STANDARD_SCROLLBAR_W    14
#define SV_STANDARD_SCROLLBAR_GAP  8
#define SV_STANDARD_VIEWPORT_PAD   5
#define SV_STANDARD_FOOTER_H       10
#define SV_STANDARD_HELPER_H       10
#define SV_STANDARD_INDICATOR_W    46
#define SV_STANDARD_INDICATOR_H    6
#define SV_STANDARD_TRACK_PAD_Y    2
#define SV_STANDARD_THUMB_MIN_H    18

#define SV_COMPACT_RADIUS         8
#define SV_COMPACT_PAD_X          6
#define SV_COMPACT_PAD_Y          6
#define SV_COMPACT_TITLE_H        10
#define SV_COMPACT_TITLE_GAP      4
#define SV_COMPACT_BODY_GAP       4
#define SV_COMPACT_SCROLLBAR_W    12
#define SV_COMPACT_SCROLLBAR_GAP  6
#define SV_COMPACT_VIEWPORT_PAD   4
#define SV_COMPACT_FOOTER_H       8
#define SV_COMPACT_INDICATOR_W    30
#define SV_COMPACT_INDICATOR_H    5
#define SV_COMPACT_TRACK_PAD_Y    1
#define SV_COMPACT_THUMB_MIN_H    12

typedef struct egui_view_scroll_viewer_metrics egui_view_scroll_viewer_metrics_t;
struct egui_view_scroll_viewer_metrics
{
    egui_region_t content_region;
    egui_region_t eyebrow_region;
    egui_region_t title_region;
    egui_region_t summary_region;
    egui_region_t viewport_region;
    egui_region_t viewport_content_region;
    egui_region_t footer_region;
    egui_region_t helper_region;
    egui_region_t indicator_region;
    egui_region_t track_region;
    egui_region_t thumb_region;
    uint8_t show_eyebrow;
    uint8_t show_summary;
    uint8_t show_helper;
    uint8_t show_scrollbar;
};

static egui_view_scroll_viewer_t *scroll_viewer_local(egui_view_t *self)
{
    return (egui_view_scroll_viewer_t *)self;
}

static uint8_t scroll_viewer_has_text(const char *text)
{
    return (text != NULL && text[0] != '\0') ? 1 : 0;
}

static uint8_t scroll_viewer_clamp_snapshot_count(uint8_t count)
{
    return count > EGUI_VIEW_SCROLL_VIEWER_MAX_SNAPSHOTS ? EGUI_VIEW_SCROLL_VIEWER_MAX_SNAPSHOTS : count;
}

static uint8_t scroll_viewer_clamp_block_count(uint8_t count)
{
    return count > EGUI_VIEW_SCROLL_VIEWER_MAX_BLOCKS ? EGUI_VIEW_SCROLL_VIEWER_MAX_BLOCKS : count;
}

static egui_color_t scroll_viewer_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 68);
}

static const egui_view_scroll_viewer_snapshot_t *scroll_viewer_get_snapshot(egui_view_scroll_viewer_t *local)
{
    if (local->snapshots == NULL || local->snapshot_count == 0 || local->current_snapshot >= local->snapshot_count)
    {
        return NULL;
    }
    return &local->snapshots[local->current_snapshot];
}

static egui_dim_t scroll_viewer_snapshot_content_width(const egui_view_scroll_viewer_snapshot_t *snapshot)
{
    if (snapshot == NULL)
    {
        return 1;
    }
    if (snapshot->content_width < 1)
    {
        return snapshot->viewport_width > 0 ? snapshot->viewport_width : 1;
    }
    return snapshot->content_width;
}

static egui_dim_t scroll_viewer_snapshot_content_height(const egui_view_scroll_viewer_snapshot_t *snapshot)
{
    if (snapshot == NULL)
    {
        return 1;
    }
    if (snapshot->content_height < 1)
    {
        return snapshot->viewport_height > 0 ? snapshot->viewport_height : 1;
    }
    return snapshot->content_height;
}

static egui_dim_t scroll_viewer_snapshot_viewport_width(const egui_view_scroll_viewer_snapshot_t *snapshot)
{
    if (snapshot == NULL)
    {
        return 1;
    }
    if (snapshot->viewport_width < 1)
    {
        return snapshot->content_width > 0 ? snapshot->content_width : 1;
    }
    return snapshot->viewport_width;
}

static egui_dim_t scroll_viewer_snapshot_viewport_height(const egui_view_scroll_viewer_snapshot_t *snapshot)
{
    if (snapshot == NULL)
    {
        return 1;
    }
    if (snapshot->viewport_height < 1)
    {
        return snapshot->content_height > 0 ? snapshot->content_height : 1;
    }
    return snapshot->viewport_height;
}

static egui_dim_t scroll_viewer_get_max_vertical_offset_inner(egui_view_scroll_viewer_t *local)
{
    const egui_view_scroll_viewer_snapshot_t *snapshot = scroll_viewer_get_snapshot(local);
    egui_dim_t content_height = scroll_viewer_snapshot_content_height(snapshot);
    egui_dim_t viewport_height = scroll_viewer_snapshot_viewport_height(snapshot);

    return content_height > viewport_height ? (content_height - viewport_height) : 0;
}

static egui_dim_t scroll_viewer_get_max_horizontal_offset_inner(egui_view_scroll_viewer_t *local)
{
    const egui_view_scroll_viewer_snapshot_t *snapshot = scroll_viewer_get_snapshot(local);
    egui_dim_t content_width = scroll_viewer_snapshot_content_width(snapshot);
    egui_dim_t viewport_width = scroll_viewer_snapshot_viewport_width(snapshot);

    return content_width > viewport_width ? (content_width - viewport_width) : 0;
}

static egui_dim_t scroll_viewer_get_line_step_inner(egui_view_scroll_viewer_t *local)
{
    const egui_view_scroll_viewer_snapshot_t *snapshot = scroll_viewer_get_snapshot(local);
    egui_dim_t step = scroll_viewer_snapshot_viewport_height(snapshot) / 6;

    if (step < 10)
    {
        step = 10;
    }
    return step;
}

static egui_dim_t scroll_viewer_get_page_step_inner(egui_view_scroll_viewer_t *local)
{
    const egui_view_scroll_viewer_snapshot_t *snapshot = scroll_viewer_get_snapshot(local);
    egui_dim_t viewport_height = scroll_viewer_snapshot_viewport_height(snapshot);
    egui_dim_t step = viewport_height - viewport_height / 6;

    if (step < scroll_viewer_get_line_step_inner(local))
    {
        step = scroll_viewer_get_line_step_inner(local);
    }
    return step;
}

static egui_dim_t scroll_viewer_get_horizontal_step_inner(egui_view_scroll_viewer_t *local)
{
    const egui_view_scroll_viewer_snapshot_t *snapshot = scroll_viewer_get_snapshot(local);
    egui_dim_t step = scroll_viewer_snapshot_viewport_width(snapshot) / 6;

    if (step < 8)
    {
        step = 8;
    }
    return step;
}

static uint8_t scroll_viewer_clear_pressed_state(egui_view_t *self, egui_view_scroll_viewer_t *local)
{
    uint8_t had_pressed =
            self->is_pressed || local->pressed_part != EGUI_VIEW_SCROLL_VIEWER_PART_NONE || local->thumb_dragging || local->track_direction != 0;

    local->pressed_part = EGUI_VIEW_SCROLL_VIEWER_PART_NONE;
    local->thumb_dragging = 0;
    local->track_direction = 0;
    egui_view_set_pressed(self, false);
    return had_pressed;
}

static void scroll_viewer_normalize_state(egui_view_scroll_viewer_t *local)
{
    const egui_view_scroll_viewer_snapshot_t *snapshot = scroll_viewer_get_snapshot(local);
    egui_dim_t max_vertical;
    egui_dim_t max_horizontal;

    if (snapshot == NULL)
    {
        local->current_part = EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE;
        local->scrollbar_visibility = 1;
        local->vertical_offset = 0;
        local->horizontal_offset = 0;
        return;
    }

    max_vertical = scroll_viewer_get_max_vertical_offset_inner(local);
    max_horizontal = scroll_viewer_get_max_horizontal_offset_inner(local);

    if (local->vertical_offset < 0)
    {
        local->vertical_offset = 0;
    }
    else if (local->vertical_offset > max_vertical)
    {
        local->vertical_offset = max_vertical;
    }

    if (local->horizontal_offset < 0)
    {
        local->horizontal_offset = 0;
    }
    else if (local->horizontal_offset > max_horizontal)
    {
        local->horizontal_offset = max_horizontal;
    }

    if (local->current_part != EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE && local->current_part != EGUI_VIEW_SCROLL_VIEWER_PART_THUMB)
    {
        local->current_part = EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE;
    }
    if (local->pressed_part != EGUI_VIEW_SCROLL_VIEWER_PART_NONE && local->pressed_part != EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE &&
        local->pressed_part != EGUI_VIEW_SCROLL_VIEWER_PART_TRACK && local->pressed_part != EGUI_VIEW_SCROLL_VIEWER_PART_THUMB)
    {
        local->pressed_part = EGUI_VIEW_SCROLL_VIEWER_PART_NONE;
    }

    if (!local->scrollbar_visibility)
    {
        local->current_part = EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE;
        local->pressed_part = EGUI_VIEW_SCROLL_VIEWER_PART_NONE;
        local->thumb_dragging = 0;
        local->track_direction = 0;
    }
    if (local->compact_mode || local->read_only_mode)
    {
        local->pressed_part = EGUI_VIEW_SCROLL_VIEWER_PART_NONE;
        local->thumb_dragging = 0;
        local->track_direction = 0;
        if (local->current_part != EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE)
        {
            local->current_part = EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE;
        }
    }
}

static void scroll_viewer_load_current_snapshot(egui_view_scroll_viewer_t *local)
{
    const egui_view_scroll_viewer_snapshot_t *snapshot = scroll_viewer_get_snapshot(local);

    if (snapshot == NULL)
    {
        local->current_part = EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE;
        local->scrollbar_visibility = 1;
        local->vertical_offset = 0;
        local->horizontal_offset = 0;
        return;
    }

    local->current_part = EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE;
    local->scrollbar_visibility = snapshot->show_scrollbar ? 1 : 0;
    local->vertical_offset = snapshot->vertical_offset;
    local->horizontal_offset = snapshot->horizontal_offset;
    scroll_viewer_normalize_state(local);
}

static void scroll_viewer_notify_change(egui_view_t *self, egui_view_scroll_viewer_t *local, uint8_t part)
{
    if (local->on_view_changed != NULL)
    {
        local->on_view_changed(self, local->current_snapshot, local->vertical_offset, local->horizontal_offset, part);
    }
}

static void scroll_viewer_format_pair(egui_dim_t a, egui_dim_t b, char *buffer, int buffer_size)
{
    int pos = 0;

    pos += egui_sprintf_int(buffer, buffer_size, a);
    pos += egui_sprintf_str(&buffer[pos], buffer_size - pos, " / ");
    egui_sprintf_int(&buffer[pos], buffer_size - pos, b);
}

static void scroll_viewer_format_axes(egui_dim_t vertical_offset, egui_dim_t vertical_max, egui_dim_t horizontal_offset, egui_dim_t horizontal_max, char *buffer,
                                      int buffer_size)
{
    int pos = 0;

    pos += egui_sprintf_str(buffer, buffer_size, "V ");
    pos += egui_sprintf_int(&buffer[pos], buffer_size - pos, vertical_offset);
    pos += egui_sprintf_str(&buffer[pos], buffer_size - pos, " / ");
    pos += egui_sprintf_int(&buffer[pos], buffer_size - pos, vertical_max);
    pos += egui_sprintf_str(&buffer[pos], buffer_size - pos, "  H ");
    pos += egui_sprintf_int(&buffer[pos], buffer_size - pos, horizontal_offset);
    pos += egui_sprintf_str(&buffer[pos], buffer_size - pos, " / ");
    egui_sprintf_int(&buffer[pos], buffer_size - pos, horizontal_max);
}

static egui_color_t scroll_viewer_tone_color(egui_view_scroll_viewer_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_SCROLL_VIEWER_TONE_SUCCESS:
        return egui_rgb_mix(EGUI_COLOR_HEX(0x0F9D58), local->preview_color, 18);
    case EGUI_VIEW_SCROLL_VIEWER_TONE_WARNING:
        return egui_rgb_mix(EGUI_COLOR_HEX(0xF59E0B), local->accent_color, 10);
    case EGUI_VIEW_SCROLL_VIEWER_TONE_NEUTRAL:
        return egui_rgb_mix(local->border_color, local->preview_color, 18);
    default:
        return local->accent_color;
    }
}

static void scroll_viewer_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                    egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (!scroll_viewer_has_text(text))
    {
        return;
    }
    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, self->alpha);
}

static void scroll_viewer_draw_focus(egui_view_t *self, const egui_region_t *region, egui_dim_t radius, egui_color_t color, egui_alpha_t alpha)
{
    egui_canvas_draw_round_rectangle(region->location.x - 1, region->location.y - 1, region->size.width + 2, region->size.height + 2, radius, 1, color,
                                     egui_color_alpha_mix(self->alpha, alpha));
}

static void scroll_viewer_get_metrics(egui_view_scroll_viewer_t *local, egui_view_t *self, egui_view_scroll_viewer_metrics_t *metrics)
{
    const egui_view_scroll_viewer_snapshot_t *snapshot = scroll_viewer_get_snapshot(local);
    egui_region_t work_region;
    egui_dim_t pad_x = local->compact_mode ? SV_COMPACT_PAD_X : SV_STANDARD_PAD_X;
    egui_dim_t pad_y = local->compact_mode ? SV_COMPACT_PAD_Y : SV_STANDARD_PAD_Y;
    egui_dim_t title_h = local->compact_mode ? SV_COMPACT_TITLE_H : SV_STANDARD_TITLE_H;
    egui_dim_t title_gap = local->compact_mode ? SV_COMPACT_TITLE_GAP : SV_STANDARD_TITLE_GAP;
    egui_dim_t body_gap = local->compact_mode ? SV_COMPACT_BODY_GAP : SV_STANDARD_BODY_GAP;
    egui_dim_t footer_h = local->compact_mode ? SV_COMPACT_FOOTER_H : SV_STANDARD_FOOTER_H;
    egui_dim_t scrollbar_w = local->compact_mode ? SV_COMPACT_SCROLLBAR_W : SV_STANDARD_SCROLLBAR_W;
    egui_dim_t scrollbar_gap = local->compact_mode ? SV_COMPACT_SCROLLBAR_GAP : SV_STANDARD_SCROLLBAR_GAP;
    egui_dim_t viewport_pad = local->compact_mode ? SV_COMPACT_VIEWPORT_PAD : SV_STANDARD_VIEWPORT_PAD;
    egui_dim_t track_pad_y = local->compact_mode ? SV_COMPACT_TRACK_PAD_Y : SV_STANDARD_TRACK_PAD_Y;
    egui_dim_t thumb_min_h = local->compact_mode ? SV_COMPACT_THUMB_MIN_H : SV_STANDARD_THUMB_MIN_H;
    egui_dim_t cursor_y;
    egui_dim_t footer_y;
    egui_dim_t viewport_bottom;
    egui_dim_t content_height;
    egui_dim_t viewport_height;
    egui_dim_t max_vertical;
    egui_dim_t thumb_h;
    egui_dim_t thumb_travel;
    egui_dim_t thumb_pos = 0;

    egui_view_get_work_region(self, &work_region);

    metrics->content_region.location.x = work_region.location.x + pad_x;
    metrics->content_region.location.y = work_region.location.y + pad_y;
    metrics->content_region.size.width = work_region.size.width - pad_x * 2;
    metrics->content_region.size.height = work_region.size.height - pad_y * 2;
    if (metrics->content_region.size.width < 0)
    {
        metrics->content_region.size.width = 0;
    }
    if (metrics->content_region.size.height < 0)
    {
        metrics->content_region.size.height = 0;
    }

    metrics->show_eyebrow = (!local->compact_mode && snapshot != NULL && scroll_viewer_has_text(snapshot->eyebrow)) ? 1 : 0;
    metrics->show_summary = (!local->compact_mode && snapshot != NULL && scroll_viewer_has_text(snapshot->summary)) ? 1 : 0;
    metrics->show_helper = (!local->compact_mode && snapshot != NULL && scroll_viewer_has_text(snapshot->helper)) ? 1 : 0;
    metrics->show_scrollbar = local->scrollbar_visibility ? 1 : 0;

    cursor_y = metrics->content_region.location.y;

    if (metrics->show_eyebrow)
    {
        metrics->eyebrow_region.location.x = metrics->content_region.location.x;
        metrics->eyebrow_region.location.y = cursor_y;
        metrics->eyebrow_region.size.width = metrics->content_region.size.width;
        metrics->eyebrow_region.size.height = SV_STANDARD_EYEBROW_H;
        cursor_y += SV_STANDARD_EYEBROW_H + SV_STANDARD_EYEBROW_GAP;
    }
    else
    {
        egui_region_init_empty(&metrics->eyebrow_region);
    }

    metrics->title_region.location.x = metrics->content_region.location.x;
    metrics->title_region.location.y = cursor_y;
    metrics->title_region.size.width = metrics->content_region.size.width;
    metrics->title_region.size.height = title_h;
    cursor_y += title_h;

    if (metrics->show_summary)
    {
        cursor_y += title_gap;
        metrics->summary_region.location.x = metrics->content_region.location.x;
        metrics->summary_region.location.y = cursor_y;
        metrics->summary_region.size.width = metrics->content_region.size.width;
        metrics->summary_region.size.height = SV_STANDARD_SUMMARY_H;
        cursor_y += SV_STANDARD_SUMMARY_H;
    }
    else
    {
        egui_region_init_empty(&metrics->summary_region);
    }

    footer_y = metrics->content_region.location.y + metrics->content_region.size.height - footer_h;
    if (footer_y < cursor_y)
    {
        footer_y = cursor_y;
    }
    viewport_bottom = footer_y - body_gap;
    if (viewport_bottom < cursor_y)
    {
        viewport_bottom = cursor_y;
    }

    metrics->viewport_region.location.x = metrics->content_region.location.x;
    metrics->viewport_region.location.y = cursor_y + (cursor_y > metrics->content_region.location.y ? body_gap : 0);
    if (metrics->viewport_region.location.y > viewport_bottom)
    {
        metrics->viewport_region.location.y = cursor_y;
    }
    metrics->viewport_region.size.width = metrics->content_region.size.width - (metrics->show_scrollbar ? (scrollbar_w + scrollbar_gap) : 0);
    metrics->viewport_region.size.height = viewport_bottom - metrics->viewport_region.location.y;
    if (metrics->viewport_region.size.width < 24)
    {
        metrics->viewport_region.size.width = 24;
    }
    if (metrics->viewport_region.size.height < 20)
    {
        metrics->viewport_region.size.height = 20;
    }

    metrics->viewport_content_region.location.x = metrics->viewport_region.location.x + viewport_pad;
    metrics->viewport_content_region.location.y = metrics->viewport_region.location.y + viewport_pad;
    metrics->viewport_content_region.size.width = metrics->viewport_region.size.width - viewport_pad * 2;
    metrics->viewport_content_region.size.height = metrics->viewport_region.size.height - viewport_pad * 2;
    if (metrics->viewport_content_region.size.width < 8)
    {
        metrics->viewport_content_region.size.width = 8;
    }
    if (metrics->viewport_content_region.size.height < 8)
    {
        metrics->viewport_content_region.size.height = 8;
    }

    metrics->track_region.location.x = metrics->viewport_region.location.x + metrics->viewport_region.size.width + scrollbar_gap;
    metrics->track_region.location.y = metrics->viewport_region.location.y + track_pad_y;
    metrics->track_region.size.width = metrics->show_scrollbar ? scrollbar_w : 0;
    metrics->track_region.size.height = metrics->viewport_region.size.height - track_pad_y * 2;
    if (metrics->track_region.size.height < 0)
    {
        metrics->track_region.size.height = 0;
    }

    metrics->footer_region.location.x = metrics->content_region.location.x;
    metrics->footer_region.location.y = footer_y;
    metrics->footer_region.size.width = metrics->content_region.size.width / 2;
    metrics->footer_region.size.height = footer_h;

    metrics->helper_region.location.x = metrics->content_region.location.x + metrics->content_region.size.width / 2;
    metrics->helper_region.location.y = footer_y;
    metrics->helper_region.size.width = metrics->content_region.size.width / 2;
    metrics->helper_region.size.height = footer_h;

    metrics->indicator_region.size.width = local->compact_mode ? SV_COMPACT_INDICATOR_W : SV_STANDARD_INDICATOR_W;
    if (metrics->indicator_region.size.width > metrics->helper_region.size.width - 4)
    {
        metrics->indicator_region.size.width = metrics->helper_region.size.width - 4;
    }
    if (metrics->indicator_region.size.width < 12)
    {
        metrics->indicator_region.size.width = 12;
    }
    metrics->indicator_region.size.height = local->compact_mode ? SV_COMPACT_INDICATOR_H : SV_STANDARD_INDICATOR_H;
    metrics->indicator_region.location.x =
            metrics->helper_region.location.x + metrics->helper_region.size.width - metrics->indicator_region.size.width - 1;
    metrics->indicator_region.location.y =
            metrics->helper_region.location.y + (metrics->helper_region.size.height - metrics->indicator_region.size.height) / 2;

    content_height = scroll_viewer_snapshot_content_height(snapshot);
    viewport_height = scroll_viewer_snapshot_viewport_height(snapshot);
    max_vertical = scroll_viewer_get_max_vertical_offset_inner(local);

    if (!metrics->show_scrollbar || metrics->track_region.size.height <= 0)
    {
        egui_region_init_empty(&metrics->thumb_region);
        return;
    }

    thumb_h = (egui_dim_t)(((int32_t)metrics->track_region.size.height * viewport_height) / content_height);
    if (thumb_h < thumb_min_h)
    {
        thumb_h = thumb_min_h;
    }
    if (thumb_h > metrics->track_region.size.height)
    {
        thumb_h = metrics->track_region.size.height;
    }

    thumb_travel = metrics->track_region.size.height - thumb_h;
    if (thumb_travel > 0 && max_vertical > 0)
    {
        thumb_pos = (egui_dim_t)(((int32_t)local->vertical_offset * thumb_travel) / max_vertical);
    }

    metrics->thumb_region.location.x = metrics->track_region.location.x;
    metrics->thumb_region.location.y = metrics->track_region.location.y + thumb_pos;
    metrics->thumb_region.size.width = metrics->track_region.size.width;
    metrics->thumb_region.size.height = thumb_h;
}

static void scroll_viewer_apply_vertical_offset_inner(egui_view_t *self, egui_dim_t offset, uint8_t notify, uint8_t part)
{
    egui_view_scroll_viewer_t *local = scroll_viewer_local(self);
    egui_dim_t max_offset = scroll_viewer_get_max_vertical_offset_inner(local);

    if (offset < 0)
    {
        offset = 0;
    }
    else if (offset > max_offset)
    {
        offset = max_offset;
    }

    if (local->vertical_offset == offset)
    {
        return;
    }
    local->vertical_offset = offset;
    if (notify)
    {
        scroll_viewer_notify_change(self, local, part);
    }
    egui_view_invalidate(self);
}

static void scroll_viewer_apply_horizontal_offset_inner(egui_view_t *self, egui_dim_t offset, uint8_t notify, uint8_t part)
{
    egui_view_scroll_viewer_t *local = scroll_viewer_local(self);
    egui_dim_t max_offset = scroll_viewer_get_max_horizontal_offset_inner(local);

    if (offset < 0)
    {
        offset = 0;
    }
    else if (offset > max_offset)
    {
        offset = max_offset;
    }

    if (local->horizontal_offset == offset)
    {
        return;
    }
    local->horizontal_offset = offset;
    if (notify)
    {
        scroll_viewer_notify_change(self, local, part);
    }
    egui_view_invalidate(self);
}

static void scroll_viewer_draw_block(egui_view_t *self, egui_view_scroll_viewer_t *local, const egui_view_scroll_viewer_block_t *block,
                                     const egui_region_t *viewport_content_region, egui_color_t surface_color, egui_color_t border_color,
                                     egui_color_t text_color, egui_color_t muted_color)
{
    egui_region_t block_region;
    egui_region_t badge_region;
    egui_region_t title_region;
    egui_region_t meta_region;
    egui_color_t tone_color;
    egui_color_t fill_color;
    egui_color_t line_color;

    if (block == NULL)
    {
        return;
    }

    block_region.location.x = viewport_content_region->location.x + block->origin_x - local->horizontal_offset;
    block_region.location.y = viewport_content_region->location.y + block->origin_y - local->vertical_offset;
    block_region.size.width = block->width;
    block_region.size.height = block->height;

    if (block_region.size.width < 10 || block_region.size.height < 10)
    {
        return;
    }
    if (block_region.location.x + block_region.size.width < viewport_content_region->location.x ||
        block_region.location.x > viewport_content_region->location.x + viewport_content_region->size.width ||
        block_region.location.y + block_region.size.height < viewport_content_region->location.y ||
        block_region.location.y > viewport_content_region->location.y + viewport_content_region->size.height)
    {
        return;
    }

    tone_color = scroll_viewer_tone_color(local, block->tone);
    fill_color = egui_rgb_mix(surface_color, tone_color, block->emphasized ? 18 : 12);
    line_color = egui_rgb_mix(border_color, tone_color, 22);

    egui_canvas_draw_round_rectangle_fill(block_region.location.x, block_region.location.y, block_region.size.width, block_region.size.height, 7, fill_color,
                                          egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_round_rectangle(block_region.location.x, block_region.location.y, block_region.size.width, block_region.size.height, 7, 1, line_color,
                                     egui_color_alpha_mix(self->alpha, 74));

    badge_region.location.x = block_region.location.x + 6;
    badge_region.location.y = block_region.location.y + 5;
    badge_region.size.width = block_region.size.width - 12;
    badge_region.size.height = 8;

    title_region.location.x = block_region.location.x + 6;
    title_region.location.y = block_region.location.y + 18;
    title_region.size.width = block_region.size.width - 12;
    title_region.size.height = 10;

    meta_region.location.x = block_region.location.x + 6;
    meta_region.location.y = block_region.location.y + block_region.size.height - 16;
    meta_region.size.width = block_region.size.width - 12;
    meta_region.size.height = 10;

    if (scroll_viewer_has_text(block->badge))
    {
        egui_region_t pill_region = badge_region;

        pill_region.size.width = 22;
        if (pill_region.size.width > block_region.size.width - 12)
        {
            pill_region.size.width = block_region.size.width - 12;
        }
        egui_canvas_draw_round_rectangle_fill(pill_region.location.x, pill_region.location.y, pill_region.size.width, pill_region.size.height, 4,
                                              egui_rgb_mix(tone_color, EGUI_COLOR_WHITE, 20), egui_color_alpha_mix(self->alpha, 92));
        scroll_viewer_draw_text(local->meta_font, self, block->badge, &pill_region, EGUI_ALIGN_CENTER, EGUI_COLOR_HEX(0xFFFFFF));
    }

    scroll_viewer_draw_text(local->font, self, block->title, &title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text_color);
    scroll_viewer_draw_text(local->meta_font, self, block->meta, &meta_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                            egui_rgb_mix(text_color, muted_color, 34));
}

static void scroll_viewer_draw_surface(egui_view_t *self, egui_view_scroll_viewer_t *local, const egui_view_scroll_viewer_metrics_t *metrics,
                                       egui_color_t surface_color, egui_color_t border_color, egui_color_t viewport_color, egui_color_t text_color,
                                       egui_color_t muted_color, egui_color_t accent_color)
{
    const egui_view_scroll_viewer_snapshot_t *snapshot = scroll_viewer_get_snapshot(local);
    const egui_region_t *prev_clip = egui_canvas_get_extra_clip();
    const egui_region_t *active_clip = NULL;
    egui_region_t screen_clip_region;
    egui_region_t clip_region;
    egui_region_t view_pill_region = metrics->viewport_region;
    uint8_t focus_surface = (local->current_part == EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE && !local->read_only_mode && !local->compact_mode &&
                             egui_view_get_enable(self)) ?
                                    1 :
                                    0;
    uint8_t i;

    egui_canvas_draw_round_rectangle_fill(metrics->viewport_region.location.x, metrics->viewport_region.location.y, metrics->viewport_region.size.width,
                                          metrics->viewport_region.size.height, local->compact_mode ? 7 : 9, viewport_color,
                                          egui_color_alpha_mix(self->alpha, 94));
    egui_canvas_draw_round_rectangle(metrics->viewport_region.location.x, metrics->viewport_region.location.y, metrics->viewport_region.size.width,
                                     metrics->viewport_region.size.height, local->compact_mode ? 7 : 9, 1, border_color,
                                     egui_color_alpha_mix(self->alpha, 68));

    if (focus_surface)
    {
        scroll_viewer_draw_focus(self, &metrics->viewport_region, local->compact_mode ? 8 : 10, egui_rgb_mix(accent_color, EGUI_COLOR_WHITE, 10), 58);
    }

    view_pill_region.location.x += 6;
    view_pill_region.location.y += 5;
    view_pill_region.size.width = metrics->viewport_region.size.width / 2;
    view_pill_region.size.height = local->compact_mode ? 8 : 9;

    if (snapshot != NULL && !local->compact_mode)
    {
        scroll_viewer_draw_text(local->meta_font, self, snapshot->footer, &view_pill_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                                egui_rgb_mix(text_color, muted_color, 28));
    }

    screen_clip_region = metrics->viewport_content_region;
    screen_clip_region.location.x += self->region_screen.location.x;
    screen_clip_region.location.y += self->region_screen.location.y;
    active_clip = &screen_clip_region;
    if (prev_clip != NULL)
    {
        egui_region_intersect(&screen_clip_region, prev_clip, &clip_region);
        active_clip = &clip_region;
    }
    egui_canvas_set_extra_clip(active_clip);

    for (i = 0; snapshot != NULL && snapshot->blocks != NULL && i < scroll_viewer_clamp_block_count(snapshot->block_count); i++)
    {
        scroll_viewer_draw_block(self, local, &snapshot->blocks[i], &metrics->viewport_content_region, surface_color, border_color, text_color, muted_color);
    }

    if (snapshot == NULL || snapshot->blocks == NULL || scroll_viewer_clamp_block_count(snapshot->block_count) == 0)
    {
        egui_region_t empty_region = metrics->viewport_content_region;
        scroll_viewer_draw_text(local->meta_font, self, "No content", &empty_region, EGUI_ALIGN_CENTER, muted_color);
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

static void scroll_viewer_draw_indicator(egui_view_t *self, egui_view_scroll_viewer_t *local, const egui_view_scroll_viewer_metrics_t *metrics,
                                         egui_color_t border_color, egui_color_t accent_color, egui_color_t preview_color)
{
    egui_region_t thumb_region = metrics->indicator_region;
    egui_dim_t max_horizontal = scroll_viewer_get_max_horizontal_offset_inner(local);

    egui_canvas_draw_round_rectangle_fill(metrics->indicator_region.location.x, metrics->indicator_region.location.y, metrics->indicator_region.size.width,
                                          metrics->indicator_region.size.height, 4, egui_rgb_mix(preview_color, border_color, 18),
                                          egui_color_alpha_mix(self->alpha, 52));

    if (thumb_region.size.width <= 0)
    {
        return;
    }

    if (max_horizontal > 0)
    {
        egui_dim_t viewport_width = scroll_viewer_snapshot_viewport_width(scroll_viewer_get_snapshot(local));
        egui_dim_t content_width = scroll_viewer_snapshot_content_width(scroll_viewer_get_snapshot(local));
        egui_dim_t thumb_width = (egui_dim_t)(((int32_t)metrics->indicator_region.size.width * viewport_width) / content_width);
        egui_dim_t travel;

        if (thumb_width < 8)
        {
            thumb_width = 8;
        }
        if (thumb_width > metrics->indicator_region.size.width)
        {
            thumb_width = metrics->indicator_region.size.width;
        }
        travel = metrics->indicator_region.size.width - thumb_width;
        thumb_region.size.width = thumb_width;
        if (travel > 0)
        {
            thumb_region.location.x += (egui_dim_t)(((int32_t)local->horizontal_offset * travel) / max_horizontal);
        }
    }

    egui_canvas_draw_round_rectangle_fill(thumb_region.location.x, thumb_region.location.y, thumb_region.size.width, thumb_region.size.height, 4,
                                          egui_rgb_mix(accent_color, preview_color, 16), egui_color_alpha_mix(self->alpha, 84));
}

static void scroll_viewer_draw_track(egui_view_t *self, egui_view_scroll_viewer_t *local, const egui_view_scroll_viewer_metrics_t *metrics,
                                     egui_color_t surface_color, egui_color_t border_color, egui_color_t accent_color, egui_color_t preview_color)
{
    egui_color_t track_fill;
    egui_color_t thumb_fill;
    egui_color_t thumb_border;
    uint8_t thumb_focused;
    uint8_t thumb_pressed;

    if (!metrics->show_scrollbar || metrics->track_region.size.width <= 0 || metrics->track_region.size.height <= 0)
    {
        return;
    }

    track_fill = egui_rgb_mix(surface_color, border_color, 12);
    thumb_fill = scroll_viewer_get_max_vertical_offset_inner(local) > 0 ? accent_color : egui_rgb_mix(preview_color, border_color, 30);
    thumb_border = egui_rgb_mix(thumb_fill, EGUI_COLOR_WHITE, 10);
    thumb_focused =
            (local->current_part == EGUI_VIEW_SCROLL_VIEWER_PART_THUMB && !local->read_only_mode && !local->compact_mode && egui_view_get_enable(self)) ? 1 : 0;
    thumb_pressed = ((local->pressed_part == EGUI_VIEW_SCROLL_VIEWER_PART_THUMB && egui_view_get_pressed(self)) || local->thumb_dragging) ? 1 : 0;

    egui_canvas_draw_round_rectangle_fill(metrics->track_region.location.x, metrics->track_region.location.y, metrics->track_region.size.width,
                                          metrics->track_region.size.height, 6, track_fill, egui_color_alpha_mix(self->alpha, 86));
    egui_canvas_draw_round_rectangle(metrics->track_region.location.x, metrics->track_region.location.y, metrics->track_region.size.width,
                                     metrics->track_region.size.height, 6, 1, egui_rgb_mix(border_color, preview_color, 18),
                                     egui_color_alpha_mix(self->alpha, 50));

    if (local->pressed_part == EGUI_VIEW_SCROLL_VIEWER_PART_TRACK && local->track_direction != 0 && egui_view_get_pressed(self))
    {
        egui_region_t page_region = metrics->track_region;

        if (local->track_direction == 1)
        {
            page_region.size.height = metrics->thumb_region.location.y - page_region.location.y;
        }
        else
        {
            page_region.location.y = metrics->thumb_region.location.y + metrics->thumb_region.size.height;
            page_region.size.height = metrics->track_region.location.y + metrics->track_region.size.height - page_region.location.y;
        }

        if (page_region.size.height > 0)
        {
            egui_canvas_draw_round_rectangle_fill(page_region.location.x + 1, page_region.location.y, page_region.size.width - 2, page_region.size.height, 5,
                                                  egui_rgb_mix(preview_color, accent_color, 24), egui_color_alpha_mix(self->alpha, 30));
        }
    }

    if (metrics->thumb_region.size.height <= 0)
    {
        return;
    }
    if (thumb_focused)
    {
        scroll_viewer_draw_focus(self, &metrics->thumb_region, 6, egui_rgb_mix(accent_color, EGUI_COLOR_WHITE, 10), 60);
    }
    if (thumb_pressed)
    {
        thumb_fill = egui_rgb_mix(thumb_fill, EGUI_COLOR_WHITE, 8);
    }

    egui_canvas_draw_round_rectangle_fill(metrics->thumb_region.location.x, metrics->thumb_region.location.y, metrics->thumb_region.size.width,
                                          metrics->thumb_region.size.height, 6, thumb_fill, egui_color_alpha_mix(self->alpha, 94));
    egui_canvas_draw_round_rectangle(metrics->thumb_region.location.x, metrics->thumb_region.location.y, metrics->thumb_region.size.width,
                                     metrics->thumb_region.size.height, 6, 1, thumb_border, egui_color_alpha_mix(self->alpha, 80));
}

static void egui_view_scroll_viewer_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);
    const egui_view_scroll_viewer_snapshot_t *snapshot;
    egui_view_scroll_viewer_metrics_t metrics;
    egui_region_t region;
    egui_color_t surface_color = local->surface_color;
    egui_color_t border_color = local->border_color;
    egui_color_t viewport_color = local->viewport_color;
    egui_color_t text_color = local->text_color;
    egui_color_t muted_color = local->muted_text_color;
    egui_color_t accent_color = local->accent_color;
    egui_color_t preview_color = local->preview_color;
    egui_color_t shadow_color;
    char footer_text[40];
    uint8_t enabled = egui_view_get_enable(self) ? 1 : 0;

    scroll_viewer_normalize_state(local);
    snapshot = scroll_viewer_get_snapshot(local);
    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0)
    {
        return;
    }

    if (local->read_only_mode)
    {
        accent_color = egui_rgb_mix(accent_color, muted_color, 62);
        preview_color = egui_rgb_mix(preview_color, muted_color, 48);
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xFBFCFD), 30);
        border_color = egui_rgb_mix(border_color, muted_color, 26);
        viewport_color = egui_rgb_mix(viewport_color, EGUI_COLOR_HEX(0xFBFCFD), 26);
        text_color = egui_rgb_mix(text_color, muted_color, 32);
    }
    if (!enabled)
    {
        accent_color = scroll_viewer_mix_disabled(accent_color);
        preview_color = scroll_viewer_mix_disabled(preview_color);
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xFBFCFD), 34);
        border_color = scroll_viewer_mix_disabled(border_color);
        viewport_color = egui_rgb_mix(viewport_color, EGUI_COLOR_HEX(0xFBFCFD), 36);
        text_color = scroll_viewer_mix_disabled(text_color);
        muted_color = scroll_viewer_mix_disabled(muted_color);
    }

    scroll_viewer_get_metrics(local, self, &metrics);
    shadow_color = egui_rgb_mix(border_color, surface_color, 38);

    if (!local->compact_mode)
    {
        egui_canvas_draw_round_rectangle_fill(region.location.x, region.location.y + 2, region.size.width, region.size.height, SV_STANDARD_RADIUS + 1, shadow_color,
                                              egui_color_alpha_mix(self->alpha, enabled ? 16 : 10));
    }
    egui_canvas_draw_round_rectangle_fill(region.location.x, region.location.y, region.size.width, region.size.height,
                                          local->compact_mode ? SV_COMPACT_RADIUS : SV_STANDARD_RADIUS, surface_color,
                                          egui_color_alpha_mix(self->alpha, local->compact_mode ? 94 : 96));
    egui_canvas_draw_round_rectangle(region.location.x, region.location.y, region.size.width, region.size.height,
                                     local->compact_mode ? SV_COMPACT_RADIUS : SV_STANDARD_RADIUS, 1, border_color,
                                     egui_color_alpha_mix(self->alpha, local->compact_mode ? 56 : 60));

    if (metrics.show_eyebrow && snapshot != NULL)
    {
        scroll_viewer_draw_text(local->meta_font, self, snapshot->eyebrow, &metrics.eyebrow_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                                egui_rgb_mix(text_color, accent_color, 20));
    }
    if (snapshot != NULL)
    {
        scroll_viewer_draw_text(local->font, self, snapshot->title, &metrics.title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text_color);
        if (metrics.show_summary)
        {
            scroll_viewer_draw_text(local->meta_font, self, snapshot->summary, &metrics.summary_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                                    egui_rgb_mix(text_color, muted_color, 34));
        }
    }
    else
    {
        scroll_viewer_draw_text(local->font, self, "Scroll Viewer", &metrics.title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text_color);
    }

    scroll_viewer_draw_surface(self, local, &metrics, surface_color, border_color, viewport_color, text_color, muted_color, accent_color);
    scroll_viewer_draw_track(self, local, &metrics, surface_color, border_color, accent_color, preview_color);
    scroll_viewer_draw_indicator(self, local, &metrics, border_color, accent_color, preview_color);

    if (snapshot != NULL && scroll_viewer_has_text(snapshot->footer))
    {
        scroll_viewer_draw_text(local->meta_font, self, snapshot->footer, &metrics.footer_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                                egui_rgb_mix(text_color, muted_color, 32));
    }
    else
    {
        scroll_viewer_format_pair(scroll_viewer_snapshot_viewport_height(snapshot), scroll_viewer_snapshot_content_height(snapshot), footer_text, (int)sizeof(footer_text));
        scroll_viewer_draw_text(local->meta_font, self, footer_text, &metrics.footer_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                                egui_rgb_mix(text_color, muted_color, 32));
    }

    scroll_viewer_format_axes(local->vertical_offset, scroll_viewer_get_max_vertical_offset_inner(local), local->horizontal_offset,
                              scroll_viewer_get_max_horizontal_offset_inner(local), footer_text, (int)sizeof(footer_text));
    scroll_viewer_draw_text(local->meta_font, self, footer_text, &metrics.helper_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                            egui_rgb_mix(text_color, muted_color, local->compact_mode ? 26 : 30));

    if (metrics.show_helper && snapshot != NULL)
    {
        egui_region_t helper_region = metrics.helper_region;

        helper_region.size.width -= metrics.indicator_region.size.width + 6;
        if (helper_region.size.width > 8)
        {
            scroll_viewer_draw_text(local->meta_font, self, snapshot->helper, &helper_region, EGUI_ALIGN_RIGHT | EGUI_ALIGN_VCENTER,
                                    egui_rgb_mix(text_color, muted_color, 42));
        }
    }
}

static uint8_t scroll_viewer_hit_part(egui_view_scroll_viewer_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    egui_view_scroll_viewer_metrics_t metrics;
    egui_dim_t local_x = x - self->region_screen.location.x;
    egui_dim_t local_y = y - self->region_screen.location.y;

    scroll_viewer_get_metrics(local, self, &metrics);
    if (metrics.show_scrollbar && egui_region_pt_in_rect(&metrics.thumb_region, local_x, local_y))
    {
        return EGUI_VIEW_SCROLL_VIEWER_PART_THUMB;
    }
    if (metrics.show_scrollbar && egui_region_pt_in_rect(&metrics.track_region, local_x, local_y))
    {
        return EGUI_VIEW_SCROLL_VIEWER_PART_TRACK;
    }
    if (egui_region_pt_in_rect(&metrics.viewport_region, local_x, local_y))
    {
        return EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE;
    }
    return EGUI_VIEW_SCROLL_VIEWER_PART_NONE;
}

static uint8_t scroll_viewer_track_direction_from_point(egui_view_scroll_viewer_t *local, egui_view_t *self, egui_dim_t screen_y)
{
    egui_view_scroll_viewer_metrics_t metrics;
    egui_dim_t local_y = screen_y - self->region_screen.location.y;

    scroll_viewer_get_metrics(local, self, &metrics);
    if (local_y < metrics.thumb_region.location.y)
    {
        return 1;
    }
    if (local_y >= metrics.thumb_region.location.y + metrics.thumb_region.size.height)
    {
        return 2;
    }
    return 0;
}

static void scroll_viewer_drag_thumb_to(egui_view_t *self, egui_dim_t screen_y)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);
    egui_view_scroll_viewer_metrics_t metrics;
    egui_dim_t max_offset = scroll_viewer_get_max_vertical_offset_inner(local);
    egui_dim_t thumb_travel;
    egui_dim_t delta_y;
    egui_dim_t target_offset;

    scroll_viewer_get_metrics(local, self, &metrics);
    thumb_travel = metrics.track_region.size.height - metrics.thumb_region.size.height;
    if (thumb_travel <= 0 || max_offset <= 0)
    {
        scroll_viewer_apply_vertical_offset_inner(self, 0, 1, EGUI_VIEW_SCROLL_VIEWER_PART_THUMB);
        return;
    }

    delta_y = screen_y - local->drag_anchor_y;
    target_offset = local->drag_anchor_offset + (egui_dim_t)(((int32_t)delta_y * max_offset) / thumb_travel);
    scroll_viewer_apply_vertical_offset_inner(self, target_offset, 1, EGUI_VIEW_SCROLL_VIEWER_PART_THUMB);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_scroll_viewer_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);
    uint8_t hit_part;

    if (!egui_view_get_enable(self) || local->read_only_mode || local->compact_mode)
    {
        if (scroll_viewer_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }
    scroll_viewer_normalize_state(local);

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_part = scroll_viewer_hit_part(local, self, event->location.x, event->location.y);
        if (hit_part == EGUI_VIEW_SCROLL_VIEWER_PART_NONE)
        {
            if (scroll_viewer_clear_pressed_state(self, local))
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
        local->pressed_part = hit_part;
        local->thumb_dragging = hit_part == EGUI_VIEW_SCROLL_VIEWER_PART_THUMB ? 1 : 0;
        local->track_direction = hit_part == EGUI_VIEW_SCROLL_VIEWER_PART_TRACK ? scroll_viewer_track_direction_from_point(local, self, event->location.y) : 0;
        local->drag_anchor_y = event->location.y;
        local->drag_anchor_offset = local->vertical_offset;
        local->current_part = hit_part == EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE ? EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE : EGUI_VIEW_SCROLL_VIEWER_PART_THUMB;
        egui_view_set_pressed(self, true);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_part == EGUI_VIEW_SCROLL_VIEWER_PART_NONE)
        {
            return 0;
        }
        if (local->thumb_dragging)
        {
            scroll_viewer_drag_thumb_to(self, event->location.y);
        }
        else
        {
            uint8_t should_press = scroll_viewer_hit_part(local, self, event->location.x, event->location.y) == local->pressed_part ? 1 : 0;

            if ((self->is_pressed ? 1 : 0) != should_press)
            {
                egui_view_set_pressed(self, should_press);
                egui_view_invalidate(self);
            }
        }
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
    {
        uint8_t handled;

        hit_part = scroll_viewer_hit_part(local, self, event->location.x, event->location.y);
        if (local->thumb_dragging)
        {
            scroll_viewer_drag_thumb_to(self, event->location.y);
        }
        else if (local->pressed_part == EGUI_VIEW_SCROLL_VIEWER_PART_TRACK && hit_part == EGUI_VIEW_SCROLL_VIEWER_PART_TRACK)
        {
            if (local->track_direction == 1)
            {
                scroll_viewer_apply_vertical_offset_inner(self, local->vertical_offset - scroll_viewer_get_page_step_inner(local), 1,
                                                          EGUI_VIEW_SCROLL_VIEWER_PART_TRACK);
            }
            else if (local->track_direction == 2)
            {
                scroll_viewer_apply_vertical_offset_inner(self, local->vertical_offset + scroll_viewer_get_page_step_inner(local), 1,
                                                          EGUI_VIEW_SCROLL_VIEWER_PART_TRACK);
            }
        }

        handled = scroll_viewer_clear_pressed_state(self, local);
        if (handled)
        {
            egui_view_invalidate(self);
        }
        return handled || hit_part != EGUI_VIEW_SCROLL_VIEWER_PART_NONE;
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (scroll_viewer_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}

static int egui_view_scroll_viewer_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);
    EGUI_UNUSED(event);

    if (scroll_viewer_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

static void scroll_viewer_set_current_snapshot_inner(egui_view_t *self, uint8_t snapshot_index, uint8_t notify)
{
    egui_view_scroll_viewer_t *local = scroll_viewer_local(self);

    if (local->snapshots == NULL || local->snapshot_count == 0)
    {
        local->current_snapshot = 0;
        scroll_viewer_load_current_snapshot(local);
        egui_view_invalidate(self);
        return;
    }
    if (snapshot_index >= local->snapshot_count)
    {
        snapshot_index = 0;
    }
    if (local->current_snapshot == snapshot_index)
    {
        if (scroll_viewer_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_snapshot = snapshot_index;
    scroll_viewer_load_current_snapshot(local);
    scroll_viewer_clear_pressed_state(self, local);
    if (notify)
    {
        scroll_viewer_notify_change(self, local, EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE);
    }
    egui_view_invalidate(self);
}

void egui_view_scroll_viewer_set_snapshots(egui_view_t *self, const egui_view_scroll_viewer_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);

    local->snapshots = snapshots;
    local->snapshot_count = scroll_viewer_clamp_snapshot_count(snapshot_count);
    local->current_snapshot = 0;
    scroll_viewer_load_current_snapshot(local);
    scroll_viewer_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_scroll_viewer_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    scroll_viewer_set_current_snapshot_inner(self, snapshot_index, 1);
}

uint8_t egui_view_scroll_viewer_get_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);
    return local->current_snapshot;
}

void egui_view_scroll_viewer_set_vertical_offset(egui_view_t *self, egui_dim_t offset)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);

    scroll_viewer_clear_pressed_state(self, local);
    scroll_viewer_normalize_state(local);
    scroll_viewer_apply_vertical_offset_inner(self, offset, 1, EGUI_VIEW_SCROLL_VIEWER_PART_THUMB);
}

egui_dim_t egui_view_scroll_viewer_get_vertical_offset(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);
    scroll_viewer_normalize_state(local);
    return local->vertical_offset;
}

egui_dim_t egui_view_scroll_viewer_get_max_vertical_offset(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);
    scroll_viewer_normalize_state(local);
    return scroll_viewer_get_max_vertical_offset_inner(local);
}

void egui_view_scroll_viewer_set_horizontal_offset(egui_view_t *self, egui_dim_t offset)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);

    scroll_viewer_clear_pressed_state(self, local);
    scroll_viewer_normalize_state(local);
    scroll_viewer_apply_horizontal_offset_inner(self, offset, 1, EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE);
}

egui_dim_t egui_view_scroll_viewer_get_horizontal_offset(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);
    scroll_viewer_normalize_state(local);
    return local->horizontal_offset;
}

egui_dim_t egui_view_scroll_viewer_get_max_horizontal_offset(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);
    scroll_viewer_normalize_state(local);
    return scroll_viewer_get_max_horizontal_offset_inner(local);
}

void egui_view_scroll_viewer_scroll_line(egui_view_t *self, egui_dim_t delta_lines)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);

    scroll_viewer_clear_pressed_state(self, local);
    scroll_viewer_normalize_state(local);
    scroll_viewer_apply_vertical_offset_inner(self, local->vertical_offset + delta_lines * scroll_viewer_get_line_step_inner(local), 1,
                                              EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE);
}

void egui_view_scroll_viewer_scroll_page(egui_view_t *self, egui_dim_t delta_pages)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);

    scroll_viewer_clear_pressed_state(self, local);
    scroll_viewer_normalize_state(local);
    scroll_viewer_apply_vertical_offset_inner(self, local->vertical_offset + delta_pages * scroll_viewer_get_page_step_inner(local), 1,
                                              EGUI_VIEW_SCROLL_VIEWER_PART_TRACK);
}

void egui_view_scroll_viewer_set_scrollbar_visibility(egui_view_t *self, uint8_t visible)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);
    uint8_t changed = 0;
    uint8_t had_pressed = scroll_viewer_clear_pressed_state(self, local);

    visible = visible ? 1 : 0;
    if (local->scrollbar_visibility != visible)
    {
        local->scrollbar_visibility = visible;
        changed = 1;
    }
    if (!visible && local->current_part != EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE)
    {
        local->current_part = EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE;
        changed = 1;
    }
    if (changed || had_pressed)
    {
        egui_view_invalidate(self);
    }
}

uint8_t egui_view_scroll_viewer_get_scrollbar_visibility(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);
    scroll_viewer_normalize_state(local);
    return local->scrollbar_visibility;
}

void egui_view_scroll_viewer_set_on_view_changed_listener(egui_view_t *self, egui_view_on_scroll_viewer_changed_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);
    local->on_view_changed = listener;
}

void egui_view_scroll_viewer_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);

    scroll_viewer_clear_pressed_state(self, local);
    local->font = font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : font;
    egui_view_invalidate(self);
}

void egui_view_scroll_viewer_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);

    scroll_viewer_clear_pressed_state(self, local);
    local->meta_font = font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : font;
    egui_view_invalidate(self);
}

void egui_view_scroll_viewer_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);
    uint8_t changed = 0;
    uint8_t had_pressed;

    compact_mode = compact_mode ? 1 : 0;
    had_pressed = scroll_viewer_clear_pressed_state(self, local);
    if (local->compact_mode != compact_mode)
    {
        local->compact_mode = compact_mode;
        changed = 1;
    }
    if (compact_mode && local->current_part != EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE)
    {
        local->current_part = EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE;
        changed = 1;
    }
    if (changed || had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_scroll_viewer_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);
    uint8_t changed = 0;
    uint8_t had_pressed;

    read_only_mode = read_only_mode ? 1 : 0;
    had_pressed = scroll_viewer_clear_pressed_state(self, local);
    if (local->read_only_mode != read_only_mode)
    {
        local->read_only_mode = read_only_mode;
        changed = 1;
    }
    if (read_only_mode && local->current_part != EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE)
    {
        local->current_part = EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE;
        changed = 1;
    }
    if (changed || had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_scroll_viewer_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t viewport_color,
                                         egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                         egui_color_t preview_color)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);

    scroll_viewer_clear_pressed_state(self, local);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->viewport_color = viewport_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->preview_color = preview_color;
    egui_view_invalidate(self);
}

uint8_t egui_view_scroll_viewer_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);
    egui_view_scroll_viewer_metrics_t metrics;

    if (region == NULL)
    {
        return 0;
    }

    scroll_viewer_normalize_state(local);
    scroll_viewer_get_metrics(local, self, &metrics);

    if (part == EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE)
    {
        *region = metrics.viewport_region;
    }
    else if (part == EGUI_VIEW_SCROLL_VIEWER_PART_THUMB && metrics.show_scrollbar)
    {
        *region = metrics.thumb_region;
    }
    else if (part == EGUI_VIEW_SCROLL_VIEWER_PART_TRACK && metrics.show_scrollbar)
    {
        *region = metrics.track_region;
    }
    else
    {
        return 0;
    }

    region->location.x += self->region_screen.location.x;
    region->location.y += self->region_screen.location.y;
    return 1;
}

uint8_t egui_view_scroll_viewer_handle_navigation_key(egui_view_t *self, uint8_t key_code)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);

    if (scroll_viewer_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    scroll_viewer_normalize_state(local);
    if (!egui_view_get_enable(self) || local->read_only_mode || local->compact_mode)
    {
        return 0;
    }

    switch (key_code)
    {
    case EGUI_KEY_CODE_TAB:
        local->current_part =
                local->current_part == EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE && local->scrollbar_visibility ? EGUI_VIEW_SCROLL_VIEWER_PART_THUMB :
                                                                                                            EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE;
        egui_view_invalidate(self);
        return 1;
    case EGUI_KEY_CODE_UP:
        scroll_viewer_apply_vertical_offset_inner(self, local->vertical_offset - scroll_viewer_get_line_step_inner(local), 1, local->current_part);
        return 1;
    case EGUI_KEY_CODE_DOWN:
        scroll_viewer_apply_vertical_offset_inner(self, local->vertical_offset + scroll_viewer_get_line_step_inner(local), 1, local->current_part);
        return 1;
    case EGUI_KEY_CODE_LEFT:
        scroll_viewer_apply_horizontal_offset_inner(self, local->horizontal_offset - scroll_viewer_get_horizontal_step_inner(local), 1,
                                                    EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE);
        return 1;
    case EGUI_KEY_CODE_RIGHT:
        scroll_viewer_apply_horizontal_offset_inner(self, local->horizontal_offset + scroll_viewer_get_horizontal_step_inner(local), 1,
                                                    EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE);
        return 1;
    case EGUI_KEY_CODE_HOME:
        scroll_viewer_apply_vertical_offset_inner(self, 0, 1, local->current_part);
        return 1;
    case EGUI_KEY_CODE_END:
        scroll_viewer_apply_vertical_offset_inner(self, scroll_viewer_get_max_vertical_offset_inner(local), 1, local->current_part);
        return 1;
    case EGUI_KEY_CODE_MINUS:
        scroll_viewer_apply_vertical_offset_inner(self, local->vertical_offset - scroll_viewer_get_page_step_inner(local), 1,
                                                  EGUI_VIEW_SCROLL_VIEWER_PART_TRACK);
        return 1;
    case EGUI_KEY_CODE_PLUS:
        scroll_viewer_apply_vertical_offset_inner(self, local->vertical_offset + scroll_viewer_get_page_step_inner(local), 1,
                                                  EGUI_VIEW_SCROLL_VIEWER_PART_TRACK);
        return 1;
    case EGUI_KEY_CODE_ENTER:
    case EGUI_KEY_CODE_SPACE:
        if (local->current_part == EGUI_VIEW_SCROLL_VIEWER_PART_THUMB)
        {
            scroll_viewer_apply_vertical_offset_inner(self, local->vertical_offset + scroll_viewer_get_page_step_inner(local), 1,
                                                      EGUI_VIEW_SCROLL_VIEWER_PART_THUMB);
            return 1;
        }
        return 0;
    case EGUI_KEY_CODE_ESCAPE:
        if (local->current_part != EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE)
        {
            local->current_part = EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE;
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_scroll_viewer_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);

    if (!egui_view_get_enable(self) || local->read_only_mode || local->compact_mode)
    {
        if (scroll_viewer_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    if (event->type != EGUI_KEY_EVENT_ACTION_UP)
    {
        switch (event->key_code)
        {
        case EGUI_KEY_CODE_TAB:
        case EGUI_KEY_CODE_UP:
        case EGUI_KEY_CODE_DOWN:
        case EGUI_KEY_CODE_LEFT:
        case EGUI_KEY_CODE_RIGHT:
        case EGUI_KEY_CODE_HOME:
        case EGUI_KEY_CODE_END:
        case EGUI_KEY_CODE_MINUS:
        case EGUI_KEY_CODE_PLUS:
        case EGUI_KEY_CODE_ESCAPE:
            return 1;
        case EGUI_KEY_CODE_ENTER:
        case EGUI_KEY_CODE_SPACE:
            if (local->current_part == EGUI_VIEW_SCROLL_VIEWER_PART_THUMB && local->scrollbar_visibility)
            {
                local->pressed_part = EGUI_VIEW_SCROLL_VIEWER_PART_THUMB;
                egui_view_set_pressed(self, true);
                egui_view_invalidate(self);
                return 1;
            }
            return 0;
        default:
            return 0;
        }
    }

    if (egui_view_scroll_viewer_handle_navigation_key(self, event->key_code))
    {
        return 1;
    }
    return egui_view_on_key_event(self, event);
}

static int egui_view_scroll_viewer_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);
    EGUI_UNUSED(event);

    if (scroll_viewer_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static void egui_view_scroll_viewer_on_focus_change(egui_view_t *self, int is_focused)
{
    EGUI_LOCAL_INIT(egui_view_scroll_viewer_t);
    uint8_t needs_invalidate = 0;

    if (is_focused)
    {
        return;
    }
    if (scroll_viewer_clear_pressed_state(self, local))
    {
        needs_invalidate = 1;
    }
    if (local->current_part != EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE)
    {
        local->current_part = EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE;
        needs_invalidate = 1;
    }
    if (needs_invalidate)
    {
        egui_view_invalidate(self);
    }
}
#endif

void egui_view_scroll_viewer_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_scroll_viewer_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_scroll_viewer_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_scroll_viewer_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_scroll_viewer_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_scroll_viewer_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_scroll_viewer_on_key_event,
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        .on_focus_changed = egui_view_scroll_viewer_on_focus_change,
#endif
};

void egui_view_scroll_viewer_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_scroll_viewer_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_scroll_viewer_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif

    local->snapshots = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->on_view_changed = NULL;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xD6DEE6);
    local->viewport_color = EGUI_COLOR_HEX(0xF8FBFD);
    local->text_color = EGUI_COLOR_HEX(0x1B2834);
    local->muted_text_color = EGUI_COLOR_HEX(0x6C7A88);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->preview_color = EGUI_COLOR_HEX(0x6AA8FF);
    local->snapshot_count = 0;
    local->current_snapshot = 0;
    local->current_part = EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->scrollbar_visibility = 1;
    local->pressed_part = EGUI_VIEW_SCROLL_VIEWER_PART_NONE;
    local->thumb_dragging = 0;
    local->track_direction = 0;
    local->vertical_offset = 0;
    local->horizontal_offset = 0;
    local->drag_anchor_y = 0;
    local->drag_anchor_offset = 0;

    egui_view_set_view_name(self, "egui_view_scroll_viewer");
}
