#include "egui.h"
#include "egui_view_button.h"

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_button_standard_bg_normal_param, HCW_COLOR_PRIMARY, EGUI_ALPHA_100, 10, 1,
                                                        HCW_COLOR_PRIMARY_DARK, EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_button_standard_bg_pressed_param, HCW_COLOR_PRIMARY_DARK, EGUI_ALPHA_100, 10, 1,
                                                        HCW_COLOR_PRIMARY_DARK, EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_button_standard_bg_disabled_param, HCW_COLOR_DISABLED, EGUI_ALPHA_100, 10, 1,
                                                        HCW_COLOR_DISABLED, EGUI_ALPHA_100);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_button_standard_bg_focused_param, HCW_COLOR_PRIMARY, EGUI_ALPHA_100, 10, 2,
                                                        HCW_COLOR_PRIMARY_LIGHT, EGUI_ALPHA_100);
EGUI_BACKGROUND_PARAM_INIT_WITH_FOCUS(hcw_button_standard_bg_params, &hcw_button_standard_bg_normal_param, &hcw_button_standard_bg_pressed_param,
                                      &hcw_button_standard_bg_disabled_param, &hcw_button_standard_bg_focused_param);
#else
EGUI_BACKGROUND_PARAM_INIT(hcw_button_standard_bg_params, &hcw_button_standard_bg_normal_param, &hcw_button_standard_bg_pressed_param,
                           &hcw_button_standard_bg_disabled_param);
#endif
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(hcw_button_standard_background, &hcw_button_standard_bg_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_button_disabled_bg_normal_param, HCW_COLOR_SURFACE_DISABLED, EGUI_ALPHA_100, 8, 1,
                                                        HCW_COLOR_BORDER, EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_button_disabled_bg_pressed_param, HCW_COLOR_SURFACE_DISABLED, EGUI_ALPHA_100, 8, 1,
                                                        HCW_COLOR_BORDER, EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_button_disabled_bg_disabled_param, HCW_COLOR_SURFACE_DISABLED, EGUI_ALPHA_100, 8, 1,
                                                        HCW_COLOR_BORDER, EGUI_ALPHA_100);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_button_disabled_bg_focused_param, HCW_COLOR_SURFACE_DISABLED, EGUI_ALPHA_100, 8, 2,
                                                        HCW_COLOR_BORDER, EGUI_ALPHA_100);
EGUI_BACKGROUND_PARAM_INIT_WITH_FOCUS(hcw_button_disabled_bg_params, &hcw_button_disabled_bg_normal_param, &hcw_button_disabled_bg_pressed_param,
                                      &hcw_button_disabled_bg_disabled_param, &hcw_button_disabled_bg_focused_param);
#else
EGUI_BACKGROUND_PARAM_INIT(hcw_button_disabled_bg_params, &hcw_button_disabled_bg_normal_param, &hcw_button_disabled_bg_pressed_param,
                           &hcw_button_disabled_bg_disabled_param);
#endif
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(hcw_button_disabled_background, &hcw_button_disabled_bg_params);

static egui_view_button_t *hcw_button_local(egui_view_t *self)
{
    return (egui_view_button_t *)self;
}

static uint8_t hcw_button_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static void hcw_button_apply_style(egui_view_t *self, egui_background_t *background, egui_color_t text_color, egui_dim_t gap, egui_dim_t horizontal_padding)
{
    egui_view_button_t *local = hcw_button_local(self);
    uint8_t had_pressed = hcw_button_clear_pressed_state(self);

    egui_view_set_shadow(self, NULL);
    egui_view_set_background(self, background);
    self->padding.left = horizontal_padding;
    self->padding.right = horizontal_padding;
    self->padding.top = 0;
    self->padding.bottom = 0;
    local->base.color = text_color;
    local->base.alpha = EGUI_ALPHA_100;
    local->icon_text_gap = gap;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_button_apply_standard_style(egui_view_t *self)
{
    hcw_button_apply_style(self, EGUI_BG_OF(&hcw_button_standard_background), EGUI_COLOR_WHITE, 6, 12);
}

void hcw_button_apply_disabled_style(egui_view_t *self)
{
    hcw_button_apply_style(self, EGUI_BG_OF(&hcw_button_disabled_background), HCW_COLOR_NEUTRAL, 4, 10);
}

void hcw_button_set_text(egui_view_t *self, const char *text)
{
    hcw_button_clear_pressed_state(self);
    egui_view_label_set_text(self, text);
}

void hcw_button_set_icon(egui_view_t *self, const char *icon)
{
    hcw_button_clear_pressed_state(self);
    egui_view_button_set_icon(self, icon);
}

void hcw_button_set_icon_font(egui_view_t *self, const egui_font_t *font)
{
    hcw_button_clear_pressed_state(self);
    egui_view_button_set_icon_font(self, font);
}

void hcw_button_set_icon_text_gap(egui_view_t *self, egui_dim_t gap)
{
    hcw_button_clear_pressed_state(self);
    egui_view_button_set_icon_text_gap(self, gap);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_button_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_button_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_button_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_button_clear_pressed_state(self);
    return 1;
}
#endif

void hcw_button_override_interaction_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_button_on_key_event;
#endif
}

void hcw_button_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_button_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_button_on_static_key_event;
#endif
}

int hcw_button_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    uint8_t was_pressed = hcw_button_clear_pressed_state(self);

    if (!egui_view_get_enable(self))
    {
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_ENTER:
    case EGUI_KEY_CODE_SPACE:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            egui_view_set_pressed(self, 1);
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            if (was_pressed)
            {
                egui_view_perform_click(self);
            }
            return 1;
        }
        return 0;
    default:
        return egui_view_on_key_event(self, event);
    }
}
