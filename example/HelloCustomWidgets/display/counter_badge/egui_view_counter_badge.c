#include "egui_view_counter_badge.h"

#include <string.h>

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_notification_badge.h"
#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_notification_badge_internal.h"

typedef struct egui_view_counter_badge_metrics egui_view_counter_badge_metrics_t;
struct egui_view_counter_badge_metrics
{
    egui_region_t badge_region;
    egui_region_t text_region;
    uint8_t show_badge;
    uint8_t use_circle;
};

static egui_view_notification_badge_t *egui_view_counter_badge_as_notification(egui_view_t *self)
{
    return (egui_view_notification_badge_t *)self;
}

static egui_color_t egui_view_counter_badge_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x83909D), 48);
}

static uint8_t egui_view_counter_badge_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static const egui_font_t *egui_view_counter_badge_get_text_font(const egui_view_counter_badge_t *local)
{
    return local->font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : local->font;
}

static void egui_view_counter_badge_local_region_to_screen(egui_view_t *self, const egui_region_t *local_region, egui_region_t *screen_region)
{
    screen_region->location.x = self->region_screen.location.x + local_region->location.x;
    screen_region->location.y = self->region_screen.location.y + local_region->location.y;
    screen_region->size.width = local_region->size.width;
    screen_region->size.height = local_region->size.height;
}

static void egui_view_counter_badge_get_metrics(egui_view_t *self, egui_view_counter_badge_t *local, egui_view_counter_badge_metrics_t *metrics)
{
    egui_view_notification_badge_t *notification = egui_view_counter_badge_as_notification(self);
    egui_region_t region;
    egui_dim_t min_side;
    egui_dim_t text_width = 0;
    egui_dim_t text_height = 0;
    egui_dim_t badge_w;
    egui_dim_t badge_h;
    egui_dim_t pad_x;
    const egui_font_t *font;

    memset(metrics, 0, sizeof(*metrics));
    egui_view_get_work_region(self, &region);
    if (egui_region_is_empty(&region))
    {
        return;
    }

    min_side = EGUI_MIN(region.size.width, region.size.height);
    if (min_side <= 0)
    {
        return;
    }

    if (local->dot_mode)
    {
        egui_dim_t dot_d = local->compact_mode ? 8 : 10;

        if (!local->compact_mode && min_side >= 18)
        {
            dot_d = 12;
        }
        if (dot_d > min_side)
        {
            dot_d = min_side;
        }
        if (dot_d <= 0)
        {
            return;
        }

        metrics->badge_region.location.x = region.location.x + (region.size.width - dot_d) / 2;
        metrics->badge_region.location.y = region.location.y + (region.size.height - dot_d) / 2;
        metrics->badge_region.size.width = dot_d;
        metrics->badge_region.size.height = dot_d;
        metrics->show_badge = 1;
        metrics->use_circle = 1;
        return;
    }

    if (local->count == 0)
    {
        return;
    }

    font = egui_view_counter_badge_get_text_font(local);
    egui_view_notification_badge_format_count_text(notification);
    if (font != NULL && font->api != NULL && font->api->get_str_size != NULL)
    {
        font->api->get_str_size(font, local->text_buffer, 0, 0, &text_width, &text_height);
    }
    if (text_height <= 0)
    {
        text_height = local->compact_mode ? 8 : 10;
    }

    badge_h = text_height + (local->compact_mode ? 6 : 8);
    if (badge_h < (local->compact_mode ? 12 : 14))
    {
        badge_h = local->compact_mode ? 12 : 14;
    }
    if (badge_h > region.size.height)
    {
        badge_h = region.size.height;
    }

    pad_x = local->compact_mode ? 4 : 6;
    badge_w = text_width + pad_x * 2;
    if (badge_w < badge_h)
    {
        badge_w = badge_h;
    }
    if (badge_w > region.size.width)
    {
        badge_w = region.size.width;
    }

    metrics->badge_region.location.x = region.location.x + (region.size.width - badge_w) / 2;
    metrics->badge_region.location.y = region.location.y + (region.size.height - badge_h) / 2;
    metrics->badge_region.size.width = badge_w;
    metrics->badge_region.size.height = badge_h;
    metrics->use_circle = (uint8_t)(badge_w <= badge_h + 2);
    metrics->show_badge = egui_view_notification_badge_get_text_region(&metrics->badge_region, &metrics->text_region);
}

static void egui_view_counter_badge_draw_outline(const egui_region_t *badge_region, uint8_t use_circle, egui_color_t outline_color, egui_alpha_t alpha)
{
    if (alpha == 0)
    {
        return;
    }

    if (use_circle)
    {
        egui_dim_t radius = EGUI_MIN(badge_region->size.width, badge_region->size.height) / 2 - 1;

        if (radius <= 0)
        {
            return;
        }

        egui_canvas_draw_circle(badge_region->location.x + badge_region->size.width / 2, badge_region->location.y + badge_region->size.height / 2, radius, 1,
                                outline_color, alpha);
        return;
    }

    egui_canvas_draw_round_rectangle(badge_region->location.x, badge_region->location.y, badge_region->size.width, badge_region->size.height,
                                     badge_region->size.height / 2, 1, outline_color, alpha);
}

static void egui_view_counter_badge_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_counter_badge_t);
    egui_view_counter_badge_metrics_t metrics;
    egui_color_t badge_color = local->badge_color;
    egui_color_t text_color = local->text_color;
    egui_color_t outline_color = local->outline_color;
    egui_alpha_t outline_alpha = local->dot_mode ? 72 : 88;
    const egui_font_t *font = egui_view_counter_badge_get_text_font(local);

    egui_view_counter_badge_get_metrics(self, local, &metrics);
    if (!metrics.show_badge)
    {
        return;
    }

    if (local->read_only_mode)
    {
        badge_color = egui_rgb_mix(badge_color, EGUI_COLOR_HEX(0xF5F7FA), 34);
        text_color = egui_rgb_mix(text_color, EGUI_COLOR_HEX(0x7C8794), 42);
        outline_color = egui_rgb_mix(outline_color, EGUI_COLOR_HEX(0xD6DEE6), 42);
        outline_alpha = local->dot_mode ? 60 : 72;
    }
    if (!egui_view_get_enable(self))
    {
        badge_color = egui_view_counter_badge_mix_disabled(badge_color);
        text_color = egui_view_counter_badge_mix_disabled(text_color);
        outline_color = egui_view_counter_badge_mix_disabled(outline_color);
        outline_alpha = 58;
    }

    if (local->dot_mode)
    {
        egui_dim_t radius = EGUI_MIN(metrics.badge_region.size.width, metrics.badge_region.size.height) / 2 - 1;

        if (radius <= 0)
        {
            return;
        }

        egui_canvas_draw_circle_fill(metrics.badge_region.location.x + metrics.badge_region.size.width / 2,
                                     metrics.badge_region.location.y + metrics.badge_region.size.height / 2, radius, badge_color,
                                     egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
        egui_view_counter_badge_draw_outline(&metrics.badge_region, 1, outline_color, egui_color_alpha_mix(self->alpha, outline_alpha));
        return;
    }

    egui_view_notification_badge_draw_background(&metrics.badge_region, badge_color, metrics.use_circle);
    egui_view_counter_badge_draw_outline(&metrics.badge_region, metrics.use_circle, outline_color, egui_color_alpha_mix(self->alpha, outline_alpha));
    egui_canvas_draw_text_in_rect(font, local->text_buffer, &metrics.text_region, EGUI_ALIGN_CENTER, text_color,
                                  egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
}

void egui_view_counter_badge_set_count(egui_view_t *self, uint16_t count)
{
    EGUI_LOCAL_INIT(egui_view_counter_badge_t);

    egui_view_counter_badge_clear_pressed_state(self);
    local->count = count;
    egui_view_invalidate(self);
}

uint16_t egui_view_counter_badge_get_count(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_counter_badge_t);
    return local->count;
}

void egui_view_counter_badge_set_max_display(egui_view_t *self, uint8_t max_display)
{
    EGUI_LOCAL_INIT(egui_view_counter_badge_t);

    egui_view_counter_badge_clear_pressed_state(self);
    local->max_display = max_display < 1 ? 1 : max_display;
    egui_view_invalidate(self);
}

uint8_t egui_view_counter_badge_get_max_display(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_counter_badge_t);
    return local->max_display;
}

void egui_view_counter_badge_set_dot_mode(egui_view_t *self, uint8_t dot_mode)
{
    EGUI_LOCAL_INIT(egui_view_counter_badge_t);

    egui_view_counter_badge_clear_pressed_state(self);
    local->dot_mode = dot_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_counter_badge_get_dot_mode(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_counter_badge_t);
    return local->dot_mode;
}

void egui_view_counter_badge_set_palette(egui_view_t *self, egui_color_t badge_color, egui_color_t text_color, egui_color_t outline_color)
{
    EGUI_LOCAL_INIT(egui_view_counter_badge_t);

    egui_view_counter_badge_clear_pressed_state(self);
    local->badge_color = badge_color;
    local->text_color = text_color;
    local->outline_color = outline_color;
    egui_view_invalidate(self);
}

void egui_view_counter_badge_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_counter_badge_t);

    egui_view_counter_badge_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_counter_badge_get_compact_mode(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_counter_badge_t);
    return local->compact_mode;
}

void egui_view_counter_badge_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_counter_badge_t);

    egui_view_counter_badge_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_counter_badge_get_read_only_mode(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_counter_badge_t);
    return local->read_only_mode;
}

uint8_t egui_view_counter_badge_get_badge_region(egui_view_t *self, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_counter_badge_t);
    egui_view_counter_badge_metrics_t metrics;

    if (region == NULL)
    {
        return 0;
    }

    egui_view_counter_badge_get_metrics(self, local, &metrics);
    if (!metrics.show_badge)
    {
        return 0;
    }

    egui_view_counter_badge_local_region_to_screen(self, &metrics.badge_region, region);
    return 1;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_counter_badge_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_counter_badge_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_counter_badge_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_counter_badge_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_counter_badge_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_counter_badge_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_counter_badge_on_static_key_event;
#endif
}

static egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_counter_badge_t) = {
        .draw = egui_view_draw,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .dispatch_touch_event = egui_view_dispatch_touch_event,
        .on_touch_event = egui_view_on_touch_event,
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .on_draw = egui_view_counter_badge_on_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

void egui_view_counter_badge_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_counter_badge_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_counter_badge_t);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);

    local->count = 1;
    local->max_display = 99;
    local->badge_color = EGUI_COLOR_HEX(0xC42B1C);
    local->text_color = EGUI_COLOR_WHITE;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->content_style = EGUI_VIEW_NOTIFICATION_BADGE_CONTENT_STYLE_COUNT;
    local->icon = NULL;
    local->icon_font = NULL;
    memset(local->text_buffer, 0, sizeof(local->text_buffer));
    local->outline_color = EGUI_COLOR_WHITE;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->dot_mode = 0;

    egui_view_set_view_name(self, "egui_view_counter_badge");
}
