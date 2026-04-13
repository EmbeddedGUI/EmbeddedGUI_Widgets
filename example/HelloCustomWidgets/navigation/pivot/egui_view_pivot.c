#include "egui_view_pivot.h"

#include <string.h>

#define HCW_PIVOT_STANDARD_BASE_WIDTH        18
#define HCW_PIVOT_STANDARD_CHAR_WIDTH        5
#define HCW_PIVOT_STANDARD_ACTIVE_BONUS      10
#define HCW_PIVOT_STANDARD_MIN_WIDTH         34
#define HCW_PIVOT_STANDARD_MAX_WIDTH         66
#define HCW_PIVOT_STANDARD_GAP               9
#define HCW_PIVOT_STANDARD_PAD_X             10
#define HCW_PIVOT_STANDARD_HEADER_Y          8
#define HCW_PIVOT_STANDARD_HEADER_HEIGHT     14
#define HCW_PIVOT_STANDARD_DIVIDER_Y         26
#define HCW_PIVOT_STANDARD_BODY_Y            34
#define HCW_PIVOT_STANDARD_BODY_PAD_X        10
#define HCW_PIVOT_STANDARD_BODY_PAD_Y        8
#define HCW_PIVOT_STANDARD_FILL_ALPHA        92
#define HCW_PIVOT_STANDARD_BORDER_ALPHA      60
#define HCW_PIVOT_STANDARD_ACTIVE_FILL_ALPHA 22

#define HCW_PIVOT_COMPACT_BASE_WIDTH        12
#define HCW_PIVOT_COMPACT_CHAR_WIDTH        4
#define HCW_PIVOT_COMPACT_ACTIVE_BONUS      7
#define HCW_PIVOT_COMPACT_MIN_WIDTH         24
#define HCW_PIVOT_COMPACT_MAX_WIDTH         42
#define HCW_PIVOT_COMPACT_GAP               5
#define HCW_PIVOT_COMPACT_PAD_X             8
#define HCW_PIVOT_COMPACT_HEADER_Y          5
#define HCW_PIVOT_COMPACT_HEADER_HEIGHT     11
#define HCW_PIVOT_COMPACT_DIVIDER_Y         18
#define HCW_PIVOT_COMPACT_BODY_Y            24
#define HCW_PIVOT_COMPACT_BODY_PAD_X        8
#define HCW_PIVOT_COMPACT_BODY_PAD_Y        6
#define HCW_PIVOT_COMPACT_FILL_ALPHA        90
#define HCW_PIVOT_COMPACT_BORDER_ALPHA      56
#define HCW_PIVOT_COMPACT_ACTIVE_FILL_ALPHA 18

typedef struct hcw_pivot_layout_item hcw_pivot_layout_item_t;
struct hcw_pivot_layout_item
{
    egui_region_t region;
    char label[16];
};

static uint8_t hcw_pivot_clamp_count(uint8_t count)
{
    if (count > HCW_PIVOT_MAX_ITEMS)
    {
        return HCW_PIVOT_MAX_ITEMS;
    }
    return count;
}

static egui_color_t hcw_pivot_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x83909D), 48);
}

static uint8_t hcw_pivot_clear_pressed_state(egui_view_t *self)
{
    EGUI_LOCAL_INIT(hcw_pivot_t);
    uint8_t was_pressed = egui_view_get_pressed(self) ? 1 : 0;
    uint8_t had_pressed = (uint8_t)(was_pressed || local->pressed_index != HCW_PIVOT_INDEX_NONE);

    if (!had_pressed)
    {
        return 0;
    }

    local->pressed_index = HCW_PIVOT_INDEX_NONE;
    if (was_pressed)
    {
        egui_view_set_pressed(self, 0);
    }
    else
    {
        egui_view_invalidate(self);
    }
    return 1;
}

static uint8_t hcw_pivot_text_len(const char *text)
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

static void hcw_pivot_copy_elided(char *buffer, uint8_t capacity, const char *text, uint8_t max_chars)
{
    uint8_t length = 0;
    uint8_t copy_length;
    uint8_t index;

    if (buffer == NULL || capacity == 0)
    {
        return;
    }

    buffer[0] = '\0';
    if (text == NULL || max_chars == 0)
    {
        return;
    }

    while (text[length] != '\0')
    {
        length++;
    }

    if (length <= max_chars)
    {
        copy_length = length;
        if (copy_length >= capacity)
        {
            copy_length = capacity - 1;
        }
        for (index = 0; index < copy_length; index++)
        {
            buffer[index] = text[index];
        }
        buffer[copy_length] = '\0';
        return;
    }

    if (max_chars <= 3)
    {
        copy_length = max_chars;
        if (copy_length >= capacity)
        {
            copy_length = capacity - 1;
        }
        for (index = 0; index < copy_length; index++)
        {
            buffer[index] = '.';
        }
        buffer[copy_length] = '\0';
        return;
    }

    copy_length = max_chars - 3;
    if (copy_length > capacity - 4)
    {
        copy_length = capacity - 4;
    }
    for (index = 0; index < copy_length; index++)
    {
        buffer[index] = text[index];
    }
    buffer[copy_length + 0] = '.';
    buffer[copy_length + 1] = '.';
    buffer[copy_length + 2] = '.';
    buffer[copy_length + 3] = '\0';
}

static egui_dim_t hcw_pivot_header_width(uint8_t compact_mode, uint8_t is_active, const char *text)
{
    egui_dim_t width;
    uint8_t length = hcw_pivot_text_len(text);

    width = compact_mode ? HCW_PIVOT_COMPACT_BASE_WIDTH : HCW_PIVOT_STANDARD_BASE_WIDTH;
    width += (egui_dim_t)length * (compact_mode ? HCW_PIVOT_COMPACT_CHAR_WIDTH : HCW_PIVOT_STANDARD_CHAR_WIDTH);
    if (is_active)
    {
        width += compact_mode ? HCW_PIVOT_COMPACT_ACTIVE_BONUS : HCW_PIVOT_STANDARD_ACTIVE_BONUS;
    }

    if (compact_mode)
    {
        if (width < HCW_PIVOT_COMPACT_MIN_WIDTH)
        {
            width = HCW_PIVOT_COMPACT_MIN_WIDTH;
        }
        if (width > HCW_PIVOT_COMPACT_MAX_WIDTH)
        {
            width = HCW_PIVOT_COMPACT_MAX_WIDTH;
        }
    }
    else
    {
        if (width < HCW_PIVOT_STANDARD_MIN_WIDTH)
        {
            width = HCW_PIVOT_STANDARD_MIN_WIDTH;
        }
        if (width > HCW_PIVOT_STANDARD_MAX_WIDTH)
        {
            width = HCW_PIVOT_STANDARD_MAX_WIDTH;
        }
    }
    return width;
}

static egui_dim_t hcw_pivot_pad_x(uint8_t compact_mode)
{
    return compact_mode ? HCW_PIVOT_COMPACT_PAD_X : HCW_PIVOT_STANDARD_PAD_X;
}

static egui_dim_t hcw_pivot_gap(uint8_t compact_mode)
{
    return compact_mode ? HCW_PIVOT_COMPACT_GAP : HCW_PIVOT_STANDARD_GAP;
}

static egui_dim_t hcw_pivot_header_y(uint8_t compact_mode)
{
    return compact_mode ? HCW_PIVOT_COMPACT_HEADER_Y : HCW_PIVOT_STANDARD_HEADER_Y;
}

static egui_dim_t hcw_pivot_header_height(uint8_t compact_mode)
{
    return compact_mode ? HCW_PIVOT_COMPACT_HEADER_HEIGHT : HCW_PIVOT_STANDARD_HEADER_HEIGHT;
}

static egui_dim_t hcw_pivot_divider_y(uint8_t compact_mode)
{
    return compact_mode ? HCW_PIVOT_COMPACT_DIVIDER_Y : HCW_PIVOT_STANDARD_DIVIDER_Y;
}

static egui_dim_t hcw_pivot_body_y(uint8_t compact_mode)
{
    return compact_mode ? HCW_PIVOT_COMPACT_BODY_Y : HCW_PIVOT_STANDARD_BODY_Y;
}

static egui_dim_t hcw_pivot_body_pad_x(uint8_t compact_mode)
{
    return compact_mode ? HCW_PIVOT_COMPACT_BODY_PAD_X : HCW_PIVOT_STANDARD_BODY_PAD_X;
}

static egui_dim_t hcw_pivot_body_pad_y(uint8_t compact_mode)
{
    return compact_mode ? HCW_PIVOT_COMPACT_BODY_PAD_Y : HCW_PIVOT_STANDARD_BODY_PAD_Y;
}

static const egui_font_t *hcw_pivot_get_font(const hcw_pivot_t *local)
{
    return local->font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : local->font;
}

static const egui_font_t *hcw_pivot_get_meta_font(const hcw_pivot_t *local)
{
    return local->meta_font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : local->meta_font;
}

static const hcw_pivot_item_t *hcw_pivot_get_current_item(const hcw_pivot_t *local)
{
    if (local->items == NULL || local->item_count == 0 || local->current_index >= local->item_count)
    {
        return NULL;
    }
    return &local->items[local->current_index];
}

static egui_color_t hcw_pivot_tone_accent(const hcw_pivot_t *local, uint8_t tone)
{
    switch (tone)
    {
    case HCW_PIVOT_TONE_WARM:
        return EGUI_COLOR_HEX(0xA15C00);
    case HCW_PIVOT_TONE_SUCCESS:
        return EGUI_COLOR_HEX(0x0F7B45);
    case HCW_PIVOT_TONE_NEUTRAL:
        return egui_rgb_mix(local->accent_color, local->text_color, 8);
    case HCW_PIVOT_TONE_ACCENT:
    default:
        return local->accent_color;
    }
}

static egui_color_t hcw_pivot_tone_fill(const hcw_pivot_t *local, uint8_t tone)
{
    switch (tone)
    {
    case HCW_PIVOT_TONE_WARM:
        return EGUI_COLOR_HEX(0xF8EFE3);
    case HCW_PIVOT_TONE_SUCCESS:
        return EGUI_COLOR_HEX(0xEAF6EF);
    case HCW_PIVOT_TONE_ACCENT:
        return EGUI_COLOR_HEX(0xEAF3FC);
    case HCW_PIVOT_TONE_NEUTRAL:
    default:
        return local->card_surface_color;
    }
}

static uint8_t hcw_pivot_prepare_header_layout(hcw_pivot_t *local, egui_view_t *self, hcw_pivot_layout_item_t *items)
{
    egui_region_t region;
    egui_dim_t content_x;
    egui_dim_t content_w;
    egui_dim_t total_width = 0;
    egui_dim_t gap = hcw_pivot_gap(local->compact_mode);
    egui_dim_t header_y;
    egui_dim_t header_height;
    uint8_t count = hcw_pivot_clamp_count(local->item_count);
    uint8_t index;

    if (items == NULL || count == 0 || local->items == NULL)
    {
        return 0;
    }

    egui_view_get_work_region(self, &region);
    content_x = region.location.x + hcw_pivot_pad_x(local->compact_mode);
    content_w = region.size.width - hcw_pivot_pad_x(local->compact_mode) * 2;
    if (content_w <= 0)
    {
        return 0;
    }

    header_y = region.location.y + hcw_pivot_header_y(local->compact_mode);
    header_height = hcw_pivot_header_height(local->compact_mode);

    for (index = 0; index < count; index++)
    {
        const char *header = local->items[index].header;
        uint8_t max_chars = local->compact_mode ? (index == local->current_index ? 6 : 5) : (index == local->current_index ? 10 : 8);

        hcw_pivot_copy_elided(items[index].label, sizeof(items[index].label), header, max_chars);
        items[index].region.size.width = hcw_pivot_header_width(local->compact_mode, index == local->current_index, items[index].label);
        items[index].region.size.height = header_height;
        total_width += items[index].region.size.width;
    }

    if (count > 1)
    {
        total_width += gap * (count - 1);
    }

    if (total_width > content_w)
    {
        egui_dim_t fallback_width = (content_w - gap * (count - 1)) / count;

        if (fallback_width < (local->compact_mode ? 20 : 24))
        {
            fallback_width = local->compact_mode ? 20 : 24;
        }
        for (index = 0; index < count; index++)
        {
            items[index].region.size.width = fallback_width;
        }
        total_width = fallback_width * count + gap * (count - 1);
    }

    if (total_width < content_w)
    {
        content_x += (content_w - total_width) / 2;
    }

    for (index = 0; index < count; index++)
    {
        items[index].region.location.x = content_x;
        items[index].region.location.y = header_y;
        content_x += items[index].region.size.width + gap;
    }

    return count;
}

static uint8_t hcw_pivot_resolve_hit(hcw_pivot_t *local, egui_view_t *self, egui_dim_t screen_x, egui_dim_t screen_y)
{
    hcw_pivot_layout_item_t items[HCW_PIVOT_MAX_ITEMS];
    uint8_t count = hcw_pivot_prepare_header_layout(local, self, items);
    uint8_t index;

    for (index = 0; index < count; index++)
    {
        if (egui_region_pt_in_rect(&items[index].region, screen_x, screen_y))
        {
            return index;
        }
    }
    return HCW_PIVOT_INDEX_NONE;
}

static void hcw_pivot_draw_text(egui_view_t *self, const egui_font_t *font, const char *text, const egui_region_t *region, uint8_t align_type,
                                egui_color_t color)
{
    egui_region_t draw_region;

    if (font == NULL || text == NULL || text[0] == '\0' || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    draw_region = *region;
    egui_canvas_draw_text_in_rect(font, text, &draw_region, align_type, color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
}

static void hcw_pivot_draw_body(egui_view_t *self, hcw_pivot_t *local, egui_color_t text_color, egui_color_t muted_text_color, egui_color_t border_color,
                                uint8_t is_enabled)
{
    const hcw_pivot_item_t *item = hcw_pivot_get_current_item(local);
    egui_region_t work_region;
    egui_region_t body_region;
    egui_region_t eyebrow_region;
    egui_region_t title_region;
    egui_region_t body_text_region;
    egui_region_t meta_region;
    egui_color_t accent_color;
    egui_color_t body_fill;
    egui_dim_t body_y;
    egui_dim_t body_pad_x;
    egui_dim_t body_pad_y;
    egui_dim_t eyebrow_height;
    egui_dim_t title_height;
    egui_dim_t meta_height;

    if (item == NULL)
    {
        return;
    }

    egui_view_get_work_region(self, &work_region);
    body_y = hcw_pivot_body_y(local->compact_mode);
    body_pad_x = hcw_pivot_body_pad_x(local->compact_mode);
    body_pad_y = hcw_pivot_body_pad_y(local->compact_mode);

    body_region.location.x = work_region.location.x + body_pad_x;
    body_region.location.y = work_region.location.y + body_y;
    body_region.size.width = work_region.size.width - body_pad_x * 2;
    body_region.size.height = work_region.size.height - body_y - body_pad_y;
    if (body_region.size.width <= 0 || body_region.size.height <= 0)
    {
        return;
    }

    accent_color = hcw_pivot_tone_accent(local, item->tone);
    body_fill = hcw_pivot_tone_fill(local, item->tone);

    if (local->read_only_mode)
    {
        body_fill = egui_rgb_mix(body_fill, EGUI_COLOR_HEX(0xF7F9FB), 28);
        accent_color = egui_rgb_mix(accent_color, muted_text_color, 42);
    }
    if (!is_enabled)
    {
        body_fill = hcw_pivot_mix_disabled(body_fill);
        accent_color = hcw_pivot_mix_disabled(accent_color);
    }

    egui_canvas_draw_round_rectangle_fill(body_region.location.x, body_region.location.y, body_region.size.width, body_region.size.height,
                                          local->compact_mode ? 8 : 10, body_fill, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
    egui_canvas_draw_round_rectangle(body_region.location.x, body_region.location.y, body_region.size.width, body_region.size.height, local->compact_mode ? 8 : 10,
                                     1, border_color, egui_color_alpha_mix(self->alpha, 42));

    egui_canvas_draw_line(body_region.location.x + 8, body_region.location.y + 6, body_region.location.x + body_region.size.width - 8, body_region.location.y + 6,
                          1, accent_color, egui_color_alpha_mix(self->alpha, local->read_only_mode ? 24 : 46));

    eyebrow_height = local->compact_mode ? 8 : 9;
    title_height = local->compact_mode ? 10 : 13;
    meta_height = local->compact_mode ? 8 : 9;

    eyebrow_region.location.x = body_region.location.x + 8;
    eyebrow_region.location.y = body_region.location.y + (local->compact_mode ? 10 : 12);
    eyebrow_region.size.width = body_region.size.width - 16;
    eyebrow_region.size.height = eyebrow_height;

    title_region.location.x = body_region.location.x + 8;
    title_region.location.y = eyebrow_region.location.y + eyebrow_height + (local->compact_mode ? 2 : 4);
    title_region.size.width = body_region.size.width - 16;
    title_region.size.height = title_height;

    body_text_region.location.x = body_region.location.x + 8;
    body_text_region.location.y = title_region.location.y + title_height + (local->compact_mode ? 2 : 5);
    body_text_region.size.width = body_region.size.width - 16;
    body_text_region.size.height = local->compact_mode ? 10 : 12;

    meta_region.location.x = body_region.location.x + 8;
    meta_region.location.y = body_region.location.y + body_region.size.height - meta_height - (local->compact_mode ? 7 : 8);
    meta_region.size.width = body_region.size.width - 16;
    meta_region.size.height = meta_height;

    hcw_pivot_draw_text(self, hcw_pivot_get_meta_font(local), item->eyebrow, &eyebrow_region, EGUI_ALIGN_CENTER, accent_color);
    hcw_pivot_draw_text(self, hcw_pivot_get_font(local), item->title, &title_region, EGUI_ALIGN_CENTER, text_color);
    if (!local->compact_mode)
    {
        hcw_pivot_draw_text(self, hcw_pivot_get_meta_font(local), item->body, &body_text_region, EGUI_ALIGN_CENTER, muted_text_color);
    }
    hcw_pivot_draw_text(self, hcw_pivot_get_meta_font(local), item->meta, &meta_region, EGUI_ALIGN_CENTER, muted_text_color);
}

static void hcw_pivot_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(hcw_pivot_t);
    egui_region_t region;
    hcw_pivot_layout_item_t items[HCW_PIVOT_MAX_ITEMS];
    egui_color_t surface_color = local->surface_color;
    egui_color_t border_color = local->border_color;
    egui_color_t text_color = local->text_color;
    egui_color_t muted_text_color = local->muted_text_color;
    egui_color_t accent_color = local->accent_color;
    uint8_t count;
    uint8_t is_enabled;
    uint8_t index;
    egui_dim_t divider_y;

    egui_view_get_work_region(self, &region);
    if (egui_region_is_empty(&region) || local->items == NULL || local->item_count == 0)
    {
        return;
    }

    is_enabled = egui_view_get_enable(self) ? 1 : 0;
    if (local->compact_mode)
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xFBFCFD), 18);
    }
    else
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xFFFFFF), 16);
        border_color = egui_rgb_mix(border_color, EGUI_COLOR_HEX(0xFFFFFF), 10);
    }

    if (local->read_only_mode)
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xF7F9FB), 18);
        border_color = egui_rgb_mix(border_color, muted_text_color, 18);
        text_color = egui_rgb_mix(text_color, muted_text_color, 38);
        muted_text_color = egui_rgb_mix(muted_text_color, border_color, 10);
        accent_color = egui_rgb_mix(accent_color, muted_text_color, 54);
    }
    if (!is_enabled)
    {
        surface_color = hcw_pivot_mix_disabled(surface_color);
        border_color = hcw_pivot_mix_disabled(border_color);
        text_color = hcw_pivot_mix_disabled(text_color);
        muted_text_color = hcw_pivot_mix_disabled(muted_text_color);
        accent_color = hcw_pivot_mix_disabled(accent_color);
    }

    egui_canvas_draw_round_rectangle_fill(
            region.location.x, region.location.y, region.size.width, region.size.height, local->compact_mode ? 10 : 12, surface_color,
            egui_color_alpha_mix(self->alpha, local->compact_mode ? HCW_PIVOT_COMPACT_FILL_ALPHA : HCW_PIVOT_STANDARD_FILL_ALPHA));
    egui_canvas_draw_round_rectangle(
            region.location.x, region.location.y, region.size.width, region.size.height, local->compact_mode ? 10 : 12, 1, border_color,
            egui_color_alpha_mix(self->alpha, local->compact_mode ? HCW_PIVOT_COMPACT_BORDER_ALPHA : HCW_PIVOT_STANDARD_BORDER_ALPHA));

    divider_y = region.location.y + hcw_pivot_divider_y(local->compact_mode);
    egui_canvas_draw_line(region.location.x + hcw_pivot_pad_x(local->compact_mode), divider_y, region.location.x + region.size.width - hcw_pivot_pad_x(local->compact_mode),
                          divider_y, 1, border_color, egui_color_alpha_mix(self->alpha, 18));

    count = hcw_pivot_prepare_header_layout(local, self, items);
    for (index = 0; index < count; index++)
    {
        uint8_t is_active = index == local->current_index;
        egui_color_t item_text_color = is_active ? text_color : muted_text_color;

        if (is_active)
        {
            egui_color_t active_fill = egui_rgb_mix(surface_color, accent_color, local->compact_mode ? 3 : 4);

            egui_canvas_draw_round_rectangle_fill(items[index].region.location.x, items[index].region.location.y - 1, items[index].region.size.width,
                                                  items[index].region.size.height + 2, local->compact_mode ? 5 : 6, active_fill,
                                                  egui_color_alpha_mix(self->alpha, local->compact_mode ? HCW_PIVOT_COMPACT_ACTIVE_FILL_ALPHA
                                                                                                        : HCW_PIVOT_STANDARD_ACTIVE_FILL_ALPHA));
            egui_canvas_draw_line(items[index].region.location.x + 6, divider_y, items[index].region.location.x + items[index].region.size.width - 6, divider_y, 1,
                                  accent_color, egui_color_alpha_mix(self->alpha, local->read_only_mode ? 24 : 56));
        }
        if (is_enabled && !local->read_only_mode && local->pressed_index == index)
        {
            egui_canvas_draw_round_rectangle_fill(items[index].region.location.x, items[index].region.location.y - 1, items[index].region.size.width,
                                                  items[index].region.size.height + 2, local->compact_mode ? 5 : 6, EGUI_THEME_PRESS_OVERLAY,
                                                  EGUI_THEME_PRESS_OVERLAY_ALPHA);
        }

        hcw_pivot_draw_text(self, hcw_pivot_get_font(local), items[index].label, &items[index].region, EGUI_ALIGN_CENTER, item_text_color);
    }

    hcw_pivot_draw_body(self, local, text_color, muted_text_color, border_color, is_enabled);

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (self->is_focused && is_enabled)
    {
        egui_dim_t inner_x = region.location.x + 2;
        egui_dim_t inner_y = region.location.y + 2;
        egui_dim_t inner_w = region.size.width - 4;
        egui_dim_t inner_h = region.size.height - 4;
        egui_dim_t radius = local->compact_mode ? 8 : 10;

        egui_canvas_draw_round_rectangle(region.location.x, region.location.y, region.size.width, region.size.height, radius + 2, 2, EGUI_THEME_FOCUS,
                                         egui_color_alpha_mix(self->alpha, 90));
        egui_canvas_draw_round_rectangle(inner_x, inner_y, inner_w, inner_h, radius, 1, EGUI_THEME_FOCUS, egui_color_alpha_mix(self->alpha, 44));
    }
#endif
}

static void hcw_pivot_apply_style(egui_view_t *self, uint8_t compact_mode, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(hcw_pivot_t);

    hcw_pivot_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    local->read_only_mode = read_only_mode ? 1 : 0;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xD4DCE4);
    local->text_color = EGUI_COLOR_HEX(0x1D2732);
    local->muted_text_color = EGUI_COLOR_HEX(0x6A7886);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->card_surface_color = EGUI_COLOR_HEX(0xF7F9FB);
    egui_view_invalidate(self);
}

void hcw_pivot_apply_standard_style(egui_view_t *self)
{
    hcw_pivot_apply_style(self, 0, 0);
}

void hcw_pivot_apply_compact_style(egui_view_t *self)
{
    hcw_pivot_apply_style(self, 1, 0);
}

void hcw_pivot_apply_read_only_style(egui_view_t *self)
{
    hcw_pivot_apply_style(self, 0, 1);
}

void hcw_pivot_set_items(egui_view_t *self, const hcw_pivot_item_t *items, uint8_t item_count)
{
    EGUI_LOCAL_INIT(hcw_pivot_t);
    uint8_t had_pressed = hcw_pivot_clear_pressed_state(self);

    local->items = items;
    local->item_count = items == NULL ? 0 : hcw_pivot_clamp_count(item_count);
    if (local->item_count == 0)
    {
        local->current_index = 0;
    }
    else if (local->current_index >= local->item_count)
    {
        local->current_index = 0;
    }

    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_pivot_set_current_index(egui_view_t *self, uint8_t index)
{
    EGUI_LOCAL_INIT(hcw_pivot_t);
    uint8_t had_pressed;

    if (local->item_count == 0 || index >= local->item_count)
    {
        hcw_pivot_clear_pressed_state(self);
        return;
    }
    if (local->current_index == index)
    {
        hcw_pivot_clear_pressed_state(self);
        return;
    }

    had_pressed = hcw_pivot_clear_pressed_state(self);
    local->current_index = index;
    if (local->on_changed != NULL)
    {
        local->on_changed(self, index);
    }
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

uint8_t hcw_pivot_get_current_index(egui_view_t *self)
{
    EGUI_LOCAL_INIT(hcw_pivot_t);
    return local->current_index;
}

void hcw_pivot_set_on_changed_listener(egui_view_t *self, hcw_pivot_on_changed_listener_t listener)
{
    EGUI_LOCAL_INIT(hcw_pivot_t);
    local->on_changed = listener;
}

void hcw_pivot_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(hcw_pivot_t);
    uint8_t had_pressed = hcw_pivot_clear_pressed_state(self);

    local->font = font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : font;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_pivot_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(hcw_pivot_t);
    uint8_t had_pressed = hcw_pivot_clear_pressed_state(self);

    local->meta_font = font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : font;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_pivot_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                           egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t card_surface_color)
{
    EGUI_LOCAL_INIT(hcw_pivot_t);
    uint8_t had_pressed = hcw_pivot_clear_pressed_state(self);

    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->card_surface_color = card_surface_color;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_pivot_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(hcw_pivot_t);
    uint8_t had_pressed = hcw_pivot_clear_pressed_state(self);

    local->compact_mode = compact_mode ? 1 : 0;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_pivot_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(hcw_pivot_t);
    uint8_t had_pressed = hcw_pivot_clear_pressed_state(self);

    local->read_only_mode = read_only_mode ? 1 : 0;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

uint8_t hcw_pivot_get_header_region(egui_view_t *self, uint8_t index, egui_region_t *region)
{
    EGUI_LOCAL_INIT(hcw_pivot_t);
    hcw_pivot_layout_item_t items[HCW_PIVOT_MAX_ITEMS];
    uint8_t count;

    if (region == NULL)
    {
        return 0;
    }

    count = hcw_pivot_prepare_header_layout(local, self, items);
    if (index >= count)
    {
        memset(region, 0, sizeof(*region));
        return 0;
    }

    *region = items[index].region;
    return 1;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_pivot_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(hcw_pivot_t);
    uint8_t hit_index;
    uint8_t same_target;

    if (!egui_view_get_enable(self) || local->item_count == 0 || local->read_only_mode)
    {
        hcw_pivot_clear_pressed_state(self);
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_index = hcw_pivot_resolve_hit(local, self, event->location.x, event->location.y);
        if (hit_index == HCW_PIVOT_INDEX_NONE)
        {
            hcw_pivot_clear_pressed_state(self);
            return 0;
        }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        same_target = (uint8_t)(local->pressed_index == hit_index);
        if (same_target && egui_view_get_pressed(self))
        {
            return 1;
        }
        local->pressed_index = hit_index;
        if (!egui_view_get_pressed(self))
        {
            egui_view_set_pressed(self, 1);
        }
        else
        {
            egui_view_invalidate(self);
        }
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_index == HCW_PIVOT_INDEX_NONE)
        {
            return 0;
        }
        hit_index = hcw_pivot_resolve_hit(local, self, event->location.x, event->location.y);
        if (hit_index == local->pressed_index)
        {
            if (!egui_view_get_pressed(self))
            {
                egui_view_set_pressed(self, 1);
            }
            return 1;
        }
        if (egui_view_get_pressed(self))
        {
            egui_view_set_pressed(self, 0);
        }
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
    {
        uint8_t handled;

        hit_index = hcw_pivot_resolve_hit(local, self, event->location.x, event->location.y);
        handled = (local->pressed_index != HCW_PIVOT_INDEX_NONE) || hit_index != HCW_PIVOT_INDEX_NONE;
        same_target = (uint8_t)(local->pressed_index != HCW_PIVOT_INDEX_NONE && local->pressed_index == hit_index);
        if (same_target && egui_view_get_pressed(self))
        {
            hcw_pivot_set_current_index(self, hit_index);
        }
        hcw_pivot_clear_pressed_state(self);
        return handled;
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        return hcw_pivot_clear_pressed_state(self);
    default:
        return 0;
    }
}

static int hcw_pivot_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_pivot_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_pivot_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(hcw_pivot_t);
    uint8_t next_index;

    if (!egui_view_get_enable(self) || local->item_count == 0 || local->read_only_mode)
    {
        hcw_pivot_clear_pressed_state(self);
        return 0;
    }

    if ((event->key_code == EGUI_KEY_CODE_ENTER || event->key_code == EGUI_KEY_CODE_SPACE) &&
        (event->type == EGUI_KEY_EVENT_ACTION_DOWN || event->type == EGUI_KEY_EVENT_ACTION_UP))
    {
        hcw_pivot_clear_pressed_state(self);
        return 1;
    }

    if (event->type != EGUI_KEY_EVENT_ACTION_UP)
    {
        return 0;
    }

    next_index = local->current_index;
    switch (event->key_code)
    {
    case EGUI_KEY_CODE_LEFT:
    case EGUI_KEY_CODE_UP:
        if (next_index > 0)
        {
            next_index--;
        }
        hcw_pivot_set_current_index(self, next_index);
        return 1;
    case EGUI_KEY_CODE_RIGHT:
    case EGUI_KEY_CODE_DOWN:
        if (next_index + 1 < local->item_count)
        {
            next_index++;
        }
        hcw_pivot_set_current_index(self, next_index);
        return 1;
    case EGUI_KEY_CODE_HOME:
        hcw_pivot_set_current_index(self, 0);
        return 1;
    case EGUI_KEY_CODE_END:
        hcw_pivot_set_current_index(self, (uint8_t)(local->item_count - 1));
        return 1;
    case EGUI_KEY_CODE_TAB:
        next_index++;
        if (next_index >= local->item_count)
        {
            next_index = 0;
        }
        hcw_pivot_set_current_index(self, next_index);
        return 1;
    default:
        return egui_view_on_key_event(self, event);
    }
}

static int hcw_pivot_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_pivot_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static void hcw_pivot_on_focus_change(egui_view_t *self, int is_focused)
{
    if (!is_focused)
    {
        hcw_pivot_clear_pressed_state(self);
    }
    egui_view_invalidate(self);
}
#endif

void hcw_pivot_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_pivot_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_pivot_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(hcw_pivot_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = hcw_pivot_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = hcw_pivot_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = hcw_pivot_on_key_event,
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        .on_focus_changed = hcw_pivot_on_focus_change,
#endif
};

void hcw_pivot_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(hcw_pivot_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(hcw_pivot_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 1);
#endif

    local->on_changed = NULL;
    local->items = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->item_count = 0;
    local->current_index = 0;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_index = HCW_PIVOT_INDEX_NONE;

    hcw_pivot_apply_standard_style(self);
    egui_view_set_view_name(self, "hcw_pivot");
}
