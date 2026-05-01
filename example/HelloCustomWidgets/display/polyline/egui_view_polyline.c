#include "egui_view_polyline.h"

#define EGUI_VIEW_POLYLINE_STROKE_MIN 0
#define EGUI_VIEW_POLYLINE_STROKE_MAX 6
#define EGUI_VIEW_POLYLINE_PERCENT_MAX 100

static const uint8_t polyline_standard_points[] = {
        8, 72,
        32, 38,
        55, 58,
        78, 24,
        92, 34,
};

static const uint8_t polyline_accent_points[] = {
        8, 78,
        25, 64,
        42, 72,
        60, 34,
        78, 22,
        92, 12,
};

static egui_view_polyline_t *egui_view_polyline_local(egui_view_t *self)
{
    return (egui_view_polyline_t *)self;
}

static uint8_t egui_view_polyline_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static egui_dim_t egui_view_polyline_clamp_dim(egui_dim_t value, egui_dim_t min_value, egui_dim_t max_value)
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

static uint8_t egui_view_polyline_clamp_percent(uint8_t value)
{
    return value > EGUI_VIEW_POLYLINE_PERCENT_MAX ? EGUI_VIEW_POLYLINE_PERCENT_MAX : value;
}

static egui_color_t egui_view_polyline_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x8A97A5), 58);
}

static egui_dim_t egui_view_polyline_resolve_axis(egui_dim_t origin, egui_dim_t length, uint8_t percent)
{
    if (length <= 1)
    {
        return origin;
    }
    return origin + (egui_dim_t)(((int32_t)(length - 1) * percent) / 100);
}

static void egui_view_polyline_resolve_points(egui_view_polyline_t *local, const egui_region_t *region, egui_dim_t *points)
{
    for (uint8_t i = 0; i < local->point_count; i++)
    {
        points[i * 2] = egui_view_polyline_resolve_axis(region->location.x, region->size.width, local->points_percent[i * 2]);
        points[i * 2 + 1] = egui_view_polyline_resolve_axis(region->location.y, region->size.height, local->points_percent[i * 2 + 1]);
    }
}

static void egui_view_polyline_on_draw(egui_view_t *self)
{
    egui_view_polyline_t *local = egui_view_polyline_local(self);
    egui_region_t region;
    egui_color_t stroke_color = local->stroke_color;
    egui_color_t accent_color = local->accent_color;
    egui_alpha_t stroke_alpha = 82;
    egui_dim_t points[EGUI_VIEW_POLYLINE_MAX_POINTS * 2];

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0 || local->point_count < 2 || local->stroke_width <= 0)
    {
        return;
    }

    if (!egui_view_get_enable(self))
    {
        stroke_color = egui_view_polyline_mix_disabled(stroke_color);
        accent_color = egui_view_polyline_mix_disabled(accent_color);
        stroke_alpha = 34;
    }
    if (egui_view_get_pressed(self))
    {
        stroke_color = egui_rgb_mix(stroke_color, accent_color, 16);
    }

    egui_view_polyline_resolve_points(local, &region, points);
    for (uint8_t i = 0; i + 1 < local->point_count; i++)
    {
        egui_canvas_draw_line(&uicode_get_core()->canvas, points[i * 2], points[i * 2 + 1], points[(i + 1) * 2], points[(i + 1) * 2 + 1],
                              local->stroke_width, stroke_color, egui_color_alpha_mix(self->alpha, stroke_alpha));
    }
}

void egui_view_polyline_set_palette(egui_view_t *self, egui_color_t stroke_color, egui_color_t accent_color)
{
    egui_view_polyline_t *local = egui_view_polyline_local(self);

    egui_view_polyline_clear_pressed_state(self);
    local->stroke_color = stroke_color;
    local->accent_color = accent_color;
    egui_view_invalidate(self);
}

void egui_view_polyline_set_stroke_width(egui_view_t *self, egui_dim_t stroke_width)
{
    egui_view_polyline_t *local = egui_view_polyline_local(self);

    egui_view_polyline_clear_pressed_state(self);
    local->stroke_width = egui_view_polyline_clamp_dim(stroke_width, EGUI_VIEW_POLYLINE_STROKE_MIN, EGUI_VIEW_POLYLINE_STROKE_MAX);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_polyline_get_stroke_width(egui_view_t *self)
{
    egui_view_polyline_t *local = egui_view_polyline_local(self);

    return local->stroke_width;
}

void egui_view_polyline_set_points(egui_view_t *self, const uint8_t *points_percent, uint8_t point_count)
{
    egui_view_polyline_t *local = egui_view_polyline_local(self);
    uint8_t count = point_count;

    egui_view_polyline_clear_pressed_state(self);
    if (points_percent == NULL || count < 2)
    {
        local->point_count = 0;
        egui_view_invalidate(self);
        return;
    }
    if (count > EGUI_VIEW_POLYLINE_MAX_POINTS)
    {
        count = EGUI_VIEW_POLYLINE_MAX_POINTS;
    }

    local->point_count = count;
    for (uint8_t i = 0; i < count * 2; i++)
    {
        local->points_percent[i] = egui_view_polyline_clamp_percent(points_percent[i]);
    }
    egui_view_invalidate(self);
}

uint8_t egui_view_polyline_get_point_count(egui_view_t *self)
{
    egui_view_polyline_t *local = egui_view_polyline_local(self);

    return local->point_count;
}

void egui_view_polyline_get_point(egui_view_t *self, uint8_t index, uint8_t *x_percent, uint8_t *y_percent)
{
    egui_view_polyline_t *local = egui_view_polyline_local(self);

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

void egui_view_polyline_apply_standard_style(egui_view_t *self)
{
    egui_view_polyline_set_palette(self, EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xD7E3EE));
    egui_view_polyline_set_stroke_width(self, 2);
    egui_view_polyline_set_points(self, polyline_standard_points, 5);
}

void egui_view_polyline_apply_accent_style(egui_view_t *self)
{
    egui_view_polyline_set_palette(self, EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xCFE2F3));
    egui_view_polyline_set_stroke_width(self, 3);
    egui_view_polyline_set_points(self, polyline_accent_points, 6);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_polyline_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_polyline_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_polyline_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_polyline_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_polyline_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_polyline_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_polyline_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_polyline_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_polyline_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_polyline_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_polyline_on_static_key_event;
#endif
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_polyline_t) = {
        .draw = egui_view_draw,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_polyline_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .on_draw = egui_view_polyline_on_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_polyline_on_key_event,
#endif
};

void egui_view_polyline_init(egui_view_t *self)
{
    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_polyline_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    egui_view_polyline_apply_standard_style(self);
    egui_view_set_view_name(self, "egui_view_polyline");
}
