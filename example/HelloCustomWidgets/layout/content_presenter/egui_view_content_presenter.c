#include "egui_view_content_presenter.h"

#define EGUI_VIEW_CONTENT_PRESENTER_RADIUS_MAX 18
#define EGUI_VIEW_CONTENT_PRESENTER_WIDTH_MAX  3

static egui_view_content_presenter_t *egui_view_content_presenter_local(egui_view_t *self)
{
    return (egui_view_content_presenter_t *)self;
}

static uint8_t egui_view_content_presenter_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static egui_color_t egui_view_content_presenter_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_MAKE(40));
}

static egui_dim_t egui_view_content_presenter_clamp_radius(egui_dim_t radius)
{
    if (radius < 0)
    {
        return 0;
    }
    if (radius > EGUI_VIEW_CONTENT_PRESENTER_RADIUS_MAX)
    {
        return EGUI_VIEW_CONTENT_PRESENTER_RADIUS_MAX;
    }
    return radius;
}

static egui_dim_t egui_view_content_presenter_clamp_width(egui_dim_t width)
{
    if (width < 0)
    {
        return 0;
    }
    if (width > EGUI_VIEW_CONTENT_PRESENTER_WIDTH_MAX)
    {
        return EGUI_VIEW_CONTENT_PRESENTER_WIDTH_MAX;
    }
    return width;
}

static void egui_view_content_presenter_on_draw(egui_view_t *self)
{
    egui_view_content_presenter_t *local = egui_view_content_presenter_local(self);
    egui_region_t region;
    egui_color_t surface_color = local->surface_color;
    egui_color_t guide_color = local->guide_color;
    egui_color_t accent_color = local->accent_color;
    egui_dim_t radius = local->corner_radius;
    egui_alpha_t guide_alpha = EGUI_ALPHA_MAKE(local->compact_mode ? 54 : 72);
    egui_alpha_t accent_alpha = EGUI_ALPHA_MAKE(local->compact_mode ? 30 : 38);

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
        guide_color = egui_rgb_mix(guide_color, HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_MAKE(32));
        accent_color = egui_rgb_mix(accent_color, HCW_COLOR_TEXT_MUTED, EGUI_ALPHA_MAKE(34));
        guide_alpha = EGUI_ALPHA_MAKE(62);
        accent_alpha = EGUI_ALPHA_MAKE(30);
    }
    if (!egui_view_get_enable(self))
    {
        surface_color = egui_view_content_presenter_mix_disabled(surface_color);
        guide_color = egui_view_content_presenter_mix_disabled(guide_color);
        accent_color = egui_view_content_presenter_mix_disabled(accent_color);
        guide_alpha = EGUI_ALPHA_MAKE(54);
        accent_alpha = EGUI_ALPHA_MAKE(26);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height, radius,
                                          surface_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));

    if (local->guide_width > 0)
    {
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height, radius,
                                         local->guide_width, guide_color, egui_color_alpha_mix(self->alpha, guide_alpha));
    }

    if (region.size.width > 18 && region.size.height > 14)
    {
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region.location.x + 7, region.location.y + region.size.height - 8,
                                              region.size.width - 14, local->compact_mode ? 2 : 3, 1, accent_color,
                                              egui_color_alpha_mix(self->alpha, accent_alpha));
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_content_presenter_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_content_presenter_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_content_presenter_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_content_presenter_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_content_presenter_set_child(egui_view_t *self, egui_view_t *child)
{
    egui_view_content_presenter_t *local = egui_view_content_presenter_local(self);

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
        egui_view_content_presenter_layout_child(self);
    }
    egui_view_invalidate(self);
}

egui_view_t *egui_view_content_presenter_get_child(egui_view_t *self)
{
    egui_view_content_presenter_t *local = egui_view_content_presenter_local(self);

    return local->child;
}

void egui_view_content_presenter_layout_child(egui_view_t *self)
{
    egui_view_content_presenter_t *local = egui_view_content_presenter_local(self);

    if (local->child == NULL)
    {
        return;
    }
    egui_view_group_layout_childs(self, 0, 0, 0, local->content_align_type);
}

void egui_view_content_presenter_set_padding(egui_view_t *self, egui_dim_margin_padding_t left, egui_dim_margin_padding_t right,
                                             egui_dim_margin_padding_t top, egui_dim_margin_padding_t bottom)
{
    egui_view_content_presenter_clear_pressed_state(self);
    egui_view_set_padding(self, left, right, top, bottom);
    egui_view_content_presenter_layout_child(self);
    egui_view_invalidate(self);
}

void egui_view_content_presenter_set_content_align_type(egui_view_t *self, uint8_t align_type)
{
    egui_view_content_presenter_t *local = egui_view_content_presenter_local(self);

    egui_view_content_presenter_clear_pressed_state(self);
    local->content_align_type = align_type;
    egui_view_content_presenter_layout_child(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_content_presenter_get_content_align_type(egui_view_t *self)
{
    egui_view_content_presenter_t *local = egui_view_content_presenter_local(self);

    return local->content_align_type;
}

void egui_view_content_presenter_set_corner_radius(egui_view_t *self, egui_dim_t radius)
{
    egui_view_content_presenter_t *local = egui_view_content_presenter_local(self);

    egui_view_content_presenter_clear_pressed_state(self);
    local->corner_radius = egui_view_content_presenter_clamp_radius(radius);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_content_presenter_get_corner_radius(egui_view_t *self)
{
    egui_view_content_presenter_t *local = egui_view_content_presenter_local(self);

    return local->corner_radius;
}

void egui_view_content_presenter_set_guide_width(egui_view_t *self, egui_dim_t width)
{
    egui_view_content_presenter_t *local = egui_view_content_presenter_local(self);

    egui_view_content_presenter_clear_pressed_state(self);
    local->guide_width = egui_view_content_presenter_clamp_width(width);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_content_presenter_get_guide_width(egui_view_t *self)
{
    egui_view_content_presenter_t *local = egui_view_content_presenter_local(self);

    return local->guide_width;
}

void egui_view_content_presenter_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t guide_color,
                                             egui_color_t accent_color)
{
    egui_view_content_presenter_t *local = egui_view_content_presenter_local(self);

    egui_view_content_presenter_clear_pressed_state(self);
    local->surface_color = surface_color;
    local->guide_color = guide_color;
    local->accent_color = accent_color;
    egui_view_invalidate(self);
}

void egui_view_content_presenter_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    egui_view_content_presenter_t *local = egui_view_content_presenter_local(self);

    egui_view_content_presenter_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_content_presenter_get_compact_mode(egui_view_t *self)
{
    egui_view_content_presenter_t *local = egui_view_content_presenter_local(self);

    return local->compact_mode;
}

void egui_view_content_presenter_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    egui_view_content_presenter_t *local = egui_view_content_presenter_local(self);

    egui_view_content_presenter_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_content_presenter_get_read_only_mode(egui_view_t *self)
{
    egui_view_content_presenter_t *local = egui_view_content_presenter_local(self);

    return local->read_only_mode;
}

void egui_view_content_presenter_apply_standard_style(egui_view_t *self)
{
    egui_view_content_presenter_set_palette(self, HCW_COLOR_SURFACE, HCW_COLOR_BORDER_STRONG, HCW_COLOR_PRIMARY);
    egui_view_content_presenter_set_corner_radius(self, 8);
    egui_view_content_presenter_set_guide_width(self, 1);
    egui_view_content_presenter_set_padding(self, 12, 12, 10, 10);
    egui_view_content_presenter_set_content_align_type(self, EGUI_ALIGN_CENTER);
    egui_view_content_presenter_set_compact_mode(self, 0);
    egui_view_content_presenter_set_read_only_mode(self, 0);
}

void egui_view_content_presenter_apply_template_style(egui_view_t *self)
{
    egui_view_content_presenter_set_palette(self, HCW_COLOR_PANEL, HCW_COLOR_PRIMARY_SOFT, HCW_COLOR_PRIMARY);
    egui_view_content_presenter_set_corner_radius(self, 8);
    egui_view_content_presenter_set_guide_width(self, 1);
    egui_view_content_presenter_set_padding(self, 12, 12, 10, 10);
    egui_view_content_presenter_set_content_align_type(self, EGUI_ALIGN_LEFT | EGUI_ALIGN_TOP);
    egui_view_content_presenter_set_compact_mode(self, 0);
    egui_view_content_presenter_set_read_only_mode(self, 0);
}

void egui_view_content_presenter_apply_compact_style(egui_view_t *self)
{
    egui_view_content_presenter_set_palette(self, HCW_COLOR_SURFACE, HCW_COLOR_BORDER, HCW_COLOR_SUCCESS);
    egui_view_content_presenter_set_corner_radius(self, 6);
    egui_view_content_presenter_set_guide_width(self, 1);
    egui_view_content_presenter_set_padding(self, 8, 8, 6, 6);
    egui_view_content_presenter_set_content_align_type(self, EGUI_ALIGN_TOP_LEFT);
    egui_view_content_presenter_set_compact_mode(self, 1);
    egui_view_content_presenter_set_read_only_mode(self, 0);
}

void egui_view_content_presenter_apply_read_only_style(egui_view_t *self)
{
    egui_view_content_presenter_set_palette(self, HCW_COLOR_SURFACE_SUBTLE, HCW_COLOR_BORDER_STRONG, HCW_COLOR_TEXT_SOFT);
    egui_view_content_presenter_set_corner_radius(self, 6);
    egui_view_content_presenter_set_guide_width(self, 1);
    egui_view_content_presenter_set_padding(self, 8, 8, 6, 6);
    egui_view_content_presenter_set_content_align_type(self, EGUI_ALIGN_CENTER);
    egui_view_content_presenter_set_compact_mode(self, 1);
    egui_view_content_presenter_set_read_only_mode(self, 1);
}

void egui_view_content_presenter_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_content_presenter_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_content_presenter_on_static_key_event;
#endif
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_content_presenter_t) = {
        .dispatch_touch_event = egui_view_group_dispatch_touch_event,
        .on_touch_event = egui_view_group_on_touch_event,
        .on_intercept_touch_event = egui_view_group_on_intercept_touch_event,
        .compute_scroll = egui_view_group_compute_scroll,
        .calculate_layout = egui_view_group_calculate_layout,
        .request_layout = egui_view_group_request_layout,
        .draw = egui_view_group_draw,
        .on_attach_to_window = egui_view_group_on_attach_to_window,
        .on_draw = egui_view_content_presenter_on_draw,
        .on_detach_from_window = egui_view_group_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_group_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

void egui_view_content_presenter_init(egui_view_t *self)
{
    egui_view_content_presenter_t *local = egui_view_content_presenter_local(self);

    egui_view_group_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_content_presenter_t);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->child = NULL;
    local->surface_color = HCW_COLOR_SURFACE;
    local->guide_color = HCW_COLOR_BORDER_STRONG;
    local->accent_color = HCW_COLOR_PRIMARY;
    local->corner_radius = 8;
    local->guide_width = 1;
    local->content_align_type = EGUI_ALIGN_CENTER;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    egui_view_set_padding(self, 12, 12, 10, 10);
    egui_view_set_view_name(self, "egui_view_content_presenter");
}
