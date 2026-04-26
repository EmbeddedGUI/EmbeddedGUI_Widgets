#include "egui_view_resize_grip.h"

#define EGUI_VIEW_RESIZE_GRIP_SIZE_MIN       12
#define EGUI_VIEW_RESIZE_GRIP_SIZE_MAX       64
#define EGUI_VIEW_RESIZE_GRIP_DOT_SIZE_MIN   2
#define EGUI_VIEW_RESIZE_GRIP_DOT_SIZE_MAX   8
#define EGUI_VIEW_RESIZE_GRIP_DOT_GAP_MIN    1
#define EGUI_VIEW_RESIZE_GRIP_DOT_GAP_MAX    12
#define EGUI_VIEW_RESIZE_GRIP_RADIUS_MIN     2
#define EGUI_VIEW_RESIZE_GRIP_RADIUS_MAX     14

static egui_view_resize_grip_t *egui_view_resize_grip_local(egui_view_t *self)
{
    return (egui_view_resize_grip_t *)self;
}

static uint8_t egui_view_resize_grip_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static egui_dim_t egui_view_resize_grip_clamp(egui_dim_t value, egui_dim_t min_value, egui_dim_t max_value)
{
    if (value < min_value)
    {
        return min_value;
    }
    if (value > max_value)
    {
        return max_value;
    }
    return value;
}

static uint8_t egui_view_resize_grip_clamp_corner(uint8_t corner)
{
    if (corner > EGUI_VIEW_RESIZE_GRIP_CORNER_BOTTOM_LEFT)
    {
        return EGUI_VIEW_RESIZE_GRIP_CORNER_BOTTOM_RIGHT;
    }
    return corner;
}

static egui_color_t egui_view_resize_grip_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x8A97A5), 58);
}

void egui_view_resize_grip_get_grip_region(egui_view_t *self, egui_region_t *grip_region)
{
    egui_view_resize_grip_t *local = egui_view_resize_grip_local(self);
    egui_region_t work_region;
    egui_dim_t size = local->grip_size;

    if (grip_region == NULL)
    {
        return;
    }

    egui_view_get_work_region(self, &work_region);
    if (work_region.size.width < 0)
    {
        work_region.size.width = 0;
    }
    if (work_region.size.height < 0)
    {
        work_region.size.height = 0;
    }
    if (size > work_region.size.width)
    {
        size = work_region.size.width;
    }
    if (size > work_region.size.height)
    {
        size = work_region.size.height;
    }

    grip_region->location.y = work_region.location.y + work_region.size.height - size;
    grip_region->size.width = size;
    grip_region->size.height = size;
    if (local->corner == EGUI_VIEW_RESIZE_GRIP_CORNER_BOTTOM_LEFT)
    {
        grip_region->location.x = work_region.location.x;
    }
    else
    {
        grip_region->location.x = work_region.location.x + work_region.size.width - size;
    }
}

static void egui_view_resize_grip_draw_diagonal_marks(egui_view_t *self, egui_view_resize_grip_t *local, const egui_region_t *region,
                                                      egui_color_t color, egui_alpha_t alpha)
{
    egui_dim_t inset = local->compact_mode ? 6 : 8;
    egui_dim_t step = local->dot_size + local->dot_gap + 1;
    egui_dim_t right = region->location.x + region->size.width - inset;
    egui_dim_t left = region->location.x + inset;
    egui_dim_t bottom = region->location.y + region->size.height - inset;
    uint8_t index;

    if (region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    for (index = 0; index < 2; ++index)
    {
        egui_dim_t offset = (egui_dim_t)(index * step);

        if (local->corner == EGUI_VIEW_RESIZE_GRIP_CORNER_BOTTOM_LEFT)
        {
            egui_canvas_draw_line(&uicode_get_core()->canvas, left + offset, bottom, left, bottom - offset, 1, color,
                                  egui_color_alpha_mix(self->alpha, alpha));
        }
        else
        {
            egui_canvas_draw_line(&uicode_get_core()->canvas, right - offset, bottom, right, bottom - offset, 1, color,
                                  egui_color_alpha_mix(self->alpha, alpha));
        }
    }
}

static void egui_view_resize_grip_draw_dots(egui_view_t *self, egui_view_resize_grip_t *local, const egui_region_t *region, egui_color_t color,
                                            egui_alpha_t alpha)
{
    egui_dim_t step = local->dot_size + local->dot_gap;
    egui_dim_t radius = local->dot_size / 2;
    egui_dim_t inset = local->compact_mode ? 6 : 8;
    egui_dim_t anchor_x;
    egui_dim_t anchor_y;
    uint8_t row;

    if (region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    if (radius < 1)
    {
        radius = 1;
    }

    anchor_y = region->location.y + region->size.height - inset;
    if (local->corner == EGUI_VIEW_RESIZE_GRIP_CORNER_BOTTOM_LEFT)
    {
        anchor_x = region->location.x + inset;
    }
    else
    {
        anchor_x = region->location.x + region->size.width - inset;
    }

    for (row = 0; row < 3; ++row)
    {
        uint8_t col;

        for (col = 0; col <= row; ++col)
        {
            egui_dim_t x = anchor_x;
            egui_dim_t y = anchor_y - (egui_dim_t)(row * step);

            if (local->corner == EGUI_VIEW_RESIZE_GRIP_CORNER_BOTTOM_LEFT)
            {
                x += (egui_dim_t)(col * step);
            }
            else
            {
                x -= (egui_dim_t)(col * step);
            }
            egui_canvas_draw_circle_fill_basic(&uicode_get_core()->canvas, x, y, radius, color, egui_color_alpha_mix(self->alpha, alpha));
        }
    }
}

static void egui_view_resize_grip_on_draw(egui_view_t *self)
{
    egui_view_resize_grip_t *local = egui_view_resize_grip_local(self);
    egui_region_t region;
    egui_region_t grip_region;
    egui_color_t surface_color = local->surface_color;
    egui_color_t border_color = local->border_color;
    egui_color_t dot_color = local->dot_color;
    egui_color_t accent_color = local->accent_color;
    egui_alpha_t surface_alpha = local->compact_mode ? 74 : 90;
    egui_alpha_t dot_alpha = local->compact_mode ? 78 : 92;
    egui_alpha_t accent_alpha = local->compact_mode ? 26 : 36;

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0)
    {
        return;
    }

    if (local->read_only_mode || local->disabled_mode)
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xF5F7FA), 48);
        border_color = egui_rgb_mix(border_color, EGUI_COLOR_HEX(0xAEB8C2), 52);
        dot_color = egui_rgb_mix(dot_color, EGUI_COLOR_HEX(0x8A97A5), 50);
        accent_color = egui_rgb_mix(accent_color, EGUI_COLOR_HEX(0x8A97A5), 56);
        surface_alpha = 72;
        dot_alpha = local->disabled_mode ? 52 : 64;
        accent_alpha = 18;
    }
    if (!egui_view_get_enable(self))
    {
        surface_color = egui_view_resize_grip_mix_disabled(surface_color);
        border_color = egui_view_resize_grip_mix_disabled(border_color);
        dot_color = egui_view_resize_grip_mix_disabled(dot_color);
        accent_color = egui_view_resize_grip_mix_disabled(accent_color);
        surface_alpha = 58;
        dot_alpha = 46;
        accent_alpha = 14;
    }
    if (egui_view_get_pressed(self))
    {
        surface_color = egui_rgb_mix(surface_color, accent_color, 14);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height,
                                          local->corner_radius, surface_color, egui_color_alpha_mix(self->alpha, surface_alpha));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height,
                                     local->corner_radius, 1, border_color, egui_color_alpha_mix(self->alpha, local->compact_mode ? 28 : 42));

    egui_view_resize_grip_get_grip_region(self, &grip_region);
    egui_view_resize_grip_draw_diagonal_marks(self, local, &grip_region, accent_color, accent_alpha);
    egui_view_resize_grip_draw_dots(self, local, &grip_region, dot_color, dot_alpha);
}

void egui_view_resize_grip_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t dot_color,
                                       egui_color_t accent_color)
{
    egui_view_resize_grip_t *local = egui_view_resize_grip_local(self);

    egui_view_resize_grip_clear_pressed_state(self);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->dot_color = dot_color;
    local->accent_color = accent_color;
    egui_view_invalidate(self);
}

void egui_view_resize_grip_set_metrics(egui_view_t *self, egui_dim_t grip_size, egui_dim_t dot_size, egui_dim_t dot_gap)
{
    egui_view_resize_grip_t *local = egui_view_resize_grip_local(self);

    egui_view_resize_grip_clear_pressed_state(self);
    local->grip_size = egui_view_resize_grip_clamp(grip_size, EGUI_VIEW_RESIZE_GRIP_SIZE_MIN, EGUI_VIEW_RESIZE_GRIP_SIZE_MAX);
    local->dot_size = egui_view_resize_grip_clamp(dot_size, EGUI_VIEW_RESIZE_GRIP_DOT_SIZE_MIN, EGUI_VIEW_RESIZE_GRIP_DOT_SIZE_MAX);
    local->dot_gap = egui_view_resize_grip_clamp(dot_gap, EGUI_VIEW_RESIZE_GRIP_DOT_GAP_MIN, EGUI_VIEW_RESIZE_GRIP_DOT_GAP_MAX);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_resize_grip_get_grip_size(egui_view_t *self)
{
    egui_view_resize_grip_t *local = egui_view_resize_grip_local(self);

    return local->grip_size;
}

egui_dim_t egui_view_resize_grip_get_dot_size(egui_view_t *self)
{
    egui_view_resize_grip_t *local = egui_view_resize_grip_local(self);

    return local->dot_size;
}

egui_dim_t egui_view_resize_grip_get_dot_gap(egui_view_t *self)
{
    egui_view_resize_grip_t *local = egui_view_resize_grip_local(self);

    return local->dot_gap;
}

void egui_view_resize_grip_set_corner_radius(egui_view_t *self, egui_dim_t radius)
{
    egui_view_resize_grip_t *local = egui_view_resize_grip_local(self);

    egui_view_resize_grip_clear_pressed_state(self);
    local->corner_radius = egui_view_resize_grip_clamp(radius, EGUI_VIEW_RESIZE_GRIP_RADIUS_MIN, EGUI_VIEW_RESIZE_GRIP_RADIUS_MAX);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_resize_grip_get_corner_radius(egui_view_t *self)
{
    egui_view_resize_grip_t *local = egui_view_resize_grip_local(self);

    return local->corner_radius;
}

void egui_view_resize_grip_set_corner(egui_view_t *self, uint8_t corner)
{
    egui_view_resize_grip_t *local = egui_view_resize_grip_local(self);

    egui_view_resize_grip_clear_pressed_state(self);
    local->corner = egui_view_resize_grip_clamp_corner(corner);
    egui_view_invalidate(self);
}

uint8_t egui_view_resize_grip_get_corner(egui_view_t *self)
{
    egui_view_resize_grip_t *local = egui_view_resize_grip_local(self);

    return local->corner;
}

void egui_view_resize_grip_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    egui_view_resize_grip_t *local = egui_view_resize_grip_local(self);

    egui_view_resize_grip_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_resize_grip_get_compact_mode(egui_view_t *self)
{
    egui_view_resize_grip_t *local = egui_view_resize_grip_local(self);

    return local->compact_mode;
}

void egui_view_resize_grip_set_disabled_mode(egui_view_t *self, uint8_t disabled_mode)
{
    egui_view_resize_grip_t *local = egui_view_resize_grip_local(self);

    egui_view_resize_grip_clear_pressed_state(self);
    local->disabled_mode = disabled_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_resize_grip_get_disabled_mode(egui_view_t *self)
{
    egui_view_resize_grip_t *local = egui_view_resize_grip_local(self);

    return local->disabled_mode;
}

void egui_view_resize_grip_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    egui_view_resize_grip_t *local = egui_view_resize_grip_local(self);

    egui_view_resize_grip_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_resize_grip_get_read_only_mode(egui_view_t *self)
{
    egui_view_resize_grip_t *local = egui_view_resize_grip_local(self);

    return local->read_only_mode;
}

void egui_view_resize_grip_apply_standard_style(egui_view_t *self)
{
    egui_view_resize_grip_set_palette(self, EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DEE8), EGUI_COLOR_HEX(0x607080),
                                      EGUI_COLOR_HEX(0xC9D5E0));
    egui_view_resize_grip_set_metrics(self, 34, 4, 5);
    egui_view_resize_grip_set_corner_radius(self, 8);
    egui_view_resize_grip_set_compact_mode(self, 0);
    egui_view_resize_grip_set_disabled_mode(self, 0);
    egui_view_resize_grip_set_read_only_mode(self, 0);
}

void egui_view_resize_grip_apply_accent_style(egui_view_t *self)
{
    egui_view_resize_grip_set_palette(self, EGUI_COLOR_HEX(0xF7FBFF), EGUI_COLOR_HEX(0xB9D6F0), EGUI_COLOR_HEX(0x0F6CBD),
                                      EGUI_COLOR_HEX(0xCFE2F3));
    egui_view_resize_grip_set_metrics(self, 36, 4, 5);
    egui_view_resize_grip_set_corner_radius(self, 8);
    egui_view_resize_grip_set_compact_mode(self, 0);
    egui_view_resize_grip_set_disabled_mode(self, 0);
    egui_view_resize_grip_set_read_only_mode(self, 0);
}

void egui_view_resize_grip_apply_compact_style(egui_view_t *self)
{
    egui_view_resize_grip_set_palette(self, EGUI_COLOR_HEX(0xF8FBFD), EGUI_COLOR_HEX(0xD2DCE6), EGUI_COLOR_HEX(0x0C7C73),
                                      EGUI_COLOR_HEX(0xD9E7E5));
    egui_view_resize_grip_set_metrics(self, 24, 3, 4);
    egui_view_resize_grip_set_corner_radius(self, 6);
    egui_view_resize_grip_set_compact_mode(self, 1);
    egui_view_resize_grip_set_disabled_mode(self, 0);
    egui_view_resize_grip_set_read_only_mode(self, 0);
}

void egui_view_resize_grip_apply_disabled_style(egui_view_t *self)
{
    egui_view_resize_grip_set_palette(self, EGUI_COLOR_HEX(0xF5F7FA), EGUI_COLOR_HEX(0xD7DEE6), EGUI_COLOR_HEX(0x788593),
                                      EGUI_COLOR_HEX(0xE1E6EB));
    egui_view_resize_grip_set_metrics(self, 30, 3, 5);
    egui_view_resize_grip_set_corner_radius(self, 8);
    egui_view_resize_grip_set_compact_mode(self, 0);
    egui_view_resize_grip_set_disabled_mode(self, 1);
    egui_view_resize_grip_set_read_only_mode(self, 0);
}

void egui_view_resize_grip_apply_read_only_style(egui_view_t *self)
{
    egui_view_resize_grip_set_palette(self, EGUI_COLOR_HEX(0xF5F7FA), EGUI_COLOR_HEX(0xD7DEE6), EGUI_COLOR_HEX(0x687684),
                                      EGUI_COLOR_HEX(0xE1E6EB));
    egui_view_resize_grip_set_metrics(self, 24, 3, 4);
    egui_view_resize_grip_set_corner_radius(self, 6);
    egui_view_resize_grip_set_compact_mode(self, 1);
    egui_view_resize_grip_set_disabled_mode(self, 0);
    egui_view_resize_grip_set_read_only_mode(self, 1);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_resize_grip_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_resize_grip_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_resize_grip_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_resize_grip_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_resize_grip_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_resize_grip_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_resize_grip_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_resize_grip_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_resize_grip_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_resize_grip_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_resize_grip_on_static_key_event;
#endif
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_resize_grip_t) = {
        .draw = egui_view_draw,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_resize_grip_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .on_draw = egui_view_resize_grip_on_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_resize_grip_on_key_event,
#endif
};

void egui_view_resize_grip_init(egui_view_t *self)
{
    egui_view_resize_grip_t *local = egui_view_resize_grip_local(self);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_resize_grip_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->corner = EGUI_VIEW_RESIZE_GRIP_CORNER_BOTTOM_RIGHT;
    local->compact_mode = 0;
    local->disabled_mode = 0;
    local->read_only_mode = 0;
    egui_view_resize_grip_apply_standard_style(self);
    egui_view_set_view_name(self, "egui_view_resize_grip");
}
