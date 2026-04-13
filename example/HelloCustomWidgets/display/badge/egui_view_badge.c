#include "egui_view_badge.h"

#include <string.h>

typedef struct egui_view_badge_metrics egui_view_badge_metrics_t;
struct egui_view_badge_metrics
{
    egui_region_t text_region;
    egui_region_t icon_region;
    uint8_t show_icon;
};

static egui_color_t egui_view_badge_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x83909D), 48);
}

static void egui_view_badge_copy_text(char *dst, uint8_t capacity, const char *src)
{
    size_t length = 0;

    if (dst == NULL || capacity == 0)
    {
        return;
    }

    if (src != NULL)
    {
        length = strlen(src);
        if (length >= capacity)
        {
            length = capacity - 1;
        }
        if (length > 0)
        {
            memcpy(dst, src, length);
        }
    }

    dst[length] = '\0';
}

static void egui_view_badge_measure_text(const egui_font_t *font, const char *text, egui_dim_t *width, egui_dim_t *height)
{
    egui_dim_t measured_width = 0;
    egui_dim_t measured_height = 0;

    if (font != NULL && font->api != NULL && font->api->get_str_size != NULL && text != NULL && text[0] != '\0')
    {
        font->api->get_str_size(font, text, 0, 0, &measured_width, &measured_height);
    }

    if (measured_height <= 0)
    {
        measured_height = 10;
    }

    if (width != NULL)
    {
        *width = measured_width;
    }
    if (height != NULL)
    {
        *height = measured_height;
    }
}

static const egui_font_t *egui_view_badge_get_text_font(const egui_view_badge_t *local)
{
    return local->font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : local->font;
}

static const egui_font_t *egui_view_badge_get_icon_font(const egui_view_badge_t *local)
{
    return local->icon_font == NULL ? EGUI_FONT_ICON_MS_16 : local->icon_font;
}

static uint8_t egui_view_badge_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static uint8_t egui_view_badge_has_icon(const egui_view_badge_t *local)
{
    return local->icon != NULL && local->icon[0] != '\0';
}

static void egui_view_badge_local_region_to_screen(egui_view_t *self, const egui_region_t *local_region, egui_region_t *screen_region)
{
    screen_region->location.x = self->region_screen.location.x + local_region->location.x;
    screen_region->location.y = self->region_screen.location.y + local_region->location.y;
    screen_region->size.width = local_region->size.width;
    screen_region->size.height = local_region->size.height;
}

static void egui_view_badge_get_metrics(egui_view_t *self, egui_view_badge_t *local, egui_view_badge_metrics_t *metrics)
{
    egui_region_t region;
    egui_dim_t pad_x = local->compact_mode ? 7 : 10;
    egui_dim_t gap = local->compact_mode ? 4 : 6;
    egui_dim_t icon_size = local->compact_mode ? 11 : 13;
    egui_dim_t text_width = 0;
    egui_dim_t available_width;
    egui_dim_t content_width = 0;
    egui_dim_t cursor_x;

    memset(metrics, 0, sizeof(*metrics));
    egui_view_get_work_region(self, &region);
    if (egui_region_is_empty(&region))
    {
        return;
    }

    available_width = region.size.width - pad_x * 2;
    if (available_width <= 0)
    {
        return;
    }

    metrics->show_icon = egui_view_badge_has_icon(local) ? 1 : 0;
    egui_view_badge_measure_text(egui_view_badge_get_text_font(local), local->text, &text_width, NULL);

    if (metrics->show_icon)
    {
        content_width += icon_size;
        if (local->text[0] != '\0')
        {
            content_width += gap;
        }
    }
    if (local->text[0] != '\0')
    {
        content_width += text_width;
    }
    if (content_width <= 0)
    {
        content_width = available_width;
    }
    if (content_width > available_width)
    {
        content_width = available_width;
    }

    cursor_x = region.location.x + pad_x + (available_width - content_width) / 2;
    if (metrics->show_icon)
    {
        metrics->icon_region.location.x = cursor_x;
        metrics->icon_region.location.y = region.location.y + (region.size.height - icon_size) / 2;
        metrics->icon_region.size.width = icon_size;
        metrics->icon_region.size.height = icon_size;
        cursor_x += icon_size;
        if (local->text[0] != '\0')
        {
            cursor_x += gap;
        }
    }

    metrics->text_region.location.x = cursor_x;
    metrics->text_region.location.y = region.location.y;
    metrics->text_region.size.width = region.location.x + region.size.width - pad_x - cursor_x;
    if (metrics->text_region.size.width < 0)
    {
        metrics->text_region.size.width = 0;
    }
    metrics->text_region.size.height = region.size.height;
}

static void egui_view_badge_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (text == NULL || text[0] == '\0' || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    egui_canvas_draw_text_in_rect(font, text, &draw_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
}

static void egui_view_badge_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_badge_t);
    egui_view_badge_metrics_t metrics;
    egui_region_t region;
    egui_color_t fill_color = local->surface_color;
    egui_color_t border_color = local->border_color;
    egui_color_t text_color = local->text_color;
    egui_color_t accent_color = local->accent_color;
    egui_alpha_t border_alpha = local->subtle_mode ? 28 : (local->outline_mode ? 54 : 74);
    egui_dim_t radius = local->compact_mode ? 8 : 10;

    egui_view_get_work_region(self, &region);
    if (egui_region_is_empty(&region))
    {
        return;
    }

    egui_view_badge_get_metrics(self, local, &metrics);
    if (local->read_only_mode)
    {
        fill_color = egui_rgb_mix(fill_color, EGUI_COLOR_HEX(0xF5F7FA), 28);
        border_color = egui_rgb_mix(border_color, EGUI_COLOR_HEX(0xBAC7D3), 36);
        text_color = egui_rgb_mix(text_color, EGUI_COLOR_HEX(0x7D8995), 42);
        accent_color = egui_rgb_mix(accent_color, EGUI_COLOR_HEX(0x85919D), 42);
        border_alpha = local->subtle_mode ? 24 : 40;
    }
    if (!egui_view_get_enable(self))
    {
        fill_color = egui_view_badge_mix_disabled(fill_color);
        border_color = egui_view_badge_mix_disabled(border_color);
        text_color = egui_view_badge_mix_disabled(text_color);
        accent_color = egui_view_badge_mix_disabled(accent_color);
    }

    egui_canvas_draw_round_rectangle_fill(region.location.x, region.location.y, region.size.width, region.size.height, radius, fill_color,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
    egui_canvas_draw_round_rectangle(region.location.x, region.location.y, region.size.width, region.size.height, radius, 1, border_color,
                                     egui_color_alpha_mix(self->alpha, border_alpha));

    if (metrics.show_icon)
    {
        egui_canvas_draw_text_in_rect(egui_view_badge_get_icon_font(local), local->icon, &metrics.icon_region, EGUI_ALIGN_CENTER, accent_color,
                                      egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
    }
    egui_view_badge_draw_text(egui_view_badge_get_text_font(local), self, local->text, &metrics.text_region, text_color);
}

void egui_view_badge_apply_filled_style(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_badge_t);

    egui_view_badge_clear_pressed_state(self);
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->outline_mode = 0;
    local->subtle_mode = 0;
    local->surface_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->border_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->text_color = EGUI_COLOR_WHITE;
    local->accent_color = EGUI_COLOR_WHITE;
    egui_view_invalidate(self);
}

void egui_view_badge_apply_outline_style(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_badge_t);

    egui_view_badge_clear_pressed_state(self);
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->outline_mode = 1;
    local->subtle_mode = 0;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xB3C7E5);
    local->text_color = EGUI_COLOR_HEX(0x0F548C);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    egui_view_invalidate(self);
}

void egui_view_badge_apply_subtle_style(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_badge_t);

    egui_view_badge_clear_pressed_state(self);
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->outline_mode = 0;
    local->subtle_mode = 1;
    local->surface_color = EGUI_COLOR_HEX(0xF3F7FB);
    local->border_color = EGUI_COLOR_HEX(0xD6E3F3);
    local->text_color = EGUI_COLOR_HEX(0x12456F);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    egui_view_invalidate(self);
}

void egui_view_badge_apply_read_only_style(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_badge_t);

    egui_view_badge_clear_pressed_state(self);
    local->compact_mode = 1;
    local->read_only_mode = 1;
    local->outline_mode = 0;
    local->subtle_mode = 1;
    local->surface_color = EGUI_COLOR_HEX(0xF5F7FA);
    local->border_color = EGUI_COLOR_HEX(0xD7DEE6);
    local->text_color = EGUI_COLOR_HEX(0x73808C);
    local->accent_color = EGUI_COLOR_HEX(0x87939E);
    egui_view_invalidate(self);
}

void egui_view_badge_set_text(egui_view_t *self, const char *text)
{
    EGUI_LOCAL_INIT(egui_view_badge_t);

    egui_view_badge_clear_pressed_state(self);
    egui_view_badge_copy_text(local->text, sizeof(local->text), text);
    egui_view_invalidate(self);
}

void egui_view_badge_set_icon(egui_view_t *self, const char *icon)
{
    EGUI_LOCAL_INIT(egui_view_badge_t);

    egui_view_badge_clear_pressed_state(self);
    local->icon = (icon != NULL && icon[0] != '\0') ? icon : NULL;
    egui_view_invalidate(self);
}

void egui_view_badge_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_badge_t);

    egui_view_badge_clear_pressed_state(self);
    local->font = font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : font;
    egui_view_invalidate(self);
}

void egui_view_badge_set_icon_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_badge_t);

    egui_view_badge_clear_pressed_state(self);
    local->icon_font = font == NULL ? EGUI_FONT_ICON_MS_16 : font;
    egui_view_invalidate(self);
}

void egui_view_badge_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color, egui_color_t accent_color)
{
    EGUI_LOCAL_INIT(egui_view_badge_t);

    egui_view_badge_clear_pressed_state(self);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->accent_color = accent_color;
    egui_view_invalidate(self);
}

void egui_view_badge_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_badge_t);

    egui_view_badge_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

void egui_view_badge_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_badge_t);

    egui_view_badge_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_badge_get_icon_region(egui_view_t *self, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_badge_t);
    egui_view_badge_metrics_t metrics;

    if (region == NULL)
    {
        return 0;
    }

    egui_view_badge_get_metrics(self, local, &metrics);
    if (!metrics.show_icon)
    {
        return 0;
    }

    egui_view_badge_local_region_to_screen(self, &metrics.icon_region, region);
    return 1;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_badge_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_badge_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_badge_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_badge_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_badge_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_badge_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_badge_on_static_key_event;
#endif
}

static egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_badge_t) = {
        .draw = egui_view_draw,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .dispatch_touch_event = egui_view_dispatch_touch_event,
        .on_touch_event = egui_view_on_touch_event,
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .on_draw = egui_view_badge_on_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

void egui_view_badge_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_badge_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_badge_t);
    egui_view_set_padding_all(self, 2);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);

    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->icon_font = EGUI_FONT_ICON_MS_16;
    local->icon = NULL;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->outline_mode = 1;
    local->subtle_mode = 0;
    egui_view_badge_copy_text(local->text, sizeof(local->text), "Badge");
    egui_view_badge_apply_outline_style(self);
    egui_view_set_view_name(self, "egui_view_badge");
}
