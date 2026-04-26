#include "egui_view_headered_items_control.h"

#define EGUI_VIEW_HEADERED_ITEMS_CONTROL_RADIUS_MAX     18
#define EGUI_VIEW_HEADERED_ITEMS_CONTROL_WIDTH_MAX      3
#define EGUI_VIEW_HEADERED_ITEMS_CONTROL_HEADER_GAP_MAX 16
#define EGUI_VIEW_HEADERED_ITEMS_CONTROL_ITEM_GAP_MAX   18

static egui_view_headered_items_control_t *egui_view_headered_items_control_local(egui_view_t *self)
{
    return (egui_view_headered_items_control_t *)self;
}

static uint8_t egui_view_headered_items_control_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static egui_color_t egui_view_headered_items_control_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x8A97A5), 58);
}

static egui_dim_t egui_view_headered_items_control_clamp_radius(egui_dim_t radius)
{
    if (radius < 0)
    {
        return 0;
    }
    if (radius > EGUI_VIEW_HEADERED_ITEMS_CONTROL_RADIUS_MAX)
    {
        return EGUI_VIEW_HEADERED_ITEMS_CONTROL_RADIUS_MAX;
    }
    return radius;
}

static egui_dim_t egui_view_headered_items_control_clamp_width(egui_dim_t width)
{
    if (width < 0)
    {
        return 0;
    }
    if (width > EGUI_VIEW_HEADERED_ITEMS_CONTROL_WIDTH_MAX)
    {
        return EGUI_VIEW_HEADERED_ITEMS_CONTROL_WIDTH_MAX;
    }
    return width;
}

static egui_dim_t egui_view_headered_items_control_clamp_header_gap(egui_dim_t gap)
{
    if (gap < 0)
    {
        return 0;
    }
    if (gap > EGUI_VIEW_HEADERED_ITEMS_CONTROL_HEADER_GAP_MAX)
    {
        return EGUI_VIEW_HEADERED_ITEMS_CONTROL_HEADER_GAP_MAX;
    }
    return gap;
}

static egui_dim_t egui_view_headered_items_control_clamp_item_gap(egui_dim_t gap)
{
    if (gap < 0)
    {
        return 0;
    }
    if (gap > EGUI_VIEW_HEADERED_ITEMS_CONTROL_ITEM_GAP_MAX)
    {
        return EGUI_VIEW_HEADERED_ITEMS_CONTROL_ITEM_GAP_MAX;
    }
    return gap;
}

static uint8_t egui_view_headered_items_control_normalize_layout(uint8_t layout_mode)
{
    if (layout_mode > EGUI_VIEW_HEADERED_ITEMS_CONTROL_LAYOUT_WRAP)
    {
        return EGUI_VIEW_HEADERED_ITEMS_CONTROL_LAYOUT_VERTICAL;
    }
    return layout_mode;
}

static egui_dim_t egui_view_headered_items_control_align_x(egui_dim_t parent_width, egui_dim_t child_width, uint8_t align_type)
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

static egui_dim_t egui_view_headered_items_control_align_y(egui_dim_t parent_height, egui_dim_t child_height, uint8_t align_type)
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

static egui_dim_t egui_view_headered_items_control_child_width(egui_view_t *child)
{
    return child->region.size.width + child->margin.left + child->margin.right;
}

static egui_dim_t egui_view_headered_items_control_child_height(egui_view_t *child)
{
    return child->region.size.height + child->margin.top + child->margin.bottom;
}

static uint8_t egui_view_headered_items_control_is_item(egui_view_headered_items_control_t *local, egui_view_t *child)
{
    return child != NULL && child != local->header;
}

static uint8_t egui_view_headered_items_control_has_child(egui_view_headered_items_control_t *local, egui_view_t *child)
{
    egui_dnode_t *node;
    egui_view_t *current_child;

    EGUI_DLIST_FOR_EACH_NODE(&local->base.childs, node)
    {
        current_child = EGUI_DLIST_ENTRY(node, egui_view_t, node);
        if (current_child == child)
        {
            return 1;
        }
    }
    return 0;
}

static void egui_view_headered_items_control_get_linear_size(egui_view_t *self, uint8_t horizontal, egui_dim_t gap, egui_dim_t *width,
                                                             egui_dim_t *height)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);
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
        if (!egui_view_headered_items_control_is_item(local, child) || child->is_gone)
        {
            continue;
        }
        visible_count++;
        if (horizontal)
        {
            total_width += egui_view_headered_items_control_child_width(child);
            if (egui_view_headered_items_control_child_height(child) > max_height)
            {
                max_height = egui_view_headered_items_control_child_height(child);
            }
        }
        else
        {
            total_height += egui_view_headered_items_control_child_height(child);
            if (egui_view_headered_items_control_child_width(child) > max_width)
            {
                max_width = egui_view_headered_items_control_child_width(child);
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

static void egui_view_headered_items_control_get_items_region(egui_view_t *self, egui_region_t *items_region)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    egui_view_get_work_region(self, items_region);
    if (items_region->size.width < 0)
    {
        items_region->size.width = 0;
    }
    if (items_region->size.height < 0)
    {
        items_region->size.height = 0;
    }

    if (local->header != NULL)
    {
        egui_dim_t header_block = local->header->region.size.height + local->header_gap;

        items_region->location.y += header_block;
        items_region->size.height -= header_block;
        if (items_region->size.height < 0)
        {
            items_region->size.height = 0;
        }
    }
}

static void egui_view_headered_items_control_on_draw(egui_view_t *self)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);
    egui_region_t region;
    egui_dnode_t *node;
    egui_view_t *child;
    egui_color_t surface_color = local->surface_color;
    egui_color_t border_color = local->border_color;
    egui_color_t header_color = local->header_color;
    egui_color_t item_surface_color = local->item_surface_color;
    egui_color_t accent_color = local->accent_color;
    egui_dim_t radius = local->corner_radius;
    egui_dim_t header_bottom = self->padding.top;
    egui_alpha_t border_alpha = EGUI_ALPHA_100;
    egui_alpha_t header_alpha = local->compact_mode ? 32 : 44;
    egui_alpha_t item_alpha = local->compact_mode ? 72 : 84;
    egui_alpha_t accent_alpha = local->compact_mode ? 30 : 42;

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
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xF7F9FB), 52);
        border_color = egui_rgb_mix(border_color, EGUI_COLOR_HEX(0xAEB8C2), 50);
        header_color = egui_rgb_mix(header_color, EGUI_COLOR_HEX(0xDDE4EC), 54);
        item_surface_color = egui_rgb_mix(item_surface_color, EGUI_COLOR_HEX(0xEEF2F6), 54);
        accent_color = egui_rgb_mix(accent_color, EGUI_COLOR_HEX(0x7A8794), 58);
        border_alpha = 70;
        header_alpha = 28;
        item_alpha = 60;
        accent_alpha = 18;
    }
    if (!egui_view_get_enable(self))
    {
        surface_color = egui_view_headered_items_control_mix_disabled(surface_color);
        border_color = egui_view_headered_items_control_mix_disabled(border_color);
        header_color = egui_view_headered_items_control_mix_disabled(header_color);
        item_surface_color = egui_view_headered_items_control_mix_disabled(item_surface_color);
        accent_color = egui_view_headered_items_control_mix_disabled(accent_color);
        border_alpha = 50;
        header_alpha = 22;
        item_alpha = 46;
        accent_alpha = 14;
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height, radius,
                                          surface_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));

    if (local->header != NULL)
    {
        header_bottom = local->header->region.location.y + local->header->region.size.height + local->header_gap / 2;
        if (header_bottom > region.size.height)
        {
            header_bottom = region.size.height;
        }
        if (header_bottom > 0 && region.size.width > 2)
        {
            egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region.location.x + 1, region.location.y + 1, region.size.width - 2,
                                                  header_bottom, radius > 1 ? radius - 1 : 0, header_color,
                                                  egui_color_alpha_mix(self->alpha, header_alpha));
        }
        if (region.size.width > 18 && header_bottom + 1 < region.size.height)
        {
            egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region.location.x + 8, region.location.y + header_bottom,
                                                  region.size.width - 16, 1, 1, border_color, egui_color_alpha_mix(self->alpha, 58));
        }
    }

    EGUI_DLIST_FOR_EACH_NODE(&local->base.childs, node)
    {
        egui_region_t item_region;

        child = EGUI_DLIST_ENTRY(node, egui_view_t, node);
        if (!egui_view_headered_items_control_is_item(local, child) || child->is_gone || !child->is_visible)
        {
            continue;
        }
        item_region.location.x = child->region.location.x - (local->compact_mode ? 2 : 3);
        item_region.location.y = child->region.location.y - (local->compact_mode ? 1 : 2);
        item_region.size.width = child->region.size.width + (local->compact_mode ? 4 : 6);
        item_region.size.height = child->region.size.height + (local->compact_mode ? 2 : 4);
        if (item_region.location.x < 1)
        {
            item_region.location.x = 1;
        }
        if (item_region.location.y < 1)
        {
            item_region.location.y = 1;
        }
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, item_region.location.x, item_region.location.y, item_region.size.width,
                                              item_region.size.height, local->compact_mode ? 4 : 6, item_surface_color,
                                              egui_color_alpha_mix(self->alpha, item_alpha));
    }

    if (region.size.width > 18 && region.size.height > 14)
    {
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region.location.x + 7, region.location.y + 6,
                                              local->compact_mode ? 22 : 30, local->compact_mode ? 2 : 3, 1, accent_color,
                                              egui_color_alpha_mix(self->alpha, accent_alpha));
    }

    if (local->border_width > 0)
    {
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height, radius,
                                         local->border_width, border_color, egui_color_alpha_mix(self->alpha, border_alpha));
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_headered_items_control_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_headered_items_control_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_headered_items_control_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_headered_items_control_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_headered_items_control_set_header(egui_view_t *self, egui_view_t *header)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    if (local->header == header)
    {
        return;
    }
    if (local->header != NULL)
    {
        egui_view_group_remove_child(self, local->header);
    }
    if (header != NULL && egui_view_headered_items_control_has_child(local, header))
    {
        egui_view_group_remove_child(self, header);
    }

    local->header = header;
    if (header != NULL)
    {
        egui_view_group_add_child(self, header);
    }
    egui_view_headered_items_control_layout_childs(self);
    egui_view_invalidate(self);
}

egui_view_t *egui_view_headered_items_control_get_header(egui_view_t *self)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    return local->header;
}

void egui_view_headered_items_control_add_item(egui_view_t *self, egui_view_t *item)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    if (item == NULL || item == local->header)
    {
        return;
    }
    if (egui_view_headered_items_control_has_child(local, item))
    {
        egui_view_group_remove_child(self, item);
    }

    egui_view_group_add_child(self, item);
    egui_view_headered_items_control_layout_childs(self);
    egui_view_invalidate(self);
}

void egui_view_headered_items_control_clear_items(egui_view_t *self)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);
    egui_dnode_t *node;
    egui_dnode_t *next_node;
    egui_view_t *child;

    egui_view_headered_items_control_clear_pressed_state(self);
    EGUI_DLIST_FOR_EACH_NODE_SAFE(&local->base.childs, node, next_node)
    {
        child = EGUI_DLIST_ENTRY(node, egui_view_t, node);
        if (egui_view_headered_items_control_is_item(local, child))
        {
            egui_view_group_remove_child(self, child);
        }
    }
    egui_view_headered_items_control_layout_childs(self);
    egui_view_invalidate(self);
}

int egui_view_headered_items_control_get_item_count(egui_view_t *self)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);
    egui_dnode_t *node;
    egui_view_t *child;
    int count = 0;

    EGUI_DLIST_FOR_EACH_NODE(&local->base.childs, node)
    {
        child = EGUI_DLIST_ENTRY(node, egui_view_t, node);
        if (egui_view_headered_items_control_is_item(local, child))
        {
            count++;
        }
    }
    return count;
}

void egui_view_headered_items_control_layout_childs(egui_view_t *self)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);
    egui_region_t work_region;
    egui_region_t items_region;
    egui_dnode_t *node;
    egui_view_t *child;
    egui_dim_t x;
    egui_dim_t y;
    egui_dim_t total_width;
    egui_dim_t total_height;
    egui_dim_t line_height;

    egui_view_get_work_region(self, &work_region);
    if (work_region.size.width < 0)
    {
        work_region.size.width = 0;
    }
    if (work_region.size.height < 0)
    {
        work_region.size.height = 0;
    }

    if (local->header != NULL)
    {
        egui_dim_t header_x = work_region.location.x + egui_view_headered_items_control_align_x(work_region.size.width, local->header->region.size.width,
                                                                                                local->header_align_type);
        egui_view_set_position(local->header, header_x, work_region.location.y);
    }

    egui_view_headered_items_control_get_items_region(self, &items_region);
    if (local->layout_mode == EGUI_VIEW_HEADERED_ITEMS_CONTROL_LAYOUT_HORIZONTAL)
    {
        egui_view_headered_items_control_get_linear_size(self, 1, local->item_gap, &total_width, &total_height);
        x = items_region.location.x + egui_view_headered_items_control_align_x(items_region.size.width, total_width, local->item_align_type);
        y = items_region.location.y + egui_view_headered_items_control_align_y(items_region.size.height, total_height, local->item_align_type);
        EGUI_DLIST_FOR_EACH_NODE(&local->base.childs, node)
        {
            child = EGUI_DLIST_ENTRY(node, egui_view_t, node);
            if (!egui_view_headered_items_control_is_item(local, child) || child->is_gone)
            {
                continue;
            }
            egui_view_set_position(child, x + child->margin.left, y + child->margin.top);
            x += egui_view_headered_items_control_child_width(child) + local->item_gap;
        }
        return;
    }

    if (local->layout_mode == EGUI_VIEW_HEADERED_ITEMS_CONTROL_LAYOUT_WRAP)
    {
        x = items_region.location.x;
        y = items_region.location.y;
        line_height = 0;
        EGUI_DLIST_FOR_EACH_NODE(&local->base.childs, node)
        {
            child = EGUI_DLIST_ENTRY(node, egui_view_t, node);
            if (!egui_view_headered_items_control_is_item(local, child) || child->is_gone)
            {
                continue;
            }
            if (x > items_region.location.x &&
                x + egui_view_headered_items_control_child_width(child) > items_region.location.x + items_region.size.width)
            {
                x = items_region.location.x;
                y += line_height + local->item_gap;
                line_height = 0;
            }
            egui_view_set_position(child, x + child->margin.left, y + child->margin.top);
            x += egui_view_headered_items_control_child_width(child) + local->item_gap;
            if (egui_view_headered_items_control_child_height(child) > line_height)
            {
                line_height = egui_view_headered_items_control_child_height(child);
            }
        }
        return;
    }

    egui_view_headered_items_control_get_linear_size(self, 0, local->item_gap, &total_width, &total_height);
    x = items_region.location.x + egui_view_headered_items_control_align_x(items_region.size.width, total_width, local->item_align_type);
    y = items_region.location.y + egui_view_headered_items_control_align_y(items_region.size.height, total_height, local->item_align_type);
    EGUI_DLIST_FOR_EACH_NODE(&local->base.childs, node)
    {
        child = EGUI_DLIST_ENTRY(node, egui_view_t, node);
        if (!egui_view_headered_items_control_is_item(local, child) || child->is_gone)
        {
            continue;
        }
        egui_view_set_position(child, x + child->margin.left, y + child->margin.top);
        y += egui_view_headered_items_control_child_height(child) + local->item_gap;
    }
}

void egui_view_headered_items_control_set_padding(egui_view_t *self, egui_dim_margin_padding_t left, egui_dim_margin_padding_t right,
                                                  egui_dim_margin_padding_t top, egui_dim_margin_padding_t bottom)
{
    egui_view_headered_items_control_clear_pressed_state(self);
    egui_view_set_padding(self, left, right, top, bottom);
    egui_view_headered_items_control_layout_childs(self);
    egui_view_invalidate(self);
}

void egui_view_headered_items_control_set_header_gap(egui_view_t *self, egui_dim_t gap)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    egui_view_headered_items_control_clear_pressed_state(self);
    local->header_gap = egui_view_headered_items_control_clamp_header_gap(gap);
    egui_view_headered_items_control_layout_childs(self);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_headered_items_control_get_header_gap(egui_view_t *self)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    return local->header_gap;
}

void egui_view_headered_items_control_set_item_gap(egui_view_t *self, egui_dim_t gap)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    egui_view_headered_items_control_clear_pressed_state(self);
    local->item_gap = egui_view_headered_items_control_clamp_item_gap(gap);
    egui_view_headered_items_control_layout_childs(self);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_headered_items_control_get_item_gap(egui_view_t *self)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    return local->item_gap;
}

void egui_view_headered_items_control_set_layout_mode(egui_view_t *self, uint8_t layout_mode)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    egui_view_headered_items_control_clear_pressed_state(self);
    local->layout_mode = egui_view_headered_items_control_normalize_layout(layout_mode);
    egui_view_headered_items_control_layout_childs(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_headered_items_control_get_layout_mode(egui_view_t *self)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    return local->layout_mode;
}

void egui_view_headered_items_control_set_header_align_type(egui_view_t *self, uint8_t align_type)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    egui_view_headered_items_control_clear_pressed_state(self);
    local->header_align_type = align_type;
    egui_view_headered_items_control_layout_childs(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_headered_items_control_get_header_align_type(egui_view_t *self)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    return local->header_align_type;
}

void egui_view_headered_items_control_set_item_align_type(egui_view_t *self, uint8_t align_type)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    egui_view_headered_items_control_clear_pressed_state(self);
    local->item_align_type = align_type;
    egui_view_headered_items_control_layout_childs(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_headered_items_control_get_item_align_type(egui_view_t *self)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    return local->item_align_type;
}

void egui_view_headered_items_control_set_corner_radius(egui_view_t *self, egui_dim_t radius)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    egui_view_headered_items_control_clear_pressed_state(self);
    local->corner_radius = egui_view_headered_items_control_clamp_radius(radius);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_headered_items_control_get_corner_radius(egui_view_t *self)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    return local->corner_radius;
}

void egui_view_headered_items_control_set_border_width(egui_view_t *self, egui_dim_t width)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    egui_view_headered_items_control_clear_pressed_state(self);
    local->border_width = egui_view_headered_items_control_clamp_width(width);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_headered_items_control_get_border_width(egui_view_t *self)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    return local->border_width;
}

void egui_view_headered_items_control_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color,
                                                  egui_color_t header_color, egui_color_t item_surface_color, egui_color_t accent_color)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    egui_view_headered_items_control_clear_pressed_state(self);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->header_color = header_color;
    local->item_surface_color = item_surface_color;
    local->accent_color = accent_color;
    egui_view_invalidate(self);
}

void egui_view_headered_items_control_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    egui_view_headered_items_control_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_headered_items_control_get_compact_mode(egui_view_t *self)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    return local->compact_mode;
}

void egui_view_headered_items_control_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    egui_view_headered_items_control_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_headered_items_control_get_read_only_mode(egui_view_t *self)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    return local->read_only_mode;
}

void egui_view_headered_items_control_apply_standard_style(egui_view_t *self)
{
    egui_view_headered_items_control_set_palette(self, EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xC7D3DE), EGUI_COLOR_HEX(0xEDF5FD),
                                                EGUI_COLOR_HEX(0xF2F7FC), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_headered_items_control_set_corner_radius(self, 8);
    egui_view_headered_items_control_set_border_width(self, 1);
    egui_view_headered_items_control_set_header_gap(self, 6);
    egui_view_headered_items_control_set_item_gap(self, 5);
    egui_view_headered_items_control_set_padding(self, 12, 12, 10, 10);
    egui_view_headered_items_control_set_layout_mode(self, EGUI_VIEW_HEADERED_ITEMS_CONTROL_LAYOUT_VERTICAL);
    egui_view_headered_items_control_set_header_align_type(self, EGUI_ALIGN_TOP_LEFT);
    egui_view_headered_items_control_set_item_align_type(self, EGUI_ALIGN_TOP_LEFT);
    egui_view_headered_items_control_set_compact_mode(self, 0);
    egui_view_headered_items_control_set_read_only_mode(self, 0);
}

void egui_view_headered_items_control_apply_strip_style(egui_view_t *self)
{
    egui_view_headered_items_control_set_palette(self, EGUI_COLOR_HEX(0xF8FBFF), EGUI_COLOR_HEX(0xA7CBEA), EGUI_COLOR_HEX(0xEAF4FE),
                                                EGUI_COLOR_HEX(0xEEF6FD), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_headered_items_control_set_corner_radius(self, 8);
    egui_view_headered_items_control_set_border_width(self, 1);
    egui_view_headered_items_control_set_header_gap(self, 6);
    egui_view_headered_items_control_set_item_gap(self, 6);
    egui_view_headered_items_control_set_padding(self, 12, 12, 10, 10);
    egui_view_headered_items_control_set_layout_mode(self, EGUI_VIEW_HEADERED_ITEMS_CONTROL_LAYOUT_HORIZONTAL);
    egui_view_headered_items_control_set_header_align_type(self, EGUI_ALIGN_TOP_LEFT);
    egui_view_headered_items_control_set_item_align_type(self, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_headered_items_control_set_compact_mode(self, 0);
    egui_view_headered_items_control_set_read_only_mode(self, 0);
}

void egui_view_headered_items_control_apply_wrap_style(egui_view_t *self)
{
    egui_view_headered_items_control_set_palette(self, EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD3DCE5), EGUI_COLOR_HEX(0xF3F7FA),
                                                EGUI_COLOR_HEX(0xF3F8F5), EGUI_COLOR_HEX(0x0F7B45));
    egui_view_headered_items_control_set_corner_radius(self, 6);
    egui_view_headered_items_control_set_border_width(self, 1);
    egui_view_headered_items_control_set_header_gap(self, 4);
    egui_view_headered_items_control_set_item_gap(self, 5);
    egui_view_headered_items_control_set_padding(self, 8, 8, 6, 6);
    egui_view_headered_items_control_set_layout_mode(self, EGUI_VIEW_HEADERED_ITEMS_CONTROL_LAYOUT_WRAP);
    egui_view_headered_items_control_set_header_align_type(self, EGUI_ALIGN_TOP_LEFT);
    egui_view_headered_items_control_set_item_align_type(self, EGUI_ALIGN_TOP_LEFT);
    egui_view_headered_items_control_set_compact_mode(self, 1);
    egui_view_headered_items_control_set_read_only_mode(self, 0);
}

void egui_view_headered_items_control_apply_read_only_style(egui_view_t *self)
{
    egui_view_headered_items_control_set_palette(self, EGUI_COLOR_HEX(0xF7F9FB), EGUI_COLOR_HEX(0xD8E0E8), EGUI_COLOR_HEX(0xEEF2F6),
                                                EGUI_COLOR_HEX(0xEEF2F6), EGUI_COLOR_HEX(0x6B7785));
    egui_view_headered_items_control_set_corner_radius(self, 6);
    egui_view_headered_items_control_set_border_width(self, 1);
    egui_view_headered_items_control_set_header_gap(self, 4);
    egui_view_headered_items_control_set_item_gap(self, 5);
    egui_view_headered_items_control_set_padding(self, 8, 8, 6, 6);
    egui_view_headered_items_control_set_layout_mode(self, EGUI_VIEW_HEADERED_ITEMS_CONTROL_LAYOUT_VERTICAL);
    egui_view_headered_items_control_set_header_align_type(self, EGUI_ALIGN_TOP_LEFT);
    egui_view_headered_items_control_set_item_align_type(self, EGUI_ALIGN_TOP_LEFT);
    egui_view_headered_items_control_set_compact_mode(self, 1);
    egui_view_headered_items_control_set_read_only_mode(self, 1);
}

void egui_view_headered_items_control_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_headered_items_control_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_headered_items_control_on_static_key_event;
#endif
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_headered_items_control_t) = {
        .dispatch_touch_event = egui_view_group_dispatch_touch_event,
        .on_touch_event = egui_view_group_on_touch_event,
        .on_intercept_touch_event = egui_view_group_on_intercept_touch_event,
        .compute_scroll = egui_view_group_compute_scroll,
        .calculate_layout = egui_view_group_calculate_layout,
        .request_layout = egui_view_group_request_layout,
        .draw = egui_view_group_draw,
        .on_attach_to_window = egui_view_group_on_attach_to_window,
        .on_draw = egui_view_headered_items_control_on_draw,
        .on_detach_from_window = egui_view_group_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_group_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

void egui_view_headered_items_control_init(egui_view_t *self)
{
    egui_view_headered_items_control_t *local = egui_view_headered_items_control_local(self);

    egui_view_group_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_headered_items_control_t);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->header = NULL;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xC7D3DE);
    local->header_color = EGUI_COLOR_HEX(0xEDF5FD);
    local->item_surface_color = EGUI_COLOR_HEX(0xF2F7FC);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->corner_radius = 8;
    local->border_width = 1;
    local->header_gap = 6;
    local->item_gap = 5;
    local->layout_mode = EGUI_VIEW_HEADERED_ITEMS_CONTROL_LAYOUT_VERTICAL;
    local->header_align_type = EGUI_ALIGN_TOP_LEFT;
    local->item_align_type = EGUI_ALIGN_TOP_LEFT;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    egui_view_set_padding(self, 12, 12, 10, 10);
    egui_view_set_view_name(self, "egui_view_headered_items_control");
}
