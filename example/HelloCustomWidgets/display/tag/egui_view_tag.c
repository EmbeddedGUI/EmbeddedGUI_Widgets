#include "egui_view_tag.h"

#include <string.h>

#include "../../../../sdk/EmbeddedGUI/src/resource/egui_icon_material_symbols.h"

typedef struct egui_view_tag_metrics egui_view_tag_metrics_t;
struct egui_view_tag_metrics
{
    egui_region_t primary_region;
    egui_region_t secondary_region;
    egui_region_t dismiss_region;
    uint8_t show_secondary;
    uint8_t show_dismiss;
};

static egui_color_t egui_view_tag_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x7B8794), 48);
}

static void egui_view_tag_copy_text(char *dst, uint8_t capacity, const char *src)
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

static void egui_view_tag_measure_text(const egui_font_t *font, const char *text, egui_dim_t *width, egui_dim_t *height)
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

static uint8_t egui_view_tag_has_dismiss(const egui_view_t *self, const egui_view_tag_t *local)
{
    return local->dismissible && !local->read_only_mode && egui_view_get_enable((egui_view_t *)self);
}

static uint8_t egui_view_tag_clear_pressed_state(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_tag_t);
    uint8_t had_pressed = (uint8_t)(local->dismiss_pressed || egui_view_get_pressed(self));

    local->dismiss_pressed = 0;
    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static const egui_font_t *egui_view_tag_get_text_font(const egui_view_tag_t *local)
{
    return local->font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : local->font;
}

static const egui_font_t *egui_view_tag_get_secondary_font(const egui_view_tag_t *local)
{
    return local->secondary_font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : local->secondary_font;
}

static const egui_font_t *egui_view_tag_get_icon_font(const egui_view_tag_t *local)
{
    return local->icon_font == NULL ? EGUI_FONT_ICON_MS_16 : local->icon_font;
}

static void egui_view_tag_local_region_to_screen(egui_view_t *self, const egui_region_t *local_region, egui_region_t *screen_region)
{
    screen_region->location.x = self->region_screen.location.x + local_region->location.x;
    screen_region->location.y = self->region_screen.location.y + local_region->location.y;
    screen_region->size.width = local_region->size.width;
    screen_region->size.height = local_region->size.height;
}

static void egui_view_tag_get_metrics(egui_view_t *self, egui_view_tag_t *local, egui_view_tag_metrics_t *metrics)
{
    egui_region_t region;
    egui_dim_t pad_x = local->compact_mode ? 7 : 10;
    egui_dim_t gap = local->compact_mode ? 4 : 6;
    egui_dim_t dismiss_size = local->compact_mode ? 12 : 14;
    egui_dim_t primary_width = 0;
    egui_dim_t primary_height = 0;
    egui_dim_t secondary_width = 0;
    egui_dim_t secondary_height = 0;
    egui_dim_t available_width;
    egui_dim_t secondary_max_width;

    memset(metrics, 0, sizeof(*metrics));
    egui_view_get_work_region(self, &region);
    if (egui_region_is_empty(&region))
    {
        return;
    }

    metrics->show_dismiss = egui_view_tag_has_dismiss(self, local) ? 1 : 0;

    if (metrics->show_dismiss)
    {
        metrics->dismiss_region.size.width = dismiss_size;
        metrics->dismiss_region.size.height = dismiss_size;
        metrics->dismiss_region.location.x = region.location.x + region.size.width - pad_x - dismiss_size;
        metrics->dismiss_region.location.y = region.location.y + (region.size.height - dismiss_size) / 2;
    }

    available_width = region.size.width - pad_x * 2;
    if (metrics->show_dismiss)
    {
        available_width -= dismiss_size + gap;
    }
    if (available_width <= 0)
    {
        return;
    }

    egui_view_tag_measure_text(egui_view_tag_get_text_font(local), local->text, &primary_width, &primary_height);
    egui_view_tag_measure_text(egui_view_tag_get_secondary_font(local), local->secondary_text, &secondary_width, &secondary_height);

    metrics->show_secondary = local->secondary_text[0] != '\0' ? 1 : 0;
    if (metrics->show_secondary)
    {
        secondary_max_width = available_width / 2;
        if (secondary_width > secondary_max_width)
        {
            secondary_width = secondary_max_width;
        }
        if (secondary_width < 18)
        {
            secondary_width = 18;
        }
        if (primary_width + secondary_width + gap > available_width && available_width > secondary_width + gap)
        {
            primary_width = available_width - secondary_width - gap;
        }
        else if (primary_width + secondary_width + gap <= available_width)
        {
            primary_width = available_width - secondary_width - gap;
        }
        else
        {
            metrics->show_secondary = 0;
            primary_width = available_width;
        }
    }
    else
    {
        primary_width = available_width;
    }

    metrics->primary_region.location.x = region.location.x + pad_x;
    metrics->primary_region.location.y = region.location.y;
    metrics->primary_region.size.width = primary_width;
    metrics->primary_region.size.height = region.size.height;

    if (metrics->show_secondary)
    {
        metrics->secondary_region.location.x = metrics->primary_region.location.x + primary_width + gap;
        metrics->secondary_region.location.y = region.location.y;
        metrics->secondary_region.size.width = available_width - primary_width - gap;
        metrics->secondary_region.size.height = region.size.height;
    }
}

static void egui_view_tag_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, egui_color_t color,
                                    egui_alpha_t alpha)
{
    egui_region_t draw_region = *region;

    if (text == NULL || text[0] == '\0')
    {
        return;
    }

    egui_canvas_draw_text_in_rect(font, text, &draw_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, color, egui_color_alpha_mix(self->alpha, alpha));
}

static void egui_view_tag_notify_dismiss(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_tag_t);

    if (local->on_dismiss != NULL)
    {
        local->on_dismiss(self);
    }
    else
    {
        egui_view_invalidate(self);
    }
}

static void egui_view_tag_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_tag_t);
    egui_view_tag_metrics_t metrics;
    egui_region_t region;
    egui_color_t fill_color = local->surface_color;
    egui_color_t border_color = local->border_color;
    egui_color_t text_color = local->text_color;
    egui_color_t secondary_color = local->secondary_color;
    egui_color_t accent_color = local->accent_color;
    egui_color_t dismiss_fill;
    egui_color_t dismiss_icon;
    egui_dim_t radius = local->compact_mode ? 8 : 10;

    egui_view_get_work_region(self, &region);
    if (egui_region_is_empty(&region))
    {
        return;
    }

    egui_view_tag_get_metrics(self, local, &metrics);
    if (self->is_focused && !local->read_only_mode && egui_view_tag_has_dismiss(self, local))
    {
        fill_color = egui_rgb_mix(fill_color, accent_color, local->compact_mode ? 6 : 8);
        border_color = egui_rgb_mix(border_color, accent_color, 44);
    }
    if (self->is_pressed && !local->read_only_mode)
    {
        fill_color = egui_rgb_mix(fill_color, accent_color, 12);
        border_color = egui_rgb_mix(border_color, accent_color, 56);
        text_color = egui_rgb_mix(text_color, accent_color, 16);
    }
    if (local->read_only_mode)
    {
        border_color = egui_rgb_mix(border_color, EGUI_COLOR_HEX(0x9AA6B2), 20);
        text_color = egui_rgb_mix(text_color, EGUI_COLOR_HEX(0x8A96A2), 26);
        secondary_color = egui_rgb_mix(secondary_color, EGUI_COLOR_HEX(0x99A5B0), 32);
    }
    if (!egui_view_get_enable(self))
    {
        fill_color = egui_view_tag_mix_disabled(fill_color);
        border_color = egui_view_tag_mix_disabled(border_color);
        text_color = egui_view_tag_mix_disabled(text_color);
        secondary_color = egui_view_tag_mix_disabled(secondary_color);
    }

    egui_canvas_draw_round_rectangle_fill(region.location.x, region.location.y, region.size.width, region.size.height, radius, fill_color,
                                          egui_color_alpha_mix(self->alpha, 100));
    egui_canvas_draw_round_rectangle(region.location.x, region.location.y, region.size.width, region.size.height, radius, 1, border_color,
                                     egui_color_alpha_mix(self->alpha, self->is_focused ? 88 : 64));

    egui_view_tag_draw_text(egui_view_tag_get_text_font(local), self, local->text, &metrics.primary_region, text_color, EGUI_ALPHA_100);
    if (metrics.show_secondary)
    {
        egui_view_tag_draw_text(egui_view_tag_get_secondary_font(local), self, local->secondary_text, &metrics.secondary_region, secondary_color,
                                EGUI_ALPHA_100);
    }

    if (metrics.show_dismiss)
    {
        egui_dim_t radius_fill = EGUI_MAX(metrics.dismiss_region.size.width / 2 - 1, 4);
        egui_alpha_t fill_alpha = local->dismiss_pressed ? 78 : (self->is_focused ? 52 : 38);

        dismiss_fill = egui_rgb_mix(local->surface_color, accent_color, local->dismiss_pressed ? 18 : 10);
        dismiss_icon = local->dismiss_pressed ? egui_rgb_mix(text_color, accent_color, 30) : egui_rgb_mix(secondary_color, text_color, 18);
        if (!egui_view_get_enable(self))
        {
            dismiss_fill = egui_view_tag_mix_disabled(dismiss_fill);
            dismiss_icon = egui_view_tag_mix_disabled(dismiss_icon);
        }

        egui_canvas_draw_circle_fill_basic(metrics.dismiss_region.location.x + metrics.dismiss_region.size.width / 2,
                                           metrics.dismiss_region.location.y + metrics.dismiss_region.size.height / 2, radius_fill, dismiss_fill, fill_alpha);
        egui_canvas_draw_text_in_rect(egui_view_tag_get_icon_font(local), EGUI_ICON_MS_CLOSE, &metrics.dismiss_region, EGUI_ALIGN_CENTER, dismiss_icon,
                                      egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
    }
}

void egui_view_tag_apply_standard_style(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_tag_t);

    egui_view_tag_clear_pressed_state(self);
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xD5DDE6);
    local->text_color = EGUI_COLOR_HEX(0x1F2A35);
    local->secondary_color = EGUI_COLOR_HEX(0x637283);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    egui_view_invalidate(self);
}

void egui_view_tag_apply_compact_style(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_tag_t);

    egui_view_tag_clear_pressed_state(self);
    local->compact_mode = 1;
    local->read_only_mode = 0;
    local->surface_color = EGUI_COLOR_HEX(0xF8FBFD);
    local->border_color = EGUI_COLOR_HEX(0xCCD8E3);
    local->text_color = EGUI_COLOR_HEX(0x1D3440);
    local->secondary_color = EGUI_COLOR_HEX(0x6A7A88);
    local->accent_color = EGUI_COLOR_HEX(0x0C7C73);
    egui_view_invalidate(self);
}

void egui_view_tag_apply_read_only_style(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_tag_t);

    egui_view_tag_clear_pressed_state(self);
    local->compact_mode = 1;
    local->read_only_mode = 1;
    local->surface_color = EGUI_COLOR_HEX(0xF5F7FA);
    local->border_color = EGUI_COLOR_HEX(0xD7DEE6);
    local->text_color = EGUI_COLOR_HEX(0x73808C);
    local->secondary_color = EGUI_COLOR_HEX(0x8E99A4);
    local->accent_color = EGUI_COLOR_HEX(0x9AA6B2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_clear_focus(self);
#endif
    egui_view_invalidate(self);
}

void egui_view_tag_set_text(egui_view_t *self, const char *text)
{
    EGUI_LOCAL_INIT(egui_view_tag_t);

    egui_view_tag_clear_pressed_state(self);
    egui_view_tag_copy_text(local->text, sizeof(local->text), text);
    egui_view_invalidate(self);
}

void egui_view_tag_set_secondary_text(egui_view_t *self, const char *text)
{
    EGUI_LOCAL_INIT(egui_view_tag_t);

    egui_view_tag_clear_pressed_state(self);
    egui_view_tag_copy_text(local->secondary_text, sizeof(local->secondary_text), text);
    egui_view_invalidate(self);
}

void egui_view_tag_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_tag_t);

    egui_view_tag_clear_pressed_state(self);
    local->font = font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : font;
    egui_view_invalidate(self);
}

void egui_view_tag_set_secondary_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_tag_t);

    egui_view_tag_clear_pressed_state(self);
    local->secondary_font = font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : font;
    egui_view_invalidate(self);
}

void egui_view_tag_set_icon_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_tag_t);

    egui_view_tag_clear_pressed_state(self);
    local->icon_font = font == NULL ? EGUI_FONT_ICON_MS_16 : font;
    egui_view_invalidate(self);
}

void egui_view_tag_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                               egui_color_t secondary_color, egui_color_t accent_color)
{
    EGUI_LOCAL_INIT(egui_view_tag_t);

    egui_view_tag_clear_pressed_state(self);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->secondary_color = secondary_color;
    local->accent_color = accent_color;
    egui_view_invalidate(self);
}

void egui_view_tag_set_dismissible(egui_view_t *self, uint8_t dismissible)
{
    EGUI_LOCAL_INIT(egui_view_tag_t);

    egui_view_tag_clear_pressed_state(self);
    local->dismissible = dismissible ? 1 : 0;
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (!local->dismissible)
    {
        egui_view_clear_focus(self);
    }
#endif
    egui_view_invalidate(self);
}

void egui_view_tag_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_tag_t);

    egui_view_tag_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

void egui_view_tag_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_tag_t);

    egui_view_tag_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (local->read_only_mode)
    {
        egui_view_clear_focus(self);
    }
#endif
    egui_view_invalidate(self);
}

void egui_view_tag_set_on_dismiss_listener(egui_view_t *self, egui_view_on_tag_dismiss_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_tag_t);

    local->on_dismiss = listener;
}

uint8_t egui_view_tag_get_dismiss_region(egui_view_t *self, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_tag_t);
    egui_view_tag_metrics_t metrics;

    if (region == NULL)
    {
        return 0;
    }

    egui_view_tag_get_metrics(self, local, &metrics);
    if (!metrics.show_dismiss)
    {
        return 0;
    }

    egui_view_tag_local_region_to_screen(self, &metrics.dismiss_region, region);
    return 1;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_tag_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_tag_t);
    egui_region_t dismiss_region;
    uint8_t inside_dismiss = 0;
    uint8_t had_pressed = local->dismiss_pressed;

    if (!egui_view_tag_has_dismiss(self, local))
    {
        return egui_view_tag_clear_pressed_state(self);
    }

    if (!egui_view_tag_get_dismiss_region(self, &dismiss_region))
    {
        return egui_view_tag_clear_pressed_state(self);
    }

    inside_dismiss = egui_region_pt_in_rect(&dismiss_region, event->location.x, event->location.y);
    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        if (inside_dismiss)
        {
            local->dismiss_pressed = 1;
            egui_view_set_pressed(self, 1);
            return 1;
        }
        break;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (had_pressed != inside_dismiss)
        {
            local->dismiss_pressed = inside_dismiss;
            egui_view_set_pressed(self, inside_dismiss);
        }
        return had_pressed ? 1 : 0;
    case EGUI_MOTION_EVENT_ACTION_UP:
        if (had_pressed)
        {
            local->dismiss_pressed = 0;
            egui_view_set_pressed(self, 0);
            if (inside_dismiss)
            {
                egui_view_tag_notify_dismiss(self);
            }
            return 1;
        }
        break;
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (had_pressed)
        {
            local->dismiss_pressed = 0;
            egui_view_set_pressed(self, 0);
            return 1;
        }
        break;
    default:
        break;
    }

    return 0;
}

static int egui_view_tag_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_tag_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_tag_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    uint8_t was_pressed = egui_view_tag_clear_pressed_state(self);
    EGUI_LOCAL_INIT(egui_view_tag_t);

    if (!egui_view_tag_has_dismiss(self, local))
    {
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_DELETE:
    case EGUI_KEY_CODE_BACKSPACE:
    case EGUI_KEY_CODE_ENTER:
    case EGUI_KEY_CODE_SPACE:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->dismiss_pressed = 1;
            egui_view_set_pressed(self, 1);
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            if (was_pressed)
            {
                egui_view_tag_notify_dismiss(self);
            }
            return 1;
        }
        return 0;
    default:
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_tag_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_tag_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_tag_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_tag_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_tag_on_static_key_event;
#endif
}

static egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_tag_t) = {
        .draw = egui_view_draw,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_tag_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .on_draw = egui_view_tag_on_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_tag_on_key_event,
#endif
};

void egui_view_tag_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_tag_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_tag_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 1);
#endif

    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->secondary_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->icon_font = EGUI_FONT_ICON_MS_16;
    local->on_dismiss = NULL;
    local->dismissible = 1;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->dismiss_pressed = 0;
    egui_view_tag_copy_text(local->text, sizeof(local->text), "Tag");
    egui_view_tag_copy_text(local->secondary_text, sizeof(local->secondary_text), "");
    egui_view_tag_apply_standard_style(self);
    egui_view_set_view_name(self, "egui_view_tag");
}
