#include "egui_view_data_grid.h"
#include "../../hcw_selection_marker.h"

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

static egui_dim_t egui_view_data_grid_measure_font_line_height(const egui_font_t *font)
{
    egui_dim_t width = 0;
    egui_dim_t height = 0;

    if (font == NULL || font->api == NULL || font->api->get_str_size == NULL)
    {
        return 0;
    }

    font->api->get_str_size(font, "A", 0, 0, &width, &height);
    return height;
}

static egui_dim_t egui_view_data_grid_resolve_line_height(const egui_font_t *font, egui_dim_t fallback)
{
    egui_dim_t line_height = egui_view_data_grid_measure_font_line_height(font);

    return line_height > fallback ? line_height : fallback;
}

static egui_dim_t egui_view_data_grid_measure_text_width(const egui_font_t *font, const char *text)
{
    egui_dim_t width = 0;
    egui_dim_t height = 0;

    if (text == NULL || text[0] == '\0' || font == NULL || font->api == NULL || font->api->get_str_size == NULL)
    {
        return 0;
    }

    font->api->get_str_size(font, text, 0, 0, &width, &height);
    return width;
}

static void egui_view_data_grid_copy_elided(char *buffer, uint8_t buffer_size, const char *text, uint8_t max_chars)
{
    uint8_t copy_length;
    uint8_t index;
    uint8_t length;

    if (buffer == NULL || buffer_size == 0)
    {
        return;
    }

    buffer[0] = '\0';
    if (text == NULL || max_chars == 0)
    {
        return;
    }

    length = egui_view_data_grid_text_len(text);
    if (length <= max_chars)
    {
        copy_length = length;
        if (copy_length >= buffer_size)
        {
            copy_length = buffer_size - 1;
        }
        for (index = 0; index < copy_length; ++index)
        {
            buffer[index] = text[index];
        }
        buffer[copy_length] = '\0';
        return;
    }

    if (max_chars <= 3)
    {
        copy_length = max_chars;
        if (copy_length >= buffer_size)
        {
            copy_length = buffer_size - 1;
        }
        for (index = 0; index < copy_length; ++index)
        {
            buffer[index] = '.';
        }
        buffer[copy_length] = '\0';
        return;
    }

    copy_length = max_chars - 3;
    if (copy_length > buffer_size - 4)
    {
        copy_length = buffer_size - 4;
    }
    for (index = 0; index < copy_length; ++index)
    {
        buffer[index] = text[index];
    }
    buffer[copy_length] = '.';
    buffer[copy_length + 1] = '.';
    buffer[copy_length + 2] = '.';
    buffer[copy_length + 3] = '\0';
}

static void egui_view_data_grid_fit_text_to_width(const egui_font_t *font, const char *text, char *buffer, uint8_t buffer_size, egui_dim_t max_width,
                                                  egui_dim_t fallback_char_width)
{
    uint8_t max_chars;

    if (buffer == NULL || buffer_size == 0)
    {
        return;
    }

    buffer[0] = '\0';
    if (text == NULL || text[0] == '\0' || max_width <= 0)
    {
        return;
    }

    max_chars = egui_view_data_grid_text_len(text);
    egui_view_data_grid_copy_elided(buffer, buffer_size, text, max_chars);
    while (max_chars > 0)
    {
        egui_dim_t text_width = egui_view_data_grid_measure_text_width(font, buffer);

        if (text_width <= 0)
        {
            text_width = (egui_dim_t)egui_view_data_grid_text_len(buffer) * fallback_char_width;
        }
        if (text_width <= max_width)
        {
            break;
        }

        max_chars--;
        egui_view_data_grid_copy_elided(buffer, buffer_size, text, max_chars);
    }
}

static egui_color_t egui_view_data_grid_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(44));
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

static egui_dim_t egui_view_data_grid_pill_width(const egui_font_t *font, const char *text, uint8_t compact_mode, egui_dim_t min_width,
                                                 egui_dim_t max_width)
{
    egui_dim_t width = min_width;

    if (egui_view_data_grid_has_text(text))
    {
        width += egui_view_data_grid_measure_text_width(font, text);
        if (width <= min_width)
        {
            width = min_width + egui_view_data_grid_text_len(text) * (compact_mode ? 4 : 5);
        }
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
    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, align, color, self->alpha);
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
    egui_dim_t badge_slot_h = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_BADGE_H : EGUI_VIEW_DATA_GRID_STANDARD_BADGE_H;
    egui_dim_t badge_h = egui_view_data_grid_resolve_line_height(local->meta_font, badge_slot_h);
    egui_dim_t badge_gap = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_BADGE_GAP : EGUI_VIEW_DATA_GRID_STANDARD_BADGE_GAP;
    egui_dim_t title_slot_h = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_TITLE_H : EGUI_VIEW_DATA_GRID_STANDARD_TITLE_H;
    egui_dim_t title_h = egui_view_data_grid_resolve_line_height(local->font, title_slot_h);
    egui_dim_t summary_slot_h = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_SUMMARY_H : EGUI_VIEW_DATA_GRID_STANDARD_SUMMARY_H;
    egui_dim_t summary_h = summary_slot_h > 0 ? egui_view_data_grid_resolve_line_height(local->meta_font, summary_slot_h) : 0;
    egui_dim_t title_gap = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_TITLE_GAP : EGUI_VIEW_DATA_GRID_STANDARD_TITLE_GAP;
    egui_dim_t table_gap = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_TABLE_GAP : EGUI_VIEW_DATA_GRID_STANDARD_TABLE_GAP;
    egui_dim_t header_h = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_HEADER_H : EGUI_VIEW_DATA_GRID_STANDARD_HEADER_H;
    egui_dim_t row_h = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_ROW_H : EGUI_VIEW_DATA_GRID_STANDARD_ROW_H;
    egui_dim_t footer_slot_h = local->compact_mode ? EGUI_VIEW_DATA_GRID_COMPACT_FOOTER_H : EGUI_VIEW_DATA_GRID_STANDARD_FOOTER_H;
    egui_dim_t footer_h = footer_slot_h > 0 ? egui_view_data_grid_resolve_line_height(local->meta_font, footer_slot_h) : 0;
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
    uint8_t has_summary = summary_slot_h > 0 && snapshot != NULL && egui_view_data_grid_has_text(snapshot->summary) ? 1 : 0;
    uint8_t has_footer = footer_slot_h > 0 && snapshot != NULL && egui_view_data_grid_has_text(snapshot->footer) ? 1 : 0;
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
        stack_h += badge_slot_h + badge_gap;
    }
    stack_h += title_slot_h;
    if (has_summary)
    {
        stack_h += title_gap + summary_slot_h;
    }
    if (row_count > 0)
    {
        table_h = header_h + row_count * row_h;
        stack_h += table_gap + table_h;
    }
    if (has_footer)
    {
        stack_h += footer_gap + footer_slot_h;
    }

    if (stack_h > inner_h && row_count > 0)
    {
        available_rows_h = inner_h - (has_badge ? (badge_slot_h + badge_gap) : 0) - title_slot_h - (has_summary ? (title_gap + summary_slot_h) : 0) -
                           table_gap - (has_footer ? (footer_gap + footer_slot_h) : 0) - header_h;
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
        stack_h = (has_badge ? (badge_slot_h + badge_gap) : 0) + title_slot_h + (has_summary ? (title_gap + summary_slot_h) : 0) + table_gap + table_h +
                  (has_footer ? (footer_gap + footer_slot_h) : 0);
    }

    start_y = inner_y;
    if (inner_h > stack_h)
    {
        start_y += (inner_h - stack_h) / 2;
    }

    if (has_badge)
    {
        metrics->badge_region.location.x = inner_x;
        metrics->badge_region.location.y = start_y + (badge_slot_h - badge_h) / 2;
        metrics->badge_region.size.width =
                egui_view_data_grid_pill_width(local->meta_font, snapshot->header, local->compact_mode, local->compact_mode ? 24 : 30, inner_w / 2);
        metrics->badge_region.size.height = badge_h;
        start_y += badge_slot_h + badge_gap;
    }

    metrics->title_region.location.x = inner_x;
    metrics->title_region.location.y = start_y + (title_slot_h - title_h) / 2;
    metrics->title_region.size.width = inner_w;
    metrics->title_region.size.height = title_h;
    start_y += title_slot_h;

    if (has_summary)
    {
        start_y += title_gap;
        metrics->summary_region.location.x = inner_x + 2;
        metrics->summary_region.location.y = start_y + (summary_slot_h - summary_h) / 2;
        metrics->summary_region.size.width = inner_w - 4;
        metrics->summary_region.size.height = summary_h;
        start_y += summary_slot_h;
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
        metrics->footer_region.location.y = start_y + (footer_slot_h - footer_h) / 2;
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
    char cell_label[24];
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
    egui_view_data_grid_fit_text_to_width(local->font, text, cell_label, sizeof(cell_label), text_region.size.width, compact_mode ? 4 : 5);
    egui_view_data_grid_draw_text(local->font, self, cell_label, &text_region, egui_view_data_grid_resolve_text_align(align), color);
}

static void egui_view_data_grid_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_data_grid_t);
    const egui_view_data_grid_snapshot_t *snapshot = egui_view_data_grid_get_snapshot(local);
    const egui_view_data_grid_row_t *active_row;
    egui_view_data_grid_metrics_t metrics;
    char badge_label[16];
    char title_label[32];
    char summary_label[48];
    char header_label[16];
    char footer_label[48];
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

    card_fill = egui_rgb_mix(local->surface_color, local->section_color, EGUI_ALPHA_MAKE(local->compact_mode ? 22 : 28));
    card_border = egui_rgb_mix(local->border_color, tone_color, EGUI_ALPHA_MAKE(local->compact_mode ? 42 : 50));
    badge_fill = egui_rgb_mix(local->surface_color, tone_color, EGUI_ALPHA_MAKE(local->compact_mode ? 30 : 36));
    badge_border = egui_rgb_mix(local->border_color, tone_color, EGUI_ALPHA_MAKE(local->compact_mode ? 44 : 52));
    badge_color = tone_color;
    title_color = local->text_color;
    summary_color = egui_rgb_mix(local->muted_text_color, tone_color, EGUI_ALPHA_MAKE(32));
    table_fill = egui_rgb_mix(local->surface_color, local->section_color, EGUI_ALPHA_MAKE(local->compact_mode ? 18 : 24));
    table_border = egui_rgb_mix(local->border_color, tone_color, EGUI_ALPHA_MAKE(local->compact_mode ? 40 : 48));
    header_fill = egui_rgb_mix(local->section_color, tone_color, EGUI_ALPHA_MAKE(local->compact_mode ? 30 : 36));
    header_text = egui_rgb_mix(local->text_color, tone_color, EGUI_ALPHA_MAKE(30));
    grid_line_color = egui_rgb_mix(local->border_color, tone_color, EGUI_ALPHA_MAKE(local->compact_mode ? 36 : 44));
    footer_fill = egui_rgb_mix(local->surface_color, tone_color, EGUI_ALPHA_MAKE(local->compact_mode ? 0 : 34));
    footer_border = egui_rgb_mix(local->border_color, tone_color, EGUI_ALPHA_MAKE(local->compact_mode ? 0 : 48));
    footer_text = local->compact_mode ? local->muted_text_color : egui_rgb_mix(local->muted_text_color, tone_color, EGUI_ALPHA_MAKE(34));

    if (local->read_only_mode)
    {
        card_fill = egui_rgb_mix(card_fill, local->surface_color, EGUI_ALPHA_MAKE(22));
        card_border = egui_rgb_mix(card_border, local->muted_text_color, EGUI_ALPHA_MAKE(20));
        badge_fill = egui_rgb_mix(badge_fill, local->surface_color, EGUI_ALPHA_MAKE(24));
        badge_border = egui_rgb_mix(badge_border, local->muted_text_color, EGUI_ALPHA_MAKE(20));
        badge_color = egui_rgb_mix(badge_color, local->muted_text_color, EGUI_ALPHA_MAKE(24));
        title_color = egui_rgb_mix(title_color, local->muted_text_color, EGUI_ALPHA_MAKE(12));
        summary_color = egui_rgb_mix(summary_color, local->muted_text_color, EGUI_ALPHA_MAKE(18));
        table_fill = egui_rgb_mix(table_fill, local->surface_color, EGUI_ALPHA_MAKE(20));
        table_border = egui_rgb_mix(table_border, local->muted_text_color, EGUI_ALPHA_MAKE(18));
        header_fill = egui_rgb_mix(header_fill, local->surface_color, EGUI_ALPHA_MAKE(18));
        header_text = egui_rgb_mix(header_text, local->muted_text_color, EGUI_ALPHA_MAKE(20));
        grid_line_color = egui_rgb_mix(grid_line_color, local->muted_text_color, EGUI_ALPHA_MAKE(18));
        footer_fill = egui_rgb_mix(footer_fill, local->surface_color, EGUI_ALPHA_MAKE(20));
        footer_border = egui_rgb_mix(footer_border, local->muted_text_color, EGUI_ALPHA_MAKE(18));
        footer_text = egui_rgb_mix(footer_text, local->muted_text_color, EGUI_ALPHA_MAKE(22));
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

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.card_region.location.x, metrics.card_region.location.y, metrics.card_region.size.width,
                                          metrics.card_region.size.height, card_radius, card_fill, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(96)));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics.card_region.location.x, metrics.card_region.location.y, metrics.card_region.size.width,
                                     metrics.card_region.size.height, card_radius, 1, card_border,
                                     egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->read_only_mode ? 78 : 94)));

    if (metrics.badge_region.size.width > 0)
    {
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.badge_region.location.x, metrics.badge_region.location.y, metrics.badge_region.size.width,
                                              metrics.badge_region.size.height, metrics.badge_region.size.height / 2, badge_fill,
                                              egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->read_only_mode ? 92 : 98)));
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics.badge_region.location.x, metrics.badge_region.location.y, metrics.badge_region.size.width,
                                         metrics.badge_region.size.height, metrics.badge_region.size.height / 2, 1, badge_border,
                                         egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->read_only_mode ? 78 : 94)));
        egui_view_data_grid_fit_text_to_width(local->meta_font, snapshot->header, badge_label, sizeof(badge_label), metrics.badge_region.size.width - 4,
                                              local->compact_mode ? 4 : 5);
        egui_view_data_grid_draw_text(local->meta_font, self, badge_label, &metrics.badge_region, EGUI_ALIGN_CENTER, badge_color);
    }

    egui_view_data_grid_fit_text_to_width(local->font, snapshot->title, title_label, sizeof(title_label), metrics.title_region.size.width,
                                          local->compact_mode ? 4 : 5);
    egui_view_data_grid_draw_text(local->font, self, title_label, &metrics.title_region, EGUI_ALIGN_CENTER, title_color);
    if (metrics.summary_region.size.width > 0)
    {
        egui_view_data_grid_fit_text_to_width(local->meta_font, snapshot->summary, summary_label, sizeof(summary_label), metrics.summary_region.size.width,
                                              local->compact_mode ? 4 : 5);
        egui_view_data_grid_draw_text(local->meta_font, self, summary_label, &metrics.summary_region, EGUI_ALIGN_CENTER, summary_color);
    }

    if (metrics.table_region.size.width > 0 && metrics.table_region.size.height > 0)
    {
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.table_region.location.x, metrics.table_region.location.y, metrics.table_region.size.width,
                                              metrics.table_region.size.height, table_radius, table_fill,
                                              egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->read_only_mode ? 92 : 98)));
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics.table_region.location.x, metrics.table_region.location.y, metrics.table_region.size.width,
                                         metrics.table_region.size.height, table_radius, 1, table_border,
                                         egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->read_only_mode ? 78 : 94)));

        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.header_region.location.x, metrics.header_region.location.y, metrics.header_region.size.width,
                                              metrics.header_region.size.height, table_radius, header_fill, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(98)));

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
            egui_view_data_grid_fit_text_to_width(local->meta_font, snapshot->columns[c].title, header_label, sizeof(header_label), text_region.size.width,
                                                  local->compact_mode ? 4 : 5);
            egui_view_data_grid_draw_text(local->meta_font, self, header_label, &text_region,
                                          egui_view_data_grid_resolve_text_align(snapshot->columns[c].align), header_text);

            if (c + 1 < column_count)
            {
                egui_canvas_draw_line(&uicode_get_core()->canvas, cell_region.location.x + cell_region.size.width, metrics.header_region.location.y + 2,
                                      cell_region.location.x + cell_region.size.width,
                                      metrics.table_region.location.y + metrics.table_region.size.height - 3, 1, grid_line_color,
                                      egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->read_only_mode ? 72 : 92)));
            }
        }

        egui_canvas_draw_line(&uicode_get_core()->canvas, metrics.header_region.location.x, metrics.header_region.location.y + metrics.header_region.size.height,
                              metrics.header_region.location.x + metrics.header_region.size.width - 1,
                              metrics.header_region.location.y + metrics.header_region.size.height, 1, grid_line_color,
                              egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->read_only_mode ? 74 : 94)));

        for (i = 0; i < row_count; ++i)
        {
            const egui_view_data_grid_row_t *row = &snapshot->rows[i];
            egui_color_t row_tone = egui_view_data_grid_tone_color(local, row->tone);
            egui_color_t row_fill = egui_rgb_mix(local->surface_color, row_tone, EGUI_ALPHA_MAKE(i == local->current_row ? 48 : (i % 2 == 0 ? 24 : 28)));
            egui_color_t row_text = i == local->current_row ? egui_rgb_mix(local->text_color, row_tone, EGUI_ALPHA_MAKE(32)) : local->text_color;
            egui_color_t row_bar = egui_rgb_mix(row_tone, local->text_color, EGUI_ALPHA_MAKE(24));

            if (row->emphasized)
            {
                row_fill = egui_rgb_mix(row_fill, row_tone, EGUI_ALPHA_MAKE(24));
                row_text = egui_rgb_mix(row_text, row_tone, EGUI_ALPHA_MAKE(22));
            }
            if (i == local->pressed_row && self->is_pressed)
            {
                row_fill = egui_rgb_mix(row_fill, row_tone, EGUI_ALPHA_MAKE(30));
            }
            if (local->read_only_mode)
            {
                row_fill = egui_rgb_mix(row_fill, local->surface_color, EGUI_ALPHA_MAKE(24));
                row_text = egui_rgb_mix(row_text, local->muted_text_color, EGUI_ALPHA_MAKE(18));
                row_bar = egui_rgb_mix(row_bar, local->muted_text_color, EGUI_ALPHA_MAKE(24));
            }
            if (!egui_view_get_enable(self))
            {
                row_fill = egui_view_data_grid_mix_disabled(row_fill);
                row_text = egui_view_data_grid_mix_disabled(row_text);
                row_bar = egui_view_data_grid_mix_disabled(row_bar);
            }

            egui_canvas_draw_rectangle_fill(&uicode_get_core()->canvas, metrics.row_regions[i].location.x, metrics.row_regions[i].location.y, metrics.row_regions[i].size.width,
                                            metrics.row_regions[i].size.height, row_fill,
                                            egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->read_only_mode ? 94 : 98)));
            if (i == local->current_row)
            {
                hcw_selection_marker_draw_left(&metrics.row_regions[i], local->compact_mode ? 3 : 4, local->compact_mode ? 3 : 4, row_bar,
                                               egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(100)));
            }
            if (i + 1 < row_count)
            {
                egui_canvas_draw_line(&uicode_get_core()->canvas, metrics.row_regions[i].location.x + 1, metrics.row_regions[i].location.y + metrics.row_regions[i].size.height,
                                      metrics.row_regions[i].location.x + metrics.row_regions[i].size.width - 2,
                                      metrics.row_regions[i].location.y + metrics.row_regions[i].size.height, 1, grid_line_color,
                                      egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->read_only_mode ? 72 : 92)));
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
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                              metrics.footer_region.size.height, metrics.footer_region.size.height / 2, footer_fill,
                                              egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->read_only_mode ? 92 : 98)));
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                         metrics.footer_region.size.height, metrics.footer_region.size.height / 2, 1, footer_border,
                                         egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->read_only_mode ? 78 : 94)));
        egui_view_data_grid_fit_text_to_width(local->meta_font, snapshot->footer, footer_label, sizeof(footer_label), metrics.footer_region.size.width,
                                              local->compact_mode ? 4 : 5);
        egui_view_data_grid_draw_text(local->meta_font, self, footer_label, &metrics.footer_region, EGUI_ALIGN_CENTER, footer_text);
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

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_data_grid_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif

    local->snapshots = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->on_action = NULL;
    local->surface_color = HCW_COLOR_SURFACE;
    local->section_color = HCW_COLOR_SURFACE_SUBTLE;
    local->border_color = HCW_COLOR_BORDER;
    local->text_color = HCW_COLOR_TEXT;
    local->muted_text_color = HCW_COLOR_TEXT_MUTED;
    local->accent_color = HCW_COLOR_PRIMARY;
    local->success_color = HCW_COLOR_SUCCESS;
    local->warning_color = HCW_COLOR_WARNING;
    local->neutral_color = HCW_COLOR_NEUTRAL;
    local->snapshot_count = 0;
    local->current_snapshot = 0;
    local->current_row = EGUI_VIEW_DATA_GRID_ROW_NONE;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_row = EGUI_VIEW_DATA_GRID_ROW_NONE;

    egui_view_set_view_name(self, "egui_view_data_grid");
}
