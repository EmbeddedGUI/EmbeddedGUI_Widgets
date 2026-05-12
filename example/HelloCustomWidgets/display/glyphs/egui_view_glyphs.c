#include "egui_view_glyphs.h"

#define EGUI_VIEW_GLYPHS_PERCENT_MAX 100

static egui_view_glyphs_t *egui_view_glyphs_local(egui_view_t *self)
{
    return (egui_view_glyphs_t *)self;
}

static uint8_t egui_view_glyphs_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static const char *egui_view_glyphs_default_text(void)
{
    return "Glyphs";
}

static const egui_font_t *egui_view_glyphs_default_font(void)
{
    return (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
}

static uint8_t egui_view_glyphs_clamp_percent(uint8_t value)
{
    return value > EGUI_VIEW_GLYPHS_PERCENT_MAX ? EGUI_VIEW_GLYPHS_PERCENT_MAX : value;
}

static egui_dim_t egui_view_glyphs_resolve_axis(egui_dim_t origin, egui_dim_t length, uint8_t percent)
{
    if (length <= 1)
    {
        return origin;
    }
    return origin + (egui_dim_t)(((int32_t)(length - 1) * percent) / 100);
}

static egui_color_t egui_view_glyphs_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_MAKE(38));
}

static uint8_t egui_view_glyphs_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static void egui_view_glyphs_on_draw(egui_view_t *self)
{
    egui_view_glyphs_t *local = egui_view_glyphs_local(self);
    egui_region_t region;
    egui_region_t draw_region;
    egui_color_t fill_color = local->fill_color;
    egui_color_t accent_color = local->accent_color;
    egui_alpha_t text_alpha = EGUI_ALPHA_100;
    egui_alpha_t marker_alpha = EGUI_ALPHA_MAKE(46);
    egui_alpha_t marker_cross_alpha = EGUI_ALPHA_MAKE(23);
    egui_alpha_t marker_tick_alpha = EGUI_ALPHA_MAKE(56);
    const egui_font_t *font = local->font != NULL ? local->font : egui_view_glyphs_default_font();
    const char *text = egui_view_glyphs_has_text(local->unicode_string) ? local->unicode_string : egui_view_glyphs_default_text();
    egui_dim_t origin_x;
    egui_dim_t origin_y;

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0 || font == NULL)
    {
        return;
    }

    if (!egui_view_get_enable(self))
    {
        fill_color = egui_view_glyphs_mix_disabled(fill_color);
        accent_color = egui_view_glyphs_mix_disabled(accent_color);
        text_alpha = EGUI_ALPHA_MAKE(58);
        marker_alpha = EGUI_ALPHA_MAKE(34);
        marker_cross_alpha = EGUI_ALPHA_MAKE(17);
        marker_tick_alpha = EGUI_ALPHA_MAKE(44);
    }
    if (egui_view_get_pressed(self))
    {
        fill_color = egui_rgb_mix(fill_color, accent_color, EGUI_ALPHA_MAKE(18));
    }

    origin_x = egui_view_glyphs_resolve_axis(region.location.x, region.size.width, local->origin_x_percent);
    origin_y = egui_view_glyphs_resolve_axis(region.location.y, region.size.height, local->origin_y_percent);

    if (marker_alpha > 0)
    {
        egui_dim_t marker_w = 12;
        egui_canvas_draw_line(&uicode_get_core()->canvas, origin_x, region.location.y + 1, origin_x, region.location.y + region.size.height - 2, 1,
                              accent_color, egui_color_alpha_mix(self->alpha, marker_alpha));
        egui_canvas_draw_line(&uicode_get_core()->canvas, region.location.x + 1, origin_y, region.location.x + region.size.width - 2, origin_y, 1,
                              accent_color, egui_color_alpha_mix(self->alpha, marker_cross_alpha));
        egui_canvas_draw_line(&uicode_get_core()->canvas, origin_x, origin_y, origin_x + marker_w, origin_y, 1, accent_color,
                              egui_color_alpha_mix(self->alpha, marker_tick_alpha));
    }

    draw_region.location.x = origin_x;
    draw_region.location.y = origin_y;
    draw_region.size.width = region.location.x + region.size.width - origin_x;
    draw_region.size.height = region.location.y + region.size.height - origin_y;
    if (draw_region.size.width <= 0 || draw_region.size.height <= 0)
    {
        return;
    }

    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_TOP, fill_color,
                                  egui_color_alpha_mix(self->alpha, text_alpha));
}

void egui_view_glyphs_set_unicode_string(egui_view_t *self, const char *unicode_string)
{
    egui_view_glyphs_t *local = egui_view_glyphs_local(self);

    egui_view_glyphs_clear_pressed_state(self);
    local->unicode_string = unicode_string != NULL ? unicode_string : egui_view_glyphs_default_text();
    egui_view_invalidate(self);
}

const char *egui_view_glyphs_get_unicode_string(egui_view_t *self)
{
    egui_view_glyphs_t *local = egui_view_glyphs_local(self);

    return local->unicode_string;
}

void egui_view_glyphs_set_font(egui_view_t *self, const egui_font_t *font, uint8_t font_rendering_em_size)
{
    egui_view_glyphs_t *local = egui_view_glyphs_local(self);

    egui_view_glyphs_clear_pressed_state(self);
    local->font = font != NULL ? font : egui_view_glyphs_default_font();
    local->font_rendering_em_size = font_rendering_em_size;
    egui_view_invalidate(self);
}

const egui_font_t *egui_view_glyphs_get_font(egui_view_t *self)
{
    egui_view_glyphs_t *local = egui_view_glyphs_local(self);

    return local->font;
}

uint8_t egui_view_glyphs_get_font_rendering_em_size(egui_view_t *self)
{
    egui_view_glyphs_t *local = egui_view_glyphs_local(self);

    return local->font_rendering_em_size;
}

void egui_view_glyphs_set_fill(egui_view_t *self, egui_color_t fill_color, egui_color_t accent_color)
{
    egui_view_glyphs_t *local = egui_view_glyphs_local(self);

    egui_view_glyphs_clear_pressed_state(self);
    local->fill_color = fill_color;
    local->accent_color = accent_color;
    egui_view_invalidate(self);
}

void egui_view_glyphs_set_origin(egui_view_t *self, uint8_t origin_x_percent, uint8_t origin_y_percent)
{
    egui_view_glyphs_t *local = egui_view_glyphs_local(self);

    egui_view_glyphs_clear_pressed_state(self);
    local->origin_x_percent = egui_view_glyphs_clamp_percent(origin_x_percent);
    local->origin_y_percent = egui_view_glyphs_clamp_percent(origin_y_percent);
    egui_view_invalidate(self);
}

void egui_view_glyphs_get_origin(egui_view_t *self, uint8_t *origin_x_percent, uint8_t *origin_y_percent)
{
    egui_view_glyphs_t *local = egui_view_glyphs_local(self);

    if (origin_x_percent != NULL)
    {
        *origin_x_percent = local->origin_x_percent;
    }
    if (origin_y_percent != NULL)
    {
        *origin_y_percent = local->origin_y_percent;
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_glyphs_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_glyphs_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_glyphs_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_glyphs_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_glyphs_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_glyphs_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_glyphs_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_glyphs_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_glyphs_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_glyphs_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_glyphs_on_static_key_event;
#endif
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_glyphs_t) = {
        .draw = egui_view_draw,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_glyphs_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .on_draw = egui_view_glyphs_on_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_glyphs_on_key_event,
#endif
};

void egui_view_glyphs_init(egui_view_t *self)
{
    egui_view_glyphs_t *local = egui_view_glyphs_local(self);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_glyphs_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->unicode_string = egui_view_glyphs_default_text();
    local->font = (const egui_font_t *)&egui_res_font_montserrat_16_4;
    local->font_rendering_em_size = 16;
    local->fill_color = HCW_COLOR_TEXT;
    local->accent_color = HCW_COLOR_PRIMARY_SOFT;
    local->origin_x_percent = 8;
    local->origin_y_percent = 18;
    egui_view_set_view_name(self, "egui_view_glyphs");
}
