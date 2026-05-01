#include "egui_view_rectangle.h"

#define EGUI_VIEW_RECTANGLE_STROKE_MIN  0
#define EGUI_VIEW_RECTANGLE_STROKE_MAX  6
#define EGUI_VIEW_RECTANGLE_RADIUS_MIN  0
#define EGUI_VIEW_RECTANGLE_RADIUS_MAX  18

static egui_view_rectangle_t *egui_view_rectangle_local(egui_view_t *self)
{
    return (egui_view_rectangle_t *)self;
}

static uint8_t egui_view_rectangle_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static egui_dim_t egui_view_rectangle_clamp(egui_dim_t value, egui_dim_t min_value, egui_dim_t max_value)
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

static egui_color_t egui_view_rectangle_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x8A97A5), 58);
}

static void egui_view_rectangle_on_draw(egui_view_t *self)
{
    egui_view_rectangle_t *local = egui_view_rectangle_local(self);
    egui_region_t region;
    egui_color_t fill_color = local->fill_color;
    egui_color_t stroke_color = local->stroke_color;
    egui_color_t accent_color = local->accent_color;
    egui_alpha_t fill_alpha = 92;
    egui_alpha_t stroke_alpha = 76;

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0)
    {
        return;
    }

    if (!egui_view_get_enable(self))
    {
        fill_color = egui_view_rectangle_mix_disabled(fill_color);
        stroke_color = egui_view_rectangle_mix_disabled(stroke_color);
        accent_color = egui_view_rectangle_mix_disabled(accent_color);
        fill_alpha = 58;
        stroke_alpha = 36;
    }
    if (egui_view_get_pressed(self))
    {
        fill_color = egui_rgb_mix(fill_color, accent_color, 12);
    }

    if (local->fill_enabled)
    {
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height,
                                              local->corner_radius, fill_color, egui_color_alpha_mix(self->alpha, fill_alpha));
    }
    if (local->stroke_width > 0)
    {
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height,
                                         local->corner_radius, local->stroke_width, stroke_color, egui_color_alpha_mix(self->alpha, stroke_alpha));
    }
}

void egui_view_rectangle_set_palette(egui_view_t *self, egui_color_t fill_color, egui_color_t stroke_color, egui_color_t accent_color)
{
    egui_view_rectangle_t *local = egui_view_rectangle_local(self);

    egui_view_rectangle_clear_pressed_state(self);
    local->fill_color = fill_color;
    local->stroke_color = stroke_color;
    local->accent_color = accent_color;
    egui_view_invalidate(self);
}

void egui_view_rectangle_set_stroke_width(egui_view_t *self, egui_dim_t stroke_width)
{
    egui_view_rectangle_t *local = egui_view_rectangle_local(self);

    egui_view_rectangle_clear_pressed_state(self);
    local->stroke_width = egui_view_rectangle_clamp(stroke_width, EGUI_VIEW_RECTANGLE_STROKE_MIN, EGUI_VIEW_RECTANGLE_STROKE_MAX);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_rectangle_get_stroke_width(egui_view_t *self)
{
    egui_view_rectangle_t *local = egui_view_rectangle_local(self);

    return local->stroke_width;
}

void egui_view_rectangle_set_corner_radius(egui_view_t *self, egui_dim_t corner_radius)
{
    egui_view_rectangle_t *local = egui_view_rectangle_local(self);

    egui_view_rectangle_clear_pressed_state(self);
    local->corner_radius = egui_view_rectangle_clamp(corner_radius, EGUI_VIEW_RECTANGLE_RADIUS_MIN, EGUI_VIEW_RECTANGLE_RADIUS_MAX);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_rectangle_get_corner_radius(egui_view_t *self)
{
    egui_view_rectangle_t *local = egui_view_rectangle_local(self);

    return local->corner_radius;
}

void egui_view_rectangle_set_fill_enabled(egui_view_t *self, uint8_t fill_enabled)
{
    egui_view_rectangle_t *local = egui_view_rectangle_local(self);

    egui_view_rectangle_clear_pressed_state(self);
    local->fill_enabled = fill_enabled ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_rectangle_get_fill_enabled(egui_view_t *self)
{
    egui_view_rectangle_t *local = egui_view_rectangle_local(self);

    return local->fill_enabled;
}

void egui_view_rectangle_apply_standard_style(egui_view_t *self)
{
    egui_view_rectangle_set_palette(self, EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xD7E3EE));
    egui_view_rectangle_set_stroke_width(self, 2);
    egui_view_rectangle_set_corner_radius(self, 8);
    egui_view_rectangle_set_fill_enabled(self, 1);
}

void egui_view_rectangle_apply_accent_style(egui_view_t *self)
{
    egui_view_rectangle_set_palette(self, EGUI_COLOR_HEX(0xF7FBFF), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xCFE2F3));
    egui_view_rectangle_set_stroke_width(self, 2);
    egui_view_rectangle_set_corner_radius(self, 12);
    egui_view_rectangle_set_fill_enabled(self, 1);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_rectangle_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_rectangle_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_rectangle_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_rectangle_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_rectangle_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_rectangle_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_rectangle_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_rectangle_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_rectangle_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_rectangle_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_rectangle_on_static_key_event;
#endif
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_rectangle_t) = {
        .draw = egui_view_draw,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_rectangle_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .on_draw = egui_view_rectangle_on_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_rectangle_on_key_event,
#endif
};

void egui_view_rectangle_init(egui_view_t *self)
{
    egui_view_rectangle_t *local = egui_view_rectangle_local(self);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_rectangle_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->fill_enabled = 1;
    egui_view_rectangle_apply_standard_style(self);
    egui_view_set_view_name(self, "egui_view_rectangle");
}
