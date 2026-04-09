#include <string.h>

#include "egui_view_parallax_view.h"
#include "utils/egui_sprintf.h"

#define PV_STD_RADIUS          12
#define PV_STD_PAD_X           8
#define PV_STD_PAD_Y           8
#define PV_STD_HERO_HEIGHT     42
#define PV_STD_ROW_HEIGHT      18
#define PV_STD_ROW_GAP         4
#define PV_STD_FOOTER_HEIGHT   13
#define PV_STD_FOOTER_GAP      6
#define PV_STD_OVERLAP         8
#define PV_STD_TITLE_HEIGHT    12
#define PV_STD_SUBTITLE_HEIGHT 10

#define PV_COMPACT_RADIUS          9
#define PV_COMPACT_PAD_X           5
#define PV_COMPACT_PAD_Y           5
#define PV_COMPACT_HERO_HEIGHT     28
#define PV_COMPACT_ROW_HEIGHT      14
#define PV_COMPACT_ROW_GAP         3
#define PV_COMPACT_FOOTER_HEIGHT   11
#define PV_COMPACT_FOOTER_GAP      4
#define PV_COMPACT_OVERLAP         5
#define PV_COMPACT_TITLE_HEIGHT    10
#define PV_COMPACT_SUBTITLE_HEIGHT 8

typedef struct egui_view_parallax_view_metrics egui_view_parallax_view_metrics_t;
struct egui_view_parallax_view_metrics
{
    egui_region_t content_region;
    egui_region_t hero_region;
    egui_region_t title_region;
    egui_region_t subtitle_region;
    egui_region_t progress_region;
    egui_region_t row_regions[EGUI_VIEW_PARALLAX_VIEW_MAX_ROWS];
    egui_region_t footer_region;
};

static uint8_t parallax_view_clamp_row_count(uint8_t count)
{
    if (count > EGUI_VIEW_PARALLAX_VIEW_MAX_ROWS)
    {
        return EGUI_VIEW_PARALLAX_VIEW_MAX_ROWS;
    }
    return count;
}

static uint8_t parallax_view_has_text(const char *text)
{
    return (text != NULL && text[0] != '\0') ? 1 : 0;
}

static uint8_t parallax_view_text_len(const char *text)
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

static egui_dim_t parallax_view_get_max_offset_inner(const egui_view_parallax_view_t *local)
{
    if (local->content_length <= local->viewport_length)
    {
        return 0;
    }
    return (egui_dim_t)(local->content_length - local->viewport_length);
}

static egui_dim_t parallax_view_clamp_offset_value(const egui_view_parallax_view_t *local, egui_dim_t offset)
{
    egui_dim_t max_offset = parallax_view_get_max_offset_inner(local);

    if (offset < 0)
    {
        return 0;
    }
    if (offset > max_offset)
    {
        return max_offset;
    }
    return offset;
}

static egui_color_t parallax_view_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0xA8B4BF), 54);
}

static void parallax_view_clear_pressed_state(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);

    local->pressed_row = EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE;
    egui_view_set_pressed(self, false);
}

static egui_color_t parallax_view_tone_color(egui_view_parallax_view_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_PARALLAX_VIEW_TONE_SUCCESS:
        return local->success_color;
    case EGUI_VIEW_PARALLAX_VIEW_TONE_WARNING:
        return local->warning_color;
    case EGUI_VIEW_PARALLAX_VIEW_TONE_NEUTRAL:
        return local->neutral_color;
    default:
        return local->accent_color;
    }
}

static void parallax_view_draw_round_fill_safe(egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height, egui_dim_t radius, egui_color_t color,
                                               egui_alpha_t alpha)
{
    if (width <= 0 || height <= 0)
    {
        return;
    }
    if (radius <= 0 || width <= (radius << 1) || height <= (radius << 1))
    {
        egui_canvas_draw_rectangle_fill(x, y, width, height, color, alpha);
        return;
    }
    egui_canvas_draw_round_rectangle_fill(x, y, width, height, radius, color, alpha);
}

static void parallax_view_draw_round_stroke_safe(egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height, egui_dim_t radius, egui_dim_t stroke_width,
                                                 egui_color_t color, egui_alpha_t alpha)
{
    if (width <= 0 || height <= 0)
    {
        return;
    }
    if (radius <= 0 || width <= (radius << 1) || height <= (radius << 1))
    {
        egui_canvas_draw_rectangle(x, y, width, height, stroke_width, color, alpha);
        return;
    }
    egui_canvas_draw_round_rectangle(x, y, width, height, radius, stroke_width, color, alpha);
}

static void parallax_view_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                    egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (!parallax_view_has_text(text) || region == NULL || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, self->alpha);
}

static void parallax_view_format_progress(char *buffer, size_t size, egui_dim_t offset, egui_dim_t max_offset)
{
    int pos = 0;

    if (buffer == NULL || size == 0)
    {
        return;
    }

    buffer[0] = '\0';
    pos += egui_sprintf_int(buffer, (int)size, offset);
    if (pos < (int)size)
    {
        pos += egui_sprintf_str(&buffer[pos], (int)size - pos, "/");
    }
    if (pos < (int)size)
    {
        egui_sprintf_int(&buffer[pos], (int)size - pos, max_offset);
    }
}

static void parallax_view_format_footer(egui_view_parallax_view_t *local, char *buffer, size_t size, uint8_t active_row)
{
    int pos = 0;
    const char *prefix = local->footer_prefix;
    const char *title = NULL;

    if (buffer == NULL || size == 0)
    {
        return;
    }

    if (!parallax_view_has_text(prefix))
    {
        prefix = "Active";
    }
    if (local->rows != NULL && active_row < local->row_count)
    {
        title = local->rows[active_row].title;
    }

    buffer[0] = '\0';
    pos += egui_sprintf_str(buffer, (int)size, prefix);
    if (pos < (int)size && parallax_view_has_text(title))
    {
        pos += egui_sprintf_str(&buffer[pos], (int)size - pos, ": ");
        egui_sprintf_str(&buffer[pos], (int)size - pos, title);
    }
}

static uint8_t parallax_view_get_active_row_inner(egui_view_parallax_view_t *local)
{
    uint8_t active = EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE;
    uint8_t i;

    if (local->rows == NULL || local->row_count == 0)
    {
        return EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE;
    }

    active = 0;
    for (i = 0; i < local->row_count; i++)
    {
        if (local->offset >= local->rows[i].anchor_offset)
        {
            active = i;
        }
        else
        {
            break;
        }
    }

    return active;
}

static void parallax_view_notify_changed(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);

    if (local->on_changed != NULL)
    {
        local->on_changed(self, local->offset, parallax_view_get_active_row_inner(local));
    }
}

static void parallax_view_get_metrics(egui_view_parallax_view_t *local, egui_view_t *self, egui_view_parallax_view_metrics_t *metrics)
{
    egui_region_t work_region;
    egui_dim_t inner_pad_x = local->compact_mode ? PV_COMPACT_PAD_X : PV_STD_PAD_X;
    egui_dim_t inner_pad_y = local->compact_mode ? PV_COMPACT_PAD_Y : PV_STD_PAD_Y;
    egui_dim_t hero_height = local->compact_mode ? PV_COMPACT_HERO_HEIGHT : PV_STD_HERO_HEIGHT;
    egui_dim_t footer_height = local->compact_mode ? PV_COMPACT_FOOTER_HEIGHT : PV_STD_FOOTER_HEIGHT;
    egui_dim_t footer_gap = local->compact_mode ? PV_COMPACT_FOOTER_GAP : PV_STD_FOOTER_GAP;
    egui_dim_t overlap = local->compact_mode ? PV_COMPACT_OVERLAP : PV_STD_OVERLAP;
    egui_dim_t row_height = local->compact_mode ? PV_COMPACT_ROW_HEIGHT : PV_STD_ROW_HEIGHT;
    egui_dim_t row_gap = local->compact_mode ? PV_COMPACT_ROW_GAP : PV_STD_ROW_GAP;
    egui_dim_t title_height = local->compact_mode ? PV_COMPACT_TITLE_HEIGHT : PV_STD_TITLE_HEIGHT;
    egui_dim_t subtitle_height = local->compact_mode ? PV_COMPACT_SUBTITLE_HEIGHT : PV_STD_SUBTITLE_HEIGHT;
    egui_dim_t rows_start_y;
    egui_dim_t rows_limit_y;
    egui_dim_t total_rows_height;
    egui_dim_t available_rows_height;
    egui_dim_t progress_width;
    egui_dim_t progress_height = local->compact_mode ? 10 : 11;
    uint8_t i;

    memset(metrics, 0, sizeof(*metrics));
    egui_view_get_work_region(self, &work_region);

    metrics->content_region.location.x = work_region.location.x + inner_pad_x;
    metrics->content_region.location.y = work_region.location.y + inner_pad_y;
    metrics->content_region.size.width = work_region.size.width - inner_pad_x * 2;
    metrics->content_region.size.height = work_region.size.height - inner_pad_y * 2;

    if (metrics->content_region.size.width <= 0 || metrics->content_region.size.height <= 0)
    {
        return;
    }

    if (hero_height > metrics->content_region.size.height - footer_height - footer_gap - 8)
    {
        hero_height = metrics->content_region.size.height - footer_height - footer_gap - 8;
    }
    if (hero_height < (local->compact_mode ? 16 : 22))
    {
        hero_height = (local->compact_mode ? 16 : 22);
    }

    metrics->hero_region.location.x = metrics->content_region.location.x;
    metrics->hero_region.location.y = metrics->content_region.location.y;
    metrics->hero_region.size.width = metrics->content_region.size.width;
    metrics->hero_region.size.height = hero_height;

    metrics->title_region.location.x = metrics->hero_region.location.x + (local->compact_mode ? 7 : 9);
    metrics->title_region.location.y = metrics->hero_region.location.y + (local->compact_mode ? 5 : 7);
    metrics->title_region.size.width = metrics->hero_region.size.width - (local->compact_mode ? 14 : 18);
    metrics->title_region.size.height = title_height;

    metrics->subtitle_region.location.x = metrics->title_region.location.x;
    metrics->subtitle_region.location.y = metrics->title_region.location.y + title_height + 2;
    metrics->subtitle_region.size.width = metrics->title_region.size.width;
    metrics->subtitle_region.size.height = subtitle_height;

    progress_width = (egui_dim_t)(26 + parallax_view_text_len("999/999") * 4);
    if (progress_width > metrics->hero_region.size.width - 16)
    {
        progress_width = metrics->hero_region.size.width - 16;
    }
    metrics->progress_region.size.width = progress_width;
    metrics->progress_region.size.height = progress_height;
    metrics->progress_region.location.x = metrics->hero_region.location.x + metrics->hero_region.size.width - progress_width - (local->compact_mode ? 7 : 9);
    metrics->progress_region.location.y = metrics->hero_region.location.y + (local->compact_mode ? 5 : 7);

    metrics->footer_region.size.height = footer_height;
    metrics->footer_region.size.width = metrics->content_region.size.width;
    metrics->footer_region.location.x = metrics->content_region.location.x;
    metrics->footer_region.location.y = metrics->content_region.location.y + metrics->content_region.size.height - footer_height;

    rows_start_y = metrics->hero_region.location.y + metrics->hero_region.size.height - overlap;
    rows_limit_y = metrics->footer_region.location.y - footer_gap;
    available_rows_height = rows_limit_y - rows_start_y;
    total_rows_height = local->row_count > 0 ? (egui_dim_t)(local->row_count * row_height + (local->row_count - 1) * row_gap) : 0;

    while (local->row_count > 0 && total_rows_height > available_rows_height && row_gap > 1)
    {
        row_gap--;
        total_rows_height = (egui_dim_t)(local->row_count * row_height + (local->row_count - 1) * row_gap);
    }
    while (local->row_count > 0 && total_rows_height > available_rows_height && row_height > (local->compact_mode ? 11 : 13))
    {
        row_height--;
        total_rows_height = (egui_dim_t)(local->row_count * row_height + (local->row_count - 1) * row_gap);
    }

    for (i = 0; i < local->row_count; i++)
    {
        metrics->row_regions[i].location.x = metrics->content_region.location.x;
        metrics->row_regions[i].location.y = rows_start_y + i * (row_height + row_gap);
        metrics->row_regions[i].size.width = metrics->content_region.size.width;
        metrics->row_regions[i].size.height = row_height;
    }
}

static void parallax_view_set_offset_inner(egui_view_t *self, egui_dim_t offset, uint8_t notify)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);
    egui_dim_t old_offset = local->offset;
    uint8_t old_active = parallax_view_get_active_row_inner(local);
    uint8_t new_active;

    local->offset = parallax_view_clamp_offset_value(local, offset);
    new_active = parallax_view_get_active_row_inner(local);
    if (old_offset != local->offset || old_active != new_active)
    {
        egui_view_invalidate(self);
        if (notify)
        {
            parallax_view_notify_changed(self);
        }
    }
}

static uint8_t parallax_view_hit_row(egui_view_parallax_view_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    egui_view_parallax_view_metrics_t metrics;
    uint8_t i;

    parallax_view_get_metrics(local, self, &metrics);
    for (i = 0; i < local->row_count; i++)
    {
        if (egui_region_pt_in_rect(&metrics.row_regions[i], x, y))
        {
            return i;
        }
    }
    return EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE;
}

static void parallax_view_draw_hero_layers(egui_view_t *self, egui_view_parallax_view_t *local, const egui_region_t *hero_region, egui_color_t tone_color)
{
    egui_dim_t max_offset = parallax_view_get_max_offset_inner(local);
    egui_dim_t shift = max_offset == 0 ? 0 : (egui_dim_t)((local->offset * local->vertical_shift) / max_offset);
    egui_color_t layer_color_0 = egui_rgb_mix(local->panel_color, tone_color, 4);
    egui_color_t layer_color_1 = egui_rgb_mix(local->panel_color, tone_color, 7);
    egui_color_t layer_color_2 = egui_rgb_mix(local->panel_color, tone_color, 10);
    egui_dim_t y0 = hero_region->location.y + (local->compact_mode ? 7 : 10) - shift / 3;
    egui_dim_t y1 = hero_region->location.y + (local->compact_mode ? 13 : 18) - shift / 2;
    egui_dim_t y2 = hero_region->location.y + (local->compact_mode ? 18 : 25) - shift;

    if (local->read_only_mode)
    {
        layer_color_0 = egui_rgb_mix(layer_color_0, local->surface_color, 40);
        layer_color_1 = egui_rgb_mix(layer_color_1, local->surface_color, 44);
        layer_color_2 = egui_rgb_mix(layer_color_2, local->surface_color, 48);
    }
    if (!egui_view_get_enable(self))
    {
        layer_color_0 = parallax_view_mix_disabled(layer_color_0);
        layer_color_1 = parallax_view_mix_disabled(layer_color_1);
        layer_color_2 = parallax_view_mix_disabled(layer_color_2);
    }

    parallax_view_draw_round_fill_safe(hero_region->location.x + (local->compact_mode ? 8 : 12), y0, hero_region->size.width - (local->compact_mode ? 20 : 30),
                                       local->compact_mode ? 4 : 6, local->compact_mode ? 3 : 4, layer_color_0, egui_color_alpha_mix(self->alpha, 42));
    parallax_view_draw_round_fill_safe(hero_region->location.x + (local->compact_mode ? 14 : 22), y1, hero_region->size.width - (local->compact_mode ? 34 : 54),
                                       local->compact_mode ? 5 : 8, local->compact_mode ? 3 : 4, layer_color_1, egui_color_alpha_mix(self->alpha, 50));
    parallax_view_draw_round_fill_safe(hero_region->location.x + (local->compact_mode ? 20 : 34), y2, hero_region->size.width - (local->compact_mode ? 50 : 82),
                                       local->compact_mode ? 6 : 10, local->compact_mode ? 3 : 5, layer_color_2, egui_color_alpha_mix(self->alpha, 58));
}

static void parallax_view_draw_row(egui_view_t *self, egui_view_parallax_view_t *local, const egui_region_t *row_region,
                                   const egui_view_parallax_view_row_t *row, uint8_t active, uint8_t pressed)
{
    egui_region_t text_region;
    egui_color_t tone_color = parallax_view_tone_color(local, row->tone);
    egui_color_t fill_color = egui_rgb_mix(local->surface_color, tone_color, active ? (local->compact_mode ? 4 : 6) : 2);
    egui_color_t border_color = egui_rgb_mix(local->border_color, tone_color, active ? (local->compact_mode ? 10 : 14) : 5);
    egui_color_t title_color = active ? egui_rgb_mix(local->text_color, tone_color, 4) : local->text_color;
    egui_color_t meta_fill = egui_rgb_mix(local->panel_color, tone_color, active ? (local->compact_mode ? 5 : 7) : 3);
    egui_color_t meta_border = egui_rgb_mix(local->border_color, tone_color, active ? (local->compact_mode ? 7 : 9) : 5);
    egui_color_t meta_color = egui_rgb_mix(local->muted_text_color, tone_color, active ? (local->compact_mode ? 7 : 9) : 5);
    egui_dim_t radius = local->compact_mode ? 5 : 7;
    egui_dim_t meta_height = local->compact_mode ? 7 : 9;
    egui_dim_t meta_width = parallax_view_has_text(row->meta)
                                    ? (egui_dim_t)(parallax_view_text_len(row->meta) * (local->compact_mode ? 4 : 5) + (local->compact_mode ? 10 : 14))
                                    : 0;
    egui_dim_t meta_x = row_region->location.x + row_region->size.width - meta_width - (local->compact_mode ? 5 : 7);
    egui_dim_t title_x = row_region->location.x + (local->compact_mode ? 9 : 11);

    if (local->read_only_mode)
    {
        fill_color = egui_rgb_mix(fill_color, local->surface_color, 30);
        border_color = egui_rgb_mix(border_color, local->muted_text_color, 16);
        title_color = egui_rgb_mix(title_color, local->muted_text_color, 24);
        meta_fill = egui_rgb_mix(meta_fill, local->surface_color, 34);
        meta_border = egui_rgb_mix(meta_border, local->muted_text_color, 16);
        meta_color = egui_rgb_mix(meta_color, local->muted_text_color, 28);
        tone_color = egui_rgb_mix(tone_color, local->muted_text_color, 36);
    }
    if (!egui_view_get_enable(self))
    {
        fill_color = parallax_view_mix_disabled(fill_color);
        border_color = parallax_view_mix_disabled(border_color);
        title_color = parallax_view_mix_disabled(title_color);
        meta_fill = parallax_view_mix_disabled(meta_fill);
        meta_border = parallax_view_mix_disabled(meta_border);
        meta_color = parallax_view_mix_disabled(meta_color);
        tone_color = parallax_view_mix_disabled(tone_color);
    }

    parallax_view_draw_round_fill_safe(row_region->location.x, row_region->location.y, row_region->size.width, row_region->size.height, radius, fill_color,
                                       egui_color_alpha_mix(self->alpha, pressed ? 82 : 92));
    parallax_view_draw_round_stroke_safe(row_region->location.x, row_region->location.y, row_region->size.width, row_region->size.height, radius, 1,
                                         border_color, egui_color_alpha_mix(self->alpha, active ? 38 : 24));
    parallax_view_draw_round_fill_safe(row_region->location.x + 3, row_region->location.y + 3, local->compact_mode ? 2 : 3, row_region->size.height - 6, 1,
                                       tone_color, egui_color_alpha_mix(self->alpha, active ? 78 : 42));

    if (meta_width > 0)
    {
        parallax_view_draw_round_fill_safe(meta_x, row_region->location.y + (row_region->size.height - meta_height) / 2, meta_width, meta_height,
                                           meta_height / 2, meta_fill, egui_color_alpha_mix(self->alpha, 82));
        parallax_view_draw_round_stroke_safe(meta_x, row_region->location.y + (row_region->size.height - meta_height) / 2, meta_width, meta_height,
                                             meta_height / 2, 1, meta_border, egui_color_alpha_mix(self->alpha, 28));
        text_region.location.x = meta_x;
        text_region.location.y = row_region->location.y + (row_region->size.height - meta_height) / 2;
        text_region.size.width = meta_width;
        text_region.size.height = meta_height;
        parallax_view_draw_text(local->meta_font, self, row->meta, &text_region, EGUI_ALIGN_CENTER, meta_color);
    }

    text_region.location.x = title_x;
    text_region.location.y = row_region->location.y;
    text_region.size.width = meta_width > 0 ? (meta_x - title_x - 4) : (row_region->size.width - (local->compact_mode ? 18 : 22));
    text_region.size.height = row_region->size.height;
    parallax_view_draw_text(local->font, self, row->title, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);
}

void egui_view_parallax_view_set_rows(egui_view_t *self, const egui_view_parallax_view_row_t *rows, uint8_t row_count)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);

    local->rows = rows;
    local->row_count = parallax_view_clamp_row_count(row_count);
    parallax_view_clear_pressed_state(self);
    local->offset = parallax_view_clamp_offset_value(local, local->offset);
    parallax_view_notify_changed(self);
    egui_view_invalidate(self);
}

void egui_view_parallax_view_set_header(egui_view_t *self, const char *title, const char *subtitle, const char *footer_prefix)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);

    local->title = title;
    local->subtitle = subtitle;
    local->footer_prefix = footer_prefix;
    egui_view_invalidate(self);
}

void egui_view_parallax_view_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);
    local->font = font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : font;
    egui_view_invalidate(self);
}

void egui_view_parallax_view_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);
    local->meta_font = font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : font;
    egui_view_invalidate(self);
}

void egui_view_parallax_view_set_on_changed_listener(egui_view_t *self, egui_view_on_parallax_view_changed_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);
    local->on_changed = listener;
}

void egui_view_parallax_view_set_content_metrics(egui_view_t *self, egui_dim_t content_length, egui_dim_t viewport_length)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);

    local->content_length = content_length < 0 ? 0 : content_length;
    local->viewport_length = viewport_length < 0 ? 0 : viewport_length;
    parallax_view_set_offset_inner(self, local->offset, 1);
}

egui_dim_t egui_view_parallax_view_get_content_length(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);
    return local->content_length;
}

egui_dim_t egui_view_parallax_view_get_viewport_length(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);
    return local->viewport_length;
}

void egui_view_parallax_view_set_offset(egui_view_t *self, egui_dim_t offset)
{
    parallax_view_set_offset_inner(self, offset, 1);
}

egui_dim_t egui_view_parallax_view_get_offset(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);
    return local->offset;
}

egui_dim_t egui_view_parallax_view_get_max_offset(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);
    return parallax_view_get_max_offset_inner(local);
}

void egui_view_parallax_view_set_vertical_shift(egui_view_t *self, egui_dim_t vertical_shift)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);
    local->vertical_shift = vertical_shift < 0 ? 0 : vertical_shift;
    egui_view_invalidate(self);
}

egui_dim_t egui_view_parallax_view_get_vertical_shift(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);
    return local->vertical_shift;
}

void egui_view_parallax_view_set_step_size(egui_view_t *self, egui_dim_t line_step, egui_dim_t page_step)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);

    local->line_step = line_step <= 0 ? 1 : line_step;
    local->page_step = page_step <= 0 ? 1 : page_step;
}

void egui_view_parallax_view_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);

    local->compact_mode = compact_mode ? 1 : 0;
    parallax_view_clear_pressed_state(self);
    egui_view_invalidate(self);
}

void egui_view_parallax_view_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);

    local->read_only_mode = read_only_mode ? 1 : 0;
    parallax_view_clear_pressed_state(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_parallax_view_get_active_row(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);
    return parallax_view_get_active_row_inner(local);
}

uint8_t egui_view_parallax_view_get_row_region(egui_view_t *self, uint8_t row_index, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);
    egui_view_parallax_view_metrics_t metrics;

    if (region == NULL || row_index >= local->row_count)
    {
        return 0;
    }

    parallax_view_get_metrics(local, self, &metrics);
    *region = metrics.row_regions[row_index];
    return 1;
}

static void egui_view_parallax_view_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);
    egui_region_t region;
    egui_region_t text_region;
    egui_view_parallax_view_metrics_t metrics;
    char progress_text[24];
    char footer_text[64];
    uint8_t active_row;
    const egui_view_parallax_view_row_t *active = NULL;
    egui_color_t tone_color;
    egui_color_t card_fill;
    egui_color_t card_border;
    egui_color_t hero_fill;
    egui_color_t hero_border;
    egui_color_t footer_fill;
    egui_color_t footer_border;
    egui_color_t title_color = local->text_color;
    egui_color_t subtitle_color = local->muted_text_color;
    egui_color_t footer_color;
    egui_dim_t radius = local->compact_mode ? PV_COMPACT_RADIUS : PV_STD_RADIUS;
    egui_dim_t track_x;
    egui_dim_t track_y;
    egui_dim_t track_width;
    egui_dim_t thumb_width;
    egui_dim_t thumb_x;
    egui_dim_t max_offset;
    uint8_t i;

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0 || local->rows == NULL || local->row_count == 0)
    {
        return;
    }

    active_row = parallax_view_get_active_row_inner(local);
    if (active_row != EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE)
    {
        active = &local->rows[active_row];
    }
    tone_color = active == NULL ? local->accent_color : parallax_view_tone_color(local, active->tone);
    card_fill = egui_rgb_mix(local->surface_color, tone_color, local->compact_mode ? 2 : 3);
    card_border = egui_rgb_mix(local->border_color, tone_color, local->compact_mode ? 3 : 5);
    hero_fill = egui_rgb_mix(local->panel_color, tone_color, local->compact_mode ? 4 : 6);
    hero_border = egui_rgb_mix(local->border_color, tone_color, local->compact_mode ? 6 : 9);
    footer_fill = egui_rgb_mix(local->panel_color, tone_color, local->compact_mode ? 3 : 5);
    footer_border = egui_rgb_mix(local->border_color, tone_color, local->compact_mode ? 6 : 9);
    footer_color = egui_rgb_mix(local->text_color, tone_color, local->compact_mode ? 2 : 4);

    if (local->read_only_mode)
    {
        tone_color = egui_rgb_mix(tone_color, local->muted_text_color, 38);
        card_fill = egui_rgb_mix(card_fill, local->surface_color, 24);
        card_border = egui_rgb_mix(card_border, local->muted_text_color, 16);
        hero_fill = egui_rgb_mix(hero_fill, local->surface_color, 28);
        hero_border = egui_rgb_mix(hero_border, local->muted_text_color, 16);
        footer_fill = egui_rgb_mix(footer_fill, local->surface_color, 30);
        footer_border = egui_rgb_mix(footer_border, local->muted_text_color, 16);
        title_color = egui_rgb_mix(title_color, local->muted_text_color, 22);
        subtitle_color = egui_rgb_mix(subtitle_color, local->muted_text_color, 24);
        footer_color = egui_rgb_mix(footer_color, local->muted_text_color, 28);
    }
    if (!egui_view_get_enable(self))
    {
        tone_color = parallax_view_mix_disabled(tone_color);
        card_fill = parallax_view_mix_disabled(card_fill);
        card_border = parallax_view_mix_disabled(card_border);
        hero_fill = parallax_view_mix_disabled(hero_fill);
        hero_border = parallax_view_mix_disabled(hero_border);
        footer_fill = parallax_view_mix_disabled(footer_fill);
        footer_border = parallax_view_mix_disabled(footer_border);
        title_color = parallax_view_mix_disabled(title_color);
        subtitle_color = parallax_view_mix_disabled(subtitle_color);
        footer_color = parallax_view_mix_disabled(footer_color);
    }

    parallax_view_get_metrics(local, self, &metrics);
    parallax_view_draw_round_fill_safe(region.location.x, region.location.y, region.size.width, region.size.height, radius, card_fill,
                                       egui_color_alpha_mix(self->alpha, 96));
    parallax_view_draw_round_stroke_safe(region.location.x, region.location.y, region.size.width, region.size.height, radius, 1, card_border,
                                         egui_color_alpha_mix(self->alpha, 40));

    parallax_view_draw_round_fill_safe(metrics.hero_region.location.x, metrics.hero_region.location.y, metrics.hero_region.size.width,
                                       metrics.hero_region.size.height, local->compact_mode ? 7 : 9, hero_fill, egui_color_alpha_mix(self->alpha, 92));
    parallax_view_draw_round_stroke_safe(metrics.hero_region.location.x, metrics.hero_region.location.y, metrics.hero_region.size.width,
                                         metrics.hero_region.size.height, local->compact_mode ? 7 : 9, 1, hero_border, egui_color_alpha_mix(self->alpha, 28));
    parallax_view_draw_hero_layers(self, local, &metrics.hero_region, tone_color);

    parallax_view_format_progress(progress_text, sizeof(progress_text), local->offset, parallax_view_get_max_offset_inner(local));
    parallax_view_draw_round_fill_safe(metrics.progress_region.location.x, metrics.progress_region.location.y, metrics.progress_region.size.width,
                                       metrics.progress_region.size.height, metrics.progress_region.size.height / 2,
                                       egui_rgb_mix(local->surface_color, tone_color, 8), egui_color_alpha_mix(self->alpha, 84));
    parallax_view_draw_round_stroke_safe(metrics.progress_region.location.x, metrics.progress_region.location.y, metrics.progress_region.size.width,
                                         metrics.progress_region.size.height, metrics.progress_region.size.height / 2, 1,
                                         egui_rgb_mix(local->border_color, tone_color, 10), egui_color_alpha_mix(self->alpha, 26));
    parallax_view_draw_text(local->meta_font, self, progress_text, &metrics.progress_region, EGUI_ALIGN_CENTER, tone_color);

    parallax_view_draw_text(local->font, self, local->title, &metrics.title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);
    parallax_view_draw_text(local->meta_font, self, local->subtitle, &metrics.subtitle_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, subtitle_color);

    max_offset = parallax_view_get_max_offset_inner(local);
    track_x = metrics.hero_region.location.x + (local->compact_mode ? 7 : 9);
    track_y = metrics.hero_region.location.y + metrics.hero_region.size.height - (local->compact_mode ? 8 : 10);
    track_width = metrics.hero_region.size.width - (local->compact_mode ? 14 : 18);
    thumb_width = local->content_length <= 0 ? track_width : (egui_dim_t)((track_width * local->viewport_length) / local->content_length);
    if (thumb_width < (local->compact_mode ? 18 : 24))
    {
        thumb_width = local->compact_mode ? 18 : 24;
    }
    if (thumb_width > track_width)
    {
        thumb_width = track_width;
    }
    thumb_x = track_x;
    if (max_offset > 0 && track_width > thumb_width)
    {
        thumb_x = track_x + (egui_dim_t)(((track_width - thumb_width) * local->offset) / max_offset);
    }
    parallax_view_draw_round_fill_safe(track_x, track_y, track_width, local->compact_mode ? 2 : 3, 1, egui_rgb_mix(local->surface_color, tone_color, 4),
                                       egui_color_alpha_mix(self->alpha, 44));
    parallax_view_draw_round_fill_safe(thumb_x, track_y, thumb_width, local->compact_mode ? 2 : 3, 1, tone_color, egui_color_alpha_mix(self->alpha, 76));

    for (i = 0; i < local->row_count; i++)
    {
        parallax_view_draw_row(self, local, &metrics.row_regions[i], &local->rows[i], i == active_row, local->pressed_row == i);
    }

    parallax_view_draw_round_fill_safe(metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                       metrics.footer_region.size.height, metrics.footer_region.size.height / 2, footer_fill,
                                       egui_color_alpha_mix(self->alpha, 84));
    parallax_view_draw_round_stroke_safe(metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                         metrics.footer_region.size.height, metrics.footer_region.size.height / 2, 1, footer_border,
                                         egui_color_alpha_mix(self->alpha, 24));
    parallax_view_format_footer(local, footer_text, sizeof(footer_text), active_row == EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE ? 0 : active_row);
    text_region.location.x = metrics.footer_region.location.x + (local->compact_mode ? 4 : 6);
    text_region.location.y = metrics.footer_region.location.y;
    text_region.size.width = metrics.footer_region.size.width - (local->compact_mode ? 8 : 12);
    text_region.size.height = metrics.footer_region.size.height;
    parallax_view_draw_text(local->meta_font, self, footer_text, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, footer_color);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_parallax_view_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);
    uint8_t hit_row;

    if (!egui_view_get_enable(self) || local->compact_mode || local->read_only_mode)
    {
        if (self->is_pressed || local->pressed_row != EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE)
        {
            parallax_view_clear_pressed_state(self);
            egui_view_invalidate(self);
        }
        EGUI_UNUSED(event);
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_row = parallax_view_hit_row(local, self, event->location.x, event->location.y);
        if (hit_row == EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE)
        {
            return 0;
        }
        local->pressed_row = hit_row;
        egui_view_set_pressed(self, true);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
        hit_row = parallax_view_hit_row(local, self, event->location.x, event->location.y);
        if (hit_row != EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE && hit_row == local->pressed_row)
        {
            parallax_view_set_offset_inner(self, local->rows[hit_row].anchor_offset, 1);
        }
        parallax_view_clear_pressed_state(self);
        egui_view_invalidate(self);
        return hit_row != EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE;
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        parallax_view_clear_pressed_state(self);
        egui_view_invalidate(self);
        return 1;
    default:
        return 0;
    }
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_parallax_view_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_parallax_view_t);

    if (!egui_view_get_enable(self) || event->type != EGUI_KEY_EVENT_ACTION_UP || local->compact_mode || local->read_only_mode)
    {
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_UP:
        parallax_view_set_offset_inner(self, local->offset - local->line_step, 1);
        return 1;
    case EGUI_KEY_CODE_DOWN:
        parallax_view_set_offset_inner(self, local->offset + local->line_step, 1);
        return 1;
    case EGUI_KEY_CODE_HOME:
        parallax_view_set_offset_inner(self, 0, 1);
        return 1;
    case EGUI_KEY_CODE_END:
        parallax_view_set_offset_inner(self, parallax_view_get_max_offset_inner(local), 1);
        return 1;
    case EGUI_KEY_CODE_MINUS:
        parallax_view_set_offset_inner(self, local->offset - local->page_step, 1);
        return 1;
    case EGUI_KEY_CODE_PLUS:
        parallax_view_set_offset_inner(self, local->offset + local->page_step, 1);
        return 1;
    default:
        return egui_view_on_key_event(self, event);
    }
}
#endif

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_parallax_view_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_parallax_view_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_parallax_view_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_parallax_view_on_key_event,
#endif
};

void egui_view_parallax_view_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_parallax_view_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_parallax_view_t);
    self->is_clickable = true;
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif

    local->rows = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->on_changed = NULL;
    local->title = "Parallax";
    local->subtitle = "Hero depth";
    local->footer_prefix = "Active";
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->panel_color = EGUI_COLOR_HEX(0xF7F8FA);
    local->border_color = EGUI_COLOR_HEX(0xD2DBE3);
    local->text_color = EGUI_COLOR_HEX(0x1B2734);
    local->muted_text_color = EGUI_COLOR_HEX(0x6B7A89);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->success_color = EGUI_COLOR_HEX(0x0F7B45);
    local->warning_color = EGUI_COLOR_HEX(0x9D5D00);
    local->neutral_color = EGUI_COLOR_HEX(0x7A8796);
    local->content_length = 720;
    local->viewport_length = 160;
    local->offset = 0;
    local->vertical_shift = 18;
    local->line_step = 60;
    local->page_step = 180;
    local->row_count = 0;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_row = EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE;

    egui_view_set_view_name(self, "egui_view_parallax_view");
}
