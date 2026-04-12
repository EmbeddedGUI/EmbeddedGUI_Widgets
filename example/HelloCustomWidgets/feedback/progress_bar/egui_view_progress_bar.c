#include "egui.h"
#include "egui_view_progress_bar.h"

static egui_view_progress_bar_t *hcw_progress_bar_local(egui_view_t *self)
{
    return (egui_view_progress_bar_t *)self;
}

static uint8_t hcw_progress_bar_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static void hcw_progress_bar_apply_style(egui_view_t *self, egui_color_t track_color, egui_color_t fill_color, egui_color_t control_color)
{
    egui_view_progress_bar_t *local = hcw_progress_bar_local(self);

    hcw_progress_bar_clear_pressed_state(self);
    egui_view_set_shadow(self, NULL);
    egui_view_set_background(self, NULL);
    egui_view_set_enable(self, 1);
    self->padding.left = 0;
    self->padding.right = 0;
    self->padding.top = 0;
    self->padding.bottom = 0;
    local->bk_color = track_color;
    local->progress_color = fill_color;
    local->control_color = control_color;
    local->is_show_control = 0;
    egui_view_invalidate(self);
}

void hcw_progress_bar_apply_standard_style(egui_view_t *self)
{
    hcw_progress_bar_apply_style(self, EGUI_COLOR_HEX(0xD8E1EA), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F6CBD));
}

void hcw_progress_bar_apply_paused_style(egui_view_t *self)
{
    hcw_progress_bar_apply_style(self, EGUI_COLOR_HEX(0xE7DDCA), EGUI_COLOR_HEX(0xB95A00), EGUI_COLOR_HEX(0xB95A00));
}

void hcw_progress_bar_apply_error_style(egui_view_t *self)
{
    hcw_progress_bar_apply_style(self, EGUI_COLOR_HEX(0xEED6DA), EGUI_COLOR_HEX(0xC42B1C), EGUI_COLOR_HEX(0xC42B1C));
}

void hcw_progress_bar_set_value(egui_view_t *self, uint8_t value)
{
    hcw_progress_bar_clear_pressed_state(self);
    egui_view_progress_bar_set_process(self, value);
}

void hcw_progress_bar_set_palette(egui_view_t *self, egui_color_t track_color, egui_color_t fill_color, egui_color_t control_color)
{
    egui_view_progress_bar_t *local = hcw_progress_bar_local(self);

    hcw_progress_bar_clear_pressed_state(self);
    local->bk_color = track_color;
    local->progress_color = fill_color;
    local->control_color = control_color;
    egui_view_invalidate(self);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_progress_bar_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_progress_bar_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_progress_bar_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_progress_bar_clear_pressed_state(self);
    return 1;
}
#endif

void hcw_progress_bar_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_progress_bar_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_progress_bar_on_static_key_event;
#endif
}
