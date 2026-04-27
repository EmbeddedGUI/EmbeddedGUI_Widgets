#include "egui_view_adorner_decorator.h"

#define EGUI_VIEW_ADORNER_DECORATOR_RADIUS_MAX 18
#define EGUI_VIEW_ADORNER_DECORATOR_INSET_MAX  10

static egui_view_adorner_decorator_t *egui_view_adorner_decorator_local(egui_view_t *self)
{
    return (egui_view_adorner_decorator_t *)self;
}

static uint8_t egui_view_adorner_decorator_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static egui_dim_t egui_view_adorner_decorator_clamp_dim(egui_dim_t value, egui_dim_t max_value)
{
    if (value < 0)
    {
        return 0;
    }
    if (value > max_value)
    {
        return max_value;
    }
    return value;
}

static egui_color_t egui_view_adorner_decorator_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x8A97A5), 56);
}

static void egui_view_adorner_decorator_get_body_region(egui_view_t *self, egui_region_t *region)
{
    region->location.x = 0;
    region->location.y = 0;
    region->size.width = self->region.size.width;
    region->size.height = self->region.size.height;
}

static void egui_view_adorner_decorator_get_child_region(egui_view_t *self, egui_region_t *region)
{
    egui_view_adorner_decorator_t *local = egui_view_adorner_decorator_local(self);

    if (local->child != NULL)
    {
        egui_region_copy(region, &local->child->region);
        return;
    }

    region->location.x = self->padding.left;
    region->location.y = self->padding.top;
    region->size.width = self->region.size.width - self->padding.left - self->padding.right;
    region->size.height = self->region.size.height - self->padding.top - self->padding.bottom;
}

static egui_region_t egui_view_adorner_decorator_expand_region(const egui_region_t *region, egui_dim_t inset)
{
    egui_region_t expanded = *region;

    expanded.location.x -= inset;
    expanded.location.y -= inset;
    expanded.size.width += inset * 2;
    expanded.size.height += inset * 2;
    return expanded;
}

static void egui_view_adorner_decorator_draw_corner_handle(egui_dim_t x, egui_dim_t y, egui_dim_t size, egui_color_t color, egui_alpha_t alpha)
{
    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, x, y, size, size, 2, color, alpha);
}

static void egui_view_adorner_decorator_draw_resize_handles(const egui_region_t *region, egui_color_t color, egui_alpha_t alpha)
{
    egui_dim_t handle = region->size.width < 70 ? 5 : 6;
    egui_dim_t right = region->location.x + region->size.width - handle;
    egui_dim_t bottom = region->location.y + region->size.height - handle;

    egui_view_adorner_decorator_draw_corner_handle(region->location.x, region->location.y, handle, color, alpha);
    egui_view_adorner_decorator_draw_corner_handle(right, region->location.y, handle, color, alpha);
    egui_view_adorner_decorator_draw_corner_handle(region->location.x, bottom, handle, color, alpha);
    egui_view_adorner_decorator_draw_corner_handle(right, bottom, handle, color, alpha);
}

static void egui_view_adorner_decorator_on_draw(egui_view_t *self)
{
    egui_view_adorner_decorator_t *local = egui_view_adorner_decorator_local(self);
    egui_region_t body_region;
    egui_region_t child_region;
    egui_region_t layer_region;
    egui_color_t surface_color = local->surface_color;
    egui_color_t child_surface_color = local->child_surface_color;
    egui_color_t child_border_color = local->child_border_color;
    egui_color_t focus_color = local->focus_color;
    egui_color_t validation_color = local->validation_color;
    egui_color_t resize_color = local->resize_color;
    egui_dim_t radius = local->compact_mode && local->corner_radius > 8 ? 8 : local->corner_radius;
    egui_alpha_t adorner_alpha = local->compact_mode ? 76 : 92;

    egui_view_adorner_decorator_get_body_region(self, &body_region);
    egui_view_adorner_decorator_get_child_region(self, &child_region);
    if (body_region.size.width <= 0 || body_region.size.height <= 0 || child_region.size.width <= 0 || child_region.size.height <= 0)
    {
        return;
    }

    if (local->read_only_mode)
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xF7F9FB), 42);
        child_surface_color = egui_rgb_mix(child_surface_color, EGUI_COLOR_HEX(0xF7F9FB), 46);
        child_border_color = egui_rgb_mix(child_border_color, EGUI_COLOR_HEX(0xAEB8C2), 50);
        focus_color = egui_rgb_mix(focus_color, EGUI_COLOR_HEX(0x8A97A5), 60);
        validation_color = egui_rgb_mix(validation_color, EGUI_COLOR_HEX(0x8A97A5), 70);
        resize_color = egui_rgb_mix(resize_color, EGUI_COLOR_HEX(0x8A97A5), 58);
        adorner_alpha = 54;
    }
    if (!egui_view_get_enable(self))
    {
        surface_color = egui_view_adorner_decorator_mix_disabled(surface_color);
        child_surface_color = egui_view_adorner_decorator_mix_disabled(child_surface_color);
        child_border_color = egui_view_adorner_decorator_mix_disabled(child_border_color);
        focus_color = egui_view_adorner_decorator_mix_disabled(focus_color);
        validation_color = egui_view_adorner_decorator_mix_disabled(validation_color);
        resize_color = egui_view_adorner_decorator_mix_disabled(resize_color);
        adorner_alpha = 42;
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, body_region.location.x, body_region.location.y, body_region.size.width,
                                          body_region.size.height, radius + 2, surface_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, body_region.location.x, body_region.location.y, body_region.size.width,
                                     body_region.size.height, radius + 2, 1, child_border_color, egui_color_alpha_mix(self->alpha, 34));

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, child_region.location.x, child_region.location.y, child_region.size.width,
                                          child_region.size.height, radius, child_surface_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, child_region.location.x, child_region.location.y, child_region.size.width,
                                     child_region.size.height, radius, 1, child_border_color, egui_color_alpha_mix(self->alpha, 72));

    layer_region = egui_view_adorner_decorator_expand_region(&child_region, local->layer_inset);
    if (local->adorner_flags & EGUI_VIEW_ADORNER_DECORATOR_ADORNER_FOCUS)
    {
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, layer_region.location.x, layer_region.location.y, layer_region.size.width,
                                         layer_region.size.height, radius + local->layer_inset, 1, focus_color,
                                         egui_color_alpha_mix(self->alpha, adorner_alpha));
    }
    if (local->adorner_flags & EGUI_VIEW_ADORNER_DECORATOR_ADORNER_VALIDATION)
    {
        egui_dim_t marker_x = layer_region.location.x + layer_region.size.width - 12;
        egui_dim_t marker_y = layer_region.location.y + 4;

        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, layer_region.location.x, layer_region.location.y, layer_region.size.width,
                                         layer_region.size.height, radius + local->layer_inset, 2, validation_color,
                                         egui_color_alpha_mix(self->alpha, adorner_alpha));
        egui_canvas_draw_circle_fill_basic(&uicode_get_core()->canvas, marker_x, marker_y, 3, validation_color,
                                           egui_color_alpha_mix(self->alpha, adorner_alpha));
    }
    if (local->adorner_flags & EGUI_VIEW_ADORNER_DECORATOR_ADORNER_RESIZE)
    {
        egui_view_adorner_decorator_draw_resize_handles(&layer_region, resize_color, egui_color_alpha_mix(self->alpha, adorner_alpha));
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_adorner_decorator_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_adorner_decorator_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_adorner_decorator_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_adorner_decorator_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_adorner_decorator_set_child(egui_view_t *self, egui_view_t *child)
{
    egui_view_adorner_decorator_t *local = egui_view_adorner_decorator_local(self);

    if (local->child == child)
    {
        return;
    }

    if (local->child != NULL)
    {
        egui_view_group_remove_child(self, local->child);
    }

    local->child = child;
    if (child != NULL)
    {
        egui_view_group_add_child(self, child);
        egui_view_adorner_decorator_layout_child(self);
    }
    egui_view_invalidate(self);
}

egui_view_t *egui_view_adorner_decorator_get_child(egui_view_t *self)
{
    return egui_view_adorner_decorator_local(self)->child;
}

void egui_view_adorner_decorator_layout_child(egui_view_t *self)
{
    egui_view_adorner_decorator_t *local = egui_view_adorner_decorator_local(self);

    if (local->child == NULL)
    {
        return;
    }
    egui_view_group_layout_childs(self, 0, 0, 0, EGUI_ALIGN_CENTER);
}

void egui_view_adorner_decorator_set_padding(egui_view_t *self, egui_dim_margin_padding_t left, egui_dim_margin_padding_t right, egui_dim_margin_padding_t top,
                                             egui_dim_margin_padding_t bottom)
{
    egui_view_adorner_decorator_clear_pressed_state(self);
    egui_view_set_padding(self, left, right, top, bottom);
    egui_view_adorner_decorator_layout_child(self);
    egui_view_invalidate(self);
}

void egui_view_adorner_decorator_set_adorner_flags(egui_view_t *self, uint8_t adorner_flags)
{
    egui_view_adorner_decorator_t *local = egui_view_adorner_decorator_local(self);

    egui_view_adorner_decorator_clear_pressed_state(self);
    local->adorner_flags = adorner_flags & (EGUI_VIEW_ADORNER_DECORATOR_ADORNER_FOCUS | EGUI_VIEW_ADORNER_DECORATOR_ADORNER_VALIDATION |
                                            EGUI_VIEW_ADORNER_DECORATOR_ADORNER_RESIZE);
    egui_view_invalidate(self);
}

uint8_t egui_view_adorner_decorator_get_adorner_flags(egui_view_t *self)
{
    return egui_view_adorner_decorator_local(self)->adorner_flags;
}

void egui_view_adorner_decorator_set_corner_radius(egui_view_t *self, egui_dim_t corner_radius)
{
    egui_view_adorner_decorator_t *local = egui_view_adorner_decorator_local(self);

    egui_view_adorner_decorator_clear_pressed_state(self);
    local->corner_radius = egui_view_adorner_decorator_clamp_dim(corner_radius, EGUI_VIEW_ADORNER_DECORATOR_RADIUS_MAX);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_adorner_decorator_get_corner_radius(egui_view_t *self)
{
    return egui_view_adorner_decorator_local(self)->corner_radius;
}

void egui_view_adorner_decorator_set_layer_inset(egui_view_t *self, egui_dim_t layer_inset)
{
    egui_view_adorner_decorator_t *local = egui_view_adorner_decorator_local(self);

    egui_view_adorner_decorator_clear_pressed_state(self);
    local->layer_inset = egui_view_adorner_decorator_clamp_dim(layer_inset, EGUI_VIEW_ADORNER_DECORATOR_INSET_MAX);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_adorner_decorator_get_layer_inset(egui_view_t *self)
{
    return egui_view_adorner_decorator_local(self)->layer_inset;
}

void egui_view_adorner_decorator_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t child_surface_color, egui_color_t child_border_color,
                                             egui_color_t focus_color, egui_color_t validation_color, egui_color_t resize_color)
{
    egui_view_adorner_decorator_t *local = egui_view_adorner_decorator_local(self);

    egui_view_adorner_decorator_clear_pressed_state(self);
    local->surface_color = surface_color;
    local->child_surface_color = child_surface_color;
    local->child_border_color = child_border_color;
    local->focus_color = focus_color;
    local->validation_color = validation_color;
    local->resize_color = resize_color;
    egui_view_invalidate(self);
}

void egui_view_adorner_decorator_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    egui_view_adorner_decorator_t *local = egui_view_adorner_decorator_local(self);

    egui_view_adorner_decorator_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_adorner_decorator_get_compact_mode(egui_view_t *self)
{
    return egui_view_adorner_decorator_local(self)->compact_mode;
}

void egui_view_adorner_decorator_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    egui_view_adorner_decorator_t *local = egui_view_adorner_decorator_local(self);

    egui_view_adorner_decorator_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_adorner_decorator_get_read_only_mode(egui_view_t *self)
{
    return egui_view_adorner_decorator_local(self)->read_only_mode;
}

void egui_view_adorner_decorator_apply_standard_style(egui_view_t *self)
{
    egui_view_adorner_decorator_set_palette(self, EGUI_COLOR_HEX(0xF5F8FB), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xC6D2DE), EGUI_COLOR_HEX(0x2563EB),
                                            EGUI_COLOR_HEX(0xC42B1C), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_adorner_decorator_set_corner_radius(self, 10);
    egui_view_adorner_decorator_set_layer_inset(self, 4);
    egui_view_adorner_decorator_set_padding(self, 18, 18, 16, 16);
    egui_view_adorner_decorator_set_adorner_flags(self, EGUI_VIEW_ADORNER_DECORATOR_ADORNER_FOCUS);
    egui_view_adorner_decorator_set_compact_mode(self, 0);
    egui_view_adorner_decorator_set_read_only_mode(self, 0);
}

void egui_view_adorner_decorator_apply_validation_style(egui_view_t *self)
{
    egui_view_adorner_decorator_set_palette(self, EGUI_COLOR_HEX(0xFFF8F6), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xE5B8B2), EGUI_COLOR_HEX(0x2563EB),
                                            EGUI_COLOR_HEX(0xC42B1C), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_adorner_decorator_set_corner_radius(self, 10);
    egui_view_adorner_decorator_set_layer_inset(self, 4);
    egui_view_adorner_decorator_set_padding(self, 18, 18, 16, 16);
    egui_view_adorner_decorator_set_adorner_flags(self, EGUI_VIEW_ADORNER_DECORATOR_ADORNER_VALIDATION);
    egui_view_adorner_decorator_set_compact_mode(self, 0);
    egui_view_adorner_decorator_set_read_only_mode(self, 0);
}

void egui_view_adorner_decorator_apply_resize_style(egui_view_t *self)
{
    egui_view_adorner_decorator_set_palette(self, EGUI_COLOR_HEX(0xF5FAF9), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xBBDAD5), EGUI_COLOR_HEX(0x2563EB),
                                            EGUI_COLOR_HEX(0xC42B1C), EGUI_COLOR_HEX(0x0C7C73));
    egui_view_adorner_decorator_set_corner_radius(self, 10);
    egui_view_adorner_decorator_set_layer_inset(self, 5);
    egui_view_adorner_decorator_set_padding(self, 18, 18, 16, 16);
    egui_view_adorner_decorator_set_adorner_flags(self, EGUI_VIEW_ADORNER_DECORATOR_ADORNER_FOCUS | EGUI_VIEW_ADORNER_DECORATOR_ADORNER_RESIZE);
    egui_view_adorner_decorator_set_compact_mode(self, 0);
    egui_view_adorner_decorator_set_read_only_mode(self, 0);
}

void egui_view_adorner_decorator_apply_compact_style(egui_view_t *self)
{
    egui_view_adorner_decorator_set_palette(self, EGUI_COLOR_HEX(0xF7FAFA), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD2DBE3), EGUI_COLOR_HEX(0x0C7C73),
                                            EGUI_COLOR_HEX(0xC42B1C), EGUI_COLOR_HEX(0x0C7C73));
    egui_view_adorner_decorator_set_corner_radius(self, 6);
    egui_view_adorner_decorator_set_layer_inset(self, 3);
    egui_view_adorner_decorator_set_padding(self, 10, 10, 8, 8);
    egui_view_adorner_decorator_set_adorner_flags(self, EGUI_VIEW_ADORNER_DECORATOR_ADORNER_FOCUS);
    egui_view_adorner_decorator_set_compact_mode(self, 1);
    egui_view_adorner_decorator_set_read_only_mode(self, 0);
}

void egui_view_adorner_decorator_apply_read_only_style(egui_view_t *self)
{
    egui_view_adorner_decorator_set_palette(self, EGUI_COLOR_HEX(0xF7F9FB), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD7DFE7), EGUI_COLOR_HEX(0x8A97A5),
                                            EGUI_COLOR_HEX(0x8A97A5), EGUI_COLOR_HEX(0x8A97A5));
    egui_view_adorner_decorator_set_corner_radius(self, 6);
    egui_view_adorner_decorator_set_layer_inset(self, 3);
    egui_view_adorner_decorator_set_padding(self, 10, 10, 8, 8);
    egui_view_adorner_decorator_set_adorner_flags(self, EGUI_VIEW_ADORNER_DECORATOR_ADORNER_RESIZE);
    egui_view_adorner_decorator_set_compact_mode(self, 1);
    egui_view_adorner_decorator_set_read_only_mode(self, 1);
}

void egui_view_adorner_decorator_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_adorner_decorator_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_adorner_decorator_on_static_key_event;
#endif
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_adorner_decorator_t) = {
        .dispatch_touch_event = egui_view_group_dispatch_touch_event,
        .on_touch_event = egui_view_group_on_touch_event,
        .on_intercept_touch_event = egui_view_group_on_intercept_touch_event,
        .compute_scroll = egui_view_group_compute_scroll,
        .calculate_layout = egui_view_group_calculate_layout,
        .request_layout = egui_view_group_request_layout,
        .draw = egui_view_group_draw,
        .on_attach_to_window = egui_view_group_on_attach_to_window,
        .on_draw = egui_view_adorner_decorator_on_draw,
        .on_detach_from_window = egui_view_group_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_group_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

void egui_view_adorner_decorator_init(egui_view_t *self)
{
    egui_view_adorner_decorator_t *local = egui_view_adorner_decorator_local(self);

    egui_view_group_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_adorner_decorator_t);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->child = NULL;
    local->surface_color = EGUI_COLOR_HEX(0xF5F8FB);
    local->child_surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->child_border_color = EGUI_COLOR_HEX(0xC6D2DE);
    local->focus_color = EGUI_COLOR_HEX(0x2563EB);
    local->validation_color = EGUI_COLOR_HEX(0xC42B1C);
    local->resize_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->corner_radius = 10;
    local->layer_inset = 4;
    local->adorner_flags = EGUI_VIEW_ADORNER_DECORATOR_ADORNER_FOCUS;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    egui_view_set_padding(self, 18, 18, 16, 16);
    egui_view_set_view_name(self, "egui_view_adorner_decorator");
}
