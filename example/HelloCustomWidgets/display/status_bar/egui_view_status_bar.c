#include "egui_view_status_bar.h"

#include <string.h>

#define EGUI_VIEW_STATUS_BAR_LABEL_MAX 15
#define EGUI_VIEW_STATUS_BAR_VALUE_MAX 23
#define EGUI_VIEW_STATUS_BAR_RADIUS    10
#define EGUI_VIEW_STATUS_BAR_DOT_SIZE  5

typedef struct egui_view_status_bar_metrics egui_view_status_bar_metrics_t;
struct egui_view_status_bar_metrics
{
    egui_region_t region;
    egui_region_t item_regions[EGUI_VIEW_STATUS_BAR_MAX_ITEMS];
};

static egui_view_status_bar_t *egui_view_status_bar_local(egui_view_t *self)
{
    return (egui_view_status_bar_t *)self;
}

static uint8_t egui_view_status_bar_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static uint8_t egui_view_status_bar_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static uint8_t egui_view_status_bar_text_len(const char *text)
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

static void egui_view_status_bar_copy_elided(char *buffer, uint8_t buffer_size, const char *text, uint8_t max_chars)
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

    length = egui_view_status_bar_text_len(text);
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

static egui_dim_t egui_view_status_bar_measure_text_width(const egui_font_t *font, const char *text)
{
    egui_dim_t width = 0;
    egui_dim_t height = 0;

    if (!egui_view_status_bar_has_text(text) || font == NULL || font->api == NULL || font->api->get_str_size == NULL)
    {
        return 0;
    }
    font->api->get_str_size(font, text, 0, 0, &width, &height);
    return width;
}

static void egui_view_status_bar_fit_text_to_width(const egui_font_t *font, const char *text, char *buffer, uint8_t buffer_size,
                                                   egui_dim_t max_width, egui_dim_t fallback_char_width)
{
    uint8_t max_chars;

    if (buffer == NULL || buffer_size == 0)
    {
        return;
    }
    buffer[0] = '\0';
    if (!egui_view_status_bar_has_text(text) || max_width <= 0)
    {
        return;
    }

    max_chars = egui_view_status_bar_text_len(text);
    egui_view_status_bar_copy_elided(buffer, buffer_size, text, max_chars);
    while (max_chars > 0)
    {
        egui_dim_t width = egui_view_status_bar_measure_text_width(font, buffer);

        if (width <= 0)
        {
            width = (egui_dim_t)egui_view_status_bar_text_len(buffer) * fallback_char_width;
        }
        if (width <= max_width)
        {
            break;
        }
        max_chars--;
        egui_view_status_bar_copy_elided(buffer, buffer_size, text, max_chars);
    }
}

static egui_color_t egui_view_status_bar_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x8A97A5), 58);
}

static uint8_t egui_view_status_bar_normalize_state(uint8_t state)
{
    if (state > EGUI_VIEW_STATUS_BAR_STATE_WARN)
    {
        return EGUI_VIEW_STATUS_BAR_STATE_NORMAL;
    }
    return state;
}

static void egui_view_status_bar_reset_items(egui_view_status_bar_t *local)
{
    uint8_t index;

    for (index = 0; index < EGUI_VIEW_STATUS_BAR_MAX_ITEMS; ++index)
    {
        local->items[index].label = "";
        local->items[index].value = "";
        local->items[index].weight = 1;
        local->items[index].state = EGUI_VIEW_STATUS_BAR_STATE_NORMAL;
        local->items[index].emphasized = 0;
    }
}

static void egui_view_status_bar_copy_item(egui_view_status_bar_item_t *dst, const egui_view_status_bar_item_t *src)
{
    if (dst == NULL)
    {
        return;
    }

    if (src == NULL)
    {
        dst->label = "";
        dst->value = "";
        dst->weight = 1;
        dst->state = EGUI_VIEW_STATUS_BAR_STATE_NORMAL;
        dst->emphasized = 0;
        return;
    }

    dst->label = src->label == NULL ? "" : src->label;
    dst->value = src->value == NULL ? "" : src->value;
    dst->weight = src->weight == 0 ? 1 : src->weight;
    dst->state = egui_view_status_bar_normalize_state(src->state);
    dst->emphasized = src->emphasized ? 1 : 0;
}

static uint8_t egui_view_status_bar_total_weight(egui_view_status_bar_t *local)
{
    uint8_t total = 0;
    uint8_t index;

    for (index = 0; index < local->item_count; ++index)
    {
        total = (uint8_t)(total + (local->items[index].weight == 0 ? 1 : local->items[index].weight));
    }
    return total == 0 ? 1 : total;
}

static void egui_view_status_bar_get_metrics(egui_view_t *self, egui_view_status_bar_t *local, egui_view_status_bar_metrics_t *metrics)
{
    egui_dim_t pad_x = local->compact_mode ? 4 : 6;
    egui_dim_t pad_y = local->compact_mode ? 4 : 5;
    egui_dim_t gap = local->compact_mode ? 3 : 5;
    egui_dim_t inner_width;
    egui_dim_t cursor_x;
    egui_dim_t remaining_width;
    uint8_t remaining_weight;
    uint8_t index;

    egui_view_get_work_region(self, &metrics->region);
    for (index = 0; index < EGUI_VIEW_STATUS_BAR_MAX_ITEMS; ++index)
    {
        metrics->item_regions[index] = metrics->region;
        metrics->item_regions[index].size.width = 0;
        metrics->item_regions[index].size.height = 0;
    }

    if (metrics->region.size.width <= pad_x * 2 || metrics->region.size.height <= pad_y * 2 || local->item_count == 0)
    {
        return;
    }

    inner_width = metrics->region.size.width - pad_x * 2 - gap * (egui_dim_t)(local->item_count - 1U);
    if (inner_width <= 0)
    {
        return;
    }

    cursor_x = metrics->region.location.x + pad_x;
    remaining_width = inner_width;
    remaining_weight = egui_view_status_bar_total_weight(local);
    for (index = 0; index < local->item_count; ++index)
    {
        egui_dim_t item_width;
        uint8_t weight = local->items[index].weight == 0 ? 1 : local->items[index].weight;

        if (index == local->item_count - 1U || remaining_weight <= weight)
        {
            item_width = remaining_width;
        }
        else
        {
            item_width = (egui_dim_t)((int32_t)remaining_width * weight / remaining_weight);
            if (item_width < 12)
            {
                item_width = 12;
            }
        }

        metrics->item_regions[index].location.x = cursor_x;
        metrics->item_regions[index].location.y = metrics->region.location.y + pad_y;
        metrics->item_regions[index].size.width = item_width;
        metrics->item_regions[index].size.height = metrics->region.size.height - pad_y * 2;

        cursor_x += item_width + gap;
        remaining_width -= item_width;
        if (index + 1U < local->item_count)
        {
            remaining_width -= gap;
        }
        remaining_weight = remaining_weight > weight ? (uint8_t)(remaining_weight - weight) : 1;
    }
}

static egui_color_t egui_view_status_bar_state_color(egui_view_status_bar_t *local, uint8_t state)
{
    switch (egui_view_status_bar_normalize_state(state))
    {
    case EGUI_VIEW_STATUS_BAR_STATE_INFO:
        return local->accent_color;
    case EGUI_VIEW_STATUS_BAR_STATE_OK:
        return local->ok_color;
    case EGUI_VIEW_STATUS_BAR_STATE_WARN:
        return local->warn_color;
    default:
        return local->muted_text_color;
    }
}

static void egui_view_status_bar_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region,
                                           uint8_t align, egui_color_t color, egui_alpha_t alpha)
{
    egui_region_t draw_region = *region;

    if (font == NULL || !egui_view_status_bar_has_text(text) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, align, color, egui_color_alpha_mix(self->alpha, alpha));
}

static void egui_view_status_bar_draw_item(egui_view_t *self, egui_view_status_bar_t *local, const egui_view_status_bar_metrics_t *metrics,
                                           uint8_t index)
{
    egui_view_status_bar_item_t *item = &local->items[index];
    egui_region_t item_region = metrics->item_regions[index];
    egui_region_t dot_region;
    egui_region_t label_region;
    egui_region_t value_region;
    egui_color_t state_color = egui_view_status_bar_state_color(local, item->state);
    egui_color_t label_color = local->muted_text_color;
    egui_color_t value_color = local->text_color;
    egui_dim_t dot_size = local->compact_mode ? 4 : EGUI_VIEW_STATUS_BAR_DOT_SIZE;
    egui_dim_t text_x;
    egui_dim_t label_width;
    char label[EGUI_VIEW_STATUS_BAR_LABEL_MAX + 1];
    char value[EGUI_VIEW_STATUS_BAR_VALUE_MAX + 1];

    if (item_region.size.width <= 0 || item_region.size.height <= 0)
    {
        return;
    }

    if (local->read_only_mode)
    {
        state_color = egui_rgb_mix(state_color, EGUI_COLOR_HEX(0x8F9CAA), 48);
        label_color = egui_rgb_mix(label_color, EGUI_COLOR_HEX(0x8F9CAA), 36);
        value_color = egui_rgb_mix(value_color, EGUI_COLOR_HEX(0x8F9CAA), 34);
    }
    if (!egui_view_get_enable(self))
    {
        state_color = egui_view_status_bar_mix_disabled(state_color);
        label_color = egui_view_status_bar_mix_disabled(label_color);
        value_color = egui_view_status_bar_mix_disabled(value_color);
    }

    if (item->emphasized && !local->read_only_mode)
    {
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, item_region.location.x, item_region.location.y, item_region.size.width,
                                              item_region.size.height, local->compact_mode ? 7 : 9,
                                              egui_rgb_mix(local->surface_color, state_color, local->compact_mode ? 8 : 10),
                                              egui_color_alpha_mix(self->alpha, 78));
    }

    dot_region.location.x = item_region.location.x + (local->compact_mode ? 4 : 5);
    dot_region.location.y = item_region.location.y + (item_region.size.height - dot_size) / 2;
    dot_region.size.width = dot_size;
    dot_region.size.height = dot_size;
    egui_canvas_draw_circle_fill_basic(&uicode_get_core()->canvas, dot_region.location.x + dot_size / 2, dot_region.location.y + dot_size / 2,
                                       EGUI_MAX(dot_size / 2, 2), state_color, egui_color_alpha_mix(self->alpha, local->read_only_mode ? 70 : 92));

    text_x = dot_region.location.x + dot_size + (local->compact_mode ? 4 : 5);
    label_region.location.x = text_x;
    label_region.location.y = item_region.location.y;
    label_region.size.height = item_region.size.height;
    value_region = label_region;

    if (local->compact_mode || item_region.size.width < 64 || !egui_view_status_bar_has_text(item->label))
    {
        value_region.size.width = item_region.location.x + item_region.size.width - text_x - 4;
        egui_view_status_bar_fit_text_to_width(local->value_font, item->value, value, sizeof(value), value_region.size.width, 5);
        egui_view_status_bar_draw_text(local->value_font, self, value, &value_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, value_color, EGUI_ALPHA_100);
        return;
    }

    label_width = item_region.size.width / 3;
    if (label_width < 24)
    {
        label_width = 24;
    }
    if (label_width > 42)
    {
        label_width = 42;
    }
    label_region.size.width = label_width;
    value_region.location.x = label_region.location.x + label_width + 3;
    value_region.size.width = item_region.location.x + item_region.size.width - value_region.location.x - 4;

    egui_view_status_bar_fit_text_to_width(local->label_font, item->label, label, sizeof(label), label_region.size.width, 4);
    egui_view_status_bar_fit_text_to_width(local->value_font, item->value, value, sizeof(value), value_region.size.width, 5);
    egui_view_status_bar_draw_text(local->label_font, self, label, &label_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, label_color, EGUI_ALPHA_100);
    egui_view_status_bar_draw_text(local->value_font, self, value, &value_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, value_color, EGUI_ALPHA_100);
}

static void egui_view_status_bar_on_draw(egui_view_t *self)
{
    egui_view_status_bar_t *local = egui_view_status_bar_local(self);
    egui_view_status_bar_metrics_t metrics;
    egui_color_t surface = local->surface_color;
    egui_color_t border = local->border_color;
    egui_color_t separator = local->separator_color;
    uint8_t index;

    egui_view_status_bar_get_metrics(self, local, &metrics);
    if (metrics.region.size.width <= 0 || metrics.region.size.height <= 0)
    {
        return;
    }

    if (local->read_only_mode)
    {
        surface = egui_rgb_mix(surface, EGUI_COLOR_HEX(0xF4F7FA), 44);
        border = egui_rgb_mix(border, EGUI_COLOR_HEX(0xAAB5C0), 42);
        separator = egui_rgb_mix(separator, EGUI_COLOR_HEX(0xB6C0CA), 50);
    }
    if (!egui_view_get_enable(self))
    {
        surface = egui_view_status_bar_mix_disabled(surface);
        border = egui_view_status_bar_mix_disabled(border);
        separator = egui_view_status_bar_mix_disabled(separator);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.region.location.x, metrics.region.location.y, metrics.region.size.width,
                                          metrics.region.size.height, local->compact_mode ? 8 : EGUI_VIEW_STATUS_BAR_RADIUS, surface,
                                          egui_color_alpha_mix(self->alpha, 98));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics.region.location.x, metrics.region.location.y, metrics.region.size.width,
                                     metrics.region.size.height, local->compact_mode ? 8 : EGUI_VIEW_STATUS_BAR_RADIUS, 1, border,
                                     egui_color_alpha_mix(self->alpha, local->read_only_mode ? 44 : 58));
    if (!local->read_only_mode && metrics.region.size.width > 22)
    {
        egui_canvas_draw_rectangle_fill(&uicode_get_core()->canvas, metrics.region.location.x + 9, metrics.region.location.y + 5,
                                        local->compact_mode ? 18 : 28, 2, local->accent_color, egui_color_alpha_mix(self->alpha, 46));
    }

    for (index = 0; index < local->item_count; ++index)
    {
        if (index > 0 && metrics.item_regions[index].size.height > 4)
        {
            egui_dim_t x = metrics.item_regions[index].location.x - (local->compact_mode ? 2 : 3);

            egui_canvas_draw_rectangle_fill(&uicode_get_core()->canvas, x, metrics.item_regions[index].location.y + 3, 1,
                                            metrics.item_regions[index].size.height - 6, separator, egui_color_alpha_mix(self->alpha, 48));
        }
        egui_view_status_bar_draw_item(self, local, &metrics, index);
    }
}

void egui_view_status_bar_set_items(egui_view_t *self, const egui_view_status_bar_item_t *items, uint8_t item_count)
{
    egui_view_status_bar_t *local = egui_view_status_bar_local(self);
    uint8_t count = items == NULL ? 0 : item_count;
    uint8_t index;

    egui_view_status_bar_clear_pressed_state(self);
    if (count > EGUI_VIEW_STATUS_BAR_MAX_ITEMS)
    {
        count = EGUI_VIEW_STATUS_BAR_MAX_ITEMS;
    }
    egui_view_status_bar_reset_items(local);
    for (index = 0; index < count; ++index)
    {
        egui_view_status_bar_copy_item(&local->items[index], &items[index]);
    }
    local->item_count = count;
    egui_view_invalidate(self);
}

void egui_view_status_bar_set_item(egui_view_t *self, uint8_t index, const egui_view_status_bar_item_t *item)
{
    egui_view_status_bar_t *local = egui_view_status_bar_local(self);

    if (index >= EGUI_VIEW_STATUS_BAR_MAX_ITEMS)
    {
        return;
    }

    egui_view_status_bar_clear_pressed_state(self);
    egui_view_status_bar_copy_item(&local->items[index], item);
    if (index >= local->item_count)
    {
        local->item_count = (uint8_t)(index + 1U);
    }
    egui_view_invalidate(self);
}

void egui_view_status_bar_set_fonts(egui_view_t *self, const egui_font_t *label_font, const egui_font_t *value_font)
{
    egui_view_status_bar_t *local = egui_view_status_bar_local(self);

    egui_view_status_bar_clear_pressed_state(self);
    local->label_font = label_font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : label_font;
    local->value_font = value_font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : value_font;
    egui_view_invalidate(self);
}

void egui_view_status_bar_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color,
                                      egui_color_t separator_color, egui_color_t text_color, egui_color_t muted_text_color,
                                      egui_color_t accent_color, egui_color_t ok_color, egui_color_t warn_color)
{
    egui_view_status_bar_t *local = egui_view_status_bar_local(self);

    egui_view_status_bar_clear_pressed_state(self);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->separator_color = separator_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->ok_color = ok_color;
    local->warn_color = warn_color;
    egui_view_invalidate(self);
}

void egui_view_status_bar_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    egui_view_status_bar_t *local = egui_view_status_bar_local(self);

    egui_view_status_bar_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_status_bar_get_compact_mode(egui_view_t *self)
{
    egui_view_status_bar_t *local = egui_view_status_bar_local(self);

    return local->compact_mode;
}

void egui_view_status_bar_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    egui_view_status_bar_t *local = egui_view_status_bar_local(self);

    egui_view_status_bar_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_status_bar_get_read_only_mode(egui_view_t *self)
{
    egui_view_status_bar_t *local = egui_view_status_bar_local(self);

    return local->read_only_mode;
}

void egui_view_status_bar_apply_standard_style(egui_view_t *self)
{
    egui_view_status_bar_set_palette(self, EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xCCD6E0), EGUI_COLOR_HEX(0xDCE4EC),
                                     EGUI_COLOR_HEX(0x1D2A36), EGUI_COLOR_HEX(0x637283), EGUI_COLOR_HEX(0x0F6CBD),
                                     EGUI_COLOR_HEX(0x107C41), EGUI_COLOR_HEX(0xB26A00));
    egui_view_status_bar_set_compact_mode(self, 0);
    egui_view_status_bar_set_read_only_mode(self, 0);
}

void egui_view_status_bar_apply_accent_style(egui_view_t *self)
{
    egui_view_status_bar_set_palette(self, EGUI_COLOR_HEX(0xF7FBFF), EGUI_COLOR_HEX(0xB9D6F0), EGUI_COLOR_HEX(0xCDE0F2),
                                     EGUI_COLOR_HEX(0x173247), EGUI_COLOR_HEX(0x5D7183), EGUI_COLOR_HEX(0x0F6CBD),
                                     EGUI_COLOR_HEX(0x0F7B45), EGUI_COLOR_HEX(0xA15C00));
    egui_view_status_bar_set_compact_mode(self, 0);
    egui_view_status_bar_set_read_only_mode(self, 0);
}

void egui_view_status_bar_apply_compact_style(egui_view_t *self)
{
    egui_view_status_bar_set_palette(self, EGUI_COLOR_HEX(0xF8FBFD), EGUI_COLOR_HEX(0xD2DCE6), EGUI_COLOR_HEX(0xDFE7EF),
                                     EGUI_COLOR_HEX(0x21313E), EGUI_COLOR_HEX(0x6E7E8E), EGUI_COLOR_HEX(0x0C7C73),
                                     EGUI_COLOR_HEX(0x107C41), EGUI_COLOR_HEX(0xA15C00));
    egui_view_status_bar_set_compact_mode(self, 1);
    egui_view_status_bar_set_read_only_mode(self, 0);
}

void egui_view_status_bar_apply_read_only_style(egui_view_t *self)
{
    egui_view_status_bar_set_palette(self, EGUI_COLOR_HEX(0xF5F7FA), EGUI_COLOR_HEX(0xD7DEE6), EGUI_COLOR_HEX(0xE1E7ED),
                                     EGUI_COLOR_HEX(0x687684), EGUI_COLOR_HEX(0x8B98A5), EGUI_COLOR_HEX(0x788593),
                                     EGUI_COLOR_HEX(0x768777), EGUI_COLOR_HEX(0x92765F));
    egui_view_status_bar_set_compact_mode(self, 1);
    egui_view_status_bar_set_read_only_mode(self, 1);
}

uint8_t egui_view_status_bar_get_item_region(egui_view_t *self, uint8_t index, egui_region_t *region)
{
    egui_view_status_bar_t *local = egui_view_status_bar_local(self);
    egui_view_status_bar_metrics_t metrics;

    if (region == NULL || index >= EGUI_VIEW_STATUS_BAR_MAX_ITEMS || index >= local->item_count)
    {
        return 0;
    }

    egui_view_status_bar_get_metrics(self, local, &metrics);
    *region = metrics.item_regions[index];
    region->location.x += self->region_screen.location.x;
    region->location.y += self->region_screen.location.y;
    return region->size.width > 0 && region->size.height > 0 ? 1 : 0;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_status_bar_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_status_bar_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_status_bar_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_status_bar_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_status_bar_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_status_bar_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_status_bar_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_status_bar_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_status_bar_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_status_bar_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_status_bar_on_static_key_event;
#endif
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_status_bar_t) = {
        .draw = egui_view_draw,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_status_bar_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .on_draw = egui_view_status_bar_on_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_status_bar_on_key_event,
#endif
};

void egui_view_status_bar_init(egui_view_t *self)
{
    egui_view_status_bar_t *local = egui_view_status_bar_local(self);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_status_bar_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->label_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->value_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->item_count = 0;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    egui_view_status_bar_reset_items(local);
    egui_view_status_bar_apply_standard_style(self);
    egui_view_set_view_name(self, "egui_view_status_bar");
}
