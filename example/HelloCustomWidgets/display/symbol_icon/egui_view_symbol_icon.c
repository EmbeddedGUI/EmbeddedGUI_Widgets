#include "egui_view_symbol_icon.h"

#include "resource/egui_icon_material_symbols.h"
#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_icon_font.h"

static egui_view_symbol_icon_t *egui_view_symbol_icon_local(egui_view_t *self)
{
    return (egui_view_symbol_icon_t *)self;
}

static const char *egui_view_symbol_icon_default_symbol(void)
{
    return EGUI_ICON_MS_INFO;
}

static uint8_t egui_view_symbol_icon_has_symbol(const char *symbol)
{
    return EGUI_VIEW_ICON_TEXT_VALID(symbol) ? 1 : 0;
}

static uint8_t egui_view_symbol_icon_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static const egui_font_t *egui_view_symbol_icon_resolve_font(egui_view_symbol_icon_t *local, egui_dim_t area_size)
{
    return EGUI_VIEW_ICON_FONT_RESOLVE(local->icon_font, area_size, 18, 22);
}

static void egui_view_symbol_icon_apply_color(egui_view_t *self, egui_color_t icon_color)
{
    egui_view_symbol_icon_t *local = egui_view_symbol_icon_local(self);
    uint8_t had_pressed = egui_view_symbol_icon_clear_pressed_state(self);

    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);
    local->icon_color = icon_color;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_symbol_icon_apply_standard_style(egui_view_t *self)
{
    egui_view_symbol_icon_apply_color(self, EGUI_COLOR_HEX(0x0F6CBD));
}

void egui_view_symbol_icon_apply_subtle_style(egui_view_t *self)
{
    egui_view_symbol_icon_apply_color(self, EGUI_COLOR_HEX(0x6F7C8A));
}

void egui_view_symbol_icon_apply_accent_style(egui_view_t *self)
{
    egui_view_symbol_icon_apply_color(self, EGUI_COLOR_HEX(0xA15C00));
}

void egui_view_symbol_icon_set_symbol(egui_view_t *self, const char *symbol)
{
    egui_view_symbol_icon_t *local = egui_view_symbol_icon_local(self);

    egui_view_symbol_icon_clear_pressed_state(self);
    local->symbol = egui_view_symbol_icon_has_symbol(symbol) ? symbol : egui_view_symbol_icon_default_symbol();
    egui_view_invalidate(self);
}

const char *egui_view_symbol_icon_get_symbol(egui_view_t *self)
{
    egui_view_symbol_icon_t *local = egui_view_symbol_icon_local(self);

    if (!egui_view_symbol_icon_has_symbol(local->symbol))
    {
        local->symbol = egui_view_symbol_icon_default_symbol();
    }
    return local->symbol;
}

void egui_view_symbol_icon_set_icon_font(egui_view_t *self, const egui_font_t *font)
{
    egui_view_symbol_icon_t *local = egui_view_symbol_icon_local(self);

    egui_view_symbol_icon_clear_pressed_state(self);
    local->icon_font = font;
    egui_view_invalidate(self);
}

void egui_view_symbol_icon_set_palette(egui_view_t *self, egui_color_t icon_color)
{
    egui_view_symbol_icon_apply_color(self, icon_color);
}

static void egui_view_symbol_icon_on_draw(egui_view_t *self)
{
    egui_view_symbol_icon_t *local = egui_view_symbol_icon_local(self);
    egui_region_t region;
    const egui_font_t *font;
    egui_color_t icon_color = local->icon_color;

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0 || !egui_view_symbol_icon_has_symbol(local->symbol))
    {
        return;
    }

    font = egui_view_symbol_icon_resolve_font(local, EGUI_MIN(region.size.width, region.size.height));
    if (font == NULL)
    {
        return;
    }

    if (!egui_view_get_enable(self))
    {
        icon_color = egui_rgb_mix(icon_color, EGUI_COLOR_HEX(0x97A4B1), 58);
    }

    egui_canvas_draw_text_in_rect(font, local->symbol, &region, EGUI_ALIGN_CENTER, icon_color, self->alpha);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_symbol_icon_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_symbol_icon_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_symbol_icon_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_symbol_icon_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_symbol_icon_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_symbol_icon_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_symbol_icon_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_symbol_icon_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_symbol_icon_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

void egui_view_symbol_icon_init(egui_view_t *self)
{
    egui_view_symbol_icon_t *local = egui_view_symbol_icon_local(self);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_symbol_icon_t);
    egui_view_set_padding_all(self, 0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->symbol = egui_view_symbol_icon_default_symbol();
    local->icon_font = NULL;
    local->icon_color = EGUI_COLOR_HEX(0x0F6CBD);

    egui_view_set_view_name(self, "egui_view_symbol_icon");
}
