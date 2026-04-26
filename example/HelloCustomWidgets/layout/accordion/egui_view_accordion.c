#include "egui_view_accordion.h"

#define EGUI_VIEW_ACCORDION_STANDARD_RADIUS       8
#define EGUI_VIEW_ACCORDION_STANDARD_PAD_X        7
#define EGUI_VIEW_ACCORDION_STANDARD_PAD_Y        7
#define EGUI_VIEW_ACCORDION_STANDARD_ITEM_GAP     4
#define EGUI_VIEW_ACCORDION_STANDARD_HEADER_H     28
#define EGUI_VIEW_ACCORDION_STANDARD_BODY_H       30
#define EGUI_VIEW_ACCORDION_STANDARD_ICON_SIZE    13
#define EGUI_VIEW_ACCORDION_STANDARD_CHEVRON_SIZE 9

#define EGUI_VIEW_ACCORDION_COMPACT_RADIUS       6
#define EGUI_VIEW_ACCORDION_COMPACT_PAD_X        5
#define EGUI_VIEW_ACCORDION_COMPACT_PAD_Y        5
#define EGUI_VIEW_ACCORDION_COMPACT_ITEM_GAP     3
#define EGUI_VIEW_ACCORDION_COMPACT_HEADER_H     22
#define EGUI_VIEW_ACCORDION_COMPACT_BODY_H       18
#define EGUI_VIEW_ACCORDION_COMPACT_ICON_SIZE    10
#define EGUI_VIEW_ACCORDION_COMPACT_CHEVRON_SIZE 7

typedef struct egui_view_accordion_metrics egui_view_accordion_metrics_t;
struct egui_view_accordion_metrics
{
    egui_region_t region;
    egui_region_t content_region;
    egui_region_t item_region[EGUI_VIEW_ACCORDION_MAX_ITEMS];
    egui_region_t header_region[EGUI_VIEW_ACCORDION_MAX_ITEMS];
    egui_region_t body_region[EGUI_VIEW_ACCORDION_MAX_ITEMS];
    egui_region_t accent_region[EGUI_VIEW_ACCORDION_MAX_ITEMS];
    egui_region_t icon_region[EGUI_VIEW_ACCORDION_MAX_ITEMS];
    egui_region_t title_region[EGUI_VIEW_ACCORDION_MAX_ITEMS];
    egui_region_t description_region[EGUI_VIEW_ACCORDION_MAX_ITEMS];
    egui_region_t meta_region[EGUI_VIEW_ACCORDION_MAX_ITEMS];
    egui_region_t chevron_region[EGUI_VIEW_ACCORDION_MAX_ITEMS];
};

static uint8_t egui_view_accordion_clamp_item_count(uint8_t count)
{
    return count > EGUI_VIEW_ACCORDION_MAX_ITEMS ? EGUI_VIEW_ACCORDION_MAX_ITEMS : count;
}

static uint8_t egui_view_accordion_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static uint8_t egui_view_accordion_text_len(const char *text)
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

static uint8_t egui_view_accordion_is_space_char(char c)
{
    return (uint8_t)(c == ' ' || c == '\t');
}

static uint8_t egui_view_accordion_is_break_after_char(char c)
{
    return (uint8_t)(c == '-' || c == '/');
}

static uint8_t egui_view_accordion_find_elide_boundary(const char *text, uint8_t visible_chars)
{
    uint8_t index;

    if (text == NULL || visible_chars == 0)
    {
        return 0;
    }

    for (index = visible_chars; index > 0; --index)
    {
        if (egui_view_accordion_is_space_char(text[index - 1]))
        {
            return (uint8_t)(index - 1);
        }
    }
    for (index = visible_chars; index > 0; --index)
    {
        if (egui_view_accordion_is_break_after_char(text[index - 1]))
        {
            return index;
        }
    }
    return visible_chars;
}

static void egui_view_accordion_copy_elided(char *buffer, uint8_t buffer_size, const char *text, uint8_t max_chars)
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

    length = egui_view_accordion_text_len(text);
    if (length <= max_chars)
    {
        copy_length = length;
        if (copy_length >= buffer_size)
        {
            copy_length = buffer_size - 1;
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
        if (copy_length >= buffer_size)
        {
            copy_length = buffer_size - 1;
        }
        for (index = 0; index < copy_length; index++)
        {
            buffer[index] = '.';
        }
        buffer[copy_length] = '\0';
        return;
    }

    copy_length = egui_view_accordion_find_elide_boundary(text, (uint8_t)(max_chars - 3));
    if (copy_length > buffer_size - 4)
    {
        copy_length = buffer_size - 4;
    }
    for (index = 0; index < copy_length; index++)
    {
        buffer[index] = text[index];
    }
    buffer[copy_length] = '.';
    buffer[copy_length + 1] = '.';
    buffer[copy_length + 2] = '.';
    buffer[copy_length + 3] = '\0';
}

static egui_dim_t egui_view_accordion_measure_text_width(const egui_font_t *font, const char *text)
{
    egui_dim_t text_width = 0;
    egui_dim_t dummy_height = 0;

    if (!egui_view_accordion_has_text(text) || font == NULL || font->api == NULL || font->api->get_str_size == NULL)
    {
        return 0;
    }
    font->api->get_str_size(font, text, 0, 0, &text_width, &dummy_height);
    return text_width;
}

static void egui_view_accordion_fit_text_to_width(const egui_font_t *font, const char *text, char *buffer, uint8_t buffer_size, egui_dim_t max_width,
                                                  egui_dim_t fallback_char_width)
{
    uint8_t max_chars;

    if (buffer == NULL || buffer_size == 0)
    {
        return;
    }

    buffer[0] = '\0';
    if (!egui_view_accordion_has_text(text) || max_width <= 0)
    {
        return;
    }

    max_chars = egui_view_accordion_text_len(text);
    egui_view_accordion_copy_elided(buffer, buffer_size, text, max_chars);
    while (max_chars > 0)
    {
        egui_dim_t text_width = egui_view_accordion_measure_text_width(font, buffer);

        if (text_width <= 0)
        {
            text_width = (egui_dim_t)egui_view_accordion_text_len(buffer) * fallback_char_width;
        }
        if (text_width <= max_width)
        {
            break;
        }
        max_chars--;
        egui_view_accordion_copy_elided(buffer, buffer_size, text, max_chars);
    }
}

static egui_color_t egui_view_accordion_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 66);
}

static egui_color_t egui_view_accordion_tone_color(egui_view_accordion_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_ACCORDION_TONE_SUCCESS:
        return local->success_color;
    case EGUI_VIEW_ACCORDION_TONE_WARNING:
        return local->warning_color;
    case EGUI_VIEW_ACCORDION_TONE_NEUTRAL:
        return local->neutral_color;
    case EGUI_VIEW_ACCORDION_TONE_ACCENT:
    default:
        return local->accent_color;
    }
}

static uint8_t egui_view_accordion_index_is_valid(egui_view_accordion_t *local, uint8_t index)
{
    return index < local->item_count ? 1 : 0;
}

static const egui_view_accordion_item_t *egui_view_accordion_get_item(egui_view_accordion_t *local, uint8_t index)
{
    if (local->items == NULL || !egui_view_accordion_index_is_valid(local, index))
    {
        return NULL;
    }
    return &local->items[index];
}

static uint8_t egui_view_accordion_clear_pressed_state(egui_view_t *self, egui_view_accordion_t *local)
{
    uint8_t had_pressed = (uint8_t)(self->is_pressed || local->pressed_index != EGUI_VIEW_ACCORDION_INDEX_NONE);

    local->pressed_index = EGUI_VIEW_ACCORDION_INDEX_NONE;
    egui_view_set_pressed(self, false);
    return had_pressed;
}

static void egui_view_accordion_sync_indexes(egui_view_accordion_t *local)
{
    if (local->item_count == 0)
    {
        local->expanded_index = EGUI_VIEW_ACCORDION_INDEX_NONE;
        local->focused_index = EGUI_VIEW_ACCORDION_INDEX_NONE;
        return;
    }
    if (!egui_view_accordion_index_is_valid(local, local->expanded_index))
    {
        local->expanded_index = EGUI_VIEW_ACCORDION_INDEX_NONE;
    }
    if (!egui_view_accordion_index_is_valid(local, local->focused_index))
    {
        local->focused_index = 0;
    }
}

static uint8_t egui_view_accordion_is_interactive(egui_view_accordion_t *local, egui_view_t *self)
{
    return (uint8_t)(local->item_count > 0 && !local->read_only_mode && egui_view_get_enable(self));
}

static uint8_t egui_view_accordion_find_default_expanded(const egui_view_accordion_item_t *items, uint8_t count)
{
    uint8_t index;

    if (items == NULL)
    {
        return EGUI_VIEW_ACCORDION_INDEX_NONE;
    }
    count = egui_view_accordion_clamp_item_count(count);
    for (index = 0; index < count; index++)
    {
        if (items[index].expanded)
        {
            return index;
        }
    }
    return EGUI_VIEW_ACCORDION_INDEX_NONE;
}

void egui_view_accordion_set_items(egui_view_t *self, const egui_view_accordion_item_t *items, uint8_t item_count)
{
    EGUI_LOCAL_INIT(egui_view_accordion_t);

    egui_view_accordion_clear_pressed_state(self, local);
    local->items = items;
    local->item_count = items == NULL ? 0 : egui_view_accordion_clamp_item_count(item_count);
    local->expanded_index = egui_view_accordion_find_default_expanded(items, local->item_count);
    local->focused_index = local->item_count > 0 ? 0 : EGUI_VIEW_ACCORDION_INDEX_NONE;
    egui_view_invalidate(self);
}

void egui_view_accordion_set_expanded_index(egui_view_t *self, uint8_t item_index)
{
    EGUI_LOCAL_INIT(egui_view_accordion_t);
    uint8_t had_pressed = egui_view_accordion_clear_pressed_state(self, local);

    if (item_index != EGUI_VIEW_ACCORDION_INDEX_NONE && !egui_view_accordion_index_is_valid(local, item_index))
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }
    if (local->expanded_index == item_index)
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }
    local->expanded_index = item_index;
    if (item_index != EGUI_VIEW_ACCORDION_INDEX_NONE)
    {
        local->focused_index = item_index;
    }
    egui_view_invalidate(self);
}

uint8_t egui_view_accordion_get_expanded_index(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_accordion_t);
    egui_view_accordion_sync_indexes(local);
    return local->expanded_index;
}

void egui_view_accordion_set_focused_index(egui_view_t *self, uint8_t item_index)
{
    EGUI_LOCAL_INIT(egui_view_accordion_t);
    uint8_t had_pressed = egui_view_accordion_clear_pressed_state(self, local);

    if (!egui_view_accordion_index_is_valid(local, item_index))
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }
    local->focused_index = item_index;
    egui_view_invalidate(self);
}

uint8_t egui_view_accordion_get_focused_index(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_accordion_t);
    egui_view_accordion_sync_indexes(local);
    return local->focused_index;
}

uint8_t egui_view_accordion_activate_focused(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_accordion_t);
    uint8_t index;
    uint8_t expanded;

    egui_view_accordion_sync_indexes(local);
    if (!egui_view_accordion_is_interactive(local, self) || !egui_view_accordion_index_is_valid(local, local->focused_index))
    {
        return 0;
    }

    index = local->focused_index;
    expanded = local->expanded_index == index ? 0 : 1;
    local->expanded_index = expanded ? index : EGUI_VIEW_ACCORDION_INDEX_NONE;
    if (local->on_action != NULL)
    {
        local->on_action(self, index, expanded);
    }
    egui_view_invalidate(self);
    return 1;
}

void egui_view_accordion_set_on_action_listener(egui_view_t *self, egui_view_accordion_action_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_accordion_t);
    local->on_action = listener;
}

void egui_view_accordion_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_accordion_t);
    egui_view_accordion_clear_pressed_state(self, local);
    local->font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_invalidate(self);
}

void egui_view_accordion_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_accordion_t);
    egui_view_accordion_clear_pressed_state(self, local);
    local->meta_font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_invalidate(self);
}

void egui_view_accordion_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_accordion_t);
    egui_view_accordion_clear_pressed_state(self, local);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

void egui_view_accordion_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_accordion_t);
    egui_view_accordion_clear_pressed_state(self, local);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

void egui_view_accordion_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                                     egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                     egui_color_t success_color, egui_color_t warning_color, egui_color_t neutral_color)
{
    EGUI_LOCAL_INIT(egui_view_accordion_t);

    egui_view_accordion_clear_pressed_state(self, local);
    local->surface_color = surface_color;
    local->section_color = section_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->neutral_color = neutral_color;
    egui_view_invalidate(self);
}

static void egui_view_accordion_clear_metrics(egui_view_accordion_metrics_t *metrics)
{
    uint8_t index;

    metrics->region.location.x = 0;
    metrics->region.location.y = 0;
    metrics->region.size.width = 0;
    metrics->region.size.height = 0;
    metrics->content_region = metrics->region;
    for (index = 0; index < EGUI_VIEW_ACCORDION_MAX_ITEMS; index++)
    {
        metrics->item_region[index] = metrics->region;
        metrics->header_region[index] = metrics->region;
        metrics->body_region[index] = metrics->region;
        metrics->accent_region[index] = metrics->region;
        metrics->icon_region[index] = metrics->region;
        metrics->title_region[index] = metrics->region;
        metrics->description_region[index] = metrics->region;
        metrics->meta_region[index] = metrics->region;
        metrics->chevron_region[index] = metrics->region;
    }
}

static void egui_view_accordion_get_metrics(egui_view_accordion_t *local, egui_view_t *self, egui_view_accordion_metrics_t *metrics)
{
    egui_dim_t pad_x = local->compact_mode ? EGUI_VIEW_ACCORDION_COMPACT_PAD_X : EGUI_VIEW_ACCORDION_STANDARD_PAD_X;
    egui_dim_t pad_y = local->compact_mode ? EGUI_VIEW_ACCORDION_COMPACT_PAD_Y : EGUI_VIEW_ACCORDION_STANDARD_PAD_Y;
    egui_dim_t gap = local->compact_mode ? EGUI_VIEW_ACCORDION_COMPACT_ITEM_GAP : EGUI_VIEW_ACCORDION_STANDARD_ITEM_GAP;
    egui_dim_t header_h = local->compact_mode ? EGUI_VIEW_ACCORDION_COMPACT_HEADER_H : EGUI_VIEW_ACCORDION_STANDARD_HEADER_H;
    egui_dim_t body_h = local->compact_mode ? EGUI_VIEW_ACCORDION_COMPACT_BODY_H : EGUI_VIEW_ACCORDION_STANDARD_BODY_H;
    egui_dim_t icon_size = local->compact_mode ? EGUI_VIEW_ACCORDION_COMPACT_ICON_SIZE : EGUI_VIEW_ACCORDION_STANDARD_ICON_SIZE;
    egui_dim_t chevron_size = local->compact_mode ? EGUI_VIEW_ACCORDION_COMPACT_CHEVRON_SIZE : EGUI_VIEW_ACCORDION_STANDARD_CHEVRON_SIZE;
    egui_dim_t stack_h = 0;
    egui_dim_t y;
    uint8_t index;

    egui_view_accordion_clear_metrics(metrics);
    egui_view_get_work_region(self, &metrics->region);
    metrics->content_region.location.x = metrics->region.location.x + pad_x;
    metrics->content_region.location.y = metrics->region.location.y + pad_y;
    metrics->content_region.size.width = metrics->region.size.width - pad_x * 2;
    metrics->content_region.size.height = metrics->region.size.height - pad_y * 2;
    if (metrics->content_region.size.width <= 0 || metrics->content_region.size.height <= 0 || local->item_count == 0)
    {
        return;
    }

    for (index = 0; index < local->item_count; index++)
    {
        stack_h += header_h;
        if (local->expanded_index == index)
        {
            stack_h += body_h;
        }
        if (index + 1 < local->item_count)
        {
            stack_h += gap;
        }
    }
    y = metrics->content_region.location.y;
    if (metrics->content_region.size.height > stack_h)
    {
        y += (metrics->content_region.size.height - stack_h) / 2;
    }

    for (index = 0; index < local->item_count; index++)
    {
        egui_dim_t item_h = header_h;
        egui_dim_t text_x;
        egui_dim_t right_x;
        egui_dim_t meta_w;
        egui_dim_t desc_h = local->compact_mode ? 0 : 10;

        if (local->expanded_index == index)
        {
            item_h += body_h;
        }
        metrics->item_region[index].location.x = metrics->content_region.location.x;
        metrics->item_region[index].location.y = y;
        metrics->item_region[index].size.width = metrics->content_region.size.width;
        metrics->item_region[index].size.height = item_h;

        metrics->header_region[index] = metrics->item_region[index];
        metrics->header_region[index].size.height = header_h;

        metrics->accent_region[index].location.x = metrics->item_region[index].location.x;
        metrics->accent_region[index].location.y = metrics->item_region[index].location.y + 2;
        metrics->accent_region[index].size.width = local->expanded_index == index ? 3 : 2;
        metrics->accent_region[index].size.height = header_h - 4;

        metrics->icon_region[index].location.x = metrics->header_region[index].location.x + (local->compact_mode ? 6 : 8);
        metrics->icon_region[index].location.y = metrics->header_region[index].location.y + (header_h - icon_size) / 2;
        metrics->icon_region[index].size.width = icon_size;
        metrics->icon_region[index].size.height = icon_size;

        metrics->chevron_region[index].location.x = metrics->header_region[index].location.x + metrics->header_region[index].size.width - chevron_size - 8;
        metrics->chevron_region[index].location.y = metrics->header_region[index].location.y + (header_h - chevron_size) / 2;
        metrics->chevron_region[index].size.width = chevron_size;
        metrics->chevron_region[index].size.height = chevron_size;

        meta_w = local->compact_mode ? 0 : 32;
        right_x = metrics->chevron_region[index].location.x - (local->compact_mode ? 5 : 8);
        text_x = metrics->icon_region[index].location.x + icon_size + (local->compact_mode ? 5 : 7);
        metrics->meta_region[index].location.x = right_x - meta_w;
        metrics->meta_region[index].location.y = metrics->header_region[index].location.y + (header_h - 11) / 2;
        metrics->meta_region[index].size.width = meta_w;
        metrics->meta_region[index].size.height = local->compact_mode ? 0 : 11;

        metrics->title_region[index].location.x = text_x;
        metrics->title_region[index].location.y = metrics->header_region[index].location.y + (local->compact_mode ? 5 : 5);
        metrics->title_region[index].size.width = metrics->meta_region[index].size.width > 0 ? metrics->meta_region[index].location.x - text_x - 5 : right_x - text_x;
        metrics->title_region[index].size.height = 11;

        metrics->description_region[index].location.x = text_x;
        metrics->description_region[index].location.y = metrics->title_region[index].location.y + 12;
        metrics->description_region[index].size.width = metrics->title_region[index].size.width;
        metrics->description_region[index].size.height = desc_h;

        if (local->expanded_index == index)
        {
            metrics->body_region[index].location.x = metrics->item_region[index].location.x + (local->compact_mode ? 7 : 9);
            metrics->body_region[index].location.y = metrics->header_region[index].location.y + header_h;
            metrics->body_region[index].size.width = metrics->item_region[index].size.width - (local->compact_mode ? 14 : 18);
            metrics->body_region[index].size.height = body_h - (local->compact_mode ? 3 : 4);
        }
        y += item_h + gap;
    }
}

uint8_t egui_view_accordion_get_item_region(egui_view_t *self, uint8_t item_index, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_accordion_t);
    egui_view_accordion_metrics_t metrics;

    if (region == NULL || !egui_view_accordion_index_is_valid(local, item_index))
    {
        return 0;
    }
    egui_view_accordion_get_metrics(local, self, &metrics);
    *region = metrics.header_region[item_index];
    return metrics.header_region[item_index].size.width > 0 && metrics.header_region[item_index].size.height > 0 ? 1 : 0;
}

static void egui_view_accordion_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                          egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (font == NULL || !egui_view_accordion_has_text(text) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, align, color, self->alpha);
}

static void egui_view_accordion_draw_chevron(egui_view_t *self, const egui_region_t *region, uint8_t expanded, egui_color_t color)
{
    egui_dim_t x = region->location.x + 1;
    egui_dim_t y = region->location.y + 1;
    egui_dim_t w = region->size.width - 2;
    egui_dim_t h = region->size.height - 2;

    if (region->size.width <= 0 || region->size.height <= 0 || w < 3 || h < 3)
    {
        return;
    }
    if (expanded)
    {
        egui_canvas_draw_triangle_fill(&uicode_get_core()->canvas, x, y + 1, x + w, y + 1, x + w / 2, y + h - 1, color,
                                       egui_color_alpha_mix(self->alpha, 92));
    }
    else
    {
        egui_canvas_draw_triangle_fill(&uicode_get_core()->canvas, x + 1, y, x + 1, y + h, x + w - 1, y + h / 2, color,
                                       egui_color_alpha_mix(self->alpha, 92));
    }
}

static void egui_view_accordion_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_accordion_t);
    egui_view_accordion_metrics_t metrics;
    char body_label[52];
    char desc_label[48];
    char meta_label[16];
    char title_label[32];
    uint8_t index;
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_ACCORDION_COMPACT_RADIUS : EGUI_VIEW_ACCORDION_STANDARD_RADIUS;

    egui_view_accordion_sync_indexes(local);
    egui_view_accordion_get_metrics(local, self, &metrics);
    if (metrics.region.size.width <= 0 || metrics.region.size.height <= 0 || local->item_count == 0)
    {
        return;
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.region.location.x, metrics.region.location.y, metrics.region.size.width,
                                          metrics.region.size.height, radius + 2, local->surface_color, egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics.region.location.x, metrics.region.location.y, metrics.region.size.width,
                                     metrics.region.size.height, radius + 2, 1, local->border_color, egui_color_alpha_mix(self->alpha, 55));

    for (index = 0; index < local->item_count; index++)
    {
        const egui_view_accordion_item_t *item = egui_view_accordion_get_item(local, index);
        egui_color_t tone_color;
        egui_color_t section_fill;
        egui_color_t border_color;
        egui_color_t text_color = local->text_color;
        egui_color_t muted_color = local->muted_text_color;
        egui_color_t icon_text = EGUI_COLOR_WHITE;
        uint8_t expanded = local->expanded_index == index ? 1 : 0;
        uint8_t focused = local->focused_index == index ? 1 : 0;
        uint8_t pressed = self->is_pressed && local->pressed_index == index ? 1 : 0;

        if (item == NULL || metrics.item_region[index].size.width <= 0)
        {
            continue;
        }

        tone_color = egui_view_accordion_tone_color(local, item->tone);
        section_fill = expanded ? egui_rgb_mix(local->section_color, tone_color, 5) : local->section_color;
        border_color = focused ? egui_rgb_mix(local->border_color, tone_color, 18) : local->border_color;
        if (!egui_view_get_enable(self))
        {
            tone_color = egui_view_accordion_mix_disabled(tone_color);
            section_fill = egui_view_accordion_mix_disabled(section_fill);
            border_color = egui_view_accordion_mix_disabled(border_color);
            text_color = egui_view_accordion_mix_disabled(text_color);
            muted_color = egui_view_accordion_mix_disabled(muted_color);
            icon_text = egui_view_accordion_mix_disabled(icon_text);
        }
        else if (local->read_only_mode)
        {
            tone_color = egui_rgb_mix(tone_color, local->muted_text_color, 54);
            section_fill = egui_rgb_mix(section_fill, local->surface_color, 24);
            border_color = egui_rgb_mix(border_color, local->muted_text_color, 32);
            text_color = egui_rgb_mix(text_color, local->muted_text_color, 30);
            muted_color = egui_rgb_mix(muted_color, local->text_color, 8);
            icon_text = egui_rgb_mix(icon_text, local->muted_text_color, 28);
        }
        else if (pressed)
        {
            section_fill = egui_rgb_mix(section_fill, tone_color, 7);
            border_color = egui_rgb_mix(border_color, tone_color, 28);
        }

        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.item_region[index].location.x, metrics.item_region[index].location.y,
                                              metrics.item_region[index].size.width, metrics.item_region[index].size.height, radius, section_fill,
                                              egui_color_alpha_mix(self->alpha, 94));
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics.item_region[index].location.x, metrics.item_region[index].location.y,
                                         metrics.item_region[index].size.width, metrics.item_region[index].size.height, radius, 1, border_color,
                                         egui_color_alpha_mix(self->alpha, focused ? 74 : 44));
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.accent_region[index].location.x, metrics.accent_region[index].location.y,
                                              metrics.accent_region[index].size.width, metrics.accent_region[index].size.height, 2, tone_color,
                                              egui_color_alpha_mix(self->alpha, expanded ? 76 : 34));
        egui_canvas_draw_circle_fill(&uicode_get_core()->canvas, metrics.icon_region[index].location.x + metrics.icon_region[index].size.width / 2,
                                     metrics.icon_region[index].location.y + metrics.icon_region[index].size.height / 2,
                                     metrics.icon_region[index].size.width / 2, tone_color, egui_color_alpha_mix(self->alpha, expanded ? 78 : 48));
        egui_view_accordion_copy_elided(meta_label, sizeof(meta_label), item->meta, 2);
        egui_view_accordion_draw_text(local->meta_font, self, meta_label, &metrics.icon_region[index], EGUI_ALIGN_CENTER, icon_text);

        egui_view_accordion_fit_text_to_width(local->font, item->title, title_label, sizeof(title_label), metrics.title_region[index].size.width,
                                              local->compact_mode ? 4 : 5);
        egui_view_accordion_draw_text(local->font, self, title_label, &metrics.title_region[index], EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text_color);

        if (!local->compact_mode)
        {
            egui_view_accordion_fit_text_to_width(local->meta_font, item->description, desc_label, sizeof(desc_label),
                                                  metrics.description_region[index].size.width, 4);
            egui_view_accordion_draw_text(local->meta_font, self, desc_label, &metrics.description_region[index], EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                                          muted_color);

            egui_view_accordion_fit_text_to_width(local->meta_font, item->meta, meta_label, sizeof(meta_label), metrics.meta_region[index].size.width, 4);
            egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.meta_region[index].location.x, metrics.meta_region[index].location.y,
                                                  metrics.meta_region[index].size.width, metrics.meta_region[index].size.height, 5,
                                                  egui_rgb_mix(local->surface_color, tone_color, expanded ? 8 : 3), egui_color_alpha_mix(self->alpha, 58));
            egui_view_accordion_draw_text(local->meta_font, self, meta_label, &metrics.meta_region[index], EGUI_ALIGN_CENTER, muted_color);
        }
        egui_view_accordion_draw_chevron(self, &metrics.chevron_region[index], expanded, tone_color);

        if (expanded && metrics.body_region[index].size.width > 0)
        {
            egui_region_t text_region = metrics.body_region[index];
            egui_color_t body_fill = egui_rgb_mix(local->surface_color, tone_color, local->compact_mode ? 2 : 3);

            egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.body_region[index].location.x, metrics.body_region[index].location.y,
                                                  metrics.body_region[index].size.width, metrics.body_region[index].size.height, local->compact_mode ? 5 : 6,
                                                  body_fill, egui_color_alpha_mix(self->alpha, 88));
            text_region.location.x += local->compact_mode ? 5 : 7;
            text_region.size.width -= local->compact_mode ? 10 : 14;
            egui_view_accordion_fit_text_to_width(local->meta_font, item->body, body_label, sizeof(body_label), text_region.size.width,
                                                  local->compact_mode ? 4 : 5);
            egui_view_accordion_draw_text(local->meta_font, self, body_label, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, muted_color);
        }
    }
}

static uint8_t egui_view_accordion_resolve_hit(egui_view_accordion_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    egui_view_accordion_metrics_t metrics;
    uint8_t index;

    egui_view_accordion_get_metrics(local, self, &metrics);
    for (index = 0; index < local->item_count; index++)
    {
        if (egui_region_pt_in_rect(&metrics.header_region[index], x, y))
        {
            return index;
        }
    }
    return EGUI_VIEW_ACCORDION_INDEX_NONE;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_accordion_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_accordion_t);
    uint8_t hit_index;

    if (!egui_view_accordion_is_interactive(local, self))
    {
        if (egui_view_accordion_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_index = egui_view_accordion_resolve_hit(local, self, event->location.x, event->location.y);
        if (!egui_view_accordion_index_is_valid(local, hit_index))
        {
            return 0;
        }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        local->pressed_index = hit_index;
        local->focused_index = hit_index;
        egui_view_set_pressed(self, true);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_index == EGUI_VIEW_ACCORDION_INDEX_NONE)
        {
            return 0;
        }
        hit_index = egui_view_accordion_resolve_hit(local, self, event->location.x, event->location.y);
        egui_view_set_pressed(self, hit_index == local->pressed_index);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
    {
        uint8_t handled;

        hit_index = egui_view_accordion_resolve_hit(local, self, event->location.x, event->location.y);
        if (self->is_pressed && hit_index == local->pressed_index && egui_view_accordion_index_is_valid(local, hit_index))
        {
            local->focused_index = hit_index;
            egui_view_accordion_activate_focused(self);
        }
        handled = egui_view_accordion_clear_pressed_state(self, local);
        if (handled)
        {
            egui_view_invalidate(self);
        }
        return handled || hit_index != EGUI_VIEW_ACCORDION_INDEX_NONE;
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (egui_view_accordion_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}

static int egui_view_accordion_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_accordion_t);
    EGUI_UNUSED(event);

    if (egui_view_accordion_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_accordion_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_accordion_t);

    if (!egui_view_accordion_is_interactive(local, self))
    {
        if (egui_view_accordion_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    egui_view_accordion_sync_indexes(local);
    if (event->key_code == EGUI_KEY_CODE_ENTER || event->key_code == EGUI_KEY_CODE_SPACE)
    {
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->pressed_index = local->focused_index;
            egui_view_set_pressed(self, true);
            egui_view_invalidate(self);
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            uint8_t handled = 0;

            if (local->pressed_index == local->focused_index)
            {
                handled = egui_view_accordion_activate_focused(self);
            }
            if (egui_view_accordion_clear_pressed_state(self, local))
            {
                egui_view_invalidate(self);
            }
            return handled;
        }
    }

    if (egui_view_accordion_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    if (event->type != EGUI_KEY_EVENT_ACTION_UP)
    {
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_UP:
        if (local->focused_index > 0 && local->focused_index != EGUI_VIEW_ACCORDION_INDEX_NONE)
        {
            local->focused_index--;
            egui_view_invalidate(self);
        }
        return 1;
    case EGUI_KEY_CODE_DOWN:
    case EGUI_KEY_CODE_TAB:
        if (local->focused_index + 1 < local->item_count)
        {
            local->focused_index++;
            egui_view_invalidate(self);
        }
        return 1;
    case EGUI_KEY_CODE_HOME:
        local->focused_index = 0;
        egui_view_invalidate(self);
        return 1;
    case EGUI_KEY_CODE_END:
        local->focused_index = local->item_count - 1;
        egui_view_invalidate(self);
        return 1;
    case EGUI_KEY_CODE_ESCAPE:
        if (local->expanded_index != EGUI_VIEW_ACCORDION_INDEX_NONE)
        {
            local->expanded_index = EGUI_VIEW_ACCORDION_INDEX_NONE;
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_accordion_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_accordion_t);
    EGUI_UNUSED(event);

    if (egui_view_accordion_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_accordion_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_accordion_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_accordion_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_accordion_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_accordion_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_accordion_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_accordion_on_key_event,
#endif
};

void egui_view_accordion_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_accordion_t);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_accordion_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif

    local->items = NULL;
    local->on_action = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->section_color = EGUI_COLOR_HEX(0xF7F9FC);
    local->border_color = EGUI_COLOR_HEX(0xD3DCE5);
    local->text_color = EGUI_COLOR_HEX(0x182331);
    local->muted_text_color = EGUI_COLOR_HEX(0x667789);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->success_color = EGUI_COLOR_HEX(0x107C41);
    local->warning_color = EGUI_COLOR_HEX(0x9A6400);
    local->neutral_color = EGUI_COLOR_HEX(0x687484);
    local->item_count = 0;
    local->expanded_index = EGUI_VIEW_ACCORDION_INDEX_NONE;
    local->focused_index = EGUI_VIEW_ACCORDION_INDEX_NONE;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_index = EGUI_VIEW_ACCORDION_INDEX_NONE;
    egui_view_set_view_name(self, "egui_view_accordion");
}
