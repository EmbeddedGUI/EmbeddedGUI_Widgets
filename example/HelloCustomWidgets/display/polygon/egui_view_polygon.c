#include "egui_view_polygon.h"

#define EGUI_VIEW_POLYGON_STROKE_MIN 0
#define EGUI_VIEW_POLYGON_STROKE_MAX 6
#define EGUI_VIEW_POLYGON_PERCENT_MAX 100

static const uint8_t polygon_standard_points[] = {
        50, 8,
        90, 50,
        50, 92,
        10, 50,
};

static const uint8_t polygon_accent_points[] = {
        50, 6,
        92, 36,
        76, 90,
        24, 90,
        8, 36,
};

static egui_view_polygon_t *egui_view_polygon_local(egui_view_t *self)
{
    return (egui_view_polygon_t *)self;
}

static uint8_t egui_view_polygon_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static egui_dim_t egui_view_polygon_clamp_dim(egui_dim_t value, egui_dim_t min_value, egui_dim_t max_value)
{
    if (value < min_value)
    {
        return min_value;
    }
    if (value > max_value)
    {
        return max_value;
    }
    return value;
}

static uint8_t egui_view_polygon_clamp_percent(uint8_t value)
{
    return value > EGUI_VIEW_POLYGON_PERCENT_MAX ? EGUI_VIEW_POLYGON_PERCENT_MAX : value;
}

static egui_color_t egui_view_polygon_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0xAEB8C2), 62);
}

static egui_dim_t egui_view_polygon_resolve_axis(egui_dim_t origin, egui_dim_t length, uint8_t percent)
{
    if (length <= 1)
    {
        return origin;
    }
    return origin + (egui_dim_t)(((int32_t)(length - 1) * percent) / 100);
}

static void egui_view_polygon_resolve_points(egui_view_polygon_t *local, const egui_region_t *region, egui_dim_t *points)
{
    for (uint8_t i = 0; i < local->point_count; i++)
    {
        points[i * 2] = egui_view_polygon_resolve_axis(region->location.x, region->size.width, local->points_percent[i * 2]);
        points[i * 2 + 1] = egui_view_polygon_resolve_axis(region->location.y, region->size.height, local->points_percent[i * 2 + 1]);
    }
}

static void egui_view_polygon_on_draw(egui_view_t *self)
{
    egui_view_polygon_t *local = egui_view_polygon_local(self);
    egui_region_t region;
    egui_color_t fill_color = local->fill_color;
    egui_color_t stroke_color = local->stroke_color;
    egui_color_t accent_color = local->accent_color;
    egui_alpha_t fill_alpha = 52;
    egui_alpha_t stroke_alpha = 82;
    egui_dim_t points[EGUI_VIEW_POLYGON_MAX_POINTS * 2];

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0 || local->point_count < 3)
    {
        return;
    }

    if (!egui_view_get_enable(self))
    {
        fill_color = egui_view_polygon_mix_disabled(fill_color);
        stroke_color = egui_view_polygon_mix_disabled(stroke_color);
        accent_color = egui_view_polygon_mix_disabled(accent_color);
        fill_alpha = 28;
        stroke_alpha = 34;
    }
    if (egui_view_get_pressed(self))
    {
        stroke_color = egui_rgb_mix(stroke_color, accent_color, 18);
    }

    egui_view_polygon_resolve_points(local, &region, points);
    egui_canvas_draw_polygon_fill(&uicode_get_core()->canvas, points, local->point_count, fill_color, egui_color_alpha_mix(self->alpha, fill_alpha));
    if (local->stroke_width > 0)
    {
        egui_canvas_draw_polygon(&uicode_get_core()->canvas, points, local->point_count, local->stroke_width, stroke_color,
                                 egui_color_alpha_mix(self->alpha, stroke_alpha));
    }
}

void egui_view_polygon_set_palette(egui_view_t *self, egui_color_t fill_color, egui_color_t stroke_color, egui_color_t accent_color)
{
    egui_view_polygon_t *local = egui_view_polygon_local(self);

    egui_view_polygon_clear_pressed_state(self);
    local->fill_color = fill_color;
    local->stroke_color = stroke_color;
    local->accent_color = accent_color;
    egui_view_invalidate(self);
}

void egui_view_polygon_set_stroke_width(egui_view_t *self, egui_dim_t stroke_width)
{
    egui_view_polygon_t *local = egui_view_polygon_local(self);

    egui_view_polygon_clear_pressed_state(self);
    local->stroke_width = egui_view_polygon_clamp_dim(stroke_width, EGUI_VIEW_POLYGON_STROKE_MIN, EGUI_VIEW_POLYGON_STROKE_MAX);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_polygon_get_stroke_width(egui_view_t *self)
{
    egui_view_polygon_t *local = egui_view_polygon_local(self);

    return local->stroke_width;
}

void egui_view_polygon_set_points(egui_view_t *self, const uint8_t *points_percent, uint8_t point_count)
{
    egui_view_polygon_t *local = egui_view_polygon_local(self);
    uint8_t count = point_count;

    egui_view_polygon_clear_pressed_state(self);
    if (points_percent == NULL || count < 3)
    {
        local->point_count = 0;
        egui_view_invalidate(self);
        return;
    }
    if (count > EGUI_VIEW_POLYGON_MAX_POINTS)
    {
        count = EGUI_VIEW_POLYGON_MAX_POINTS;
    }

    local->point_count = count;
    for (uint8_t i = 0; i < count * 2; i++)
    {
        local->points_percent[i] = egui_view_polygon_clamp_percent(points_percent[i]);
    }
    egui_view_invalidate(self);
}

uint8_t egui_view_polygon_get_point_count(egui_view_t *self)
{
    egui_view_polygon_t *local = egui_view_polygon_local(self);

    return local->point_count;
}

void egui_view_polygon_get_point(egui_view_t *self, uint8_t index, uint8_t *x_percent, uint8_t *y_percent)
{
    egui_view_polygon_t *local = egui_view_polygon_local(self);

    if (index >= local->point_count)
    {
        if (x_percent)
        {
            *x_percent = 0;
        }
        if (y_percent)
        {
            *y_percent = 0;
        }
        return;
    }
    if (x_percent)
    {
        *x_percent = local->points_percent[index * 2];
    }
    if (y_percent)
    {
        *y_percent = local->points_percent[index * 2 + 1];
    }
}

void egui_view_polygon_apply_standard_style(egui_view_t *self)
{
    egui_view_polygon_set_palette(self, EGUI_COLOR_HEX(0xDDEBFA), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xBBD7F0));
    egui_view_polygon_set_stroke_width(self, 2);
    egui_view_polygon_set_points(self, polygon_standard_points, 4);
}

void egui_view_polygon_apply_accent_style(egui_view_t *self)
{
    egui_view_polygon_set_palette(self, EGUI_COLOR_HEX(0xD4E8FA), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x9EC7EA));
    egui_view_polygon_set_stroke_width(self, 3);
    egui_view_polygon_set_points(self, polygon_accent_points, 5);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_polygon_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_polygon_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_polygon_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_polygon_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_polygon_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_polygon_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_polygon_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_polygon_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_polygon_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_polygon_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_polygon_on_static_key_event;
#endif
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_polygon_t) = {
        .draw = egui_view_draw,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_polygon_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .on_draw = egui_view_polygon_on_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_polygon_on_key_event,
#endif
};

void egui_view_polygon_init(egui_view_t *self)
{
    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_polygon_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    egui_view_polygon_apply_standard_style(self);
    egui_view_set_view_name(self, "egui_view_polygon");
}
