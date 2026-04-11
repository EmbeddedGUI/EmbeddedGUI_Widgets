#include "egui_view_toggle_button.h"

static egui_view_toggle_button_t *hcw_toggle_button_local(egui_view_t *self)
{
    return (egui_view_toggle_button_t *)self;
}

static uint8_t hcw_toggle_button_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static void hcw_toggle_button_apply_style(egui_view_t *self, egui_dim_t radius, egui_color_t on_color, egui_color_t off_color, egui_color_t text_color,
                                          egui_dim_t gap)
{
    egui_view_toggle_button_t *local = hcw_toggle_button_local(self);
    uint8_t had_pressed = hcw_toggle_button_clear_pressed_state(self);

    local->corner_radius = radius;
    local->on_color = on_color;
    local->off_color = off_color;
    local->text_color = text_color;
    local->icon_text_gap = gap;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_toggle_button_apply_standard_style(egui_view_t *self)
{
    hcw_toggle_button_apply_style(self, 10, EGUI_COLOR_HEX(0x2563EB), EGUI_COLOR_HEX(0xEAF1FB), EGUI_COLOR_WHITE, 5);
}

void hcw_toggle_button_apply_compact_style(egui_view_t *self)
{
    hcw_toggle_button_apply_style(self, 7, EGUI_COLOR_HEX(0x0C7C73), EGUI_COLOR_HEX(0xDBEAE5), EGUI_COLOR_WHITE, 3);
}

void hcw_toggle_button_apply_read_only_style(egui_view_t *self)
{
    hcw_toggle_button_apply_style(self, 7, EGUI_COLOR_HEX(0xAFB8C3), EGUI_COLOR_HEX(0xF3F6F8), EGUI_COLOR_HEX(0xF7F9FB), 3);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_toggle_button_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    egui_view_toggle_button_t *local = hcw_toggle_button_local(self);
    int is_inside = egui_region_pt_in_rect(&self->region_screen, event->location.x, event->location.y);
    int was_pressed;

    if (!egui_view_get_enable(self))
    {
        hcw_toggle_button_clear_pressed_state(self);
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        if (!is_inside)
        {
            hcw_toggle_button_clear_pressed_state(self);
            return 0;
        }
        egui_view_set_pressed(self, 1);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
        else if (!self->is_no_focus_clear)
        {
            egui_focus_manager_clear_focus();
        }
#endif
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (!egui_view_get_pressed(self) && !is_inside)
        {
            return 0;
        }
        egui_view_set_pressed(self, is_inside);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
        was_pressed = egui_view_get_pressed(self);
        hcw_toggle_button_clear_pressed_state(self);
        if (was_pressed && is_inside)
        {
            hcw_toggle_button_set_toggled(self, !local->is_toggled);
            return 1;
        }
        return was_pressed || is_inside;
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        return hcw_toggle_button_clear_pressed_state(self);
    default:
        return 0;
    }
}

static int hcw_toggle_button_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_toggle_button_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_toggle_button_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_toggle_button_clear_pressed_state(self);
    return 1;
}
#endif

void hcw_toggle_button_set_toggled(egui_view_t *self, uint8_t is_toggled)
{
    hcw_toggle_button_clear_pressed_state(self);
    egui_view_toggle_button_set_toggled(self, is_toggled ? 1 : 0);
}

void hcw_toggle_button_override_interaction_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_toggle_button_on_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_toggle_button_on_key_event;
#endif
}

void hcw_toggle_button_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_toggle_button_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_toggle_button_on_static_key_event;
#endif
}

int hcw_toggle_button_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    uint8_t was_pressed = hcw_toggle_button_clear_pressed_state(self);

    if (!egui_view_get_enable(self))
    {
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_ENTER:
    case EGUI_KEY_CODE_SPACE:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            egui_view_set_pressed(self, 1);
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            if (was_pressed)
            {
                hcw_toggle_button_set_toggled(self, !egui_view_toggle_button_is_toggled(self));
            }
            return 1;
        }
        return 0;
    default:
        return egui_view_on_key_event(self, event);
    }
}
