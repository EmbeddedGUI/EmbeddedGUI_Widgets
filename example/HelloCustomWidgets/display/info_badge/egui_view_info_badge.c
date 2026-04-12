#include "egui.h"
#include "egui_view_info_badge.h"

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_icon_font.h"
#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_notification_badge_internal.h"

extern const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_notification_badge_t);

static egui_view_api_t g_hcw_info_badge_api;
static uint8_t g_hcw_info_badge_api_ready;

static egui_view_notification_badge_t *hcw_info_badge_local(egui_view_t *self)
{
    return (egui_view_notification_badge_t *)self;
}

static uint8_t hcw_info_badge_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static void hcw_info_badge_on_draw(egui_view_t *self)
{
    egui_view_notification_badge_t *local = hcw_info_badge_local(self);

    if (local->content_style == EGUI_VIEW_NOTIFICATION_BADGE_CONTENT_STYLE_ICON && !EGUI_VIEW_ICON_TEXT_VALID(local->icon))
    {
        egui_region_t region;

        egui_view_get_work_region(self, &region);
        if (egui_region_is_empty(&region))
        {
            return;
        }

        egui_view_notification_badge_draw_background(&region, local->badge_color, 1);
        return;
    }

    egui_view_notification_badge_on_draw(self);
}

static egui_view_api_t *hcw_info_badge_resolve_api(void)
{
    if (!g_hcw_info_badge_api_ready)
    {
        g_hcw_info_badge_api = EGUI_VIEW_API_TABLE_NAME(egui_view_notification_badge_t);
        g_hcw_info_badge_api.on_draw = hcw_info_badge_on_draw;
        g_hcw_info_badge_api_ready = 1;
    }

    return &g_hcw_info_badge_api;
}

static void hcw_info_badge_prepare_base(egui_view_t *self)
{
    hcw_info_badge_clear_pressed_state(self);
    if (self->api == NULL || self->api->on_draw != hcw_info_badge_on_draw)
    {
        self->api = hcw_info_badge_resolve_api();
    }
    egui_view_set_enable(self, 1);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    self->padding.left = 0;
    self->padding.right = 0;
    self->padding.top = 0;
    self->padding.bottom = 0;
}

static void hcw_info_badge_apply_style(egui_view_t *self, egui_view_notification_badge_content_style_t style, egui_color_t badge_color,
                                       egui_color_t text_color, const char *icon)
{
    egui_view_notification_badge_t *local = hcw_info_badge_local(self);

    hcw_info_badge_prepare_base(self);
    local->max_display = 99;
    local->badge_color = badge_color;
    local->text_color = text_color;
    local->content_style = (uint8_t)style;
    if (style == EGUI_VIEW_NOTIFICATION_BADGE_CONTENT_STYLE_ICON)
    {
        local->icon = icon;
    }
    egui_view_invalidate(self);
}

void hcw_info_badge_apply_count_style(egui_view_t *self)
{
    hcw_info_badge_apply_style(self, EGUI_VIEW_NOTIFICATION_BADGE_CONTENT_STYLE_COUNT, EGUI_COLOR_HEX(0xC42B1C), EGUI_COLOR_WHITE, NULL);
}

void hcw_info_badge_apply_icon_style(egui_view_t *self)
{
    hcw_info_badge_apply_style(self, EGUI_VIEW_NOTIFICATION_BADGE_CONTENT_STYLE_ICON, EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_WHITE, EGUI_ICON_MS_INFO);
}

void hcw_info_badge_apply_attention_style(egui_view_t *self)
{
    hcw_info_badge_apply_style(self, EGUI_VIEW_NOTIFICATION_BADGE_CONTENT_STYLE_ICON, EGUI_COLOR_HEX(0xC42B1C), EGUI_COLOR_WHITE, NULL);
}

void hcw_info_badge_set_count(egui_view_t *self, uint16_t count)
{
    egui_view_notification_badge_t *local = hcw_info_badge_local(self);

    hcw_info_badge_prepare_base(self);
    local->content_style = EGUI_VIEW_NOTIFICATION_BADGE_CONTENT_STYLE_COUNT;
    local->count = count;
    egui_view_invalidate(self);
}

void hcw_info_badge_set_icon(egui_view_t *self, const char *icon)
{
    egui_view_notification_badge_t *local = hcw_info_badge_local(self);

    hcw_info_badge_prepare_base(self);
    local->content_style = EGUI_VIEW_NOTIFICATION_BADGE_CONTENT_STYLE_ICON;
    local->icon = icon;
    egui_view_invalidate(self);
}

void hcw_info_badge_set_palette(egui_view_t *self, egui_color_t badge_color, egui_color_t text_color)
{
    egui_view_notification_badge_t *local = hcw_info_badge_local(self);

    hcw_info_badge_prepare_base(self);
    local->badge_color = badge_color;
    local->text_color = text_color;
    egui_view_invalidate(self);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_info_badge_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_info_badge_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_info_badge_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_info_badge_clear_pressed_state(self);
    return 1;
}
#endif

void hcw_info_badge_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    self->api = hcw_info_badge_resolve_api();
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_info_badge_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_info_badge_on_static_key_event;
#endif
}
