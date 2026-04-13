#include "egui_view_dock_panel.h"

static void hcw_dock_panel_clear_pressed_state(egui_view_t *self)
{
    egui_view_set_pressed(self, 0);
}

static hcw_dock_panel_t *hcw_dock_panel_get_local(egui_view_t *self)
{
    return (hcw_dock_panel_t *)self;
}

static void hcw_dock_panel_prepare_base(egui_view_t *self, egui_dim_t inset)
{
    hcw_dock_panel_t *local = hcw_dock_panel_get_local(self);

    hcw_dock_panel_clear_pressed_state(self);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    local->content_inset = inset;
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif
}

static egui_dim_t hcw_dock_panel_child_get_desired_width(egui_view_t *child)
{
    return child->margin.right > 0 ? child->margin.right : child->region.size.width;
}

static egui_dim_t hcw_dock_panel_child_get_desired_height(egui_view_t *child)
{
    return child->margin.top > 0 ? child->margin.top : child->region.size.height;
}

static uint8_t hcw_dock_panel_child_get_dock(egui_view_t *child)
{
    if (child->margin.left > HCW_DOCK_PANEL_DOCK_FILL)
    {
        return HCW_DOCK_PANEL_DOCK_FILL;
    }
    return (uint8_t)child->margin.left;
}

void hcw_dock_panel_init(egui_view_t *self)
{
    hcw_dock_panel_t *local = hcw_dock_panel_get_local(self);

    egui_view_group_init(self);
    local->last_child_fill = 1;
    local->content_inset = 4;
    egui_view_set_view_name(self, "hcw_dock_panel");
}

void hcw_dock_panel_apply_standard_style(egui_view_t *self)
{
    hcw_dock_panel_prepare_base(self, 4);
}

void hcw_dock_panel_apply_compact_style(egui_view_t *self)
{
    hcw_dock_panel_prepare_base(self, 2);
}

void hcw_dock_panel_set_last_child_fill(egui_view_t *self, uint8_t last_child_fill)
{
    hcw_dock_panel_t *local = hcw_dock_panel_get_local(self);

    hcw_dock_panel_clear_pressed_state(self);
    local->last_child_fill = last_child_fill ? 1 : 0;
    egui_view_invalidate(self);
}

void hcw_dock_panel_set_child_dock(egui_view_t *child, uint8_t dock_type)
{
    if (dock_type > HCW_DOCK_PANEL_DOCK_FILL)
    {
        dock_type = HCW_DOCK_PANEL_DOCK_FILL;
    }
    egui_view_set_pressed(child, 0);
    child->margin.left = dock_type;
    child->margin.right = child->region.size.width;
    child->margin.top = child->region.size.height;
    egui_view_invalidate(child);
}

void hcw_dock_panel_layout_childs(egui_view_t *self)
{
    hcw_dock_panel_t *local = hcw_dock_panel_get_local(self);
    egui_view_group_t *group = (egui_view_group_t *)self;
    egui_dnode_t *node;
    egui_view_t *child;
    egui_view_t *last_visible_child = NULL;
    egui_dim_t left = local->content_inset;
    egui_dim_t top = local->content_inset;
    egui_dim_t right = self->region.size.width - local->content_inset;
    egui_dim_t bottom = self->region.size.height - local->content_inset;

    hcw_dock_panel_clear_pressed_state(self);

    EGUI_DLIST_FOR_EACH_NODE(&group->childs, node)
    {
        child = EGUI_DLIST_ENTRY(node, egui_view_t, node);
        if (!child->is_gone)
        {
            last_visible_child = child;
        }
    }

    EGUI_DLIST_FOR_EACH_NODE(&group->childs, node)
    {
        egui_dim_t width;
        egui_dim_t height;
        uint8_t dock_type;

        child = EGUI_DLIST_ENTRY(node, egui_view_t, node);
        if (child->is_gone)
        {
            continue;
        }

        dock_type = hcw_dock_panel_child_get_dock(child);
        if (local->last_child_fill && child == last_visible_child)
        {
            dock_type = HCW_DOCK_PANEL_DOCK_FILL;
        }

        if (right < left)
        {
            right = left;
        }
        if (bottom < top)
        {
            bottom = top;
        }

        width = right - left;
        height = bottom - top;

        switch (dock_type)
        {
        case HCW_DOCK_PANEL_DOCK_TOP:
        {
            egui_dim_t dock_height = hcw_dock_panel_child_get_desired_height(child);

            if (dock_height > height)
            {
                dock_height = height;
            }
            egui_view_set_position(child, left, top);
            egui_view_set_size(child, width, dock_height);
            top += dock_height;
            break;
        }
        case HCW_DOCK_PANEL_DOCK_RIGHT:
        {
            egui_dim_t dock_width = hcw_dock_panel_child_get_desired_width(child);

            if (dock_width > width)
            {
                dock_width = width;
            }
            egui_view_set_position(child, right - dock_width, top);
            egui_view_set_size(child, dock_width, height);
            right -= dock_width;
            break;
        }
        case HCW_DOCK_PANEL_DOCK_BOTTOM:
        {
            egui_dim_t dock_height = hcw_dock_panel_child_get_desired_height(child);

            if (dock_height > height)
            {
                dock_height = height;
            }
            egui_view_set_position(child, left, bottom - dock_height);
            egui_view_set_size(child, width, dock_height);
            bottom -= dock_height;
            break;
        }
        case HCW_DOCK_PANEL_DOCK_FILL:
            egui_view_set_position(child, left, top);
            egui_view_set_size(child, width, height);
            break;
        case HCW_DOCK_PANEL_DOCK_LEFT:
        default:
        {
            egui_dim_t dock_width = hcw_dock_panel_child_get_desired_width(child);

            if (dock_width > width)
            {
                dock_width = width;
            }
            egui_view_set_position(child, left, top);
            egui_view_set_size(child, dock_width, height);
            left += dock_width;
            break;
        }
        }
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_dock_panel_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_dock_panel_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_dock_panel_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_dock_panel_clear_pressed_state(self);
    return 1;
}
#endif

void hcw_dock_panel_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_dock_panel_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_dock_panel_on_static_key_event;
#endif
}
