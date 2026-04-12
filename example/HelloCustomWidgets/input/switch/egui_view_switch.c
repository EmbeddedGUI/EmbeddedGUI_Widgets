#include "egui_view_switch.h"

static egui_view_switch_t *hcw_switch_local(egui_view_t *self)
{
    return (egui_view_switch_t *)self;
}

static uint8_t hcw_switch_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static void hcw_switch_apply_palette(egui_view_t *self, egui_color_t track_on, egui_color_t track_off, egui_color_t thumb_on, egui_color_t thumb_off,
                                     egui_alpha_t alpha)
{
    egui_view_switch_t *local = hcw_switch_local(self);
    uint8_t had_pressed = hcw_switch_clear_pressed_state(self);

    local->bk_color_on = track_on;
    local->bk_color_off = track_off;
    local->switch_color_on = thumb_on;
    local->switch_color_off = thumb_off;
    local->alpha = alpha;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

static void hcw_switch_on_draw(egui_view_t *self)
{
    egui_view_switch_t *local = hcw_switch_local(self);
    egui_region_t region;
    egui_dim_t radius;

    egui_view_switch_on_draw(self);

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (!self->is_focused || !egui_view_get_enable(self))
    {
        return;
    }

    egui_view_get_work_region(self, &region);
    radius = region.size.height / 2;
    if (radius < 8)
    {
        radius = 8;
    }

    egui_canvas_draw_round_rectangle(region.location.x - 1, region.location.y - 1, region.size.width + 2, region.size.height + 2, radius + 1, 1,
                                     local->bk_color_on, egui_color_alpha_mix(local->alpha, 56));
#else
    EGUI_UNUSED(local);
#endif
}

void hcw_switch_apply_standard_style(egui_view_t *self)
{
    hcw_switch_apply_palette(self, EGUI_COLOR_HEX(0x2563EB), EGUI_COLOR_HEX(0xD8E0E9), EGUI_COLOR_WHITE, EGUI_COLOR_WHITE, EGUI_ALPHA_100);
}

void hcw_switch_apply_compact_style(egui_view_t *self)
{
    hcw_switch_apply_palette(self, EGUI_COLOR_HEX(0x0C7C73), EGUI_COLOR_HEX(0xD9E5DE), EGUI_COLOR_WHITE, EGUI_COLOR_WHITE, EGUI_ALPHA_100);
}

void hcw_switch_apply_read_only_style(egui_view_t *self)
{
    hcw_switch_apply_palette(self, EGUI_COLOR_HEX(0xAFB8C3), EGUI_COLOR_HEX(0xE4E9EE), EGUI_COLOR_HEX(0xF7F9FB), EGUI_COLOR_HEX(0xF7F9FB),
                             EGUI_ALPHA_100);
}

void hcw_switch_set_checked(egui_view_t *self, uint8_t is_checked)
{
    hcw_switch_clear_pressed_state(self);
    egui_view_switch_set_checked(self, is_checked ? 1 : 0);
}

void hcw_switch_set_state_icons(egui_view_t *self, const char *icon_on, const char *icon_off)
{
    hcw_switch_clear_pressed_state(self);
    egui_view_switch_set_state_icons(self, icon_on, icon_off);
}

void hcw_switch_set_icon_font(egui_view_t *self, const egui_font_t *font)
{
    hcw_switch_clear_pressed_state(self);
    egui_view_switch_set_icon_font(self, font);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_switch_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_switch_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_switch_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_switch_clear_pressed_state(self);
    return 1;
}
#endif

void hcw_switch_override_interaction_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
    api->on_draw = hcw_switch_on_draw;
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_switch_on_key_event;
#endif
}

void hcw_switch_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
    api->on_draw = hcw_switch_on_draw;
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_switch_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_switch_on_static_key_event;
#endif
}

int hcw_switch_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    egui_view_switch_t *local = hcw_switch_local(self);
    uint8_t was_pressed = hcw_switch_clear_pressed_state(self);

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
                hcw_switch_set_checked(self, !local->is_checked);
            }
            return 1;
        }
        return 0;
    default:
        return egui_view_on_key_event(self, event);
    }
}
