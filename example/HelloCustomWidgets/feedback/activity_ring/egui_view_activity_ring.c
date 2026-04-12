#include "egui.h"
#include "egui_view_activity_ring.h"

static egui_view_activity_ring_t *hcw_activity_ring_local(egui_view_t *self)
{
    return (egui_view_activity_ring_t *)self;
}

static uint8_t hcw_activity_ring_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static void hcw_activity_ring_apply_style(egui_view_t *self, egui_dim_t stroke_width, egui_color_t ring_color, egui_color_t ring_bg_color)
{
    egui_view_activity_ring_t *local = hcw_activity_ring_local(self);

    hcw_activity_ring_clear_pressed_state(self);
    egui_view_set_shadow(self, NULL);
    egui_view_set_background(self, NULL);
    egui_view_set_enable(self, 1);
    self->padding.left = 0;
    self->padding.right = 0;
    self->padding.top = 0;
    self->padding.bottom = 0;
    local->ring_count = 1;
    local->stroke_width = stroke_width;
    local->ring_gap = 0;
    local->start_angle = -90;
    local->show_round_cap = 1;
    local->ring_colors[0] = ring_color;
    local->ring_bg_colors[0] = ring_bg_color;
    local->values[1] = 0;
    local->values[2] = 0;
    egui_view_invalidate(self);
}

void hcw_activity_ring_apply_standard_style(egui_view_t *self)
{
    hcw_activity_ring_apply_style(self, 8, EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xD8E1EA));
}

void hcw_activity_ring_apply_compact_style(egui_view_t *self)
{
    hcw_activity_ring_apply_style(self, 6, EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xD8E1EA));
}

void hcw_activity_ring_apply_paused_style(egui_view_t *self)
{
    hcw_activity_ring_apply_style(self, 6, EGUI_COLOR_HEX(0xB95A00), EGUI_COLOR_HEX(0xE7DDCA));
}

void hcw_activity_ring_set_value(egui_view_t *self, uint8_t value)
{
    hcw_activity_ring_clear_pressed_state(self);
    egui_view_activity_ring_set_value(self, 0, value);
}

void hcw_activity_ring_set_palette(egui_view_t *self, egui_color_t ring_color, egui_color_t ring_bg_color)
{
    egui_view_activity_ring_t *local = hcw_activity_ring_local(self);

    hcw_activity_ring_clear_pressed_state(self);
    local->ring_colors[0] = ring_color;
    local->ring_bg_colors[0] = ring_bg_color;
    egui_view_invalidate(self);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_activity_ring_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_activity_ring_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_activity_ring_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_activity_ring_clear_pressed_state(self);
    return 1;
}
#endif

void hcw_activity_ring_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_activity_ring_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_activity_ring_on_static_key_event;
#endif
}
