#include "egui.h"
#include "egui_view_hyperlink_button.h"

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_icon_font.h"

extern const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_button_t);
void egui_view_button_on_draw(egui_view_t *self);

static egui_view_api_t g_hcw_hyperlink_button_api;
static uint8_t g_hcw_hyperlink_button_api_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_hyperlink_button_standard_bg_normal_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_0, 6, 0,
                                                        EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_0);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_hyperlink_button_standard_bg_pressed_param, EGUI_COLOR_HEX(0xE8F2FF), EGUI_ALPHA_100, 6, 0,
                                                        EGUI_COLOR_HEX(0xE8F2FF), EGUI_ALPHA_0);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_hyperlink_button_standard_bg_disabled_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_0, 6, 0,
                                                        EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_hyperlink_button_standard_bg_focused_param, EGUI_COLOR_HEX(0xF4F9FF), EGUI_ALPHA_100, 6, 1,
                                                        EGUI_COLOR_HEX(0x9AC5FF), EGUI_ALPHA_100);
EGUI_BACKGROUND_PARAM_INIT_WITH_FOCUS(hcw_hyperlink_button_standard_bg_params, &hcw_hyperlink_button_standard_bg_normal_param,
                                      &hcw_hyperlink_button_standard_bg_pressed_param, &hcw_hyperlink_button_standard_bg_disabled_param,
                                      &hcw_hyperlink_button_standard_bg_focused_param);
#else
EGUI_BACKGROUND_PARAM_INIT(hcw_hyperlink_button_standard_bg_params, &hcw_hyperlink_button_standard_bg_normal_param,
                           &hcw_hyperlink_button_standard_bg_pressed_param, &hcw_hyperlink_button_standard_bg_disabled_param);
#endif
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(hcw_hyperlink_button_standard_background, &hcw_hyperlink_button_standard_bg_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_hyperlink_button_inline_bg_normal_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_0, 6, 0,
                                                        EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_0);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_hyperlink_button_inline_bg_pressed_param, EGUI_COLOR_HEX(0xEDF4FA), EGUI_ALPHA_100, 6, 0,
                                                        EGUI_COLOR_HEX(0xEDF4FA), EGUI_ALPHA_0);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_hyperlink_button_inline_bg_disabled_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_0, 6, 0,
                                                        EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_hyperlink_button_inline_bg_focused_param, EGUI_COLOR_HEX(0xF6FAFD), EGUI_ALPHA_100, 6, 1,
                                                        EGUI_COLOR_HEX(0xB5C3D1), EGUI_ALPHA_100);
EGUI_BACKGROUND_PARAM_INIT_WITH_FOCUS(hcw_hyperlink_button_inline_bg_params, &hcw_hyperlink_button_inline_bg_normal_param,
                                      &hcw_hyperlink_button_inline_bg_pressed_param, &hcw_hyperlink_button_inline_bg_disabled_param,
                                      &hcw_hyperlink_button_inline_bg_focused_param);
#else
EGUI_BACKGROUND_PARAM_INIT(hcw_hyperlink_button_inline_bg_params, &hcw_hyperlink_button_inline_bg_normal_param,
                           &hcw_hyperlink_button_inline_bg_pressed_param, &hcw_hyperlink_button_inline_bg_disabled_param);
#endif
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(hcw_hyperlink_button_inline_background, &hcw_hyperlink_button_inline_bg_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_hyperlink_button_disabled_bg_normal_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_0, 6, 0,
                                                        EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_0);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_hyperlink_button_disabled_bg_pressed_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_0, 6, 0,
                                                        EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_0);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_hyperlink_button_disabled_bg_disabled_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_0, 6, 0,
                                                        EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_hyperlink_button_disabled_bg_focused_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_0, 6, 0,
                                                        EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_0);
EGUI_BACKGROUND_PARAM_INIT_WITH_FOCUS(hcw_hyperlink_button_disabled_bg_params, &hcw_hyperlink_button_disabled_bg_normal_param,
                                      &hcw_hyperlink_button_disabled_bg_pressed_param, &hcw_hyperlink_button_disabled_bg_disabled_param,
                                      &hcw_hyperlink_button_disabled_bg_focused_param);
#else
EGUI_BACKGROUND_PARAM_INIT(hcw_hyperlink_button_disabled_bg_params, &hcw_hyperlink_button_disabled_bg_normal_param,
                           &hcw_hyperlink_button_disabled_bg_pressed_param, &hcw_hyperlink_button_disabled_bg_disabled_param);
#endif
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(hcw_hyperlink_button_disabled_background, &hcw_hyperlink_button_disabled_bg_params);

static egui_view_button_t *hcw_hyperlink_button_local(egui_view_t *self)
{
    return (egui_view_button_t *)self;
}

static uint8_t hcw_hyperlink_button_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static const egui_font_t *hcw_hyperlink_button_get_text_font(const egui_view_button_t *local)
{
    if (local->base.font != NULL)
    {
        return local->base.font;
    }

    return (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
}

static void hcw_hyperlink_button_draw_underline(egui_view_t *self)
{
    egui_view_button_t *local = hcw_hyperlink_button_local(self);
    const egui_font_t *font = hcw_hyperlink_button_get_text_font(local);
    const char *text = local->base.text;
    egui_region_t region;
    egui_dim_t text_width = 0;
    egui_dim_t text_height = 0;
    egui_dim_t start_x;
    egui_dim_t line_y;
    egui_dim_t end_x;
    egui_alpha_t alpha = egui_color_alpha_mix(self->alpha, self->is_pressed ? 96 : 84);

    if (!egui_view_get_enable(self) || !EGUI_VIEW_TEXT_VALID(text) || font == NULL || font->api == NULL || font->api->get_str_size == NULL)
    {
        return;
    }

    egui_view_get_work_region(self, &region);
    if (egui_region_is_empty(&region))
    {
        return;
    }

    font->api->get_str_size(font, text, 0, 0, &text_width, &text_height);
    EGUI_UNUSED(text_height);
    if (text_width <= 0)
    {
        return;
    }

    switch (local->base.align_type & EGUI_ALIGN_HMASK)
    {
    case EGUI_ALIGN_RIGHT:
        start_x = region.location.x + region.size.width - text_width;
        break;
    case EGUI_ALIGN_CENTER:
        start_x = region.location.x + (region.size.width - text_width) / 2;
        break;
    default:
        start_x = region.location.x;
        break;
    }

    if (start_x < region.location.x)
    {
        start_x = region.location.x;
    }
    end_x = start_x + text_width - 1;
    if (end_x >= region.location.x + region.size.width)
    {
        end_x = region.location.x + region.size.width - 1;
    }
    if (end_x <= start_x)
    {
        return;
    }

    line_y = region.location.y + region.size.height - 2;
    if (line_y < region.location.y)
    {
        line_y = region.location.y;
    }
    egui_canvas_draw_line(start_x, line_y, end_x, line_y, 1, local->base.color, alpha);
}

static void hcw_hyperlink_button_on_draw(egui_view_t *self)
{
    egui_view_button_on_draw(self);
    hcw_hyperlink_button_draw_underline(self);
}

static egui_view_api_t *hcw_hyperlink_button_resolve_api(void)
{
    if (!g_hcw_hyperlink_button_api_ready)
    {
        g_hcw_hyperlink_button_api = EGUI_VIEW_API_TABLE_NAME(egui_view_button_t);
        g_hcw_hyperlink_button_api.on_draw = hcw_hyperlink_button_on_draw;
        g_hcw_hyperlink_button_api_ready = 1;
    }

    return &g_hcw_hyperlink_button_api;
}

static void hcw_hyperlink_button_prepare_base(egui_view_t *self)
{
    egui_view_button_t *local = hcw_hyperlink_button_local(self);

    hcw_hyperlink_button_clear_pressed_state(self);
    if (self->api == NULL || self->api->on_draw != hcw_hyperlink_button_on_draw)
    {
        self->api = hcw_hyperlink_button_resolve_api();
    }
    egui_view_set_shadow(self, NULL);
    egui_view_label_set_align_type(self, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    self->padding.left = 0;
    self->padding.right = 0;
    self->padding.top = 0;
    self->padding.bottom = 1;
    local->icon = NULL;
    local->icon_text_gap = 0;
}

static void hcw_hyperlink_button_apply_style(egui_view_t *self, egui_background_t *background, egui_color_t text_color)
{
    egui_view_button_t *local = hcw_hyperlink_button_local(self);

    hcw_hyperlink_button_prepare_base(self);
    egui_view_set_background(self, background);
    local->base.color = text_color;
    local->base.alpha = EGUI_ALPHA_100;
    egui_view_invalidate(self);
}

void hcw_hyperlink_button_apply_standard_style(egui_view_t *self)
{
    hcw_hyperlink_button_apply_style(self, EGUI_BG_OF(&hcw_hyperlink_button_standard_background), EGUI_COLOR_HEX(0x0F6CBD));
}

void hcw_hyperlink_button_apply_inline_style(egui_view_t *self)
{
    hcw_hyperlink_button_apply_style(self, EGUI_BG_OF(&hcw_hyperlink_button_inline_background), EGUI_COLOR_HEX(0x24527A));
}

void hcw_hyperlink_button_apply_disabled_style(egui_view_t *self)
{
    hcw_hyperlink_button_apply_style(self, EGUI_BG_OF(&hcw_hyperlink_button_disabled_background), EGUI_COLOR_HEX(0x8A96A3));
}

void hcw_hyperlink_button_set_text(egui_view_t *self, const char *text)
{
    hcw_hyperlink_button_prepare_base(self);
    egui_view_label_set_text(self, text);
}

void hcw_hyperlink_button_set_font(egui_view_t *self, const egui_font_t *font)
{
    hcw_hyperlink_button_prepare_base(self);
    egui_view_label_set_font(self, font);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_hyperlink_button_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_hyperlink_button_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_hyperlink_button_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_hyperlink_button_clear_pressed_state(self);
    return 1;
}
#endif

void hcw_hyperlink_button_override_interaction_api(egui_view_t *self, egui_view_api_t *api)
{
    hcw_hyperlink_button_prepare_base(self);
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_hyperlink_button_on_key_event;
#endif
}

void hcw_hyperlink_button_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    hcw_hyperlink_button_prepare_base(self);
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_hyperlink_button_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_hyperlink_button_on_static_key_event;
#endif
}

int hcw_hyperlink_button_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    uint8_t was_pressed = hcw_hyperlink_button_clear_pressed_state(self);

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
