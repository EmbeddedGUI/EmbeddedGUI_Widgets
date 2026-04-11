#include "egui_view_slider.h"

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_linear_value_helper.h"

extern const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_slider_t);

#define HCW_SLIDER_SMALL_STEP 5
#define HCW_SLIDER_LARGE_STEP 12

static egui_view_slider_t *hcw_slider_local(egui_view_t *self)
{
    return (egui_view_slider_t *)self;
}

static uint8_t hcw_slider_clear_interaction_state(egui_view_t *self)
{
    egui_view_slider_t *local = hcw_slider_local(self);
    uint8_t had_state = self->is_pressed || local->is_dragging;

    local->is_dragging = 0;
    if (self->is_pressed)
    {
        egui_view_set_pressed(self, 0);
    }
    else if (had_state)
    {
        egui_view_invalidate(self);
    }

    return had_state;
}

static void hcw_slider_apply_palette(egui_view_t *self, egui_dim_t pad_x, egui_dim_t pad_y, egui_color_t track_color, egui_color_t active_color,
                                     egui_color_t thumb_color)
{
    egui_view_slider_t *local = hcw_slider_local(self);

    hcw_slider_clear_interaction_state(self);
    egui_view_set_padding(self, pad_x, pad_x, pad_y, pad_y);
    local->track_color = track_color;
    local->active_color = active_color;
    local->thumb_color = thumb_color;
    egui_view_invalidate(self);
}

static uint8_t hcw_slider_shift_value(egui_view_t *self, int16_t delta)
{
    int16_t next_value = (int16_t)egui_view_slider_get_value(self) + delta;

    if (next_value < 0)
    {
        next_value = 0;
    }
    else if (next_value > 100)
    {
        next_value = 100;
    }

    hcw_slider_set_value(self, (uint8_t)next_value);
    return 1;
}

static void hcw_slider_on_draw(egui_view_t *self)
{
    egui_view_slider_t *local = hcw_slider_local(self);
    egui_region_t region;
    egui_view_linear_value_metrics_t metrics;
    egui_dim_t thumb_x;
    egui_dim_t thumb_y;
    egui_dim_t active_width;
    egui_dim_t outer_radius;
    egui_color_t track_color = local->track_color;
    egui_color_t active_color = local->active_color;
    egui_color_t thumb_color = local->thumb_color;
    egui_color_t shell_fill;
    egui_color_t shell_border;
    egui_color_t thumb_border;
    uint8_t compact_mode = self->padding.top <= 6 ? 1 : 0;

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0)
    {
        return;
    }

    if (!egui_view_linear_value_get_metrics(&region, 1, &metrics))
    {
        return;
    }

    if (!egui_view_get_enable(self))
    {
        track_color = egui_rgb_mix(track_color, EGUI_COLOR_DARK_GREY, 72);
        active_color = egui_rgb_mix(active_color, EGUI_COLOR_DARK_GREY, 72);
        thumb_color = egui_rgb_mix(thumb_color, EGUI_COLOR_DARK_GREY, 32);
    }

    shell_fill = egui_rgb_mix(track_color, EGUI_COLOR_WHITE, compact_mode ? 86 : 90);
    shell_border = egui_rgb_mix(track_color, active_color, compact_mode ? 18 : 24);
    thumb_border = egui_rgb_mix(shell_border, active_color, local->is_dragging ? 42 : 18);
    outer_radius = region.size.height / 2;
    if (outer_radius < 8)
    {
        outer_radius = 8;
    }

    egui_canvas_draw_round_rectangle_fill(region.location.x, region.location.y, region.size.width, region.size.height, outer_radius, shell_fill,
                                          egui_color_alpha_mix(self->alpha, compact_mode ? 94 : 100));
    egui_canvas_draw_round_rectangle(region.location.x, region.location.y, region.size.width, region.size.height, outer_radius, 1, shell_border,
                                     egui_color_alpha_mix(self->alpha, compact_mode ? 46 : 58));

    egui_canvas_draw_round_rectangle_fill(metrics.start_x, metrics.track_y, metrics.usable_width, metrics.track_height, metrics.track_radius, track_color,
                                          egui_color_alpha_mix(self->alpha, 100));

    thumb_x = egui_view_linear_value_get_x(&metrics, local->value);
    thumb_y = metrics.center_y;
    active_width = thumb_x - metrics.start_x;
    if (active_width > 0)
    {
        egui_canvas_draw_round_rectangle_fill(metrics.start_x, metrics.track_y, active_width, metrics.track_height, metrics.track_radius, active_color,
                                              egui_color_alpha_mix(self->alpha, 100));
    }

    if (self->is_focused && egui_view_get_enable(self))
    {
        egui_canvas_draw_round_rectangle(region.location.x - 1, region.location.y - 1, region.size.width + 2, region.size.height + 2, outer_radius, 1,
                                         active_color, egui_color_alpha_mix(self->alpha, 72));
    }

    if (self->is_pressed || local->is_dragging)
    {
        egui_canvas_draw_circle_fill_basic(thumb_x, thumb_y, metrics.knob_radius + 3, egui_rgb_mix(active_color, EGUI_COLOR_WHITE, 22),
                                           egui_color_alpha_mix(self->alpha, 32));
    }

    egui_canvas_draw_circle_fill_basic(thumb_x, thumb_y, metrics.knob_radius, thumb_color, egui_color_alpha_mix(self->alpha, 100));
    egui_canvas_draw_circle_basic(thumb_x, thumb_y, metrics.knob_radius, 1, thumb_border, egui_color_alpha_mix(self->alpha, 94));
}

void hcw_slider_apply_standard_style(egui_view_t *self)
{
    hcw_slider_apply_palette(self, 10, 8, EGUI_COLOR_HEX(0xD8E0E9), EGUI_COLOR_HEX(0x2563EB), EGUI_COLOR_WHITE);
}

void hcw_slider_apply_compact_style(egui_view_t *self)
{
    hcw_slider_apply_palette(self, 8, 6, EGUI_COLOR_HEX(0xD9E5DE), EGUI_COLOR_HEX(0x0C7C73), EGUI_COLOR_WHITE);
}

void hcw_slider_apply_read_only_style(egui_view_t *self)
{
    hcw_slider_apply_palette(self, 8, 6, EGUI_COLOR_HEX(0xE4E9EE), EGUI_COLOR_HEX(0xAFB8C3), EGUI_COLOR_HEX(0xF7F9FB));
}

void hcw_slider_set_value(egui_view_t *self, uint8_t value)
{
    hcw_slider_clear_interaction_state(self);
    egui_view_slider_set_value(self, value);
}

uint8_t hcw_slider_handle_navigation_key(egui_view_t *self, uint8_t key_code)
{
    if (!egui_view_get_enable(self))
    {
        hcw_slider_clear_interaction_state(self);
        return 0;
    }

    switch (key_code)
    {
    case EGUI_KEY_CODE_LEFT:
    case EGUI_KEY_CODE_DOWN:
        return hcw_slider_shift_value(self, -HCW_SLIDER_SMALL_STEP);
    case EGUI_KEY_CODE_RIGHT:
    case EGUI_KEY_CODE_UP:
        return hcw_slider_shift_value(self, HCW_SLIDER_SMALL_STEP);
    case EGUI_KEY_CODE_MINUS:
        return hcw_slider_shift_value(self, -HCW_SLIDER_LARGE_STEP);
    case EGUI_KEY_CODE_PLUS:
        return hcw_slider_shift_value(self, HCW_SLIDER_LARGE_STEP);
    case EGUI_KEY_CODE_HOME:
        hcw_slider_set_value(self, 0);
        return 1;
    case EGUI_KEY_CODE_END:
        hcw_slider_set_value(self, 100);
        return 1;
    default:
        return 0;
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_slider_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    egui_view_slider_t *local = hcw_slider_local(self);
    int is_inside = egui_region_pt_in_rect(&self->region_screen, event->location.x, event->location.y);

    if (!egui_view_get_enable(self))
    {
        hcw_slider_clear_interaction_state(self);
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        if (!is_inside)
        {
            hcw_slider_clear_interaction_state(self);
            return 0;
        }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        return EGUI_VIEW_API_TABLE_NAME(egui_view_slider_t).on_touch_event(self, event);
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (!local->is_dragging)
        {
            return 0;
        }
        return EGUI_VIEW_API_TABLE_NAME(egui_view_slider_t).on_touch_event(self, event);
    case EGUI_MOTION_EVENT_ACTION_UP:
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (!local->is_dragging && !self->is_pressed)
        {
            return 0;
        }
        return EGUI_VIEW_API_TABLE_NAME(egui_view_slider_t).on_touch_event(self, event);
    default:
        return 0;
    }
}

static int hcw_slider_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_slider_clear_interaction_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_slider_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    uint8_t was_pressed = hcw_slider_clear_interaction_state(self);

    if (!egui_view_get_enable(self))
    {
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_LEFT:
    case EGUI_KEY_CODE_RIGHT:
    case EGUI_KEY_CODE_UP:
    case EGUI_KEY_CODE_DOWN:
    case EGUI_KEY_CODE_HOME:
    case EGUI_KEY_CODE_END:
    case EGUI_KEY_CODE_MINUS:
    case EGUI_KEY_CODE_PLUS:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            egui_view_set_pressed(self, 1);
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            if (was_pressed)
            {
                hcw_slider_handle_navigation_key(self, event->key_code);
            }
            return 1;
        }
        return 0;
    default:
        return egui_view_on_key_event(self, event);
    }
}

static int hcw_slider_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_slider_clear_interaction_state(self);
    return 1;
}
#endif

void hcw_slider_override_interaction_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
    api->on_draw = hcw_slider_on_draw;
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_slider_on_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_slider_on_key_event;
#endif
}

void hcw_slider_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
    api->on_draw = hcw_slider_on_draw;
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_slider_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_slider_on_static_key_event;
#endif
}
