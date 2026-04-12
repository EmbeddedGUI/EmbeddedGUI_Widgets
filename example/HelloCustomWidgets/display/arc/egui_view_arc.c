#include "egui_view_arc.h"

static uint8_t egui_view_arc_clear_pressed_state(egui_view_t *self)
{
    if (!self->is_pressed)
    {
        return 0;
    }
    egui_view_set_pressed(self, false);
    return 1;
}

static int16_t egui_view_arc_normalize_start_angle(int16_t start_angle)
{
    int16_t normalized = (int16_t)(start_angle % 360);

    if (normalized < 0)
    {
        normalized = (int16_t)(normalized + 360);
    }
    return normalized;
}

static int16_t egui_view_arc_clamp_sweep(int16_t sweep_angle)
{
    if (sweep_angle < 0)
    {
        return 0;
    }
    if (sweep_angle > 359)
    {
        return 359;
    }
    return sweep_angle;
}

static egui_dim_t egui_view_arc_clamp_stroke_width(egui_dim_t stroke_width)
{
    if (stroke_width < 1)
    {
        return 1;
    }
    return stroke_width;
}

static egui_color_t egui_view_arc_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 65);
}

static void egui_view_arc_apply_style(egui_view_t *self, egui_dim_t stroke_width, int16_t start_angle, int16_t sweep_angle, egui_color_t track_color,
                                      egui_color_t active_color)
{
    EGUI_LOCAL_INIT(egui_view_arc_t);

    egui_view_arc_clear_pressed_state(self);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);

    local->stroke_width = egui_view_arc_clamp_stroke_width(stroke_width);
    local->start_angle = egui_view_arc_normalize_start_angle(start_angle);
    local->sweep_angle = egui_view_arc_clamp_sweep(sweep_angle);
    local->track_color = track_color;
    local->active_color = active_color;
    egui_view_invalidate(self);
}

void egui_view_arc_apply_standard_style(egui_view_t *self)
{
    egui_view_arc_apply_style(self, 8, 140, 260, EGUI_COLOR_HEX(0xD9E2EA), EGUI_COLOR_HEX(0x0F6CBD));
}

void egui_view_arc_apply_subtle_style(egui_view_t *self)
{
    egui_view_arc_apply_style(self, 6, 140, 260, EGUI_COLOR_HEX(0xE5EBF0), EGUI_COLOR_HEX(0x8A9AA9));
}

void egui_view_arc_apply_attention_style(egui_view_t *self)
{
    egui_view_arc_apply_style(self, 8, 140, 260, EGUI_COLOR_HEX(0xEFD8D4), EGUI_COLOR_HEX(0xC42B1C));
}

void egui_view_arc_set_value(egui_view_t *self, uint8_t value)
{
    EGUI_LOCAL_INIT(egui_view_arc_t);

    egui_view_arc_clear_pressed_state(self);
    if (value > 100)
    {
        value = 100;
    }
    local->value = value;
    egui_view_invalidate(self);
}

uint8_t egui_view_arc_get_value(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_arc_t);
    return local->value;
}

void egui_view_arc_set_palette(egui_view_t *self, egui_color_t track_color, egui_color_t active_color)
{
    EGUI_LOCAL_INIT(egui_view_arc_t);

    egui_view_arc_clear_pressed_state(self);
    local->track_color = track_color;
    local->active_color = active_color;
    egui_view_invalidate(self);
}

void egui_view_arc_set_angles(egui_view_t *self, int16_t start_angle, int16_t sweep_angle)
{
    EGUI_LOCAL_INIT(egui_view_arc_t);

    egui_view_arc_clear_pressed_state(self);
    local->start_angle = egui_view_arc_normalize_start_angle(start_angle);
    local->sweep_angle = egui_view_arc_clamp_sweep(sweep_angle);
    egui_view_invalidate(self);
}

void egui_view_arc_set_stroke_width(egui_view_t *self, egui_dim_t stroke_width)
{
    EGUI_LOCAL_INIT(egui_view_arc_t);

    egui_view_arc_clear_pressed_state(self);
    local->stroke_width = egui_view_arc_clamp_stroke_width(stroke_width);
    egui_view_invalidate(self);
}

static void egui_view_arc_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_arc_t);
    egui_region_t region;
    egui_dim_t center_x;
    egui_dim_t center_y;
    egui_dim_t radius;
    egui_color_t track_color;
    egui_color_t active_color;
    int16_t progress_end_angle;

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0 || local->stroke_width <= 0 || local->sweep_angle <= 0)
    {
        return;
    }

    center_x = region.location.x + region.size.width / 2;
    center_y = region.location.y + region.size.height / 2;
    radius = EGUI_MIN(region.size.width, region.size.height) / 2 - local->stroke_width / 2 - 1;
    if (radius <= 0)
    {
        return;
    }

    track_color = local->track_color;
    active_color = local->active_color;
    if (!egui_view_get_enable(self))
    {
        track_color = egui_view_arc_mix_disabled(track_color);
        active_color = egui_view_arc_mix_disabled(active_color);
    }

    egui_canvas_draw_arc_round_cap_hq(center_x, center_y, radius, local->start_angle, local->start_angle + local->sweep_angle, local->stroke_width, track_color,
                                      egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));

    if (local->value == 0)
    {
        return;
    }

    progress_end_angle = (int16_t)(local->start_angle + (int16_t)((int32_t)local->sweep_angle * local->value / 100));
    egui_canvas_draw_arc_round_cap_hq(center_x, center_y, radius, local->start_angle, progress_end_angle, local->stroke_width, active_color,
                                      egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_arc_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_arc_clear_pressed_state(self);
    return 0;
}

static int egui_view_arc_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_arc_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_arc_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_arc_clear_pressed_state(self);
    return 0;
}

static int egui_view_arc_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_arc_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_arc_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_arc_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_arc_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_arc_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_arc_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_arc_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_arc_on_key_event,
#endif
};

void egui_view_arc_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_arc_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_arc_t);

    local->value = 0;
    local->start_angle = 140;
    local->sweep_angle = 260;
    local->stroke_width = 8;
    local->track_color = EGUI_COLOR_HEX(0xD9E2EA);
    local->active_color = EGUI_COLOR_HEX(0x0F6CBD);

    egui_view_set_view_name(self, "egui_view_arc");
}
