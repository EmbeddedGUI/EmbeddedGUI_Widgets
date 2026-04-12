#include "egui_view_text_block.h"

static egui_view_text_block_t *egui_view_text_block_local(egui_view_t *self)
{
    return (egui_view_text_block_t *)self;
}

static egui_view_textblock_t *egui_view_text_block_widget(egui_view_t *self)
{
    return (egui_view_textblock_t *)self;
}

static const char *egui_view_text_block_default_text(void)
{
    return "";
}

static const egui_font_t *egui_view_text_block_default_font(void)
{
    return (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
}

static uint8_t egui_view_text_block_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static egui_color_t egui_view_text_block_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x7B8794), 52);
}

static egui_color_t egui_view_text_block_resolve_color(const egui_view_text_block_t *local)
{
    switch (local->style)
    {
    case EGUI_VIEW_TEXT_BLOCK_STYLE_SUBTLE:
        return local->subtle_color;
    case EGUI_VIEW_TEXT_BLOCK_STYLE_ACCENT:
        return local->accent_color;
    case EGUI_VIEW_TEXT_BLOCK_STYLE_STANDARD:
    default:
        return local->standard_color;
    }
}

static void egui_view_text_block_sync_visuals(egui_view_t *self)
{
    egui_view_text_block_t *local = egui_view_text_block_local(self);
    egui_view_textblock_t *widget = egui_view_text_block_widget(self);
    egui_color_t text_color = egui_view_text_block_resolve_color(local);
    egui_alpha_t text_alpha = local->text_alpha;

    if (local->read_only_mode)
    {
        text_color = egui_view_text_block_mix_disabled(text_color);
        text_alpha = (text_alpha > 28) ? (egui_alpha_t)(text_alpha - 28) : (egui_alpha_t)(text_alpha / 2);
    }

    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);
    egui_view_textblock_set_auto_wrap(self, 1);
    egui_view_textblock_set_scroll_enabled(self, 0);
    egui_view_textblock_set_border_enabled(self, 0);
    egui_view_textblock_set_align_type(self, EGUI_ALIGN_LEFT | EGUI_ALIGN_TOP);
    egui_view_textblock_set_line_space(self, local->compact_mode ? 2 : 4);
    egui_view_textblock_set_font_color(self, text_color, text_alpha);

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY && EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_textblock_set_editable(self, 0);
    egui_view_set_focusable(self, 0);
#endif

    widget->is_scroll_enabled = 0;
    widget->is_border_enabled = 0;
}

void egui_view_text_block_apply_standard_style(egui_view_t *self)
{
    egui_view_text_block_t *local = egui_view_text_block_local(self);

    egui_view_text_block_clear_pressed_state(self);
    local->style = EGUI_VIEW_TEXT_BLOCK_STYLE_STANDARD;
    egui_view_text_block_sync_visuals(self);
}

void egui_view_text_block_apply_subtle_style(egui_view_t *self)
{
    egui_view_text_block_t *local = egui_view_text_block_local(self);

    egui_view_text_block_clear_pressed_state(self);
    local->style = EGUI_VIEW_TEXT_BLOCK_STYLE_SUBTLE;
    egui_view_text_block_sync_visuals(self);
}

void egui_view_text_block_apply_accent_style(egui_view_t *self)
{
    egui_view_text_block_t *local = egui_view_text_block_local(self);

    egui_view_text_block_clear_pressed_state(self);
    local->style = EGUI_VIEW_TEXT_BLOCK_STYLE_ACCENT;
    egui_view_text_block_sync_visuals(self);
}

void egui_view_text_block_set_text(egui_view_t *self, const char *text)
{
    egui_view_text_block_clear_pressed_state(self);
    egui_view_textblock_set_text(self, (text != NULL) ? text : egui_view_text_block_default_text());
}

void egui_view_text_block_set_font(egui_view_t *self, const egui_font_t *font)
{
    egui_view_text_block_clear_pressed_state(self);
    egui_view_textblock_set_font(self, (font != NULL) ? font : egui_view_text_block_default_font());
    egui_view_text_block_sync_visuals(self);
}

void egui_view_text_block_set_palette(egui_view_t *self, egui_color_t standard_color, egui_color_t subtle_color, egui_color_t accent_color,
                                      egui_alpha_t text_alpha)
{
    egui_view_text_block_t *local = egui_view_text_block_local(self);

    egui_view_text_block_clear_pressed_state(self);
    local->standard_color = standard_color;
    local->subtle_color = subtle_color;
    local->accent_color = accent_color;
    local->text_alpha = text_alpha;
    egui_view_text_block_sync_visuals(self);
}

void egui_view_text_block_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    egui_view_text_block_t *local = egui_view_text_block_local(self);

    egui_view_text_block_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_text_block_sync_visuals(self);
}

uint8_t egui_view_text_block_get_compact_mode(egui_view_t *self)
{
    return egui_view_text_block_local(self)->compact_mode;
}

void egui_view_text_block_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    egui_view_text_block_t *local = egui_view_text_block_local(self);

    egui_view_text_block_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_text_block_sync_visuals(self);
}

uint8_t egui_view_text_block_get_read_only_mode(egui_view_t *self)
{
    return egui_view_text_block_local(self)->read_only_mode;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_text_block_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_text_block_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_text_block_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_text_block_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_text_block_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_text_block_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_text_block_on_static_key_event;
#endif
}

void egui_view_text_block_init(egui_view_t *self)
{
    egui_view_text_block_t *local = egui_view_text_block_local(self);

    egui_view_textblock_init(self);
    local->standard_color = EGUI_COLOR_HEX(0x22303F);
    local->subtle_color = EGUI_COLOR_HEX(0x6B7A89);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->text_alpha = EGUI_ALPHA_100;
    local->style = EGUI_VIEW_TEXT_BLOCK_STYLE_STANDARD;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    egui_view_text_block_set_text(self, egui_view_text_block_default_text());
    egui_view_text_block_set_font(self, egui_view_text_block_default_font());
    egui_view_text_block_sync_visuals(self);
    egui_view_set_view_name(self, "egui_view_text_block");
}
