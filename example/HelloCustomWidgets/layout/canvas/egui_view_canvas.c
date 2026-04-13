#include "egui_view_canvas.h"

static void hcw_canvas_clear_pressed_state(egui_view_t *self)
{
    egui_view_set_pressed(self, 0);
}

static void hcw_canvas_prepare_base(egui_view_t *self, egui_dim_margin_padding_t padding)
{
    EGUI_UNUSED(padding);
    hcw_canvas_clear_pressed_state(self);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif
}

void hcw_canvas_apply_standard_style(egui_view_t *self)
{
    hcw_canvas_prepare_base(self, 4);
}

void hcw_canvas_apply_compact_style(egui_view_t *self)
{
    hcw_canvas_prepare_base(self, 2);
}

void hcw_canvas_set_child_origin(egui_view_t *child, egui_dim_t x, egui_dim_t y)
{
    egui_view_set_pressed(child, 0);
    egui_view_set_position(child, x, y);
}

void hcw_canvas_layout_childs(egui_view_t *self)
{
    hcw_canvas_clear_pressed_state(self);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_canvas_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_canvas_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_canvas_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_canvas_clear_pressed_state(self);
    return 1;
}
#endif

void hcw_canvas_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_canvas_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_canvas_on_static_key_event;
#endif
}
