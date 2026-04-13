#include "egui_view_settings_expander.h"

#define EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_RADIUS        10
#define EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_PAD_X         8
#define EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_PAD_Y         8
#define EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_HEADER_HEIGHT 36
#define EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_BODY_GAP      5
#define EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_BODY_RADIUS   8
#define EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_ROW_HEIGHT    18
#define EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_ROW_GAP       3
#define EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_BADGE_H       11
#define EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_FOOTER_H      12
#define EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_ICON_SIZE     14
#define EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_SWITCH_W      24
#define EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_SWITCH_H      12

#define EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_RADIUS        8
#define EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_PAD_X         5
#define EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_PAD_Y         5
#define EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_HEADER_HEIGHT 22
#define EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_BODY_GAP      4
#define EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_BODY_RADIUS   6
#define EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_ROW_HEIGHT    14
#define EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_ROW_GAP       2
#define EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_BADGE_H       8
#define EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_FOOTER_H      0
#define EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_ICON_SIZE     10
#define EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_SWITCH_W      20
#define EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_SWITCH_H      10

typedef struct egui_view_settings_expander_metrics egui_view_settings_expander_metrics_t;
struct egui_view_settings_expander_metrics
{
    egui_region_t region;
    egui_region_t content_region;
    egui_region_t header_region;
    egui_region_t body_region;
    egui_region_t footer_region;
    egui_region_t row_regions[EGUI_VIEW_SETTINGS_EXPANDER_MAX_ROWS];
    uint8_t row_count;
    uint8_t visible_row_count;
};

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_settings_expander_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event);
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_settings_expander_on_static_key_event(egui_view_t *self, egui_key_event_t *event);
#endif

static uint8_t egui_view_settings_expander_clamp_snapshot_count(uint8_t count)
{
    return count > EGUI_VIEW_SETTINGS_EXPANDER_MAX_SNAPSHOTS ? EGUI_VIEW_SETTINGS_EXPANDER_MAX_SNAPSHOTS : count;
}

static uint8_t egui_view_settings_expander_clamp_row_count(uint8_t count)
{
    return count > EGUI_VIEW_SETTINGS_EXPANDER_MAX_ROWS ? EGUI_VIEW_SETTINGS_EXPANDER_MAX_ROWS : count;
}

static uint8_t egui_view_settings_expander_text_len(const char *text)
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

static uint8_t egui_view_settings_expander_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static egui_color_t egui_view_settings_expander_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 68);
}

static const egui_view_settings_expander_snapshot_t *egui_view_settings_expander_get_snapshot(egui_view_settings_expander_t *local)
{
    if (local->snapshots == NULL || local->snapshot_count == 0 || local->current_snapshot >= local->snapshot_count)
    {
        return NULL;
    }

    return &local->snapshots[local->current_snapshot];
}

static egui_color_t egui_view_settings_expander_tone_color(egui_view_settings_expander_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_SETTINGS_EXPANDER_TONE_SUCCESS:
        return local->success_color;
    case EGUI_VIEW_SETTINGS_EXPANDER_TONE_WARNING:
        return local->warning_color;
    case EGUI_VIEW_SETTINGS_EXPANDER_TONE_NEUTRAL:
        return local->neutral_color;
    case EGUI_VIEW_SETTINGS_EXPANDER_TONE_ACCENT:
    default:
        return local->accent_color;
    }
}

static uint8_t egui_view_settings_expander_clear_pressed_state(egui_view_t *self, egui_view_settings_expander_t *local)
{
    uint8_t had_pressed = self->is_pressed || local->pressed_part != EGUI_VIEW_SETTINGS_EXPANDER_PART_NONE;

    local->pressed_part = EGUI_VIEW_SETTINGS_EXPANDER_PART_NONE;
    egui_view_set_pressed(self, false);
    return had_pressed;
}

static uint8_t egui_view_settings_expander_part_to_row_index(uint8_t part)
{
    if (part < EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE)
    {
        return EGUI_VIEW_SETTINGS_EXPANDER_MAX_ROWS;
    }

    return (uint8_t)(part - EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE);
}

static uint8_t egui_view_settings_expander_row_part(uint8_t row_index)
{
    return (uint8_t)(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + row_index);
}

static uint8_t egui_view_settings_expander_part_exists(const egui_view_settings_expander_snapshot_t *snapshot, uint8_t expanded_state, uint8_t part)
{
    uint8_t row_index;

    if (snapshot == NULL)
    {
        return 0;
    }

    if (part == EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER)
    {
        return 1;
    }

    if (!expanded_state)
    {
        return 0;
    }

    row_index = egui_view_settings_expander_part_to_row_index(part);
    if (row_index >= egui_view_settings_expander_clamp_row_count(snapshot->row_count))
    {
        return 0;
    }

    return 1;
}

static uint8_t egui_view_settings_expander_part_is_interactive(egui_view_settings_expander_t *local, egui_view_t *self,
                                                               const egui_view_settings_expander_snapshot_t *snapshot, uint8_t part)
{
    if (snapshot == NULL || local->read_only_mode || !egui_view_get_enable(self))
    {
        return 0;
    }

    return egui_view_settings_expander_part_exists(snapshot, local->expanded_state, part);
}

static uint8_t egui_view_settings_expander_find_first_row(const egui_view_settings_expander_snapshot_t *snapshot, uint8_t expanded_state)
{
    if (!expanded_state || snapshot == NULL || egui_view_settings_expander_clamp_row_count(snapshot->row_count) == 0)
    {
        return EGUI_VIEW_SETTINGS_EXPANDER_PART_NONE;
    }

    return EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE;
}

static uint8_t egui_view_settings_expander_find_last_row(const egui_view_settings_expander_snapshot_t *snapshot, uint8_t expanded_state)
{
    uint8_t row_count;

    if (!expanded_state || snapshot == NULL)
    {
        return EGUI_VIEW_SETTINGS_EXPANDER_PART_NONE;
    }

    row_count = egui_view_settings_expander_clamp_row_count(snapshot->row_count);
    if (row_count == 0)
    {
        return EGUI_VIEW_SETTINGS_EXPANDER_PART_NONE;
    }

    return egui_view_settings_expander_row_part((uint8_t)(row_count - 1));
}

static uint8_t egui_view_settings_expander_resolve_default_part(egui_view_settings_expander_t *local,
                                                                const egui_view_settings_expander_snapshot_t *snapshot)
{
    uint8_t focus_part;
    uint8_t first_row;

    if (snapshot == NULL)
    {
        return EGUI_VIEW_SETTINGS_EXPANDER_PART_NONE;
    }

    if (!local->expanded_state)
    {
        return EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER;
    }

    focus_part = snapshot->focus_part;
    if (egui_view_settings_expander_part_exists(snapshot, local->expanded_state, focus_part))
    {
        return focus_part;
    }

    first_row = egui_view_settings_expander_find_first_row(snapshot, local->expanded_state);
    if (first_row != EGUI_VIEW_SETTINGS_EXPANDER_PART_NONE)
    {
        return first_row;
    }

    return EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER;
}

static void egui_view_settings_expander_sync_current_part(egui_view_settings_expander_t *local)
{
    const egui_view_settings_expander_snapshot_t *snapshot = egui_view_settings_expander_get_snapshot(local);

    if (!egui_view_settings_expander_part_exists(snapshot, local->expanded_state, local->current_part))
    {
        local->current_part = egui_view_settings_expander_resolve_default_part(local, snapshot);
    }
}

static egui_dim_t egui_view_settings_expander_pill_width(const char *text, uint8_t compact_mode, egui_dim_t min_width, egui_dim_t max_width)
{
    egui_dim_t width = min_width;

    if (egui_view_settings_expander_has_text(text))
    {
        width += egui_view_settings_expander_text_len(text) * (compact_mode ? 4 : 5);
    }

    if (width > max_width)
    {
        width = max_width;
    }

    return width;
}

static void egui_view_settings_expander_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region,
                                                  uint8_t align, egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (!egui_view_settings_expander_has_text(text) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, self->alpha);
}

static void egui_view_settings_expander_draw_switch(egui_view_t *self, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height,
                                                    uint8_t checked, egui_color_t tone_color, egui_color_t muted_color)
{
    egui_color_t track_color = checked ? egui_rgb_mix(EGUI_COLOR_WHITE, tone_color, 56) : egui_rgb_mix(EGUI_COLOR_WHITE, muted_color, 26);
    egui_color_t border_color = checked ? egui_rgb_mix(muted_color, tone_color, 36) : egui_rgb_mix(muted_color, EGUI_COLOR_WHITE, 10);
    egui_color_t thumb_color = checked ? EGUI_COLOR_WHITE : egui_rgb_mix(EGUI_COLOR_WHITE, muted_color, 18);
    egui_dim_t radius = height / 2;
    egui_dim_t thumb_r = radius - 2;
    egui_dim_t thumb_x = checked ? (x + width - radius) : (x + radius);
    egui_dim_t thumb_y = y + radius;

    if (thumb_r < 1)
    {
        thumb_r = 1;
    }

    egui_canvas_draw_round_rectangle_fill(x, y, width, height, radius, track_color, egui_color_alpha_mix(self->alpha, 88));
    egui_canvas_draw_round_rectangle(x, y, width, height, radius, 1, border_color, egui_color_alpha_mix(self->alpha, 36));
    egui_canvas_draw_circle_fill(thumb_x, thumb_y, thumb_r, thumb_color, egui_color_alpha_mix(self->alpha, 100));
}

static void egui_view_settings_expander_draw_chevron(egui_view_t *self, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height,
                                                     uint8_t expanded, egui_color_t color)
{
    if (expanded)
    {
        egui_canvas_draw_triangle_fill(x, y + 1, x + width, y + 1, x + width / 2, y + height - 1, color, egui_color_alpha_mix(self->alpha, 92));
    }
    else
    {
        egui_canvas_draw_triangle_fill(x + 1, y, x + 1, y + height, x + width - 1, y + height / 2, color, egui_color_alpha_mix(self->alpha, 92));
    }
}

static void egui_view_settings_expander_get_metrics(egui_view_settings_expander_t *local, egui_view_t *self,
                                                    const egui_view_settings_expander_snapshot_t *snapshot,
                                                    egui_view_settings_expander_metrics_t *metrics)
{
    egui_dim_t pad_x = local->compact_mode ? EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_PAD_X : EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_PAD_X;
    egui_dim_t pad_y = local->compact_mode ? EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_PAD_Y : EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_PAD_Y;
    egui_dim_t header_h = local->compact_mode ? EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_HEADER_HEIGHT : EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_HEADER_HEIGHT;
    egui_dim_t body_gap = local->compact_mode ? EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_BODY_GAP : EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_BODY_GAP;
    egui_dim_t row_h = local->compact_mode ? EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_ROW_HEIGHT : EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_ROW_HEIGHT;
    egui_dim_t row_gap = local->compact_mode ? EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_ROW_GAP : EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_ROW_GAP;
    egui_dim_t footer_h = local->compact_mode ? EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_FOOTER_H : EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_FOOTER_H;
    egui_dim_t available_body_h;
    egui_dim_t cursor_y;
    uint8_t row_count;
    uint8_t i;

    egui_view_get_work_region(self, &metrics->region);
    metrics->content_region = metrics->region;
    metrics->header_region.size.width = 0;
    metrics->header_region.size.height = 0;
    metrics->body_region.size.width = 0;
    metrics->body_region.size.height = 0;
    metrics->footer_region.size.width = 0;
    metrics->footer_region.size.height = 0;
    metrics->row_count = snapshot == NULL ? 0 : egui_view_settings_expander_clamp_row_count(snapshot->row_count);
    metrics->visible_row_count = 0;

    for (i = 0; i < EGUI_VIEW_SETTINGS_EXPANDER_MAX_ROWS; ++i)
    {
        metrics->row_regions[i].size.width = 0;
        metrics->row_regions[i].size.height = 0;
    }

    if (metrics->region.size.width <= 0 || metrics->region.size.height <= 0)
    {
        return;
    }

    metrics->content_region.location.x = metrics->region.location.x + pad_x;
    metrics->content_region.location.y = metrics->region.location.y + pad_y;
    metrics->content_region.size.width = metrics->region.size.width - pad_x * 2;
    metrics->content_region.size.height = metrics->region.size.height - pad_y * 2;

    if (metrics->content_region.size.width <= 0 || metrics->content_region.size.height <= 0 || snapshot == NULL)
    {
        return;
    }

    metrics->header_region.location = metrics->content_region.location;
    metrics->header_region.size.width = metrics->content_region.size.width;
    metrics->header_region.size.height = header_h;

    if (!local->expanded_state || metrics->row_count == 0)
    {
        return;
    }

    available_body_h = metrics->content_region.size.height - header_h - body_gap;
    if (available_body_h <= 0)
    {
        return;
    }

    metrics->body_region.location.x = metrics->content_region.location.x;
    metrics->body_region.location.y = metrics->header_region.location.y + metrics->header_region.size.height + body_gap;
    metrics->body_region.size.width = metrics->content_region.size.width;
    metrics->body_region.size.height = available_body_h;

    if (!local->compact_mode && egui_view_settings_expander_has_text(snapshot->footer))
    {
        metrics->footer_region.location.x = metrics->body_region.location.x + 6;
        metrics->footer_region.location.y = metrics->body_region.location.y + metrics->body_region.size.height - footer_h - 5;
        metrics->footer_region.size.width = metrics->body_region.size.width - 12;
        metrics->footer_region.size.height = footer_h;
    }

    cursor_y = metrics->body_region.location.y + (local->compact_mode ? 4 : 6);
    row_count = metrics->row_count;
    for (i = 0; i < row_count; ++i)
    {
        egui_dim_t row_limit_y;

        metrics->row_regions[i].location.x = metrics->body_region.location.x + 4;
        metrics->row_regions[i].location.y = cursor_y;
        metrics->row_regions[i].size.width = metrics->body_region.size.width - 8;
        metrics->row_regions[i].size.height = row_h;

        row_limit_y = metrics->body_region.location.y + metrics->body_region.size.height - 4;
        if (metrics->footer_region.size.height > 0)
        {
            row_limit_y = metrics->footer_region.location.y - 4;
        }

        if (metrics->row_regions[i].location.y + metrics->row_regions[i].size.height > row_limit_y)
        {
            metrics->row_regions[i].size.width = 0;
            metrics->row_regions[i].size.height = 0;
            break;
        }

        metrics->visible_row_count++;
        cursor_y += row_h;
        if (i + 1 < row_count)
        {
            cursor_y += row_gap;
        }
    }
}

static uint8_t egui_view_settings_expander_resolve_hit(egui_view_settings_expander_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    egui_view_settings_expander_metrics_t metrics;
    const egui_view_settings_expander_snapshot_t *snapshot = egui_view_settings_expander_get_snapshot(local);
    uint8_t i;

    egui_view_settings_expander_get_metrics(local, self, snapshot, &metrics);
    if (egui_region_pt_in_rect(&metrics.header_region, x, y))
    {
        return EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER;
    }

    for (i = 0; i < metrics.visible_row_count; ++i)
    {
        if (egui_region_pt_in_rect(&metrics.row_regions[i], x, y))
        {
            return egui_view_settings_expander_row_part(i);
        }
    }

    return EGUI_VIEW_SETTINGS_EXPANDER_PART_NONE;
}

static void egui_view_settings_expander_set_current_part_inner(egui_view_t *self, uint8_t part, uint8_t invalidate_on_clear)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);
    const egui_view_settings_expander_snapshot_t *snapshot = egui_view_settings_expander_get_snapshot(local);
    uint8_t had_pressed = egui_view_settings_expander_clear_pressed_state(self, local);

    if (!egui_view_settings_expander_part_exists(snapshot, local->expanded_state, part))
    {
        if (had_pressed && invalidate_on_clear)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    if (local->current_part == part)
    {
        if (had_pressed && invalidate_on_clear)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_part = part;
    egui_view_invalidate(self);
}

static void egui_view_settings_expander_move_linear(egui_view_t *self, int8_t step, uint8_t wrap)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);
    const egui_view_settings_expander_snapshot_t *snapshot = egui_view_settings_expander_get_snapshot(local);
    uint8_t parts[EGUI_VIEW_SETTINGS_EXPANDER_MAX_ROWS + 1];
    uint8_t count = 0;
    uint8_t current = 0;
    int16_t next;
    uint8_t row_count;

    if (snapshot == NULL)
    {
        return;
    }

    parts[count++] = EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER;
    row_count = local->expanded_state ? egui_view_settings_expander_clamp_row_count(snapshot->row_count) : 0;
    while (count < row_count + 1)
    {
        parts[count] = egui_view_settings_expander_row_part((uint8_t)(count - 1));
        count++;
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

    next = (int16_t)current + step;
    if (wrap)
    {
        if (next < 0)
        {
            next = (int16_t)(count - 1);
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

static void egui_view_settings_expander_draw_header(egui_view_t *self, egui_view_settings_expander_t *local,
                                                    const egui_view_settings_expander_snapshot_t *snapshot, const egui_region_t *region,
                                                    uint8_t focused, uint8_t pressed)
{
    egui_region_t text_region;
    egui_color_t tone_color = egui_view_settings_expander_tone_color(local, snapshot->tone);
    egui_color_t fill_color = egui_rgb_mix(local->section_color, tone_color, local->expanded_state ? 16 : (focused ? 12 : 7));
    egui_color_t border_color = egui_rgb_mix(local->border_color, tone_color, local->expanded_state ? 24 : (focused ? 18 : 10));
    egui_color_t title_color = focused || local->expanded_state ? egui_rgb_mix(local->text_color, tone_color, 8) : local->text_color;
    egui_color_t desc_color = egui_rgb_mix(local->muted_text_color, tone_color, focused ? 16 : 10);
    egui_color_t badge_fill = egui_rgb_mix(local->surface_color, tone_color, local->expanded_state ? 14 : 10);
    egui_color_t badge_border = egui_rgb_mix(local->border_color, tone_color, local->expanded_state ? 20 : 14);
    egui_color_t badge_color = tone_color;
    egui_color_t value_fill = egui_rgb_mix(local->surface_color, tone_color, local->expanded_state ? 12 : 8);
    egui_color_t value_border = egui_rgb_mix(local->border_color, tone_color, local->expanded_state ? 18 : 14);
    egui_color_t value_color = egui_rgb_mix(local->muted_text_color, tone_color, local->expanded_state ? 18 : 12);
    egui_color_t icon_fill = egui_rgb_mix(local->surface_color, tone_color, focused ? 10 : 6);
    egui_color_t icon_color = egui_rgb_mix(local->text_color, tone_color, focused ? 16 : 10);
    egui_dim_t radius = local->compact_mode ? 6 : 8;
    egui_dim_t icon_size = local->compact_mode ? EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_ICON_SIZE : EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_ICON_SIZE;
    egui_dim_t badge_h = local->compact_mode ? EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_BADGE_H : EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_BADGE_H;
    egui_dim_t inner_x = region->location.x + (local->compact_mode ? 5 : 7);
    egui_dim_t inner_y = region->location.y + (local->compact_mode ? 4 : 5);
    egui_dim_t chevron_w = local->compact_mode ? 8 : 10;
    egui_dim_t chevron_h = local->compact_mode ? 8 : 10;
    egui_dim_t chevron_x = region->location.x + region->size.width - (local->compact_mode ? 7 : 9) - chevron_w;
    egui_dim_t chevron_y = region->location.y + (region->size.height - chevron_h) / 2;
    egui_dim_t value_w = egui_view_settings_expander_pill_width(snapshot->value, local->compact_mode, local->compact_mode ? 18 : 24,
                                                                region->size.width / 3);
    egui_dim_t value_h = local->compact_mode ? 10 : 12;
    egui_dim_t value_x = chevron_x - (egui_view_settings_expander_has_text(snapshot->value) ? (value_w + (local->compact_mode ? 4 : 6)) : 0);
    egui_dim_t title_x = inner_x + icon_size + 7;
    egui_dim_t text_right = value_x > title_x ? value_x - 4 : chevron_x - 4;

    if (pressed)
    {
        fill_color = egui_rgb_mix(fill_color, tone_color, 12);
        border_color = egui_rgb_mix(border_color, tone_color, 18);
    }

    if (local->read_only_mode)
    {
        fill_color = egui_rgb_mix(fill_color, local->surface_color, 28);
        border_color = egui_rgb_mix(border_color, local->muted_text_color, 18);
        title_color = egui_rgb_mix(title_color, local->muted_text_color, 22);
        desc_color = egui_rgb_mix(desc_color, local->muted_text_color, 28);
        badge_fill = egui_rgb_mix(badge_fill, local->surface_color, 28);
        badge_border = egui_rgb_mix(badge_border, local->muted_text_color, 18);
        badge_color = egui_rgb_mix(badge_color, local->muted_text_color, 32);
        value_fill = egui_rgb_mix(value_fill, local->surface_color, 28);
        value_border = egui_rgb_mix(value_border, local->muted_text_color, 18);
        value_color = egui_rgb_mix(value_color, local->muted_text_color, 30);
        icon_fill = egui_rgb_mix(icon_fill, local->surface_color, 28);
        icon_color = egui_rgb_mix(icon_color, local->muted_text_color, 28);
        tone_color = egui_rgb_mix(tone_color, local->muted_text_color, 28);
    }

    if (!egui_view_get_enable(self))
    {
        fill_color = egui_view_settings_expander_mix_disabled(fill_color);
        border_color = egui_view_settings_expander_mix_disabled(border_color);
        title_color = egui_view_settings_expander_mix_disabled(title_color);
        desc_color = egui_view_settings_expander_mix_disabled(desc_color);
        badge_fill = egui_view_settings_expander_mix_disabled(badge_fill);
        badge_border = egui_view_settings_expander_mix_disabled(badge_border);
        badge_color = egui_view_settings_expander_mix_disabled(badge_color);
        value_fill = egui_view_settings_expander_mix_disabled(value_fill);
        value_border = egui_view_settings_expander_mix_disabled(value_border);
        value_color = egui_view_settings_expander_mix_disabled(value_color);
        icon_fill = egui_view_settings_expander_mix_disabled(icon_fill);
        icon_color = egui_view_settings_expander_mix_disabled(icon_color);
        tone_color = egui_view_settings_expander_mix_disabled(tone_color);
    }

    egui_canvas_draw_round_rectangle_fill(region->location.x, region->location.y, region->size.width, region->size.height, radius, fill_color,
                                          egui_color_alpha_mix(self->alpha, 94));
    egui_canvas_draw_round_rectangle(region->location.x, region->location.y, region->size.width, region->size.height, radius, 1, border_color,
                                     egui_color_alpha_mix(self->alpha, 44));

    egui_canvas_draw_round_rectangle_fill(inner_x, region->location.y + (region->size.height - icon_size) / 2, icon_size, icon_size, icon_size / 2,
                                          icon_fill, egui_color_alpha_mix(self->alpha, 92));
    text_region.location.x = inner_x;
    text_region.location.y = region->location.y + (region->size.height - icon_size) / 2;
    text_region.size.width = icon_size;
    text_region.size.height = icon_size;
    egui_view_settings_expander_draw_text(local->meta_font, self, snapshot->icon_text, &text_region, EGUI_ALIGN_CENTER, icon_color);

    if (!local->compact_mode && egui_view_settings_expander_has_text(snapshot->eyebrow))
    {
        egui_dim_t badge_w = egui_view_settings_expander_pill_width(snapshot->eyebrow, local->compact_mode, 26, region->size.width / 3);

        egui_canvas_draw_round_rectangle_fill(title_x, inner_y, badge_w, badge_h, badge_h / 2, badge_fill, egui_color_alpha_mix(self->alpha, 90));
        egui_canvas_draw_round_rectangle(title_x, inner_y, badge_w, badge_h, badge_h / 2, 1, badge_border, egui_color_alpha_mix(self->alpha, 36));

        text_region.location.x = title_x;
        text_region.location.y = inner_y;
        text_region.size.width = badge_w;
        text_region.size.height = badge_h;
        egui_view_settings_expander_draw_text(local->meta_font, self, snapshot->eyebrow, &text_region, EGUI_ALIGN_CENTER, badge_color);
    }

    if (egui_view_settings_expander_has_text(snapshot->value))
    {
        egui_canvas_draw_round_rectangle_fill(value_x, region->location.y + (region->size.height - value_h) / 2, value_w, value_h, value_h / 2, value_fill,
                                              egui_color_alpha_mix(self->alpha, 88));
        egui_canvas_draw_round_rectangle(value_x, region->location.y + (region->size.height - value_h) / 2, value_w, value_h, value_h / 2, 1, value_border,
                                         egui_color_alpha_mix(self->alpha, 34));

        text_region.location.x = value_x;
        text_region.location.y = region->location.y + (region->size.height - value_h) / 2;
        text_region.size.width = value_w;
        text_region.size.height = value_h;
        egui_view_settings_expander_draw_text(local->meta_font, self, snapshot->value, &text_region, EGUI_ALIGN_CENTER, value_color);
    }

    egui_view_settings_expander_draw_chevron(self, chevron_x, chevron_y, chevron_w, chevron_h, local->expanded_state, tone_color);

    text_region.location.x = title_x;
    text_region.location.y = local->compact_mode ? (region->location.y + (region->size.height - 10) / 2) : (region->location.y + 16);
    text_region.size.width = text_right - title_x;
    text_region.size.height = local->compact_mode ? 10 : 11;
    egui_view_settings_expander_draw_text(local->font, self, snapshot->title, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);

    if (!local->compact_mode)
    {
        text_region.location.y += text_region.size.height;
        text_region.size.height = 9;
        egui_view_settings_expander_draw_text(local->meta_font, self, snapshot->description, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                                              desc_color);
    }
}

static void egui_view_settings_expander_draw_row(egui_view_t *self, egui_view_settings_expander_t *local,
                                                 const egui_view_settings_expander_row_t *row, const egui_region_t *region, uint8_t focused, uint8_t pressed)
{
    egui_region_t text_region;
    egui_color_t tone_color = egui_view_settings_expander_tone_color(local, row->tone);
    egui_color_t row_fill = egui_rgb_mix(local->section_color, tone_color, focused ? 10 : (row->emphasized ? 8 : 4));
    egui_color_t row_border = egui_rgb_mix(local->border_color, tone_color, focused ? 18 : 10);
    egui_color_t icon_fill = egui_rgb_mix(local->surface_color, tone_color, focused ? 12 : 8);
    egui_color_t icon_color = egui_rgb_mix(local->text_color, tone_color, focused ? 16 : 10);
    egui_color_t title_color = focused ? egui_rgb_mix(local->text_color, tone_color, 10) : local->text_color;
    egui_color_t value_fill = egui_rgb_mix(local->surface_color, tone_color, focused ? 12 : 8);
    egui_color_t value_border = egui_rgb_mix(local->border_color, tone_color, focused ? 18 : 12);
    egui_color_t value_color = egui_rgb_mix(local->muted_text_color, tone_color, focused ? 18 : 12);
    egui_dim_t icon_size = local->compact_mode ? EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_ICON_SIZE : EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_ICON_SIZE;
    egui_dim_t title_x = region->location.x + 5 + icon_size + 7;
    egui_dim_t trailing_w = 0;
    egui_dim_t trailing_h = local->compact_mode ? 10 : 12;
    egui_dim_t trailing_x;
    egui_dim_t trailing_y = region->location.y + (region->size.height - trailing_h) / 2;

    if (pressed)
    {
        row_fill = egui_rgb_mix(row_fill, tone_color, 10);
        row_border = egui_rgb_mix(row_border, tone_color, 18);
    }

    if (local->read_only_mode)
    {
        tone_color = egui_rgb_mix(tone_color, local->muted_text_color, 30);
        row_fill = egui_rgb_mix(row_fill, local->surface_color, 26);
        row_border = egui_rgb_mix(row_border, local->muted_text_color, 18);
        icon_fill = egui_rgb_mix(icon_fill, local->surface_color, 28);
        icon_color = egui_rgb_mix(icon_color, local->muted_text_color, 28);
        title_color = egui_rgb_mix(title_color, local->muted_text_color, 24);
        value_fill = egui_rgb_mix(value_fill, local->surface_color, 28);
        value_border = egui_rgb_mix(value_border, local->muted_text_color, 18);
        value_color = egui_rgb_mix(value_color, local->muted_text_color, 30);
    }

    if (!egui_view_get_enable(self))
    {
        tone_color = egui_view_settings_expander_mix_disabled(tone_color);
        row_fill = egui_view_settings_expander_mix_disabled(row_fill);
        row_border = egui_view_settings_expander_mix_disabled(row_border);
        icon_fill = egui_view_settings_expander_mix_disabled(icon_fill);
        icon_color = egui_view_settings_expander_mix_disabled(icon_color);
        title_color = egui_view_settings_expander_mix_disabled(title_color);
        value_fill = egui_view_settings_expander_mix_disabled(value_fill);
        value_border = egui_view_settings_expander_mix_disabled(value_border);
        value_color = egui_view_settings_expander_mix_disabled(value_color);
    }

    egui_canvas_draw_round_rectangle_fill(region->location.x, region->location.y, region->size.width, region->size.height, local->compact_mode ? 5 : 6,
                                          row_fill, egui_color_alpha_mix(self->alpha, 88));
    egui_canvas_draw_round_rectangle(region->location.x, region->location.y, region->size.width, region->size.height, local->compact_mode ? 5 : 6, 1,
                                     row_border, egui_color_alpha_mix(self->alpha, 32));

    egui_canvas_draw_round_rectangle_fill(region->location.x + 5, region->location.y + (region->size.height - icon_size) / 2, icon_size, icon_size,
                                          icon_size / 2, icon_fill, egui_color_alpha_mix(self->alpha, 90));

    text_region.location.x = region->location.x + 5;
    text_region.location.y = region->location.y + (region->size.height - icon_size) / 2;
    text_region.size.width = icon_size;
    text_region.size.height = icon_size;
    egui_view_settings_expander_draw_text(local->meta_font, self, row->icon_text, &text_region, EGUI_ALIGN_CENTER, icon_color);

    switch (row->trailing_kind)
    {
    case EGUI_VIEW_SETTINGS_EXPANDER_TRAILING_SWITCH_ON:
    case EGUI_VIEW_SETTINGS_EXPANDER_TRAILING_SWITCH_OFF:
        trailing_w = local->compact_mode ? EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_SWITCH_W : EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_SWITCH_W;
        trailing_h = local->compact_mode ? EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_SWITCH_H : EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_SWITCH_H;
        break;
    case EGUI_VIEW_SETTINGS_EXPANDER_TRAILING_CHEVRON:
        trailing_w = local->compact_mode ? 8 : 10;
        trailing_h = local->compact_mode ? 8 : 10;
        break;
    default:
        trailing_w = egui_view_settings_expander_pill_width(row->value, local->compact_mode, local->compact_mode ? 18 : 24, region->size.width / 3);
        break;
    }

    trailing_x = region->location.x + region->size.width - trailing_w - (local->compact_mode ? 6 : 7);
    trailing_y = region->location.y + (region->size.height - trailing_h) / 2;

    text_region.location.x = title_x;
    text_region.location.y = region->location.y;
    text_region.size.width = trailing_x - title_x - 4;
    text_region.size.height = region->size.height;
    egui_view_settings_expander_draw_text(local->font, self, row->title, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);

    switch (row->trailing_kind)
    {
    case EGUI_VIEW_SETTINGS_EXPANDER_TRAILING_SWITCH_ON:
        egui_view_settings_expander_draw_switch(self, trailing_x, trailing_y, trailing_w, trailing_h, 1, tone_color, local->muted_text_color);
        break;
    case EGUI_VIEW_SETTINGS_EXPANDER_TRAILING_SWITCH_OFF:
        egui_view_settings_expander_draw_switch(self, trailing_x, trailing_y, trailing_w, trailing_h, 0, tone_color, local->muted_text_color);
        break;
    case EGUI_VIEW_SETTINGS_EXPANDER_TRAILING_CHEVRON:
        egui_view_settings_expander_draw_chevron(self, trailing_x, trailing_y, trailing_w, trailing_h, 0, value_color);
        break;
    default:
        egui_canvas_draw_round_rectangle_fill(trailing_x, trailing_y, trailing_w, trailing_h, trailing_h / 2, value_fill, egui_color_alpha_mix(self->alpha, 86));
        egui_canvas_draw_round_rectangle(trailing_x, trailing_y, trailing_w, trailing_h, trailing_h / 2, 1, value_border,
                                         egui_color_alpha_mix(self->alpha, 30));
        text_region.location.x = trailing_x;
        text_region.location.y = trailing_y;
        text_region.size.width = trailing_w;
        text_region.size.height = trailing_h;
        egui_view_settings_expander_draw_text(local->meta_font, self, row->value, &text_region, EGUI_ALIGN_CENTER, value_color);
        break;
    }
}

static void egui_view_settings_expander_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);
    const egui_view_settings_expander_snapshot_t *snapshot = egui_view_settings_expander_get_snapshot(local);
    egui_view_settings_expander_metrics_t metrics;
    egui_color_t tone_color;
    egui_color_t card_fill;
    egui_color_t card_border;
    uint8_t i;

    if (snapshot == NULL)
    {
        return;
    }

    egui_view_settings_expander_sync_current_part(local);
    egui_view_settings_expander_get_metrics(local, self, snapshot, &metrics);
    if (metrics.region.size.width <= 0 || metrics.region.size.height <= 0)
    {
        return;
    }

    tone_color = egui_view_settings_expander_tone_color(local, snapshot->tone);
    card_fill = egui_rgb_mix(local->surface_color, tone_color, local->compact_mode ? 4 : 6);
    card_border = egui_rgb_mix(local->border_color, tone_color, local->compact_mode ? 10 : 14);

    if (local->read_only_mode)
    {
        card_fill = egui_rgb_mix(card_fill, local->surface_color, 18);
        card_border = egui_rgb_mix(card_border, local->muted_text_color, 18);
        tone_color = egui_rgb_mix(tone_color, local->muted_text_color, 28);
    }

    if (!egui_view_get_enable(self))
    {
        card_fill = egui_view_settings_expander_mix_disabled(card_fill);
        card_border = egui_view_settings_expander_mix_disabled(card_border);
        tone_color = egui_view_settings_expander_mix_disabled(tone_color);
    }

    egui_canvas_draw_round_rectangle_fill(metrics.region.location.x, metrics.region.location.y, metrics.region.size.width, metrics.region.size.height,
                                          local->compact_mode ? EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_RADIUS : EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_RADIUS,
                                          card_fill, egui_color_alpha_mix(self->alpha, 100));
    egui_canvas_draw_round_rectangle(metrics.region.location.x, metrics.region.location.y, metrics.region.size.width, metrics.region.size.height,
                                     local->compact_mode ? EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_RADIUS : EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_RADIUS, 1,
                                     card_border, egui_color_alpha_mix(self->alpha, 58));
    egui_canvas_draw_round_rectangle_fill(metrics.region.location.x + 2, metrics.region.location.y + 2, metrics.region.size.width - 4,
                                          local->compact_mode ? 3 : 4, local->compact_mode ? 6 : 8, tone_color,
                                          egui_color_alpha_mix(self->alpha, local->read_only_mode ? 10 : 18));

    egui_view_settings_expander_draw_header(self, local, snapshot, &metrics.header_region,
                                            local->current_part == EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER ? 1 : 0,
                                            local->pressed_part == EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER ? 1 : 0);

    if (local->expanded_state && metrics.body_region.size.height > 0)
    {
        egui_color_t body_fill = egui_rgb_mix(local->section_color, tone_color, 4);
        egui_color_t body_border = egui_rgb_mix(local->border_color, tone_color, 10);

        if (local->read_only_mode)
        {
            body_fill = egui_rgb_mix(body_fill, local->surface_color, 22);
            body_border = egui_rgb_mix(body_border, local->muted_text_color, 18);
        }
        if (!egui_view_get_enable(self))
        {
            body_fill = egui_view_settings_expander_mix_disabled(body_fill);
            body_border = egui_view_settings_expander_mix_disabled(body_border);
        }

        egui_canvas_draw_round_rectangle_fill(metrics.body_region.location.x, metrics.body_region.location.y, metrics.body_region.size.width,
                                              metrics.body_region.size.height,
                                              local->compact_mode ? EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_BODY_RADIUS
                                                                 : EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_BODY_RADIUS,
                                              body_fill, egui_color_alpha_mix(self->alpha, 92));
        egui_canvas_draw_round_rectangle(metrics.body_region.location.x, metrics.body_region.location.y, metrics.body_region.size.width,
                                         metrics.body_region.size.height,
                                         local->compact_mode ? EGUI_VIEW_SETTINGS_EXPANDER_COMPACT_BODY_RADIUS
                                                            : EGUI_VIEW_SETTINGS_EXPANDER_STANDARD_BODY_RADIUS,
                                         1, body_border, egui_color_alpha_mix(self->alpha, 30));

        for (i = 0; i < metrics.visible_row_count; ++i)
        {
            egui_view_settings_expander_draw_row(self, local, &snapshot->rows[i], &metrics.row_regions[i],
                                                 local->current_part == egui_view_settings_expander_row_part(i) ? 1 : 0,
                                                 local->pressed_part == egui_view_settings_expander_row_part(i) ? 1 : 0);
        }

        if (metrics.footer_region.size.height > 0)
        {
            egui_region_t text_region;
            egui_color_t footer_fill = egui_rgb_mix(local->surface_color, tone_color, 10);
            egui_color_t footer_border = egui_rgb_mix(local->border_color, tone_color, 16);
            egui_color_t footer_color = egui_rgb_mix(local->muted_text_color, tone_color, 16);

            if (local->read_only_mode)
            {
                footer_fill = egui_rgb_mix(footer_fill, local->surface_color, 24);
                footer_border = egui_rgb_mix(footer_border, local->muted_text_color, 18);
                footer_color = egui_rgb_mix(footer_color, local->muted_text_color, 28);
            }
            if (!egui_view_get_enable(self))
            {
                footer_fill = egui_view_settings_expander_mix_disabled(footer_fill);
                footer_border = egui_view_settings_expander_mix_disabled(footer_border);
                footer_color = egui_view_settings_expander_mix_disabled(footer_color);
            }

            egui_canvas_draw_round_rectangle_fill(metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                                  metrics.footer_region.size.height, metrics.footer_region.size.height / 2, footer_fill,
                                                  egui_color_alpha_mix(self->alpha, 90));
            egui_canvas_draw_round_rectangle(metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                             metrics.footer_region.size.height, metrics.footer_region.size.height / 2, 1, footer_border,
                                             egui_color_alpha_mix(self->alpha, 30));

            text_region.location.x = metrics.footer_region.location.x + 5;
            text_region.location.y = metrics.footer_region.location.y;
            text_region.size.width = metrics.footer_region.size.width - 10;
            text_region.size.height = metrics.footer_region.size.height;
            egui_view_settings_expander_draw_text(local->meta_font, self, snapshot->footer, &text_region,
                                                  EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, footer_color);
        }
    }
}

void egui_view_settings_expander_set_snapshots(egui_view_t *self, const egui_view_settings_expander_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);
    const egui_view_settings_expander_snapshot_t *snapshot;

    egui_view_settings_expander_clear_pressed_state(self, local);
    local->snapshots = snapshots;
    local->snapshot_count = snapshots == NULL ? 0 : egui_view_settings_expander_clamp_snapshot_count(snapshot_count);
    if (local->current_snapshot >= local->snapshot_count)
    {
        local->current_snapshot = 0;
    }

    snapshot = egui_view_settings_expander_get_snapshot(local);
    local->expanded_state = snapshot != NULL && snapshot->expanded ? 1 : 0;
    local->current_part = egui_view_settings_expander_resolve_default_part(local, snapshot);
    egui_view_invalidate(self);
}

void egui_view_settings_expander_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);
    const egui_view_settings_expander_snapshot_t *snapshot;
    uint8_t had_pressed = egui_view_settings_expander_clear_pressed_state(self, local);

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
    snapshot = egui_view_settings_expander_get_snapshot(local);
    local->expanded_state = snapshot != NULL && snapshot->expanded ? 1 : 0;
    local->current_part = egui_view_settings_expander_resolve_default_part(local, snapshot);
    egui_view_invalidate(self);
}

uint8_t egui_view_settings_expander_get_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);
    return local->current_snapshot;
}

void egui_view_settings_expander_set_expanded(egui_view_t *self, uint8_t expanded)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);
    const egui_view_settings_expander_snapshot_t *snapshot = egui_view_settings_expander_get_snapshot(local);
    uint8_t had_pressed = egui_view_settings_expander_clear_pressed_state(self, local);

    expanded = expanded ? 1 : 0;
    if (snapshot == NULL)
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    if (local->expanded_state == expanded)
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->expanded_state = expanded;
    local->current_part = egui_view_settings_expander_resolve_default_part(local, snapshot);
    egui_view_invalidate(self);
}

uint8_t egui_view_settings_expander_get_expanded(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);
    return local->expanded_state;
}

void egui_view_settings_expander_set_current_part(egui_view_t *self, uint8_t part)
{
    egui_view_settings_expander_set_current_part_inner(self, part, 1);
}

uint8_t egui_view_settings_expander_get_current_part(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);
    egui_view_settings_expander_sync_current_part(local);
    return local->current_part;
}

uint8_t egui_view_settings_expander_activate_current_part(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);
    const egui_view_settings_expander_snapshot_t *snapshot = egui_view_settings_expander_get_snapshot(local);
    uint8_t action_part;

    egui_view_settings_expander_sync_current_part(local);
    if (!egui_view_settings_expander_part_is_interactive(local, self, snapshot, local->current_part))
    {
        return 0;
    }

    if (local->current_part == EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER)
    {
        local->expanded_state = local->expanded_state ? 0 : 1;
        local->current_part = egui_view_settings_expander_resolve_default_part(local, snapshot);
        egui_view_invalidate(self);
        return 1;
    }

    action_part = local->current_part;
    if (local->on_action != NULL)
    {
        local->on_action(self, local->current_snapshot, action_part);
    }
    egui_view_invalidate(self);
    return 1;
}

void egui_view_settings_expander_set_on_action_listener(egui_view_t *self, egui_view_on_settings_expander_action_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);
    local->on_action = listener;
}

void egui_view_settings_expander_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);

    local->font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_settings_expander_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_settings_expander_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);

    local->meta_font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_settings_expander_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_settings_expander_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);

    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_settings_expander_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_settings_expander_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);

    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_settings_expander_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_settings_expander_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                                             egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                             egui_color_t success_color, egui_color_t warning_color, egui_color_t neutral_color)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);

    local->surface_color = surface_color;
    local->section_color = section_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->neutral_color = neutral_color;
    egui_view_settings_expander_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

uint8_t egui_view_settings_expander_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);
    const egui_view_settings_expander_snapshot_t *snapshot = egui_view_settings_expander_get_snapshot(local);
    egui_view_settings_expander_metrics_t metrics;
    uint8_t row_index;

    if (region == NULL)
    {
        return 0;
    }

    egui_view_settings_expander_get_metrics(local, self, snapshot, &metrics);
    if (part == EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER)
    {
        *region = metrics.header_region;
        return metrics.header_region.size.width > 0 && metrics.header_region.size.height > 0 ? 1 : 0;
    }

    row_index = egui_view_settings_expander_part_to_row_index(part);
    if (row_index < metrics.visible_row_count)
    {
        *region = metrics.row_regions[row_index];
        return 1;
    }

    return 0;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_settings_expander_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);
    const egui_view_settings_expander_snapshot_t *snapshot = egui_view_settings_expander_get_snapshot(local);
    uint8_t hit_part;

    if (snapshot == NULL || local->read_only_mode || !egui_view_get_enable(self))
    {
        if (egui_view_settings_expander_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_part = egui_view_settings_expander_resolve_hit(local, self, event->location.x, event->location.y);
        if (!egui_view_settings_expander_part_is_interactive(local, self, snapshot, hit_part))
        {
            if (egui_view_settings_expander_clear_pressed_state(self, local))
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
        local->current_part = hit_part;
        egui_view_set_pressed(self, true);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_part == EGUI_VIEW_SETTINGS_EXPANDER_PART_NONE)
        {
            return 0;
        }
        hit_part = egui_view_settings_expander_resolve_hit(local, self, event->location.x, event->location.y);
        egui_view_set_pressed(self, hit_part == local->pressed_part);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
    {
        uint8_t handled;

        hit_part = egui_view_settings_expander_resolve_hit(local, self, event->location.x, event->location.y);
        if (hit_part == local->pressed_part && egui_view_settings_expander_part_is_interactive(local, self, snapshot, hit_part))
        {
            local->current_part = hit_part;
            egui_view_settings_expander_activate_current_part(self);
        }
        handled = egui_view_settings_expander_clear_pressed_state(self, local);
        if (handled)
        {
            egui_view_invalidate(self);
        }
        return handled || hit_part != EGUI_VIEW_SETTINGS_EXPANDER_PART_NONE;
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (egui_view_settings_expander_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}

static int egui_view_settings_expander_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);
    EGUI_UNUSED(event);

    if (egui_view_settings_expander_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_settings_expander_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);
    const egui_view_settings_expander_snapshot_t *snapshot = egui_view_settings_expander_get_snapshot(local);
    uint8_t target_part;

    if (snapshot == NULL || local->read_only_mode || !egui_view_get_enable(self))
    {
        if (egui_view_settings_expander_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    if (event->key_code == EGUI_KEY_CODE_ENTER || event->key_code == EGUI_KEY_CODE_SPACE)
    {
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            if (!egui_view_settings_expander_part_is_interactive(local, self, snapshot, local->current_part))
            {
                return 0;
            }
            local->pressed_part = local->current_part;
            egui_view_set_pressed(self, true);
            egui_view_invalidate(self);
            return 1;
        }

        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            uint8_t handled = 0;

            if (local->pressed_part != EGUI_VIEW_SETTINGS_EXPANDER_PART_NONE && local->pressed_part == local->current_part &&
                egui_view_settings_expander_part_is_interactive(local, self, snapshot, local->pressed_part))
            {
                handled = egui_view_settings_expander_activate_current_part(self);
            }
            if (egui_view_settings_expander_clear_pressed_state(self, local))
            {
                egui_view_invalidate(self);
            }
            return handled;
        }

        return 0;
    }

    if (egui_view_settings_expander_clear_pressed_state(self, local))
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
        egui_view_settings_expander_move_linear(self, -1, 0);
        return 1;
    case EGUI_KEY_CODE_DOWN:
        egui_view_settings_expander_move_linear(self, 1, 0);
        return 1;
    case EGUI_KEY_CODE_TAB:
        egui_view_settings_expander_move_linear(self, 1, 1);
        return 1;
    case EGUI_KEY_CODE_HOME:
        egui_view_settings_expander_set_current_part_inner(self, EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER, 0);
        return 1;
    case EGUI_KEY_CODE_END:
        target_part = egui_view_settings_expander_find_last_row(snapshot, local->expanded_state);
        if (target_part == EGUI_VIEW_SETTINGS_EXPANDER_PART_NONE)
        {
            target_part = EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER;
        }
        egui_view_settings_expander_set_current_part_inner(self, target_part, 0);
        return 1;
    case EGUI_KEY_CODE_ESCAPE:
        if (local->expanded_state)
        {
            egui_view_settings_expander_set_expanded(self, 0);
            return 1;
        }
        return 0;
    default:
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_settings_expander_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_settings_expander_t);
    EGUI_UNUSED(event);

    if (egui_view_settings_expander_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_settings_expander_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_settings_expander_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_settings_expander_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_settings_expander_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_settings_expander_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_settings_expander_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_settings_expander_on_key_event,
#endif
};

void egui_view_settings_expander_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_settings_expander_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_settings_expander_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif

    local->snapshots = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->on_action = NULL;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->section_color = EGUI_COLOR_HEX(0xF7F9FC);
    local->border_color = EGUI_COLOR_HEX(0xD2DBE3);
    local->text_color = EGUI_COLOR_HEX(0x1A2734);
    local->muted_text_color = EGUI_COLOR_HEX(0x6B7A89);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->success_color = EGUI_COLOR_HEX(0x0F7B45);
    local->warning_color = EGUI_COLOR_HEX(0x9D5D00);
    local->neutral_color = EGUI_COLOR_HEX(0x7A8796);
    local->snapshot_count = 0;
    local->current_snapshot = 0;
    local->current_part = EGUI_VIEW_SETTINGS_EXPANDER_PART_NONE;
    local->expanded_state = 0;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_part = EGUI_VIEW_SETTINGS_EXPANDER_PART_NONE;

    egui_view_set_view_name(self, "egui_view_settings_expander");
}
