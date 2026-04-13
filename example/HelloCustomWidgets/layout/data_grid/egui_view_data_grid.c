#include "egui_view_data_grid.h"

#define EGUI_VIEW_DATA_GRID_STANDARD_RADIUS        10
#define EGUI_VIEW_DATA_GRID_STANDARD_OUTER_PAD_X   8
#define EGUI_VIEW_DATA_GRID_STANDARD_OUTER_PAD_Y   8
#define EGUI_VIEW_DATA_GRID_STANDARD_INNER_PAD_X   8
#define EGUI_VIEW_DATA_GRID_STANDARD_INNER_PAD_Y   7
#define EGUI_VIEW_DATA_GRID_STANDARD_BADGE_H       11
#define EGUI_VIEW_DATA_GRID_STANDARD_BADGE_GAP     5
#define EGUI_VIEW_DATA_GRID_STANDARD_TITLE_H       12
#define EGUI_VIEW_DATA_GRID_STANDARD_SUMMARY_H     10
#define EGUI_VIEW_DATA_GRID_STANDARD_TITLE_GAP     2
#define EGUI_VIEW_DATA_GRID_STANDARD_TABLE_GAP     7
#define EGUI_VIEW_DATA_GRID_STANDARD_TABLE_RADIUS  8
#define EGUI_VIEW_DATA_GRID_STANDARD_HEADER_H      14
#define EGUI_VIEW_DATA_GRID_STANDARD_ROW_H         15
#define EGUI_VIEW_DATA_GRID_STANDARD_FOOTER_H      11
#define EGUI_VIEW_DATA_GRID_STANDARD_FOOTER_GAP    5
#define EGUI_VIEW_DATA_GRID_STANDARD_CELL_PAD_X    6

#define EGUI_VIEW_DATA_GRID_COMPACT_RADIUS        8
#define EGUI_VIEW_DATA_GRID_COMPACT_OUTER_PAD_X   6
#define EGUI_VIEW_DATA_GRID_COMPACT_OUTER_PAD_Y   6
#define EGUI_VIEW_DATA_GRID_COMPACT_INNER_PAD_X   6
#define EGUI_VIEW_DATA_GRID_COMPACT_INNER_PAD_Y   5
#define EGUI_VIEW_DATA_GRID_COMPACT_BADGE_H       8
#define EGUI_VIEW_DATA_GRID_COMPACT_BADGE_GAP     4
#define EGUI_VIEW_DATA_GRID_COMPACT_TITLE_H       10
#define EGUI_VIEW_DATA_GRID_COMPACT_SUMMARY_H     0
#define EGUI_VIEW_DATA_GRID_COMPACT_TITLE_GAP     0
#define EGUI_VIEW_DATA_GRID_COMPACT_TABLE_GAP     4
#define EGUI_VIEW_DATA_GRID_COMPACT_TABLE_RADIUS  6
#define EGUI_VIEW_DATA_GRID_COMPACT_HEADER_H      10
#define EGUI_VIEW_DATA_GRID_COMPACT_ROW_H         11
#define EGUI_VIEW_DATA_GRID_COMPACT_FOOTER_H      0
#define EGUI_VIEW_DATA_GRID_COMPACT_FOOTER_GAP    0
#define EGUI_VIEW_DATA_GRID_COMPACT_CELL_PAD_X    4

typedef struct egui_view_data_grid_metrics egui_view_data_grid_metrics_t;
struct egui_view_data_grid_metrics
{
    egui_region_t region;
    egui_region_t content_region;
    egui_region_t card_region;
    egui_region_t badge_region;
    egui_region_t title_region;
    egui_region_t summary_region;
    egui_region_t table_region;
    egui_region_t header_region;
    egui_region_t row_regions[EGUI_VIEW_DATA_GRID_MAX_ROWS];
    egui_region_t footer_region;
};

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_data_grid_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event);
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_data_grid_on_static_key_event(egui_view_t *self, egui_key_event_t *event);
#endif

static uint8_t egui_view_data_grid_clamp_snapshot_count(uint8_t count)
{
    return count > EGUI_VIEW_DATA_GRID_MAX_SNAPSHOTS ? EGUI_VIEW_DATA_GRID_MAX_SNAPSHOTS : count;
}

static uint8_t egui_view_data_grid_clamp_row_count(uint8_t count)
{
    return count > EGUI_VIEW_DATA_GRID_MAX_ROWS ? EGUI_VIEW_DATA_GRID_MAX_ROWS : count;
}

static uint8_t egui_view_data_grid_clamp_column_count(uint8_t count)
{
    return count > EGUI_VIEW_DATA_GRID_MAX_COLS ? EGUI_VIEW_DATA_GRID_MAX_COLS : count;
}

static uint8_t egui_view_data_grid_text_len(const char *text)
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

static uint8_t egui_view_data_grid_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static egui_color_t egui_view_data_grid_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 68);
}

static const egui_view_data_grid_snapshot_t *egui_view_data_grid_get_snapshot(egui_view_data_grid_t *local)
{
    if (local->snapshots == NULL || local->snapshot_count == 0 || local->current_snapshot >= local->snapshot_count)
    {
        return NULL;
    }
    return &local->snapshots[local->current_snapshot];
}

static uint8_t egui_view_data_grid_get_row_count(const egui_view_data_grid_snapshot_t *snapshot)
{
    if (snapshot == NULL || snapshot->rows == NULL)
    {
        return 0;
    }
    return egui_view_data_grid_clamp_row_count(snapshot->row_count);
}

static uint8_t egui_view_data_grid_get_column_count(const egui_view_data_grid_snapshot_t *snapshot)
{
    if (snapshot == NULL || snapshot->columns == NULL)
    {
        return 0;
    }
    return egui_view_data_grid_clamp_column_count(snapshot->column_count);
}

static const egui_view_data_grid_row_t *egui_view_data_grid_get_row(const egui_view_data_grid_snapshot_t *snapshot, uint8_t row_index)
{
    uint8_t row_count = egui_view_data_grid_get_row_count(snapshot);

    if (row_index >= row_count)
    {
        return NULL;
    }
    return &snapshot->rows[row_index];
}

static egui_color_t egui_view_data_grid_tone_color(egui_view_data_grid_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_DATA_GRID_TONE_SUCCESS:
        return local->success_color;
    case EGUI_VIEW_DATA_GRID_TONE_WARNING:
        return local->warning_color;
    case EGUI_VIEW_DATA_GRID_TONE_NEUTRAL:
        return local->neutral_color;
    case EGUI_VIEW_DATA_GRID_TONE_ACCENT:
    default:
        return local->accent_color;
    }
}

static uint8_t egui_view_data_grid_clear_pressed_state(egui_view_t *self, egui_view_data_grid_t *local)
{
    uint8_t had_pressed = self->is_pressed || local->pressed_row != EGUI_VIEW_DATA_GRID_ROW_NONE;

    local->pressed_row = EGUI_VIEW_DATA_GRID_ROW_NONE;
    egui_view_set_pressed(self, false);
    return had_pressed;
}

static uint8_t egui_view_data_grid_row_exists(const egui_view_data_grid_snapshot_t *snapshot, uint8_t row_index)
{
    return row_index < egui_view_data_grid_get_row_count(snapshot) ? 1 : 0;
}

static uint8_t egui_view_data_grid_resolve_default_row(const egui_view_data_grid_snapshot_t *snapshot)
{
    uint8_t row_count = egui_view_data_grid_get_row_count(snapshot);

    if (row_count == 0)
    {
        return EGUI_VIEW_DATA_GRID_ROW_NONE;
    }
    return snapshot->selected_row < row_count ? snapshot->selected_row : 0;
}

static void egui_view_data_grid_sync_current_row(egui_view_data_grid_t *local)
{
    const egui_view_data_grid_snapshot_t *snapshot = egui_view_data_grid_get_snapshot(local);

    if (!egui_view_data_grid_row_exists(snapshot, local->current_row))
    {
        local->current_row = egui_view_data_grid_resolve_default_row(snapshot);
    }
}

static uint8_t egui_view_data_grid_row_is_interactive(egui_view_data_grid_t *local, egui_view_t *self,
                                                       const egui_view_data_grid_snapshot_t *snapshot, uint8_t row_index)
{
    if (snapshot == NULL || local->read_only_mode || !egui_view_get_enable(self))
    {
        return 0;
    }
    return egui_view_data_grid_row_exists(snapshot, row_index);
}

static egui_dim_t egui_view_data_grid_pill_width(const char *text, uint8_t compact_mode, egui_dim_t min_width, egui_dim_t max_width)
{
    egui_dim_t width = min_width;

    if (egui_view_data_grid_has_text(text))
    {
        width += egui_view_data_grid_text_len(text) * (compact_mode ? 4 : 5);
    }
    if (width > max_width)
    {
        width = max_width;
    }
    return width;
}

static uint8_t egui_view_data_grid_resolve_text_align(uint8_t align)
{
    switch (align)
    {
    case EGUI_VIEW_DATA_GRID_ALIGN_RIGHT:
        return EGUI_ALIGN_RIGHT | EGUI_ALIGN_VCENTER;
    case EGUI_VIEW_DATA_GRID_ALIGN_CENTER:
        return EGUI_ALIGN_CENTER;
    case EGUI_VIEW_DATA_GRID_ALIGN_LEFT:
    default:
        return EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER;
    }
}

static void egui_view_data_grid_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                          egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (!egui_view_data_grid_has_text(text) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, self->alpha);
}

static void egui_view_data_grid_get_column_region(const egui_region_t *row_region, uint8_t column_count, uint8_t column_index, egui_region_t *region)
{
    egui_dim_t base_width;
    egui_dim_t remainder;
    egui_dim_t x = row_region->location.x;
    uint8_t i;

    region->location.x = 0;
    region->location.y = row_region->location.y;
    region->size.width = 0;
    region->size.height = row_region->size.height;

    if (column_count == 0 || column_index >= column_count)
    {
        return;
    }

    base_width = row_region->size.width / column_count;
    remainder = row_region->size.width % column_count;
    for (i = 0; i < column_index; ++i)
    {
        x += base_width;
        if (remainder > 0)
        {
            x += 1;
            remainder--;
        }
    }

    region->location.x = x;
    region->size.width = base_width;
    if (remainder > 0)
    {
        region->size.width += 1;
    }
}

static void egui_view_data_grid_get_metrics(egui_view_data_grid_t *local, egui_view_t *self,
                                            const egui_view_data_grid_snapshot_t *snapshot, egui_view_data_grid_metrics_t *metrics)
{
    egui_dim_t outer_pad_x = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_OUTER_PAD_X : EGUI_VIEW_DATA_GRID_STANDARD_OUTER_PAD_X;
    egui_dim_t outer_pad_y = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_OUTER_PAD_Y : EGUI_VIEW_DATA_GRID_STANDARD_OUTER_PAD_Y;
    egui_dim_t inner_pad_x = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_INNER_PAD_X : EGUI_VIEW_DATA_GRID_STANDARD_INNER_PAD_X;
    egui_dim_t inner_pad_y = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_INNER_PAD_Y : EGUI_VIEW_DATA_GRID_STANDARD_INNER_PAD_Y;
    egui_dim_t badge_h = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_BADGE_H : EGUI_VIEW_DATA_GRID_STANDARD_BADGE_H;
    egui_dim_t badge_gap = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_BADGE_GAP : EGUI_VIEW_DATA_GRID_STANDARD_BADGE_GAP;
    egui_dim_t title_h = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_TITLE_H : EGUI_VIEW_DATA_GRID_STANDARD_TITLE_H;
    egui_dim_t summary_h = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_SUMMARY_H : EGUI_VIEW_DATA_GRID_STANDARD_SUMMARY_H;
    egui_dim_t title_gap = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_TITLE_GAP : EGUI_VIEW_DATA_GRID_STANDARD_TITLE_GAP;
    egui_dim_t table_gap = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_TABLE_GAP : EGUI_VIEW_DATA_GRID_STANDARD_TABLE_GAP;
    egui_dim_t header_h = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_HEADER_H : EGUI_VIEW_DATA_GRID_STANDARD_HEADER_H;
    egui_dim_t row_h = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_ROW_H : EGUI_VIEW_DATA_GRID_STANDARD_ROW_H;
    egui_dim_t footer_h = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_FOOTER_H : EGUI_VIEW_DATA_GRID_STANDARD_FOOTER_H;
    egui_dim_t footer_gap = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_FOOTER_GAP : EGUI_VIEW_DATA_GRID_STANDARD_FOOTER_GAP;
    egui_dim_t inner_x;
    egui_dim_t inner_y;
    egui_dim_t inner_w;
    egui_dim_t inner_h;
    egui_dim_t stack_h = 0;
    egui_dim_t table_h = 0;
    egui_dim_t available_rows_h;
    egui_dim_t start_y;
    uint8_t row_count = egui_view_data_grid_get_row_count(snapshot);
    uint8_t has_badge = snapshot != NULL && egui_view_data_grid_has_text(snapshot->header) ? 1 : 0;
    uint8_t has_summary = summary_h > 0 && snapshot != NULL && egui_view_data_grid_has_text(snapshot->summary) ? 1 : 0;
    uint8_t has_footer = footer_h > 0 && snapshot != NULL && egui_view_data_grid_has_text(snapshot->footer) ? 1 : 0;
    uint8_t i;

    egui_view_get_work_region(self, &metrics->region);
    metrics->content_region = metrics->region;
    metrics->card_region = metrics->region;
    metrics->badge_region.size.width = 0;
    metrics->badge_region.size.height = 0;
    metrics->title_region.size.width = 0;
    metrics->title_region.size.height = 0;
    metrics->summary_region.size.width = 0;
    metrics->summary_region.size.height = 0;
    metrics->table_region.size.width = 0;
    metrics->table_region.size.height = 0;
    metrics->header_region.size.width = 0;
    metrics->header_region.size.height = 0;
    metrics->footer_region.size.width = 0;
    metrics->footer_region.size.height = 0;
    for (i = 0; i < EGUI_VIEW_DATA_GRID_MAX_ROWS; ++i)
    {
        metrics->row_regions[i].size.width = 0;
        metrics->row_regions[i].size.height = 0;
    }

    if (metrics->region.size.width <= 0 || metrics->region.size.height <= 0)
    {
        return;
    }

    metrics->content_region.location.x = metrics->region.location.x + outer_pad_x;
    metrics->content_region.location.y = metrics->region.location.y + outer_pad_y;
    metrics->content_region.size.width = metrics->region.size.width - outer_pad_x * 2;
    metrics->content_region.size.height = metrics->region.size.height - outer_pad_y * 2;
    metrics->card_region = metrics->content_region;

    if (metrics->content_region.size.width <= 0 || metrics->content_region.size.height <= 0 || snapshot == NULL)
    {
        return;
    }

    inner_x = metrics->card_region.location.x + inner_pad_x;
    inner_y = metrics->card_region.location.y + inner_pad_y;
    inner_w = metrics->card_region.size.width - inner_pad_x * 2;
    inner_h = metrics->card_region.size.height - inner_pad_y * 2;
    if (inner_w <= 0 || inner_h <= 0)
    {
        return;
    }

    if (has_badge)
    {
        stack_h += badge_h + badge_gap;
    }
    stack_h += title_h;
    if (has_summary)
    {
        stack_h += title_gap + summary_h;
    }
    if (row_count > 0)
    {
        table_h = header_h + row_count * row_h;
        stack_h += table_gap + table_h;
    }
    if (has_footer)
    {
        stack_h += footer_gap + footer_h;
    }

    if (stack_h > inner_h && row_count > 0)
    {
        available_rows_h = inner_h - (has_badge ? (badge_h + badge_gap) : 0) - title_h - (has_summary ? (title_gap + summary_h) : 0) -
                           table_gap - (has_footer ? (footer_gap + footer_h) : 0) - header_h;
        if (available_rows_h < row_count * (local->compact_mode ? 8 : 10))
        {
            available_rows_h = row_count * (local->compact_mode ? 8 : 10);
        }
        row_h = row_count > 0 ? available_rows_h / row_count : row_h;
        if (row_h < (local->compact_mode ? 8 : 10))
        {
            row_h = local->compact_mode ? 8 : 10;
        }
        table_h = header_h + row_count * row_h;
        stack_h = (has_badge ? (badge_h + badge_gap) : 0) + title_h + (has_summary ? (title_gap + summary_h) : 0) + table_gap + table_h +
                  (has_footer ? (footer_gap + footer_h) : 0);
    }

    start_y = inner_y;
    if (inner_h > stack_h)
    {
        start_y += (inner_h - stack_h) / 2;
    }

    if (has_badge)
    {
        metrics->badge_region.location.x = inner_x;
        metrics->badge_region.location.y = start_y;
        metrics->badge_region.size.width =
                egui_view_data_grid_pill_width(snapshot->header, local->compact_mode, local->compact_mode ? 24 : 30, inner_w / 2);
        metrics->badge_region.size.height = badge_h;
        start_y += badge_h + badge_gap;
    }

    metrics->title_region.location.x = inner_x;
    metrics->title_region.location.y = start_y;
    metrics->title_region.size.width = inner_w;
    metrics->title_region.size.height = title_h;
    start_y += title_h;

    if (has_summary)
    {
        start_y += title_gap;
        metrics->summary_region.location.x = inner_x + 2;
        metrics->summary_region.location.y = start_y;
        metrics->summary_region.size.width = inner_w - 4;
        metrics->summary_region.size.height = summary_h;
        start_y += summary_h;
    }

    if (row_count > 0)
    {
        start_y += table_gap;
        metrics->table_region.location.x = inner_x;
        metrics->table_region.location.y = start_y;
        metrics->table_region.size.width = inner_w;
        metrics->table_region.size.height = table_h;
        metrics->header_region.location.x = inner_x;
        metrics->header_region.location.y = start_y;
        metrics->header_region.size.width = inner_w;
        metrics->header_region.size.height = header_h;
        start_y += header_h;

        for (i = 0; i < row_count; ++i)
        {
            metrics->row_regions[i].location.x = inner_x;
            metrics->row_regions[i].location.y = start_y + i * row_h;
            metrics->row_regions[i].size.width = inner_w;
            metrics->row_regions[i].size.height = row_h;
        }

        start_y += row_count * row_h;
    }

    if (has_footer)
    {
        start_y += footer_gap;
        metrics->footer_region.location.x = inner_x + 2;
        metrics->footer_region.location.y = start_y;
        metrics->footer_region.size.width = inner_w - 4;
        metrics->footer_region.size.height = footer_h;
    }
}

static uint8_t egui_view_data_grid_hit_row(egui_view_data_grid_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    egui_view_data_grid_metrics_t metrics;
    const egui_view_data_grid_snapshot_t *snapshot = egui_view_data_grid_get_snapshot(local);
    uint8_t row_count = egui_view_data_grid_get_row_count(snapshot);
    uint8_t i;

    egui_view_data_grid_get_metrics(local, self, snapshot, &metrics);
    for (i = 0; i < row_count; ++i)
    {
        if (egui_region_pt_in_rect(&metrics.row_regions[i], x, y))
        {
            return i;
        }
    }
    return EGUI_VIEW_DATA_GRID_ROW_NONE;
}

static void egui_view_data_grid_set_current_row_inner(egui_view_t *self, uint8_t row_index, uint8_t invalidate_on_clear)
{
    EGUI_LOCAL_INIT(egui_view_data_grid_t);
    const egui_view_data_grid_snapshot_t *snapshot = egui_view_data_grid_get_snapshot(local);
    uint8_t had_pressed = egui_view_data_grid_clear_pressed_state(self, local);

    if (!egui_view_data_grid_row_exists(snapshot, row_index))
    {
        if (had_pressed && invalidate_on_clear)
        {
            egui_view_invalidate(self);
        }
        return;
    }
    if (local->current_row == row_index)
    {
        if (had_pressed && invalidate_on_clear)
        {
            egui_view_invalidate(self);
        }
        return;
    }
    local->current_row = row_index;
    egui_view_invalidate(self);
}

static void egui_view_data_grid_draw_cell_text(egui_view_t *self, egui_view_data_grid_t *local, const char *text, const egui_region_t *cell_region, uint8_t align,
                                               uint8_t compact_mode, egui_color_t color)
{
    egui_region_t text_region = *cell_region;
    egui_dim_t pad_x = compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_CELL_PAD_X : EGUI_VIEW_DATA_GRID_STANDARD_CELL_PAD_X;

    if (align == EGUI_VIEW_DATA_GRID_ALIGN_RIGHT)
    {
        text_region.location.x += pad_x;
        text_region.size.width -= pad_x * 2;
    }
    else
    {
        text_region.location.x += pad_x;
        text_region.size.width -= pad_x + (compact_mode ? 3 : 4);
    }
    egui_view_data_grid_draw_text(local->font, self, text, &text_region, egui_view_data_grid_resolve_text_align(align), color);
}

static void egui_view_data_grid_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_data_grid_t);
    const egui_view_data_grid_snapshot_t *snapshot = egui_view_data_grid_get_snapshot(local);
    const egui_view_data_grid_row_t *active_row;
    egui_view_data_grid_metrics_t metrics;
    egui_region_t text_region;
    egui_region_t cell_region;
    egui_color_t tone_color;
    egui_color_t card_fill;
    egui_color_t card_border;
    egui_color_t badge_fill;
    egui_color_t badge_border;
    egui_color_t badge_color;
    egui_color_t title_color;
    egui_color_t summary_color;
    egui_color_t table_fill;
    egui_color_t table_border;
    egui_color_t header_fill;
    egui_color_t header_text;
    egui_color_t grid_line_color;
    egui_color_t footer_fill;
    egui_color_t footer_border;
    egui_color_t footer_text;
    egui_dim_t card_radius = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_RADIUS : EGUI_VIEW_DATA_GRID_STANDARD_RADIUS;
    egui_dim_t table_radius = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_TABLE_RADIUS : EGUI_VIEW_DATA_GRID_STANDARD_TABLE_RADIUS;
    uint8_t row_count;
    uint8_t column_count;
    uint8_t i;
    uint8_t c;

    if (snapshot == NULL)
    {
        return;
    }

    egui_view_data_grid_sync_current_row(local);
    row_count = egui_view_data_grid_get_row_count(snapshot);
    column_count = egui_view_data_grid_get_column_count(snapshot);
    active_row = egui_view_data_grid_get_row(snapshot, local->current_row);
    tone_color = active_row != NULL ? egui_view_data_grid_tone_color(local, active_row->tone) : local->accent_color;

    egui_view_data_grid_get_metrics(local, self, snapshot, &metrics);
    if (metrics.content_region.size.width <= 0 || metrics.content_region.size.height <= 0)
    {
        return;
    }

    card_fill = egui_rgb_mix(local->surface_color, local->section_color, local->compact_mode ? 5 : 8);
    card_border = egui_rgb_mix(local->border_color, tone_color, local->compact_mode ? 12 : 16);
    badge_fill = egui_rgb_mix(local->surface_color, tone_color, local->compact_mode ? 10 : 14);
    badge_border = egui_rgb_mix(local->border_color, tone_color, local->compact_mode ? 14 : 20);
    badge_color = tone_color;
    title_color = local->text_color;
    summary_color = egui_rgb_mix(local->muted_text_color, tone_color, 12);
    table_fill = egui_rgb_mix(local->surface_color, local->section_color, local->compact_mode ? 3 : 6);
    table_border = egui_rgb_mix(local->border_color, tone_color, local->compact_mode ? 10 : 14);
    header_fill = egui_rgb_mix(local->section_color, tone_color, local->compact_mode ? 8 : 10);
    header_text = egui_rgb_mix(local->text_color, tone_color, 8);
    grid_line_color = egui_rgb_mix(local->border_color, tone_color, local->compact_mode ? 8 : 12);
    footer_fill = egui_rgb_mix(local->surface_color, tone_color, local->compact_mode ? 0 : 12);
    footer_border = egui_rgb_mix(local->border_color, tone_color, local->compact_mode ? 0 : 18);
    footer_text = local->compact_mode ? local->muted_text_color : egui_rgb_mix(local->muted_text_color, tone_color, 16);

    if (local->read_only_mode)
    {
        card_fill = egui_rgb_mix(card_fill, local->surface_color, 22);
        card_border = egui_rgb_mix(card_border, local->muted_text_color, 20);
        badge_fill = egui_rgb_mix(badge_fill, local->surface_color, 24);
        badge_border = egui_rgb_mix(badge_border, local->muted_text_color, 20);
        badge_color = egui_rgb_mix(badge_color, local->muted_text_color, 24);
        title_color = egui_rgb_mix(title_color, local->muted_text_color, 12);
        summary_color = egui_rgb_mix(summary_color, local->muted_text_color, 18);
        table_fill = egui_rgb_mix(table_fill, local->surface_color, 20);
        table_border = egui_rgb_mix(table_border, local->muted_text_color, 18);
        header_fill = egui_rgb_mix(header_fill, local->surface_color, 18);
        header_text = egui_rgb_mix(header_text, local->muted_text_color, 20);
        grid_line_color = egui_rgb_mix(grid_line_color, local->muted_text_color, 18);
        footer_fill = egui_rgb_mix(footer_fill, local->surface_color, 20);
        footer_border = egui_rgb_mix(footer_border, local->muted_text_color, 18);
        footer_text = egui_rgb_mix(footer_text, local->muted_text_color, 22);
    }
    if (!egui_view_get_enable(self))
    {
        card_fill = egui_view_data_grid_mix_disabled(card_fill);
        card_border = egui_view_data_grid_mix_disabled(card_border);
        badge_fill = egui_view_data_grid_mix_disabled(badge_fill);
        badge_border = egui_view_data_grid_mix_disabled(badge_border);
        badge_color = egui_view_data_grid_mix_disabled(badge_color);
        title_color = egui_view_data_grid_mix_disabled(title_color);
        summary_color = egui_view_data_grid_mix_disabled(summary_color);
        table_fill = egui_view_data_grid_mix_disabled(table_fill);
        table_border = egui_view_data_grid_mix_disabled(table_border);
        header_fill = egui_view_data_grid_mix_disabled(header_fill);
        header_text = egui_view_data_grid_mix_disabled(header_text);
        grid_line_color = egui_view_data_grid_mix_disabled(grid_line_color);
        footer_fill = egui_view_data_grid_mix_disabled(footer_fill);
        footer_border = egui_view_data_grid_mix_disabled(footer_border);
        footer_text = egui_view_data_grid_mix_disabled(footer_text);
    }

    egui_canvas_draw_round_rectangle_fill(metrics.card_region.location.x, metrics.card_region.location.y, metrics.card_region.size.width,
                                          metrics.card_region.size.height, card_radius, card_fill, egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_round_rectangle(metrics.card_region.location.x, metrics.card_region.location.y, metrics.card_region.size.width,
                                     metrics.card_region.size.height, card_radius, 1, card_border, egui_color_alpha_mix(self->alpha, 54));

    if (metrics.badge_region.size.width > 0)
    {
        egui_canvas_draw_round_rectangle_fill(metrics.badge_region.location.x, metrics.badge_region.location.y, metrics.badge_region.size.width,
                                              metrics.badge_region.size.height, metrics.badge_region.size.height / 2, badge_fill,
                                              egui_color_alpha_mix(self->alpha, 92));
        egui_canvas_draw_round_rectangle(metrics.badge_region.location.x, metrics.badge_region.location.y, metrics.badge_region.size.width,
                                         metrics.badge_region.size.height, metrics.badge_region.size.height / 2, 1, badge_border,
                                         egui_color_alpha_mix(self->alpha, 34));
        egui_view_data_grid_draw_text(local->meta_font, self, snapshot->header, &metrics.badge_region, EGUI_ALIGN_CENTER, badge_color);
    }

    egui_view_data_grid_draw_text(local->font, self, snapshot->title, &metrics.title_region, EGUI_ALIGN_CENTER, title_color);
    if (metrics.summary_region.size.width > 0)
    {
        egui_view_data_grid_draw_text(local->meta_font, self, snapshot->summary, &metrics.summary_region, EGUI_ALIGN_CENTER, summary_color);
    }

    if (metrics.table_region.size.width > 0 && metrics.table_region.size.height > 0)
    {
        egui_canvas_draw_round_rectangle_fill(metrics.table_region.location.x, metrics.table_region.location.y, metrics.table_region.size.width,
                                              metrics.table_region.size.height, table_radius, table_fill, egui_color_alpha_mix(self->alpha, 92));
        egui_canvas_draw_round_rectangle(metrics.table_region.location.x, metrics.table_region.location.y, metrics.table_region.size.width,
                                         metrics.table_region.size.height, table_radius, 1, table_border, egui_color_alpha_mix(self->alpha, 34));

        egui_canvas_draw_round_rectangle_fill(metrics.header_region.location.x, metrics.header_region.location.y, metrics.header_region.size.width,
                                              metrics.header_region.size.height, table_radius, header_fill, egui_color_alpha_mix(self->alpha, 96));

        for (c = 0; c < column_count; ++c)
        {
            egui_view_data_grid_get_column_region(&metrics.header_region, column_count, c, &cell_region);
            if (cell_region.size.width <= 0)
            {
                continue;
            }

            text_region = cell_region;
            text_region.location.x += local->compact_mode ? 4 : 6;
            text_region.size.width -= local->compact_mode ? 8 : 12;
            egui_view_data_grid_draw_text(local->meta_font, self, snapshot->columns[c].title, &text_region,
                                          egui_view_data_grid_resolve_text_align(snapshot->columns[c].align), header_text);

            if (c + 1 < column_count)
            {
                egui_canvas_draw_line(cell_region.location.x + cell_region.size.width, metrics.header_region.location.y + 2,
                                      cell_region.location.x + cell_region.size.width,
                                      metrics.table_region.location.y + metrics.table_region.size.height - 3, 1, grid_line_color,
                                      egui_color_alpha_mix(self->alpha, 22));
            }
        }

        egui_canvas_draw_line(metrics.header_region.location.x, metrics.header_region.location.y + metrics.header_region.size.height,
                              metrics.header_region.location.x + metrics.header_region.size.width - 1,
                              metrics.header_region.location.y + metrics.header_region.size.height, 1, grid_line_color,
                              egui_color_alpha_mix(self->alpha, 26));

        for (i = 0; i < row_count; ++i)
        {
            const egui_view_data_grid_row_t *row = &snapshot->rows[i];
            egui_color_t row_tone = egui_view_data_grid_tone_color(local, row->tone);
            egui_color_t row_fill = egui_rgb_mix(local->surface_color, row_tone, i == local->current_row ? 14 : (i % 2 == 0 ? 2 : 4));
            egui_color_t row_text = i == local->current_row ? egui_rgb_mix(local->text_color, row_tone, 10) : local->text_color;
            egui_color_t row_bar = egui_rgb_mix(row_tone, local->text_color, 2);

            if (row->emphasized)
            {
                row_fill = egui_rgb_mix(row_fill, row_tone, 6);
                row_text = egui_rgb_mix(row_text, row_tone, 6);
            }
            if (i == local->pressed_row && self->is_pressed)
            {
                row_fill = egui_rgb_mix(row_fill, row_tone, 10);
            }
            if (local->read_only_mode)
            {
                row_fill = egui_rgb_mix(row_fill, local->surface_color, 24);
                row_text = egui_rgb_mix(row_text, local->muted_text_color, 18);
                row_bar = egui_rgb_mix(row_bar, local->muted_text_color, 24);
            }
            if (!egui_view_get_enable(self))
            {
                row_fill = egui_view_data_grid_mix_disabled(row_fill);
                row_text = egui_view_data_grid_mix_disabled(row_text);
                row_bar = egui_view_data_grid_mix_disabled(row_bar);
            }

            egui_canvas_draw_rectangle_fill(metrics.row_regions[i].location.x, metrics.row_regions[i].location.y, metrics.row_regions[i].size.width,
                                            metrics.row_regions[i].size.height, row_fill, egui_color_alpha_mix(self->alpha, 94));
            if (i == local->current_row)
            {
                egui_canvas_draw_round_rectangle_fill(metrics.row_regions[i].location.x + 1, metrics.row_regions[i].location.y + 2, 3,
                                                      metrics.row_regions[i].size.height - 4, 1, row_bar, egui_color_alpha_mix(self->alpha, 100));
            }
            if (i + 1 < row_count)
            {
                egui_canvas_draw_line(metrics.row_regions[i].location.x + 1, metrics.row_regions[i].location.y + metrics.row_regions[i].size.height,
                                      metrics.row_regions[i].location.x + metrics.row_regions[i].size.width - 2,
                                      metrics.row_regions[i].location.y + metrics.row_regions[i].size.height, 1, grid_line_color,
                                      egui_color_alpha_mix(self->alpha, 20));
            }

            for (c = 0; c < column_count; ++c)
            {
                egui_view_data_grid_get_column_region(&metrics.row_regions[i], column_count, c, &cell_region);
                egui_view_data_grid_draw_cell_text(self, local, row->cells[c], &cell_region, snapshot->columns[c].align, local->compact_mode, row_text);
            }
        }
    }

    if (metrics.footer_region.size.width > 0)
    {
        egui_canvas_draw_round_rectangle_fill(metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                              metrics.footer_region.size.height, metrics.footer_region.size.height / 2, footer_fill,
                                              egui_color_alpha_mix(self->alpha, 92));
        egui_canvas_draw_round_rectangle(metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                         metrics.footer_region.size.height, metrics.footer_region.size.height / 2, 1, footer_border,
                                         egui_color_alpha_mix(self->alpha, 32));
        egui_view_data_grid_draw_text(local->meta_font, self, snapshot->footer, &metrics.footer_region, EGUI_ALIGN_CENTER, footer_text);
    }
}

void egui_view_data_grid_set_snapshots(egui_view_t *self, const egui_view_data_grid_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_data_grid_t);
    const egui_view_data_grid_snapshot_t *snapshot;

    egui_view_data_grid_clear_pressed_state(self, local);
    local->snapshots = snapshots;
    local->snapshot_count = snapshots == NULL ? 0 : egui_view_data_grid_clamp_snapshot_count(snapshot_count);
    if (local->current_snapshot >= local->snapshot_count)
    {
        local->current_snapshot = 0;
    }

    snapshot = egui_view_data_grid_get_snapshot(local);
    local->current_row = egui_view_data_grid_resolve_default_row(snapshot);
    egui_view_invalidate(self);
}

void egui_view_data_grid_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    EGUI_LOCAL_INIT(egui_view_data_grid_t);
    const egui_view_data_grid_snapshot_t *snapshot;
    uint8_t had_pressed = egui_view_data_grid_clear_pressed_state(self, local);

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
    snapshot = egui_view_data_grid_get_snapshot(local);
    local->current_row = egui_view_data_grid_resolve_default_row(snapshot);
    egui_view_invalidate(self);
}

uint8_t egui_view_data_grid_get_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_data_grid_t);
    return local->current_snapshot;
}

void egui_view_data_grid_set_current_row(egui_view_t *self, uint8_t row_index)
{
    egui_view_data_grid_set_current_row_inner(self, row_index, 1);
}

uint8_t egui_view_data_grid_get_current_row(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_data_grid_t);
    egui_view_data_grid_sync_current_row(local);
    return local->current_row;
}

uint8_t egui_view_data_grid_activate_current_row(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_data_grid_t);
    const egui_view_data_grid_snapshot_t *snapshot = egui_view_data_grid_get_snapshot(local);

    egui_view_data_grid_sync_current_row(local);
    if (!egui_view_data_grid_row_is_interactive(local, self, snapshot, local->current_row))
    {
        return 0;
    }

    if (local->on_action != NULL)
    {
        local->on_action(self, local->current_snapshot, local->current_row);
    }
    egui_view_invalidate(self);
    return 1;
}

void egui_view_data_grid_set_on_action_listener(egui_view_t *self, egui_view_on_data_grid_action_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_data_grid_t);
    local->on_action = listener;
}

void egui_view_data_grid_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_data_grid_t);

    local->font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_data_grid_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_data_grid_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_data_grid_t);

    local->meta_font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_data_grid_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_data_grid_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_data_grid_t);

    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_data_grid_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_data_grid_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_data_grid_t);

    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_data_grid_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_data_grid_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                                     egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                     egui_color_t success_color, egui_color_t warning_color, egui_color_t neutral_color)
{
    EGUI_LOCAL_INIT(egui_view_data_grid_t);

    local->surface_color = surface_color;
    local->section_color = section_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->neutral_color = neutral_color;
    egui_view_data_grid_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

uint8_t egui_view_data_grid_get_row_region(egui_view_t *self, uint8_t row_index, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_data_grid_t);
    const egui_view_data_grid_snapshot_t *snapshot = egui_view_data_grid_get_snapshot(local);
    egui_view_data_grid_metrics_t metrics;

    if (region == NULL || !egui_view_data_grid_row_exists(snapshot, row_index))
    {
        return 0;
    }

    egui_view_data_grid_get_metrics(local, self, snapshot, &metrics);
    *region = metrics.row_regions[row_index];
    return metrics.row_regions[row_index].size.width > 0 && metrics.row_regions[row_index].size.height > 0 ? 1 : 0;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_data_grid_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_data_grid_t);
    const egui_view_data_grid_snapshot_t *snapshot = egui_view_data_grid_get_snapshot(local);
    uint8_t hit_row;

    if (snapshot == NULL || egui_view_data_grid_get_row_count(snapshot) == 0 || local->read_only_mode || !egui_view_get_enable(self))
    {
        if (egui_view_data_grid_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_row = egui_view_data_grid_hit_row(local, self, event->location.x, event->location.y);
        if (!egui_view_data_grid_row_is_interactive(local, self, snapshot, hit_row))
        {
            if (egui_view_data_grid_clear_pressed_state(self, local))
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
        local->pressed_row = hit_row;
        egui_view_set_pressed(self, true);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_row == EGUI_VIEW_DATA_GRID_ROW_NONE)
        {
            return 0;
        }
        hit_row = egui_view_data_grid_hit_row(local, self, event->location.x, event->location.y);
        egui_view_set_pressed(self, hit_row == local->pressed_row);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
    {
        uint8_t handled;

        hit_row = egui_view_data_grid_hit_row(local, self, event->location.x, event->location.y);
        handled = local->pressed_row != EGUI_VIEW_DATA_GRID_ROW_NONE ? 1 : 0;
        if (hit_row == local->pressed_row && egui_view_data_grid_row_is_interactive(local, self, snapshot, hit_row))
        {
            local->current_row = hit_row;
            egui_view_data_grid_activate_current_row(self);
        }
        if (egui_view_data_grid_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return handled || hit_row != EGUI_VIEW_DATA_GRID_ROW_NONE;
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (egui_view_data_grid_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}

static int egui_view_data_grid_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_data_grid_t);
    EGUI_UNUSED(event);

    if (egui_view_data_grid_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_data_grid_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_data_grid_t);
    const egui_view_data_grid_snapshot_t *snapshot = egui_view_data_grid_get_snapshot(local);
    uint8_t row_count;
    uint8_t next_row;

    if (snapshot == NULL || local->read_only_mode || !egui_view_get_enable(self))
    {
        if (egui_view_data_grid_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    row_count = egui_view_data_grid_get_row_count(snapshot);
    if (row_count == 0)
    {
        return 0;
    }

    egui_view_data_grid_sync_current_row(local);
    if (event->key_code == EGUI_KEY_CODE_ENTER || event->key_code == EGUI_KEY_CODE_SPACE)
    {
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            if (!egui_view_data_grid_row_is_interactive(local, self, snapshot, local->current_row))
            {
                return 0;
            }
            local->pressed_row = local->current_row;
            egui_view_set_pressed(self, true);
            egui_view_invalidate(self);
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            uint8_t handled = 0;

            if (local->pressed_row != EGUI_VIEW_DATA_GRID_ROW_NONE && local->pressed_row == local->current_row &&
                egui_view_data_grid_row_is_interactive(local, self, snapshot, local->pressed_row))
            {
                handled = egui_view_data_grid_activate_current_row(self);
            }
            if (egui_view_data_grid_clear_pressed_state(self, local))
            {
                egui_view_invalidate(self);
            }
            return handled;
        }
        return 0;
    }

    if (egui_view_data_grid_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    if (event->type != EGUI_KEY_EVENT_ACTION_UP)
    {
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_UP:
        next_row = local->current_row > 0 ? (local->current_row - 1) : 0;
        egui_view_data_grid_set_current_row_inner(self, next_row, 0);
        return 1;
    case EGUI_KEY_CODE_DOWN:
        next_row = local->current_row + 1 < row_count ? (local->current_row + 1) : (row_count - 1);
        egui_view_data_grid_set_current_row_inner(self, next_row, 0);
        return 1;
    case EGUI_KEY_CODE_HOME:
        egui_view_data_grid_set_current_row_inner(self, 0, 0);
        return 1;
    case EGUI_KEY_CODE_END:
        egui_view_data_grid_set_current_row_inner(self, row_count - 1, 0);
        return 1;
    case EGUI_KEY_CODE_TAB:
        next_row = local->current_row + 1;
        if (next_row >= row_count)
        {
            next_row = 0;
        }
        egui_view_data_grid_set_current_row_inner(self, next_row, 0);
        return 1;
    default:
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_data_grid_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_data_grid_t);
    EGUI_UNUSED(event);

    if (egui_view_data_grid_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_data_grid_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_data_grid_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_data_grid_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_data_grid_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_data_grid_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_data_grid_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_data_grid_on_key_event,
#endif
};

void egui_view_data_grid_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_data_grid_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_data_grid_t);
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
    local->success_color = EGUI_COLOR_HEX(0x0F7B45);
    local->warning_color = EGUI_COLOR_HEX(0x9D5D00);
    local->neutral_color = EGUI_COLOR_HEX(0x7A8796);
    local->snapshot_count = 0;
    local->current_snapshot = 0;
    local->current_row = EGUI_VIEW_DATA_GRID_ROW_NONE;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_row = EGUI_VIEW_DATA_GRID_ROW_NONE;

    egui_view_set_view_name(self, "egui_view_data_grid");
}
