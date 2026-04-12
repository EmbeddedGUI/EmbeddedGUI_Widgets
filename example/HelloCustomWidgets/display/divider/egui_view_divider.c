#include "egui.h"
#include "egui_view_divider.h"

static egui_view_divider_t *hcw_divider_local(egui_view_t *self)
{
    return (egui_view_divider_t *)self;
}

static void hcw_divider_clear_pressed_state(egui_view_t *self)
{
    egui_view_set_pressed(self, 0);
}

static void hcw_divider_apply_style(egui_view_t *self, egui_color_t color, egui_alpha_t alpha)
{
    egui_view_divider_t *local = hcw_divider_local(self);

    hcw_divider_clear_pressed_state(self);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding(self, 0, 0, 0, 0);
    local->color = color;
    local->alpha = alpha;
    egui_view_invalidate(self);
}

void hcw_divider_apply_standard_style(egui_view_t *self)
{
    hcw_divider_apply_style(self, EGUI_COLOR_HEX(0xC7D1DB), EGUI_ALPHA_100);
}

void hcw_divider_apply_subtle_style(egui_view_t *self)
{
    hcw_divider_apply_style(self, EGUI_COLOR_HEX(0xD9E1E8), 72);
}

void hcw_divider_apply_accent_style(egui_view_t *self)
{
    hcw_divider_apply_style(self, EGUI_COLOR_HEX(0x0F6CBD), EGUI_ALPHA_100);
}

void hcw_divider_set_palette(egui_view_t *self, egui_color_t color, egui_alpha_t alpha)
{
    hcw_divider_apply_style(self, color, alpha);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_divider_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_divider_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_divider_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_divider_clear_pressed_state(self);
    return 1;
}
#endif

void hcw_divider_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_divider_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_divider_on_static_key_event;
#endif
}
