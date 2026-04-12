#include "egui_view_font_icon.h"

#include "resource/egui_icon_material_symbols.h"
#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_icon_font.h"

static egui_view_font_icon_t *egui_view_font_icon_local(egui_view_t *self)
{
    return (egui_view_font_icon_t *)self;
}

static const char *egui_view_font_icon_default_glyph(void)
{
    return EGUI_ICON_MS_FAVORITE;
}

static const egui_font_t *egui_view_font_icon_default_font(void)
{
    return EGUI_FONT_ICON_MS_24;
}

static uint8_t egui_view_font_icon_has_glyph(const char *glyph)
{
    return EGUI_VIEW_ICON_TEXT_VALID(glyph) ? 1 : 0;
}

static uint8_t egui_view_font_icon_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static void egui_view_font_icon_apply_color(egui_view_t *self, egui_color_t icon_color)
{
    egui_view_font_icon_clear_pressed_state(self);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);
    egui_view_label_set_font_color(self, icon_color, EGUI_ALPHA_100);
}

void egui_view_font_icon_apply_standard_style(egui_view_t *self)
{
    egui_view_font_icon_apply_color(self, EGUI_COLOR_HEX(0x0F6CBD));
}

void egui_view_font_icon_apply_subtle_style(egui_view_t *self)
{
    egui_view_font_icon_apply_color(self, EGUI_COLOR_HEX(0x6F7C8A));
}

void egui_view_font_icon_apply_accent_style(egui_view_t *self)
{
    egui_view_font_icon_apply_color(self, EGUI_COLOR_HEX(0xA15C00));
}

void egui_view_font_icon_set_glyph(egui_view_t *self, const char *glyph)
{
    egui_view_font_icon_clear_pressed_state(self);
    egui_view_label_set_text(self, egui_view_font_icon_has_glyph(glyph) ? glyph : egui_view_font_icon_default_glyph());
}

const char *egui_view_font_icon_get_glyph(egui_view_t *self)
{
    egui_view_font_icon_t *local = egui_view_font_icon_local(self);

    if (!egui_view_font_icon_has_glyph(local->text))
    {
        local->text = egui_view_font_icon_default_glyph();
    }
    return local->text;
}

void egui_view_font_icon_set_icon_font(egui_view_t *self, const egui_font_t *font)
{
    egui_view_font_icon_clear_pressed_state(self);
    egui_view_label_set_font(self, font != NULL ? font : egui_view_font_icon_default_font());
}

void egui_view_font_icon_set_palette(egui_view_t *self, egui_color_t icon_color)
{
    egui_view_font_icon_apply_color(self, icon_color);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_font_icon_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_font_icon_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_font_icon_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_font_icon_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_font_icon_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_font_icon_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_font_icon_on_static_key_event;
#endif
}

void egui_view_font_icon_init(egui_view_t *self)
{
    egui_view_label_init(self);
    egui_view_label_set_align_type(self, EGUI_ALIGN_CENTER);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif
    egui_view_font_icon_set_icon_font(self, egui_view_font_icon_default_font());
    egui_view_font_icon_set_glyph(self, egui_view_font_icon_default_glyph());
    egui_view_font_icon_apply_standard_style(self);
    egui_view_set_view_name(self, "egui_view_font_icon");
}
