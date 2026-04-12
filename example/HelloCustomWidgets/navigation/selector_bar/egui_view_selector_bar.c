#include <stdlib.h>

#include "egui_view_selector_bar.h"
#include "egui_view_icon_font.h"

#define EGUI_VIEW_SELECTOR_BAR_STANDARD_BASE_WIDTH        20
#define EGUI_VIEW_SELECTOR_BAR_STANDARD_CHAR_WIDTH        5
#define EGUI_VIEW_SELECTOR_BAR_STANDARD_ICON_BONUS        12
#define EGUI_VIEW_SELECTOR_BAR_STANDARD_ICON_ONLY_WIDTH   30
#define EGUI_VIEW_SELECTOR_BAR_STANDARD_ACTIVE_BONUS      8
#define EGUI_VIEW_SELECTOR_BAR_STANDARD_MIN_WIDTH         34
#define EGUI_VIEW_SELECTOR_BAR_STANDARD_MAX_WIDTH         66
#define EGUI_VIEW_SELECTOR_BAR_STANDARD_GAP               8
#define EGUI_VIEW_SELECTOR_BAR_STANDARD_CONTENT_PAD_X     10
#define EGUI_VIEW_SELECTOR_BAR_STANDARD_CONTENT_PAD_Y     7
#define EGUI_VIEW_SELECTOR_BAR_STANDARD_LABEL_HEIGHT      11
#define EGUI_VIEW_SELECTOR_BAR_STANDARD_STACK_GAP         1
#define EGUI_VIEW_SELECTOR_BAR_STANDARD_DIVIDER_OFFSET    7
#define EGUI_VIEW_SELECTOR_BAR_STANDARD_FILL_ALPHA        82
#define EGUI_VIEW_SELECTOR_BAR_STANDARD_BORDER_ALPHA      38
#define EGUI_VIEW_SELECTOR_BAR_STANDARD_ACTIVE_FILL_ALPHA 24
#define EGUI_VIEW_SELECTOR_BAR_STANDARD_ACTIVE_FILL_MIX   5
#define EGUI_VIEW_SELECTOR_BAR_STANDARD_TEXT_MIX          9

#define EGUI_VIEW_SELECTOR_BAR_COMPACT_BASE_WIDTH        14
#define EGUI_VIEW_SELECTOR_BAR_COMPACT_CHAR_WIDTH        4
#define EGUI_VIEW_SELECTOR_BAR_COMPACT_ICON_BONUS        9
#define EGUI_VIEW_SELECTOR_BAR_COMPACT_ICON_ONLY_WIDTH   24
#define EGUI_VIEW_SELECTOR_BAR_COMPACT_ACTIVE_BONUS      6
#define EGUI_VIEW_SELECTOR_BAR_COMPACT_MIN_WIDTH         24
#define EGUI_VIEW_SELECTOR_BAR_COMPACT_MAX_WIDTH         48
#define EGUI_VIEW_SELECTOR_BAR_COMPACT_GAP               5
#define EGUI_VIEW_SELECTOR_BAR_COMPACT_CONTENT_PAD_X     8
#define EGUI_VIEW_SELECTOR_BAR_COMPACT_CONTENT_PAD_Y     5
#define EGUI_VIEW_SELECTOR_BAR_COMPACT_LABEL_HEIGHT      9
#define EGUI_VIEW_SELECTOR_BAR_COMPACT_STACK_GAP         0
#define EGUI_VIEW_SELECTOR_BAR_COMPACT_DIVIDER_OFFSET    6
#define EGUI_VIEW_SELECTOR_BAR_COMPACT_FILL_ALPHA        80
#define EGUI_VIEW_SELECTOR_BAR_COMPACT_BORDER_ALPHA      34
#define EGUI_VIEW_SELECTOR_BAR_COMPACT_ACTIVE_FILL_ALPHA 18
#define EGUI_VIEW_SELECTOR_BAR_COMPACT_ACTIVE_FILL_MIX   4
#define EGUI_VIEW_SELECTOR_BAR_COMPACT_TEXT_MIX          8

typedef struct egui_view_selector_bar_layout_item egui_view_selector_bar_layout_item_t;
struct egui_view_selector_bar_layout_item
{
    egui_dim_t x;
    egui_dim_t width;
    char label[16];
};

static uint8_t egui_view_selector_bar_clamp_count(uint8_t count)
{
    if (count > EGUI_VIEW_SELECTOR_BAR_MAX_ITEMS)
    {
        return EGUI_VIEW_SELECTOR_BAR_MAX_ITEMS;
    }
    return count;
}

static egui_color_t egui_view_selector_bar_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 64);
}

static uint8_t egui_view_selector_bar_clear_pressed_state(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_selector_bar_t);
    uint8_t was_pressed = self->is_pressed ? 1 : 0;
    uint8_t had_pressed = (uint8_t)(was_pressed || local->pressed_index != EGUI_VIEW_SELECTOR_BAR_INDEX_NONE);

    if (!had_pressed)
    {
        return 0;
    }

    local->pressed_index = EGUI_VIEW_SELECTOR_BAR_INDEX_NONE;
    if (was_pressed)
    {
        egui_view_set_pressed(self, false);
    }
    else
    {
        egui_view_invalidate(self);
    }
    return 1;
}

static uint8_t egui_view_selector_bar_text_len(const char *text)
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

static void egui_view_selector_bar_copy_elided(char *buffer, uint8_t buffer_size, const char *text, uint8_t max_chars)
{
    uint8_t length = 0;
    uint8_t copy_length;
    uint8_t i;

    if (buffer_size == 0)
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
        if (copy_length >= buffer_size)
        {
            copy_length = buffer_size - 1;
        }
        for (i = 0; i < copy_length; i++)
        {
            buffer[i] = text[i];
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
        for (i = 0; i < copy_length; i++)
        {
            buffer[i] = '.';
        }
        buffer[copy_length] = '\0';
        return;
    }

    copy_length = max_chars - 3;
    if (copy_length > buffer_size - 4)
    {
        copy_length = buffer_size - 4;
    }
    for (i = 0; i < copy_length; i++)
    {
        buffer[i] = text[i];
    }
    buffer[copy_length + 0] = '.';
    buffer[copy_length + 1] = '.';
    buffer[copy_length + 2] = '.';
    buffer[copy_length + 3] = '\0';
}

static egui_dim_t egui_view_selector_bar_measure_item_width(uint8_t compact_mode, uint8_t has_icon, uint8_t is_active, const char *text)
{
    egui_dim_t width;
    uint8_t length = egui_view_selector_bar_text_len(text);

    if (length == 0 && has_icon)
    {
        width = compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_ICON_ONLY_WIDTH : EGUI_VIEW_SELECTOR_BAR_STANDARD_ICON_ONLY_WIDTH;
        if (is_active)
        {
            width += compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_ACTIVE_BONUS : EGUI_VIEW_SELECTOR_BAR_STANDARD_ACTIVE_BONUS;
        }
        return width;
    }

    width = compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_BASE_WIDTH : EGUI_VIEW_SELECTOR_BAR_STANDARD_BASE_WIDTH;
    width += (egui_dim_t)length * (compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_CHAR_WIDTH : EGUI_VIEW_SELECTOR_BAR_STANDARD_CHAR_WIDTH);
    if (has_icon)
    {
        width += compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_ICON_BONUS : EGUI_VIEW_SELECTOR_BAR_STANDARD_ICON_BONUS;
    }
    if (is_active)
    {
        width += compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_ACTIVE_BONUS : EGUI_VIEW_SELECTOR_BAR_STANDARD_ACTIVE_BONUS;
    }

    if (compact_mode)
    {
        if (width < EGUI_VIEW_SELECTOR_BAR_COMPACT_MIN_WIDTH)
        {
            width = EGUI_VIEW_SELECTOR_BAR_COMPACT_MIN_WIDTH;
        }
        if (width > EGUI_VIEW_SELECTOR_BAR_COMPACT_MAX_WIDTH)
        {
            width = EGUI_VIEW_SELECTOR_BAR_COMPACT_MAX_WIDTH;
        }
    }
    else
    {
        if (width < EGUI_VIEW_SELECTOR_BAR_STANDARD_MIN_WIDTH)
        {
            width = EGUI_VIEW_SELECTOR_BAR_STANDARD_MIN_WIDTH;
        }
        if (width > EGUI_VIEW_SELECTOR_BAR_STANDARD_MAX_WIDTH)
        {
            width = EGUI_VIEW_SELECTOR_BAR_STANDARD_MAX_WIDTH;
        }
    }
    return width;
}

static egui_dim_t egui_view_selector_bar_item_gap(uint8_t compact_mode)
{
    return compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_GAP : EGUI_VIEW_SELECTOR_BAR_STANDARD_GAP;
}

static uint8_t egui_view_selector_bar_prepare_layout(egui_view_selector_bar_t *local, egui_dim_t start_x, egui_dim_t available_width,
                                                     egui_view_selector_bar_layout_item_t *items)
{
    egui_dim_t total_width = 0;
    egui_dim_t gap = egui_view_selector_bar_item_gap(local->compact_mode);
    uint8_t count = egui_view_selector_bar_clamp_count(local->item_count);
    uint8_t i;

    if (count == 0 || (local->item_texts == NULL && local->item_icons == NULL))
    {
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        const char *text = local->item_texts != NULL ? local->item_texts[i] : NULL;
        const char *icon = local->item_icons != NULL ? local->item_icons[i] : NULL;
        uint8_t has_icon = EGUI_VIEW_TEXT_VALID(icon) ? 1 : 0;
        uint8_t is_active = i == local->current_index;
        uint8_t max_chars = local->compact_mode ? (has_icon ? (is_active ? 7 : 6) : (is_active ? 9 : 7)) : (has_icon ? (is_active ? 9 : 8) : (is_active ? 11 : 9));

        egui_view_selector_bar_copy_elided(items[i].label, sizeof(items[i].label), text, max_chars);
        items[i].width = egui_view_selector_bar_measure_item_width(local->compact_mode, has_icon, is_active, items[i].label);
        total_width += items[i].width;
    }

    if (count > 1)
    {
        total_width += gap * (count - 1);
    }

    if (total_width > available_width)
    {
        egui_dim_t fallback_width = (available_width - gap * (count - 1)) / count;
        if (fallback_width < (local->compact_mode ? 20 : 24))
        {
            fallback_width = local->compact_mode ? 20 : 24;
        }
        for (i = 0; i < count; i++)
        {
            items[i].width = fallback_width;
        }
        total_width = fallback_width * count + gap * (count - 1);
    }

    if (total_width < available_width)
    {
        start_x += (available_width - total_width) / 2;
    }

    for (i = 0; i < count; i++)
    {
        items[i].x = start_x;
        start_x += items[i].width + gap;
    }

    return count;
}

static void egui_view_selector_bar_draw_item_content(egui_view_selector_bar_t *local, egui_view_t *self, const egui_region_t *item_region, const char *icon,
                                                     const char *text, egui_color_t color)
{
    uint8_t has_icon = EGUI_VIEW_TEXT_VALID(icon) ? 1 : 0;
    uint8_t has_text = EGUI_VIEW_TEXT_VALID(text) ? 1 : 0;
    const egui_font_t *icon_font;
    egui_region_t content_region;

    content_region = *item_region;
    if (content_region.size.width > 4)
    {
        content_region.location.x += 2;
        content_region.size.width -= 4;
    }
    if (content_region.size.height > 4)
    {
        content_region.location.y += 2;
        content_region.size.height -= 4;
    }

    if (has_icon && has_text)
    {
        egui_dim_t icon_height = local->compact_mode ? 15 : 18;
        egui_dim_t label_height = local->compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_LABEL_HEIGHT : EGUI_VIEW_SELECTOR_BAR_STANDARD_LABEL_HEIGHT;
        egui_dim_t stack_gap = local->compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_STACK_GAP : EGUI_VIEW_SELECTOR_BAR_STANDARD_STACK_GAP;
        egui_dim_t content_height;
        egui_dim_t content_y;
        egui_region_t icon_region;
        egui_region_t text_region;

        if (label_height >= content_region.size.height)
        {
            label_height = content_region.size.height > 0 ? content_region.size.height : 0;
        }
        if (icon_height + label_height + stack_gap > content_region.size.height)
        {
            icon_height = content_region.size.height - label_height - stack_gap;
            if (icon_height < content_region.size.height / 2)
            {
                icon_height = content_region.size.height / 2;
            }
        }
        if (icon_height < 0)
        {
            icon_height = 0;
        }

        content_height = icon_height + label_height + stack_gap;
        if (content_height > content_region.size.height)
        {
            content_height = content_region.size.height;
        }
        content_y = content_region.location.y + (content_region.size.height - content_height) / 2;

        icon_region.location.x = content_region.location.x;
        icon_region.location.y = content_y;
        icon_region.size.width = content_region.size.width;
        icon_region.size.height = icon_height;

        text_region.location.x = content_region.location.x;
        text_region.location.y = content_y + icon_height + stack_gap;
        text_region.size.width = content_region.size.width;
        text_region.size.height = label_height;

        icon_font = EGUI_VIEW_ICON_FONT_RESOLVE(local->icon_font, content_region.size.height, 18, 22);
        if (icon_font != NULL)
        {
            egui_canvas_draw_text_in_rect(icon_font, icon, &icon_region, EGUI_ALIGN_CENTER, color, self->alpha);
        }
        egui_canvas_draw_text_in_rect(local->font, text, &text_region, EGUI_ALIGN_CENTER, color, self->alpha);
        return;
    }

    if (has_icon)
    {
        icon_font = EGUI_VIEW_ICON_FONT_RESOLVE(local->icon_font, content_region.size.height, 18, 22);
        if (icon_font != NULL)
        {
            egui_canvas_draw_text_in_rect(icon_font, icon, &content_region, EGUI_ALIGN_CENTER, color, self->alpha);
        }
        return;
    }

    if (has_text)
    {
        egui_canvas_draw_text_in_rect(local->font, text, &content_region, EGUI_ALIGN_CENTER, color, self->alpha);
    }
}

static uint8_t egui_view_selector_bar_resolve_hit(egui_view_selector_bar_t *local, egui_view_t *self, egui_dim_t screen_x, egui_dim_t screen_y)
{
    egui_region_t region;
    egui_view_selector_bar_layout_item_t items[EGUI_VIEW_SELECTOR_BAR_MAX_ITEMS];
    egui_dim_t content_x;
    egui_dim_t content_w;
    uint8_t count;
    uint8_t i;

    egui_view_get_work_region(self, &region);
    if (!egui_region_pt_in_rect(&region, screen_x, screen_y))
    {
        return EGUI_VIEW_SELECTOR_BAR_INDEX_NONE;
    }

    content_x = region.location.x + (local->compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_CONTENT_PAD_X : EGUI_VIEW_SELECTOR_BAR_STANDARD_CONTENT_PAD_X);
    content_w = region.size.width -
                (local->compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_CONTENT_PAD_X * 2 : EGUI_VIEW_SELECTOR_BAR_STANDARD_CONTENT_PAD_X * 2);
    if (content_w <= 0)
    {
        return EGUI_VIEW_SELECTOR_BAR_INDEX_NONE;
    }

    count = egui_view_selector_bar_prepare_layout(local, content_x, content_w, items);
    for (i = 0; i < count; i++)
    {
        if (screen_x >= items[i].x && screen_x < items[i].x + items[i].width)
        {
            return i;
        }
    }

    return EGUI_VIEW_SELECTOR_BAR_INDEX_NONE;
}

void egui_view_selector_bar_set_items(egui_view_t *self, const char **item_texts, const char **item_icons, uint8_t item_count)
{
    EGUI_LOCAL_INIT(egui_view_selector_bar_t);
    uint8_t had_pressed = egui_view_selector_bar_clear_pressed_state(self);

    local->item_texts = item_texts;
    local->item_icons = item_icons;
    local->item_count = (item_texts != NULL || item_icons != NULL) ? egui_view_selector_bar_clamp_count(item_count) : 0;
    if (local->item_count == 0)
    {
        local->current_index = EGUI_VIEW_SELECTOR_BAR_INDEX_NONE;
    }
    else if (local->current_index != EGUI_VIEW_SELECTOR_BAR_INDEX_NONE && local->current_index >= local->item_count)
    {
        local->current_index = 0;
    }

    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_selector_bar_set_current_index(egui_view_t *self, uint8_t index)
{
    EGUI_LOCAL_INIT(egui_view_selector_bar_t);
    uint8_t had_pressed;

    if (local->item_count == 0)
    {
        egui_view_selector_bar_clear_pressed_state(self);
        local->current_index = EGUI_VIEW_SELECTOR_BAR_INDEX_NONE;
        return;
    }
    if (index != EGUI_VIEW_SELECTOR_BAR_INDEX_NONE && index >= local->item_count)
    {
        egui_view_selector_bar_clear_pressed_state(self);
        return;
    }
    if (local->current_index == index)
    {
        egui_view_selector_bar_clear_pressed_state(self);
        return;
    }

    had_pressed = egui_view_selector_bar_clear_pressed_state(self);
    local->current_index = index;
    if (local->on_selection_changed)
    {
        local->on_selection_changed(self, index);
    }
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

uint8_t egui_view_selector_bar_get_current_index(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_selector_bar_t);
    return local->current_index;
}

void egui_view_selector_bar_set_on_selection_changed_listener(egui_view_t *self, egui_view_on_selector_bar_changed_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_selector_bar_t);
    local->on_selection_changed = listener;
}

void egui_view_selector_bar_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_selector_bar_t);
    uint8_t had_pressed = egui_view_selector_bar_clear_pressed_state(self);

    local->font = font ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_selector_bar_set_icon_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_selector_bar_t);
    uint8_t had_pressed = egui_view_selector_bar_clear_pressed_state(self);

    local->icon_font = font;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_selector_bar_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_selector_bar_t);
    uint8_t had_pressed = egui_view_selector_bar_clear_pressed_state(self);

    local->compact_mode = compact_mode ? 1 : 0;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_selector_bar_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                        egui_color_t muted_text_color, egui_color_t accent_color)
{
    EGUI_LOCAL_INIT(egui_view_selector_bar_t);
    uint8_t had_pressed = egui_view_selector_bar_clear_pressed_state(self);

    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

static void egui_view_selector_bar_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_selector_bar_t);
    egui_region_t region;
    egui_view_selector_bar_layout_item_t items[EGUI_VIEW_SELECTOR_BAR_MAX_ITEMS];
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_dim_t content_x;
    egui_dim_t content_y;
    egui_dim_t content_w;
    egui_dim_t divider_y;
    egui_dim_t radius;
    egui_dim_t item_height;
    uint8_t count;
    uint8_t is_enabled;
    uint8_t i;

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0 || local->item_count == 0 || (local->item_texts == NULL && local->item_icons == NULL))
    {
        return;
    }

    surface_color = local->surface_color;
    border_color = local->border_color;
    text_color = local->text_color;
    muted_text_color = local->muted_text_color;
    accent_color = local->accent_color;
    is_enabled = egui_view_get_enable(self) ? 1 : 0;
    radius = local->compact_mode ? 8 : 10;

    if (!local->compact_mode)
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xFFFFFF), 16);
        border_color = egui_rgb_mix(border_color, EGUI_COLOR_HEX(0xFFFFFF), 10);
    }
    else
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xFBFCFD), 20);
        muted_text_color = egui_rgb_mix(muted_text_color, text_color, 4);
    }

    if (!is_enabled)
    {
        surface_color = egui_view_selector_bar_mix_disabled(surface_color);
        border_color = egui_view_selector_bar_mix_disabled(border_color);
        text_color = egui_view_selector_bar_mix_disabled(text_color);
        muted_text_color = egui_view_selector_bar_mix_disabled(muted_text_color);
        accent_color = egui_view_selector_bar_mix_disabled(accent_color);
    }

    egui_canvas_draw_round_rectangle_fill(
            region.location.x, region.location.y, region.size.width, region.size.height, radius, surface_color,
            egui_color_alpha_mix(self->alpha, local->compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_FILL_ALPHA : EGUI_VIEW_SELECTOR_BAR_STANDARD_FILL_ALPHA));
    egui_canvas_draw_round_rectangle(
            region.location.x, region.location.y, region.size.width, region.size.height, radius, 1, border_color,
            egui_color_alpha_mix(self->alpha, local->compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_BORDER_ALPHA : EGUI_VIEW_SELECTOR_BAR_STANDARD_BORDER_ALPHA));

    content_x = region.location.x + (local->compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_CONTENT_PAD_X : EGUI_VIEW_SELECTOR_BAR_STANDARD_CONTENT_PAD_X);
    content_y = region.location.y + (local->compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_CONTENT_PAD_Y : EGUI_VIEW_SELECTOR_BAR_STANDARD_CONTENT_PAD_Y);
    content_w = region.size.width -
                (local->compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_CONTENT_PAD_X * 2 : EGUI_VIEW_SELECTOR_BAR_STANDARD_CONTENT_PAD_X * 2);
    if (content_w <= 0)
    {
        return;
    }

    divider_y = region.location.y + region.size.height -
                (local->compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_DIVIDER_OFFSET : EGUI_VIEW_SELECTOR_BAR_STANDARD_DIVIDER_OFFSET);
    item_height = divider_y - content_y - 1;
    if (item_height <= 0)
    {
        return;
    }

    count = egui_view_selector_bar_prepare_layout(local, content_x, content_w, items);
    egui_canvas_draw_line(content_x, divider_y, content_x + content_w, divider_y, 1, border_color,
                          egui_color_alpha_mix(self->alpha, local->compact_mode ? 20 : 18));

    for (i = 0; i < count; i++)
    {
        egui_region_t item_region;
        const char *text = local->item_texts != NULL ? local->item_texts[i] : NULL;
        const char *icon = local->item_icons != NULL ? local->item_icons[i] : NULL;
        uint8_t is_active = i == local->current_index;
        egui_color_t item_text_color = is_active ? text_color : muted_text_color;

        item_region.location.x = items[i].x;
        item_region.location.y = content_y;
        item_region.size.width = items[i].width;
        item_region.size.height = item_height;

        if (is_active)
        {
            egui_color_t active_fill =
                    egui_rgb_mix(surface_color, accent_color,
                                 local->compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_ACTIVE_FILL_MIX : EGUI_VIEW_SELECTOR_BAR_STANDARD_ACTIVE_FILL_MIX);
            egui_alpha_t active_alpha =
                    local->compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_ACTIVE_FILL_ALPHA : EGUI_VIEW_SELECTOR_BAR_STANDARD_ACTIVE_FILL_ALPHA;
            egui_dim_t fill_x = item_region.location.x + 1;
            egui_dim_t fill_y = item_region.location.y + 1;
            egui_dim_t fill_w = item_region.size.width > 2 ? item_region.size.width - 2 : item_region.size.width;
            egui_dim_t fill_h = item_region.size.height > 2 ? item_region.size.height - 2 : item_region.size.height;

            egui_canvas_draw_round_rectangle_fill(fill_x, fill_y, fill_w, fill_h, local->compact_mode ? 7 : 8, active_fill,
                                                  egui_color_alpha_mix(self->alpha, active_alpha));
            if (is_enabled)
            {
                item_text_color = egui_rgb_mix(text_color, accent_color,
                                               local->compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_TEXT_MIX : EGUI_VIEW_SELECTOR_BAR_STANDARD_TEXT_MIX);
            }
        }

        if (is_enabled && local->pressed_index == i)
        {
            egui_canvas_draw_round_rectangle_fill(item_region.location.x + 1, item_region.location.y + 1,
                                                  item_region.size.width > 2 ? item_region.size.width - 2 : item_region.size.width,
                                                  item_region.size.height > 2 ? item_region.size.height - 2 : item_region.size.height,
                                                  local->compact_mode ? 7 : 8, EGUI_THEME_PRESS_OVERLAY, EGUI_THEME_PRESS_OVERLAY_ALPHA);
        }

        egui_view_selector_bar_draw_item_content(local, self, &item_region, icon, text, item_text_color);

        if (is_active)
        {
            egui_dim_t indicator_w = item_region.size.width - (local->compact_mode ? 10 : 12);
            egui_dim_t indicator_x;

            if (indicator_w < (local->compact_mode ? 10 : 14))
            {
                indicator_w = local->compact_mode ? 10 : 14;
            }
            indicator_x = item_region.location.x + (item_region.size.width - indicator_w) / 2;
            egui_canvas_draw_line(indicator_x, divider_y, indicator_x + indicator_w, divider_y, 1, accent_color,
                                  egui_color_alpha_mix(self->alpha, 56));
            egui_canvas_draw_line(indicator_x, divider_y + 1, indicator_x + indicator_w, divider_y + 1, 1, accent_color,
                                  egui_color_alpha_mix(self->alpha, 22));
        }
    }

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (self->is_focused && is_enabled && region.size.width > 4 && region.size.height > 4)
    {
        egui_dim_t focus_x = region.location.x + 2;
        egui_dim_t focus_y = region.location.y + 2;
        egui_dim_t focus_w = region.size.width - 4;
        egui_dim_t focus_h = region.size.height - 4;
        egui_dim_t focus_radius = radius > 2 ? radius - 2 : radius;

        egui_canvas_draw_round_rectangle(region.location.x, region.location.y, region.size.width, region.size.height, radius, 2, EGUI_THEME_FOCUS,
                                         egui_color_alpha_mix(self->alpha, 96));
        egui_canvas_draw_round_rectangle(focus_x, focus_y, focus_w, focus_h, focus_radius, 1, EGUI_THEME_FOCUS, egui_color_alpha_mix(self->alpha, 48));
    }
#endif
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_selector_bar_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_selector_bar_t);
    uint8_t hit_index;
    uint8_t handled;
    uint8_t same_target;

    if (!egui_view_get_enable(self) || local->item_count == 0 || (local->item_texts == NULL && local->item_icons == NULL))
    {
        egui_view_selector_bar_clear_pressed_state(self);
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_index = egui_view_selector_bar_resolve_hit(local, self, event->location.x, event->location.y);
        if (hit_index == EGUI_VIEW_SELECTOR_BAR_INDEX_NONE)
        {
            egui_view_selector_bar_clear_pressed_state(self);
            return 0;
        }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        same_target = (uint8_t)(local->pressed_index == hit_index);
        if (same_target && self->is_pressed)
        {
            return 1;
        }
        local->pressed_index = hit_index;
        if (!self->is_pressed)
        {
            egui_view_set_pressed(self, true);
        }
        else
        {
            egui_view_invalidate(self);
        }
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_index == EGUI_VIEW_SELECTOR_BAR_INDEX_NONE)
        {
            return 0;
        }
        hit_index = egui_view_selector_bar_resolve_hit(local, self, event->location.x, event->location.y);
        if (hit_index == local->pressed_index)
        {
            if (!self->is_pressed)
            {
                egui_view_set_pressed(self, true);
            }
            return 1;
        }
        if (self->is_pressed)
        {
            egui_view_set_pressed(self, false);
        }
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
        hit_index = egui_view_selector_bar_resolve_hit(local, self, event->location.x, event->location.y);
        handled = (local->pressed_index != EGUI_VIEW_SELECTOR_BAR_INDEX_NONE) || hit_index != EGUI_VIEW_SELECTOR_BAR_INDEX_NONE;
        same_target = (uint8_t)(local->pressed_index != EGUI_VIEW_SELECTOR_BAR_INDEX_NONE && local->pressed_index == hit_index);
        if (same_target && self->is_pressed)
        {
            egui_view_selector_bar_set_current_index(self, hit_index);
        }
        egui_view_selector_bar_clear_pressed_state(self);
        return handled;
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        return egui_view_selector_bar_clear_pressed_state(self);
    default:
        return 0;
    }
}

static int egui_view_selector_bar_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_selector_bar_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_selector_bar_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_selector_bar_t);
    uint8_t next_index;

    if (!egui_view_get_enable(self) || local->item_count == 0)
    {
        egui_view_selector_bar_clear_pressed_state(self);
        return 0;
    }

    if ((event->key_code == EGUI_KEY_CODE_ENTER || event->key_code == EGUI_KEY_CODE_SPACE) &&
        (event->type == EGUI_KEY_EVENT_ACTION_DOWN || event->type == EGUI_KEY_EVENT_ACTION_UP))
    {
        egui_view_selector_bar_clear_pressed_state(self);
        return 1;
    }

    if (event->type != EGUI_KEY_EVENT_ACTION_UP)
    {
        return 0;
    }

    next_index = local->current_index;
    if (next_index == EGUI_VIEW_SELECTOR_BAR_INDEX_NONE)
    {
        next_index = 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_LEFT:
    case EGUI_KEY_CODE_UP:
        if (next_index > 0)
        {
            next_index--;
        }
        egui_view_selector_bar_set_current_index(self, next_index);
        return 1;
    case EGUI_KEY_CODE_RIGHT:
    case EGUI_KEY_CODE_DOWN:
        if (next_index + 1 < local->item_count)
        {
            next_index++;
        }
        egui_view_selector_bar_set_current_index(self, next_index);
        return 1;
    case EGUI_KEY_CODE_HOME:
        egui_view_selector_bar_set_current_index(self, 0);
        return 1;
    case EGUI_KEY_CODE_END:
        egui_view_selector_bar_set_current_index(self, (uint8_t)(local->item_count - 1));
        return 1;
    case EGUI_KEY_CODE_TAB:
        next_index++;
        if (next_index >= local->item_count)
        {
            next_index = 0;
        }
        egui_view_selector_bar_set_current_index(self, next_index);
        return 1;
    default:
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_selector_bar_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_selector_bar_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static void egui_view_selector_bar_on_focus_change(egui_view_t *self, int is_focused)
{
    EGUI_LOCAL_INIT(egui_view_selector_bar_t);

    if (!is_focused)
    {
        egui_view_selector_bar_clear_pressed_state(self);
        egui_view_invalidate(self);
        return;
    }

    if (local->current_index == EGUI_VIEW_SELECTOR_BAR_INDEX_NONE && local->item_count > 0)
    {
        egui_view_selector_bar_set_current_index(self, 0);
        return;
    }

    egui_view_invalidate(self);
}
#endif

void egui_view_selector_bar_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_selector_bar_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_selector_bar_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_selector_bar_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_selector_bar_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_selector_bar_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_selector_bar_on_key_event,
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        .on_focus_changed = egui_view_selector_bar_on_focus_change,
#endif
};

void egui_view_selector_bar_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_selector_bar_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_selector_bar_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif

    local->on_selection_changed = NULL;
    local->item_texts = NULL;
    local->item_icons = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->icon_font = NULL;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xD4DCE4);
    local->text_color = EGUI_COLOR_HEX(0x1D2732);
    local->muted_text_color = EGUI_COLOR_HEX(0x687786);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->item_count = 0;
    local->current_index = EGUI_VIEW_SELECTOR_BAR_INDEX_NONE;
    local->compact_mode = 0;
    local->pressed_index = EGUI_VIEW_SELECTOR_BAR_INDEX_NONE;

    egui_view_set_view_name(self, "egui_view_selector_bar");
}
