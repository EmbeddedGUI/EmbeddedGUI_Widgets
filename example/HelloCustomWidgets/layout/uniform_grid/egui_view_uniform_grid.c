#include "egui_view_uniform_grid.h"
#include "../../hcw_text_center.h"
#include "../../hcw_selection_marker.h"

#define EGUI_VIEW_UNIFORM_GRID_STANDARD_RADIUS       10
#define EGUI_VIEW_UNIFORM_GRID_STANDARD_OUTER_PAD_X  8
#define EGUI_VIEW_UNIFORM_GRID_STANDARD_OUTER_PAD_Y  8
#define EGUI_VIEW_UNIFORM_GRID_STANDARD_INNER_PAD_X  8
#define EGUI_VIEW_UNIFORM_GRID_STANDARD_INNER_PAD_Y  7
#define EGUI_VIEW_UNIFORM_GRID_STANDARD_BADGE_H      11
#define EGUI_VIEW_UNIFORM_GRID_STANDARD_BADGE_GAP    5
#define EGUI_VIEW_UNIFORM_GRID_STANDARD_TITLE_H      12
#define EGUI_VIEW_UNIFORM_GRID_STANDARD_SUMMARY_H    10
#define EGUI_VIEW_UNIFORM_GRID_STANDARD_TITLE_GAP    2
#define EGUI_VIEW_UNIFORM_GRID_STANDARD_GRID_GAP     7
#define EGUI_VIEW_UNIFORM_GRID_STANDARD_GRID_RADIUS  8
#define EGUI_VIEW_UNIFORM_GRID_STANDARD_GRID_PAD_X   4
#define EGUI_VIEW_UNIFORM_GRID_STANDARD_GRID_PAD_Y   4
#define EGUI_VIEW_UNIFORM_GRID_STANDARD_FOOTER_H     11
#define EGUI_VIEW_UNIFORM_GRID_STANDARD_FOOTER_GAP   5
#define EGUI_VIEW_UNIFORM_GRID_STANDARD_CELL_GAP     6
#define EGUI_VIEW_UNIFORM_GRID_STANDARD_CELL_RADIUS  7
#define EGUI_VIEW_UNIFORM_GRID_STANDARD_CELL_PAD_X   6
#define EGUI_VIEW_UNIFORM_GRID_STANDARD_CELL_PAD_Y   5

#define EGUI_VIEW_UNIFORM_GRID_COMPACT_RADIUS       8
#define EGUI_VIEW_UNIFORM_GRID_COMPACT_OUTER_PAD_X  6
#define EGUI_VIEW_UNIFORM_GRID_COMPACT_OUTER_PAD_Y  6
#define EGUI_VIEW_UNIFORM_GRID_COMPACT_INNER_PAD_X  6
#define EGUI_VIEW_UNIFORM_GRID_COMPACT_INNER_PAD_Y  5
#define EGUI_VIEW_UNIFORM_GRID_COMPACT_BADGE_H      8
#define EGUI_VIEW_UNIFORM_GRID_COMPACT_BADGE_GAP    4
#define EGUI_VIEW_UNIFORM_GRID_COMPACT_TITLE_H      10
#define EGUI_VIEW_UNIFORM_GRID_COMPACT_SUMMARY_H    0
#define EGUI_VIEW_UNIFORM_GRID_COMPACT_TITLE_GAP    0
#define EGUI_VIEW_UNIFORM_GRID_COMPACT_GRID_GAP     4
#define EGUI_VIEW_UNIFORM_GRID_COMPACT_GRID_RADIUS  6
#define EGUI_VIEW_UNIFORM_GRID_COMPACT_GRID_PAD_X   3
#define EGUI_VIEW_UNIFORM_GRID_COMPACT_GRID_PAD_Y   3
#define EGUI_VIEW_UNIFORM_GRID_COMPACT_FOOTER_H     0
#define EGUI_VIEW_UNIFORM_GRID_COMPACT_FOOTER_GAP   0
#define EGUI_VIEW_UNIFORM_GRID_COMPACT_CELL_GAP     4
#define EGUI_VIEW_UNIFORM_GRID_COMPACT_CELL_RADIUS  5
#define EGUI_VIEW_UNIFORM_GRID_COMPACT_CELL_PAD_X   5
#define EGUI_VIEW_UNIFORM_GRID_COMPACT_CELL_PAD_Y   4

#define EGUI_VIEW_UNIFORM_GRID_SELECTION_MARK_GAP 4

typedef struct egui_view_uniform_grid_metrics egui_view_uniform_grid_metrics_t;
struct egui_view_uniform_grid_metrics
{
    egui_region_t region;
    egui_region_t content_region;
    egui_region_t card_region;
    egui_region_t badge_region;
    egui_region_t title_region;
    egui_region_t summary_region;
    egui_region_t grid_region;
    egui_region_t footer_region;
    egui_region_t cell_regions[EGUI_VIEW_UNIFORM_GRID_MAX_CELLS];
};

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_uniform_grid_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event);
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_uniform_grid_on_static_key_event(egui_view_t *self, egui_key_event_t *event);
#endif

static uint8_t uniform_grid_clamp_snapshot_count(uint8_t count)
{
    return count > EGUI_VIEW_UNIFORM_GRID_MAX_SNAPSHOTS ? EGUI_VIEW_UNIFORM_GRID_MAX_SNAPSHOTS : count;
}

static uint8_t uniform_grid_clamp_cell_count(uint8_t count)
{
    return count > EGUI_VIEW_UNIFORM_GRID_MAX_CELLS ? EGUI_VIEW_UNIFORM_GRID_MAX_CELLS : count;
}

static uint8_t uniform_grid_text_len(const char *text)
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

static void uniform_grid_copy_elided(char *buffer, uint8_t buffer_size, const char *text, uint8_t max_chars)
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

    length = uniform_grid_text_len(text);
    if (length <= max_chars)
    {
        copy_length = length;
        if (copy_length >= buffer_size)
        {
            copy_length = buffer_size - 1;
        }
        for (index = 0; index < copy_length; index++)
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
        for (index = 0; index < copy_length; index++)
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
    for (index = 0; index < copy_length; index++)
    {
        buffer[index] = text[index];
    }
    buffer[copy_length] = '.';
    buffer[copy_length + 1] = '.';
    buffer[copy_length + 2] = '.';
    buffer[copy_length + 3] = '\0';
}

static uint8_t uniform_grid_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static egui_dim_t uniform_grid_measure_font_line_height(const egui_font_t *font)
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

static egui_dim_t uniform_grid_resolve_line_height(const egui_font_t *font, egui_dim_t fallback)
{
    egui_dim_t line_height = uniform_grid_measure_font_line_height(font);

    return line_height > fallback ? line_height : fallback;
}

static egui_dim_t uniform_grid_measure_text_width(const egui_font_t *font, const char *text)
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

static void uniform_grid_fit_text_to_width(const egui_font_t *font, const char *text, char *buffer, uint8_t buffer_size, egui_dim_t max_width,
                                           egui_dim_t fallback_char_width)
{
    uint8_t max_chars;

    if (buffer == NULL || buffer_size == 0)
    {
        return;
    }

    buffer[0] = '\0';
    if (!uniform_grid_has_text(text) || max_width <= 0)
    {
        return;
    }

    max_chars = uniform_grid_text_len(text);
    uniform_grid_copy_elided(buffer, buffer_size, text, max_chars);
    while (max_chars > 0)
    {
        egui_dim_t text_width = uniform_grid_measure_text_width(font, buffer);

        if (text_width <= 0)
        {
            text_width = (egui_dim_t)uniform_grid_text_len(buffer) * fallback_char_width;
        }
        if (text_width <= max_width)
        {
            break;
        }

        max_chars--;
        uniform_grid_copy_elided(buffer, buffer_size, text, max_chars);
    }
}

static egui_color_t uniform_grid_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(44));
}

static const egui_view_uniform_grid_snapshot_t *uniform_grid_get_snapshot(egui_view_uniform_grid_t *local)
{
    if (local->snapshots == NULL || local->snapshot_count == 0 || local->current_snapshot >= local->snapshot_count)
    {
        return NULL;
    }
    return &local->snapshots[local->current_snapshot];
}

static uint8_t uniform_grid_get_cell_count(const egui_view_uniform_grid_snapshot_t *snapshot)
{
    if (snapshot == NULL || snapshot->cells == NULL)
    {
        return 0;
    }
    return uniform_grid_clamp_cell_count(snapshot->cell_count);
}

static uint8_t uniform_grid_get_column_count(const egui_view_uniform_grid_snapshot_t *snapshot)
{
    uint8_t cell_count = uniform_grid_get_cell_count(snapshot);
    uint8_t column_count;

    if (cell_count == 0)
    {
        return 0;
    }

    column_count = snapshot->column_count == 0 ? cell_count : snapshot->column_count;
    if (column_count > cell_count)
    {
        column_count = cell_count;
    }
    return column_count;
}

static uint8_t uniform_grid_get_row_count(const egui_view_uniform_grid_snapshot_t *snapshot)
{
    uint8_t cell_count = uniform_grid_get_cell_count(snapshot);
    uint8_t column_count = uniform_grid_get_column_count(snapshot);

    if (cell_count == 0 || column_count == 0)
    {
        return 0;
    }
    return (uint8_t)((cell_count + column_count - 1) / column_count);
}

static const egui_view_uniform_grid_cell_t *uniform_grid_get_cell(const egui_view_uniform_grid_snapshot_t *snapshot, uint8_t cell_index)
{
    if (cell_index >= uniform_grid_get_cell_count(snapshot))
    {
        return NULL;
    }
    return &snapshot->cells[cell_index];
}

static egui_color_t uniform_grid_tone_color(egui_view_uniform_grid_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_UNIFORM_GRID_TONE_SUCCESS:
        return local->success_color;
    case EGUI_VIEW_UNIFORM_GRID_TONE_WARNING:
        return local->warning_color;
    case EGUI_VIEW_UNIFORM_GRID_TONE_NEUTRAL:
        return local->neutral_color;
    case EGUI_VIEW_UNIFORM_GRID_TONE_ACCENT:
    default:
        return local->accent_color;
    }
}

static uint8_t uniform_grid_clear_pressed_state(egui_view_t *self, egui_view_uniform_grid_t *local)
{
    uint8_t had_pressed = self->is_pressed || local->pressed_cell != EGUI_VIEW_UNIFORM_GRID_CELL_NONE;

    local->pressed_cell = EGUI_VIEW_UNIFORM_GRID_CELL_NONE;
    egui_view_set_pressed(self, false);
    return had_pressed;
}

static uint8_t uniform_grid_cell_exists(const egui_view_uniform_grid_snapshot_t *snapshot, uint8_t cell_index)
{
    return cell_index < uniform_grid_get_cell_count(snapshot) ? 1 : 0;
}

static uint8_t uniform_grid_resolve_default_cell(const egui_view_uniform_grid_snapshot_t *snapshot)
{
    uint8_t cell_count = uniform_grid_get_cell_count(snapshot);

    if (cell_count == 0)
    {
        return EGUI_VIEW_UNIFORM_GRID_CELL_NONE;
    }
    return snapshot->selected_cell < cell_count ? snapshot->selected_cell : 0;
}

static void uniform_grid_sync_current_cell(egui_view_uniform_grid_t *local)
{
    const egui_view_uniform_grid_snapshot_t *snapshot = uniform_grid_get_snapshot(local);

    if (!uniform_grid_cell_exists(snapshot, local->current_cell))
    {
        local->current_cell = uniform_grid_resolve_default_cell(snapshot);
    }
}

static uint8_t uniform_grid_cell_is_interactive(egui_view_uniform_grid_t *local, egui_view_t *self,
                                                const egui_view_uniform_grid_snapshot_t *snapshot, uint8_t cell_index)
{
    if (snapshot == NULL || local->read_only_mode || !egui_view_get_enable(self))
    {
        return 0;
    }
    return uniform_grid_cell_exists(snapshot, cell_index);
}

static egui_dim_t uniform_grid_pill_width(const egui_font_t *font, const char *text, uint8_t compact_mode, egui_dim_t min_width, egui_dim_t max_width)
{
    egui_dim_t width = min_width;

    if (uniform_grid_has_text(text))
    {
        width += uniform_grid_measure_text_width(font, text);
        if (width <= min_width)
        {
            width = min_width + uniform_grid_text_len(text) * (compact_mode ? 4 : 5);
        }
    }
    if (width > max_width)
    {
        width = max_width;
    }
    return width;
}

static void uniform_grid_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                   egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (font == NULL || !uniform_grid_has_text(text) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    draw_region.location.y += hcw_text_center_get_delta(font, text, region, align);
    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, align, color, self->alpha);
}

static void uniform_grid_get_metrics(egui_view_uniform_grid_t *local, egui_view_t *self, const egui_view_uniform_grid_snapshot_t *snapshot,
                                     egui_view_uniform_grid_metrics_t *metrics)
{
    egui_dim_t outer_pad_x = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_OUTER_PAD_X : EGUI_VIEW_UNIFORM_GRID_STANDARD_OUTER_PAD_X;
    egui_dim_t outer_pad_y = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_OUTER_PAD_Y : EGUI_VIEW_UNIFORM_GRID_STANDARD_OUTER_PAD_Y;
    egui_dim_t inner_pad_x = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_INNER_PAD_X : EGUI_VIEW_UNIFORM_GRID_STANDARD_INNER_PAD_X;
    egui_dim_t inner_pad_y = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_INNER_PAD_Y : EGUI_VIEW_UNIFORM_GRID_STANDARD_INNER_PAD_Y;
    egui_dim_t badge_slot_h = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_BADGE_H : EGUI_VIEW_UNIFORM_GRID_STANDARD_BADGE_H;
    egui_dim_t badge_h = uniform_grid_resolve_line_height(local->meta_font, badge_slot_h);
    egui_dim_t badge_gap = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_BADGE_GAP : EGUI_VIEW_UNIFORM_GRID_STANDARD_BADGE_GAP;
    egui_dim_t title_slot_h = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_TITLE_H : EGUI_VIEW_UNIFORM_GRID_STANDARD_TITLE_H;
    egui_dim_t title_h = uniform_grid_resolve_line_height(local->font, title_slot_h);
    egui_dim_t summary_slot_h = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_SUMMARY_H : EGUI_VIEW_UNIFORM_GRID_STANDARD_SUMMARY_H;
    egui_dim_t summary_h = summary_slot_h > 0 ? uniform_grid_resolve_line_height(local->meta_font, summary_slot_h) : 0;
    egui_dim_t title_gap = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_TITLE_GAP : EGUI_VIEW_UNIFORM_GRID_STANDARD_TITLE_GAP;
    egui_dim_t grid_gap = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_GRID_GAP : EGUI_VIEW_UNIFORM_GRID_STANDARD_GRID_GAP;
    egui_dim_t grid_pad_x = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_GRID_PAD_X : EGUI_VIEW_UNIFORM_GRID_STANDARD_GRID_PAD_X;
    egui_dim_t grid_pad_y = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_GRID_PAD_Y : EGUI_VIEW_UNIFORM_GRID_STANDARD_GRID_PAD_Y;
    egui_dim_t footer_slot_h = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_FOOTER_H : EGUI_VIEW_UNIFORM_GRID_STANDARD_FOOTER_H;
    egui_dim_t footer_h = footer_slot_h > 0 ? uniform_grid_resolve_line_height(local->meta_font, footer_slot_h) : 0;
    egui_dim_t footer_gap = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_FOOTER_GAP : EGUI_VIEW_UNIFORM_GRID_STANDARD_FOOTER_GAP;
    egui_dim_t cell_gap = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_CELL_GAP : EGUI_VIEW_UNIFORM_GRID_STANDARD_CELL_GAP;
    egui_dim_t inner_x;
    egui_dim_t inner_y;
    egui_dim_t inner_w;
    egui_dim_t inner_h;
    egui_dim_t start_y;
    egui_dim_t footer_y;
    egui_dim_t grid_h;
    egui_dim_t cells_x;
    egui_dim_t cells_y;
    egui_dim_t cells_w;
    egui_dim_t cells_h;
    egui_dim_t available_w;
    egui_dim_t available_h;
    egui_dim_t base_w;
    egui_dim_t base_h;
    egui_dim_t rem_w;
    egui_dim_t rem_h;
    egui_dim_t y;
    uint8_t cell_count = uniform_grid_get_cell_count(snapshot);
    uint8_t column_count = uniform_grid_get_column_count(snapshot);
    uint8_t row_count = uniform_grid_get_row_count(snapshot);
    uint8_t has_badge = snapshot != NULL && uniform_grid_has_text(snapshot->header) ? 1 : 0;
    uint8_t has_summary = summary_slot_h > 0 && snapshot != NULL && uniform_grid_has_text(snapshot->summary) ? 1 : 0;
    uint8_t has_footer = footer_slot_h > 0 && snapshot != NULL && uniform_grid_has_text(snapshot->footer) ? 1 : 0;
    uint8_t row_index;
    uint8_t col_index;
    uint8_t cell_index = 0;

    egui_view_get_work_region(self, &metrics->region);
    metrics->content_region = metrics->region;
    metrics->card_region = metrics->region;
    metrics->badge_region.size.width = 0;
    metrics->badge_region.size.height = 0;
    metrics->title_region.size.width = 0;
    metrics->title_region.size.height = 0;
    metrics->summary_region.size.width = 0;
    metrics->summary_region.size.height = 0;
    metrics->grid_region.size.width = 0;
    metrics->grid_region.size.height = 0;
    metrics->footer_region.size.width = 0;
    metrics->footer_region.size.height = 0;
    for (cell_index = 0; cell_index < EGUI_VIEW_UNIFORM_GRID_MAX_CELLS; ++cell_index)
    {
        metrics->cell_regions[cell_index].size.width = 0;
        metrics->cell_regions[cell_index].size.height = 0;
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

    start_y = inner_y;
    if (has_badge)
    {
        metrics->badge_region.location.x = inner_x;
        metrics->badge_region.location.y = start_y + (badge_slot_h - badge_h) / 2;
        metrics->badge_region.size.width =
                uniform_grid_pill_width(local->meta_font, snapshot->header, local->compact_mode, local->compact_mode ? 22 : 28, inner_w);
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
        metrics->summary_region.location.x = inner_x;
        metrics->summary_region.location.y = start_y + (summary_slot_h - summary_h) / 2;
        metrics->summary_region.size.width = inner_w;
        metrics->summary_region.size.height = summary_h;
        start_y += summary_slot_h;
    }

    if (cell_count == 0 || column_count == 0 || row_count == 0)
    {
        return;
    }

    start_y += grid_gap;
    footer_y = inner_y + inner_h;
    if (has_footer)
    {
        footer_y -= footer_slot_h;
        metrics->footer_region.location.x = inner_x;
        metrics->footer_region.location.y = footer_y + (footer_slot_h - footer_h) / 2;
        metrics->footer_region.size.width =
                uniform_grid_pill_width(local->meta_font, snapshot->footer, local->compact_mode, local->compact_mode ? 18 : 24, inner_w);
        metrics->footer_region.size.height = footer_h;
        footer_y -= footer_gap;
    }

    grid_h = footer_y - start_y;
    if (grid_h <= 0)
    {
        return;
    }

    metrics->grid_region.location.x = inner_x;
    metrics->grid_region.location.y = start_y;
    metrics->grid_region.size.width = inner_w;
    metrics->grid_region.size.height = grid_h;

    cells_x = metrics->grid_region.location.x + grid_pad_x;
    cells_y = metrics->grid_region.location.y + grid_pad_y;
    cells_w = metrics->grid_region.size.width - grid_pad_x * 2;
    cells_h = metrics->grid_region.size.height - grid_pad_y * 2;
    if (cells_w <= 0 || cells_h <= 0)
    {
        return;
    }

    available_w = cells_w - cell_gap * (column_count - 1);
    available_h = cells_h - cell_gap * (row_count - 1);
    if (available_w <= 0 || available_h <= 0)
    {
        return;
    }

    base_w = available_w / column_count;
    rem_w = available_w % column_count;
    base_h = available_h / row_count;
    rem_h = available_h % row_count;

    y = cells_y;
    cell_index = 0;
    for (row_index = 0; row_index < row_count; ++row_index)
    {
        egui_dim_t x = cells_x;
        egui_dim_t cell_h = base_h + (row_index < rem_h ? 1 : 0);

        for (col_index = 0; col_index < column_count; ++col_index)
        {
            egui_dim_t cell_w = base_w + (col_index < rem_w ? 1 : 0);

            if (cell_index < cell_count)
            {
                metrics->cell_regions[cell_index].location.x = x;
                metrics->cell_regions[cell_index].location.y = y;
                metrics->cell_regions[cell_index].size.width = cell_w;
                metrics->cell_regions[cell_index].size.height = cell_h;
            }
            cell_index++;
            x += cell_w + cell_gap;
        }
        y += cell_h + cell_gap;
    }
}

static uint8_t uniform_grid_hit_cell(egui_view_uniform_grid_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    egui_view_uniform_grid_metrics_t metrics;
    const egui_view_uniform_grid_snapshot_t *snapshot = uniform_grid_get_snapshot(local);
    uint8_t cell_count = uniform_grid_get_cell_count(snapshot);
    uint8_t cell_index;

    uniform_grid_get_metrics(local, self, snapshot, &metrics);
    for (cell_index = 0; cell_index < cell_count; ++cell_index)
    {
        if (egui_region_pt_in_rect(&metrics.cell_regions[cell_index], x, y))
        {
            return cell_index;
        }
    }
    return EGUI_VIEW_UNIFORM_GRID_CELL_NONE;
}

static void uniform_grid_set_current_snapshot_inner(egui_view_t *self, uint8_t snapshot_index, uint8_t invalidate_on_clear)
{
    EGUI_LOCAL_INIT(egui_view_uniform_grid_t);
    uint8_t had_pressed = uniform_grid_clear_pressed_state(self, local);

    if (local->snapshot_count == 0 || snapshot_index >= local->snapshot_count)
    {
        if (had_pressed && invalidate_on_clear)
        {
            egui_view_invalidate(self);
        }
        return;
    }
    if (local->current_snapshot == snapshot_index)
    {
        if (had_pressed && invalidate_on_clear)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_snapshot = snapshot_index;
    local->current_cell = uniform_grid_resolve_default_cell(uniform_grid_get_snapshot(local));
    egui_view_invalidate(self);
}

static void uniform_grid_set_current_cell_inner(egui_view_t *self, uint8_t cell_index, uint8_t invalidate_on_clear)
{
    EGUI_LOCAL_INIT(egui_view_uniform_grid_t);
    const egui_view_uniform_grid_snapshot_t *snapshot = uniform_grid_get_snapshot(local);
    uint8_t had_pressed = uniform_grid_clear_pressed_state(self, local);

    if (!uniform_grid_cell_exists(snapshot, cell_index))
    {
        if (had_pressed && invalidate_on_clear)
        {
            egui_view_invalidate(self);
        }
        return;
    }
    if (local->current_cell == cell_index)
    {
        if (had_pressed && invalidate_on_clear)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_cell = cell_index;
    egui_view_invalidate(self);
}

static void uniform_grid_draw_cell(egui_view_t *self, egui_view_uniform_grid_t *local, const egui_view_uniform_grid_cell_t *cell,
                                   const egui_region_t *region, uint8_t cell_index)
{
    egui_color_t tone_color = uniform_grid_tone_color(local, cell->tone);
    egui_color_t fill_color =
            egui_rgb_mix(HCW_COLOR_PANEL, tone_color, EGUI_ALPHA_MAKE(cell_index == local->current_cell ? 4 : 0));
    egui_color_t border_color = egui_rgb_mix(local->border_color, tone_color, EGUI_ALPHA_MAKE(cell_index == local->current_cell ? 34 : 22));
    egui_color_t title_color = egui_rgb_mix(local->text_color, tone_color, EGUI_ALPHA_MAKE(cell_index == local->current_cell ? 14 : 6));
    egui_color_t meta_color = egui_rgb_mix(local->muted_text_color, tone_color, EGUI_ALPHA_MAKE(cell->emphasized ? 18 : 8));
    egui_color_t body_color = egui_rgb_mix(local->text_color, local->muted_text_color, EGUI_ALPHA_MAKE(local->compact_mode ? 26 : 14));
    egui_color_t badge_fill = egui_rgb_mix(HCW_COLOR_PANEL, tone_color, EGUI_ALPHA_MAKE(cell_index == local->current_cell ? 6 : 2));
    egui_color_t badge_text = egui_rgb_mix(local->text_color, tone_color, EGUI_ALPHA_MAKE(24));
    egui_color_t focus_color = tone_color;
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_CELL_RADIUS : EGUI_VIEW_UNIFORM_GRID_STANDARD_CELL_RADIUS;
    egui_dim_t pad_x = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_CELL_PAD_X : EGUI_VIEW_UNIFORM_GRID_STANDARD_CELL_PAD_X;
    egui_dim_t pad_y = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_CELL_PAD_Y : EGUI_VIEW_UNIFORM_GRID_STANDARD_CELL_PAD_Y;
    egui_dim_t content_left_pad = pad_x;
    egui_dim_t content_right_pad = pad_x;
    egui_dim_t content_width;
    egui_dim_t badge_slot_h = local->compact_mode ? 7 : 9;
    egui_dim_t title_slot_h = local->compact_mode ? 8 : 10;
    egui_dim_t meta_slot_h = local->compact_mode ? 7 : 8;
    egui_dim_t body_slot_h = local->compact_mode ? 0 : 8;
    egui_dim_t badge_h = uniform_grid_resolve_line_height(local->meta_font, badge_slot_h);
    egui_dim_t title_h = uniform_grid_resolve_line_height(local->font, title_slot_h);
    egui_dim_t meta_h = uniform_grid_resolve_line_height(local->meta_font, meta_slot_h);
    egui_dim_t body_h = body_slot_h > 0 ? uniform_grid_resolve_line_height(local->meta_font, body_slot_h) : 0;
    egui_region_t badge_region;
    egui_region_t title_region;
    egui_region_t meta_region;
    egui_region_t body_region;
    uint8_t is_current = cell_index == local->current_cell ? 1 : 0;
    egui_dim_t cursor_y = region->location.y + pad_y;
    egui_dim_t dense_threshold = local->compact_mode ? 32 : 44;
    char badge_label[24];
    char title_label[32];
    char meta_label[24];
    char body_label[24];

    if (region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    if (is_current)
    {
        egui_dim_t selected_left_pad = radius + EGUI_VIEW_UNIFORM_GRID_SELECTION_MARK_GAP;

        if (content_left_pad < selected_left_pad)
        {
            content_left_pad = selected_left_pad;
        }
    }
    content_width = region->size.width - content_left_pad - content_right_pad;

    if (cell_index == local->pressed_cell && self->is_pressed)
    {
        fill_color = egui_rgb_mix(fill_color, tone_color, EGUI_ALPHA_MAKE(5));
        border_color = egui_rgb_mix(border_color, tone_color, EGUI_ALPHA_MAKE(22));
    }
    if (local->read_only_mode)
    {
        fill_color = egui_rgb_mix(fill_color, HCW_COLOR_PANEL, EGUI_ALPHA_MAKE(34));
        border_color = egui_rgb_mix(border_color, local->muted_text_color, EGUI_ALPHA_MAKE(18));
        title_color = egui_rgb_mix(title_color, local->muted_text_color, EGUI_ALPHA_MAKE(16));
        meta_color = egui_rgb_mix(meta_color, local->muted_text_color, EGUI_ALPHA_MAKE(20));
        body_color = egui_rgb_mix(body_color, local->muted_text_color, EGUI_ALPHA_MAKE(24));
        badge_fill = egui_rgb_mix(badge_fill, HCW_COLOR_PANEL, EGUI_ALPHA_MAKE(34));
        badge_text = egui_rgb_mix(badge_text, local->muted_text_color, EGUI_ALPHA_MAKE(18));
        focus_color = egui_rgb_mix(focus_color, local->muted_text_color, EGUI_ALPHA_MAKE(16));
    }
    if (!egui_view_get_enable(self))
    {
        fill_color = uniform_grid_mix_disabled(fill_color);
        border_color = uniform_grid_mix_disabled(border_color);
        title_color = uniform_grid_mix_disabled(title_color);
        meta_color = uniform_grid_mix_disabled(meta_color);
        body_color = uniform_grid_mix_disabled(body_color);
        badge_fill = uniform_grid_mix_disabled(badge_fill);
        badge_text = uniform_grid_mix_disabled(badge_text);
        focus_color = uniform_grid_mix_disabled(focus_color);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region->location.x, region->location.y, region->size.width, region->size.height, radius, fill_color,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(96)));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, region->location.x, region->location.y, region->size.width, region->size.height, radius, 1, border_color,
                                     egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(74)));

    if (is_current)
    {
        hcw_selection_marker_draw_left(region, radius, radius, focus_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(96)));
    }
    if (self->is_focused && is_current && egui_view_get_enable(self) && !local->read_only_mode)
    {
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, region->location.x, region->location.y, region->size.width, region->size.height, radius, 2, focus_color,
                                         egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(82)));
    }
    if (content_width <= 0)
    {
        return;
    }

    if (region->size.height < dense_threshold)
    {
        egui_dim_t dense_pad_x = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_CELL_PAD_X : EGUI_VIEW_UNIFORM_GRID_STANDARD_CELL_PAD_X;
        egui_dim_t dense_pad_y = local->compact_mode ? 3 : 5;
        egui_dim_t dense_left_pad = content_left_pad > dense_pad_x ? content_left_pad : dense_pad_x;
        egui_dim_t dense_right_pad = content_right_pad > dense_pad_x ? content_right_pad : dense_pad_x;
        egui_dim_t dense_gap = local->compact_mode ? 2 : 3;
        egui_dim_t dense_x = region->location.x + dense_left_pad;
        egui_dim_t dense_y = region->location.y + dense_pad_y;
        egui_dim_t dense_w = region->size.width - dense_left_pad - dense_right_pad;
        egui_dim_t dense_bottom = region->location.y + region->size.height - dense_pad_y;
        egui_dim_t top_h = title_h;

        if (dense_w <= 0 || dense_bottom <= dense_y)
        {
            return;
        }
        if (top_h > dense_bottom - dense_y)
        {
            top_h = dense_bottom - dense_y;
        }

        title_region.location.x = dense_x;
        title_region.location.y = dense_y;
        title_region.size.width = dense_w;
        title_region.size.height = top_h;

        uniform_grid_fit_text_to_width(local->font, cell->title, title_label, sizeof(title_label), title_region.size.width, local->compact_mode ? 4 : 5);
        uniform_grid_draw_text(local->font, self, title_label, &title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);

        meta_region.location.x = dense_x;
        meta_region.location.y = dense_y + top_h + dense_gap;
        meta_region.size.width = dense_w;
        meta_region.size.height = meta_h;
        if (meta_region.location.y + meta_region.size.height <= dense_bottom)
        {
            uniform_grid_fit_text_to_width(local->meta_font, cell->meta, meta_label, sizeof(meta_label), meta_region.size.width, 4);
            uniform_grid_draw_text(local->meta_font, self, meta_label, &meta_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, meta_color);
        }
        return;
    }

    if (uniform_grid_has_text(cell->badge))
    {
        badge_region.location.x = region->location.x + content_left_pad;
        badge_region.location.y = cursor_y + (badge_slot_h - badge_h) / 2;
        badge_region.size.width = uniform_grid_pill_width(local->meta_font, cell->badge, local->compact_mode, local->compact_mode ? 12 : 16,
                                                          content_width);
        badge_region.size.height = badge_h;
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, badge_region.location.x, badge_region.location.y, badge_region.size.width, badge_region.size.height,
                                              badge_region.size.height / 2, badge_fill, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(96)));
        uniform_grid_fit_text_to_width(local->meta_font, cell->badge, badge_label, sizeof(badge_label), badge_region.size.width - 4, 4);
        uniform_grid_draw_text(local->meta_font, self, badge_label, &badge_region, EGUI_ALIGN_CENTER, badge_text);
        cursor_y += badge_slot_h + (local->compact_mode ? 2 : 3);
    }

    title_region.location.x = region->location.x + content_left_pad;
    title_region.location.y = cursor_y + (title_slot_h - title_h) / 2;
    title_region.size.width = content_width;
    title_region.size.height = title_h;
    cursor_y += title_slot_h;

    meta_region = title_region;
    meta_region.location.y = cursor_y + (meta_slot_h - meta_h) / 2;
    meta_region.size.height = meta_h;
    cursor_y += meta_slot_h;

    body_region = meta_region;
    body_region.location.y = cursor_y + 1 + (body_slot_h - body_h) / 2;
    body_region.size.height = body_h;

    uniform_grid_fit_text_to_width(local->font, cell->title, title_label, sizeof(title_label), title_region.size.width, local->compact_mode ? 4 : 5);
    uniform_grid_draw_text(local->font, self, title_label, &title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);
    uniform_grid_fit_text_to_width(local->meta_font, cell->meta, meta_label, sizeof(meta_label), meta_region.size.width, 4);
    uniform_grid_draw_text(local->meta_font, self, meta_label, &meta_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, meta_color);
    if (!local->compact_mode)
    {
        uniform_grid_fit_text_to_width(local->meta_font, cell->body, body_label, sizeof(body_label), body_region.size.width, 4);
        uniform_grid_draw_text(local->meta_font, self, body_label, &body_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, body_color);
    }
}

static void egui_view_uniform_grid_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_uniform_grid_t);
    const egui_view_uniform_grid_snapshot_t *snapshot = uniform_grid_get_snapshot(local);
    egui_view_uniform_grid_metrics_t metrics;
    egui_color_t card_fill = HCW_COLOR_PANEL;
    egui_color_t card_border = local->border_color;
    egui_color_t badge_fill = egui_rgb_mix(HCW_COLOR_PANEL, local->accent_color, EGUI_ALPHA_MAKE(6));
    egui_color_t badge_text = egui_rgb_mix(local->text_color, local->accent_color, EGUI_ALPHA_MAKE(24));
    egui_color_t title_color = local->text_color;
    egui_color_t summary_color = local->muted_text_color;
    egui_color_t grid_fill = HCW_COLOR_PANEL;
    egui_color_t grid_border = egui_rgb_mix(local->border_color, local->accent_color, EGUI_ALPHA_MAKE(26));
    egui_color_t footer_fill = HCW_COLOR_PANEL;
    egui_color_t footer_border = egui_rgb_mix(local->border_color, local->accent_color, EGUI_ALPHA_MAKE(20));
    egui_color_t footer_text = egui_rgb_mix(local->muted_text_color, local->accent_color, EGUI_ALPHA_MAKE(16));
    egui_color_t focus_color = local->accent_color;
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_RADIUS : EGUI_VIEW_UNIFORM_GRID_STANDARD_RADIUS;
    egui_dim_t grid_radius = local->compact_mode ? EGUI_VIEW_UNIFORM_GRID_COMPACT_GRID_RADIUS : EGUI_VIEW_UNIFORM_GRID_STANDARD_GRID_RADIUS;
    uint8_t cell_count;
    uint8_t cell_index;
    char header_label[24];
    char title_label[40];
    char summary_label[40];
    char footer_label[24];

    uniform_grid_sync_current_cell(local);
    uniform_grid_get_metrics(local, self, snapshot, &metrics);
    if (metrics.content_region.size.width <= 0 || metrics.content_region.size.height <= 0)
    {
        return;
    }

    if (local->read_only_mode)
    {
        badge_fill = egui_rgb_mix(badge_fill, HCW_COLOR_PANEL, EGUI_ALPHA_MAKE(34));
        badge_text = egui_rgb_mix(badge_text, local->muted_text_color, EGUI_ALPHA_MAKE(18));
        title_color = egui_rgb_mix(title_color, local->muted_text_color, EGUI_ALPHA_MAKE(12));
        summary_color = egui_rgb_mix(summary_color, local->muted_text_color, EGUI_ALPHA_MAKE(18));
        grid_fill = egui_rgb_mix(grid_fill, HCW_COLOR_PANEL, EGUI_ALPHA_MAKE(34));
        grid_border = egui_rgb_mix(grid_border, local->muted_text_color, EGUI_ALPHA_MAKE(18));
        footer_fill = egui_rgb_mix(footer_fill, HCW_COLOR_PANEL, EGUI_ALPHA_MAKE(34));
        footer_border = egui_rgb_mix(footer_border, local->muted_text_color, EGUI_ALPHA_MAKE(18));
        footer_text = egui_rgb_mix(footer_text, local->muted_text_color, EGUI_ALPHA_MAKE(18));
        focus_color = egui_rgb_mix(focus_color, local->muted_text_color, EGUI_ALPHA_MAKE(16));
    }
    if (!egui_view_get_enable(self))
    {
        card_fill = uniform_grid_mix_disabled(card_fill);
        card_border = uniform_grid_mix_disabled(card_border);
        badge_fill = uniform_grid_mix_disabled(badge_fill);
        badge_text = uniform_grid_mix_disabled(badge_text);
        title_color = uniform_grid_mix_disabled(title_color);
        summary_color = uniform_grid_mix_disabled(summary_color);
        grid_fill = uniform_grid_mix_disabled(grid_fill);
        grid_border = uniform_grid_mix_disabled(grid_border);
        footer_fill = uniform_grid_mix_disabled(footer_fill);
        footer_border = uniform_grid_mix_disabled(footer_border);
        footer_text = uniform_grid_mix_disabled(footer_text);
        focus_color = uniform_grid_mix_disabled(focus_color);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.content_region.location.x, metrics.content_region.location.y, metrics.content_region.size.width,
                                          metrics.content_region.size.height, radius, card_fill, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(96)));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics.content_region.location.x, metrics.content_region.location.y, metrics.content_region.size.width,
                                     metrics.content_region.size.height, radius, 1, card_border, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(76)));

    if (self->is_focused && egui_view_get_enable(self) && !local->read_only_mode)
    {
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics.content_region.location.x, metrics.content_region.location.y, metrics.content_region.size.width,
                                         metrics.content_region.size.height, radius, 2, focus_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(82)));
    }

    if (snapshot != NULL && metrics.badge_region.size.width > 0)
    {
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.badge_region.location.x, metrics.badge_region.location.y, metrics.badge_region.size.width,
                                              metrics.badge_region.size.height, metrics.badge_region.size.height / 2, badge_fill,
                                              egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(98)));
        uniform_grid_fit_text_to_width(local->meta_font, snapshot->header, header_label, sizeof(header_label), metrics.badge_region.size.width - 4, 4);
        uniform_grid_draw_text(local->meta_font, self, header_label, &metrics.badge_region, EGUI_ALIGN_CENTER, badge_text);
    }

    if (snapshot != NULL)
    {
        uniform_grid_fit_text_to_width(local->font, snapshot->title, title_label, sizeof(title_label), metrics.title_region.size.width,
                                       local->compact_mode ? 4 : 5);
        uniform_grid_draw_text(local->font, self, title_label, &metrics.title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);
        uniform_grid_fit_text_to_width(local->meta_font, snapshot->summary, summary_label, sizeof(summary_label), metrics.summary_region.size.width, 4);
        uniform_grid_draw_text(local->meta_font, self, summary_label, &metrics.summary_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, summary_color);
    }

    if (metrics.grid_region.size.width > 0 && metrics.grid_region.size.height > 0)
    {
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.grid_region.location.x, metrics.grid_region.location.y, metrics.grid_region.size.width,
                                              metrics.grid_region.size.height, grid_radius, grid_fill, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(98)));
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics.grid_region.location.x, metrics.grid_region.location.y, metrics.grid_region.size.width,
                                         metrics.grid_region.size.height, grid_radius, 1, grid_border, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(78)));
    }

    cell_count = uniform_grid_get_cell_count(snapshot);
    for (cell_index = 0; cell_index < cell_count; ++cell_index)
    {
        const egui_view_uniform_grid_cell_t *cell = uniform_grid_get_cell(snapshot, cell_index);

        if (cell == NULL || metrics.cell_regions[cell_index].size.width <= 0 || metrics.cell_regions[cell_index].size.height <= 0)
        {
            continue;
        }
        uniform_grid_draw_cell(self, local, cell, &metrics.cell_regions[cell_index], cell_index);
    }

    if (snapshot != NULL && metrics.footer_region.size.width > 0)
    {
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                              metrics.footer_region.size.height, metrics.footer_region.size.height / 2, footer_fill,
                                              egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(96)));
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                         metrics.footer_region.size.height, metrics.footer_region.size.height / 2, 1, footer_border,
                                         egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(76)));
        uniform_grid_fit_text_to_width(local->meta_font, snapshot->footer, footer_label, sizeof(footer_label), metrics.footer_region.size.width - 4, 4);
        uniform_grid_draw_text(local->meta_font, self, footer_label, &metrics.footer_region, EGUI_ALIGN_CENTER, footer_text);
    }
}

void egui_view_uniform_grid_set_snapshots(egui_view_t *self, const egui_view_uniform_grid_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_uniform_grid_t);

    uniform_grid_clear_pressed_state(self, local);
    local->snapshots = snapshots;
    local->snapshot_count = snapshots == NULL ? 0 : uniform_grid_clamp_snapshot_count(snapshot_count);
    if (local->current_snapshot >= local->snapshot_count)
    {
        local->current_snapshot = 0;
    }

    local->current_cell = uniform_grid_resolve_default_cell(uniform_grid_get_snapshot(local));
    egui_view_invalidate(self);
}

void egui_view_uniform_grid_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    uniform_grid_set_current_snapshot_inner(self, snapshot_index, 1);
}

uint8_t egui_view_uniform_grid_get_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_uniform_grid_t);
    return local->current_snapshot;
}

void egui_view_uniform_grid_set_current_cell(egui_view_t *self, uint8_t cell_index)
{
    uniform_grid_set_current_cell_inner(self, cell_index, 1);
}

uint8_t egui_view_uniform_grid_get_current_cell(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_uniform_grid_t);

    uniform_grid_sync_current_cell(local);
    return local->current_cell;
}

uint8_t egui_view_uniform_grid_activate_current_cell(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_uniform_grid_t);
    const egui_view_uniform_grid_snapshot_t *snapshot = uniform_grid_get_snapshot(local);

    uniform_grid_sync_current_cell(local);
    if (!uniform_grid_cell_is_interactive(local, self, snapshot, local->current_cell))
    {
        return 0;
    }

    if (local->on_action != NULL)
    {
        local->on_action(self, local->current_snapshot, local->current_cell);
    }
    egui_view_invalidate(self);
    return 1;
}

void egui_view_uniform_grid_set_on_action_listener(egui_view_t *self, egui_view_on_uniform_grid_action_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_uniform_grid_t);
    local->on_action = listener;
}

void egui_view_uniform_grid_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_uniform_grid_t);

    local->font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    uniform_grid_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_uniform_grid_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_uniform_grid_t);

    local->meta_font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    uniform_grid_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_uniform_grid_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_uniform_grid_t);

    local->compact_mode = compact_mode ? 1 : 0;
    uniform_grid_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_uniform_grid_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_uniform_grid_t);

    local->read_only_mode = read_only_mode ? 1 : 0;
    uniform_grid_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_uniform_grid_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                                        egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                        egui_color_t success_color, egui_color_t warning_color, egui_color_t neutral_color)
{
    EGUI_LOCAL_INIT(egui_view_uniform_grid_t);

    local->surface_color = surface_color;
    local->section_color = section_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->neutral_color = neutral_color;
    uniform_grid_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

uint8_t egui_view_uniform_grid_get_cell_region(egui_view_t *self, uint8_t cell_index, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_uniform_grid_t);
    const egui_view_uniform_grid_snapshot_t *snapshot = uniform_grid_get_snapshot(local);
    egui_view_uniform_grid_metrics_t metrics;

    if (region == NULL || !uniform_grid_cell_exists(snapshot, cell_index))
    {
        return 0;
    }

    uniform_grid_get_metrics(local, self, snapshot, &metrics);
    *region = metrics.cell_regions[cell_index];
    return metrics.cell_regions[cell_index].size.width > 0 && metrics.cell_regions[cell_index].size.height > 0 ? 1 : 0;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_uniform_grid_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_uniform_grid_t);
    const egui_view_uniform_grid_snapshot_t *snapshot = uniform_grid_get_snapshot(local);
    uint8_t hit_cell;

    if (snapshot == NULL || uniform_grid_get_cell_count(snapshot) == 0 || local->read_only_mode || !egui_view_get_enable(self))
    {
        if (uniform_grid_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_cell = uniform_grid_hit_cell(local, self, event->location.x, event->location.y);
        if (!uniform_grid_cell_is_interactive(local, self, snapshot, hit_cell))
        {
            if (uniform_grid_clear_pressed_state(self, local))
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
        local->pressed_cell = hit_cell;
        egui_view_set_pressed(self, true);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_cell == EGUI_VIEW_UNIFORM_GRID_CELL_NONE)
        {
            return 0;
        }
        hit_cell = uniform_grid_hit_cell(local, self, event->location.x, event->location.y);
        egui_view_set_pressed(self, hit_cell == local->pressed_cell);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
    {
        uint8_t handled = local->pressed_cell != EGUI_VIEW_UNIFORM_GRID_CELL_NONE ? 1 : 0;

        hit_cell = uniform_grid_hit_cell(local, self, event->location.x, event->location.y);
        if (hit_cell == local->pressed_cell && uniform_grid_cell_is_interactive(local, self, snapshot, hit_cell))
        {
            local->current_cell = hit_cell;
            egui_view_uniform_grid_activate_current_cell(self);
        }
        if (uniform_grid_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return handled || hit_cell != EGUI_VIEW_UNIFORM_GRID_CELL_NONE;
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (uniform_grid_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}

static int egui_view_uniform_grid_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_uniform_grid_t);
    EGUI_UNUSED(event);

    if (uniform_grid_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_uniform_grid_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_uniform_grid_t);
    const egui_view_uniform_grid_snapshot_t *snapshot = uniform_grid_get_snapshot(local);
    uint8_t cell_count;
    uint8_t column_count;
    uint8_t next_cell;
    uint8_t next_snapshot;

    if (snapshot == NULL || local->read_only_mode || !egui_view_get_enable(self))
    {
        if (uniform_grid_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    cell_count = uniform_grid_get_cell_count(snapshot);
    if (cell_count == 0)
    {
        return 0;
    }

    column_count = uniform_grid_get_column_count(snapshot);
    uniform_grid_sync_current_cell(local);
    if (event->key_code == EGUI_KEY_CODE_ENTER || event->key_code == EGUI_KEY_CODE_SPACE)
    {
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            if (!uniform_grid_cell_is_interactive(local, self, snapshot, local->current_cell))
            {
                return 0;
            }
            local->pressed_cell = local->current_cell;
            egui_view_set_pressed(self, true);
            egui_view_invalidate(self);
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            uint8_t handled = 0;

            if (local->pressed_cell != EGUI_VIEW_UNIFORM_GRID_CELL_NONE && local->pressed_cell == local->current_cell &&
                uniform_grid_cell_is_interactive(local, self, snapshot, local->pressed_cell))
            {
                handled = egui_view_uniform_grid_activate_current_cell(self);
            }
            if (uniform_grid_clear_pressed_state(self, local))
            {
                egui_view_invalidate(self);
            }
            return handled;
        }
        return 0;
    }

    if (uniform_grid_clear_pressed_state(self, local))
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
        next_cell = local->current_cell > 0 ? (local->current_cell - 1) : 0;
        uniform_grid_set_current_cell_inner(self, next_cell, 0);
        return 1;
    case EGUI_KEY_CODE_RIGHT:
        next_cell = local->current_cell + 1 < cell_count ? (local->current_cell + 1) : (cell_count - 1);
        uniform_grid_set_current_cell_inner(self, next_cell, 0);
        return 1;
    case EGUI_KEY_CODE_UP:
        next_cell = local->current_cell >= column_count ? (uint8_t)(local->current_cell - column_count) : 0;
        uniform_grid_set_current_cell_inner(self, next_cell, 0);
        return 1;
    case EGUI_KEY_CODE_DOWN:
        next_cell = (uint8_t)(local->current_cell + column_count);
        if (next_cell >= cell_count)
        {
            next_cell = cell_count - 1;
        }
        uniform_grid_set_current_cell_inner(self, next_cell, 0);
        return 1;
    case EGUI_KEY_CODE_HOME:
        uniform_grid_set_current_cell_inner(self, 0, 0);
        return 1;
    case EGUI_KEY_CODE_END:
        uniform_grid_set_current_cell_inner(self, cell_count - 1, 0);
        return 1;
    case EGUI_KEY_CODE_TAB:
        next_cell = (uint8_t)(local->current_cell + 1);
        if (next_cell < cell_count)
        {
            uniform_grid_set_current_cell_inner(self, next_cell, 0);
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
            local->current_cell = uniform_grid_resolve_default_cell(uniform_grid_get_snapshot(local));
            egui_view_invalidate(self);
        }
        return 1;
    default:
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_uniform_grid_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_uniform_grid_t);
    EGUI_UNUSED(event);

    if (uniform_grid_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_uniform_grid_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_uniform_grid_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_uniform_grid_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_uniform_grid_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_uniform_grid_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_uniform_grid_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_uniform_grid_on_key_event,
#endif
};

void egui_view_uniform_grid_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_uniform_grid_t);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_uniform_grid_t);
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
    local->current_cell = EGUI_VIEW_UNIFORM_GRID_CELL_NONE;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_cell = EGUI_VIEW_UNIFORM_GRID_CELL_NONE;

    egui_view_set_view_name(self, "egui_view_uniform_grid");
}
