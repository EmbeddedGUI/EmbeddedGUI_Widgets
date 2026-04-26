#include "egui_view_compound_button.h"

#define EGUI_VIEW_COMPOUND_BUTTON_STANDARD_RADIUS    8
#define EGUI_VIEW_COMPOUND_BUTTON_STANDARD_PAD_X     10
#define EGUI_VIEW_COMPOUND_BUTTON_STANDARD_ICON_SIZE 20
#define EGUI_VIEW_COMPOUND_BUTTON_STANDARD_GAP       8
#define EGUI_VIEW_COMPOUND_BUTTON_COMPACT_RADIUS     6
#define EGUI_VIEW_COMPOUND_BUTTON_COMPACT_PAD_X      8
#define EGUI_VIEW_COMPOUND_BUTTON_COMPACT_ICON_SIZE  16
#define EGUI_VIEW_COMPOUND_BUTTON_COMPACT_GAP        6

typedef struct egui_view_compound_button_metrics egui_view_compound_button_metrics_t;
struct egui_view_compound_button_metrics
{
    egui_region_t region;
    egui_region_t icon_region;
    egui_region_t title_region;
    egui_region_t subtitle_region;
};

static uint8_t egui_view_compound_button_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static uint8_t egui_view_compound_button_text_len(const char *text)
{
    uint8_t length = 0;

    if (text == NULL)
    {
        return 0;
    }
    while (text[length] != '\0')
    {
        length++;
    }
    return length;
}

static uint8_t egui_view_compound_button_is_space_char(char c)
{
    return (uint8_t)(c == ' ' || c == '\t');
}

static uint8_t egui_view_compound_button_is_break_after_char(char c)
{
    return (uint8_t)(c == '-' || c == '/');
}

static uint8_t egui_view_compound_button_find_elide_boundary(const char *text, uint8_t visible_chars)
{
    uint8_t index;

    if (text == NULL || visible_chars == 0)
    {
        return 0;
    }

    for (index = visible_chars; index > 0; --index)
    {
        if (egui_view_compound_button_is_space_char(text[index - 1]))
        {
            return (uint8_t)(index - 1);
        }
    }
    for (index = visible_chars; index > 0; --index)
    {
        if (egui_view_compound_button_is_break_after_char(text[index - 1]))
        {
            return index;
        }
    }
    return visible_chars;
}

static void egui_view_compound_button_copy_elided(char *buffer, uint8_t buffer_size, const char *text, uint8_t max_chars)
{
    uint8_t copy_length;
    uint8_t index;
    uint8_t length;

    if (buffer == NULL || buffer_size == 0)
    {
        return;
    }

    buffer[0] = '\0';
    if (text == NULL || max_chars == 0)
    {
        return;
    }

    length = egui_view_compound_button_text_len(text);
    if (length <= max_chars)
    {
        copy_length = length;
        if (copy_length >= buffer_size)
        {
            copy_length = buffer_size - 1;
        }
        for (index = 0; index < copy_length; ++index)
        {
            buffer[index] = text[index];
        }
        buffer[copy_length] = '\0';
        return;
    }

    if (max_chars <= 3)
    {
        copy_length = max_chars;
        if (copy_length >= buffer_size)
        {
            copy_length = buffer_size - 1;
        }
        for (index = 0; index < copy_length; ++index)
        {
            buffer[index] = '.';
        }
        buffer[copy_length] = '\0';
        return;
    }

    copy_length = egui_view_compound_button_find_elide_boundary(text, (uint8_t)(max_chars - 3));
    if (copy_length > buffer_size - 4)
    {
        copy_length = buffer_size - 4;
    }
    for (index = 0; index < copy_length; ++index)
    {
        buffer[index] = text[index];
    }
    buffer[copy_length] = '.';
    buffer[copy_length + 1] = '.';
    buffer[copy_length + 2] = '.';
    buffer[copy_length + 3] = '\0';
}

static egui_dim_t egui_view_compound_button_measure_text_width(const egui_font_t *font, const char *text)
{
    egui_dim_t width = 0;
    egui_dim_t height = 0;

    if (!egui_view_compound_button_has_text(text) || font == NULL || font->api == NULL || font->api->get_str_size == NULL)
    {
        return 0;
    }
    font->api->get_str_size(font, text, 0, 0, &width, &height);
    return width;
}

static void egui_view_compound_button_fit_text_to_width(const egui_font_t *font, const char *text, char *buffer, uint8_t buffer_size,
                                                       egui_dim_t max_width, egui_dim_t fallback_char_width)
{
    uint8_t max_chars;

    if (buffer == NULL || buffer_size == 0)
    {
        return;
    }
    buffer[0] = '\0';
    if (!egui_view_compound_button_has_text(text) || max_width <= 0)
    {
        return;
    }

    max_chars = egui_view_compound_button_text_len(text);
    egui_view_compound_button_copy_elided(buffer, buffer_size, text, max_chars);
    while (max_chars > 0)
    {
        egui_dim_t width = egui_view_compound_button_measure_text_width(font, buffer);

        if (width <= 0)
        {
            width = (egui_dim_t)egui_view_compound_button_text_len(buffer) * fallback_char_width;
        }
        if (width <= max_width)
        {
            break;
        }
        max_chars--;
        egui_view_compound_button_copy_elided(buffer, buffer_size, text, max_chars);
    }
}

static egui_color_t egui_view_compound_button_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 66);
}

static uint8_t egui_view_compound_button_clear_pressed_state(egui_view_t *self, egui_view_compound_button_t *local)
{
    uint8_t had_pressed = (uint8_t)(self->is_pressed || local->pressed_target != EGUI_VIEW_COMPOUND_BUTTON_TARGET_NONE);

    local->pressed_target = EGUI_VIEW_COMPOUND_BUTTON_TARGET_NONE;
    egui_view_set_pressed(self, false);
    return had_pressed;
}

static uint8_t egui_view_compound_button_is_interactive(egui_view_compound_button_t *local, egui_view_t *self)
{
    return (uint8_t)(!local->read_only_mode && egui_view_get_enable(self));
}

static void egui_view_compound_button_apply_style_palette(egui_view_compound_button_t *local, uint8_t style)
{
    local->style = style;
    switch (style)
    {
    case EGUI_VIEW_COMPOUND_BUTTON_STYLE_PRIMARY:
        local->surface_color = EGUI_COLOR_HEX(0x0F6CBD);
        local->pressed_color = EGUI_COLOR_HEX(0x0A5DA3);
        local->border_color = EGUI_COLOR_HEX(0x09579A);
        local->focus_color = EGUI_COLOR_HEX(0x98C8F5);
        local->title_color = EGUI_COLOR_WHITE;
        local->subtitle_color = EGUI_COLOR_HEX(0xD8EBFB);
        local->icon_color = EGUI_COLOR_WHITE;
        break;
    case EGUI_VIEW_COMPOUND_BUTTON_STYLE_SUBTLE:
        local->surface_color = EGUI_COLOR_HEX(0xF7F9FC);
        local->pressed_color = EGUI_COLOR_HEX(0xEEF3F8);
        local->border_color = EGUI_COLOR_HEX(0xD8E0E8);
        local->focus_color = EGUI_COLOR_HEX(0x9FB6CC);
        local->title_color = EGUI_COLOR_HEX(0x1F2C38);
        local->subtitle_color = EGUI_COLOR_HEX(0x657486);
        local->icon_color = EGUI_COLOR_HEX(0x687484);
        break;
    case EGUI_VIEW_COMPOUND_BUTTON_STYLE_DEFAULT:
    default:
        local->style = EGUI_VIEW_COMPOUND_BUTTON_STYLE_DEFAULT;
        local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
        local->pressed_color = EGUI_COLOR_HEX(0xEDF5FF);
        local->border_color = EGUI_COLOR_HEX(0xC8D1DA);
        local->focus_color = EGUI_COLOR_HEX(0x78B7F2);
        local->title_color = EGUI_COLOR_HEX(0x182331);
        local->subtitle_color = EGUI_COLOR_HEX(0x647587);
        local->icon_color = EGUI_COLOR_HEX(0x0F6CBD);
        break;
    }
}

static void egui_view_compound_button_get_metrics(egui_view_compound_button_t *local, egui_view_t *self, egui_view_compound_button_metrics_t *metrics)
{
    egui_dim_t pad_x = local->compact_mode ? EGUI_VIEW_COMPOUND_BUTTON_COMPACT_PAD_X : EGUI_VIEW_COMPOUND_BUTTON_STANDARD_PAD_X;
    egui_dim_t icon_size = local->compact_mode ? EGUI_VIEW_COMPOUND_BUTTON_COMPACT_ICON_SIZE : EGUI_VIEW_COMPOUND_BUTTON_STANDARD_ICON_SIZE;
    egui_dim_t gap = local->compact_mode ? EGUI_VIEW_COMPOUND_BUTTON_COMPACT_GAP : EGUI_VIEW_COMPOUND_BUTTON_STANDARD_GAP;
    egui_dim_t text_x;
    egui_dim_t text_w;
    egui_dim_t text_block_h;
    egui_dim_t title_h = local->compact_mode ? 11 : 13;
    egui_dim_t subtitle_h = local->compact_mode ? 10 : 11;

    egui_view_get_work_region(self, &metrics->region);
    metrics->icon_region = metrics->region;
    metrics->title_region = metrics->region;
    metrics->subtitle_region = metrics->region;
    if (metrics->region.size.width <= 0 || metrics->region.size.height <= 0)
    {
        return;
    }

    metrics->icon_region.location.x = metrics->region.location.x + pad_x;
    metrics->icon_region.location.y = metrics->region.location.y + (metrics->region.size.height - icon_size) / 2;
    metrics->icon_region.size.width = icon_size;
    metrics->icon_region.size.height = icon_size;

    text_x = metrics->icon_region.location.x + icon_size + gap;
    text_w = metrics->region.size.width - (text_x - metrics->region.location.x) - pad_x;
    if (!egui_view_compound_button_has_text(local->icon))
    {
        text_x = metrics->region.location.x + pad_x;
        text_w = metrics->region.size.width - pad_x * 2;
        metrics->icon_region.size.width = 0;
        metrics->icon_region.size.height = 0;
    }
    if (text_w < 0)
    {
        text_w = 0;
    }

    text_block_h = title_h + (egui_view_compound_button_has_text(local->subtitle) ? subtitle_h : 0);
    metrics->title_region.location.x = text_x;
    metrics->title_region.location.y = metrics->region.location.y + (metrics->region.size.height - text_block_h) / 2;
    metrics->title_region.size.width = text_w;
    metrics->title_region.size.height = title_h;

    metrics->subtitle_region.location.x = text_x;
    metrics->subtitle_region.location.y = metrics->title_region.location.y + title_h;
    metrics->subtitle_region.size.width = text_w;
    metrics->subtitle_region.size.height = egui_view_compound_button_has_text(local->subtitle) ? subtitle_h : 0;
}

static uint8_t egui_view_compound_button_hit_test(egui_view_t *self, const egui_view_compound_button_metrics_t *metrics, egui_dim_t screen_x,
                                                  egui_dim_t screen_y)
{
    egui_dim_t local_x = screen_x - self->region_screen.location.x;
    egui_dim_t local_y = screen_y - self->region_screen.location.y;

    return egui_region_pt_in_rect(&metrics->region, local_x, local_y) ? 1 : 0;
}

static void egui_view_compound_button_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                                egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (font == NULL || !egui_view_compound_button_has_text(text) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, align, color, self->alpha);
}

static void egui_view_compound_button_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_compound_button_t);
    egui_view_compound_button_metrics_t metrics;
    egui_color_t fill;
    egui_color_t border;
    egui_color_t title;
    egui_color_t subtitle;
    egui_color_t icon;
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_COMPOUND_BUTTON_COMPACT_RADIUS : EGUI_VIEW_COMPOUND_BUTTON_STANDARD_RADIUS;
    char title_label[44];
    char subtitle_label[52];

    egui_view_compound_button_get_metrics(local, self, &metrics);
    if (metrics.region.size.width <= 0 || metrics.region.size.height <= 0)
    {
        return;
    }

    fill = self->is_pressed ? local->pressed_color : local->surface_color;
    border = local->border_color;
    title = local->title_color;
    subtitle = local->subtitle_color;
    icon = local->icon_color;
    if (!egui_view_get_enable(self))
    {
        fill = egui_view_compound_button_mix_disabled(fill);
        border = egui_view_compound_button_mix_disabled(border);
        title = egui_view_compound_button_mix_disabled(title);
        subtitle = egui_view_compound_button_mix_disabled(subtitle);
        icon = egui_view_compound_button_mix_disabled(icon);
    }
    else if (local->read_only_mode)
    {
        fill = egui_rgb_mix(fill, EGUI_COLOR_WHITE, 35);
        border = egui_rgb_mix(border, local->subtitle_color, 28);
        title = egui_rgb_mix(title, local->subtitle_color, 35);
        subtitle = egui_rgb_mix(subtitle, local->title_color, 10);
        icon = egui_rgb_mix(icon, local->subtitle_color, 45);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.region.location.x, metrics.region.location.y, metrics.region.size.width,
                                          metrics.region.size.height, radius, fill, egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics.region.location.x, metrics.region.location.y, metrics.region.size.width,
                                     metrics.region.size.height, radius, self->is_focused ? 2 : 1, self->is_focused ? local->focus_color : border,
                                     egui_color_alpha_mix(self->alpha, self->is_focused ? 82 : 58));

    if (metrics.icon_region.size.width > 0)
    {
        egui_canvas_draw_circle_fill(&uicode_get_core()->canvas, metrics.icon_region.location.x + metrics.icon_region.size.width / 2,
                                     metrics.icon_region.location.y + metrics.icon_region.size.height / 2, metrics.icon_region.size.width / 2,
                                     egui_rgb_mix(fill, icon, local->style == EGUI_VIEW_COMPOUND_BUTTON_STYLE_PRIMARY ? 18 : 8),
                                     egui_color_alpha_mix(self->alpha, 70));
        egui_view_compound_button_draw_text(local->icon_font, self, local->icon, &metrics.icon_region, EGUI_ALIGN_CENTER, icon);
    }

    egui_view_compound_button_fit_text_to_width(local->title_font, local->title, title_label, sizeof(title_label), metrics.title_region.size.width,
                                                local->compact_mode ? 5 : 6);
    egui_view_compound_button_fit_text_to_width(local->subtitle_font, local->subtitle, subtitle_label, sizeof(subtitle_label),
                                                metrics.subtitle_region.size.width, local->compact_mode ? 4 : 5);
    egui_view_compound_button_draw_text(local->title_font, self, title_label, &metrics.title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title);
    egui_view_compound_button_draw_text(local->subtitle_font, self, subtitle_label, &metrics.subtitle_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                                        subtitle);
}

void egui_view_compound_button_set_content(egui_view_t *self, const char *title, const char *subtitle, const char *icon)
{
    EGUI_LOCAL_INIT(egui_view_compound_button_t);

    egui_view_compound_button_clear_pressed_state(self, local);
    local->title = title;
    local->subtitle = subtitle;
    local->icon = icon;
    egui_view_invalidate(self);
}

void egui_view_compound_button_set_title(egui_view_t *self, const char *title)
{
    EGUI_LOCAL_INIT(egui_view_compound_button_t);

    egui_view_compound_button_clear_pressed_state(self, local);
    local->title = title;
    egui_view_invalidate(self);
}

void egui_view_compound_button_set_subtitle(egui_view_t *self, const char *subtitle)
{
    EGUI_LOCAL_INIT(egui_view_compound_button_t);

    egui_view_compound_button_clear_pressed_state(self, local);
    local->subtitle = subtitle;
    egui_view_invalidate(self);
}

void egui_view_compound_button_set_icon(egui_view_t *self, const char *icon)
{
    EGUI_LOCAL_INIT(egui_view_compound_button_t);

    egui_view_compound_button_clear_pressed_state(self, local);
    local->icon = icon;
    egui_view_invalidate(self);
}

void egui_view_compound_button_set_fonts(egui_view_t *self, const egui_font_t *title_font, const egui_font_t *subtitle_font, const egui_font_t *icon_font)
{
    EGUI_LOCAL_INIT(egui_view_compound_button_t);

    egui_view_compound_button_clear_pressed_state(self, local);
    local->title_font = title_font != NULL ? title_font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->subtitle_font = subtitle_font != NULL ? subtitle_font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->icon_font = icon_font != NULL ? icon_font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_invalidate(self);
}

void egui_view_compound_button_set_style(egui_view_t *self, uint8_t style)
{
    EGUI_LOCAL_INIT(egui_view_compound_button_t);

    egui_view_compound_button_clear_pressed_state(self, local);
    egui_view_compound_button_apply_style_palette(local, style);
    egui_view_invalidate(self);
}

void egui_view_compound_button_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_compound_button_t);

    egui_view_compound_button_clear_pressed_state(self, local);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

void egui_view_compound_button_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_compound_button_t);

    egui_view_compound_button_clear_pressed_state(self, local);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

void egui_view_compound_button_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t pressed_color, egui_color_t border_color,
                                           egui_color_t focus_color, egui_color_t title_color, egui_color_t subtitle_color, egui_color_t icon_color)
{
    EGUI_LOCAL_INIT(egui_view_compound_button_t);

    egui_view_compound_button_clear_pressed_state(self, local);
    local->surface_color = surface_color;
    local->pressed_color = pressed_color;
    local->border_color = border_color;
    local->focus_color = focus_color;
    local->title_color = title_color;
    local->subtitle_color = subtitle_color;
    local->icon_color = icon_color;
    egui_view_invalidate(self);
}

void egui_view_compound_button_set_on_action_listener(egui_view_t *self, egui_view_compound_button_action_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_compound_button_t);
    local->on_action = listener;
}

uint8_t egui_view_compound_button_activate(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_compound_button_t);

    if (!egui_view_compound_button_is_interactive(local, self))
    {
        return 0;
    }
    if (local->on_action != NULL)
    {
        local->on_action(self);
    }
    egui_view_invalidate(self);
    return 1;
}

uint8_t egui_view_compound_button_get_button_region(egui_view_t *self, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_compound_button_t);
    egui_view_compound_button_metrics_t metrics;

    if (region == NULL)
    {
        return 0;
    }
    egui_view_compound_button_get_metrics(local, self, &metrics);
    *region = metrics.region;
    region->location.x += self->region_screen.location.x;
    region->location.y += self->region_screen.location.y;
    return region->size.width > 0 && region->size.height > 0 ? 1 : 0;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_compound_button_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_compound_button_t);
    egui_view_compound_button_metrics_t metrics;
    uint8_t hit;

    if (!egui_view_compound_button_is_interactive(local, self))
    {
        if (egui_view_compound_button_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    egui_view_compound_button_get_metrics(local, self, &metrics);
    hit = egui_view_compound_button_hit_test(self, &metrics, event->location.x, event->location.y);
    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        if (!hit)
        {
            return 0;
        }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        local->pressed_target = EGUI_VIEW_COMPOUND_BUTTON_TARGET_BODY;
        egui_view_set_pressed(self, true);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_target == EGUI_VIEW_COMPOUND_BUTTON_TARGET_NONE)
        {
            return 0;
        }
        egui_view_set_pressed(self, hit ? true : false);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
    {
        uint8_t had_target = (uint8_t)(local->pressed_target == EGUI_VIEW_COMPOUND_BUTTON_TARGET_BODY);

        if (had_target && self->is_pressed && hit)
        {
            egui_view_compound_button_activate(self);
        }
        if (egui_view_compound_button_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return had_target || hit;
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (egui_view_compound_button_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        break;
    }
    return 0;
}

static int egui_view_compound_button_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_compound_button_t);
    EGUI_UNUSED(event);

    if (egui_view_compound_button_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_compound_button_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_compound_button_t);
    uint8_t was_pressed = (uint8_t)(local->pressed_target == EGUI_VIEW_COMPOUND_BUTTON_TARGET_BODY && self->is_pressed);

    if (!egui_view_compound_button_is_interactive(local, self))
    {
        if (egui_view_compound_button_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_ENTER:
    case EGUI_KEY_CODE_SPACE:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->pressed_target = EGUI_VIEW_COMPOUND_BUTTON_TARGET_BODY;
            egui_view_set_pressed(self, true);
            egui_view_invalidate(self);
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            if (was_pressed)
            {
                egui_view_compound_button_activate(self);
            }
            egui_view_compound_button_clear_pressed_state(self, local);
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        if (egui_view_compound_button_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_compound_button_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_compound_button_t);
    EGUI_UNUSED(event);

    if (egui_view_compound_button_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_compound_button_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_compound_button_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_compound_button_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_compound_button_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_compound_button_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_compound_button_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_compound_button_on_key_event,
#endif
};

void egui_view_compound_button_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_compound_button_t);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_compound_button_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif

    local->title = "";
    local->subtitle = "";
    local->icon = NULL;
    local->title_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->subtitle_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->icon_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->on_action = NULL;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_target = EGUI_VIEW_COMPOUND_BUTTON_TARGET_NONE;
    egui_view_compound_button_apply_style_palette(local, EGUI_VIEW_COMPOUND_BUTTON_STYLE_DEFAULT);
    egui_view_set_view_name(self, "egui_view_compound_button");
}
