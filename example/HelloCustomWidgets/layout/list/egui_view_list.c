#include "egui_view_list.h"
#include "../../hcw_selection_marker.h"

#define EGUI_VIEW_REFERENCE_LIST_STANDARD_PAD_X   8
#define EGUI_VIEW_REFERENCE_LIST_STANDARD_PAD_Y   7
#define EGUI_VIEW_REFERENCE_LIST_STANDARD_RADIUS  10
#define EGUI_VIEW_REFERENCE_LIST_STANDARD_ROW_H   18
#define EGUI_VIEW_REFERENCE_LIST_STANDARD_ROW_GAP 3

#define EGUI_VIEW_REFERENCE_LIST_COMPACT_PAD_X   6
#define EGUI_VIEW_REFERENCE_LIST_COMPACT_PAD_Y   5
#define EGUI_VIEW_REFERENCE_LIST_COMPACT_RADIUS  8
#define EGUI_VIEW_REFERENCE_LIST_COMPACT_ROW_H   10
#define EGUI_VIEW_REFERENCE_LIST_COMPACT_ROW_GAP 2

typedef struct egui_view_reference_list_metrics egui_view_reference_list_metrics_t;
struct egui_view_reference_list_metrics
{
    egui_region_t content_region;
    egui_region_t item_regions[EGUI_VIEW_REFERENCE_LIST_MAX_ITEMS];
};

static uint8_t egui_view_reference_list_clamp_item_count(uint8_t count)
{
    if (count > EGUI_VIEW_REFERENCE_LIST_MAX_ITEMS)
    {
        return EGUI_VIEW_REFERENCE_LIST_MAX_ITEMS;
    }
    return count;
}

static uint8_t egui_view_reference_list_text_len(const char *text)
{
    uint8_t len = 0;

    if (text == NULL)
    {
        return 0;
    }
    while (text[len] != '\0')
    {
        len++;
    }
    return len;
}

static egui_dim_t egui_view_reference_list_measure_font_line_height(const egui_font_t *font)
{
    egui_dim_t dummy_width = 0;
    egui_dim_t line_height = 0;

    if (font == NULL || font->api == NULL || font->api->get_str_size == NULL)
    {
        return 0;
    }

    font->api->get_str_size(font, "A", 0, 0, &dummy_width, &line_height);
    return line_height;
}

static egui_dim_t egui_view_reference_list_measure_text_width(const egui_font_t *font, const char *text)
{
    egui_dim_t text_width = 0;
    egui_dim_t dummy_height = 0;

    if (text == NULL || text[0] == '\0' || font == NULL || font->api == NULL || font->api->get_str_size == NULL)
    {
        return 0;
    }

    font->api->get_str_size(font, text, 0, 0, &text_width, &dummy_height);
    return text_width;
}

static void egui_view_reference_list_copy_elided(char *buffer, uint8_t buffer_size, const char *text, uint8_t max_chars)
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

    length = egui_view_reference_list_text_len(text);
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

    copy_length = max_chars - 3;
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

static void egui_view_reference_list_fit_text_to_width(const egui_font_t *font, const char *text, char *buffer, uint8_t buffer_size, egui_dim_t max_width,
                                                       egui_dim_t fallback_char_width)
{
    uint8_t max_chars;

    if (buffer == NULL || buffer_size == 0)
    {
        return;
    }

    buffer[0] = '\0';
    if (text == NULL || text[0] == '\0' || max_width <= 0)
    {
        return;
    }

    max_chars = egui_view_reference_list_text_len(text);
    egui_view_reference_list_copy_elided(buffer, buffer_size, text, max_chars);
    while (max_chars > 0)
    {
        egui_dim_t text_width = egui_view_reference_list_measure_text_width(font, buffer);

        if (text_width <= 0)
        {
            text_width = (egui_dim_t)egui_view_reference_list_text_len(buffer) * fallback_char_width;
        }
        if (text_width <= max_width)
        {
            break;
        }

        max_chars--;
        egui_view_reference_list_copy_elided(buffer, buffer_size, text, max_chars);
    }
}

static egui_dim_t egui_view_reference_list_get_title_line_height(egui_view_reference_list_t *local)
{
    egui_dim_t line_height = egui_view_reference_list_measure_font_line_height(local->font);
    egui_dim_t fallback = local->compact_mode ? EGUI_VIEW_REFERENCE_LIST_COMPACT_ROW_H : EGUI_VIEW_REFERENCE_LIST_STANDARD_ROW_H;

    return line_height > 0 ? line_height : fallback;
}

static egui_dim_t egui_view_reference_list_get_meta_line_height(egui_view_reference_list_t *local)
{
    egui_dim_t line_height = egui_view_reference_list_measure_font_line_height(local->meta_font);
    egui_dim_t fallback = local->compact_mode ? EGUI_VIEW_REFERENCE_LIST_COMPACT_ROW_H : EGUI_VIEW_REFERENCE_LIST_STANDARD_ROW_H;

    return line_height > 0 ? line_height : fallback;
}

static egui_dim_t egui_view_reference_list_get_badge_height(egui_view_reference_list_t *local)
{
    egui_dim_t badge_h = egui_view_reference_list_get_meta_line_height(local);
    egui_dim_t min_h = local->compact_mode ? 10 : 12;

    return badge_h > min_h ? badge_h : min_h;
}

static egui_dim_t egui_view_reference_list_get_row_height(egui_view_reference_list_t *local)
{
    egui_dim_t title_h = egui_view_reference_list_get_title_line_height(local);
    egui_dim_t meta_h = egui_view_reference_list_get_meta_line_height(local);
    egui_dim_t row_h;
    egui_dim_t min_h = local->compact_mode ? EGUI_VIEW_REFERENCE_LIST_COMPACT_ROW_H : EGUI_VIEW_REFERENCE_LIST_STANDARD_ROW_H;

    if (local->compact_mode)
    {
        row_h = title_h > meta_h ? title_h : meta_h;
        meta_h = egui_view_reference_list_get_badge_height(local);
        if (meta_h > row_h)
        {
            row_h = meta_h;
        }
    }
    else
    {
        row_h = title_h + meta_h;
    }

    return row_h > min_h ? row_h : min_h;
}

static egui_color_t egui_view_reference_list_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(44));
}

static uint8_t egui_view_reference_list_clear_pressed_state(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_reference_list_t);
    uint8_t had_pressed = self->is_pressed || local->pressed_index != EGUI_VIEW_REFERENCE_LIST_INDEX_NONE;

    local->pressed_index = EGUI_VIEW_REFERENCE_LIST_INDEX_NONE;
    egui_view_set_pressed(self, false);
    return had_pressed;
}

static egui_color_t egui_view_reference_list_tone_color(egui_view_reference_list_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_REFERENCE_LIST_TONE_SUCCESS:
        return local->success_color;
    case EGUI_VIEW_REFERENCE_LIST_TONE_WARNING:
        return local->warning_color;
    case EGUI_VIEW_REFERENCE_LIST_TONE_NEUTRAL:
        return local->neutral_color;
    default:
        return local->accent_color;
    }
}

static const egui_view_reference_list_item_t *egui_view_reference_list_get_item(egui_view_reference_list_t *local, uint8_t index)
{
    if (local->items == NULL || local->item_count == 0 || index >= local->item_count)
    {
        return NULL;
    }
    return &local->items[index];
}

static void egui_view_reference_list_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                               egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (text == NULL || text[0] == '\0')
    {
        return;
    }
    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, align, color, self->alpha);
}

static egui_dim_t egui_view_reference_list_badge_width(const egui_font_t *font, const char *text, uint8_t compact_mode, egui_dim_t max_w)
{
    egui_dim_t width = compact_mode ? 14 : 18;
    egui_dim_t min_w = compact_mode ? 18 : 22;

    if (text == NULL || text[0] == '\0')
    {
        return 0;
    }
    width += egui_view_reference_list_measure_text_width(font, text);
    if (width <= (compact_mode ? 14 : 18))
    {
        width = (compact_mode ? 14 : 18) + egui_view_reference_list_text_len(text) * (compact_mode ? 4 : 5);
    }
    if (width < min_w)
    {
        width = min_w;
    }
    if (width > max_w)
    {
        width = max_w;
    }
    return width;
}

static void egui_view_reference_list_notify_change(egui_view_t *self, egui_view_reference_list_t *local)
{
    if (local->on_selection_changed != NULL)
    {
        local->on_selection_changed(self, local->current_index);
    }
}

static void egui_view_reference_list_set_current_index_inner(egui_view_t *self, uint8_t index, uint8_t notify);

static void egui_view_reference_list_get_metrics(egui_view_reference_list_t *local, egui_view_t *self,
                                                 egui_view_reference_list_metrics_t *metrics)
{
    egui_region_t work_region;
    egui_dim_t pad_x = local->compact_mode ? EGUI_VIEW_REFERENCE_LIST_COMPACT_PAD_X : EGUI_VIEW_REFERENCE_LIST_STANDARD_PAD_X;
    egui_dim_t pad_y = local->compact_mode ? EGUI_VIEW_REFERENCE_LIST_COMPACT_PAD_Y : EGUI_VIEW_REFERENCE_LIST_STANDARD_PAD_Y;
    egui_dim_t row_h = egui_view_reference_list_get_row_height(local);
    egui_dim_t row_gap = local->compact_mode ? EGUI_VIEW_REFERENCE_LIST_COMPACT_ROW_GAP : EGUI_VIEW_REFERENCE_LIST_STANDARD_ROW_GAP;
    egui_dim_t total_h = 0;
    egui_dim_t start_y;
    uint8_t item_count = egui_view_reference_list_clamp_item_count(local->item_count);
    uint8_t i;

    egui_view_get_work_region(self, &work_region);
    metrics->content_region.location.x = work_region.location.x + pad_x;
    metrics->content_region.location.y = work_region.location.y + pad_y;
    metrics->content_region.size.width = work_region.size.width - pad_x * 2;
    metrics->content_region.size.height = work_region.size.height - pad_y * 2;

    for (i = 0; i < EGUI_VIEW_REFERENCE_LIST_MAX_ITEMS; i++)
    {
        metrics->item_regions[i].location.x = 0;
        metrics->item_regions[i].location.y = 0;
        metrics->item_regions[i].size.width = 0;
        metrics->item_regions[i].size.height = 0;
    }

    if (metrics->content_region.size.width <= 0 || metrics->content_region.size.height <= 0 || item_count == 0)
    {
        return;
    }

    total_h = item_count * row_h + (item_count > 0 ? (item_count - 1) * row_gap : 0);
    if (total_h > metrics->content_region.size.height)
    {
        row_gap = 1;
        total_h = item_count * row_h + (item_count > 0 ? (item_count - 1) * row_gap : 0);
    }
    if (total_h > metrics->content_region.size.height)
    {
        row_gap = 0;
        total_h = item_count * row_h + (item_count > 0 ? (item_count - 1) * row_gap : 0);
    }

    start_y = metrics->content_region.location.y;
    if (metrics->content_region.size.height > total_h)
    {
        start_y += (metrics->content_region.size.height - total_h) / 2;
    }

    for (i = 0; i < item_count; i++)
    {
        metrics->item_regions[i].location.x = metrics->content_region.location.x;
        metrics->item_regions[i].location.y = start_y + i * (row_h + row_gap);
        metrics->item_regions[i].size.width = metrics->content_region.size.width;
        metrics->item_regions[i].size.height = row_h;
    }
}

static uint8_t egui_view_reference_list_hit_index(egui_view_reference_list_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    egui_view_reference_list_metrics_t metrics;
    uint8_t item_count = egui_view_reference_list_clamp_item_count(local->item_count);
    uint8_t i;

    if (item_count == 0 || local->items == NULL)
    {
        return EGUI_VIEW_REFERENCE_LIST_INDEX_NONE;
    }

    egui_view_reference_list_get_metrics(local, self, &metrics);
    for (i = 0; i < item_count; i++)
    {
        if (egui_region_pt_in_rect(&metrics.item_regions[i], x, y))
        {
            return i;
        }
    }
    return EGUI_VIEW_REFERENCE_LIST_INDEX_NONE;
}

void egui_view_reference_list_set_items(egui_view_t *self, const egui_view_reference_list_item_t *items, uint8_t item_count)
{
    EGUI_LOCAL_INIT(egui_view_reference_list_t);

    local->items = items;
    local->item_count = items != NULL ? egui_view_reference_list_clamp_item_count(item_count) : 0;
    if (local->item_count == 0)
    {
        local->current_index = EGUI_VIEW_REFERENCE_LIST_INDEX_NONE;
    }
    else if (local->current_index == EGUI_VIEW_REFERENCE_LIST_INDEX_NONE || local->current_index >= local->item_count)
    {
        local->current_index = 0;
    }
    egui_view_reference_list_clear_pressed_state(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_reference_list_get_item_count(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_reference_list_t);
    return local->item_count;
}

static void egui_view_reference_list_set_current_index_inner(egui_view_t *self, uint8_t index, uint8_t notify)
{
    EGUI_LOCAL_INIT(egui_view_reference_list_t);

    if (local->item_count == 0 || local->items == NULL)
    {
        local->current_index = EGUI_VIEW_REFERENCE_LIST_INDEX_NONE;
        if (egui_view_reference_list_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return;
    }
    if (index >= local->item_count)
    {
        return;
    }
    if (local->current_index == index)
    {
        if (egui_view_reference_list_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_index = index;
    egui_view_reference_list_clear_pressed_state(self);
    if (notify)
    {
        egui_view_reference_list_notify_change(self, local);
    }
    egui_view_invalidate(self);
}

void egui_view_reference_list_set_current_index(egui_view_t *self, uint8_t index)
{
    egui_view_reference_list_set_current_index_inner(self, index, 1);
}

uint8_t egui_view_reference_list_get_current_index(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_reference_list_t);
    return local->current_index;
}

void egui_view_reference_list_set_on_selection_changed_listener(egui_view_t *self,
                                                                egui_view_on_reference_list_selection_changed_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_reference_list_t);
    local->on_selection_changed = listener;
}

void egui_view_reference_list_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_reference_list_t);
    local->font = font ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_reference_list_clear_pressed_state(self);
    egui_view_invalidate(self);
}

void egui_view_reference_list_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_reference_list_t);
    local->meta_font = font ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_reference_list_clear_pressed_state(self);
    egui_view_invalidate(self);
}

void egui_view_reference_list_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_reference_list_t);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_reference_list_clear_pressed_state(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_reference_list_get_compact_mode(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_reference_list_t);
    return local->compact_mode;
}

void egui_view_reference_list_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_reference_list_t);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_reference_list_clear_pressed_state(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_reference_list_get_read_only_mode(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_reference_list_t);
    return local->read_only_mode;
}

void egui_view_reference_list_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                          egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t success_color,
                                          egui_color_t warning_color, egui_color_t neutral_color)
{
    EGUI_LOCAL_INIT(egui_view_reference_list_t);

    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->neutral_color = neutral_color;
    egui_view_reference_list_clear_pressed_state(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_reference_list_get_item_region(egui_view_t *self, uint8_t index, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_reference_list_t);
    egui_view_reference_list_metrics_t metrics;

    if (region == NULL || index >= EGUI_VIEW_REFERENCE_LIST_MAX_ITEMS || index >= local->item_count)
    {
        return 0;
    }

    egui_view_reference_list_get_metrics(local, self, &metrics);
    if (metrics.item_regions[index].size.width <= 0 || metrics.item_regions[index].size.height <= 0)
    {
        return 0;
    }

    egui_region_copy(region, &metrics.item_regions[index]);
    return 1;
}

static void egui_view_reference_list_draw_item(egui_view_t *self, egui_view_reference_list_t *local, const egui_view_reference_list_item_t *item,
                                               const egui_region_t *region, uint8_t selected, uint8_t pressed, uint8_t last)
{
    char badge_label[12];
    char title_label[24];
    char meta_label[24];
    egui_region_t text_region;
    egui_color_t tone_color = egui_view_reference_list_tone_color(local, item->tone);
    egui_color_t row_fill =
            egui_rgb_mix(local->surface_color, tone_color, EGUI_ALPHA_MAKE(selected ? (local->compact_mode ? 12 : 10) : (item->emphasized ? 5 : 2)));
    egui_color_t row_border = egui_rgb_mix(local->border_color, tone_color, EGUI_ALPHA_MAKE(selected ? 32 : (item->emphasized ? 22 : 18)));
    egui_color_t title_color = selected ? egui_rgb_mix(local->text_color, tone_color, EGUI_ALPHA_MAKE(14)) : local->text_color;
    egui_color_t meta_color = selected ? egui_rgb_mix(local->muted_text_color, tone_color, EGUI_ALPHA_MAKE(24)) : local->muted_text_color;
    egui_color_t badge_fill = egui_rgb_mix(local->surface_color, tone_color, EGUI_ALPHA_MAKE(local->compact_mode ? 8 : 10));
    egui_color_t badge_border = egui_rgb_mix(local->border_color, tone_color, EGUI_ALPHA_MAKE(selected ? 32 : 24));
    egui_color_t badge_text = selected ? tone_color : egui_rgb_mix(local->muted_text_color, tone_color, EGUI_ALPHA_MAKE(24));
    egui_color_t divider_color = egui_rgb_mix(local->border_color, tone_color, EGUI_ALPHA_MAKE(20));
    egui_dim_t title_h = egui_view_reference_list_get_title_line_height(local);
    egui_dim_t meta_h = egui_view_reference_list_get_meta_line_height(local);
    egui_dim_t dot_size = (local->compact_mode ? 3 : 4) + (item->emphasized ? 1 : 0);
    egui_dim_t dot_x = region->location.x + (local->compact_mode ? 7 : 8);
    egui_dim_t dot_y = region->location.y + (region->size.height - dot_size) / 2;
    egui_dim_t text_x = dot_x + dot_size + (local->compact_mode ? 5 : 7);
    egui_dim_t inset_right = local->compact_mode ? 4 : 6;
    egui_dim_t badge_h = egui_view_reference_list_get_badge_height(local);
    egui_dim_t badge_max_w = region->size.width / 3;
    egui_dim_t badge_text_max_w = badge_max_w - (local->compact_mode ? 14 : 18);
    egui_dim_t badge_w = 0;
    egui_dim_t badge_x = region->location.x + region->size.width - badge_w - inset_right;
    egui_dim_t badge_y = region->location.y + (region->size.height - badge_h) / 2;
    egui_dim_t row_radius = local->compact_mode ? 6 : 7;
    egui_dim_t text_right = region->location.x + region->size.width - inset_right;

    if (selected)
    {
        dot_x = region->location.x + row_radius + (local->compact_mode ? 3 : 4);
        text_x = dot_x + dot_size + (local->compact_mode ? 5 : 7);
    }

    if (item->badge != NULL && item->badge[0] != '\0' && badge_text_max_w > 0)
    {
        egui_view_reference_list_fit_text_to_width(local->meta_font, item->badge, badge_label, sizeof(badge_label), badge_text_max_w,
                                                   local->compact_mode ? 4 : 5);
        if (badge_label[0] != '\0')
        {
            badge_w = egui_view_reference_list_badge_width(local->meta_font, badge_label, local->compact_mode, badge_max_w);
        }
    }
    badge_x = region->location.x + region->size.width - badge_w - inset_right;

    if (local->read_only_mode)
    {
        row_fill = egui_rgb_mix(row_fill, local->surface_color, EGUI_ALPHA_MAKE(24));
        row_border = egui_rgb_mix(row_border, local->muted_text_color, EGUI_ALPHA_MAKE(18));
        title_color = egui_rgb_mix(title_color, local->muted_text_color, EGUI_ALPHA_MAKE(14));
        meta_color = egui_rgb_mix(meta_color, local->muted_text_color, EGUI_ALPHA_MAKE(18));
        badge_fill = egui_rgb_mix(badge_fill, local->surface_color, EGUI_ALPHA_MAKE(24));
        badge_border = egui_rgb_mix(badge_border, local->muted_text_color, EGUI_ALPHA_MAKE(18));
        badge_text = egui_rgb_mix(badge_text, local->muted_text_color, EGUI_ALPHA_MAKE(24));
        divider_color = egui_rgb_mix(divider_color, local->muted_text_color, EGUI_ALPHA_MAKE(16));
        tone_color = egui_rgb_mix(tone_color, local->muted_text_color, EGUI_ALPHA_MAKE(24));
    }
    if (!egui_view_get_enable(self))
    {
        row_fill = egui_view_reference_list_mix_disabled(row_fill);
        row_border = egui_view_reference_list_mix_disabled(row_border);
        title_color = egui_view_reference_list_mix_disabled(title_color);
        meta_color = egui_view_reference_list_mix_disabled(meta_color);
        badge_fill = egui_view_reference_list_mix_disabled(badge_fill);
        badge_border = egui_view_reference_list_mix_disabled(badge_border);
        badge_text = egui_view_reference_list_mix_disabled(badge_text);
        divider_color = egui_view_reference_list_mix_disabled(divider_color);
        tone_color = egui_view_reference_list_mix_disabled(tone_color);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region->location.x, region->location.y, region->size.width, region->size.height, row_radius, row_fill,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(pressed ? 94 : (selected ? 92 : (local->compact_mode ? 76 : 68)))));

    if (selected)
    {
        hcw_selection_marker_draw_left(region, row_radius, row_radius, tone_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
    }
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, region->location.x, region->location.y, region->size.width, region->size.height, row_radius, 1, row_border,
                                     egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(selected ? 78 : (local->compact_mode ? 60 : 56))));

    if (!selected && !pressed && !item->emphasized && !last)
    {
        egui_canvas_draw_line(&uicode_get_core()->canvas, region->location.x + 12, region->location.y + region->size.height, region->location.x + region->size.width - 10,
                              region->location.y + region->size.height, 1, divider_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(70)));
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, dot_x, dot_y, dot_size, dot_size, dot_size / 2, tone_color, EGUI_ALPHA_100);

    if (badge_w > 0)
    {
        text_region.location.x = badge_x;
        text_region.location.y = badge_y;
        text_region.size.width = badge_w;
        text_region.size.height = badge_h;
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, badge_x, badge_y, badge_w, badge_h, badge_h / 2, badge_fill,
                                              egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(98)));
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, badge_x, badge_y, badge_w, badge_h, badge_h / 2, 1, badge_border,
                                         egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(80)));
        egui_view_reference_list_draw_text(local->meta_font, self, badge_label, &text_region, EGUI_ALIGN_CENTER, badge_text);
    }

    if (local->compact_mode)
    {
        if (badge_w == 0 && item->meta != NULL && item->meta[0] != '\0')
        {
            egui_region_t meta_region;

            meta_region.location.x = region->location.x + region->size.width / 2;
            meta_region.location.y = region->location.y;
            meta_region.size.width = region->location.x + region->size.width - inset_right - meta_region.location.x;
            meta_region.size.height = region->size.height;
            egui_view_reference_list_fit_text_to_width(local->meta_font, item->meta, meta_label, sizeof(meta_label), meta_region.size.width,
                                                       local->compact_mode ? 4 : 5);
            egui_view_reference_list_draw_text(local->meta_font, self, meta_label, &meta_region, EGUI_ALIGN_RIGHT | EGUI_ALIGN_VCENTER, meta_color);
            text_right = meta_region.location.x;
        }
        else if (badge_w > 0)
        {
            text_right = badge_x;
        }

        text_region.location.x = text_x;
        text_region.location.y = region->location.y;
        text_region.size.width = text_right - text_x - 4;
        text_region.size.height = region->size.height;
        egui_view_reference_list_fit_text_to_width(local->font, item->title, title_label, sizeof(title_label), text_region.size.width,
                                                   local->compact_mode ? 4 : 5);
        egui_view_reference_list_draw_text(local->font, self, title_label, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);
        return;
    }

    text_region.location.x = text_x;
    text_region.location.y = region->location.y;
    text_region.size.width = (badge_w > 0 ? badge_x : region->location.x + region->size.width - inset_right) - text_x - 4;
    text_region.size.height = title_h;
    egui_view_reference_list_fit_text_to_width(local->font, item->title, title_label, sizeof(title_label), text_region.size.width, local->compact_mode ? 4 : 5);
    egui_view_reference_list_draw_text(local->font, self, title_label, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);

    text_region.location.y = region->location.y + region->size.height - meta_h;
    text_region.size.height = meta_h;
    egui_view_reference_list_fit_text_to_width(local->meta_font, item->meta, meta_label, sizeof(meta_label), text_region.size.width,
                                               local->compact_mode ? 4 : 5);
    egui_view_reference_list_draw_text(local->meta_font, self, meta_label, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, meta_color);
}

static void egui_view_reference_list_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_reference_list_t);
    egui_region_t work_region;
    egui_view_reference_list_metrics_t metrics;
    egui_color_t card_fill;
    egui_color_t card_border;
    uint8_t item_count = egui_view_reference_list_clamp_item_count(local->item_count);
    uint8_t i;

    egui_view_get_work_region(self, &work_region);
    if (work_region.size.width <= 0 || work_region.size.height <= 0)
    {
        return;
    }

    card_fill = HCW_COLOR_PANEL;
    card_border = egui_rgb_mix(local->border_color, local->surface_color, EGUI_ALPHA_MAKE(local->compact_mode ? 12 : 16));
    if (local->read_only_mode)
    {
        card_fill = egui_rgb_mix(card_fill, local->surface_color, EGUI_ALPHA_MAKE(18));
        card_border = egui_rgb_mix(card_border, local->muted_text_color, EGUI_ALPHA_MAKE(18));
    }
    if (!egui_view_get_enable(self))
    {
        card_fill = egui_view_reference_list_mix_disabled(card_fill);
        card_border = egui_view_reference_list_mix_disabled(card_border);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, work_region.location.x, work_region.location.y, work_region.size.width, work_region.size.height,
                                          local->compact_mode ? EGUI_VIEW_REFERENCE_LIST_COMPACT_RADIUS : EGUI_VIEW_REFERENCE_LIST_STANDARD_RADIUS, card_fill,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->compact_mode ? 90 : 96)));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, work_region.location.x, work_region.location.y, work_region.size.width, work_region.size.height,
                                     local->compact_mode ? EGUI_VIEW_REFERENCE_LIST_COMPACT_RADIUS : EGUI_VIEW_REFERENCE_LIST_STANDARD_RADIUS, 1, card_border,
                                     egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->compact_mode ? 72 : 78)));

    if (item_count == 0 || local->items == NULL)
    {
        return;
    }

    egui_view_reference_list_get_metrics(local, self, &metrics);
    for (i = 0; i < item_count; i++)
    {
        const egui_view_reference_list_item_t *item = egui_view_reference_list_get_item(local, i);

        if (item == NULL)
        {
            continue;
        }
        egui_view_reference_list_draw_item(self, local, item, &metrics.item_regions[i], i == local->current_index, i == local->pressed_index,
                                           i + 1 >= item_count);
    }

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (self->is_focused && egui_view_get_enable(self) && work_region.size.width > 4 && work_region.size.height > 4)
    {
        egui_dim_t focus_x = work_region.location.x + 2;
        egui_dim_t focus_y = work_region.location.y + 2;
        egui_dim_t focus_w = work_region.size.width - 4;
        egui_dim_t focus_h = work_region.size.height - 4;
        egui_dim_t radius =
                (local->compact_mode ? EGUI_VIEW_REFERENCE_LIST_COMPACT_RADIUS : EGUI_VIEW_REFERENCE_LIST_STANDARD_RADIUS) - 2;

        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, work_region.location.x, work_region.location.y, work_region.size.width, work_region.size.height,
                                         local->compact_mode ? EGUI_VIEW_REFERENCE_LIST_COMPACT_RADIUS : EGUI_VIEW_REFERENCE_LIST_STANDARD_RADIUS, 2,
                                         EGUI_THEME_FOCUS, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(96)));
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, focus_x, focus_y, focus_w, focus_h, radius > 0 ? radius : 0, 1, EGUI_THEME_FOCUS,
                                         egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(68)));
    }
#endif
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_reference_list_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_reference_list_t);
    uint8_t hit_index;
    uint8_t handled;
    uint8_t same_target;

    if (local->item_count == 0 || local->items == NULL || !egui_view_get_enable(self) || local->read_only_mode)
    {
        if (egui_view_reference_list_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_index = egui_view_reference_list_hit_index(local, self, event->location.x, event->location.y);
        if (hit_index == EGUI_VIEW_REFERENCE_LIST_INDEX_NONE)
        {
            return 0;
        }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        same_target = local->pressed_index == hit_index;
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
        if (local->pressed_index == EGUI_VIEW_REFERENCE_LIST_INDEX_NONE)
        {
            return 0;
        }
        hit_index = egui_view_reference_list_hit_index(local, self, event->location.x, event->location.y);
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
        hit_index = egui_view_reference_list_hit_index(local, self, event->location.x, event->location.y);
        handled = (local->pressed_index != EGUI_VIEW_REFERENCE_LIST_INDEX_NONE) || hit_index != EGUI_VIEW_REFERENCE_LIST_INDEX_NONE;
        same_target = local->pressed_index != EGUI_VIEW_REFERENCE_LIST_INDEX_NONE && local->pressed_index == hit_index;
        if (same_target && self->is_pressed)
        {
            egui_view_reference_list_set_current_index_inner(self, hit_index, 1);
        }
        egui_view_reference_list_clear_pressed_state(self);
        return handled;
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        return egui_view_reference_list_clear_pressed_state(self);
    default:
        return 0;
    }
}

static int egui_view_reference_list_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_reference_list_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_reference_list_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_reference_list_t);
    uint8_t next_index = local->current_index;

    if (local->item_count == 0 || local->items == NULL || !egui_view_get_enable(self) || local->read_only_mode)
    {
        if (egui_view_reference_list_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    if ((event->key_code == EGUI_KEY_CODE_ENTER || event->key_code == EGUI_KEY_CODE_SPACE) &&
        (event->type == EGUI_KEY_EVENT_ACTION_DOWN || event->type == EGUI_KEY_EVENT_ACTION_UP))
    {
        egui_view_reference_list_clear_pressed_state(self);
        return 1;
    }

    if (event->type != EGUI_KEY_EVENT_ACTION_UP)
    {
        return 0;
    }

    if (next_index == EGUI_VIEW_REFERENCE_LIST_INDEX_NONE)
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
        egui_view_reference_list_set_current_index_inner(self, next_index, 1);
        return 1;
    case EGUI_KEY_CODE_RIGHT:
    case EGUI_KEY_CODE_DOWN:
        if (next_index + 1 < local->item_count)
        {
            next_index++;
        }
        egui_view_reference_list_set_current_index_inner(self, next_index, 1);
        return 1;
    case EGUI_KEY_CODE_HOME:
        egui_view_reference_list_set_current_index_inner(self, 0, 1);
        return 1;
    case EGUI_KEY_CODE_END:
        egui_view_reference_list_set_current_index_inner(self, (uint8_t)(local->item_count - 1), 1);
        return 1;
    case EGUI_KEY_CODE_TAB:
        next_index++;
        if (next_index >= local->item_count)
        {
            next_index = 0;
        }
        egui_view_reference_list_set_current_index_inner(self, next_index, 1);
        return 1;
    default:
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_reference_list_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_reference_list_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static void egui_view_reference_list_on_focus_change(egui_view_t *self, int is_focused)
{
    EGUI_LOCAL_INIT(egui_view_reference_list_t);

    if (!is_focused)
    {
        egui_view_reference_list_clear_pressed_state(self);
        egui_view_invalidate(self);
        return;
    }

    if (local->current_index == EGUI_VIEW_REFERENCE_LIST_INDEX_NONE && local->item_count > 0)
    {
        egui_view_reference_list_set_current_index_inner(self, 0, 0);
        return;
    }

    egui_view_invalidate(self);
}
#endif

void egui_view_reference_list_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_reference_list_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_reference_list_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_reference_list_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_reference_list_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_reference_list_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_reference_list_on_key_event,
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        .on_focus_changed = egui_view_reference_list_on_focus_change,
#endif
};

void egui_view_reference_list_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_reference_list_t);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_reference_list_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif

    local->items = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->on_selection_changed = NULL;
    local->surface_color = HCW_COLOR_SURFACE;
    local->border_color = HCW_COLOR_BORDER;
    local->text_color = HCW_COLOR_TEXT;
    local->muted_text_color = HCW_COLOR_TEXT_MUTED;
    local->accent_color = HCW_COLOR_PRIMARY;
    local->success_color = HCW_COLOR_SUCCESS;
    local->warning_color = HCW_COLOR_WARNING;
    local->neutral_color = HCW_COLOR_NEUTRAL;
    local->item_count = 0;
    local->current_index = EGUI_VIEW_REFERENCE_LIST_INDEX_NONE;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_index = EGUI_VIEW_REFERENCE_LIST_INDEX_NONE;

    egui_view_set_view_name(self, "egui_view_reference_list");
}
