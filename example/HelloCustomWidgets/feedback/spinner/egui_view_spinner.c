#include "egui_view_spinner.h"

static uint8_t hcw_spinner_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static void hcw_spinner_prepare_base(egui_view_t *self)
{
    hcw_spinner_clear_pressed_state(self);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif
}

void hcw_spinner_apply_standard_style(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_spinner_t);

    hcw_spinner_prepare_base(self);
    local->stroke_width = 4;
    local->arc_length = 104;
    local->rotation_angle = -90;
    local->color = EGUI_COLOR_HEX(0x0F6CBD);
    egui_view_spinner_start(self);
    egui_view_invalidate(self);
}

void hcw_spinner_apply_compact_style(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_spinner_t);

    hcw_spinner_prepare_base(self);
    local->stroke_width = 3;
    local->arc_length = 84;
    local->rotation_angle = -90;
    local->color = EGUI_COLOR_HEX(0x0F6CBD);
    egui_view_spinner_start(self);
    egui_view_invalidate(self);
}

void hcw_spinner_apply_muted_style(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_spinner_t);

    hcw_spinner_prepare_base(self);
    local->stroke_width = 3;
    local->arc_length = 76;
    local->rotation_angle = -90;
    local->color = EGUI_COLOR_HEX(0x8A96A2);
    egui_view_spinner_start(self);
    egui_view_invalidate(self);
}

void hcw_spinner_set_palette(egui_view_t *self, egui_color_t color)
{
    hcw_spinner_prepare_base(self);
    egui_view_spinner_set_color(self, color);
}

void hcw_spinner_set_stroke_width(egui_view_t *self, egui_dim_t stroke_width)
{
    EGUI_LOCAL_INIT(egui_view_spinner_t);

    hcw_spinner_prepare_base(self);
    if (stroke_width < 2)
    {
        stroke_width = 2;
    }
    local->stroke_width = stroke_width;
    egui_view_invalidate(self);
}

void hcw_spinner_set_arc_length(egui_view_t *self, int16_t arc_length)
{
    EGUI_LOCAL_INIT(egui_view_spinner_t);

    hcw_spinner_prepare_base(self);
    if (arc_length < 24)
    {
        arc_length = 24;
    }
    if (arc_length > 300)
    {
        arc_length = 300;
    }
    local->arc_length = arc_length;
    egui_view_invalidate(self);
}

void hcw_spinner_set_rotation_angle(egui_view_t *self, int16_t rotation_angle)
{
    EGUI_LOCAL_INIT(egui_view_spinner_t);

    hcw_spinner_prepare_base(self);
    rotation_angle %= 360;
    if (rotation_angle < 0)
    {
        rotation_angle += 360;
    }
    local->rotation_angle = rotation_angle;
    egui_view_invalidate(self);
}

void hcw_spinner_set_spinning(egui_view_t *self, uint8_t spinning)
{
    hcw_spinner_prepare_base(self);
    if (spinning)
    {
        egui_view_spinner_start(self);
    }
    else
    {
        egui_view_spinner_stop(self);
    }
    egui_view_invalidate(self);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_spinner_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_spinner_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_spinner_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_spinner_clear_pressed_state(self);
    return 1;
}
#endif

void hcw_spinner_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_spinner_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_spinner_on_static_key_event;
#endif
}
