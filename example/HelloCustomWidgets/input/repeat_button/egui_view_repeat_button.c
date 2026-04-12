#include "egui_view_repeat_button.h"

void egui_view_button_on_draw(egui_view_t *self);

#define EGUI_VIEW_REPEAT_BUTTON_DELAY_DEFAULT    360
#define EGUI_VIEW_REPEAT_BUTTON_INTERVAL_DEFAULT  90
#define EGUI_VIEW_REPEAT_BUTTON_DELAY_MIN         80
#define EGUI_VIEW_REPEAT_BUTTON_INTERVAL_MIN      40

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(repeat_button_standard_bg_normal_param, EGUI_COLOR_HEX(0x2563EB), EGUI_ALPHA_100, 10, 1,
                                                        EGUI_COLOR_HEX(0x1E54C8), EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(repeat_button_standard_bg_pressed_param, EGUI_COLOR_HEX(0x1E4FCA), EGUI_ALPHA_100, 10, 1,
                                                        EGUI_COLOR_HEX(0x1840A7), EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(repeat_button_standard_bg_disabled_param, EGUI_COLOR_HEX(0xAAB5C1), EGUI_ALPHA_100, 10, 1,
                                                        EGUI_COLOR_HEX(0xAAB5C1), EGUI_ALPHA_100);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(repeat_button_standard_bg_focused_param, EGUI_COLOR_HEX(0x2563EB), EGUI_ALPHA_100, 10, 2,
                                                        EGUI_COLOR_HEX(0x9AC5FF), EGUI_ALPHA_100);
EGUI_BACKGROUND_PARAM_INIT_WITH_FOCUS(repeat_button_standard_bg_params, &repeat_button_standard_bg_normal_param, &repeat_button_standard_bg_pressed_param,
                                      &repeat_button_standard_bg_disabled_param, &repeat_button_standard_bg_focused_param);
#else
EGUI_BACKGROUND_PARAM_INIT(repeat_button_standard_bg_params, &repeat_button_standard_bg_normal_param, &repeat_button_standard_bg_pressed_param,
                           &repeat_button_standard_bg_disabled_param);
#endif
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(repeat_button_standard_background, &repeat_button_standard_bg_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(repeat_button_compact_bg_normal_param, EGUI_COLOR_HEX(0x0C7C73), EGUI_ALPHA_100, 8, 1,
                                                        EGUI_COLOR_HEX(0x09655F), EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(repeat_button_compact_bg_pressed_param, EGUI_COLOR_HEX(0x0A6B63), EGUI_ALPHA_100, 8, 1,
                                                        EGUI_COLOR_HEX(0x08514A), EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(repeat_button_compact_bg_disabled_param, EGUI_COLOR_HEX(0xB4C3BC), EGUI_ALPHA_100, 8, 1,
                                                        EGUI_COLOR_HEX(0xB4C3BC), EGUI_ALPHA_100);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(repeat_button_compact_bg_focused_param, EGUI_COLOR_HEX(0x0C7C73), EGUI_ALPHA_100, 8, 2,
                                                        EGUI_COLOR_HEX(0x8ED9CE), EGUI_ALPHA_100);
EGUI_BACKGROUND_PARAM_INIT_WITH_FOCUS(repeat_button_compact_bg_params, &repeat_button_compact_bg_normal_param, &repeat_button_compact_bg_pressed_param,
                                      &repeat_button_compact_bg_disabled_param, &repeat_button_compact_bg_focused_param);
#else
EGUI_BACKGROUND_PARAM_INIT(repeat_button_compact_bg_params, &repeat_button_compact_bg_normal_param, &repeat_button_compact_bg_pressed_param,
                           &repeat_button_compact_bg_disabled_param);
#endif
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(repeat_button_compact_background, &repeat_button_compact_bg_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(repeat_button_disabled_bg_normal_param, EGUI_COLOR_HEX(0xEEF2F6), EGUI_ALPHA_100, 8, 1,
                                                        EGUI_COLOR_HEX(0xD0D8E1), EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(repeat_button_disabled_bg_pressed_param, EGUI_COLOR_HEX(0xE6ECF2), EGUI_ALPHA_100, 8, 1,
                                                        EGUI_COLOR_HEX(0xCBD4DE), EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(repeat_button_disabled_bg_disabled_param, EGUI_COLOR_HEX(0xEEF2F6), EGUI_ALPHA_100, 8, 1,
                                                        EGUI_COLOR_HEX(0xD0D8E1), EGUI_ALPHA_100);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(repeat_button_disabled_bg_focused_param, EGUI_COLOR_HEX(0xEEF2F6), EGUI_ALPHA_100, 8, 2,
                                                        EGUI_COLOR_HEX(0xC5D1DD), EGUI_ALPHA_100);
EGUI_BACKGROUND_PARAM_INIT_WITH_FOCUS(repeat_button_disabled_bg_params, &repeat_button_disabled_bg_normal_param, &repeat_button_disabled_bg_pressed_param,
                                      &repeat_button_disabled_bg_disabled_param, &repeat_button_disabled_bg_focused_param);
#else
EGUI_BACKGROUND_PARAM_INIT(repeat_button_disabled_bg_params, &repeat_button_disabled_bg_normal_param, &repeat_button_disabled_bg_pressed_param,
                           &repeat_button_disabled_bg_disabled_param);
#endif
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(repeat_button_disabled_background, &repeat_button_disabled_bg_params);

static egui_view_repeat_button_t *egui_view_repeat_button_local(egui_view_t *self)
{
    return (egui_view_repeat_button_t *)self;
}

static uint16_t egui_view_repeat_button_resolve_delay(uint16_t delay_ms)
{
    if (delay_ms == 0)
    {
        return EGUI_VIEW_REPEAT_BUTTON_DELAY_DEFAULT;
    }
    if (delay_ms < EGUI_VIEW_REPEAT_BUTTON_DELAY_MIN)
    {
        return EGUI_VIEW_REPEAT_BUTTON_DELAY_MIN;
    }
    return delay_ms;
}

static uint16_t egui_view_repeat_button_resolve_interval(uint16_t interval_ms)
{
    if (interval_ms == 0)
    {
        return EGUI_VIEW_REPEAT_BUTTON_INTERVAL_DEFAULT;
    }
    if (interval_ms < EGUI_VIEW_REPEAT_BUTTON_INTERVAL_MIN)
    {
        return EGUI_VIEW_REPEAT_BUTTON_INTERVAL_MIN;
    }
    return interval_ms;
}

static void egui_view_repeat_button_stop_timer(egui_view_t *self)
{
    egui_view_repeat_button_t *local = egui_view_repeat_button_local(self);

    if (!local->timer_started)
    {
        return;
    }

    egui_timer_stop_timer(&local->repeat_timer);
    local->timer_started = 0;
}

static void egui_view_repeat_button_start_timer(egui_view_t *self)
{
    egui_view_repeat_button_t *local = egui_view_repeat_button_local(self);

    if (local->timer_started || !self->is_attached_to_window || !egui_view_get_enable(self) || !egui_view_get_pressed(self) ||
        (!local->touch_active && !local->key_active))
    {
        return;
    }

    egui_timer_start_timer(&local->repeat_timer, local->initial_delay_ms, local->repeat_interval_ms);
    local->timer_started = 1;
}

static uint8_t egui_view_repeat_button_clear_pressed_state(egui_view_t *self)
{
    egui_view_repeat_button_t *local = egui_view_repeat_button_local(self);
    uint8_t had_state = self->is_pressed || local->touch_active || local->key_active || local->timer_started;

    egui_view_repeat_button_stop_timer(self);
    local->touch_active = 0;
    local->key_active = 0;
    egui_view_set_pressed(self, 0);
    return had_state;
}

static void egui_view_repeat_button_apply_style(egui_view_t *self, egui_background_t *background, egui_color_t text_color, egui_dim_t gap,
                                                egui_dim_t horizontal_padding)
{
    egui_view_button_t *local = (egui_view_button_t *)self;
    uint8_t had_state = egui_view_repeat_button_clear_pressed_state(self);

    egui_view_set_shadow(self, NULL);
    egui_view_set_background(self, background);
    self->padding.left = horizontal_padding;
    self->padding.right = horizontal_padding;
    self->padding.top = 0;
    self->padding.bottom = 0;
    local->base.color = text_color;
    local->base.alpha = EGUI_ALPHA_100;
    local->icon_text_gap = gap;
    if (!had_state)
    {
        egui_view_invalidate(self);
    }
}

static void egui_view_repeat_button_apply_focus_from_touch(egui_view_t *self, uint8_t is_inside)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (is_inside && self->is_focusable)
    {
        egui_view_request_focus(self);
    }
    else if (is_inside && !self->is_no_focus_clear)
    {
        egui_focus_manager_clear_focus();
    }
#else
    EGUI_UNUSED(self);
    EGUI_UNUSED(is_inside);
#endif
}

static void egui_view_repeat_button_tick(egui_timer_t *timer)
{
    egui_view_t *self = (egui_view_t *)timer->user_data;
    egui_view_repeat_button_t *local = egui_view_repeat_button_local(self);

    if (!self->is_attached_to_window || !egui_view_get_enable(self) || !egui_view_get_pressed(self) || (!local->touch_active && !local->key_active))
    {
        egui_view_repeat_button_stop_timer(self);
        return;
    }

    egui_view_perform_click(self);
}

void egui_view_repeat_button_apply_standard_style(egui_view_t *self)
{
    egui_view_repeat_button_apply_style(self, EGUI_BG_OF(&repeat_button_standard_background), EGUI_COLOR_WHITE, 6, 12);
}

void egui_view_repeat_button_apply_compact_style(egui_view_t *self)
{
    egui_view_repeat_button_apply_style(self, EGUI_BG_OF(&repeat_button_compact_background), EGUI_COLOR_WHITE, 4, 10);
}

void egui_view_repeat_button_apply_disabled_style(egui_view_t *self)
{
    egui_view_repeat_button_apply_style(self, EGUI_BG_OF(&repeat_button_disabled_background), EGUI_COLOR_HEX(0x708090), 4, 10);
}

void egui_view_repeat_button_set_text(egui_view_t *self, const char *text)
{
    egui_view_repeat_button_clear_pressed_state(self);
    egui_view_label_set_text(self, text != NULL ? text : "");
}

void egui_view_repeat_button_set_icon(egui_view_t *self, const char *icon)
{
    egui_view_repeat_button_clear_pressed_state(self);
    egui_view_button_set_icon(self, icon);
}

void egui_view_repeat_button_set_font(egui_view_t *self, const egui_font_t *font)
{
    egui_view_repeat_button_clear_pressed_state(self);
    egui_view_label_set_font(self, font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
}

void egui_view_repeat_button_set_icon_font(egui_view_t *self, const egui_font_t *font)
{
    egui_view_repeat_button_clear_pressed_state(self);
    egui_view_button_set_icon_font(self, font);
}

void egui_view_repeat_button_set_icon_text_gap(egui_view_t *self, egui_dim_t gap)
{
    egui_view_repeat_button_clear_pressed_state(self);
    egui_view_button_set_icon_text_gap(self, gap);
}

void egui_view_repeat_button_set_repeat_timing(egui_view_t *self, uint16_t initial_delay_ms, uint16_t repeat_interval_ms)
{
    egui_view_repeat_button_t *local = egui_view_repeat_button_local(self);
    uint8_t had_state = egui_view_repeat_button_clear_pressed_state(self);

    local->initial_delay_ms = egui_view_repeat_button_resolve_delay(initial_delay_ms);
    local->repeat_interval_ms = egui_view_repeat_button_resolve_interval(repeat_interval_ms);
    if (!had_state)
    {
        egui_view_invalidate(self);
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_repeat_button_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_repeat_button_clear_pressed_state(self);
    return 1;
}

static int egui_view_repeat_button_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    egui_view_repeat_button_t *local = egui_view_repeat_button_local(self);
    uint8_t is_inside = egui_region_pt_in_rect(&self->region_screen, event->location.x, event->location.y) ? 1 : 0;

    if (!egui_view_get_enable(self))
    {
        egui_view_repeat_button_clear_pressed_state(self);
        return egui_view_get_clickable(self);
    }

    if (!egui_view_get_clickable(self))
    {
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        local->key_active = 0;
        local->touch_active = is_inside ? 1 : 0;
        egui_view_set_pressed(self, is_inside);
        egui_view_repeat_button_stop_timer(self);
        if (is_inside)
        {
            egui_view_repeat_button_apply_focus_from_touch(self, 1);
            egui_view_perform_click(self);
            egui_view_repeat_button_start_timer(self);
        }
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (!local->touch_active)
        {
            egui_view_set_pressed(self, 0);
            egui_view_repeat_button_stop_timer(self);
            return 1;
        }
        if (is_inside)
        {
            if (!egui_view_get_pressed(self))
            {
                egui_view_set_pressed(self, 1);
                egui_view_repeat_button_start_timer(self);
            }
        }
        else
        {
            egui_view_set_pressed(self, 0);
            egui_view_repeat_button_stop_timer(self);
        }
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        local->touch_active = 0;
        egui_view_set_pressed(self, 0);
        egui_view_repeat_button_stop_timer(self);
        return 1;
    default:
        return 1;
    }
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_repeat_button_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_repeat_button_clear_pressed_state(self);
    return 1;
}

static int egui_view_repeat_button_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    egui_view_repeat_button_t *local = egui_view_repeat_button_local(self);

    if (!egui_view_get_enable(self))
    {
        egui_view_repeat_button_clear_pressed_state(self);
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_ENTER:
    case EGUI_KEY_CODE_SPACE:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            if (!local->key_active)
            {
                local->touch_active = 0;
                local->key_active = 1;
                egui_view_set_pressed(self, 1);
                egui_view_perform_click(self);
                egui_view_repeat_button_start_timer(self);
            }
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            local->key_active = 0;
            egui_view_set_pressed(self, 0);
            egui_view_repeat_button_stop_timer(self);
            return 1;
        }
        return 0;
    default:
        egui_view_repeat_button_clear_pressed_state(self);
        return egui_view_on_key_event(self, event);
    }
}
#endif

static void egui_view_repeat_button_on_attach_to_window(egui_view_t *self)
{
    egui_view_on_attach_to_window(self);
    egui_view_repeat_button_start_timer(self);
}

static void egui_view_repeat_button_on_detach_from_window(egui_view_t *self)
{
    egui_view_repeat_button_stop_timer(self);
    egui_view_on_detach_from_window(self);
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_repeat_button_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_repeat_button_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_repeat_button_on_attach_to_window,
        .on_draw = egui_view_button_on_draw,
        .on_detach_from_window = egui_view_repeat_button_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_repeat_button_on_key_event,
#endif
};

void egui_view_repeat_button_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_repeat_button_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_repeat_button_on_static_key_event;
#endif
}

void egui_view_repeat_button_init(egui_view_t *self)
{
    egui_view_repeat_button_t *local = egui_view_repeat_button_local(self);

    egui_view_button_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_repeat_button_t);

    egui_timer_init_timer(&local->repeat_timer, self, egui_view_repeat_button_tick);
    local->initial_delay_ms = EGUI_VIEW_REPEAT_BUTTON_DELAY_DEFAULT;
    local->repeat_interval_ms = EGUI_VIEW_REPEAT_BUTTON_INTERVAL_DEFAULT;
    local->timer_started = 0;
    local->touch_active = 0;
    local->key_active = 0;

    egui_view_repeat_button_apply_standard_style(self);
    egui_view_set_view_name(self, "egui_view_repeat_button");
}
