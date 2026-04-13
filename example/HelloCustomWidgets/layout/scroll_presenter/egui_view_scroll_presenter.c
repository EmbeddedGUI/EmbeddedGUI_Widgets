#include "egui_view_scroll_presenter.h"

#include "utils/egui_sprintf.h"

#define SP_STANDARD_RADIUS       10
#define SP_STANDARD_PAD_X        10
#define SP_STANDARD_PAD_Y        8
#define SP_STANDARD_EYEBROW_H    8
#define SP_STANDARD_EYEBROW_GAP  4
#define SP_STANDARD_TITLE_H      12
#define SP_STANDARD_TITLE_GAP    3
#define SP_STANDARD_SUMMARY_H    10
#define SP_STANDARD_BODY_GAP     6
#define SP_STANDARD_VIEWPORT_PAD 5
#define SP_STANDARD_FOOTER_H     10
#define SP_STANDARD_MINIMAP_W    46
#define SP_STANDARD_MINIMAP_H    18

#define SP_COMPACT_RADIUS       8
#define SP_COMPACT_PAD_X        6
#define SP_COMPACT_PAD_Y        6
#define SP_COMPACT_TITLE_H      10
#define SP_COMPACT_TITLE_GAP    4
#define SP_COMPACT_BODY_GAP     4
#define SP_COMPACT_VIEWPORT_PAD 4
#define SP_COMPACT_FOOTER_H     8
#define SP_COMPACT_MINIMAP_W    34
#define SP_COMPACT_MINIMAP_H    14

typedef struct egui_view_scroll_presenter_metrics egui_view_scroll_presenter_metrics_t;
struct egui_view_scroll_presenter_metrics
{
    egui_region_t content_region;
    egui_region_t eyebrow_region;
    egui_region_t title_region;
    egui_region_t summary_region;
    egui_region_t viewport_region;
    egui_region_t viewport_content_region;
    egui_region_t footer_region;
    egui_region_t helper_region;
    egui_region_t minimap_region;
    uint8_t show_eyebrow;
    uint8_t show_summary;
};

static egui_view_scroll_presenter_t *scroll_presenter_local(egui_view_t *self)
{
    return (egui_view_scroll_presenter_t *)self;
}

static uint8_t scroll_presenter_has_text(const char *text)
{
    return (text != NULL && text[0] != '\0') ? 1 : 0;
}

static uint8_t scroll_presenter_clamp_snapshot_count(uint8_t count)
{
    return count > EGUI_VIEW_SCROLL_PRESENTER_MAX_SNAPSHOTS ? EGUI_VIEW_SCROLL_PRESENTER_MAX_SNAPSHOTS : count;
}

static uint8_t scroll_presenter_clamp_item_count(uint8_t count)
{
    return count > EGUI_VIEW_SCROLL_PRESENTER_MAX_ITEMS ? EGUI_VIEW_SCROLL_PRESENTER_MAX_ITEMS : count;
}

static egui_color_t scroll_presenter_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 68);
}

static const egui_view_scroll_presenter_snapshot_t *scroll_presenter_get_snapshot(egui_view_scroll_presenter_t *local)
{
    if (local->snapshots == NULL || local->snapshot_count == 0 || local->current_snapshot >= local->snapshot_count)
    {
        return NULL;
    }
    return &local->snapshots[local->current_snapshot];
}

static egui_dim_t scroll_presenter_snapshot_content_width(const egui_view_scroll_presenter_snapshot_t *snapshot)
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

static egui_dim_t scroll_presenter_snapshot_content_height(const egui_view_scroll_presenter_snapshot_t *snapshot)
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

static egui_dim_t scroll_presenter_snapshot_viewport_width(const egui_view_scroll_presenter_snapshot_t *snapshot)
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

static egui_dim_t scroll_presenter_snapshot_viewport_height(const egui_view_scroll_presenter_snapshot_t *snapshot)
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

static egui_dim_t scroll_presenter_get_max_vertical_offset_inner(egui_view_scroll_presenter_t *local)
{
    const egui_view_scroll_presenter_snapshot_t *snapshot = scroll_presenter_get_snapshot(local);
    egui_dim_t content_height = scroll_presenter_snapshot_content_height(snapshot);
    egui_dim_t viewport_height = scroll_presenter_snapshot_viewport_height(snapshot);

    return content_height > viewport_height ? (content_height - viewport_height) : 0;
}

static egui_dim_t scroll_presenter_get_max_horizontal_offset_inner(egui_view_scroll_presenter_t *local)
{
    const egui_view_scroll_presenter_snapshot_t *snapshot = scroll_presenter_get_snapshot(local);
    egui_dim_t content_width = scroll_presenter_snapshot_content_width(snapshot);
    egui_dim_t viewport_width = scroll_presenter_snapshot_viewport_width(snapshot);

    return content_width > viewport_width ? (content_width - viewport_width) : 0;
}

static egui_dim_t scroll_presenter_get_line_step_inner(egui_view_scroll_presenter_t *local)
{
    egui_dim_t step = scroll_presenter_snapshot_viewport_height(scroll_presenter_get_snapshot(local)) / 6;

    if (step < 10)
    {
        step = 10;
    }
    return step;
}

static egui_dim_t scroll_presenter_get_page_step_inner(egui_view_scroll_presenter_t *local)
{
    egui_dim_t viewport_height = scroll_presenter_snapshot_viewport_height(scroll_presenter_get_snapshot(local));
    egui_dim_t step = viewport_height - viewport_height / 6;

    if (step < scroll_presenter_get_line_step_inner(local))
    {
        step = scroll_presenter_get_line_step_inner(local);
    }
    return step;
}

static egui_dim_t scroll_presenter_get_horizontal_step_inner(egui_view_scroll_presenter_t *local)
{
    egui_dim_t step = scroll_presenter_snapshot_viewport_width(scroll_presenter_get_snapshot(local)) / 6;

    if (step < 8)
    {
        step = 8;
    }
    return step;
}

static uint8_t scroll_presenter_clear_pressed_state(egui_view_t *self, egui_view_scroll_presenter_t *local)
{
    uint8_t had_pressed = self->is_pressed || local->pressed_part != EGUI_VIEW_SCROLL_PRESENTER_PART_NONE || local->surface_dragging;

    local->pressed_part = EGUI_VIEW_SCROLL_PRESENTER_PART_NONE;
    local->surface_dragging = 0;
    egui_view_set_pressed(self, false);
    return had_pressed;
}

static void scroll_presenter_normalize_state(egui_view_scroll_presenter_t *local)
{
    egui_dim_t max_vertical;
    egui_dim_t max_horizontal;

    if (scroll_presenter_get_snapshot(local) == NULL)
    {
        local->current_part = EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE;
        local->vertical_offset = 0;
        local->horizontal_offset = 0;
        return;
    }

    max_vertical = scroll_presenter_get_max_vertical_offset_inner(local);
    max_horizontal = scroll_presenter_get_max_horizontal_offset_inner(local);
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

    local->current_part = EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE;
    if (local->pressed_part != EGUI_VIEW_SCROLL_PRESENTER_PART_NONE && local->pressed_part != EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE)
    {
        local->pressed_part = EGUI_VIEW_SCROLL_PRESENTER_PART_NONE;
    }
    if (local->compact_mode || local->read_only_mode)
    {
        local->pressed_part = EGUI_VIEW_SCROLL_PRESENTER_PART_NONE;
        local->surface_dragging = 0;
    }
}

static void scroll_presenter_load_current_snapshot(egui_view_scroll_presenter_t *local)
{
    const egui_view_scroll_presenter_snapshot_t *snapshot = scroll_presenter_get_snapshot(local);

    local->current_part = EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE;
    if (snapshot == NULL)
    {
        local->vertical_offset = 0;
        local->horizontal_offset = 0;
        return;
    }

    local->vertical_offset = snapshot->vertical_offset;
    local->horizontal_offset = snapshot->horizontal_offset;
    scroll_presenter_normalize_state(local);
}

static void scroll_presenter_notify_change(egui_view_t *self, egui_view_scroll_presenter_t *local, uint8_t part)
{
    if (local->on_view_changed != NULL)
    {
        local->on_view_changed(self, local->current_snapshot, local->vertical_offset, local->horizontal_offset, part);
    }
}

static void scroll_presenter_format_pair(egui_dim_t a, egui_dim_t b, char *buffer, int buffer_size)
{
    int pos = 0;

    pos += egui_sprintf_int(buffer, buffer_size, a);
    pos += egui_sprintf_str(&buffer[pos], buffer_size - pos, " / ");
    egui_sprintf_int(&buffer[pos], buffer_size - pos, b);
}

static void scroll_presenter_format_axes(egui_dim_t vertical_offset, egui_dim_t vertical_max, egui_dim_t horizontal_offset, egui_dim_t horizontal_max,
                                         char *buffer, int buffer_size)
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

static egui_color_t scroll_presenter_tone_color(egui_view_scroll_presenter_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_SCROLL_PRESENTER_TONE_SUCCESS:
        return egui_rgb_mix(EGUI_COLOR_HEX(0x0F9D58), local->preview_color, 18);
    case EGUI_VIEW_SCROLL_PRESENTER_TONE_WARNING:
        return egui_rgb_mix(EGUI_COLOR_HEX(0xF59E0B), local->accent_color, 10);
    case EGUI_VIEW_SCROLL_PRESENTER_TONE_NEUTRAL:
        return egui_rgb_mix(local->border_color, local->preview_color, 18);
    default:
        return local->accent_color;
    }
}

static void scroll_presenter_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                       egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (!scroll_presenter_has_text(text))
    {
        return;
    }
    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, self->alpha);
}

static void scroll_presenter_draw_focus(egui_view_t *self, const egui_region_t *region, egui_dim_t radius, egui_color_t color, egui_alpha_t alpha)
{
    egui_canvas_draw_round_rectangle(region->location.x - 1, region->location.y - 1, region->size.width + 2, region->size.height + 2, radius, 1, color,
                                     egui_color_alpha_mix(self->alpha, alpha));
}

static void scroll_presenter_get_metrics(egui_view_scroll_presenter_t *local, egui_view_t *self, egui_view_scroll_presenter_metrics_t *metrics)
{
    const egui_view_scroll_presenter_snapshot_t *snapshot = scroll_presenter_get_snapshot(local);
    egui_region_t work_region;
    egui_dim_t pad_x = local->compact_mode ? SP_COMPACT_PAD_X : SP_STANDARD_PAD_X;
    egui_dim_t pad_y = local->compact_mode ? SP_COMPACT_PAD_Y : SP_STANDARD_PAD_Y;
    egui_dim_t title_h = local->compact_mode ? SP_COMPACT_TITLE_H : SP_STANDARD_TITLE_H;
    egui_dim_t title_gap = local->compact_mode ? SP_COMPACT_TITLE_GAP : SP_STANDARD_TITLE_GAP;
    egui_dim_t body_gap = local->compact_mode ? SP_COMPACT_BODY_GAP : SP_STANDARD_BODY_GAP;
    egui_dim_t footer_h = local->compact_mode ? SP_COMPACT_FOOTER_H : SP_STANDARD_FOOTER_H;
    egui_dim_t viewport_pad = local->compact_mode ? SP_COMPACT_VIEWPORT_PAD : SP_STANDARD_VIEWPORT_PAD;
    egui_dim_t cursor_y;
    egui_dim_t footer_y;
    egui_dim_t viewport_bottom;

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

    metrics->show_eyebrow = (!local->compact_mode && snapshot != NULL && scroll_presenter_has_text(snapshot->eyebrow)) ? 1 : 0;
    metrics->show_summary = (!local->compact_mode && snapshot != NULL && scroll_presenter_has_text(snapshot->summary)) ? 1 : 0;

    cursor_y = metrics->content_region.location.y;
    if (metrics->show_eyebrow)
    {
        metrics->eyebrow_region.location.x = metrics->content_region.location.x;
        metrics->eyebrow_region.location.y = cursor_y;
        metrics->eyebrow_region.size.width = metrics->content_region.size.width;
        metrics->eyebrow_region.size.height = SP_STANDARD_EYEBROW_H;
        cursor_y += SP_STANDARD_EYEBROW_H + SP_STANDARD_EYEBROW_GAP;
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
        metrics->summary_region.size.height = SP_STANDARD_SUMMARY_H;
        cursor_y += SP_STANDARD_SUMMARY_H;
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
    metrics->viewport_region.size.width = metrics->content_region.size.width;
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

    metrics->footer_region.location.x = metrics->content_region.location.x;
    metrics->footer_region.location.y = footer_y;
    metrics->footer_region.size.width = metrics->content_region.size.width / 2;
    metrics->footer_region.size.height = footer_h;

    metrics->helper_region.location.x = metrics->content_region.location.x + metrics->content_region.size.width / 2;
    metrics->helper_region.location.y = footer_y;
    metrics->helper_region.size.width = metrics->content_region.size.width / 2;
    metrics->helper_region.size.height = footer_h;

    metrics->minimap_region.size.width = local->compact_mode ? SP_COMPACT_MINIMAP_W : SP_STANDARD_MINIMAP_W;
    metrics->minimap_region.size.height = local->compact_mode ? SP_COMPACT_MINIMAP_H : SP_STANDARD_MINIMAP_H;
    if (metrics->minimap_region.size.width > metrics->viewport_region.size.width - 10)
    {
        metrics->minimap_region.size.width = metrics->viewport_region.size.width - 10;
    }
    if (metrics->minimap_region.size.height > metrics->viewport_region.size.height - 10)
    {
        metrics->minimap_region.size.height = metrics->viewport_region.size.height - 10;
    }
    if (metrics->minimap_region.size.width < 16)
    {
        metrics->minimap_region.size.width = 16;
    }
    if (metrics->minimap_region.size.height < 10)
    {
        metrics->minimap_region.size.height = 10;
    }
    metrics->minimap_region.location.x = metrics->viewport_region.location.x + metrics->viewport_region.size.width - metrics->minimap_region.size.width - 5;
    metrics->minimap_region.location.y = metrics->viewport_region.location.y + 5;
}

static void scroll_presenter_apply_vertical_offset_inner(egui_view_t *self, egui_dim_t offset, uint8_t notify, uint8_t part)
{
    egui_view_scroll_presenter_t *local = scroll_presenter_local(self);
    egui_dim_t max_offset = scroll_presenter_get_max_vertical_offset_inner(local);

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
        scroll_presenter_notify_change(self, local, part);
    }
    egui_view_invalidate(self);
}

static void scroll_presenter_apply_horizontal_offset_inner(egui_view_t *self, egui_dim_t offset, uint8_t notify, uint8_t part)
{
    egui_view_scroll_presenter_t *local = scroll_presenter_local(self);
    egui_dim_t max_offset = scroll_presenter_get_max_horizontal_offset_inner(local);

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
        scroll_presenter_notify_change(self, local, part);
    }
    egui_view_invalidate(self);
}

static void scroll_presenter_draw_item(egui_view_t *self, egui_view_scroll_presenter_t *local, const egui_view_scroll_presenter_item_t *item,
                                       const egui_region_t *viewport_content_region, egui_color_t surface_color, egui_color_t border_color,
                                       egui_color_t text_color, egui_color_t muted_color)
{
    egui_region_t item_region;
    egui_region_t badge_region;
    egui_region_t title_region;
    egui_region_t meta_region;
    egui_color_t tone_color;
    egui_color_t fill_color;
    egui_color_t line_color;

    if (item == NULL)
    {
        return;
    }

    item_region.location.x = viewport_content_region->location.x + item->origin_x - local->horizontal_offset;
    item_region.location.y = viewport_content_region->location.y + item->origin_y - local->vertical_offset;
    item_region.size.width = item->width;
    item_region.size.height = item->height;
    if (item_region.size.width < 10 || item_region.size.height < 10)
    {
        return;
    }
    if (item_region.location.x + item_region.size.width < viewport_content_region->location.x ||
        item_region.location.x > viewport_content_region->location.x + viewport_content_region->size.width ||
        item_region.location.y + item_region.size.height < viewport_content_region->location.y ||
        item_region.location.y > viewport_content_region->location.y + viewport_content_region->size.height)
    {
        return;
    }

    tone_color = scroll_presenter_tone_color(local, item->tone);
    fill_color = egui_rgb_mix(surface_color, tone_color, item->emphasized ? 18 : 10);
    line_color = egui_rgb_mix(border_color, tone_color, 22);

    egui_canvas_draw_round_rectangle_fill(item_region.location.x, item_region.location.y, item_region.size.width, item_region.size.height, 7, fill_color,
                                          egui_color_alpha_mix(self->alpha, item->emphasized ? 98 : 90));
    egui_canvas_draw_round_rectangle(item_region.location.x, item_region.location.y, item_region.size.width, item_region.size.height, 7, 1, line_color,
                                     egui_color_alpha_mix(self->alpha, 76));

    badge_region.location.x = item_region.location.x + 6;
    badge_region.location.y = item_region.location.y + 5;
    badge_region.size.width = item_region.size.width - 12;
    badge_region.size.height = 8;

    title_region.location.x = item_region.location.x + 6;
    title_region.location.y = item_region.location.y + 18;
    title_region.size.width = item_region.size.width - 12;
    title_region.size.height = 10;

    meta_region.location.x = item_region.location.x + 6;
    meta_region.location.y = item_region.location.y + item_region.size.height - 16;
    meta_region.size.width = item_region.size.width - 12;
    meta_region.size.height = 10;

    if (scroll_presenter_has_text(item->badge))
    {
        egui_region_t pill_region = badge_region;

        pill_region.size.width = 24;
        if (pill_region.size.width > item_region.size.width - 12)
        {
            pill_region.size.width = item_region.size.width - 12;
        }
        egui_canvas_draw_round_rectangle_fill(pill_region.location.x, pill_region.location.y, pill_region.size.width, pill_region.size.height, 4,
                                              egui_rgb_mix(tone_color, EGUI_COLOR_WHITE, 20), egui_color_alpha_mix(self->alpha, 92));
        scroll_presenter_draw_text(local->meta_font, self, item->badge, &pill_region, EGUI_ALIGN_CENTER, EGUI_COLOR_HEX(0xFFFFFF));
    }

    scroll_presenter_draw_text(local->font, self, item->title, &title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text_color);
    scroll_presenter_draw_text(local->meta_font, self, item->meta, &meta_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                               egui_rgb_mix(text_color, muted_color, 34));
}

static void scroll_presenter_draw_connectors(egui_view_t *self, egui_view_scroll_presenter_t *local, const egui_region_t *viewport_content_region,
                                             egui_color_t border_color, egui_color_t preview_color)
{
    const egui_view_scroll_presenter_snapshot_t *snapshot = scroll_presenter_get_snapshot(local);
    egui_color_t connector_color = egui_rgb_mix(border_color, preview_color, 22);
    uint8_t item_count = snapshot == NULL ? 0 : scroll_presenter_clamp_item_count(snapshot->item_count);
    uint8_t i;

    if (snapshot == NULL || snapshot->items == NULL || item_count < 2)
    {
        return;
    }

    for (i = 1; i < item_count; i++)
    {
        const egui_view_scroll_presenter_item_t *prev_item = &snapshot->items[i - 1];
        const egui_view_scroll_presenter_item_t *item = &snapshot->items[i];
        egui_dim_t prev_x = viewport_content_region->location.x + prev_item->origin_x + prev_item->width / 2 - local->horizontal_offset;
        egui_dim_t prev_y = viewport_content_region->location.y + prev_item->origin_y + prev_item->height / 2 - local->vertical_offset;
        egui_dim_t item_x = viewport_content_region->location.x + item->origin_x + item->width / 2 - local->horizontal_offset;
        egui_dim_t item_y = viewport_content_region->location.y + item->origin_y + item->height / 2 - local->vertical_offset;

        egui_canvas_draw_line(prev_x, prev_y, item_x, item_y, 1, connector_color, egui_color_alpha_mix(self->alpha, 54));
    }
}

static void scroll_presenter_draw_surface(egui_view_t *self, egui_view_scroll_presenter_t *local, const egui_view_scroll_presenter_metrics_t *metrics,
                                          egui_color_t surface_color, egui_color_t border_color, egui_color_t viewport_color, egui_color_t text_color,
                                          egui_color_t muted_color, egui_color_t accent_color, egui_color_t preview_color)
{
    const egui_view_scroll_presenter_snapshot_t *snapshot = scroll_presenter_get_snapshot(local);
    const egui_region_t *prev_clip = egui_canvas_get_extra_clip();
    const egui_region_t *active_clip = NULL;
    egui_region_t screen_clip_region;
    egui_region_t clip_region;
    egui_color_t grid_color = egui_rgb_mix(border_color, preview_color, 16);
    egui_dim_t grid_step_x = local->compact_mode ? 30 : 38;
    egui_dim_t grid_step_y = local->compact_mode ? 24 : 30;
    egui_dim_t x;
    egui_dim_t y;
    char minimap_text[32];
    uint8_t focus_surface = (local->current_part == EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE && !local->read_only_mode && !local->compact_mode &&
                             egui_view_get_enable(self)) ?
                                    1 :
                                    0;
    uint8_t item_count = snapshot == NULL ? 0 : scroll_presenter_clamp_item_count(snapshot->item_count);
    uint8_t i;

    egui_canvas_draw_round_rectangle_fill(metrics->viewport_region.location.x, metrics->viewport_region.location.y, metrics->viewport_region.size.width,
                                          metrics->viewport_region.size.height, local->compact_mode ? 7 : 9, viewport_color,
                                          egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_round_rectangle(metrics->viewport_region.location.x, metrics->viewport_region.location.y, metrics->viewport_region.size.width,
                                     metrics->viewport_region.size.height, local->compact_mode ? 7 : 9, 1, border_color,
                                     egui_color_alpha_mix(self->alpha, 72));

    if (focus_surface)
    {
        scroll_presenter_draw_focus(self, &metrics->viewport_region, local->compact_mode ? 8 : 10, egui_rgb_mix(accent_color, EGUI_COLOR_WHITE, 10), 58);
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

    for (x = metrics->viewport_content_region.location.x - (local->horizontal_offset % grid_step_x); x < metrics->viewport_content_region.location.x +
                                                                                                           metrics->viewport_content_region.size.width;
         x += grid_step_x)
    {
        egui_canvas_draw_line(x, metrics->viewport_content_region.location.y, x,
                              metrics->viewport_content_region.location.y + metrics->viewport_content_region.size.height, 1, grid_color,
                              egui_color_alpha_mix(self->alpha, 28));
    }
    for (y = metrics->viewport_content_region.location.y - (local->vertical_offset % grid_step_y); y < metrics->viewport_content_region.location.y +
                                                                                                       metrics->viewport_content_region.size.height;
         y += grid_step_y)
    {
        egui_canvas_draw_line(metrics->viewport_content_region.location.x, y,
                              metrics->viewport_content_region.location.x + metrics->viewport_content_region.size.width, y, 1, grid_color,
                              egui_color_alpha_mix(self->alpha, 24));
    }

    scroll_presenter_draw_connectors(self, local, &metrics->viewport_content_region, border_color, preview_color);
    for (i = 0; snapshot != NULL && snapshot->items != NULL && i < item_count; i++)
    {
        scroll_presenter_draw_item(self, local, &snapshot->items[i], &metrics->viewport_content_region, surface_color, border_color, text_color, muted_color);
    }
    if (snapshot == NULL || snapshot->items == NULL || item_count == 0)
    {
        egui_region_t empty_region = metrics->viewport_content_region;
        scroll_presenter_draw_text(local->meta_font, self, "No canvas", &empty_region, EGUI_ALIGN_CENTER, muted_color);
    }

    if (prev_clip != NULL)
    {
        egui_canvas_set_extra_clip(prev_clip);
    }
    else
    {
        egui_canvas_clear_extra_clip();
    }

    if (local->horizontal_offset > 0)
    {
        egui_canvas_draw_rectangle_fill(metrics->viewport_region.location.x + 1, metrics->viewport_region.location.y + 1, 3,
                                        metrics->viewport_region.size.height - 2, egui_rgb_mix(accent_color, preview_color, 18),
                                        egui_color_alpha_mix(self->alpha, 32));
    }
    if (local->horizontal_offset < scroll_presenter_get_max_horizontal_offset_inner(local))
    {
        egui_canvas_draw_rectangle_fill(metrics->viewport_region.location.x + metrics->viewport_region.size.width - 4,
                                        metrics->viewport_region.location.y + 1, 3, metrics->viewport_region.size.height - 2,
                                        egui_rgb_mix(accent_color, preview_color, 18), egui_color_alpha_mix(self->alpha, 28));
    }
    if (local->vertical_offset > 0)
    {
        egui_canvas_draw_rectangle_fill(metrics->viewport_region.location.x + 1, metrics->viewport_region.location.y + 1,
                                        metrics->viewport_region.size.width - 2, 3, egui_rgb_mix(accent_color, preview_color, 18),
                                        egui_color_alpha_mix(self->alpha, 32));
    }
    if (local->vertical_offset < scroll_presenter_get_max_vertical_offset_inner(local))
    {
        egui_canvas_draw_rectangle_fill(metrics->viewport_region.location.x + 1, metrics->viewport_region.location.y + metrics->viewport_region.size.height - 4,
                                        metrics->viewport_region.size.width - 2, 3, egui_rgb_mix(accent_color, preview_color, 18),
                                        egui_color_alpha_mix(self->alpha, 28));
    }

    egui_canvas_draw_round_rectangle_fill(metrics->minimap_region.location.x, metrics->minimap_region.location.y, metrics->minimap_region.size.width,
                                          metrics->minimap_region.size.height, metrics->minimap_region.size.height / 2,
                                          egui_rgb_mix(surface_color, preview_color, 10), egui_color_alpha_mix(self->alpha, 92));
    egui_canvas_draw_round_rectangle(metrics->minimap_region.location.x, metrics->minimap_region.location.y, metrics->minimap_region.size.width,
                                     metrics->minimap_region.size.height, metrics->minimap_region.size.height / 2, 1,
                                     egui_rgb_mix(border_color, preview_color, 14), egui_color_alpha_mix(self->alpha, 54));

    scroll_presenter_format_axes(local->vertical_offset, scroll_presenter_get_max_vertical_offset_inner(local), local->horizontal_offset,
                                 scroll_presenter_get_max_horizontal_offset_inner(local), minimap_text, (int)sizeof(minimap_text));
    scroll_presenter_draw_text(local->meta_font, self, minimap_text, &metrics->minimap_region, EGUI_ALIGN_CENTER,
                               egui_rgb_mix(text_color, muted_color, 28));
}

static void egui_view_scroll_presenter_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);
    const egui_view_scroll_presenter_snapshot_t *snapshot;
    egui_view_scroll_presenter_metrics_t metrics;
    egui_region_t region;
    egui_color_t surface_color = local->surface_color;
    egui_color_t border_color = local->border_color;
    egui_color_t viewport_color = local->viewport_color;
    egui_color_t text_color = local->text_color;
    egui_color_t muted_color = local->muted_text_color;
    egui_color_t accent_color = local->accent_color;
    egui_color_t preview_color = local->preview_color;
    egui_color_t shadow_color;
    char text_buffer[40];
    uint8_t enabled = egui_view_get_enable(self) ? 1 : 0;

    scroll_presenter_normalize_state(local);
    snapshot = scroll_presenter_get_snapshot(local);
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
        accent_color = scroll_presenter_mix_disabled(accent_color);
        preview_color = scroll_presenter_mix_disabled(preview_color);
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xFBFCFD), 34);
        border_color = scroll_presenter_mix_disabled(border_color);
        viewport_color = egui_rgb_mix(viewport_color, EGUI_COLOR_HEX(0xFBFCFD), 36);
        text_color = scroll_presenter_mix_disabled(text_color);
        muted_color = scroll_presenter_mix_disabled(muted_color);
    }

    scroll_presenter_get_metrics(local, self, &metrics);
    shadow_color = egui_rgb_mix(border_color, surface_color, 38);

    if (!local->compact_mode)
    {
        egui_canvas_draw_round_rectangle_fill(region.location.x, region.location.y + 2, region.size.width, region.size.height, SP_STANDARD_RADIUS + 1, shadow_color,
                                              egui_color_alpha_mix(self->alpha, enabled ? 16 : 10));
    }
    egui_canvas_draw_round_rectangle_fill(region.location.x, region.location.y, region.size.width, region.size.height,
                                          local->compact_mode ? SP_COMPACT_RADIUS : SP_STANDARD_RADIUS, surface_color,
                                          egui_color_alpha_mix(self->alpha, local->compact_mode ? 94 : 96));
    egui_canvas_draw_round_rectangle(region.location.x, region.location.y, region.size.width, region.size.height,
                                     local->compact_mode ? SP_COMPACT_RADIUS : SP_STANDARD_RADIUS, 1, border_color,
                                     egui_color_alpha_mix(self->alpha, local->compact_mode ? 56 : 60));

    if (metrics.show_eyebrow && snapshot != NULL)
    {
        scroll_presenter_draw_text(local->meta_font, self, snapshot->eyebrow, &metrics.eyebrow_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                                   egui_rgb_mix(text_color, accent_color, 20));
    }
    if (snapshot != NULL)
    {
        scroll_presenter_draw_text(local->font, self, snapshot->title, &metrics.title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text_color);
        if (metrics.show_summary)
        {
            scroll_presenter_draw_text(local->meta_font, self, snapshot->summary, &metrics.summary_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                                       egui_rgb_mix(text_color, muted_color, 34));
        }
    }
    else
    {
        scroll_presenter_draw_text(local->font, self, "Scroll Presenter", &metrics.title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text_color);
    }

    scroll_presenter_draw_surface(self, local, &metrics, surface_color, border_color, viewport_color, text_color, muted_color, accent_color, preview_color);

    if (snapshot != NULL && scroll_presenter_has_text(snapshot->footer))
    {
        scroll_presenter_draw_text(local->meta_font, self, snapshot->footer, &metrics.footer_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                                   egui_rgb_mix(text_color, muted_color, 32));
    }
    else
    {
        scroll_presenter_format_pair(scroll_presenter_snapshot_viewport_height(snapshot), scroll_presenter_snapshot_content_height(snapshot), text_buffer,
                                     (int)sizeof(text_buffer));
        scroll_presenter_draw_text(local->meta_font, self, text_buffer, &metrics.footer_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                                   egui_rgb_mix(text_color, muted_color, 32));
    }

    scroll_presenter_format_axes(local->vertical_offset, scroll_presenter_get_max_vertical_offset_inner(local), local->horizontal_offset,
                                 scroll_presenter_get_max_horizontal_offset_inner(local), text_buffer, (int)sizeof(text_buffer));
    scroll_presenter_draw_text(local->meta_font, self, text_buffer, &metrics.helper_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                               egui_rgb_mix(text_color, muted_color, local->compact_mode ? 26 : 30));

    if (!local->compact_mode && snapshot != NULL && scroll_presenter_has_text(snapshot->helper))
    {
        egui_region_t helper_region = metrics.helper_region;
        helper_region.size.width -= 6;
        if (helper_region.size.width > 8)
        {
            scroll_presenter_draw_text(local->meta_font, self, snapshot->helper, &helper_region, EGUI_ALIGN_RIGHT | EGUI_ALIGN_VCENTER,
                                       egui_rgb_mix(text_color, muted_color, 42));
        }
    }
}

static uint8_t scroll_presenter_hit_surface(egui_view_scroll_presenter_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    egui_view_scroll_presenter_metrics_t metrics;
    egui_dim_t local_x = x - self->region_screen.location.x;
    egui_dim_t local_y = y - self->region_screen.location.y;

    scroll_presenter_get_metrics(local, self, &metrics);
    return egui_region_pt_in_rect(&metrics.viewport_region, local_x, local_y) ? 1 : 0;
}

static void scroll_presenter_drag_surface_to(egui_view_t *self, egui_dim_t screen_x, egui_dim_t screen_y)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);
    egui_dim_t delta_x = screen_x - local->drag_anchor_x;
    egui_dim_t delta_y = screen_y - local->drag_anchor_y;

    scroll_presenter_apply_horizontal_offset_inner(self, local->drag_anchor_horizontal_offset - delta_x, 1, EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE);
    scroll_presenter_apply_vertical_offset_inner(self, local->drag_anchor_vertical_offset - delta_y, 1, EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_scroll_presenter_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);

    if (!egui_view_get_enable(self) || local->read_only_mode || local->compact_mode)
    {
        if (scroll_presenter_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }
    scroll_presenter_normalize_state(local);

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        if (!scroll_presenter_hit_surface(local, self, event->location.x, event->location.y))
        {
            if (scroll_presenter_clear_pressed_state(self, local))
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
        local->pressed_part = EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE;
        local->surface_dragging = 1;
        local->drag_anchor_x = event->location.x;
        local->drag_anchor_y = event->location.y;
        local->drag_anchor_vertical_offset = local->vertical_offset;
        local->drag_anchor_horizontal_offset = local->horizontal_offset;
        egui_view_set_pressed(self, true);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (!local->surface_dragging)
        {
            return 0;
        }
        scroll_presenter_drag_surface_to(self, event->location.x, event->location.y);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
        if (!local->surface_dragging)
        {
            return 0;
        }
        scroll_presenter_drag_surface_to(self, event->location.x, event->location.y);
        if (scroll_presenter_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 1;
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (scroll_presenter_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}

static int egui_view_scroll_presenter_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);
    EGUI_UNUSED(event);

    if (scroll_presenter_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

static void scroll_presenter_set_current_snapshot_inner(egui_view_t *self, uint8_t snapshot_index, uint8_t notify)
{
    egui_view_scroll_presenter_t *local = scroll_presenter_local(self);

    if (local->snapshots == NULL || local->snapshot_count == 0)
    {
        local->current_snapshot = 0;
        scroll_presenter_load_current_snapshot(local);
        egui_view_invalidate(self);
        return;
    }
    if (snapshot_index >= local->snapshot_count)
    {
        snapshot_index = 0;
    }
    if (local->current_snapshot == snapshot_index)
    {
        if (scroll_presenter_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_snapshot = snapshot_index;
    scroll_presenter_load_current_snapshot(local);
    scroll_presenter_clear_pressed_state(self, local);
    if (notify)
    {
        scroll_presenter_notify_change(self, local, EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE);
    }
    egui_view_invalidate(self);
}

void egui_view_scroll_presenter_set_snapshots(egui_view_t *self, const egui_view_scroll_presenter_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);

    local->snapshots = snapshots;
    local->snapshot_count = scroll_presenter_clamp_snapshot_count(snapshot_count);
    local->current_snapshot = 0;
    scroll_presenter_load_current_snapshot(local);
    scroll_presenter_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_scroll_presenter_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    scroll_presenter_set_current_snapshot_inner(self, snapshot_index, 1);
}

uint8_t egui_view_scroll_presenter_get_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);
    return local->current_snapshot;
}

void egui_view_scroll_presenter_set_vertical_offset(egui_view_t *self, egui_dim_t offset)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);

    scroll_presenter_clear_pressed_state(self, local);
    scroll_presenter_normalize_state(local);
    scroll_presenter_apply_vertical_offset_inner(self, offset, 1, EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE);
}

egui_dim_t egui_view_scroll_presenter_get_vertical_offset(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);
    scroll_presenter_normalize_state(local);
    return local->vertical_offset;
}

egui_dim_t egui_view_scroll_presenter_get_max_vertical_offset(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);
    scroll_presenter_normalize_state(local);
    return scroll_presenter_get_max_vertical_offset_inner(local);
}

void egui_view_scroll_presenter_set_horizontal_offset(egui_view_t *self, egui_dim_t offset)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);

    scroll_presenter_clear_pressed_state(self, local);
    scroll_presenter_normalize_state(local);
    scroll_presenter_apply_horizontal_offset_inner(self, offset, 1, EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE);
}

egui_dim_t egui_view_scroll_presenter_get_horizontal_offset(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);
    scroll_presenter_normalize_state(local);
    return local->horizontal_offset;
}

egui_dim_t egui_view_scroll_presenter_get_max_horizontal_offset(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);
    scroll_presenter_normalize_state(local);
    return scroll_presenter_get_max_horizontal_offset_inner(local);
}

void egui_view_scroll_presenter_scroll_line(egui_view_t *self, egui_dim_t delta_lines)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);

    scroll_presenter_clear_pressed_state(self, local);
    scroll_presenter_normalize_state(local);
    scroll_presenter_apply_vertical_offset_inner(self, local->vertical_offset + delta_lines * scroll_presenter_get_line_step_inner(local), 1,
                                                 EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE);
}

void egui_view_scroll_presenter_scroll_page(egui_view_t *self, egui_dim_t delta_pages)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);

    scroll_presenter_clear_pressed_state(self, local);
    scroll_presenter_normalize_state(local);
    scroll_presenter_apply_vertical_offset_inner(self, local->vertical_offset + delta_pages * scroll_presenter_get_page_step_inner(local), 1,
                                                 EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE);
}

void egui_view_scroll_presenter_activate_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);

    scroll_presenter_clear_pressed_state(self, local);
    if (local->snapshot_count == 0)
    {
        egui_view_invalidate(self);
        return;
    }
    scroll_presenter_set_current_snapshot_inner(self, (uint8_t)((local->current_snapshot + 1) % local->snapshot_count), 1);
}

void egui_view_scroll_presenter_set_on_view_changed_listener(egui_view_t *self, egui_view_on_scroll_presenter_changed_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);
    local->on_view_changed = listener;
}

void egui_view_scroll_presenter_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);

    scroll_presenter_clear_pressed_state(self, local);
    local->font = font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : font;
    egui_view_invalidate(self);
}

void egui_view_scroll_presenter_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);

    scroll_presenter_clear_pressed_state(self, local);
    local->meta_font = font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : font;
    egui_view_invalidate(self);
}

void egui_view_scroll_presenter_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);
    uint8_t had_pressed;

    compact_mode = compact_mode ? 1 : 0;
    had_pressed = scroll_presenter_clear_pressed_state(self, local);
    if (local->compact_mode != compact_mode || had_pressed)
    {
        local->compact_mode = compact_mode;
        egui_view_invalidate(self);
    }
}

void egui_view_scroll_presenter_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);
    uint8_t had_pressed;

    read_only_mode = read_only_mode ? 1 : 0;
    had_pressed = scroll_presenter_clear_pressed_state(self, local);
    if (local->read_only_mode != read_only_mode || had_pressed)
    {
        local->read_only_mode = read_only_mode;
        egui_view_invalidate(self);
    }
}

void egui_view_scroll_presenter_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t viewport_color,
                                            egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                            egui_color_t preview_color)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);

    scroll_presenter_clear_pressed_state(self, local);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->viewport_color = viewport_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->preview_color = preview_color;
    egui_view_invalidate(self);
}

uint8_t egui_view_scroll_presenter_get_surface_region(egui_view_t *self, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);
    egui_view_scroll_presenter_metrics_t metrics;

    if (region == NULL)
    {
        return 0;
    }

    scroll_presenter_normalize_state(local);
    scroll_presenter_get_metrics(local, self, &metrics);
    *region = metrics.viewport_region;
    region->location.x += self->region_screen.location.x;
    region->location.y += self->region_screen.location.y;
    return 1;
}

uint8_t egui_view_scroll_presenter_handle_navigation_key(egui_view_t *self, uint8_t key_code)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);

    if (scroll_presenter_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    scroll_presenter_normalize_state(local);
    if (!egui_view_get_enable(self) || local->read_only_mode || local->compact_mode)
    {
        return 0;
    }

    switch (key_code)
    {
    case EGUI_KEY_CODE_UP:
        scroll_presenter_apply_vertical_offset_inner(self, local->vertical_offset - scroll_presenter_get_line_step_inner(local), 1,
                                                     EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE);
        return 1;
    case EGUI_KEY_CODE_DOWN:
        scroll_presenter_apply_vertical_offset_inner(self, local->vertical_offset + scroll_presenter_get_line_step_inner(local), 1,
                                                     EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE);
        return 1;
    case EGUI_KEY_CODE_LEFT:
        scroll_presenter_apply_horizontal_offset_inner(self, local->horizontal_offset - scroll_presenter_get_horizontal_step_inner(local), 1,
                                                       EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE);
        return 1;
    case EGUI_KEY_CODE_RIGHT:
        scroll_presenter_apply_horizontal_offset_inner(self, local->horizontal_offset + scroll_presenter_get_horizontal_step_inner(local), 1,
                                                       EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE);
        return 1;
    case EGUI_KEY_CODE_HOME:
        scroll_presenter_apply_horizontal_offset_inner(self, 0, 1, EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE);
        scroll_presenter_apply_vertical_offset_inner(self, 0, 1, EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE);
        return 1;
    case EGUI_KEY_CODE_END:
        scroll_presenter_apply_horizontal_offset_inner(self, scroll_presenter_get_max_horizontal_offset_inner(local), 1,
                                                       EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE);
        scroll_presenter_apply_vertical_offset_inner(self, scroll_presenter_get_max_vertical_offset_inner(local), 1,
                                                     EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE);
        return 1;
    case EGUI_KEY_CODE_MINUS:
        scroll_presenter_apply_vertical_offset_inner(self, local->vertical_offset - scroll_presenter_get_page_step_inner(local), 1,
                                                     EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE);
        return 1;
    case EGUI_KEY_CODE_PLUS:
        scroll_presenter_apply_vertical_offset_inner(self, local->vertical_offset + scroll_presenter_get_page_step_inner(local), 1,
                                                     EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE);
        return 1;
    case EGUI_KEY_CODE_ENTER:
    case EGUI_KEY_CODE_SPACE:
        egui_view_scroll_presenter_activate_current_snapshot(self);
        return 1;
    default:
        return 0;
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_scroll_presenter_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);

    if (!egui_view_get_enable(self) || local->read_only_mode || local->compact_mode)
    {
        if (scroll_presenter_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    if (event->type != EGUI_KEY_EVENT_ACTION_UP)
    {
        switch (event->key_code)
        {
        case EGUI_KEY_CODE_UP:
        case EGUI_KEY_CODE_DOWN:
        case EGUI_KEY_CODE_LEFT:
        case EGUI_KEY_CODE_RIGHT:
        case EGUI_KEY_CODE_HOME:
        case EGUI_KEY_CODE_END:
        case EGUI_KEY_CODE_MINUS:
        case EGUI_KEY_CODE_PLUS:
            return 1;
        case EGUI_KEY_CODE_ENTER:
        case EGUI_KEY_CODE_SPACE:
            local->pressed_part = EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE;
            egui_view_set_pressed(self, true);
            egui_view_invalidate(self);
            return 1;
        default:
            return 0;
        }
    }

    if (egui_view_scroll_presenter_handle_navigation_key(self, event->key_code))
    {
        return 1;
    }
    return egui_view_on_key_event(self, event);
}

static int egui_view_scroll_presenter_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);
    EGUI_UNUSED(event);

    if (scroll_presenter_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static void egui_view_scroll_presenter_on_focus_change(egui_view_t *self, int is_focused)
{
    EGUI_LOCAL_INIT(egui_view_scroll_presenter_t);

    if (is_focused)
    {
        return;
    }
    if (scroll_presenter_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
}
#endif

void egui_view_scroll_presenter_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_scroll_presenter_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_scroll_presenter_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_scroll_presenter_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_scroll_presenter_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_scroll_presenter_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_scroll_presenter_on_key_event,
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        .on_focus_changed = egui_view_scroll_presenter_on_focus_change,
#endif
};

void egui_view_scroll_presenter_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_scroll_presenter_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_scroll_presenter_t);
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
    local->current_part = EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_part = EGUI_VIEW_SCROLL_PRESENTER_PART_NONE;
    local->surface_dragging = 0;
    local->vertical_offset = 0;
    local->horizontal_offset = 0;
    local->drag_anchor_x = 0;
    local->drag_anchor_y = 0;
    local->drag_anchor_vertical_offset = 0;
    local->drag_anchor_horizontal_offset = 0;

    egui_view_set_view_name(self, "egui_view_scroll_presenter");
}
