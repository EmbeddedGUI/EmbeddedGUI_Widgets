#include "egui_view_title_bar.h"

#include "resource/egui_icon_material_symbols.h"

#define EGUI_VIEW_TITLE_BAR_STANDARD_RADIUS             10
#define EGUI_VIEW_TITLE_BAR_STANDARD_FILL_ALPHA         92
#define EGUI_VIEW_TITLE_BAR_STANDARD_BORDER_ALPHA       54
#define EGUI_VIEW_TITLE_BAR_STANDARD_PAD_X              10
#define EGUI_VIEW_TITLE_BAR_STANDARD_PAD_Y              8
#define EGUI_VIEW_TITLE_BAR_STANDARD_META_HEIGHT        10
#define EGUI_VIEW_TITLE_BAR_STANDARD_META_GAP           4
#define EGUI_VIEW_TITLE_BAR_STANDARD_NAV_SIZE           16
#define EGUI_VIEW_TITLE_BAR_STANDARD_NAV_GAP            4
#define EGUI_VIEW_TITLE_BAR_STANDARD_ICON_SIZE          18
#define EGUI_VIEW_TITLE_BAR_STANDARD_ICON_GAP           6
#define EGUI_VIEW_TITLE_BAR_STANDARD_ACTION_HEIGHT      18
#define EGUI_VIEW_TITLE_BAR_STANDARD_ACTION_RADIUS      7
#define EGUI_VIEW_TITLE_BAR_STANDARD_ACTION_BASE_WIDTH  16
#define EGUI_VIEW_TITLE_BAR_STANDARD_ACTION_CHAR_WIDTH  4
#define EGUI_VIEW_TITLE_BAR_STANDARD_ACTION_MIN_WIDTH   24
#define EGUI_VIEW_TITLE_BAR_STANDARD_ACTION_MAX_WIDTH   46
#define EGUI_VIEW_TITLE_BAR_STANDARD_ACTION_GAP         4
#define EGUI_VIEW_TITLE_BAR_STANDARD_TITLE_HEIGHT       14
#define EGUI_VIEW_TITLE_BAR_STANDARD_SUBTITLE_HEIGHT    10
#define EGUI_VIEW_TITLE_BAR_STANDARD_TITLE_GAP          2

#define EGUI_VIEW_TITLE_BAR_COMPACT_RADIUS             8
#define EGUI_VIEW_TITLE_BAR_COMPACT_FILL_ALPHA         90
#define EGUI_VIEW_TITLE_BAR_COMPACT_BORDER_ALPHA       48
#define EGUI_VIEW_TITLE_BAR_COMPACT_PAD_X              8
#define EGUI_VIEW_TITLE_BAR_COMPACT_PAD_Y              6
#define EGUI_VIEW_TITLE_BAR_COMPACT_NAV_SIZE           14
#define EGUI_VIEW_TITLE_BAR_COMPACT_NAV_GAP            3
#define EGUI_VIEW_TITLE_BAR_COMPACT_ICON_SIZE          14
#define EGUI_VIEW_TITLE_BAR_COMPACT_ICON_GAP           4
#define EGUI_VIEW_TITLE_BAR_COMPACT_ACTION_HEIGHT      16
#define EGUI_VIEW_TITLE_BAR_COMPACT_ACTION_RADIUS      6
#define EGUI_VIEW_TITLE_BAR_COMPACT_ACTION_BASE_WIDTH  12
#define EGUI_VIEW_TITLE_BAR_COMPACT_ACTION_CHAR_WIDTH  4
#define EGUI_VIEW_TITLE_BAR_COMPACT_ACTION_MIN_WIDTH   22
#define EGUI_VIEW_TITLE_BAR_COMPACT_ACTION_MAX_WIDTH   38
#define EGUI_VIEW_TITLE_BAR_COMPACT_ACTION_GAP         3
#define EGUI_VIEW_TITLE_BAR_COMPACT_TITLE_HEIGHT       12

#define EGUI_VIEW_TITLE_BAR_META_BASE_WIDTH 12
#define EGUI_VIEW_TITLE_BAR_META_CHAR_WIDTH 4
#define EGUI_VIEW_TITLE_BAR_META_MIN_WIDTH  24
#define EGUI_VIEW_TITLE_BAR_META_MAX_WIDTH  52

typedef struct egui_view_title_bar_metrics egui_view_title_bar_metrics_t;
struct egui_view_title_bar_metrics
{
    egui_region_t region;
    egui_region_t leading_header_region;
    egui_region_t trailing_header_region;
    egui_region_t back_region;
    egui_region_t pane_region;
    egui_region_t icon_region;
    egui_region_t title_region;
    egui_region_t subtitle_region;
    egui_region_t primary_action_region;
    egui_region_t secondary_action_region;
    uint8_t show_leading_header;
    uint8_t show_trailing_header;
    uint8_t show_icon;
    uint8_t show_subtitle;
    uint8_t show_primary_action;
    uint8_t show_secondary_action;
};

static egui_view_title_bar_t *egui_view_title_bar_local(egui_view_t *self)
{
    return (egui_view_title_bar_t *)self;
}

static const egui_view_title_bar_snapshot_t *egui_view_title_bar_get_snapshot(egui_view_title_bar_t *local)
{
    if (local->snapshots == NULL || local->snapshot_count == 0 || local->current_snapshot >= local->snapshot_count)
    {
        return NULL;
    }

    return &local->snapshots[local->current_snapshot];
}

static uint8_t egui_view_title_bar_has_text(const char *text)
{
    return (text != NULL && text[0] != '\0') ? 1 : 0;
}

static uint8_t egui_view_title_bar_text_len(const char *text)
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

static uint8_t egui_view_title_bar_clamp_snapshot_count(uint8_t count)
{
    if (count > EGUI_VIEW_TITLE_BAR_MAX_SNAPSHOTS)
    {
        return EGUI_VIEW_TITLE_BAR_MAX_SNAPSHOTS;
    }
    return count;
}

static egui_color_t egui_view_title_bar_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 64);
}

static egui_dim_t egui_view_title_bar_measure_action_width(uint8_t compact_mode, const char *text)
{
    egui_dim_t width = (egui_dim_t)(egui_view_title_bar_text_len(text) *
                                    (compact_mode ? EGUI_VIEW_TITLE_BAR_COMPACT_ACTION_CHAR_WIDTH : EGUI_VIEW_TITLE_BAR_STANDARD_ACTION_CHAR_WIDTH));

    width += compact_mode ? EGUI_VIEW_TITLE_BAR_COMPACT_ACTION_BASE_WIDTH : EGUI_VIEW_TITLE_BAR_STANDARD_ACTION_BASE_WIDTH;
    if (compact_mode)
    {
        if (width < EGUI_VIEW_TITLE_BAR_COMPACT_ACTION_MIN_WIDTH)
        {
            width = EGUI_VIEW_TITLE_BAR_COMPACT_ACTION_MIN_WIDTH;
        }
        if (width > EGUI_VIEW_TITLE_BAR_COMPACT_ACTION_MAX_WIDTH)
        {
            width = EGUI_VIEW_TITLE_BAR_COMPACT_ACTION_MAX_WIDTH;
        }
    }
    else
    {
        if (width < EGUI_VIEW_TITLE_BAR_STANDARD_ACTION_MIN_WIDTH)
        {
            width = EGUI_VIEW_TITLE_BAR_STANDARD_ACTION_MIN_WIDTH;
        }
        if (width > EGUI_VIEW_TITLE_BAR_STANDARD_ACTION_MAX_WIDTH)
        {
            width = EGUI_VIEW_TITLE_BAR_STANDARD_ACTION_MAX_WIDTH;
        }
    }
    return width;
}

static egui_dim_t egui_view_title_bar_measure_meta_width(const char *text)
{
    egui_dim_t width = (egui_dim_t)(egui_view_title_bar_text_len(text) * EGUI_VIEW_TITLE_BAR_META_CHAR_WIDTH);

    width += EGUI_VIEW_TITLE_BAR_META_BASE_WIDTH;
    if (width < EGUI_VIEW_TITLE_BAR_META_MIN_WIDTH)
    {
        width = EGUI_VIEW_TITLE_BAR_META_MIN_WIDTH;
    }
    if (width > EGUI_VIEW_TITLE_BAR_META_MAX_WIDTH)
    {
        width = EGUI_VIEW_TITLE_BAR_META_MAX_WIDTH;
    }
    return width;
}

static uint8_t egui_view_title_bar_part_active(const egui_view_title_bar_snapshot_t *snapshot, uint8_t part)
{
    if (snapshot == NULL)
    {
        return 0;
    }

    switch (part)
    {
    case EGUI_VIEW_TITLE_BAR_PART_BACK:
        return snapshot->show_back_button ? 1 : 0;
    case EGUI_VIEW_TITLE_BAR_PART_PANE_TOGGLE:
        return snapshot->show_pane_toggle ? 1 : 0;
    case EGUI_VIEW_TITLE_BAR_PART_PRIMARY_ACTION:
        return egui_view_title_bar_has_text(snapshot->primary_action);
    case EGUI_VIEW_TITLE_BAR_PART_SECONDARY_ACTION:
        return egui_view_title_bar_has_text(snapshot->secondary_action);
    default:
        return 0;
    }
}

static uint8_t egui_view_title_bar_collect_active_parts(const egui_view_title_bar_snapshot_t *snapshot, uint8_t *parts)
{
    uint8_t count = 0;

    if (snapshot == NULL)
    {
        return 0;
    }

    if (snapshot->show_back_button)
    {
        parts[count++] = EGUI_VIEW_TITLE_BAR_PART_BACK;
    }
    if (snapshot->show_pane_toggle)
    {
        parts[count++] = EGUI_VIEW_TITLE_BAR_PART_PANE_TOGGLE;
    }
    if (egui_view_title_bar_has_text(snapshot->primary_action))
    {
        parts[count++] = EGUI_VIEW_TITLE_BAR_PART_PRIMARY_ACTION;
    }
    if (egui_view_title_bar_has_text(snapshot->secondary_action))
    {
        parts[count++] = EGUI_VIEW_TITLE_BAR_PART_SECONDARY_ACTION;
    }

    return count;
}

static uint8_t egui_view_title_bar_resolve_first_part(const egui_view_title_bar_snapshot_t *snapshot)
{
    uint8_t parts[4];
    uint8_t count = egui_view_title_bar_collect_active_parts(snapshot, parts);

    return count > 0 ? parts[0] : EGUI_VIEW_TITLE_BAR_PART_NONE;
}

static uint8_t egui_view_title_bar_resolve_next_part(const egui_view_title_bar_snapshot_t *snapshot, uint8_t current_part, uint8_t forward)
{
    uint8_t parts[4];
    uint8_t count = egui_view_title_bar_collect_active_parts(snapshot, parts);
    uint8_t i;

    if (count == 0)
    {
        return EGUI_VIEW_TITLE_BAR_PART_NONE;
    }

    for (i = 0; i < count; ++i)
    {
        if (parts[i] != current_part)
        {
            continue;
        }

        if (forward)
        {
            return parts[(uint8_t)((i + 1) % count)];
        }
        return parts[(uint8_t)((i + count - 1) % count)];
    }

    return parts[0];
}

static const egui_font_t *egui_view_title_bar_resolve_icon_font(egui_view_title_bar_t *local)
{
    if (local->icon_font != NULL)
    {
        return local->icon_font;
    }

    return local->compact_mode ? EGUI_FONT_ICON_MS_16 : EGUI_FONT_ICON_MS_20;
}

static uint8_t egui_view_title_bar_clear_pressed_state(egui_view_t *self)
{
    egui_view_title_bar_t *local = egui_view_title_bar_local(self);
    uint8_t had_pressed = (uint8_t)(self->is_pressed || local->pressed_part != EGUI_VIEW_TITLE_BAR_PART_NONE);

    if (!had_pressed)
    {
        return 0;
    }

    local->pressed_part = EGUI_VIEW_TITLE_BAR_PART_NONE;
    if (self->is_pressed)
    {
        egui_view_set_pressed(self, 0);
    }
    else
    {
        egui_view_invalidate(self);
    }
    return 1;
}

static void egui_view_title_bar_sync_current_part(egui_view_t *self)
{
    egui_view_title_bar_t *local = egui_view_title_bar_local(self);
    const egui_view_title_bar_snapshot_t *snapshot = egui_view_title_bar_get_snapshot(local);

    if (!egui_view_title_bar_part_active(snapshot, local->current_part))
    {
        local->current_part = egui_view_title_bar_resolve_first_part(snapshot);
    }
}

static void egui_view_title_bar_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                          egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (!egui_view_title_bar_has_text(text) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, self->alpha);
}

static void egui_view_title_bar_get_metrics(egui_view_title_bar_t *local, egui_view_t *self, const egui_view_title_bar_snapshot_t *snapshot,
                                            egui_view_title_bar_metrics_t *metrics)
{
    egui_dim_t pad_x = local->compact_mode ? EGUI_VIEW_TITLE_BAR_COMPACT_PAD_X : EGUI_VIEW_TITLE_BAR_STANDARD_PAD_X;
    egui_dim_t pad_y = local->compact_mode ? EGUI_VIEW_TITLE_BAR_COMPACT_PAD_Y : EGUI_VIEW_TITLE_BAR_STANDARD_PAD_Y;
    egui_dim_t meta_h = local->compact_mode ? 0 : EGUI_VIEW_TITLE_BAR_STANDARD_META_HEIGHT;
    egui_dim_t meta_gap = local->compact_mode ? 0 : EGUI_VIEW_TITLE_BAR_STANDARD_META_GAP;
    egui_dim_t nav_size = local->compact_mode ? EGUI_VIEW_TITLE_BAR_COMPACT_NAV_SIZE : EGUI_VIEW_TITLE_BAR_STANDARD_NAV_SIZE;
    egui_dim_t nav_gap = local->compact_mode ? EGUI_VIEW_TITLE_BAR_COMPACT_NAV_GAP : EGUI_VIEW_TITLE_BAR_STANDARD_NAV_GAP;
    egui_dim_t icon_size = local->compact_mode ? EGUI_VIEW_TITLE_BAR_COMPACT_ICON_SIZE : EGUI_VIEW_TITLE_BAR_STANDARD_ICON_SIZE;
    egui_dim_t icon_gap = local->compact_mode ? EGUI_VIEW_TITLE_BAR_COMPACT_ICON_GAP : EGUI_VIEW_TITLE_BAR_STANDARD_ICON_GAP;
    egui_dim_t action_h = local->compact_mode ? EGUI_VIEW_TITLE_BAR_COMPACT_ACTION_HEIGHT : EGUI_VIEW_TITLE_BAR_STANDARD_ACTION_HEIGHT;
    egui_dim_t action_gap = local->compact_mode ? EGUI_VIEW_TITLE_BAR_COMPACT_ACTION_GAP : EGUI_VIEW_TITLE_BAR_STANDARD_ACTION_GAP;
    egui_dim_t title_h = local->compact_mode ? EGUI_VIEW_TITLE_BAR_COMPACT_TITLE_HEIGHT : EGUI_VIEW_TITLE_BAR_STANDARD_TITLE_HEIGHT;
    egui_dim_t subtitle_h = EGUI_VIEW_TITLE_BAR_STANDARD_SUBTITLE_HEIGHT;
    egui_dim_t title_gap = EGUI_VIEW_TITLE_BAR_STANDARD_TITLE_GAP;
    egui_dim_t control_x;
    egui_dim_t body_y;
    egui_dim_t body_h;
    egui_dim_t button_y;
    egui_dim_t text_x;
    egui_dim_t text_right;
    egui_dim_t stack_y;
    egui_dim_t stack_h;

    *metrics = (egui_view_title_bar_metrics_t){0};
    egui_view_get_work_region(self, &metrics->region);
    if (metrics->region.size.width <= 0 || metrics->region.size.height <= 0 || snapshot == NULL)
    {
        return;
    }

    metrics->show_leading_header = (uint8_t)(!local->compact_mode && egui_view_title_bar_has_text(snapshot->leading_header));
    metrics->show_trailing_header = (uint8_t)(!local->compact_mode && egui_view_title_bar_has_text(snapshot->trailing_header));
    metrics->show_icon = egui_view_title_bar_has_text(snapshot->leading_icon);
    metrics->show_subtitle = (uint8_t)(!local->compact_mode && egui_view_title_bar_has_text(snapshot->subtitle));
    metrics->show_primary_action = egui_view_title_bar_has_text(snapshot->primary_action);
    metrics->show_secondary_action = egui_view_title_bar_has_text(snapshot->secondary_action);

    body_y = metrics->region.location.y + pad_y + meta_h + meta_gap;
    body_h = metrics->region.size.height - pad_y * 2 - meta_h - meta_gap;
    if (body_h < (local->compact_mode ? 18 : 28))
    {
        body_h = local->compact_mode ? 18 : 28;
    }

    control_x = metrics->region.location.x + pad_x;
    button_y = body_y + (body_h - nav_size) / 2;

    if (snapshot->show_back_button)
    {
        metrics->back_region.location.x = control_x;
        metrics->back_region.location.y = button_y;
        metrics->back_region.size.width = nav_size;
        metrics->back_region.size.height = nav_size;
        control_x += nav_size + nav_gap;
    }

    if (snapshot->show_pane_toggle)
    {
        metrics->pane_region.location.x = control_x;
        metrics->pane_region.location.y = button_y;
        metrics->pane_region.size.width = nav_size;
        metrics->pane_region.size.height = nav_size;
        control_x += nav_size + nav_gap;
    }

    if (metrics->show_icon)
    {
        metrics->icon_region.location.x = control_x;
        metrics->icon_region.location.y = body_y + (body_h - icon_size) / 2;
        metrics->icon_region.size.width = icon_size;
        metrics->icon_region.size.height = icon_size;
        control_x += icon_size + icon_gap;
    }

    if (metrics->show_secondary_action)
    {
        egui_dim_t action_w = egui_view_title_bar_measure_action_width(local->compact_mode, snapshot->secondary_action);

        metrics->secondary_action_region.size.width = action_w;
        metrics->secondary_action_region.size.height = action_h;
    }

    if (metrics->show_primary_action)
    {
        egui_dim_t action_w = egui_view_title_bar_measure_action_width(local->compact_mode, snapshot->primary_action);

        metrics->primary_action_region.size.width = action_w;
        metrics->primary_action_region.size.height = action_h;
    }

    text_right = metrics->region.location.x + metrics->region.size.width - pad_x;
    if (metrics->show_secondary_action)
    {
        text_right -= metrics->secondary_action_region.size.width;
        metrics->secondary_action_region.location.x = text_right;
        metrics->secondary_action_region.location.y = body_y + (body_h - action_h) / 2;
        text_right -= action_gap;
    }
    if (metrics->show_primary_action)
    {
        text_right -= metrics->primary_action_region.size.width;
        metrics->primary_action_region.location.x = text_right;
        metrics->primary_action_region.location.y = body_y + (body_h - action_h) / 2;
        text_right -= action_gap;
    }

    text_x = control_x;
    if (text_right < text_x)
    {
        text_right = text_x;
    }

    stack_h = title_h + (metrics->show_subtitle ? (title_gap + subtitle_h) : 0);
    stack_y = body_y + (body_h - stack_h) / 2;

    metrics->title_region.location.x = text_x;
    metrics->title_region.location.y = stack_y;
    metrics->title_region.size.width = text_right - text_x;
    metrics->title_region.size.height = title_h;

    if (metrics->show_subtitle)
    {
        metrics->subtitle_region.location.x = text_x;
        metrics->subtitle_region.location.y = stack_y + title_h + title_gap;
        metrics->subtitle_region.size.width = text_right - text_x;
        metrics->subtitle_region.size.height = subtitle_h;
    }

    if (metrics->show_trailing_header)
    {
        egui_dim_t header_w = egui_view_title_bar_measure_meta_width(snapshot->trailing_header);

        metrics->trailing_header_region.location.x = metrics->region.location.x + metrics->region.size.width - pad_x - header_w;
        metrics->trailing_header_region.location.y = metrics->region.location.y + pad_y;
        metrics->trailing_header_region.size.width = header_w;
        metrics->trailing_header_region.size.height = meta_h;
    }

    if (metrics->show_leading_header)
    {
        egui_dim_t leading_x = text_x;
        egui_dim_t leading_right = metrics->region.location.x + metrics->region.size.width - pad_x;

        if (metrics->show_trailing_header && leading_right > metrics->trailing_header_region.location.x)
        {
            leading_right = metrics->trailing_header_region.location.x - 6;
        }
        if (leading_right < leading_x)
        {
            leading_right = leading_x;
        }

        metrics->leading_header_region.location.x = leading_x;
        metrics->leading_header_region.location.y = metrics->region.location.y + pad_y;
        metrics->leading_header_region.size.width = leading_right - leading_x;
        metrics->leading_header_region.size.height = meta_h;
    }
}

static void egui_view_title_bar_draw_focus_ring(egui_view_t *self, const egui_region_t *region, egui_dim_t radius)
{
    egui_canvas_draw_round_rectangle(region->location.x - 2, region->location.y - 2, region->size.width + 4, region->size.height + 4, radius + 2, 1,
                                     EGUI_THEME_FOCUS, egui_color_alpha_mix(self->alpha, 94));
}

static void egui_view_title_bar_draw_button(egui_view_t *self, egui_view_title_bar_t *local, const egui_region_t *region, uint8_t part, const char *glyph,
                                            egui_color_t fill_color, egui_color_t border_color, egui_color_t text_color, egui_color_t accent_color)
{
    uint8_t is_current = (uint8_t)(part == local->current_part);
    uint8_t is_pressed = (uint8_t)(self->is_pressed && local->pressed_part == part);
    egui_color_t resolved_fill = egui_rgb_mix(fill_color, accent_color, is_current ? 10 : 4);
    egui_color_t resolved_border = egui_rgb_mix(border_color, accent_color, is_current ? 14 : 4);
    egui_color_t glyph_color = is_current ? egui_rgb_mix(text_color, accent_color, 12) : text_color;
    egui_dim_t radius = local->compact_mode ? 5 : 6;

    if (region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    if (is_current && self->is_focused && egui_view_get_enable(self) && !local->read_only_mode)
    {
        egui_view_title_bar_draw_focus_ring(self, region, radius);
    }

    egui_canvas_draw_round_rectangle_fill(region->location.x, region->location.y, region->size.width, region->size.height, radius, resolved_fill,
                                          egui_color_alpha_mix(self->alpha, is_pressed ? 62 : 42));
    egui_canvas_draw_round_rectangle(region->location.x, region->location.y, region->size.width, region->size.height, radius, 1, resolved_border,
                                     egui_color_alpha_mix(self->alpha, is_pressed ? 56 : 34));
    if (is_pressed)
    {
        egui_canvas_draw_round_rectangle_fill(region->location.x + 1, region->location.y + 1, region->size.width > 2 ? region->size.width - 2 : region->size.width,
                                              region->size.height > 2 ? region->size.height - 2 : region->size.height, radius, EGUI_THEME_PRESS_OVERLAY,
                                              EGUI_THEME_PRESS_OVERLAY_ALPHA);
    }
    egui_view_title_bar_draw_text(egui_view_title_bar_resolve_icon_font(local), self, glyph, region, EGUI_ALIGN_CENTER, glyph_color);
}

static void egui_view_title_bar_draw_action(egui_view_t *self, egui_view_title_bar_t *local, const egui_region_t *region, uint8_t part, const char *text,
                                            uint8_t emphasize, egui_color_t fill_color, egui_color_t border_color, egui_color_t text_color,
                                            egui_color_t accent_color)
{
    uint8_t is_current = (uint8_t)(part == local->current_part);
    uint8_t is_pressed = (uint8_t)(self->is_pressed && local->pressed_part == part);
    egui_color_t resolved_fill = emphasize ? egui_rgb_mix(fill_color, accent_color, 10) : egui_rgb_mix(fill_color, accent_color, is_current ? 8 : 2);
    egui_color_t resolved_border = emphasize ? egui_rgb_mix(border_color, accent_color, 18) : egui_rgb_mix(border_color, accent_color, is_current ? 14 : 6);
    egui_color_t resolved_text = emphasize ? egui_rgb_mix(text_color, accent_color, 22) : (is_current ? egui_rgb_mix(text_color, accent_color, 10) : text_color);
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_TITLE_BAR_COMPACT_ACTION_RADIUS : EGUI_VIEW_TITLE_BAR_STANDARD_ACTION_RADIUS;

    if (!egui_view_title_bar_has_text(text) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    if (is_current && self->is_focused && egui_view_get_enable(self) && !local->read_only_mode)
    {
        egui_view_title_bar_draw_focus_ring(self, region, radius);
    }

    egui_canvas_draw_round_rectangle_fill(region->location.x, region->location.y, region->size.width, region->size.height, radius, resolved_fill,
                                          egui_color_alpha_mix(self->alpha, emphasize ? 48 : (is_pressed ? 56 : 34)));
    egui_canvas_draw_round_rectangle(region->location.x, region->location.y, region->size.width, region->size.height, radius, 1, resolved_border,
                                     egui_color_alpha_mix(self->alpha, is_pressed ? 52 : 34));
    if (is_pressed)
    {
        egui_canvas_draw_round_rectangle_fill(region->location.x + 1, region->location.y + 1, region->size.width > 2 ? region->size.width - 2 : region->size.width,
                                              region->size.height > 2 ? region->size.height - 2 : region->size.height, radius, EGUI_THEME_PRESS_OVERLAY,
                                              EGUI_THEME_PRESS_OVERLAY_ALPHA);
    }
    egui_view_title_bar_draw_text(local->meta_font, self, text, region, EGUI_ALIGN_CENTER, resolved_text);
}

static void egui_view_title_bar_draw_icon_badge(egui_view_t *self, egui_view_title_bar_t *local, const egui_region_t *region, const char *glyph,
                                                egui_color_t fill_color, egui_color_t border_color, egui_color_t accent_color)
{
    egui_color_t resolved_fill = egui_rgb_mix(fill_color, accent_color, local->compact_mode ? 8 : 10);
    egui_color_t resolved_border = egui_rgb_mix(border_color, accent_color, local->compact_mode ? 12 : 16);
    egui_color_t glyph_color = egui_rgb_mix(accent_color, EGUI_COLOR_HEX(0x1A2734), 8);
    egui_dim_t radius = local->compact_mode ? 6 : 8;

    if (!egui_view_title_bar_has_text(glyph) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    egui_canvas_draw_round_rectangle_fill(region->location.x, region->location.y, region->size.width, region->size.height, radius, resolved_fill,
                                          egui_color_alpha_mix(self->alpha, 52));
    egui_canvas_draw_round_rectangle(region->location.x, region->location.y, region->size.width, region->size.height, radius, 1, resolved_border,
                                     egui_color_alpha_mix(self->alpha, 38));
    egui_view_title_bar_draw_text(egui_view_title_bar_resolve_icon_font(local), self, glyph, region, EGUI_ALIGN_CENTER, glyph_color);
}

static void egui_view_title_bar_on_draw(egui_view_t *self)
{
    egui_view_title_bar_t *local = egui_view_title_bar_local(self);
    const egui_view_title_bar_snapshot_t *snapshot = egui_view_title_bar_get_snapshot(local);
    egui_view_title_bar_metrics_t metrics;
    egui_color_t surface_color = local->surface_color;
    egui_color_t border_color = local->border_color;
    egui_color_t text_color = local->text_color;
    egui_color_t muted_text_color = local->muted_text_color;
    egui_color_t accent_color = local->accent_color;
    egui_color_t subtle_fill_color = local->subtle_fill_color;
    egui_color_t subtle_border_color = local->subtle_border_color;
    egui_color_t shadow_color = local->shadow_color;
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_TITLE_BAR_COMPACT_RADIUS : EGUI_VIEW_TITLE_BAR_STANDARD_RADIUS;

    if (snapshot == NULL)
    {
        return;
    }

    egui_view_title_bar_get_metrics(local, self, snapshot, &metrics);
    if (metrics.region.size.width <= 0 || metrics.region.size.height <= 0)
    {
        return;
    }

    if (local->read_only_mode)
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xFBFCFD), 18);
        border_color = egui_rgb_mix(border_color, muted_text_color, 12);
        text_color = egui_rgb_mix(text_color, muted_text_color, 34);
        muted_text_color = egui_rgb_mix(muted_text_color, border_color, 10);
        accent_color = egui_rgb_mix(accent_color, muted_text_color, 76);
        subtle_fill_color = egui_rgb_mix(subtle_fill_color, surface_color, 24);
        subtle_border_color = egui_rgb_mix(subtle_border_color, muted_text_color, 18);
        shadow_color = egui_rgb_mix(shadow_color, surface_color, 42);
    }

    if (!egui_view_get_enable(self))
    {
        surface_color = egui_view_title_bar_mix_disabled(surface_color);
        border_color = egui_view_title_bar_mix_disabled(border_color);
        text_color = egui_view_title_bar_mix_disabled(text_color);
        muted_text_color = egui_view_title_bar_mix_disabled(muted_text_color);
        accent_color = egui_view_title_bar_mix_disabled(accent_color);
        subtle_fill_color = egui_view_title_bar_mix_disabled(subtle_fill_color);
        subtle_border_color = egui_view_title_bar_mix_disabled(subtle_border_color);
        shadow_color = egui_view_title_bar_mix_disabled(shadow_color);
    }

    egui_canvas_draw_round_rectangle_fill(metrics.region.location.x + 1, metrics.region.location.y + 2, metrics.region.size.width, metrics.region.size.height,
                                          radius, shadow_color, egui_color_alpha_mix(self->alpha, local->compact_mode ? 10 : 18));
    egui_canvas_draw_round_rectangle_fill(metrics.region.location.x, metrics.region.location.y, metrics.region.size.width, metrics.region.size.height, radius,
                                          surface_color, egui_color_alpha_mix(self->alpha,
                                                                              local->compact_mode ? EGUI_VIEW_TITLE_BAR_COMPACT_FILL_ALPHA
                                                                                                 : EGUI_VIEW_TITLE_BAR_STANDARD_FILL_ALPHA));
    egui_canvas_draw_round_rectangle(metrics.region.location.x, metrics.region.location.y, metrics.region.size.width, metrics.region.size.height, radius, 1,
                                     border_color, egui_color_alpha_mix(self->alpha,
                                                                        local->compact_mode ? EGUI_VIEW_TITLE_BAR_COMPACT_BORDER_ALPHA
                                                                                           : EGUI_VIEW_TITLE_BAR_STANDARD_BORDER_ALPHA));

    if (metrics.show_leading_header)
    {
        egui_view_title_bar_draw_text(local->meta_font, self, snapshot->leading_header, &metrics.leading_header_region,
                                      EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, muted_text_color);
    }

    if (metrics.show_trailing_header)
    {
        egui_color_t tag_fill = egui_rgb_mix(subtle_fill_color, accent_color, 8);
        egui_color_t tag_border = egui_rgb_mix(subtle_border_color, accent_color, 12);

        egui_canvas_draw_round_rectangle_fill(metrics.trailing_header_region.location.x, metrics.trailing_header_region.location.y,
                                              metrics.trailing_header_region.size.width, metrics.trailing_header_region.size.height, 5, tag_fill,
                                              egui_color_alpha_mix(self->alpha, 44));
        egui_canvas_draw_round_rectangle(metrics.trailing_header_region.location.x, metrics.trailing_header_region.location.y,
                                         metrics.trailing_header_region.size.width, metrics.trailing_header_region.size.height, 5, 1, tag_border,
                                         egui_color_alpha_mix(self->alpha, 30));
        egui_view_title_bar_draw_text(local->meta_font, self, snapshot->trailing_header, &metrics.trailing_header_region, EGUI_ALIGN_CENTER,
                                      egui_rgb_mix(muted_text_color, accent_color, 16));
    }

    if (snapshot->show_back_button)
    {
        egui_view_title_bar_draw_button(self, local, &metrics.back_region, EGUI_VIEW_TITLE_BAR_PART_BACK, EGUI_ICON_MS_ARROW_BACK, subtle_fill_color,
                                        subtle_border_color, text_color, accent_color);
    }

    if (snapshot->show_pane_toggle)
    {
        egui_view_title_bar_draw_button(self, local, &metrics.pane_region, EGUI_VIEW_TITLE_BAR_PART_PANE_TOGGLE, EGUI_ICON_MS_MENU, subtle_fill_color,
                                        subtle_border_color, text_color, accent_color);
    }

    if (metrics.show_icon)
    {
        egui_view_title_bar_draw_icon_badge(self, local, &metrics.icon_region, snapshot->leading_icon, subtle_fill_color, subtle_border_color, accent_color);
    }

    egui_view_title_bar_draw_text(local->font, self, snapshot->title, &metrics.title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text_color);
    if (metrics.show_subtitle)
    {
        egui_view_title_bar_draw_text(local->meta_font, self, snapshot->subtitle, &metrics.subtitle_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                                      muted_text_color);
    }

    if (metrics.show_primary_action)
    {
        egui_view_title_bar_draw_action(self, local, &metrics.primary_action_region, EGUI_VIEW_TITLE_BAR_PART_PRIMARY_ACTION, snapshot->primary_action,
                                        snapshot->emphasize_primary_action, subtle_fill_color, subtle_border_color, text_color, accent_color);
    }

    if (metrics.show_secondary_action)
    {
        egui_view_title_bar_draw_action(self, local, &metrics.secondary_action_region, EGUI_VIEW_TITLE_BAR_PART_SECONDARY_ACTION, snapshot->secondary_action, 0,
                                        subtle_fill_color, subtle_border_color, text_color, accent_color);
    }
}

static uint8_t egui_view_title_bar_resolve_hit(egui_view_title_bar_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    const egui_view_title_bar_snapshot_t *snapshot = egui_view_title_bar_get_snapshot(local);
    egui_view_title_bar_metrics_t metrics;

    if (snapshot == NULL)
    {
        return EGUI_VIEW_TITLE_BAR_PART_NONE;
    }

    egui_view_title_bar_get_metrics(local, self, snapshot, &metrics);
    if (!egui_region_pt_in_rect(&metrics.region, x, y))
    {
        return EGUI_VIEW_TITLE_BAR_PART_NONE;
    }
    if (snapshot->show_back_button && egui_region_pt_in_rect(&metrics.back_region, x, y))
    {
        return EGUI_VIEW_TITLE_BAR_PART_BACK;
    }
    if (snapshot->show_pane_toggle && egui_region_pt_in_rect(&metrics.pane_region, x, y))
    {
        return EGUI_VIEW_TITLE_BAR_PART_PANE_TOGGLE;
    }
    if (metrics.show_primary_action && egui_region_pt_in_rect(&metrics.primary_action_region, x, y))
    {
        return EGUI_VIEW_TITLE_BAR_PART_PRIMARY_ACTION;
    }
    if (metrics.show_secondary_action && egui_region_pt_in_rect(&metrics.secondary_action_region, x, y))
    {
        return EGUI_VIEW_TITLE_BAR_PART_SECONDARY_ACTION;
    }
    return EGUI_VIEW_TITLE_BAR_PART_NONE;
}

static void egui_view_title_bar_notify_action(egui_view_t *self, uint8_t part)
{
    egui_view_title_bar_t *local = egui_view_title_bar_local(self);
    const egui_view_title_bar_snapshot_t *snapshot = egui_view_title_bar_get_snapshot(local);

    if (local->on_action == NULL || local->read_only_mode || !egui_view_get_enable(self) || !egui_view_title_bar_part_active(snapshot, part))
    {
        return;
    }

    local->on_action(self, part, local->current_snapshot);
}

void egui_view_title_bar_set_snapshots(egui_view_t *self, const egui_view_title_bar_snapshot_t *snapshots, uint8_t snapshot_count)
{
    egui_view_title_bar_t *local = egui_view_title_bar_local(self);
    uint8_t had_pressed = egui_view_title_bar_clear_pressed_state(self);

    local->snapshots = snapshots;
    local->snapshot_count = snapshots == NULL ? 0 : egui_view_title_bar_clamp_snapshot_count(snapshot_count);
    if (local->snapshot_count == 0)
    {
        local->current_snapshot = 0;
        local->current_part = EGUI_VIEW_TITLE_BAR_PART_NONE;
    }
    else
    {
        if (local->current_snapshot >= local->snapshot_count)
        {
            local->current_snapshot = 0;
        }
        egui_view_title_bar_sync_current_part(self);
    }

    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_title_bar_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    egui_view_title_bar_t *local = egui_view_title_bar_local(self);
    uint8_t had_pressed = egui_view_title_bar_clear_pressed_state(self);

    if (local->snapshot_count == 0)
    {
        local->current_snapshot = 0;
        local->current_part = EGUI_VIEW_TITLE_BAR_PART_NONE;
    }
    else if (snapshot_index < local->snapshot_count)
    {
        local->current_snapshot = snapshot_index;
        egui_view_title_bar_sync_current_part(self);
    }

    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

uint8_t egui_view_title_bar_get_current_snapshot(egui_view_t *self)
{
    return egui_view_title_bar_local(self)->current_snapshot;
}

void egui_view_title_bar_set_current_part(egui_view_t *self, uint8_t part)
{
    egui_view_title_bar_t *local = egui_view_title_bar_local(self);
    const egui_view_title_bar_snapshot_t *snapshot = egui_view_title_bar_get_snapshot(local);
    uint8_t had_pressed = egui_view_title_bar_clear_pressed_state(self);

    if (!egui_view_title_bar_part_active(snapshot, part))
    {
        part = egui_view_title_bar_resolve_first_part(snapshot);
    }

    if (local->current_part == part)
    {
        if (!had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_part = part;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

uint8_t egui_view_title_bar_get_current_part(egui_view_t *self)
{
    return egui_view_title_bar_local(self)->current_part;
}

void egui_view_title_bar_activate_current_part(egui_view_t *self)
{
    egui_view_title_bar_t *local = egui_view_title_bar_local(self);
    const egui_view_title_bar_snapshot_t *snapshot = egui_view_title_bar_get_snapshot(local);

    if (!egui_view_title_bar_part_active(snapshot, local->current_part))
    {
        return;
    }

    egui_view_title_bar_notify_action(self, local->current_part);
}

void egui_view_title_bar_set_font(egui_view_t *self, const egui_font_t *font)
{
    egui_view_title_bar_t *local = egui_view_title_bar_local(self);
    uint8_t had_pressed = egui_view_title_bar_clear_pressed_state(self);

    local->font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_title_bar_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    egui_view_title_bar_t *local = egui_view_title_bar_local(self);
    uint8_t had_pressed = egui_view_title_bar_clear_pressed_state(self);

    local->meta_font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_title_bar_set_icon_font(egui_view_t *self, const egui_font_t *font)
{
    egui_view_title_bar_t *local = egui_view_title_bar_local(self);
    uint8_t had_pressed = egui_view_title_bar_clear_pressed_state(self);

    local->icon_font = font;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_title_bar_set_on_action_listener(egui_view_t *self, egui_view_on_title_bar_action_listener_t listener)
{
    egui_view_title_bar_local(self)->on_action = listener;
}

void egui_view_title_bar_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    egui_view_title_bar_t *local = egui_view_title_bar_local(self);
    uint8_t had_pressed = egui_view_title_bar_clear_pressed_state(self);

    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_title_bar_sync_current_part(self);
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_title_bar_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    egui_view_title_bar_t *local = egui_view_title_bar_local(self);
    uint8_t had_pressed = egui_view_title_bar_clear_pressed_state(self);

    local->read_only_mode = read_only_mode ? 1 : 0;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_title_bar_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                     egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t subtle_fill_color,
                                     egui_color_t subtle_border_color, egui_color_t shadow_color)
{
    egui_view_title_bar_t *local = egui_view_title_bar_local(self);
    uint8_t had_pressed = egui_view_title_bar_clear_pressed_state(self);

    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->subtle_fill_color = subtle_fill_color;
    local->subtle_border_color = subtle_border_color;
    local->shadow_color = shadow_color;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

uint8_t egui_view_title_bar_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region)
{
    egui_view_title_bar_t *local = egui_view_title_bar_local(self);
    const egui_view_title_bar_snapshot_t *snapshot = egui_view_title_bar_get_snapshot(local);
    egui_view_title_bar_metrics_t metrics;

    if (region == NULL || snapshot == NULL)
    {
        return 0;
    }

    egui_view_title_bar_get_metrics(local, self, snapshot, &metrics);
    switch (part)
    {
    case EGUI_VIEW_TITLE_BAR_PART_BACK:
        if (!snapshot->show_back_button)
        {
            return 0;
        }
        *region = metrics.back_region;
        return 1;
    case EGUI_VIEW_TITLE_BAR_PART_PANE_TOGGLE:
        if (!snapshot->show_pane_toggle)
        {
            return 0;
        }
        *region = metrics.pane_region;
        return 1;
    case EGUI_VIEW_TITLE_BAR_PART_PRIMARY_ACTION:
        if (!metrics.show_primary_action)
        {
            return 0;
        }
        *region = metrics.primary_action_region;
        return 1;
    case EGUI_VIEW_TITLE_BAR_PART_SECONDARY_ACTION:
        if (!metrics.show_secondary_action)
        {
            return 0;
        }
        *region = metrics.secondary_action_region;
        return 1;
    default:
        return 0;
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_title_bar_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    egui_view_title_bar_t *local = egui_view_title_bar_local(self);
    uint8_t hit_part;
    uint8_t handled;
    uint8_t activated_part;

    if (!egui_view_get_clickable(self))
    {
        return 0;
    }
    if (local->read_only_mode || !egui_view_get_enable(self))
    {
        egui_view_title_bar_clear_pressed_state(self);
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_part = egui_view_title_bar_resolve_hit(local, self, event->location.x, event->location.y);
        if (hit_part == EGUI_VIEW_TITLE_BAR_PART_NONE)
        {
            egui_view_title_bar_clear_pressed_state(self);
            return 0;
        }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        local->current_part = hit_part;
        local->pressed_part = hit_part;
        if (!self->is_pressed)
        {
            egui_view_set_pressed(self, 1);
        }
        else
        {
            egui_view_invalidate(self);
        }
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_part == EGUI_VIEW_TITLE_BAR_PART_NONE)
        {
            return 0;
        }
        hit_part = egui_view_title_bar_resolve_hit(local, self, event->location.x, event->location.y);
        if (hit_part == local->pressed_part)
        {
            if (!self->is_pressed)
            {
                egui_view_set_pressed(self, 1);
            }
            return 1;
        }
        if (self->is_pressed)
        {
            egui_view_set_pressed(self, 0);
        }
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
        hit_part = egui_view_title_bar_resolve_hit(local, self, event->location.x, event->location.y);
        handled = (uint8_t)(local->pressed_part != EGUI_VIEW_TITLE_BAR_PART_NONE || hit_part != EGUI_VIEW_TITLE_BAR_PART_NONE);
        activated_part = (uint8_t)((self->is_pressed && local->pressed_part == hit_part) ? local->pressed_part : EGUI_VIEW_TITLE_BAR_PART_NONE);
        egui_view_title_bar_clear_pressed_state(self);
        if (activated_part != EGUI_VIEW_TITLE_BAR_PART_NONE)
        {
            local->current_part = activated_part;
            egui_view_title_bar_notify_action(self, activated_part);
        }
        else if (hit_part != EGUI_VIEW_TITLE_BAR_PART_NONE)
        {
            local->current_part = hit_part;
            egui_view_invalidate(self);
        }
        return handled;
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        return egui_view_title_bar_clear_pressed_state(self);
    default:
        return 0;
    }
}

static int egui_view_title_bar_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_title_bar_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_title_bar_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    egui_view_title_bar_t *local = egui_view_title_bar_local(self);
    const egui_view_title_bar_snapshot_t *snapshot = egui_view_title_bar_get_snapshot(local);
    uint8_t target_part;
    uint8_t activated_part;

    if (!egui_view_get_enable(self) || local->read_only_mode || snapshot == NULL)
    {
        egui_view_title_bar_clear_pressed_state(self);
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_ENTER:
    case EGUI_KEY_CODE_SPACE:
        if (!egui_view_title_bar_part_active(snapshot, local->current_part))
        {
            local->current_part = egui_view_title_bar_resolve_first_part(snapshot);
        }
        if (local->current_part == EGUI_VIEW_TITLE_BAR_PART_NONE)
        {
            return 0;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->pressed_part = local->current_part;
            if (!self->is_pressed)
            {
                egui_view_set_pressed(self, 1);
            }
            else
            {
                egui_view_invalidate(self);
            }
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            activated_part = (uint8_t)((self->is_pressed && local->pressed_part == local->current_part) ? local->current_part : EGUI_VIEW_TITLE_BAR_PART_NONE);
            egui_view_title_bar_clear_pressed_state(self);
            if (activated_part != EGUI_VIEW_TITLE_BAR_PART_NONE)
            {
                egui_view_title_bar_notify_action(self, activated_part);
            }
            return 1;
        }
        return 0;
    case EGUI_KEY_CODE_LEFT:
    case EGUI_KEY_CODE_UP:
    case EGUI_KEY_CODE_RIGHT:
    case EGUI_KEY_CODE_DOWN:
    case EGUI_KEY_CODE_HOME:
    case EGUI_KEY_CODE_END:
    case EGUI_KEY_CODE_TAB:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            return 1;
        }
        if (event->type != EGUI_KEY_EVENT_ACTION_UP)
        {
            return 0;
        }
        if (!egui_view_title_bar_part_active(snapshot, local->current_part))
        {
            local->current_part = egui_view_title_bar_resolve_first_part(snapshot);
        }
        if (local->current_part == EGUI_VIEW_TITLE_BAR_PART_NONE)
        {
            return 0;
        }

        switch (event->key_code)
        {
        case EGUI_KEY_CODE_HOME:
            target_part = egui_view_title_bar_resolve_first_part(snapshot);
            break;
        case EGUI_KEY_CODE_END:
            target_part = egui_view_title_bar_resolve_next_part(snapshot, egui_view_title_bar_resolve_first_part(snapshot), 0);
            while (target_part != EGUI_VIEW_TITLE_BAR_PART_NONE &&
                   egui_view_title_bar_resolve_next_part(snapshot, target_part, 1) != egui_view_title_bar_resolve_first_part(snapshot))
            {
                target_part = egui_view_title_bar_resolve_next_part(snapshot, target_part, 1);
            }
            break;
        case EGUI_KEY_CODE_LEFT:
        case EGUI_KEY_CODE_UP:
            target_part = egui_view_title_bar_resolve_next_part(snapshot, local->current_part, 0);
            break;
        case EGUI_KEY_CODE_RIGHT:
        case EGUI_KEY_CODE_DOWN:
        case EGUI_KEY_CODE_TAB:
        default:
            target_part = egui_view_title_bar_resolve_next_part(snapshot, local->current_part, 1);
            break;
        }

        egui_view_title_bar_set_current_part(self, target_part);
        return 1;
    case EGUI_KEY_CODE_ESCAPE:
        egui_view_title_bar_clear_pressed_state(self);
        return 1;
    default:
        egui_view_title_bar_clear_pressed_state(self);
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_title_bar_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_title_bar_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static void egui_view_title_bar_on_focus_change(egui_view_t *self, int is_focused)
{
    egui_view_title_bar_t *local = egui_view_title_bar_local(self);

    if (!is_focused)
    {
        egui_view_title_bar_clear_pressed_state(self);
        egui_view_invalidate(self);
        return;
    }

    if (local->current_part == EGUI_VIEW_TITLE_BAR_PART_NONE)
    {
        egui_view_title_bar_sync_current_part(self);
    }
    egui_view_invalidate(self);
}
#endif

void egui_view_title_bar_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_title_bar_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_title_bar_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_title_bar_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_title_bar_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_title_bar_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_title_bar_on_key_event,
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        .on_focus_changed = egui_view_title_bar_on_focus_change,
#endif
};

void egui_view_title_bar_init(egui_view_t *self)
{
    egui_view_title_bar_t *local = egui_view_title_bar_local(self);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_title_bar_t);
    egui_view_set_padding_all(self, 2);
    egui_view_set_clickable(self, 1);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 1);
#endif

    local->snapshots = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->icon_font = NULL;
    local->on_action = NULL;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xD4DCE4);
    local->text_color = EGUI_COLOR_HEX(0x1C2935);
    local->muted_text_color = EGUI_COLOR_HEX(0x6D7B89);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->subtle_fill_color = EGUI_COLOR_HEX(0xF4F7FA);
    local->subtle_border_color = EGUI_COLOR_HEX(0xD7DFE7);
    local->shadow_color = EGUI_COLOR_HEX(0xDCE4EB);
    local->snapshot_count = 0;
    local->current_snapshot = 0;
    local->current_part = EGUI_VIEW_TITLE_BAR_PART_NONE;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_part = EGUI_VIEW_TITLE_BAR_PART_NONE;

    egui_view_set_view_name(self, "egui_view_title_bar");
}
