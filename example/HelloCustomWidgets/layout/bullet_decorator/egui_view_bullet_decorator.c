#include "egui_view_bullet_decorator.h"

#include <string.h>

#define EGUI_VIEW_BULLET_DECORATOR_RADIUS            8
#define EGUI_VIEW_BULLET_DECORATOR_COMPACT_RADIUS    6
#define EGUI_VIEW_BULLET_DECORATOR_SLOT_MIN          12
#define EGUI_VIEW_BULLET_DECORATOR_SLOT_MAX          44
#define EGUI_VIEW_BULLET_DECORATOR_GAP_MAX           18
#define EGUI_VIEW_BULLET_DECORATOR_BULLET_SIZE_MIN   3
#define EGUI_VIEW_BULLET_DECORATOR_BULLET_SIZE_MAX   14

static egui_view_bullet_decorator_t *egui_view_bullet_decorator_local(egui_view_t *self)
{
    return (egui_view_bullet_decorator_t *)self;
}

static uint8_t egui_view_bullet_decorator_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static uint8_t egui_view_bullet_decorator_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static uint8_t egui_view_bullet_decorator_text_len(const char *text)
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

static void egui_view_bullet_decorator_copy_text(char *dst, uint8_t capacity, const char *src)
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

static egui_dim_t egui_view_bullet_decorator_clamp_slot(egui_dim_t value)
{
    if (value < EGUI_VIEW_BULLET_DECORATOR_SLOT_MIN)
    {
        return EGUI_VIEW_BULLET_DECORATOR_SLOT_MIN;
    }
    if (value > EGUI_VIEW_BULLET_DECORATOR_SLOT_MAX)
    {
        return EGUI_VIEW_BULLET_DECORATOR_SLOT_MAX;
    }
    return value;
}

static egui_dim_t egui_view_bullet_decorator_clamp_gap(egui_dim_t value)
{
    if (value < 0)
    {
        return 0;
    }
    if (value > EGUI_VIEW_BULLET_DECORATOR_GAP_MAX)
    {
        return EGUI_VIEW_BULLET_DECORATOR_GAP_MAX;
    }
    return value;
}

static egui_dim_t egui_view_bullet_decorator_clamp_bullet_size(egui_dim_t value)
{
    if (value < EGUI_VIEW_BULLET_DECORATOR_BULLET_SIZE_MIN)
    {
        return EGUI_VIEW_BULLET_DECORATOR_BULLET_SIZE_MIN;
    }
    if (value > EGUI_VIEW_BULLET_DECORATOR_BULLET_SIZE_MAX)
    {
        return EGUI_VIEW_BULLET_DECORATOR_BULLET_SIZE_MAX;
    }
    return value;
}

static uint8_t egui_view_bullet_decorator_clamp_bullet_kind(uint8_t bullet_kind)
{
    if (bullet_kind > EGUI_VIEW_BULLET_DECORATOR_BULLET_TEXT)
    {
        return EGUI_VIEW_BULLET_DECORATOR_BULLET_DOT;
    }
    return bullet_kind;
}

static void egui_view_bullet_decorator_copy_elided(char *buffer, uint8_t buffer_size, const char *text, uint8_t max_chars)
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

    length = egui_view_bullet_decorator_text_len(text);
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

static egui_dim_t egui_view_bullet_decorator_measure_text_width(const egui_font_t *font, const char *text)
{
    egui_dim_t width = 0;
    egui_dim_t height = 0;

    if (!egui_view_bullet_decorator_has_text(text) || font == NULL || font->api == NULL || font->api->get_str_size == NULL)
    {
        return 0;
    }
    font->api->get_str_size(font, text, 0, 0, &width, &height);
    return width;
}

static void egui_view_bullet_decorator_fit_text_to_width(const egui_font_t *font, const char *text, char *buffer, uint8_t buffer_size,
                                                         egui_dim_t max_width, egui_dim_t fallback_char_width)
{
    uint8_t max_chars;

    if (buffer == NULL || buffer_size == 0)
    {
        return;
    }
    buffer[0] = '\0';
    if (!egui_view_bullet_decorator_has_text(text) || max_width <= 0)
    {
        return;
    }

    max_chars = egui_view_bullet_decorator_text_len(text);
    egui_view_bullet_decorator_copy_elided(buffer, buffer_size, text, max_chars);
    while (max_chars > 0)
    {
        egui_dim_t width = egui_view_bullet_decorator_measure_text_width(font, buffer);

        if (width <= 0)
        {
            width = (egui_dim_t)egui_view_bullet_decorator_text_len(buffer) * fallback_char_width;
        }
        if (width <= max_width)
        {
            break;
        }
        max_chars--;
        egui_view_bullet_decorator_copy_elided(buffer, buffer_size, text, max_chars);
    }
}

static egui_color_t egui_view_bullet_decorator_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x8A97A5), 58);
}

static void egui_view_bullet_decorator_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region,
                                                uint8_t align, egui_color_t color, egui_alpha_t alpha)
{
    egui_region_t draw_region = *region;

    if (font == NULL || !egui_view_bullet_decorator_has_text(text) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, align, color, egui_color_alpha_mix(self->alpha, alpha));
}

void egui_view_bullet_decorator_get_regions(egui_view_t *self, egui_region_t *bullet_region, egui_region_t *content_region)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);
    egui_region_t work_region;
    egui_dim_t slot_width = local->bullet_slot_width;
    egui_dim_t gap = local->bullet_gap;

    egui_view_get_work_region(self, &work_region);
    if (work_region.size.width < 0)
    {
        work_region.size.width = 0;
    }
    if (work_region.size.height < 0)
    {
        work_region.size.height = 0;
    }
    if (slot_width > work_region.size.width)
    {
        slot_width = work_region.size.width;
    }
    if (slot_width + gap > work_region.size.width)
    {
        gap = work_region.size.width - slot_width;
        if (gap < 0)
        {
            gap = 0;
        }
    }

    if (bullet_region != NULL)
    {
        bullet_region->location.x = work_region.location.x;
        bullet_region->location.y = work_region.location.y;
        bullet_region->size.width = slot_width;
        bullet_region->size.height = work_region.size.height;
    }
    if (content_region != NULL)
    {
        content_region->location.x = work_region.location.x + slot_width + gap;
        content_region->location.y = work_region.location.y;
        content_region->size.width = work_region.size.width - slot_width - gap;
        content_region->size.height = work_region.size.height;
        if (content_region->size.width < 0)
        {
            content_region->size.width = 0;
        }
    }
}

static void egui_view_bullet_decorator_draw_bullet(egui_view_t *self, egui_view_bullet_decorator_t *local, const egui_region_t *bullet_region,
                                                   egui_color_t bullet_color, egui_alpha_t alpha)
{
    egui_dim_t center_x;
    egui_dim_t center_y;
    egui_dim_t size = local->bullet_size;

    if (bullet_region->size.width <= 0 || bullet_region->size.height <= 0)
    {
        return;
    }

    center_x = bullet_region->location.x + bullet_region->size.width / 2;
    center_y = bullet_region->location.y + bullet_region->size.height / 2;

    if (local->bullet_kind == EGUI_VIEW_BULLET_DECORATOR_BULLET_TEXT)
    {
        egui_view_bullet_decorator_draw_text(local->bullet_font, self, local->bullet_text, bullet_region, EGUI_ALIGN_CENTER, bullet_color, alpha);
        return;
    }

    if (size > bullet_region->size.height - 4)
    {
        size = bullet_region->size.height - 4;
    }
    if (size < EGUI_VIEW_BULLET_DECORATOR_BULLET_SIZE_MIN)
    {
        size = EGUI_VIEW_BULLET_DECORATOR_BULLET_SIZE_MIN;
    }

    if (local->bullet_kind == EGUI_VIEW_BULLET_DECORATOR_BULLET_SQUARE)
    {
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, center_x - size / 2, center_y - size / 2, size, size,
                                              local->compact_mode ? 1 : 2, bullet_color, egui_color_alpha_mix(self->alpha, alpha));
        return;
    }

    egui_canvas_draw_circle_fill_basic(&uicode_get_core()->canvas, center_x, center_y, size / 2, bullet_color,
                                       egui_color_alpha_mix(self->alpha, alpha));
}

static void egui_view_bullet_decorator_on_draw(egui_view_t *self)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);
    egui_region_t region;
    egui_region_t bullet_region;
    egui_region_t content_region;
    egui_color_t surface_color = local->surface_color;
    egui_color_t border_color = local->border_color;
    egui_color_t bullet_color = local->bullet_color;
    egui_color_t text_color = local->text_color;
    egui_color_t accent_color = local->accent_color;
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_BULLET_DECORATOR_COMPACT_RADIUS : EGUI_VIEW_BULLET_DECORATOR_RADIUS;
    egui_alpha_t bullet_alpha = local->compact_mode ? 82 : 92;
    egui_alpha_t text_alpha = EGUI_ALPHA_100;
    char text[EGUI_VIEW_BULLET_DECORATOR_MAX_TEXT_LEN + 1];

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0)
    {
        return;
    }

    if (local->read_only_mode)
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xF5F7FA), 50);
        border_color = egui_rgb_mix(border_color, EGUI_COLOR_HEX(0xAEB8C2), 52);
        bullet_color = egui_rgb_mix(bullet_color, EGUI_COLOR_HEX(0x8A97A5), 50);
        text_color = egui_rgb_mix(text_color, EGUI_COLOR_HEX(0x8A97A5), 42);
        accent_color = egui_rgb_mix(accent_color, EGUI_COLOR_HEX(0x8A97A5), 56);
        bullet_alpha = 62;
        text_alpha = 88;
    }
    if (!egui_view_get_enable(self))
    {
        surface_color = egui_view_bullet_decorator_mix_disabled(surface_color);
        border_color = egui_view_bullet_decorator_mix_disabled(border_color);
        bullet_color = egui_view_bullet_decorator_mix_disabled(bullet_color);
        text_color = egui_view_bullet_decorator_mix_disabled(text_color);
        accent_color = egui_view_bullet_decorator_mix_disabled(accent_color);
        bullet_alpha = 50;
        text_alpha = 72;
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height, radius,
                                          surface_color, egui_color_alpha_mix(self->alpha, local->compact_mode ? 74 : 92));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height, radius, 1,
                                     border_color, egui_color_alpha_mix(self->alpha, local->compact_mode ? 30 : 42));

    egui_view_bullet_decorator_get_regions(self, &bullet_region, &content_region);
    if (bullet_region.size.width > 0)
    {
        egui_canvas_draw_rectangle_fill(&uicode_get_core()->canvas, bullet_region.location.x + bullet_region.size.width - 1,
                                        bullet_region.location.y + (local->compact_mode ? 5 : 7), 1,
                                        bullet_region.size.height - (local->compact_mode ? 10 : 14), accent_color,
                                        egui_color_alpha_mix(self->alpha, local->read_only_mode ? 16 : 32));
        egui_view_bullet_decorator_draw_bullet(self, local, &bullet_region, bullet_color, bullet_alpha);
    }

    egui_view_bullet_decorator_fit_text_to_width(local->text_font, local->content_text, text, sizeof(text), content_region.size.width,
                                                 local->compact_mode ? 5 : 6);
    egui_view_bullet_decorator_draw_text(local->text_font, self, text, &content_region, local->content_align_type, text_color, text_alpha);
}

void egui_view_bullet_decorator_set_content_text(egui_view_t *self, const char *text)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);

    egui_view_bullet_decorator_clear_pressed_state(self);
    egui_view_bullet_decorator_copy_text(local->content_text, sizeof(local->content_text), text);
    egui_view_invalidate(self);
}

const char *egui_view_bullet_decorator_get_content_text(egui_view_t *self)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);

    return local->content_text;
}

void egui_view_bullet_decorator_set_bullet_text(egui_view_t *self, const char *text)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);

    egui_view_bullet_decorator_clear_pressed_state(self);
    egui_view_bullet_decorator_copy_text(local->bullet_text, sizeof(local->bullet_text), text);
    egui_view_invalidate(self);
}

const char *egui_view_bullet_decorator_get_bullet_text(egui_view_t *self)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);

    return local->bullet_text;
}

void egui_view_bullet_decorator_set_bullet_kind(egui_view_t *self, uint8_t bullet_kind)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);

    egui_view_bullet_decorator_clear_pressed_state(self);
    local->bullet_kind = egui_view_bullet_decorator_clamp_bullet_kind(bullet_kind);
    egui_view_invalidate(self);
}

uint8_t egui_view_bullet_decorator_get_bullet_kind(egui_view_t *self)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);

    return local->bullet_kind;
}

void egui_view_bullet_decorator_set_fonts(egui_view_t *self, const egui_font_t *text_font, const egui_font_t *bullet_font)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);

    egui_view_bullet_decorator_clear_pressed_state(self);
    local->text_font = text_font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : text_font;
    local->bullet_font = bullet_font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : bullet_font;
    egui_view_invalidate(self);
}

void egui_view_bullet_decorator_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color,
                                            egui_color_t bullet_color, egui_color_t text_color, egui_color_t accent_color)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);

    egui_view_bullet_decorator_clear_pressed_state(self);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->bullet_color = bullet_color;
    local->text_color = text_color;
    local->accent_color = accent_color;
    egui_view_invalidate(self);
}

void egui_view_bullet_decorator_set_metrics(egui_view_t *self, egui_dim_t bullet_slot_width, egui_dim_t bullet_gap, egui_dim_t bullet_size)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);

    egui_view_bullet_decorator_clear_pressed_state(self);
    local->bullet_slot_width = egui_view_bullet_decorator_clamp_slot(bullet_slot_width);
    local->bullet_gap = egui_view_bullet_decorator_clamp_gap(bullet_gap);
    local->bullet_size = egui_view_bullet_decorator_clamp_bullet_size(bullet_size);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_bullet_decorator_get_bullet_slot_width(egui_view_t *self)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);

    return local->bullet_slot_width;
}

egui_dim_t egui_view_bullet_decorator_get_bullet_gap(egui_view_t *self)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);

    return local->bullet_gap;
}

egui_dim_t egui_view_bullet_decorator_get_bullet_size(egui_view_t *self)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);

    return local->bullet_size;
}

void egui_view_bullet_decorator_set_content_align_type(egui_view_t *self, uint8_t align_type)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);

    egui_view_bullet_decorator_clear_pressed_state(self);
    local->content_align_type = align_type;
    egui_view_invalidate(self);
}

uint8_t egui_view_bullet_decorator_get_content_align_type(egui_view_t *self)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);

    return local->content_align_type;
}

void egui_view_bullet_decorator_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);

    egui_view_bullet_decorator_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_bullet_decorator_get_compact_mode(egui_view_t *self)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);

    return local->compact_mode;
}

void egui_view_bullet_decorator_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);

    egui_view_bullet_decorator_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_bullet_decorator_get_read_only_mode(egui_view_t *self)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);

    return local->read_only_mode;
}

void egui_view_bullet_decorator_apply_standard_style(egui_view_t *self)
{
    egui_view_bullet_decorator_set_palette(self, EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DEE8), EGUI_COLOR_HEX(0x0F6CBD),
                                           EGUI_COLOR_HEX(0x1D2A36), EGUI_COLOR_HEX(0xD7E3EE));
    egui_view_bullet_decorator_set_metrics(self, 24, 6, 8);
    egui_view_bullet_decorator_set_content_align_type(self, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_bullet_decorator_set_compact_mode(self, 0);
    egui_view_bullet_decorator_set_read_only_mode(self, 0);
}

void egui_view_bullet_decorator_apply_accent_style(egui_view_t *self)
{
    egui_view_bullet_decorator_set_palette(self, EGUI_COLOR_HEX(0xF7FBFF), EGUI_COLOR_HEX(0xB9D6F0), EGUI_COLOR_HEX(0x0F6CBD),
                                           EGUI_COLOR_HEX(0x173247), EGUI_COLOR_HEX(0xCFE2F3));
    egui_view_bullet_decorator_set_metrics(self, 26, 6, 9);
    egui_view_bullet_decorator_set_content_align_type(self, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_bullet_decorator_set_compact_mode(self, 0);
    egui_view_bullet_decorator_set_read_only_mode(self, 0);
}

void egui_view_bullet_decorator_apply_compact_style(egui_view_t *self)
{
    egui_view_bullet_decorator_set_palette(self, EGUI_COLOR_HEX(0xF8FBFD), EGUI_COLOR_HEX(0xD2DCE6), EGUI_COLOR_HEX(0x0C7C73),
                                           EGUI_COLOR_HEX(0x22313C), EGUI_COLOR_HEX(0xD9E7E5));
    egui_view_bullet_decorator_set_metrics(self, 20, 4, 6);
    egui_view_bullet_decorator_set_content_align_type(self, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_bullet_decorator_set_compact_mode(self, 1);
    egui_view_bullet_decorator_set_read_only_mode(self, 0);
}

void egui_view_bullet_decorator_apply_read_only_style(egui_view_t *self)
{
    egui_view_bullet_decorator_set_palette(self, EGUI_COLOR_HEX(0xF5F7FA), EGUI_COLOR_HEX(0xD7DEE6), EGUI_COLOR_HEX(0x788593),
                                           EGUI_COLOR_HEX(0x687684), EGUI_COLOR_HEX(0xE1E6EB));
    egui_view_bullet_decorator_set_metrics(self, 20, 4, 6);
    egui_view_bullet_decorator_set_content_align_type(self, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_bullet_decorator_set_compact_mode(self, 1);
    egui_view_bullet_decorator_set_read_only_mode(self, 1);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_bullet_decorator_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_bullet_decorator_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_bullet_decorator_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_bullet_decorator_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_bullet_decorator_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_bullet_decorator_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_bullet_decorator_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_bullet_decorator_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_bullet_decorator_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_bullet_decorator_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_bullet_decorator_on_static_key_event;
#endif
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_bullet_decorator_t) = {
        .draw = egui_view_draw,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_bullet_decorator_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .on_draw = egui_view_bullet_decorator_on_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_bullet_decorator_on_key_event,
#endif
};

void egui_view_bullet_decorator_init(egui_view_t *self)
{
    egui_view_bullet_decorator_t *local = egui_view_bullet_decorator_local(self);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_bullet_decorator_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->text_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->bullet_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->bullet_kind = EGUI_VIEW_BULLET_DECORATOR_BULLET_DOT;
    local->content_align_type = EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    egui_view_bullet_decorator_copy_text(local->content_text, sizeof(local->content_text), "Decorated content");
    egui_view_bullet_decorator_copy_text(local->bullet_text, sizeof(local->bullet_text), "1.");
    egui_view_bullet_decorator_apply_standard_style(self);
    egui_view_set_view_name(self, "egui_view_bullet_decorator");
}
