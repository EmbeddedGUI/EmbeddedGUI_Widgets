#include "egui_view_tick_bar.h"

#define EGUI_VIEW_TICK_BAR_MIN_SPAN   1
#define EGUI_VIEW_TICK_BAR_MAX_TICKS  28
#define EGUI_VIEW_TICK_BAR_RAIL_THICK 3

typedef struct tick_bar_metrics tick_bar_metrics_t;
struct tick_bar_metrics
{
    uint8_t vertical;
    egui_dim_t axis_min;
    egui_dim_t axis_max;
    egui_dim_t rail_x;
    egui_dim_t rail_y;
    egui_dim_t rail_w;
    egui_dim_t rail_h;
};

static egui_view_tick_bar_t *egui_view_tick_bar_local(egui_view_t *self)
{
    return (egui_view_tick_bar_t *)self;
}

static uint8_t egui_view_tick_bar_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static uint8_t egui_view_tick_bar_normalize_placement(uint8_t placement)
{
    return placement <= EGUI_VIEW_TICK_BAR_PLACEMENT_RIGHT ? placement : EGUI_VIEW_TICK_BAR_PLACEMENT_BOTTOM;
}

static uint8_t egui_view_tick_bar_is_vertical(uint8_t placement)
{
    return placement == EGUI_VIEW_TICK_BAR_PLACEMENT_LEFT || placement == EGUI_VIEW_TICK_BAR_PLACEMENT_RIGHT ? 1 : 0;
}

static int16_t egui_view_tick_bar_clamp_value(const egui_view_tick_bar_t *local, int16_t value)
{
    if (value < local->minimum)
    {
        return local->minimum;
    }
    if (value > local->maximum)
    {
        return local->maximum;
    }
    return value;
}

static void egui_view_tick_bar_normalize_model(egui_view_tick_bar_t *local)
{
    int16_t selection_start;
    int16_t selection_end;

    if (local->maximum <= local->minimum)
    {
        local->maximum = local->minimum + EGUI_VIEW_TICK_BAR_MIN_SPAN;
    }
    local->value = egui_view_tick_bar_clamp_value(local, local->value);
    local->selection_start = egui_view_tick_bar_clamp_value(local, local->selection_start);
    local->selection_end = egui_view_tick_bar_clamp_value(local, local->selection_end);
    if (local->selection_start > local->selection_end)
    {
        selection_start = local->selection_end;
        selection_end = local->selection_start;
        local->selection_start = selection_start;
        local->selection_end = selection_end;
    }
    if (local->tick_frequency == 0)
    {
        local->tick_frequency = 1;
    }
    local->placement = egui_view_tick_bar_normalize_placement(local->placement);
}

static egui_color_t egui_view_tick_bar_disabled_mix(egui_color_t color)
{
    return egui_rgb_mix(color, HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_MAKE(38));
}

static egui_dim_t egui_view_tick_bar_abs_dim(egui_dim_t value)
{
    return value < 0 ? -value : value;
}

static egui_dim_t egui_view_tick_bar_min_dim(egui_dim_t a, egui_dim_t b)
{
    return a < b ? a : b;
}

static uint8_t egui_view_tick_bar_build_metrics(egui_view_t *self, tick_bar_metrics_t *metrics)
{
    egui_view_tick_bar_t *local = egui_view_tick_bar_local(self);
    egui_region_t region;
    egui_dim_t inset;
    egui_dim_t rail_center;

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 12 || region.size.height <= 12)
    {
        return 0;
    }

    metrics->vertical = egui_view_tick_bar_is_vertical(local->placement);
    inset = local->compact_mode ? 5 : 8;
    if (metrics->vertical)
    {
        rail_center = local->placement == EGUI_VIEW_TICK_BAR_PLACEMENT_LEFT ? region.location.x + region.size.width - inset - 7
                                                                            : region.location.x + inset + 7;
        metrics->axis_min = region.location.y + region.size.height - inset;
        metrics->axis_max = region.location.y + inset;
        metrics->rail_x = rail_center - EGUI_VIEW_TICK_BAR_RAIL_THICK / 2;
        metrics->rail_y = metrics->axis_max;
        metrics->rail_w = EGUI_VIEW_TICK_BAR_RAIL_THICK;
        metrics->rail_h = metrics->axis_min - metrics->axis_max;
    }
    else
    {
        rail_center = local->placement == EGUI_VIEW_TICK_BAR_PLACEMENT_TOP ? region.location.y + region.size.height - inset - 7
                                                                           : region.location.y + inset + 7;
        metrics->axis_min = region.location.x + inset;
        metrics->axis_max = region.location.x + region.size.width - inset;
        metrics->rail_x = metrics->axis_min;
        metrics->rail_y = rail_center - EGUI_VIEW_TICK_BAR_RAIL_THICK / 2;
        metrics->rail_w = metrics->axis_max - metrics->axis_min;
        metrics->rail_h = EGUI_VIEW_TICK_BAR_RAIL_THICK;
    }

    return metrics->rail_w > 0 && metrics->rail_h > 0 ? 1 : 0;
}

static egui_dim_t egui_view_tick_bar_resolve_position(const egui_view_tick_bar_t *local, const tick_bar_metrics_t *metrics, int16_t value)
{
    int32_t range = (int32_t)local->maximum - local->minimum;
    int32_t offset = (int32_t)egui_view_tick_bar_clamp_value(local, value) - local->minimum;
    int32_t span = (int32_t)metrics->axis_max - metrics->axis_min;

    if (range <= 0)
    {
        return metrics->axis_min;
    }
    if (local->reversed)
    {
        offset = range - offset;
    }
    return metrics->axis_min + (egui_dim_t)((span * offset) / range);
}

static uint8_t egui_view_tick_bar_value_in_selection(const egui_view_tick_bar_t *local, int16_t value)
{
    return local->show_selected_range && value >= local->selection_start && value <= local->selection_end ? 1 : 0;
}

static void egui_view_tick_bar_draw_selected_range(egui_view_t *self, const tick_bar_metrics_t *metrics, egui_color_t color, egui_alpha_t alpha)
{
    egui_view_tick_bar_t *local = egui_view_tick_bar_local(self);
    egui_dim_t start_pos;
    egui_dim_t end_pos;
    egui_dim_t min_pos;
    egui_dim_t length;

    if (!local->show_selected_range)
    {
        return;
    }

    start_pos = egui_view_tick_bar_resolve_position(local, metrics, local->selection_start);
    end_pos = egui_view_tick_bar_resolve_position(local, metrics, local->selection_end);
    min_pos = egui_view_tick_bar_min_dim(start_pos, end_pos);
    length = egui_view_tick_bar_abs_dim(end_pos - start_pos);

    if (metrics->vertical)
    {
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics->rail_x, min_pos, metrics->rail_w, length + 1, 2, color, alpha);
    }
    else
    {
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, min_pos, metrics->rail_y, length + 1, metrics->rail_h, 2, color, alpha);
    }
}

static void egui_view_tick_bar_draw_tick(egui_view_t *self, const tick_bar_metrics_t *metrics, int16_t tick_value, uint8_t major, egui_color_t color,
                                         egui_alpha_t alpha)
{
    egui_view_tick_bar_t *local = egui_view_tick_bar_local(self);
    egui_dim_t pos = egui_view_tick_bar_resolve_position(local, metrics, tick_value);
    egui_dim_t length = major ? (local->compact_mode ? 10 : 13) : (local->compact_mode ? 6 : 8);
    egui_dim_t stroke = major ? 2 : 1;

    if (metrics->vertical)
    {
        egui_dim_t x1 = local->placement == EGUI_VIEW_TICK_BAR_PLACEMENT_LEFT ? metrics->rail_x : metrics->rail_x + metrics->rail_w;
        egui_dim_t x2 = local->placement == EGUI_VIEW_TICK_BAR_PLACEMENT_LEFT ? x1 - length : x1 + length;

        egui_canvas_draw_line(&uicode_get_core()->canvas, x1, pos, x2, pos, stroke, color, alpha);
    }
    else
    {
        egui_dim_t y1 = local->placement == EGUI_VIEW_TICK_BAR_PLACEMENT_TOP ? metrics->rail_y : metrics->rail_y + metrics->rail_h;
        egui_dim_t y2 = local->placement == EGUI_VIEW_TICK_BAR_PLACEMENT_TOP ? y1 - length : y1 + length;

        egui_canvas_draw_line(&uicode_get_core()->canvas, pos, y1, pos, y2, stroke, color, alpha);
    }
}

static void egui_view_tick_bar_draw_ticks(egui_view_t *self, const tick_bar_metrics_t *metrics, egui_color_t tick_color, egui_color_t selected_tick_color,
                                          egui_color_t value_color, egui_alpha_t tick_alpha)
{
    egui_view_tick_bar_t *local = egui_view_tick_bar_local(self);
    int32_t tick;
    int32_t step = local->tick_frequency;
    uint8_t drawn = 0;
    int16_t last_tick = local->minimum;

    if (step <= 0)
    {
        step = 1;
    }

    for (tick = local->minimum; tick <= local->maximum; tick += step)
    {
        int16_t tick_value = (int16_t)tick;
        uint8_t selected = egui_view_tick_bar_value_in_selection(local, tick_value);
        uint8_t major = tick_value == local->minimum || tick_value == local->maximum || tick_value == local->value ||
                        tick_value == local->selection_start || tick_value == local->selection_end;

        egui_view_tick_bar_draw_tick(self, metrics, tick_value, major, selected ? selected_tick_color : tick_color, tick_alpha);
        drawn++;
        last_tick = tick_value;
        if (drawn >= EGUI_VIEW_TICK_BAR_MAX_TICKS || tick > (int32_t)local->maximum - step)
        {
            break;
        }
    }

    if (last_tick != local->maximum)
    {
        egui_view_tick_bar_draw_tick(self, metrics, local->maximum, 1, egui_view_tick_bar_value_in_selection(local, local->maximum) ? selected_tick_color : tick_color,
                                     tick_alpha);
    }

    egui_view_tick_bar_draw_tick(self, metrics, local->value, 1, value_color, tick_alpha);
}

static void egui_view_tick_bar_draw_value_marker(egui_view_t *self, const tick_bar_metrics_t *metrics, egui_color_t value_color, egui_alpha_t alpha)
{
    egui_view_tick_bar_t *local = egui_view_tick_bar_local(self);
    egui_dim_t pos = egui_view_tick_bar_resolve_position(local, metrics, local->value);
    egui_dim_t radius = local->compact_mode ? 4 : 5;

    if (metrics->vertical)
    {
        egui_dim_t x = metrics->rail_x + metrics->rail_w / 2;

        egui_canvas_draw_circle_fill_basic(&uicode_get_core()->canvas, x, pos, radius, value_color, alpha);
    }
    else
    {
        egui_dim_t y = metrics->rail_y + metrics->rail_h / 2;

        egui_canvas_draw_circle_fill_basic(&uicode_get_core()->canvas, pos, y, radius, value_color, alpha);
    }
}

static void egui_view_tick_bar_on_draw(egui_view_t *self)
{
    egui_view_tick_bar_t *local = egui_view_tick_bar_local(self);
    tick_bar_metrics_t metrics;
    egui_color_t rail_color = local->rail_color;
    egui_color_t tick_color = local->tick_color;
    egui_color_t selected_tick_color = local->selected_tick_color;
    egui_color_t value_color = local->value_color;
    egui_alpha_t rail_alpha = EGUI_ALPHA_100;
    egui_alpha_t tick_alpha = EGUI_ALPHA_100;
    egui_alpha_t selected_alpha = EGUI_ALPHA_100;
    egui_alpha_t value_alpha = EGUI_ALPHA_100;

    if (!egui_view_tick_bar_build_metrics(self, &metrics))
    {
        return;
    }

    rail_color = egui_rgb_mix(rail_color, HCW_COLOR_BORDER_STRONG, EGUI_ALPHA_MAKE(local->compact_mode ? 82 : 76));
    tick_color = egui_rgb_mix(tick_color, HCW_COLOR_TEXT_STRONG, EGUI_ALPHA_MAKE(local->compact_mode ? 82 : 76));
    selected_tick_color = egui_rgb_mix(selected_tick_color, HCW_COLOR_PRIMARY_DARK, EGUI_ALPHA_MAKE(54));
    value_color = egui_rgb_mix(value_color, HCW_COLOR_PRIMARY_DARK, EGUI_ALPHA_MAKE(64));

    if (local->read_only_mode)
    {
        rail_color = egui_rgb_mix(rail_color, HCW_COLOR_BORDER_STRONG, EGUI_ALPHA_MAKE(72));
        tick_color = egui_rgb_mix(tick_color, HCW_COLOR_TEXT_STRONG, EGUI_ALPHA_MAKE(50));
        selected_tick_color = egui_rgb_mix(selected_tick_color, HCW_COLOR_PRIMARY_DARK, EGUI_ALPHA_MAKE(42));
        value_color = egui_rgb_mix(value_color, HCW_COLOR_TEXT_STRONG, EGUI_ALPHA_MAKE(44));
    }
    if (!egui_view_get_enable(self))
    {
        rail_color = egui_view_tick_bar_disabled_mix(rail_color);
        tick_color = egui_view_tick_bar_disabled_mix(tick_color);
        selected_tick_color = egui_view_tick_bar_disabled_mix(selected_tick_color);
        value_color = egui_view_tick_bar_disabled_mix(value_color);
        rail_alpha = EGUI_ALPHA_MAKE(68);
        tick_alpha = EGUI_ALPHA_MAKE(70);
        selected_alpha = EGUI_ALPHA_MAKE(68);
        value_alpha = EGUI_ALPHA_MAKE(74);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.rail_x, metrics.rail_y, metrics.rail_w, metrics.rail_h, 2, rail_color,
                                          egui_color_alpha_mix(self->alpha, rail_alpha));
    egui_view_tick_bar_draw_selected_range(self, &metrics, selected_tick_color, egui_color_alpha_mix(self->alpha, selected_alpha));
    egui_view_tick_bar_draw_ticks(self, &metrics, tick_color, selected_tick_color, value_color, egui_color_alpha_mix(self->alpha, tick_alpha));
    egui_view_tick_bar_draw_value_marker(self, &metrics, value_color, egui_color_alpha_mix(self->alpha, value_alpha));
}

void egui_view_tick_bar_set_range(egui_view_t *self, int16_t minimum, int16_t maximum)
{
    egui_view_tick_bar_t *local = egui_view_tick_bar_local(self);

    egui_view_tick_bar_clear_pressed_state(self);
    local->minimum = minimum;
    local->maximum = maximum;
    egui_view_tick_bar_normalize_model(local);
    egui_view_invalidate(self);
}

int16_t egui_view_tick_bar_get_minimum(egui_view_t *self)
{
    return egui_view_tick_bar_local(self)->minimum;
}

int16_t egui_view_tick_bar_get_maximum(egui_view_t *self)
{
    return egui_view_tick_bar_local(self)->maximum;
}

void egui_view_tick_bar_set_value(egui_view_t *self, int16_t value)
{
    egui_view_tick_bar_t *local = egui_view_tick_bar_local(self);

    egui_view_tick_bar_clear_pressed_state(self);
    local->value = egui_view_tick_bar_clamp_value(local, value);
    egui_view_invalidate(self);
}

int16_t egui_view_tick_bar_get_value(egui_view_t *self)
{
    return egui_view_tick_bar_local(self)->value;
}

void egui_view_tick_bar_set_selection_range(egui_view_t *self, int16_t selection_start, int16_t selection_end)
{
    egui_view_tick_bar_t *local = egui_view_tick_bar_local(self);

    egui_view_tick_bar_clear_pressed_state(self);
    local->selection_start = selection_start;
    local->selection_end = selection_end;
    egui_view_tick_bar_normalize_model(local);
    egui_view_invalidate(self);
}

void egui_view_tick_bar_get_selection_range(egui_view_t *self, int16_t *selection_start, int16_t *selection_end)
{
    egui_view_tick_bar_t *local = egui_view_tick_bar_local(self);

    if (selection_start != NULL)
    {
        *selection_start = local->selection_start;
    }
    if (selection_end != NULL)
    {
        *selection_end = local->selection_end;
    }
}

void egui_view_tick_bar_set_tick_frequency(egui_view_t *self, uint8_t tick_frequency)
{
    egui_view_tick_bar_t *local = egui_view_tick_bar_local(self);

    egui_view_tick_bar_clear_pressed_state(self);
    local->tick_frequency = tick_frequency == 0 ? 1 : tick_frequency;
    egui_view_invalidate(self);
}

uint8_t egui_view_tick_bar_get_tick_frequency(egui_view_t *self)
{
    return egui_view_tick_bar_local(self)->tick_frequency;
}

void egui_view_tick_bar_set_placement(egui_view_t *self, uint8_t placement)
{
    egui_view_tick_bar_t *local = egui_view_tick_bar_local(self);

    egui_view_tick_bar_clear_pressed_state(self);
    local->placement = egui_view_tick_bar_normalize_placement(placement);
    egui_view_invalidate(self);
}

uint8_t egui_view_tick_bar_get_placement(egui_view_t *self)
{
    return egui_view_tick_bar_local(self)->placement;
}

void egui_view_tick_bar_set_reversed(egui_view_t *self, uint8_t reversed)
{
    egui_view_tick_bar_t *local = egui_view_tick_bar_local(self);

    egui_view_tick_bar_clear_pressed_state(self);
    local->reversed = reversed ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_tick_bar_get_reversed(egui_view_t *self)
{
    return egui_view_tick_bar_local(self)->reversed;
}

void egui_view_tick_bar_set_show_selected_range(egui_view_t *self, uint8_t show_selected_range)
{
    egui_view_tick_bar_t *local = egui_view_tick_bar_local(self);

    egui_view_tick_bar_clear_pressed_state(self);
    local->show_selected_range = show_selected_range ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_tick_bar_get_show_selected_range(egui_view_t *self)
{
    return egui_view_tick_bar_local(self)->show_selected_range;
}

void egui_view_tick_bar_set_colors(egui_view_t *self, egui_color_t rail_color, egui_color_t tick_color, egui_color_t selected_tick_color,
                                   egui_color_t value_color)
{
    egui_view_tick_bar_t *local = egui_view_tick_bar_local(self);

    egui_view_tick_bar_clear_pressed_state(self);
    local->rail_color = rail_color;
    local->tick_color = tick_color;
    local->selected_tick_color = selected_tick_color;
    local->value_color = value_color;
    egui_view_invalidate(self);
}

void egui_view_tick_bar_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    egui_view_tick_bar_t *local = egui_view_tick_bar_local(self);

    egui_view_tick_bar_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_tick_bar_get_compact_mode(egui_view_t *self)
{
    return egui_view_tick_bar_local(self)->compact_mode;
}

void egui_view_tick_bar_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    egui_view_tick_bar_t *local = egui_view_tick_bar_local(self);

    egui_view_tick_bar_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_tick_bar_get_read_only_mode(egui_view_t *self)
{
    return egui_view_tick_bar_local(self)->read_only_mode;
}

void egui_view_tick_bar_apply_standard_style(egui_view_t *self)
{
    egui_view_tick_bar_set_range(self, 0, 100);
    egui_view_tick_bar_set_value(self, 40);
    egui_view_tick_bar_set_selection_range(self, 20, 70);
    egui_view_tick_bar_set_tick_frequency(self, 10);
    egui_view_tick_bar_set_placement(self, EGUI_VIEW_TICK_BAR_PLACEMENT_BOTTOM);
    egui_view_tick_bar_set_reversed(self, 0);
    egui_view_tick_bar_set_show_selected_range(self, 1);
    egui_view_tick_bar_set_compact_mode(self, 0);
    egui_view_tick_bar_set_read_only_mode(self, 0);
    egui_view_tick_bar_set_colors(self, HCW_COLOR_TRACK_STRONG, HCW_COLOR_TEXT_SOFT, HCW_COLOR_PRIMARY, HCW_COLOR_PRIMARY_DARK);
}

void egui_view_tick_bar_apply_accent_style(egui_view_t *self)
{
    egui_view_tick_bar_set_range(self, 0, 120);
    egui_view_tick_bar_set_value(self, 75);
    egui_view_tick_bar_set_selection_range(self, 30, 90);
    egui_view_tick_bar_set_tick_frequency(self, 15);
    egui_view_tick_bar_set_placement(self, EGUI_VIEW_TICK_BAR_PLACEMENT_TOP);
    egui_view_tick_bar_set_reversed(self, 0);
    egui_view_tick_bar_set_show_selected_range(self, 1);
    egui_view_tick_bar_set_compact_mode(self, 0);
    egui_view_tick_bar_set_read_only_mode(self, 0);
    egui_view_tick_bar_set_colors(self, HCW_COLOR_PRIMARY_TINT, HCW_COLOR_TEXT_SOFT, HCW_COLOR_PRIMARY, HCW_COLOR_PRIMARY_DARK);
}

void egui_view_tick_bar_apply_vertical_style(egui_view_t *self)
{
    egui_view_tick_bar_set_range(self, 0, 10);
    egui_view_tick_bar_set_value(self, 6);
    egui_view_tick_bar_set_selection_range(self, 3, 7);
    egui_view_tick_bar_set_tick_frequency(self, 1);
    egui_view_tick_bar_set_placement(self, EGUI_VIEW_TICK_BAR_PLACEMENT_LEFT);
    egui_view_tick_bar_set_reversed(self, 1);
    egui_view_tick_bar_set_show_selected_range(self, 1);
    egui_view_tick_bar_set_compact_mode(self, 0);
    egui_view_tick_bar_set_read_only_mode(self, 0);
    egui_view_tick_bar_set_colors(self, HCW_COLOR_TRACK, HCW_COLOR_TEXT_SOFT, HCW_COLOR_PRIMARY, HCW_COLOR_PRIMARY_DARK);
}

void egui_view_tick_bar_apply_compact_style(egui_view_t *self)
{
    egui_view_tick_bar_set_range(self, 0, 6);
    egui_view_tick_bar_set_value(self, 2);
    egui_view_tick_bar_set_selection_range(self, 1, 4);
    egui_view_tick_bar_set_tick_frequency(self, 1);
    egui_view_tick_bar_set_placement(self, EGUI_VIEW_TICK_BAR_PLACEMENT_BOTTOM);
    egui_view_tick_bar_set_reversed(self, 0);
    egui_view_tick_bar_set_show_selected_range(self, 1);
    egui_view_tick_bar_set_compact_mode(self, 1);
    egui_view_tick_bar_set_read_only_mode(self, 0);
    egui_view_tick_bar_set_colors(self, HCW_COLOR_TRACK_STRONG, HCW_COLOR_TEXT_SOFT, HCW_COLOR_PRIMARY, HCW_COLOR_PRIMARY_DARK);
}

void egui_view_tick_bar_apply_read_only_style(egui_view_t *self)
{
    egui_view_tick_bar_set_range(self, 0, 100);
    egui_view_tick_bar_set_value(self, 65);
    egui_view_tick_bar_set_selection_range(self, 25, 75);
    egui_view_tick_bar_set_tick_frequency(self, 25);
    egui_view_tick_bar_set_placement(self, EGUI_VIEW_TICK_BAR_PLACEMENT_RIGHT);
    egui_view_tick_bar_set_reversed(self, 0);
    egui_view_tick_bar_set_show_selected_range(self, 1);
    egui_view_tick_bar_set_compact_mode(self, 1);
    egui_view_tick_bar_set_read_only_mode(self, 1);
    egui_view_tick_bar_set_colors(self, HCW_COLOR_BORDER_STRONG, HCW_COLOR_TEXT_SOFT, HCW_COLOR_PRIMARY_SOFT, HCW_COLOR_TEXT_SOFT);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_tick_bar_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_tick_bar_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_tick_bar_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_tick_bar_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_tick_bar_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_tick_bar_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_tick_bar_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_tick_bar_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_tick_bar_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_tick_bar_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_tick_bar_on_static_key_event;
#endif
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_tick_bar_t) = {
        .draw = egui_view_draw,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_tick_bar_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .on_draw = egui_view_tick_bar_on_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_tick_bar_on_key_event,
#endif
};

void egui_view_tick_bar_init(egui_view_t *self)
{
    egui_view_tick_bar_t *local = egui_view_tick_bar_local(self);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_tick_bar_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->minimum = 0;
    local->maximum = 100;
    local->value = 40;
    local->selection_start = 20;
    local->selection_end = 70;
    local->tick_frequency = 10;
    local->placement = EGUI_VIEW_TICK_BAR_PLACEMENT_BOTTOM;
    local->reversed = 0;
    local->show_selected_range = 1;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    egui_view_tick_bar_apply_standard_style(self);
    egui_view_set_view_name(self, "egui_view_tick_bar");
}
