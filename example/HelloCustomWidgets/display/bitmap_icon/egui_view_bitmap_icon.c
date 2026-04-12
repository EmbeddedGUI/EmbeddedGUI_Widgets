#include "egui_view_bitmap_icon.h"

#include "resource/app_egui_resource_generate.h"

static const uint8_t egui_view_bitmap_icon_document_mask[16 * 2] = {
        0xE0, 0x07,
        0x20, 0x0C,
        0x20, 0x18,
        0x20, 0x18,
        0x20, 0x18,
        0x20, 0x1C,
        0x20, 0x1A,
        0x20, 0x19,
        0xA0, 0x18,
        0x60, 0x18,
        0x20, 0x18,
        0x20, 0x18,
        0x20, 0x18,
        0x20, 0x18,
        0xE0, 0x1F,
        0x00, 0x00,
};

static const uint8_t egui_view_bitmap_icon_mail_mask[16 * 2] = {
        0x00, 0x00,
        0x00, 0x00,
        0xE0, 0x1F,
        0x20, 0x10,
        0xA0, 0x18,
        0x20, 0x15,
        0x20, 0x12,
        0x20, 0x15,
        0xA0, 0x18,
        0x60, 0x10,
        0x20, 0x10,
        0xE0, 0x1F,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
};

static const uint8_t egui_view_bitmap_icon_alert_mask[16 * 2] = {
        0x00, 0x01,
        0x80, 0x03,
        0xC0, 0x07,
        0xE0, 0x0E,
        0x70, 0x1C,
        0x38, 0x38,
        0x1C, 0x70,
        0x3C, 0x70,
        0x3C, 0x70,
        0x1C, 0x70,
        0x1C, 0x71,
        0x1C, 0x71,
        0x1C, 0x70,
        0x1C, 0x71,
        0xFC, 0x7F,
        0x00, 0x00,
};

static uint8_t egui_view_bitmap_icon_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static const egui_image_t *egui_view_bitmap_icon_default_image(void)
{
    return (const egui_image_t *)&egui_res_image_bitmap_icon_document_alpha_8;
}

const egui_image_t *egui_view_bitmap_icon_get_document_image(void)
{
    return (const egui_image_t *)&egui_res_image_bitmap_icon_document_alpha_8;
}

const egui_image_t *egui_view_bitmap_icon_get_mail_image(void)
{
    return (const egui_image_t *)&egui_res_image_bitmap_icon_mail_alpha_8;
}

const egui_image_t *egui_view_bitmap_icon_get_alert_image(void)
{
    return (const egui_image_t *)&egui_res_image_bitmap_icon_alert_alpha_8;
}

static void egui_view_bitmap_icon_apply_color(egui_view_t *self, egui_color_t icon_color)
{
    egui_view_bitmap_icon_clear_pressed_state(self);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);
    egui_view_image_set_image_color(self, icon_color, EGUI_ALPHA_100);
}

void egui_view_bitmap_icon_apply_standard_style(egui_view_t *self)
{
    egui_view_bitmap_icon_apply_color(self, EGUI_COLOR_HEX(0x0F6CBD));
}

void egui_view_bitmap_icon_apply_subtle_style(egui_view_t *self)
{
    egui_view_bitmap_icon_apply_color(self, EGUI_COLOR_HEX(0x6F7C8A));
}

void egui_view_bitmap_icon_apply_accent_style(egui_view_t *self)
{
    egui_view_bitmap_icon_apply_color(self, EGUI_COLOR_HEX(0xA15C00));
}

void egui_view_bitmap_icon_set_image(egui_view_t *self, const egui_image_t *image)
{
    egui_view_bitmap_icon_clear_pressed_state(self);
    egui_view_image_set_image(self, (egui_image_t *)(image != NULL ? image : egui_view_bitmap_icon_default_image()));
}

const egui_image_t *egui_view_bitmap_icon_get_image(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_bitmap_icon_t);

    return local->image != NULL ? local->image : egui_view_bitmap_icon_default_image();
}

void egui_view_bitmap_icon_set_palette(egui_view_t *self, egui_color_t icon_color)
{
    egui_view_bitmap_icon_apply_color(self, icon_color);
}

static const uint8_t *egui_view_bitmap_icon_resolve_mask(const egui_image_t *image)
{
    if (image == egui_view_bitmap_icon_get_mail_image())
    {
        return egui_view_bitmap_icon_mail_mask;
    }
    if (image == egui_view_bitmap_icon_get_alert_image())
    {
        return egui_view_bitmap_icon_alert_mask;
    }
    return egui_view_bitmap_icon_document_mask;
}

static uint8_t egui_view_bitmap_icon_mask_is_opaque(const uint8_t *mask, egui_dim_t x, egui_dim_t y)
{
    return (mask[y * 2 + (x >> 3)] >> (x & 0x07)) & 0x01;
}

static void egui_view_bitmap_icon_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_bitmap_icon_t);

    egui_region_t region;
    egui_dim_t draw_width;
    egui_dim_t draw_height;
    egui_color_t icon_color;
    egui_alpha_t color_alpha;
    const uint8_t *mask;
    const egui_dim_t src_width = 16;
    const egui_dim_t src_height = 16;

    if (local->image == NULL)
    {
        return;
    }

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0)
    {
        return;
    }

    mask = egui_view_bitmap_icon_resolve_mask(local->image);

    draw_width = (local->image_type == EGUI_VIEW_IMAGE_TYPE_RESIZE) ? region.size.width : EGUI_MIN(region.size.width, src_width);
    draw_height = (local->image_type == EGUI_VIEW_IMAGE_TYPE_RESIZE) ? region.size.height : EGUI_MIN(region.size.height, src_height);

    icon_color = local->image_color_alpha != 0 ? local->image_color : EGUI_COLOR_BLACK;
    color_alpha = local->image_color_alpha != 0 ? local->image_color_alpha : EGUI_ALPHA_100;

    if (!egui_view_get_enable(self))
    {
        icon_color = egui_rgb_mix(icon_color, EGUI_COLOR_HEX(0x97A4B1), 58);
    }

    for (egui_dim_t y = 0; y < draw_height; ++y)
    {
        egui_dim_t src_y = local->image_type == EGUI_VIEW_IMAGE_TYPE_RESIZE ? (y * src_height / draw_height) : y;

        for (egui_dim_t x = 0; x < draw_width; ++x)
        {
            egui_dim_t src_x = local->image_type == EGUI_VIEW_IMAGE_TYPE_RESIZE ? (x * src_width / draw_width) : x;
            egui_alpha_t draw_alpha;

            if (!egui_view_bitmap_icon_mask_is_opaque(mask, src_x, src_y))
            {
                continue;
            }

            draw_alpha = color_alpha;
            draw_alpha = self->alpha == EGUI_ALPHA_100 ? draw_alpha : egui_color_alpha_mix(self->alpha, draw_alpha);

            if (draw_alpha != 0)
            {
                egui_canvas_draw_point(region.location.x + x, region.location.y + y, icon_color, draw_alpha);
            }
        }
    }
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_bitmap_icon_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
        .on_touch_event = egui_view_on_touch_event,
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_bitmap_icon_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_bitmap_icon_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_bitmap_icon_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_bitmap_icon_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_bitmap_icon_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_bitmap_icon_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_bitmap_icon_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_bitmap_icon_on_static_key_event;
#endif
}

void egui_view_bitmap_icon_init(egui_view_t *self)
{
    egui_view_image_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_bitmap_icon_t);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);
    egui_view_image_set_image_type(self, EGUI_VIEW_IMAGE_TYPE_RESIZE);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif
    egui_view_bitmap_icon_set_image(self, egui_view_bitmap_icon_default_image());
    egui_view_bitmap_icon_apply_standard_style(self);
    egui_view_set_view_name(self, "egui_view_bitmap_icon");
}
