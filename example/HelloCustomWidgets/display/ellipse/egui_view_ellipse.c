#include "egui_view_ellipse.h"

#define EGUI_VIEW_ELLIPSE_STROKE_MIN 0
#define EGUI_VIEW_ELLIPSE_STROKE_MAX 6

static egui_view_ellipse_t *egui_view_ellipse_local(egui_view_t *self)
{
    return (egui_view_ellipse_t *)self;
}

static uint8_t egui_view_ellipse_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static egui_dim_t egui_view_ellipse_clamp(egui_dim_t value, egui_dim_t min_value, egui_dim_t max_value)
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

static egui_color_t egui_view_ellipse_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x8A97A5), 58);
}

static void egui_view_ellipse_on_draw(egui_view_t *self)
{
    egui_view_ellipse_t *local = egui_view_ellipse_local(self);
    egui_region_t region;
    egui_color_t fill_color = local->fill_color;
    egui_color_t stroke_color = local->stroke_color;
    egui_color_t accent_color = local->accent_color;
    egui_alpha_t fill_alpha = local->compact_mode ? 74 : 92;
    egui_alpha_t stroke_alpha = local->compact_mode ? 50 : 76;
    egui_dim_t center_x;
    egui_dim_t center_y;
    egui_dim_t radius_x;
    egui_dim_t radius_y;
    egui_dim_t radius;

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0)
    {
        return;
    }

    if (local->read_only_mode)
    {
        fill_color = egui_rgb_mix(fill_color, EGUI_COLOR_HEX(0xF5F7FA), 48);
        stroke_color = egui_rgb_mix(stroke_color, EGUI_COLOR_HEX(0xAEB8C2), 52);
        accent_color = egui_rgb_mix(accent_color, EGUI_COLOR_HEX(0x8A97A5), 56);
        fill_alpha = 70;
        stroke_alpha = 44;
    }
    if (!egui_view_get_enable(self))
    {
        fill_color = egui_view_ellipse_mix_disabled(fill_color);
        stroke_color = egui_view_ellipse_mix_disabled(stroke_color);
        accent_color = egui_view_ellipse_mix_disabled(accent_color);
        fill_alpha = 58;
        stroke_alpha = 36;
    }
    if (egui_view_get_pressed(self))
    {
        fill_color = egui_rgb_mix(fill_color, accent_color, 12);
    }

    center_x = region.location.x + region.size.width / 2;
    center_y = region.location.y + region.size.height / 2;
    radius_x = region.size.width / 2;
    radius_y = region.size.height / 2;
    radius = EGUI_MIN(radius_x, radius_y);

    if (local->circle_mode)
    {
        if (local->fill_enabled)
        {
            egui_canvas_draw_circle_fill(&uicode_get_core()->canvas, center_x, center_y, radius, fill_color,
                                         egui_color_alpha_mix(self->alpha, fill_alpha));
        }
        if (local->stroke_width > 0)
        {
            egui_canvas_draw_circle(&uicode_get_core()->canvas, center_x, center_y, radius, local->stroke_width, stroke_color,
                                    egui_color_alpha_mix(self->alpha, stroke_alpha));
        }
        return;
    }

    if (local->fill_enabled)
    {
        egui_canvas_draw_ellipse_fill(&uicode_get_core()->canvas, center_x, center_y, radius_x, radius_y, fill_color,
                                      egui_color_alpha_mix(self->alpha, fill_alpha));
    }
    if (local->stroke_width > 0)
    {
        egui_canvas_draw_ellipse(&uicode_get_core()->canvas, center_x, center_y, radius_x, radius_y, local->stroke_width, stroke_color,
                                 egui_color_alpha_mix(self->alpha, stroke_alpha));
    }
}

void egui_view_ellipse_set_palette(egui_view_t *self, egui_color_t fill_color, egui_color_t stroke_color, egui_color_t accent_color)
{
    egui_view_ellipse_t *local = egui_view_ellipse_local(self);

    egui_view_ellipse_clear_pressed_state(self);
    local->fill_color = fill_color;
    local->stroke_color = stroke_color;
    local->accent_color = accent_color;
    egui_view_invalidate(self);
}

void egui_view_ellipse_set_stroke_width(egui_view_t *self, egui_dim_t stroke_width)
{
    egui_view_ellipse_t *local = egui_view_ellipse_local(self);

    egui_view_ellipse_clear_pressed_state(self);
    local->stroke_width = egui_view_ellipse_clamp(stroke_width, EGUI_VIEW_ELLIPSE_STROKE_MIN, EGUI_VIEW_ELLIPSE_STROKE_MAX);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_ellipse_get_stroke_width(egui_view_t *self)
{
    egui_view_ellipse_t *local = egui_view_ellipse_local(self);

    return local->stroke_width;
}

void egui_view_ellipse_set_circle_mode(egui_view_t *self, uint8_t circle_mode)
{
    egui_view_ellipse_t *local = egui_view_ellipse_local(self);

    egui_view_ellipse_clear_pressed_state(self);
    local->circle_mode = circle_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_ellipse_get_circle_mode(egui_view_t *self)
{
    egui_view_ellipse_t *local = egui_view_ellipse_local(self);

    return local->circle_mode;
}

void egui_view_ellipse_set_fill_enabled(egui_view_t *self, uint8_t fill_enabled)
{
    egui_view_ellipse_t *local = egui_view_ellipse_local(self);

    egui_view_ellipse_clear_pressed_state(self);
    local->fill_enabled = fill_enabled ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_ellipse_get_fill_enabled(egui_view_t *self)
{
    egui_view_ellipse_t *local = egui_view_ellipse_local(self);

    return local->fill_enabled;
}

void egui_view_ellipse_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    egui_view_ellipse_t *local = egui_view_ellipse_local(self);

    egui_view_ellipse_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_ellipse_get_compact_mode(egui_view_t *self)
{
    egui_view_ellipse_t *local = egui_view_ellipse_local(self);

    return local->compact_mode;
}

void egui_view_ellipse_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    egui_view_ellipse_t *local = egui_view_ellipse_local(self);

    egui_view_ellipse_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_ellipse_get_read_only_mode(egui_view_t *self)
{
    egui_view_ellipse_t *local = egui_view_ellipse_local(self);

    return local->read_only_mode;
}

void egui_view_ellipse_apply_standard_style(egui_view_t *self)
{
    egui_view_ellipse_set_palette(self, EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xD7E3EE));
    egui_view_ellipse_set_stroke_width(self, 2);
    egui_view_ellipse_set_circle_mode(self, 0);
    egui_view_ellipse_set_fill_enabled(self, 1);
    egui_view_ellipse_set_compact_mode(self, 0);
    egui_view_ellipse_set_read_only_mode(self, 0);
}

void egui_view_ellipse_apply_accent_style(egui_view_t *self)
{
    egui_view_ellipse_set_palette(self, EGUI_COLOR_HEX(0xF7FBFF), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xCFE2F3));
    egui_view_ellipse_set_stroke_width(self, 2);
    egui_view_ellipse_set_circle_mode(self, 1);
    egui_view_ellipse_set_fill_enabled(self, 1);
    egui_view_ellipse_set_compact_mode(self, 0);
    egui_view_ellipse_set_read_only_mode(self, 0);
}

void egui_view_ellipse_apply_compact_style(egui_view_t *self)
{
    egui_view_ellipse_set_palette(self, EGUI_COLOR_HEX(0xF8FBFD), EGUI_COLOR_HEX(0x0C7C73), EGUI_COLOR_HEX(0xD9E7E5));
    egui_view_ellipse_set_stroke_width(self, 1);
    egui_view_ellipse_set_circle_mode(self, 0);
    egui_view_ellipse_set_fill_enabled(self, 1);
    egui_view_ellipse_set_compact_mode(self, 1);
    egui_view_ellipse_set_read_only_mode(self, 0);
}

void egui_view_ellipse_apply_read_only_style(egui_view_t *self)
{
    egui_view_ellipse_set_palette(self, EGUI_COLOR_HEX(0xF5F7FA), EGUI_COLOR_HEX(0x687684), EGUI_COLOR_HEX(0xE1E6EB));
    egui_view_ellipse_set_stroke_width(self, 1);
    egui_view_ellipse_set_circle_mode(self, 0);
    egui_view_ellipse_set_fill_enabled(self, 1);
    egui_view_ellipse_set_compact_mode(self, 1);
    egui_view_ellipse_set_read_only_mode(self, 1);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_ellipse_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_ellipse_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_ellipse_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_ellipse_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_ellipse_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_ellipse_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_ellipse_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_ellipse_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_ellipse_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_ellipse_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_ellipse_on_static_key_event;
#endif
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_ellipse_t) = {
        .draw = egui_view_draw,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_ellipse_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .on_draw = egui_view_ellipse_on_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_ellipse_on_key_event,
#endif
};

void egui_view_ellipse_init(egui_view_t *self)
{
    egui_view_ellipse_t *local = egui_view_ellipse_local(self);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_ellipse_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->fill_enabled = 1;
    local->circle_mode = 0;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    egui_view_ellipse_apply_standard_style(self);
    egui_view_set_view_name(self, "egui_view_ellipse");
}
