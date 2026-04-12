#include "egui_view_image_icon.h"

#include "resource/app_egui_resource_generate.h"

static uint8_t egui_view_image_icon_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

const egui_image_t *egui_view_image_icon_get_default_image(void)
{
    return (const egui_image_t *)&egui_res_image_image_icon_default_rgb565_8;
}

const egui_image_t *egui_view_image_icon_get_warm_image(void)
{
    return (const egui_image_t *)&egui_res_image_image_icon_warm_rgb565_8;
}

const egui_image_t *egui_view_image_icon_get_fresh_image(void)
{
    return (const egui_image_t *)&egui_res_image_image_icon_fresh_rgb565_8;
}

void egui_view_image_icon_set_image(egui_view_t *self, const egui_image_t *image)
{
    egui_view_image_icon_clear_pressed_state(self);
    egui_view_image_set_image(self, (egui_image_t *)(image != NULL ? image : egui_view_image_icon_get_default_image()));
}

const egui_image_t *egui_view_image_icon_get_image(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_image_icon_t);

    return local->image != NULL ? local->image : egui_view_image_icon_get_default_image();
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_image_icon_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_image_icon_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_image_icon_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_image_icon_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_image_icon_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_image_icon_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_image_icon_on_static_key_event;
#endif
}

void egui_view_image_icon_init(egui_view_t *self)
{
    egui_view_image_init(self);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);
    egui_view_image_set_image_type(self, EGUI_VIEW_IMAGE_TYPE_RESIZE);
    egui_view_image_set_image_color(self, EGUI_COLOR_BLACK, 0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif
    egui_view_image_icon_set_image(self, egui_view_image_icon_get_default_image());
    egui_view_set_view_name(self, "egui_view_image_icon");
}
