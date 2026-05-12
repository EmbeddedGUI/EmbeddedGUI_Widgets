#include "egui_view_items_control.h"

#define EGUI_VIEW_ITEMS_CONTROL_RADIUS_MAX 18
#define EGUI_VIEW_ITEMS_CONTROL_WIDTH_MAX  3
#define EGUI_VIEW_ITEMS_CONTROL_GAP_MAX    18

static egui_view_items_control_t *egui_view_items_control_local(egui_view_t *self)
{
    return (egui_view_items_control_t *)self;
}

static uint8_t egui_view_items_control_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static egui_color_t egui_view_items_control_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_MAKE(38));
}

static egui_dim_t egui_view_items_control_clamp_radius(egui_dim_t radius)
{
    if (radius < 0)
    {
        return 0;
    }
    if (radius > EGUI_VIEW_ITEMS_CONTROL_RADIUS_MAX)
    {
        return EGUI_VIEW_ITEMS_CONTROL_RADIUS_MAX;
    }
    return radius;
}

static egui_dim_t egui_view_items_control_clamp_width(egui_dim_t width)
{
    if (width < 0)
    {
        return 0;
    }
    if (width > EGUI_VIEW_ITEMS_CONTROL_WIDTH_MAX)
    {
        return EGUI_VIEW_ITEMS_CONTROL_WIDTH_MAX;
    }
    return width;
}

static egui_dim_t egui_view_items_control_clamp_gap(egui_dim_t gap)
{
    if (gap < 0)
    {
        return 0;
    }
    if (gap > EGUI_VIEW_ITEMS_CONTROL_GAP_MAX)
    {
        return EGUI_VIEW_ITEMS_CONTROL_GAP_MAX;
    }
    return gap;
}

static uint8_t egui_view_items_control_normalize_layout(uint8_t layout_mode)
{
    if (layout_mode > EGUI_VIEW_ITEMS_CONTROL_LAYOUT_WRAP)
    {
        return EGUI_VIEW_ITEMS_CONTROL_LAYOUT_VERTICAL;
    }
    return layout_mode;
}

static egui_dim_t egui_view_items_control_align_x(egui_dim_t parent_width, egui_dim_t child_width, uint8_t align_type)
{
    egui_dim_t free_width = parent_width - child_width;

    if (free_width < 0)
    {
        free_width = 0;
    }
    if (align_type & EGUI_ALIGN_RIGHT)
    {
        return free_width;
    }
    if (align_type & EGUI_ALIGN_HCENTER)
    {
        return free_width / 2;
    }
    return 0;
}

static egui_dim_t egui_view_items_control_align_y(egui_dim_t parent_height, egui_dim_t child_height, uint8_t align_type)
{
    egui_dim_t free_height = parent_height - child_height;

    if (free_height < 0)
    {
        free_height = 0;
    }
    if (align_type & EGUI_ALIGN_BOTTOM)
    {
        return free_height;
    }
    if (align_type & EGUI_ALIGN_VCENTER)
    {
        return free_height / 2;
    }
    return 0;
}

static egui_dim_t egui_view_items_control_child_width(egui_view_t *child)
{
    return child->region.size.width + child->margin.left + child->margin.right;
}

static egui_dim_t egui_view_items_control_child_height(egui_view_t *child)
{
    return child->region.size.height + child->margin.top + child->margin.bottom;
}

static void egui_view_items_control_constrain_child_backdrop(egui_region_t *item_region, const egui_region_t *container_region, egui_dim_t inset)
{
    egui_dim_t min_x;
    egui_dim_t min_y;
    egui_dim_t max_right;
    egui_dim_t max_bottom;

    if (item_region == NULL || container_region == NULL)
    {
        return;
    }

    if (inset < 0)
    {
        inset = 0;
    }
    min_x = container_region->location.x + inset;
    min_y = container_region->location.y + inset;
    max_right = container_region->location.x + container_region->size.width - inset;
    max_bottom = container_region->location.y + container_region->size.height - inset;

    if (item_region->location.x < min_x)
    {
        item_region->size.width -= min_x - item_region->location.x;
        item_region->location.x = min_x;
    }
    if (item_region->location.y < min_y)
    {
        item_region->size.height -= min_y - item_region->location.y;
        item_region->location.y = min_y;
    }
    if (item_region->location.x + item_region->size.width > max_right)
    {
        item_region->size.width = max_right - item_region->location.x;
    }
    if (item_region->location.y + item_region->size.height > max_bottom)
    {
        item_region->size.height = max_bottom - item_region->location.y;
    }
    if (item_region->size.width < 0)
    {
        item_region->size.width = 0;
    }
    if (item_region->size.height < 0)
    {
        item_region->size.height = 0;
    }
}

static void egui_view_items_control_get_linear_size(egui_view_t *self, uint8_t horizontal, egui_dim_t gap, egui_dim_t *width, egui_dim_t *height)
{
    egui_view_items_control_t *local = egui_view_items_control_local(self);
    egui_dnode_t *node;
    egui_view_t *child;
    egui_dim_t total_width = 0;
    egui_dim_t total_height = 0;
    egui_dim_t max_width = 0;
    egui_dim_t max_height = 0;
    uint8_t visible_count = 0;

    EGUI_DLIST_FOR_EACH_NODE(&local->base.childs, node)
    {
        child = EGUI_DLIST_ENTRY(node, egui_view_t, node);
        if (child->is_gone)
        {
            continue;
        }
        visible_count++;
        if (horizontal)
        {
            total_width += egui_view_items_control_child_width(child);
            if (egui_view_items_control_child_height(child) > max_height)
            {
                max_height = egui_view_items_control_child_height(child);
            }
        }
        else
        {
            total_height += egui_view_items_control_child_height(child);
            if (egui_view_items_control_child_width(child) > max_width)
            {
                max_width = egui_view_items_control_child_width(child);
            }
        }
    }

    if (visible_count > 1)
    {
        if (horizontal)
        {
            total_width += (visible_count - 1) * gap;
        }
        else
        {
            total_height += (visible_count - 1) * gap;
        }
    }

    *width = horizontal ? total_width : max_width;
    *height = horizontal ? max_height : total_height;
}

static void egui_view_items_control_on_draw(egui_view_t *self)
{
    egui_view_items_control_t *local = egui_view_items_control_local(self);
    egui_region_t region;
    egui_dnode_t *node;
    egui_view_t *child;
    egui_color_t surface_color = local->surface_color;
    egui_color_t border_color = local->border_color;
    egui_color_t item_surface_color = local->item_surface_color;
    egui_dim_t radius = local->corner_radius;
    egui_alpha_t border_alpha = EGUI_ALPHA_100;
    egui_alpha_t item_alpha = EGUI_ALPHA_MAKE(local->compact_mode ? 76 : 86);

    region.location.x = 0;
    region.location.y = 0;
    region.size.width = self->region.size.width;
    region.size.height = self->region.size.height;
    if (region.size.width <= 0 || region.size.height <= 0)
    {
        return;
    }

    if (local->compact_mode && radius > 6)
    {
        radius = 6;
    }
    if (local->read_only_mode)
    {
        surface_color = egui_rgb_mix(surface_color, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(34));
        border_color = egui_rgb_mix(border_color, HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_MAKE(32));
        item_surface_color = egui_rgb_mix(item_surface_color, HCW_COLOR_SURFACE_DISABLED, EGUI_ALPHA_MAKE(36));
        border_alpha = EGUI_ALPHA_MAKE(96);
        item_alpha = EGUI_ALPHA_MAKE(74);
    }
    if (!egui_view_get_enable(self))
    {
        surface_color = egui_view_items_control_mix_disabled(surface_color);
        border_color = egui_view_items_control_mix_disabled(border_color);
        item_surface_color = egui_view_items_control_mix_disabled(item_surface_color);
        border_alpha = EGUI_ALPHA_MAKE(64);
        item_alpha = EGUI_ALPHA_MAKE(60);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height, radius,
                                          surface_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));

    EGUI_DLIST_FOR_EACH_NODE(&local->base.childs, node)
    {
        egui_region_t item_region;

        child = EGUI_DLIST_ENTRY(node, egui_view_t, node);
        if (child->is_gone || !child->is_visible)
        {
            continue;
        }
        item_region.location.x = child->region.location.x - (local->compact_mode ? 2 : 3);
        item_region.location.y = child->region.location.y - (local->compact_mode ? 1 : 2);
        item_region.size.width = child->region.size.width + (local->compact_mode ? 4 : 6);
        item_region.size.height = child->region.size.height + (local->compact_mode ? 2 : 4);
        egui_view_items_control_constrain_child_backdrop(&item_region, &region, local->border_width);
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, item_region.location.x, item_region.location.y, item_region.size.width,
                                              item_region.size.height, local->compact_mode ? 4 : 6, item_surface_color,
                                              egui_color_alpha_mix(self->alpha, item_alpha));
    }

    if (local->border_width > 0)
    {
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height, radius,
                                         local->border_width, border_color, egui_color_alpha_mix(self->alpha, border_alpha));
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_items_control_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_items_control_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_items_control_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_items_control_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_items_control_add_item(egui_view_t *self, egui_view_t *item)
{
    if (item == NULL)
    {
        return;
    }

    egui_view_group_add_child(self, item);
    egui_view_items_control_layout_items(self);
    egui_view_invalidate(self);
}

void egui_view_items_control_clear_items(egui_view_t *self)
{
    egui_view_items_control_clear_pressed_state(self);
    egui_view_group_clear_childs(self);
    egui_view_invalidate(self);
}

int egui_view_items_control_get_item_count(egui_view_t *self)
{
    return egui_view_group_get_child_count(self);
}

void egui_view_items_control_layout_items(egui_view_t *self)
{
    egui_view_items_control_t *local = egui_view_items_control_local(self);
    egui_region_t work_region;
    egui_dnode_t *node;
    egui_view_t *child;
    egui_dim_t x;
    egui_dim_t y;
    egui_dim_t total_width;
    egui_dim_t total_height;
    egui_dim_t line_height;

    work_region.location.x = local->content_padding_left;
    work_region.location.y = local->content_padding_top;
    work_region.size.width = self->region.size.width - (local->content_padding_left + local->content_padding_right);
    work_region.size.height = self->region.size.height - (local->content_padding_top + local->content_padding_bottom);
    if (work_region.size.width < 0)
    {
        work_region.size.width = 0;
    }
    if (work_region.size.height < 0)
    {
        work_region.size.height = 0;
    }

    if (local->layout_mode == EGUI_VIEW_ITEMS_CONTROL_LAYOUT_HORIZONTAL)
    {
        egui_view_items_control_get_linear_size(self, 1, local->item_gap, &total_width, &total_height);
        x = work_region.location.x + egui_view_items_control_align_x(work_region.size.width, total_width, local->item_align_type);
        y = work_region.location.y + egui_view_items_control_align_y(work_region.size.height, total_height, local->item_align_type);
        EGUI_DLIST_FOR_EACH_NODE(&local->base.childs, node)
        {
            child = EGUI_DLIST_ENTRY(node, egui_view_t, node);
            if (child->is_gone)
            {
                continue;
            }
            egui_view_set_position(child, x + child->margin.left, y + child->margin.top);
            x += egui_view_items_control_child_width(child) + local->item_gap;
        }
        return;
    }

    if (local->layout_mode == EGUI_VIEW_ITEMS_CONTROL_LAYOUT_WRAP)
    {
        x = work_region.location.x;
        y = work_region.location.y;
        line_height = 0;
        EGUI_DLIST_FOR_EACH_NODE(&local->base.childs, node)
        {
            child = EGUI_DLIST_ENTRY(node, egui_view_t, node);
            if (child->is_gone)
            {
                continue;
            }
            if (x > work_region.location.x && x + egui_view_items_control_child_width(child) > work_region.location.x + work_region.size.width)
            {
                x = work_region.location.x;
                y += line_height + local->item_gap;
                line_height = 0;
            }
            egui_view_set_position(child, x + child->margin.left, y + child->margin.top);
            x += egui_view_items_control_child_width(child) + local->item_gap;
            if (egui_view_items_control_child_height(child) > line_height)
            {
                line_height = egui_view_items_control_child_height(child);
            }
        }
        return;
    }

    egui_view_items_control_get_linear_size(self, 0, local->item_gap, &total_width, &total_height);
    x = work_region.location.x + egui_view_items_control_align_x(work_region.size.width, total_width, local->item_align_type);
    y = work_region.location.y + egui_view_items_control_align_y(work_region.size.height, total_height, local->item_align_type);
    EGUI_DLIST_FOR_EACH_NODE(&local->base.childs, node)
    {
        child = EGUI_DLIST_ENTRY(node, egui_view_t, node);
        if (child->is_gone)
        {
            continue;
        }
        egui_view_set_position(child, x + child->margin.left, y + child->margin.top);
        y += egui_view_items_control_child_height(child) + local->item_gap;
    }
}

void egui_view_items_control_set_padding(egui_view_t *self, egui_dim_margin_padding_t left, egui_dim_margin_padding_t right,
                                         egui_dim_margin_padding_t top, egui_dim_margin_padding_t bottom)
{
    egui_view_items_control_t *local = egui_view_items_control_local(self);

    egui_view_items_control_clear_pressed_state(self);
    local->content_padding_left = left;
    local->content_padding_right = right;
    local->content_padding_top = top;
    local->content_padding_bottom = bottom;
    egui_view_items_control_layout_items(self);
    egui_view_invalidate(self);
}

void egui_view_items_control_set_item_gap(egui_view_t *self, egui_dim_t gap)
{
    egui_view_items_control_t *local = egui_view_items_control_local(self);

    egui_view_items_control_clear_pressed_state(self);
    local->item_gap = egui_view_items_control_clamp_gap(gap);
    egui_view_items_control_layout_items(self);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_items_control_get_item_gap(egui_view_t *self)
{
    egui_view_items_control_t *local = egui_view_items_control_local(self);

    return local->item_gap;
}

void egui_view_items_control_set_layout_mode(egui_view_t *self, uint8_t layout_mode)
{
    egui_view_items_control_t *local = egui_view_items_control_local(self);

    egui_view_items_control_clear_pressed_state(self);
    local->layout_mode = egui_view_items_control_normalize_layout(layout_mode);
    egui_view_items_control_layout_items(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_items_control_get_layout_mode(egui_view_t *self)
{
    egui_view_items_control_t *local = egui_view_items_control_local(self);

    return local->layout_mode;
}

void egui_view_items_control_set_item_align_type(egui_view_t *self, uint8_t align_type)
{
    egui_view_items_control_t *local = egui_view_items_control_local(self);

    egui_view_items_control_clear_pressed_state(self);
    local->item_align_type = align_type;
    egui_view_items_control_layout_items(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_items_control_get_item_align_type(egui_view_t *self)
{
    egui_view_items_control_t *local = egui_view_items_control_local(self);

    return local->item_align_type;
}

void egui_view_items_control_set_corner_radius(egui_view_t *self, egui_dim_t radius)
{
    egui_view_items_control_t *local = egui_view_items_control_local(self);

    egui_view_items_control_clear_pressed_state(self);
    local->corner_radius = egui_view_items_control_clamp_radius(radius);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_items_control_get_corner_radius(egui_view_t *self)
{
    egui_view_items_control_t *local = egui_view_items_control_local(self);

    return local->corner_radius;
}

void egui_view_items_control_set_border_width(egui_view_t *self, egui_dim_t width)
{
    egui_view_items_control_t *local = egui_view_items_control_local(self);

    egui_view_items_control_clear_pressed_state(self);
    local->border_width = egui_view_items_control_clamp_width(width);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_items_control_get_border_width(egui_view_t *self)
{
    egui_view_items_control_t *local = egui_view_items_control_local(self);

    return local->border_width;
}

void egui_view_items_control_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color,
                                         egui_color_t item_surface_color, egui_color_t accent_color)
{
    egui_view_items_control_t *local = egui_view_items_control_local(self);

    egui_view_items_control_clear_pressed_state(self);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->item_surface_color = item_surface_color;
    local->accent_color = accent_color;
    egui_view_invalidate(self);
}

void egui_view_items_control_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    egui_view_items_control_t *local = egui_view_items_control_local(self);

    egui_view_items_control_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_items_control_get_compact_mode(egui_view_t *self)
{
    egui_view_items_control_t *local = egui_view_items_control_local(self);

    return local->compact_mode;
}

void egui_view_items_control_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    egui_view_items_control_t *local = egui_view_items_control_local(self);

    egui_view_items_control_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_items_control_get_read_only_mode(egui_view_t *self)
{
    egui_view_items_control_t *local = egui_view_items_control_local(self);

    return local->read_only_mode;
}

void egui_view_items_control_apply_standard_style(egui_view_t *self)
{
    egui_view_items_control_set_palette(self, HCW_COLOR_SURFACE, HCW_COLOR_BORDER_STRONG, HCW_COLOR_SURFACE_PRESS,
                                        HCW_COLOR_PRIMARY);
    egui_view_items_control_set_corner_radius(self, 8);
    egui_view_items_control_set_border_width(self, 1);
    egui_view_items_control_set_item_gap(self, 6);
    egui_view_items_control_set_padding(self, 10, 10, 8, 8);
    egui_view_items_control_set_layout_mode(self, EGUI_VIEW_ITEMS_CONTROL_LAYOUT_VERTICAL);
    egui_view_items_control_set_item_align_type(self, EGUI_ALIGN_TOP_LEFT);
    egui_view_items_control_set_compact_mode(self, 0);
    egui_view_items_control_set_read_only_mode(self, 0);
}

void egui_view_items_control_apply_strip_style(egui_view_t *self)
{
    egui_view_items_control_set_palette(self, HCW_COLOR_PANEL, HCW_COLOR_PRIMARY_SOFT, HCW_COLOR_PRIMARY_TINT,
                                        HCW_COLOR_PRIMARY);
    egui_view_items_control_set_corner_radius(self, 8);
    egui_view_items_control_set_border_width(self, 1);
    egui_view_items_control_set_item_gap(self, 6);
    egui_view_items_control_set_padding(self, 10, 10, 8, 8);
    egui_view_items_control_set_layout_mode(self, EGUI_VIEW_ITEMS_CONTROL_LAYOUT_HORIZONTAL);
    egui_view_items_control_set_item_align_type(self, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_items_control_set_compact_mode(self, 0);
    egui_view_items_control_set_read_only_mode(self, 0);
}

void egui_view_items_control_apply_wrap_style(egui_view_t *self)
{
    egui_view_items_control_set_palette(self, HCW_COLOR_SURFACE, HCW_COLOR_BORDER, HCW_COLOR_SURFACE_SUBTLE,
                                        HCW_COLOR_SUCCESS);
    egui_view_items_control_set_corner_radius(self, 6);
    egui_view_items_control_set_border_width(self, 1);
    egui_view_items_control_set_item_gap(self, 5);
    egui_view_items_control_set_padding(self, 8, 8, 6, 6);
    egui_view_items_control_set_layout_mode(self, EGUI_VIEW_ITEMS_CONTROL_LAYOUT_WRAP);
    egui_view_items_control_set_item_align_type(self, EGUI_ALIGN_TOP_LEFT);
    egui_view_items_control_set_compact_mode(self, 1);
    egui_view_items_control_set_read_only_mode(self, 0);
}

void egui_view_items_control_apply_read_only_style(egui_view_t *self)
{
    egui_view_items_control_set_palette(self, HCW_COLOR_SURFACE_SUBTLE, HCW_COLOR_BORDER_STRONG, HCW_COLOR_SURFACE_DISABLED,
                                        HCW_COLOR_TEXT_SOFT);
    egui_view_items_control_set_corner_radius(self, 6);
    egui_view_items_control_set_border_width(self, 1);
    egui_view_items_control_set_item_gap(self, 5);
    egui_view_items_control_set_padding(self, 8, 8, 6, 6);
    egui_view_items_control_set_layout_mode(self, EGUI_VIEW_ITEMS_CONTROL_LAYOUT_VERTICAL);
    egui_view_items_control_set_item_align_type(self, EGUI_ALIGN_TOP_LEFT);
    egui_view_items_control_set_compact_mode(self, 1);
    egui_view_items_control_set_read_only_mode(self, 1);
}

void egui_view_items_control_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_items_control_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_items_control_on_static_key_event;
#endif
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_items_control_t) = {
        .dispatch_touch_event = egui_view_group_dispatch_touch_event,
        .on_touch_event = egui_view_group_on_touch_event,
        .on_intercept_touch_event = egui_view_group_on_intercept_touch_event,
        .compute_scroll = egui_view_group_compute_scroll,
        .calculate_layout = egui_view_group_calculate_layout,
        .request_layout = egui_view_group_request_layout,
        .draw = egui_view_group_draw,
        .on_attach_to_window = egui_view_group_on_attach_to_window,
        .on_draw = egui_view_items_control_on_draw,
        .on_detach_from_window = egui_view_group_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_group_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

void egui_view_items_control_init(egui_view_t *self)
{
    egui_view_items_control_t *local = egui_view_items_control_local(self);

    egui_view_group_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_items_control_t);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->surface_color = HCW_COLOR_SURFACE;
    local->border_color = HCW_COLOR_BORDER_STRONG;
    local->item_surface_color = HCW_COLOR_SURFACE_PRESS;
    local->accent_color = HCW_COLOR_PRIMARY;
    local->corner_radius = 8;
    local->border_width = 1;
    local->item_gap = 6;
    local->content_padding_left = 10;
    local->content_padding_right = 10;
    local->content_padding_top = 8;
    local->content_padding_bottom = 8;
    local->layout_mode = EGUI_VIEW_ITEMS_CONTROL_LAYOUT_VERTICAL;
    local->item_align_type = EGUI_ALIGN_TOP_LEFT;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    egui_view_set_view_name(self, "egui_view_items_control");
}
