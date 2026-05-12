#include "egui_view_group_box.h"

#define EGUI_VIEW_GROUP_BOX_RADIUS_MAX 18
#define EGUI_VIEW_GROUP_BOX_WIDTH_MAX  3
#define EGUI_VIEW_GROUP_BOX_GAP_MAX    16
#define EGUI_VIEW_GROUP_BOX_INDENT_MAX 28

static egui_view_group_box_t *egui_view_group_box_local(egui_view_t *self)
{
    return (egui_view_group_box_t *)self;
}

static uint8_t egui_view_group_box_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static egui_color_t egui_view_group_box_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_MAKE(38));
}

static egui_dim_t egui_view_group_box_clamp_radius(egui_dim_t radius)
{
    if (radius < 0)
    {
        return 0;
    }
    if (radius > EGUI_VIEW_GROUP_BOX_RADIUS_MAX)
    {
        return EGUI_VIEW_GROUP_BOX_RADIUS_MAX;
    }
    return radius;
}

static egui_dim_t egui_view_group_box_clamp_width(egui_dim_t width)
{
    if (width < 0)
    {
        return 0;
    }
    if (width > EGUI_VIEW_GROUP_BOX_WIDTH_MAX)
    {
        return EGUI_VIEW_GROUP_BOX_WIDTH_MAX;
    }
    return width;
}

static egui_dim_t egui_view_group_box_clamp_gap(egui_dim_t gap)
{
    if (gap < 0)
    {
        return 0;
    }
    if (gap > EGUI_VIEW_GROUP_BOX_GAP_MAX)
    {
        return EGUI_VIEW_GROUP_BOX_GAP_MAX;
    }
    return gap;
}

static egui_dim_t egui_view_group_box_clamp_indent(egui_dim_t indent)
{
    if (indent < 0)
    {
        return 0;
    }
    if (indent > EGUI_VIEW_GROUP_BOX_INDENT_MAX)
    {
        return EGUI_VIEW_GROUP_BOX_INDENT_MAX;
    }
    return indent;
}

static egui_dim_t egui_view_group_box_align_x(egui_dim_t parent_width, egui_dim_t child_width, uint8_t align_type)
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

static egui_dim_t egui_view_group_box_align_y(egui_dim_t parent_height, egui_dim_t child_height, uint8_t align_type)
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

static void egui_view_group_box_constrain_child_backdrop(egui_region_t *child_region, const egui_region_t *container_region, egui_dim_t inset)
{
    egui_dim_t min_x;
    egui_dim_t min_y;
    egui_dim_t max_right;
    egui_dim_t max_bottom;

    if (child_region == NULL || container_region == NULL)
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

    if (child_region->location.x < min_x)
    {
        child_region->size.width -= min_x - child_region->location.x;
        child_region->location.x = min_x;
    }
    if (child_region->location.y < min_y)
    {
        child_region->size.height -= min_y - child_region->location.y;
        child_region->location.y = min_y;
    }
    if (child_region->location.x + child_region->size.width > max_right)
    {
        child_region->size.width = max_right - child_region->location.x;
    }
    if (child_region->location.y + child_region->size.height > max_bottom)
    {
        child_region->size.height = max_bottom - child_region->location.y;
    }
    if (child_region->size.width < 0)
    {
        child_region->size.width = 0;
    }
    if (child_region->size.height < 0)
    {
        child_region->size.height = 0;
    }
}

static void egui_view_group_box_get_content_region(egui_view_t *self, egui_region_t *content_region)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    content_region->location.x = local->content_padding_left;
    content_region->location.y = local->content_padding_top;
    content_region->size.width = self->region.size.width - (local->content_padding_left + local->content_padding_right);
    content_region->size.height = self->region.size.height - (local->content_padding_top + local->content_padding_bottom);
    if (content_region->size.width < 0)
    {
        content_region->size.width = 0;
    }
    if (content_region->size.height < 0)
    {
        content_region->size.height = 0;
    }

    if (local->header != NULL)
    {
        egui_dim_t header_block = local->header->region.size.height + local->header_gap;

        content_region->location.y += header_block;
        content_region->size.height -= header_block;
        if (content_region->size.height < 0)
        {
            content_region->size.height = 0;
        }
    }
}

static void egui_view_group_box_on_draw(egui_view_t *self)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);
    egui_region_t region;
    egui_region_t content_region;
    egui_color_t surface_color = local->surface_color;
    egui_color_t border_color = local->border_color;
    egui_color_t header_surface_color = local->header_surface_color;
    egui_color_t content_surface_color = local->content_surface_color;
    egui_color_t accent_color = local->accent_color;
    egui_dim_t radius = local->corner_radius;
    egui_alpha_t border_alpha = EGUI_ALPHA_100;
    egui_alpha_t header_alpha = EGUI_ALPHA_MAKE(local->compact_mode ? 78 : 88);
    egui_alpha_t content_alpha = EGUI_ALPHA_MAKE(local->compact_mode ? 42 : 54);
    egui_alpha_t accent_alpha = EGUI_ALPHA_MAKE(local->compact_mode ? 34 : 48);

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
        header_surface_color = egui_rgb_mix(header_surface_color, HCW_COLOR_SURFACE_DISABLED, EGUI_ALPHA_MAKE(36));
        content_surface_color = egui_rgb_mix(content_surface_color, HCW_COLOR_SURFACE_DISABLED, EGUI_ALPHA_MAKE(34));
        accent_color = egui_rgb_mix(accent_color, HCW_COLOR_TEXT_MUTED, EGUI_ALPHA_MAKE(36));
        border_alpha = EGUI_ALPHA_MAKE(82);
        header_alpha = EGUI_ALPHA_MAKE(76);
        content_alpha = EGUI_ALPHA_MAKE(48);
        accent_alpha = EGUI_ALPHA_MAKE(30);
    }
    if (!egui_view_get_enable(self))
    {
        surface_color = egui_view_group_box_mix_disabled(surface_color);
        border_color = egui_view_group_box_mix_disabled(border_color);
        header_surface_color = egui_view_group_box_mix_disabled(header_surface_color);
        content_surface_color = egui_view_group_box_mix_disabled(content_surface_color);
        accent_color = egui_view_group_box_mix_disabled(accent_color);
        border_alpha = EGUI_ALPHA_MAKE(64);
        header_alpha = EGUI_ALPHA_MAKE(62);
        content_alpha = EGUI_ALPHA_MAKE(40);
        accent_alpha = EGUI_ALPHA_MAKE(24);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height, radius,
                                          surface_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));

    egui_view_group_box_get_content_region(self, &content_region);
    if (content_region.size.width > 8 && content_region.size.height > 8)
    {
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, content_region.location.x - 2, content_region.location.y - 2,
                                              content_region.size.width + 4, content_region.size.height + 4, local->compact_mode ? 5 : 7,
                                              content_surface_color, egui_color_alpha_mix(self->alpha, content_alpha));
    }

    if (local->header != NULL)
    {
        egui_region_t header_region;
        egui_dim_t header_pad_x = local->compact_mode ? 8 : 10;
        egui_dim_t header_pad_y = local->compact_mode ? 3 : 4;

        header_region.location.x = local->header->region.location.x - header_pad_x;
        header_region.location.y = local->header->region.location.y - header_pad_y;
        header_region.size.width = local->header->region.size.width + header_pad_x * 2;
        header_region.size.height = local->header->region.size.height + header_pad_y * 2;
        egui_view_group_box_constrain_child_backdrop(&header_region, &region, local->border_width);
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, header_region.location.x, header_region.location.y, header_region.size.width,
                                              header_region.size.height, local->compact_mode ? 5 : 7, header_surface_color,
                                              egui_color_alpha_mix(self->alpha, header_alpha));
    }
    EGUI_UNUSED(accent_color);
    EGUI_UNUSED(accent_alpha);

    if (local->border_width > 0)
    {
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height, radius,
                                         local->border_width, border_color, egui_color_alpha_mix(self->alpha, border_alpha));
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_group_box_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_group_box_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_group_box_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_group_box_clear_pressed_state(self);
    return 1;
}
#endif

static void egui_view_group_box_replace_child(egui_view_t *self, egui_view_t **slot, egui_view_t *child)
{
    if (*slot == child)
    {
        return;
    }
    if (*slot != NULL)
    {
        egui_view_group_remove_child(self, *slot);
    }

    *slot = child;
    if (child != NULL)
    {
        egui_view_group_add_child(self, child);
        egui_view_group_box_layout_childs(self);
    }
    egui_view_invalidate(self);
}

void egui_view_group_box_set_header(egui_view_t *self, egui_view_t *header)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    if (header != NULL && header == local->content)
    {
        egui_view_group_remove_child(self, local->content);
        local->content = NULL;
    }
    egui_view_group_box_replace_child(self, &local->header, header);
}

egui_view_t *egui_view_group_box_get_header(egui_view_t *self)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    return local->header;
}

void egui_view_group_box_set_content(egui_view_t *self, egui_view_t *content)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    if (content != NULL && content == local->header)
    {
        egui_view_group_remove_child(self, local->header);
        local->header = NULL;
    }
    egui_view_group_box_replace_child(self, &local->content, content);
}

egui_view_t *egui_view_group_box_get_content(egui_view_t *self)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    return local->content;
}

void egui_view_group_box_layout_childs(egui_view_t *self)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);
    egui_region_t work_region;
    egui_region_t content_region;

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

    if (local->header != NULL)
    {
        egui_dim_t header_x;
        egui_dim_t header_width = local->header->region.size.width;
        egui_dim_t header_parent_width = work_region.size.width - local->header_indent * 2;

        if (header_parent_width < 0)
        {
            header_parent_width = 0;
        }
        header_x = work_region.location.x + local->header_indent +
                   egui_view_group_box_align_x(header_parent_width, header_width, local->header_align_type);
        egui_view_set_position(local->header, header_x, work_region.location.y);
    }

    egui_view_group_box_get_content_region(self, &content_region);
    if (local->content != NULL)
    {
        egui_dim_t content_x =
                content_region.location.x + egui_view_group_box_align_x(content_region.size.width, local->content->region.size.width, local->content_align_type);
        egui_dim_t content_y =
                content_region.location.y + egui_view_group_box_align_y(content_region.size.height, local->content->region.size.height, local->content_align_type);
        egui_view_set_position(local->content, content_x, content_y);
    }
}

void egui_view_group_box_set_padding(egui_view_t *self, egui_dim_margin_padding_t left, egui_dim_margin_padding_t right,
                                     egui_dim_margin_padding_t top, egui_dim_margin_padding_t bottom)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    egui_view_group_box_clear_pressed_state(self);
    local->content_padding_left = left;
    local->content_padding_right = right;
    local->content_padding_top = top;
    local->content_padding_bottom = bottom;
    egui_view_group_box_layout_childs(self);
    egui_view_invalidate(self);
}

void egui_view_group_box_set_header_gap(egui_view_t *self, egui_dim_t gap)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    egui_view_group_box_clear_pressed_state(self);
    local->header_gap = egui_view_group_box_clamp_gap(gap);
    egui_view_group_box_layout_childs(self);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_group_box_get_header_gap(egui_view_t *self)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    return local->header_gap;
}

void egui_view_group_box_set_header_indent(egui_view_t *self, egui_dim_t indent)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    egui_view_group_box_clear_pressed_state(self);
    local->header_indent = egui_view_group_box_clamp_indent(indent);
    egui_view_group_box_layout_childs(self);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_group_box_get_header_indent(egui_view_t *self)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    return local->header_indent;
}

void egui_view_group_box_set_header_align_type(egui_view_t *self, uint8_t align_type)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    egui_view_group_box_clear_pressed_state(self);
    local->header_align_type = align_type;
    egui_view_group_box_layout_childs(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_group_box_get_header_align_type(egui_view_t *self)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    return local->header_align_type;
}

void egui_view_group_box_set_content_align_type(egui_view_t *self, uint8_t align_type)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    egui_view_group_box_clear_pressed_state(self);
    local->content_align_type = align_type;
    egui_view_group_box_layout_childs(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_group_box_get_content_align_type(egui_view_t *self)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    return local->content_align_type;
}

void egui_view_group_box_set_corner_radius(egui_view_t *self, egui_dim_t radius)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    egui_view_group_box_clear_pressed_state(self);
    local->corner_radius = egui_view_group_box_clamp_radius(radius);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_group_box_get_corner_radius(egui_view_t *self)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    return local->corner_radius;
}

void egui_view_group_box_set_border_width(egui_view_t *self, egui_dim_t width)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    egui_view_group_box_clear_pressed_state(self);
    local->border_width = egui_view_group_box_clamp_width(width);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_group_box_get_border_width(egui_view_t *self)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    return local->border_width;
}

void egui_view_group_box_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color,
                                     egui_color_t header_surface_color, egui_color_t content_surface_color, egui_color_t accent_color)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    egui_view_group_box_clear_pressed_state(self);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->header_surface_color = header_surface_color;
    local->content_surface_color = content_surface_color;
    local->accent_color = accent_color;
    egui_view_invalidate(self);
}

void egui_view_group_box_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    egui_view_group_box_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_group_box_get_compact_mode(egui_view_t *self)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    return local->compact_mode;
}

void egui_view_group_box_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    egui_view_group_box_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_group_box_get_read_only_mode(egui_view_t *self)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    return local->read_only_mode;
}

void egui_view_group_box_apply_standard_style(egui_view_t *self)
{
    egui_view_group_box_set_palette(self, HCW_COLOR_SURFACE, HCW_COLOR_BORDER_STRONG, HCW_COLOR_PRIMARY_TINT,
                                    HCW_COLOR_PANEL, HCW_COLOR_PRIMARY);
    egui_view_group_box_set_corner_radius(self, 8);
    egui_view_group_box_set_border_width(self, 1);
    egui_view_group_box_set_header_gap(self, 7);
    egui_view_group_box_set_header_indent(self, 10);
    egui_view_group_box_set_padding(self, 12, 12, 10, 10);
    egui_view_group_box_set_header_align_type(self, EGUI_ALIGN_TOP_MID);
    egui_view_group_box_set_content_align_type(self, EGUI_ALIGN_CENTER);
    egui_view_group_box_set_compact_mode(self, 0);
    egui_view_group_box_set_read_only_mode(self, 0);
}

void egui_view_group_box_apply_accent_style(egui_view_t *self)
{
    egui_view_group_box_set_palette(self, HCW_COLOR_PANEL, HCW_COLOR_PRIMARY_SOFT, HCW_COLOR_PRIMARY_TINT,
                                    HCW_COLOR_PRIMARY_TINT, HCW_COLOR_PRIMARY);
    egui_view_group_box_set_corner_radius(self, 8);
    egui_view_group_box_set_border_width(self, 1);
    egui_view_group_box_set_header_gap(self, 7);
    egui_view_group_box_set_header_indent(self, 10);
    egui_view_group_box_set_padding(self, 12, 12, 10, 10);
    egui_view_group_box_set_header_align_type(self, EGUI_ALIGN_TOP_MID);
    egui_view_group_box_set_content_align_type(self, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_group_box_set_compact_mode(self, 0);
    egui_view_group_box_set_read_only_mode(self, 0);
}

void egui_view_group_box_apply_compact_style(egui_view_t *self)
{
    egui_view_group_box_set_palette(self, HCW_COLOR_SURFACE, HCW_COLOR_BORDER, HCW_COLOR_PANEL,
                                    HCW_COLOR_PANEL, HCW_COLOR_SUCCESS);
    egui_view_group_box_set_corner_radius(self, 6);
    egui_view_group_box_set_border_width(self, 1);
    egui_view_group_box_set_header_gap(self, 5);
    egui_view_group_box_set_header_indent(self, 8);
    egui_view_group_box_set_padding(self, 8, 8, 6, 6);
    egui_view_group_box_set_header_align_type(self, EGUI_ALIGN_TOP_MID);
    egui_view_group_box_set_content_align_type(self, EGUI_ALIGN_TOP_LEFT);
    egui_view_group_box_set_compact_mode(self, 1);
    egui_view_group_box_set_read_only_mode(self, 0);
}

void egui_view_group_box_apply_read_only_style(egui_view_t *self)
{
    egui_view_group_box_set_palette(self, HCW_COLOR_SURFACE_SUBTLE, HCW_COLOR_BORDER_STRONG, HCW_COLOR_SURFACE_SUBTLE,
                                    HCW_COLOR_PANEL, HCW_COLOR_TEXT_SOFT);
    egui_view_group_box_set_corner_radius(self, 6);
    egui_view_group_box_set_border_width(self, 1);
    egui_view_group_box_set_header_gap(self, 5);
    egui_view_group_box_set_header_indent(self, 8);
    egui_view_group_box_set_padding(self, 8, 8, 6, 6);
    egui_view_group_box_set_header_align_type(self, EGUI_ALIGN_TOP_MID);
    egui_view_group_box_set_content_align_type(self, EGUI_ALIGN_CENTER);
    egui_view_group_box_set_compact_mode(self, 1);
    egui_view_group_box_set_read_only_mode(self, 1);
}

void egui_view_group_box_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_group_box_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_group_box_on_static_key_event;
#endif
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_group_box_t) = {
        .dispatch_touch_event = egui_view_group_dispatch_touch_event,
        .on_touch_event = egui_view_group_on_touch_event,
        .on_intercept_touch_event = egui_view_group_on_intercept_touch_event,
        .compute_scroll = egui_view_group_compute_scroll,
        .calculate_layout = egui_view_group_calculate_layout,
        .request_layout = egui_view_group_request_layout,
        .draw = egui_view_group_draw,
        .on_attach_to_window = egui_view_group_on_attach_to_window,
        .on_draw = egui_view_group_box_on_draw,
        .on_detach_from_window = egui_view_group_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_group_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

void egui_view_group_box_init(egui_view_t *self)
{
    egui_view_group_box_t *local = egui_view_group_box_local(self);

    egui_view_group_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_group_box_t);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->header = NULL;
    local->content = NULL;
    local->surface_color = HCW_COLOR_SURFACE;
    local->border_color = HCW_COLOR_BORDER_STRONG;
    local->header_surface_color = HCW_COLOR_PRIMARY_TINT;
    local->content_surface_color = HCW_COLOR_PANEL;
    local->accent_color = HCW_COLOR_PRIMARY;
    local->corner_radius = 8;
    local->border_width = 1;
    local->header_gap = 7;
    local->header_indent = 10;
    local->content_padding_left = 12;
    local->content_padding_right = 12;
    local->content_padding_top = 10;
    local->content_padding_bottom = 10;
    local->header_align_type = EGUI_ALIGN_TOP_MID;
    local->content_align_type = EGUI_ALIGN_CENTER;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    egui_view_set_view_name(self, "egui_view_group_box");
}
