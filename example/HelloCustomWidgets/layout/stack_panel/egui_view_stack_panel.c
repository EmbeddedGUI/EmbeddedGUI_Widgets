#include "egui_view_stack_panel.h"

static void hcw_stack_panel_clear_pressed_state(egui_view_t *self)
{
    egui_view_set_pressed(self, 0);
}

static void hcw_stack_panel_prepare_base(egui_view_t *self)
{
    hcw_stack_panel_clear_pressed_state(self);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding(self, 0, 0, 0, 0);
    egui_view_linearlayout_set_auto_width(self, 0);
    egui_view_linearlayout_set_auto_height(self, 0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif
}

static void hcw_stack_panel_apply_style(egui_view_t *self, uint8_t is_horizontal, uint8_t align_type)
{
    hcw_stack_panel_prepare_base(self);
    egui_view_linearlayout_set_orientation(self, is_horizontal ? 1 : 0);
    egui_view_linearlayout_set_align_type(self, align_type);
    egui_view_invalidate(self);
}

void hcw_stack_panel_apply_standard_style(egui_view_t *self)
{
    hcw_stack_panel_apply_style(self, 0, EGUI_ALIGN_HCENTER);
}

void hcw_stack_panel_apply_horizontal_style(egui_view_t *self)
{
    hcw_stack_panel_apply_style(self, 1, EGUI_ALIGN_VCENTER);
}

void hcw_stack_panel_apply_compact_style(egui_view_t *self)
{
    hcw_stack_panel_apply_style(self, 0, EGUI_ALIGN_LEFT);
}

void hcw_stack_panel_set_orientation(egui_view_t *self, uint8_t is_horizontal)
{
    EGUI_LOCAL_INIT(egui_view_linearlayout_t);

    hcw_stack_panel_prepare_base(self);
    local->is_orientation_horizontal = is_horizontal ? 1 : 0;
    egui_view_invalidate(self);
}

void hcw_stack_panel_set_align_type(egui_view_t *self, uint8_t align_type)
{
    EGUI_LOCAL_INIT(egui_view_linearlayout_t);

    hcw_stack_panel_prepare_base(self);
    local->align_type = align_type;
    egui_view_invalidate(self);
}

void hcw_stack_panel_layout_childs(egui_view_t *self)
{
    hcw_stack_panel_clear_pressed_state(self);
    egui_view_linearlayout_layout_childs(self);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_stack_panel_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_stack_panel_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_stack_panel_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_stack_panel_clear_pressed_state(self);
    return 1;
}
#endif

void hcw_stack_panel_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_stack_panel_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_stack_panel_on_static_key_event;
#endif
}
