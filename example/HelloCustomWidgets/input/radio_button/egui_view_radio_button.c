#include "egui_view_radio_button.h"

extern const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_radio_button_t);

static egui_view_radio_button_t *hcw_radio_button_local(egui_view_t *self)
{
    return (egui_view_radio_button_t *)self;
}

static uint8_t hcw_radio_button_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static void hcw_radio_button_apply_style(egui_view_t *self, egui_color_t circle_color, egui_color_t dot_color, egui_color_t text_color, egui_dim_t gap)
{
    egui_view_radio_button_t *local = hcw_radio_button_local(self);
    uint8_t had_pressed = hcw_radio_button_clear_pressed_state(self);

    egui_view_set_shadow(self, NULL);
    local->alpha = EGUI_ALPHA_100;
    local->circle_color = circle_color;
    local->dot_color = dot_color;
    local->text_color = text_color;
    local->text_gap = gap;
    local->mark_style = EGUI_VIEW_RADIO_BUTTON_MARK_STYLE_DOT;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

static void hcw_radio_button_on_draw(egui_view_t *self)
{
    egui_view_radio_button_t *local = hcw_radio_button_local(self);
    egui_region_t region;

    EGUI_VIEW_API_TABLE_NAME(egui_view_radio_button_t).on_draw(self);

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (!self->is_focused || !egui_view_get_enable(self))
    {
        return;
    }

    egui_view_get_work_region(self, &region);
    egui_canvas_draw_round_rectangle(region.location.x, region.location.y, region.size.width, region.size.height, 8, 1, local->dot_color,
                                     egui_color_alpha_mix(self->alpha, 56));
#else
    EGUI_UNUSED(local);
#endif
}

void hcw_radio_button_apply_standard_style(egui_view_t *self)
{
    hcw_radio_button_apply_style(self, EGUI_COLOR_HEX(0xC6D2DE), EGUI_COLOR_HEX(0x2563EB), EGUI_COLOR_HEX(0x1A2734), 8);
}

void hcw_radio_button_apply_compact_style(egui_view_t *self)
{
    hcw_radio_button_apply_style(self, EGUI_COLOR_HEX(0xC7D8CE), EGUI_COLOR_HEX(0x0C7C73), EGUI_COLOR_HEX(0x21303F), 6);
}

void hcw_radio_button_apply_read_only_style(egui_view_t *self)
{
    hcw_radio_button_apply_style(self, EGUI_COLOR_HEX(0xD8E0E8), EGUI_COLOR_HEX(0xAFB8C3), EGUI_COLOR_HEX(0x546474), 6);
}

void hcw_radio_button_set_checked(egui_view_t *self, uint8_t is_checked)
{
    hcw_radio_button_clear_pressed_state(self);
    egui_view_radio_button_set_checked(self, is_checked ? 1 : 0);
}

void hcw_radio_button_set_text(egui_view_t *self, const char *text)
{
    hcw_radio_button_clear_pressed_state(self);
    egui_view_radio_button_set_text(self, text);
}

void hcw_radio_button_set_font(egui_view_t *self, const egui_font_t *font)
{
    hcw_radio_button_clear_pressed_state(self);
    egui_view_radio_button_set_font(self, font);
}

void hcw_radio_button_set_text_color(egui_view_t *self, egui_color_t color)
{
    hcw_radio_button_clear_pressed_state(self);
    egui_view_radio_button_set_text_color(self, color);
}

void hcw_radio_button_set_mark_style(egui_view_t *self, egui_view_radio_button_mark_style_t style)
{
    hcw_radio_button_clear_pressed_state(self);
    egui_view_radio_button_set_mark_style(self, style);
}

void hcw_radio_button_set_mark_icon(egui_view_t *self, const char *icon)
{
    hcw_radio_button_clear_pressed_state(self);
    egui_view_radio_button_set_mark_icon(self, icon);
}

void hcw_radio_button_set_icon_font(egui_view_t *self, const egui_font_t *font)
{
    hcw_radio_button_clear_pressed_state(self);
    egui_view_radio_button_set_icon_font(self, font);
}

void hcw_radio_button_set_icon_text_gap(egui_view_t *self, egui_dim_t gap)
{
    hcw_radio_button_clear_pressed_state(self);
    egui_view_radio_button_set_icon_text_gap(self, gap);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_radio_button_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_radio_button_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_radio_button_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_radio_button_clear_pressed_state(self);
    return 1;
}
#endif

void hcw_radio_button_override_interaction_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
    api->on_draw = hcw_radio_button_on_draw;
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_radio_button_on_key_event;
#endif
}

void hcw_radio_button_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_radio_button_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_radio_button_on_static_key_event;
#endif
}

int hcw_radio_button_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    uint8_t was_pressed = hcw_radio_button_clear_pressed_state(self);

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
                hcw_radio_button_set_checked(self, 1);
            }
            return 1;
        }
        return 0;
    default:
        return egui_view_on_key_event(self, event);
    }
}
