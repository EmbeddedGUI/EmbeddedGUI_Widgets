#include "egui_view_grid.h"

static void hcw_grid_clear_pressed_state(egui_view_t *self)
{
    egui_view_set_pressed(self, 0);
}

static void hcw_grid_prepare_base(egui_view_t *self)
{
    hcw_grid_clear_pressed_state(self);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding(self, 0, 0, 0, 0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif
}

static void hcw_grid_apply_columns(egui_view_t *self, uint8_t columns)
{
    EGUI_LOCAL_INIT(egui_view_gridlayout_t);

    hcw_grid_prepare_base(self);
    if (columns < 1)
    {
        columns = 1;
    }
    if (columns > 3)
    {
        columns = 3;
    }
    local->col_count = columns;
    local->align_type = EGUI_ALIGN_HCENTER;
    egui_view_invalidate(self);
}

void hcw_grid_apply_standard_style(egui_view_t *self)
{
    hcw_grid_apply_columns(self, 2);
}

void hcw_grid_apply_dense_style(egui_view_t *self)
{
    hcw_grid_apply_columns(self, 3);
}

void hcw_grid_apply_stack_style(egui_view_t *self)
{
    hcw_grid_apply_columns(self, 1);
}

void hcw_grid_set_columns(egui_view_t *self, uint8_t columns)
{
    hcw_grid_apply_columns(self, columns);
}

void hcw_grid_set_align_type(egui_view_t *self, uint8_t align_type)
{
    EGUI_LOCAL_INIT(egui_view_gridlayout_t);

    hcw_grid_prepare_base(self);
    local->align_type = align_type;
    egui_view_invalidate(self);
}

void hcw_grid_layout_childs(egui_view_t *self)
{
    hcw_grid_clear_pressed_state(self);
    egui_view_gridlayout_layout_childs(self);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_grid_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_grid_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_grid_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_grid_clear_pressed_state(self);
    return 1;
}
#endif

void hcw_grid_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_grid_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_grid_on_static_key_event;
#endif
}
