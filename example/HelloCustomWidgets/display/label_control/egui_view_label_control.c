#include "egui_view_label_control.h"
#include "../../hcw_selection_marker.h"

#include <string.h>

#define EGUI_VIEW_LABEL_CONTROL_RADIUS          8
#define EGUI_VIEW_LABEL_CONTROL_REQUIRED_RADIUS 2

static egui_view_label_control_t *egui_view_label_control_local(egui_view_t *self)
{
    return (egui_view_label_control_t *)self;
}

static uint8_t egui_view_label_control_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static uint8_t egui_view_label_control_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static uint8_t egui_view_label_control_text_len(const char *text)
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

static void egui_view_label_control_copy_text(char *dst, uint8_t capacity, const char *src)
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

static void egui_view_label_control_copy_elided(char *buffer, uint8_t buffer_size, const char *text, uint8_t max_chars)
{
    uint8_t length;
    uint8_t copy_length;
    uint8_t index;

    if (buffer == NULL || buffer_size == 0)
    {
        return;
    }

    buffer[0] = '\0';
    if (text == NULL || max_chars == 0)
    {
        return;
    }

    length = egui_view_label_control_text_len(text);
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

    copy_length = (uint8_t)(max_chars - 3);
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

static egui_dim_t egui_view_label_control_measure_text_width(const egui_font_t *font, const char *text)
{
    egui_dim_t width = 0;
    egui_dim_t height = 0;

    if (!egui_view_label_control_has_text(text) || font == NULL || font->api == NULL || font->api->get_str_size == NULL)
    {
        return 0;
    }
    font->api->get_str_size(font, text, 0, 0, &width, &height);
    return width;
}

static void egui_view_label_control_fit_text_to_width(const egui_font_t *font, const char *text, char *buffer, uint8_t buffer_size,
                                                      egui_dim_t max_width, egui_dim_t fallback_char_width)
{
    uint8_t max_chars;

    if (buffer == NULL || buffer_size == 0)
    {
        return;
    }
    buffer[0] = '\0';
    if (!egui_view_label_control_has_text(text) || max_width <= 0)
    {
        return;
    }

    max_chars = egui_view_label_control_text_len(text);
    egui_view_label_control_copy_elided(buffer, buffer_size, text, max_chars);
    while (max_chars > 0)
    {
        egui_dim_t width = egui_view_label_control_measure_text_width(font, buffer);

        if (width <= 0)
        {
            width = (egui_dim_t)egui_view_label_control_text_len(buffer) * fallback_char_width;
        }
        if (width <= max_width)
        {
            break;
        }
        max_chars--;
        egui_view_label_control_copy_elided(buffer, buffer_size, text, max_chars);
    }
}

static egui_color_t egui_view_label_control_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_MAKE(38));
}

static void egui_view_label_control_clamp_access_key(egui_view_label_control_t *local)
{
    uint8_t length = egui_view_label_control_text_len(local->text);

    if (local->access_key_index != EGUI_VIEW_LABEL_CONTROL_ACCESS_NONE && local->access_key_index >= length)
    {
        local->access_key_index = EGUI_VIEW_LABEL_CONTROL_ACCESS_NONE;
    }
}

static void egui_view_label_control_get_text_regions(egui_view_t *self, egui_view_label_control_t *local, egui_region_t *text_region,
                                                     egui_region_t *hint_region)
{
    egui_region_t work_region;
    egui_dim_t left_gap = 14;
    egui_dim_t right_gap = local->required ? 13 : 7;

    egui_view_get_work_region(self, &work_region);
    if (work_region.size.width < left_gap + right_gap)
    {
        work_region.size.width = left_gap + right_gap;
    }

    text_region->location.x = work_region.location.x + left_gap;
    text_region->location.y = work_region.location.y + 1;
    text_region->size.width = work_region.size.width - left_gap - right_gap;
    text_region->size.height = !egui_view_label_control_has_text(local->target_hint) ? work_region.size.height : work_region.size.height / 2;
    if (text_region->size.height < 10)
    {
        text_region->size.height = work_region.size.height;
    }

    *hint_region = *text_region;
    hint_region->location.y = text_region->location.y + text_region->size.height;
    hint_region->size.height = work_region.location.y + work_region.size.height - hint_region->location.y;
    if (hint_region->size.height < 0)
    {
        hint_region->size.height = 0;
    }
}

static void egui_view_label_control_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region,
                                              uint8_t align, egui_color_t color, egui_alpha_t alpha)
{
    egui_region_t draw_region = *region;

    if (font == NULL || !egui_view_label_control_has_text(text) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, align, color, egui_color_alpha_mix(self->alpha, alpha));
}

static void egui_view_label_control_draw_access_underline(egui_view_t *self, egui_view_label_control_t *local, const egui_region_t *text_region,
                                                          const char *draw_text, egui_color_t color)
{
    egui_dim_t underline_width = 5;
    egui_dim_t underline_x;
    egui_dim_t underline_y;
    uint8_t draw_length;

    if (local->access_key_index == EGUI_VIEW_LABEL_CONTROL_ACCESS_NONE || !egui_view_label_control_has_text(draw_text))
    {
        return;
    }

    draw_length = egui_view_label_control_text_len(draw_text);
    if (local->access_key_index >= draw_length || text_region->size.width <= 0)
    {
        return;
    }

    underline_x = text_region->location.x + (egui_dim_t)local->access_key_index * underline_width;
    if (underline_x + underline_width > text_region->location.x + text_region->size.width)
    {
        return;
    }
    underline_y = text_region->location.y + text_region->size.height - 4;
    egui_canvas_draw_rectangle_fill(&uicode_get_core()->canvas, underline_x, underline_y, underline_width, 1, color,
                                    egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(82)));
}

static void egui_view_label_control_on_draw(egui_view_t *self)
{
    egui_view_label_control_t *local = egui_view_label_control_local(self);
    egui_region_t region;
    egui_region_t text_region;
    egui_region_t hint_region;
    egui_color_t surface_color = local->surface_color;
    egui_color_t border_color = local->border_color;
    egui_color_t text_color = local->text_color;
    egui_color_t hint_color = local->hint_color;
    egui_color_t accent_color = local->accent_color;
    egui_color_t required_color = local->required_color;
    egui_dim_t radius = EGUI_VIEW_LABEL_CONTROL_RADIUS;
    char text[EGUI_VIEW_LABEL_CONTROL_MAX_TEXT_LEN + 1];
    char hint[EGUI_VIEW_LABEL_CONTROL_MAX_HINT_LEN + 1];

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0)
    {
        return;
    }

    if (local->target_highlighted)
    {
        surface_color = egui_rgb_mix(surface_color, accent_color, EGUI_ALPHA_MAKE(14));
        border_color = egui_rgb_mix(border_color, accent_color, EGUI_ALPHA_MAKE(42));
    }
    if (!egui_view_get_enable(self))
    {
        surface_color = egui_view_label_control_mix_disabled(surface_color);
        border_color = egui_view_label_control_mix_disabled(border_color);
        text_color = egui_view_label_control_mix_disabled(text_color);
        hint_color = egui_view_label_control_mix_disabled(hint_color);
        accent_color = egui_view_label_control_mix_disabled(accent_color);
        required_color = egui_view_label_control_mix_disabled(required_color);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height, radius,
                                          surface_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->target_highlighted ? 98 : 92)));
    hcw_selection_marker_draw_left(&region, radius, radius, accent_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(72)));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height, radius, 1,
                                     border_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->target_highlighted ? 78 : 64)));

    egui_view_label_control_get_text_regions(self, local, &text_region, &hint_region);
    egui_view_label_control_fit_text_to_width(local->text_font, local->text, text, sizeof(text), text_region.size.width, 6);
    egui_view_label_control_draw_text(local->text_font, self, text, &text_region, local->align_type, text_color, EGUI_ALPHA_100);
    egui_view_label_control_draw_access_underline(self, local, &text_region, text, accent_color);

    if (egui_view_label_control_has_text(local->target_hint))
    {
        egui_view_label_control_fit_text_to_width(local->hint_font, local->target_hint, hint, sizeof(hint), hint_region.size.width, 5);
        egui_view_label_control_draw_text(local->hint_font, self, hint, &hint_region, local->align_type, hint_color, EGUI_ALPHA_100);
    }

    if (local->required)
    {
        egui_canvas_draw_circle_fill_basic(&uicode_get_core()->canvas, region.location.x + region.size.width - 9, region.location.y + 10,
                                           EGUI_VIEW_LABEL_CONTROL_REQUIRED_RADIUS, required_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(92)));
    }
}

void egui_view_label_control_set_text(egui_view_t *self, const char *text)
{
    egui_view_label_control_t *local = egui_view_label_control_local(self);

    egui_view_label_control_clear_pressed_state(self);
    egui_view_label_control_copy_text(local->text, sizeof(local->text), text);
    egui_view_label_control_clamp_access_key(local);
    egui_view_invalidate(self);
}

const char *egui_view_label_control_get_text(egui_view_t *self)
{
    egui_view_label_control_t *local = egui_view_label_control_local(self);

    return local->text;
}

void egui_view_label_control_set_target_hint(egui_view_t *self, const char *hint)
{
    egui_view_label_control_t *local = egui_view_label_control_local(self);

    egui_view_label_control_clear_pressed_state(self);
    egui_view_label_control_copy_text(local->target_hint, sizeof(local->target_hint), hint);
    egui_view_invalidate(self);
}

const char *egui_view_label_control_get_target_hint(egui_view_t *self)
{
    egui_view_label_control_t *local = egui_view_label_control_local(self);

    return local->target_hint;
}

void egui_view_label_control_set_fonts(egui_view_t *self, const egui_font_t *text_font, const egui_font_t *hint_font)
{
    egui_view_label_control_t *local = egui_view_label_control_local(self);

    egui_view_label_control_clear_pressed_state(self);
    local->text_font = text_font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : text_font;
    local->hint_font = hint_font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : hint_font;
    egui_view_invalidate(self);
}

void egui_view_label_control_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color,
                                         egui_color_t text_color, egui_color_t hint_color, egui_color_t accent_color,
                                         egui_color_t required_color)
{
    egui_view_label_control_t *local = egui_view_label_control_local(self);

    egui_view_label_control_clear_pressed_state(self);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->hint_color = hint_color;
    local->accent_color = accent_color;
    local->required_color = required_color;
    egui_view_invalidate(self);
}

void egui_view_label_control_set_align_type(egui_view_t *self, uint8_t align_type)
{
    egui_view_label_control_t *local = egui_view_label_control_local(self);

    egui_view_label_control_clear_pressed_state(self);
    local->align_type = align_type;
    egui_view_invalidate(self);
}

uint8_t egui_view_label_control_get_align_type(egui_view_t *self)
{
    egui_view_label_control_t *local = egui_view_label_control_local(self);

    return local->align_type;
}

void egui_view_label_control_set_access_key_index(egui_view_t *self, uint8_t access_key_index)
{
    egui_view_label_control_t *local = egui_view_label_control_local(self);

    egui_view_label_control_clear_pressed_state(self);
    local->access_key_index = access_key_index;
    egui_view_label_control_clamp_access_key(local);
    egui_view_invalidate(self);
}

uint8_t egui_view_label_control_get_access_key_index(egui_view_t *self)
{
    egui_view_label_control_t *local = egui_view_label_control_local(self);

    return local->access_key_index;
}

void egui_view_label_control_set_required(egui_view_t *self, uint8_t required)
{
    egui_view_label_control_t *local = egui_view_label_control_local(self);

    egui_view_label_control_clear_pressed_state(self);
    local->required = required ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_label_control_get_required(egui_view_t *self)
{
    egui_view_label_control_t *local = egui_view_label_control_local(self);

    return local->required;
}

void egui_view_label_control_set_target_highlighted(egui_view_t *self, uint8_t target_highlighted)
{
    egui_view_label_control_t *local = egui_view_label_control_local(self);

    egui_view_label_control_clear_pressed_state(self);
    local->target_highlighted = target_highlighted ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_label_control_get_target_highlighted(egui_view_t *self)
{
    egui_view_label_control_t *local = egui_view_label_control_local(self);

    return local->target_highlighted;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_label_control_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_label_control_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_label_control_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_label_control_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_label_control_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_label_control_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_label_control_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_label_control_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_label_control_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_label_control_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_label_control_on_static_key_event;
#endif
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_label_control_t) = {
        .draw = egui_view_draw,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_label_control_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .on_draw = egui_view_label_control_on_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_label_control_on_key_event,
#endif
};

void egui_view_label_control_init(egui_view_t *self)
{
    egui_view_label_control_t *local = egui_view_label_control_local(self);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_label_control_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->text_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->hint_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->access_key_index = EGUI_VIEW_LABEL_CONTROL_ACCESS_NONE;
    local->required = 0;
    local->target_highlighted = 0;
    local->surface_color = HCW_COLOR_SURFACE;
    local->border_color = HCW_COLOR_BORDER;
    local->text_color = HCW_COLOR_TEXT;
    local->hint_color = HCW_COLOR_TEXT_MUTED;
    local->accent_color = HCW_COLOR_PRIMARY;
    local->required_color = HCW_COLOR_DANGER_DARK;
    local->align_type = EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER;
    egui_view_label_control_copy_text(local->text, sizeof(local->text), "Label");
    egui_view_label_control_copy_text(local->target_hint, sizeof(local->target_hint), "Target");
    egui_view_set_view_name(self, "egui_view_label_control");
}
