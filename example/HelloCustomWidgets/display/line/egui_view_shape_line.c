#include "egui_view_shape_line.h"

#define EGUI_VIEW_SHAPE_LINE_STROKE_MIN 0
#define EGUI_VIEW_SHAPE_LINE_STROKE_MAX 6
#define EGUI_VIEW_SHAPE_LINE_PERCENT_MAX 100

static egui_view_shape_line_t *egui_view_shape_line_local(egui_view_t *self)
{
    return (egui_view_shape_line_t *)self;
}

static uint8_t egui_view_shape_line_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static egui_dim_t egui_view_shape_line_clamp_dim(egui_dim_t value, egui_dim_t min_value, egui_dim_t max_value)
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

static uint8_t egui_view_shape_line_clamp_percent(uint8_t value)
{
    return value > EGUI_VIEW_SHAPE_LINE_PERCENT_MAX ? EGUI_VIEW_SHAPE_LINE_PERCENT_MAX : value;
}

static egui_color_t egui_view_shape_line_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x8A97A5), 58);
}

static egui_dim_t egui_view_shape_line_resolve_axis(egui_dim_t origin, egui_dim_t length, uint8_t percent)
{
    if (length <= 1)
    {
        return origin;
    }
    return origin + (egui_dim_t)(((int32_t)(length - 1) * percent) / 100);
}

static void egui_view_shape_line_on_draw(egui_view_t *self)
{
    egui_view_shape_line_t *local = egui_view_shape_line_local(self);
    egui_region_t region;
    egui_color_t stroke_color = local->stroke_color;
    egui_color_t accent_color = local->accent_color;
    egui_alpha_t stroke_alpha = local->compact_mode ? 58 : 82;
    egui_dim_t x1;
    egui_dim_t y1;
    egui_dim_t x2;
    egui_dim_t y2;

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0 || local->stroke_width <= 0)
    {
        return;
    }

    if (local->read_only_mode)
    {
        stroke_color = egui_rgb_mix(stroke_color, EGUI_COLOR_HEX(0xAEB8C2), 54);
        accent_color = egui_rgb_mix(accent_color, EGUI_COLOR_HEX(0x8A97A5), 56);
        stroke_alpha = 42;
    }
    if (!egui_view_get_enable(self))
    {
        stroke_color = egui_view_shape_line_mix_disabled(stroke_color);
        accent_color = egui_view_shape_line_mix_disabled(accent_color);
        stroke_alpha = 34;
    }
    if (egui_view_get_pressed(self))
    {
        stroke_color = egui_rgb_mix(stroke_color, accent_color, 16);
    }

    x1 = egui_view_shape_line_resolve_axis(region.location.x, region.size.width, local->x1_percent);
    y1 = egui_view_shape_line_resolve_axis(region.location.y, region.size.height, local->y1_percent);
    x2 = egui_view_shape_line_resolve_axis(region.location.x, region.size.width, local->x2_percent);
    y2 = egui_view_shape_line_resolve_axis(region.location.y, region.size.height, local->y2_percent);
    egui_canvas_draw_line(&uicode_get_core()->canvas, x1, y1, x2, y2, local->stroke_width, stroke_color,
                          egui_color_alpha_mix(self->alpha, stroke_alpha));
}

void egui_view_shape_line_set_palette(egui_view_t *self, egui_color_t stroke_color, egui_color_t accent_color)
{
    egui_view_shape_line_t *local = egui_view_shape_line_local(self);

    egui_view_shape_line_clear_pressed_state(self);
    local->stroke_color = stroke_color;
    local->accent_color = accent_color;
    egui_view_invalidate(self);
}

void egui_view_shape_line_set_stroke_width(egui_view_t *self, egui_dim_t stroke_width)
{
    egui_view_shape_line_t *local = egui_view_shape_line_local(self);

    egui_view_shape_line_clear_pressed_state(self);
    local->stroke_width = egui_view_shape_line_clamp_dim(stroke_width, EGUI_VIEW_SHAPE_LINE_STROKE_MIN, EGUI_VIEW_SHAPE_LINE_STROKE_MAX);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_shape_line_get_stroke_width(egui_view_t *self)
{
    egui_view_shape_line_t *local = egui_view_shape_line_local(self);

    return local->stroke_width;
}

void egui_view_shape_line_set_points(egui_view_t *self, uint8_t x1_percent, uint8_t y1_percent, uint8_t x2_percent, uint8_t y2_percent)
{
    egui_view_shape_line_t *local = egui_view_shape_line_local(self);

    egui_view_shape_line_clear_pressed_state(self);
    local->x1_percent = egui_view_shape_line_clamp_percent(x1_percent);
    local->y1_percent = egui_view_shape_line_clamp_percent(y1_percent);
    local->x2_percent = egui_view_shape_line_clamp_percent(x2_percent);
    local->y2_percent = egui_view_shape_line_clamp_percent(y2_percent);
    egui_view_invalidate(self);
}

void egui_view_shape_line_get_points(egui_view_t *self, uint8_t *x1_percent, uint8_t *y1_percent, uint8_t *x2_percent, uint8_t *y2_percent)
{
    egui_view_shape_line_t *local = egui_view_shape_line_local(self);

    if (x1_percent)
    {
        *x1_percent = local->x1_percent;
    }
    if (y1_percent)
    {
        *y1_percent = local->y1_percent;
    }
    if (x2_percent)
    {
        *x2_percent = local->x2_percent;
    }
    if (y2_percent)
    {
        *y2_percent = local->y2_percent;
    }
}

void egui_view_shape_line_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    egui_view_shape_line_t *local = egui_view_shape_line_local(self);

    egui_view_shape_line_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_shape_line_get_compact_mode(egui_view_t *self)
{
    egui_view_shape_line_t *local = egui_view_shape_line_local(self);

    return local->compact_mode;
}

void egui_view_shape_line_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    egui_view_shape_line_t *local = egui_view_shape_line_local(self);

    egui_view_shape_line_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_shape_line_get_read_only_mode(egui_view_t *self)
{
    egui_view_shape_line_t *local = egui_view_shape_line_local(self);

    return local->read_only_mode;
}

void egui_view_shape_line_apply_standard_style(egui_view_t *self)
{
    egui_view_shape_line_set_palette(self, EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xD7E3EE));
    egui_view_shape_line_set_stroke_width(self, 2);
    egui_view_shape_line_set_points(self, 8, 50, 92, 50);
    egui_view_shape_line_set_compact_mode(self, 0);
    egui_view_shape_line_set_read_only_mode(self, 0);
}

void egui_view_shape_line_apply_accent_style(egui_view_t *self)
{
    egui_view_shape_line_set_palette(self, EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xCFE2F3));
    egui_view_shape_line_set_stroke_width(self, 3);
    egui_view_shape_line_set_points(self, 10, 82, 90, 18);
    egui_view_shape_line_set_compact_mode(self, 0);
    egui_view_shape_line_set_read_only_mode(self, 0);
}

void egui_view_shape_line_apply_compact_style(egui_view_t *self)
{
    egui_view_shape_line_set_palette(self, EGUI_COLOR_HEX(0x0C7C73), EGUI_COLOR_HEX(0xD9E7E5));
    egui_view_shape_line_set_stroke_width(self, 1);
    egui_view_shape_line_set_points(self, 50, 12, 50, 88);
    egui_view_shape_line_set_compact_mode(self, 1);
    egui_view_shape_line_set_read_only_mode(self, 0);
}

void egui_view_shape_line_apply_read_only_style(egui_view_t *self)
{
    egui_view_shape_line_set_palette(self, EGUI_COLOR_HEX(0x687684), EGUI_COLOR_HEX(0xE1E6EB));
    egui_view_shape_line_set_stroke_width(self, 1);
    egui_view_shape_line_set_points(self, 12, 50, 88, 50);
    egui_view_shape_line_set_compact_mode(self, 1);
    egui_view_shape_line_set_read_only_mode(self, 1);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_shape_line_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_shape_line_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_shape_line_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_shape_line_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_shape_line_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_shape_line_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_shape_line_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_shape_line_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_shape_line_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_shape_line_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_shape_line_on_static_key_event;
#endif
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_shape_line_t) = {
        .draw = egui_view_draw,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_shape_line_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .on_draw = egui_view_shape_line_on_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_shape_line_on_key_event,
#endif
};

void egui_view_shape_line_init(egui_view_t *self)
{
    egui_view_shape_line_t *local = egui_view_shape_line_local(self);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_shape_line_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->compact_mode = 0;
    local->read_only_mode = 0;
    egui_view_shape_line_apply_standard_style(self);
    egui_view_set_view_name(self, "egui_view_shape_line");
}
