#include "egui_view_expander.h"
#include "../../hcw_selection_marker.h"

#define EGUI_VIEW_EXPANDER_STANDARD_PAD_X         8
#define EGUI_VIEW_EXPANDER_STANDARD_PAD_Y         7
#define EGUI_VIEW_EXPANDER_STANDARD_RADIUS        10
#define EGUI_VIEW_EXPANDER_STANDARD_HEADER_HEIGHT 17
#define EGUI_VIEW_EXPANDER_STANDARD_ITEM_GAP      4
#define EGUI_VIEW_EXPANDER_STANDARD_HEADER_RADIUS 6
#define EGUI_VIEW_EXPANDER_STANDARD_BODY_GAP      2
#define EGUI_VIEW_EXPANDER_STANDARD_BODY_HEIGHT   28
#define EGUI_VIEW_EXPANDER_STANDARD_BODY_RADIUS   7
#define EGUI_VIEW_EXPANDER_STANDARD_META_HEIGHT   11
#define EGUI_VIEW_EXPANDER_STANDARD_EYEBROW_H     11
#define EGUI_VIEW_EXPANDER_STANDARD_FOOTER_H      12
#define EGUI_VIEW_EXPANDER_STANDARD_GLYPH_W       10
#define EGUI_VIEW_EXPANDER_STANDARD_GLYPH_H       10

#define EGUI_VIEW_EXPANDER_COMPACT_PAD_X         5
#define EGUI_VIEW_EXPANDER_COMPACT_PAD_Y         4
#define EGUI_VIEW_EXPANDER_COMPACT_RADIUS        8
#define EGUI_VIEW_EXPANDER_COMPACT_HEADER_HEIGHT 12
#define EGUI_VIEW_EXPANDER_COMPACT_ITEM_GAP      3
#define EGUI_VIEW_EXPANDER_COMPACT_HEADER_RADIUS 4
#define EGUI_VIEW_EXPANDER_COMPACT_BODY_GAP      1
#define EGUI_VIEW_EXPANDER_COMPACT_BODY_HEIGHT   16
#define EGUI_VIEW_EXPANDER_COMPACT_BODY_RADIUS   5
#define EGUI_VIEW_EXPANDER_COMPACT_META_HEIGHT   8
#define EGUI_VIEW_EXPANDER_COMPACT_EYEBROW_H     8
#define EGUI_VIEW_EXPANDER_COMPACT_FOOTER_H      8
#define EGUI_VIEW_EXPANDER_COMPACT_GLYPH_W       8
#define EGUI_VIEW_EXPANDER_COMPACT_GLYPH_H       8

typedef struct egui_view_expander_metrics egui_view_expander_metrics_t;
struct egui_view_expander_metrics
{
    egui_region_t content_region;
    egui_region_t item_regions[EGUI_VIEW_EXPANDER_MAX_ITEMS];
    egui_region_t header_regions[EGUI_VIEW_EXPANDER_MAX_ITEMS];
    egui_region_t body_regions[EGUI_VIEW_EXPANDER_MAX_ITEMS];
    egui_dim_t body_height;
};

static uint8_t expander_clamp_item_count(uint8_t count)
{
    return count > EGUI_VIEW_EXPANDER_MAX_ITEMS ? EGUI_VIEW_EXPANDER_MAX_ITEMS : count;
}

static uint8_t expander_text_len(const char *text)
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

static egui_dim_t expander_measure_font_line_height(const egui_font_t *font)
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

static egui_dim_t expander_measure_text_width(const egui_font_t *font, const char *text)
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

static void expander_copy_elided(char *buffer, uint8_t buffer_size, const char *text, uint8_t max_chars)
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

    length = expander_text_len(text);
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

    copy_length = max_chars - 3;
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

static void expander_fit_text_to_width(const egui_font_t *font, const char *text, char *buffer, uint8_t buffer_size, egui_dim_t max_width, egui_dim_t fallback_char_width)
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

    max_chars = expander_text_len(text);
    expander_copy_elided(buffer, buffer_size, text, max_chars);
    while (max_chars > 0)
    {
        egui_dim_t text_width = expander_measure_text_width(font, buffer);

        if (text_width <= 0)
        {
            text_width = (egui_dim_t)expander_text_len(buffer) * fallback_char_width;
        }
        if (text_width <= max_width)
        {
            break;
        }

        max_chars--;
        expander_copy_elided(buffer, buffer_size, text, max_chars);
    }
}

static egui_dim_t expander_get_title_line_height(egui_view_expander_t *local)
{
    egui_dim_t line_height = expander_measure_font_line_height(local->font);
    egui_dim_t fallback = local->compact_mode ? EGUI_VIEW_EXPANDER_COMPACT_HEADER_HEIGHT : EGUI_VIEW_EXPANDER_STANDARD_HEADER_HEIGHT;

    return line_height > 0 ? line_height : fallback;
}

static egui_dim_t expander_get_meta_line_height(egui_view_expander_t *local)
{
    egui_dim_t line_height = expander_measure_font_line_height(local->meta_font);
    egui_dim_t fallback = local->compact_mode ? EGUI_VIEW_EXPANDER_COMPACT_META_HEIGHT : EGUI_VIEW_EXPANDER_STANDARD_META_HEIGHT;

    return line_height > 0 ? line_height : fallback;
}

static egui_dim_t expander_get_header_height(egui_view_expander_t *local)
{
    egui_dim_t title_h = expander_get_title_line_height(local);
    egui_dim_t meta_h = expander_get_meta_line_height(local);
    egui_dim_t header_h = title_h > meta_h ? title_h : meta_h;
    egui_dim_t min_h = local->compact_mode ? EGUI_VIEW_EXPANDER_COMPACT_HEADER_HEIGHT : EGUI_VIEW_EXPANDER_STANDARD_HEADER_HEIGHT;

    header_h += 2;
    return header_h > min_h ? header_h : min_h;
}

static egui_dim_t expander_get_meta_height(egui_view_expander_t *local)
{
    egui_dim_t meta_h = expander_get_meta_line_height(local);
    egui_dim_t min_h = local->compact_mode ? EGUI_VIEW_EXPANDER_COMPACT_META_HEIGHT : EGUI_VIEW_EXPANDER_STANDARD_META_HEIGHT;

    return meta_h > min_h ? meta_h : min_h;
}

static egui_dim_t expander_get_eyebrow_height(egui_view_expander_t *local)
{
    egui_dim_t eyebrow_h = expander_get_meta_line_height(local);
    egui_dim_t min_h = local->compact_mode ? EGUI_VIEW_EXPANDER_COMPACT_EYEBROW_H : EGUI_VIEW_EXPANDER_STANDARD_EYEBROW_H;

    return eyebrow_h > min_h ? eyebrow_h : min_h;
}

static egui_dim_t expander_get_footer_height(egui_view_expander_t *local)
{
    egui_dim_t footer_h = expander_get_meta_line_height(local);
    egui_dim_t min_h = local->compact_mode ? EGUI_VIEW_EXPANDER_COMPACT_FOOTER_H : EGUI_VIEW_EXPANDER_STANDARD_FOOTER_H;

    return footer_h > min_h ? footer_h : min_h;
}

static egui_dim_t expander_get_body_height(egui_view_expander_t *local, const egui_view_expander_item_t *item)
{
    egui_dim_t body_text_h = expander_get_meta_line_height(local);
    egui_dim_t base_h = local->compact_mode ? EGUI_VIEW_EXPANDER_COMPACT_BODY_HEIGHT : EGUI_VIEW_EXPANDER_STANDARD_BODY_HEIGHT;

    if (local->compact_mode)
    {
        egui_dim_t compact_h = body_text_h + 6;

        return compact_h > base_h ? compact_h : base_h;
    }

    if (item != NULL)
    {
        egui_dim_t eyebrow_h = 0;
        egui_dim_t body_h = 5 + expander_get_footer_height(local) + 5 + body_text_h;

        if (item->eyebrow != NULL && item->eyebrow[0] != '\0')
        {
            eyebrow_h = expander_get_eyebrow_height(local) + 3;
        }
        body_h += eyebrow_h;
        if (item->body_secondary != NULL && item->body_secondary[0] != '\0')
        {
            body_h += body_text_h;
        }
        return body_h > base_h ? body_h : base_h;
    }

    return base_h;
}

static egui_color_t expander_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(44));
}

static uint8_t expander_clear_pressed_state(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_expander_t);
    uint8_t had_pressed = self->is_pressed || local->pressed_index != EGUI_VIEW_EXPANDER_INDEX_NONE;

    local->pressed_index = EGUI_VIEW_EXPANDER_INDEX_NONE;
    egui_view_set_pressed(self, false);
    return had_pressed;
}

static egui_color_t expander_tone_color(egui_view_expander_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_EXPANDER_TONE_SUCCESS:
        return local->success_color;
    case EGUI_VIEW_EXPANDER_TONE_WARNING:
        return local->warning_color;
    case EGUI_VIEW_EXPANDER_TONE_NEUTRAL:
        return local->neutral_color;
    default:
        return local->accent_color;
    }
}

static const egui_view_expander_item_t *expander_get_current_item(egui_view_expander_t *local)
{
    if (local->items == NULL || local->item_count == 0 || local->current_index >= local->item_count)
    {
        return NULL;
    }
    return &local->items[local->current_index];
}

static egui_dim_t expander_meta_width(const egui_font_t *font, const char *text, uint8_t compact_mode, egui_dim_t max_width)
{
    egui_dim_t width;
    egui_dim_t min_width = compact_mode ? 16 : 22;

    if (text == NULL || text[0] == '\0')
    {
        return 0;
    }

    width = min_width + expander_measure_text_width(font, text);
    if (width <= min_width)
    {
        width = min_width + expander_text_len(text) * (compact_mode ? 4 : 5);
    }
    if (width > max_width)
    {
        width = max_width;
    }
    return width;
}

static egui_dim_t expander_pill_width(const egui_font_t *font, const char *text, uint8_t compact_mode, egui_dim_t min_width, egui_dim_t max_width)
{
    egui_dim_t width = min_width;

    if (text != NULL && text[0] != '\0')
    {
        width += expander_measure_text_width(font, text);
        if (width <= min_width)
        {
            width += expander_text_len(text) * (compact_mode ? 4 : 5);
        }
    }
    if (width > max_width)
    {
        width = max_width;
    }
    return width;
}

static void expander_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align, egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (text == NULL || text[0] == '\0')
    {
        return;
    }

    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, align, color, self->alpha);
}

static void expander_draw_chevron(egui_view_t *self, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height, uint8_t expanded, egui_color_t color,
                                  egui_alpha_t alpha)
{
    if (expanded)
    {
        egui_canvas_draw_triangle_fill(&uicode_get_core()->canvas, x, y + 1, x + width, y + 1, x + width / 2, y + height - 1, color, egui_color_alpha_mix(self->alpha, alpha));
    }
    else
    {
        egui_canvas_draw_triangle_fill(&uicode_get_core()->canvas, x + 1, y, x + 1, y + height, x + width - 1, y + height / 2, color, egui_color_alpha_mix(self->alpha, alpha));
    }
}

static void expander_get_metrics(egui_view_expander_t *local, egui_view_t *self, egui_view_expander_metrics_t *metrics)
{
    egui_region_t region;
    egui_dim_t pad_x = local->compact_mode ? EGUI_VIEW_EXPANDER_COMPACT_PAD_X : EGUI_VIEW_EXPANDER_STANDARD_PAD_X;
    egui_dim_t pad_y = local->compact_mode ? EGUI_VIEW_EXPANDER_COMPACT_PAD_Y : EGUI_VIEW_EXPANDER_STANDARD_PAD_Y;
    egui_dim_t header_height = expander_get_header_height(local);
    egui_dim_t item_gap = local->compact_mode ? EGUI_VIEW_EXPANDER_COMPACT_ITEM_GAP : EGUI_VIEW_EXPANDER_STANDARD_ITEM_GAP;
    egui_dim_t body_gap = local->compact_mode ? EGUI_VIEW_EXPANDER_COMPACT_BODY_GAP : EGUI_VIEW_EXPANDER_STANDARD_BODY_GAP;
    egui_dim_t body_height = 0;
    egui_dim_t total_height;
    egui_dim_t available_height;
    egui_dim_t cursor_y;
    uint8_t i;

    egui_view_get_work_region(self, &region);
    metrics->content_region.location.x = region.location.x + pad_x;
    metrics->content_region.location.y = region.location.y + pad_y;
    metrics->content_region.size.width = region.size.width - pad_x * 2;
    metrics->content_region.size.height = region.size.height - pad_y * 2;
    metrics->body_height = 0;

    if (local->item_count == 0)
    {
        return;
    }

    available_height = metrics->content_region.size.height;
    total_height = local->item_count * header_height + (local->item_count - 1) * item_gap;
    if (local->expanded_index != EGUI_VIEW_EXPANDER_INDEX_NONE)
    {
        const egui_view_expander_item_t *expanded_item = &local->items[local->expanded_index];

        body_height = expander_get_body_height(local, expanded_item);
        if (total_height + body_gap + body_height > available_height)
        {
            body_height = 0;
        }
        if (body_height > 0)
        {
            total_height += body_gap + body_height;
            metrics->body_height = body_height;
        }
    }

    cursor_y = metrics->content_region.location.y;
    if (available_height > total_height)
    {
        cursor_y += (available_height - total_height) / 2;
    }

    for (i = 0; i < local->item_count; ++i)
    {
        metrics->header_regions[i].location.x = metrics->content_region.location.x;
        metrics->header_regions[i].location.y = cursor_y;
        metrics->header_regions[i].size.width = metrics->content_region.size.width;
        metrics->header_regions[i].size.height = header_height;
        metrics->body_regions[i].location.x = metrics->content_region.location.x;
        metrics->body_regions[i].location.y = cursor_y + header_height + body_gap;
        metrics->body_regions[i].size.width = metrics->content_region.size.width;
        metrics->body_regions[i].size.height = 0;
        metrics->item_regions[i] = metrics->header_regions[i];

        if (local->expanded_index == i && metrics->body_height > 0)
        {
            metrics->body_regions[i].size.height = metrics->body_height;
            metrics->item_regions[i].size.height = header_height + body_gap + metrics->body_height;
            cursor_y += metrics->item_regions[i].size.height;
        }
        else
        {
            cursor_y += header_height;
        }

        if (i + 1 < local->item_count)
        {
            cursor_y += item_gap;
        }
    }
}

static uint8_t expander_hit_index(egui_view_expander_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    egui_view_expander_metrics_t metrics;
    uint8_t i;

    if (local->item_count == 0)
    {
        return EGUI_VIEW_EXPANDER_INDEX_NONE;
    }

    expander_get_metrics(local, self, &metrics);
    for (i = 0; i < local->item_count; ++i)
    {
        if (egui_region_pt_in_rect(&metrics.header_regions[i], x, y))
        {
            return i;
        }
    }

    return EGUI_VIEW_EXPANDER_INDEX_NONE;
}

static void expander_set_current_index_inner(egui_view_t *self, uint8_t item_index, uint8_t notify)
{
    EGUI_LOCAL_INIT(egui_view_expander_t);

    if (local->item_count == 0)
    {
        local->current_index = 0;
        return;
    }
    if (item_index >= local->item_count)
    {
        item_index = local->item_count - 1;
    }
    if (local->current_index == item_index)
    {
        if (expander_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_index = item_index;
    expander_clear_pressed_state(self);
    if (notify && local->on_selection_changed != NULL)
    {
        local->on_selection_changed(self, item_index);
    }
    egui_view_invalidate(self);
}

static void expander_set_expanded_index_inner(egui_view_t *self, uint8_t item_index, uint8_t notify)
{
    EGUI_LOCAL_INIT(egui_view_expander_t);
    uint8_t normalized = item_index;

    if (normalized != EGUI_VIEW_EXPANDER_INDEX_NONE)
    {
        if (local->item_count == 0)
        {
            normalized = EGUI_VIEW_EXPANDER_INDEX_NONE;
        }
        else if (normalized >= local->item_count)
        {
            normalized = local->item_count - 1;
        }
    }

    if (local->expanded_index == normalized)
    {
        if (expander_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->expanded_index = normalized;
    expander_clear_pressed_state(self);
    if (notify && local->on_expanded_changed != NULL)
    {
        uint8_t callback_index = normalized == EGUI_VIEW_EXPANDER_INDEX_NONE ? local->current_index : normalized;
        local->on_expanded_changed(self, callback_index, normalized == EGUI_VIEW_EXPANDER_INDEX_NONE ? 0 : 1);
    }
    egui_view_invalidate(self);
}

static void expander_toggle_index_inner(egui_view_t *self, uint8_t item_index)
{
    EGUI_LOCAL_INIT(egui_view_expander_t);

    if (local->item_count == 0 || item_index >= local->item_count)
    {
        return;
    }

    expander_set_current_index_inner(self, item_index, 1);
    if (local->expanded_index == item_index)
    {
        expander_set_expanded_index_inner(self, EGUI_VIEW_EXPANDER_INDEX_NONE, 1);
    }
    else
    {
        expander_set_expanded_index_inner(self, item_index, 1);
    }
}

void egui_view_expander_set_items(egui_view_t *self, const egui_view_expander_item_t *items, uint8_t item_count)
{
    EGUI_LOCAL_INIT(egui_view_expander_t);

    local->items = items;
    local->item_count = expander_clamp_item_count(item_count);
    local->current_index = 0;
    local->expanded_index = local->item_count > 0 ? 0 : EGUI_VIEW_EXPANDER_INDEX_NONE;
    expander_clear_pressed_state(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_expander_get_item_count(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_expander_t);
    return local->item_count;
}

void egui_view_expander_set_current_index(egui_view_t *self, uint8_t item_index)
{
    expander_set_current_index_inner(self, item_index, 1);
}

uint8_t egui_view_expander_get_current_index(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_expander_t);
    return local->current_index;
}

void egui_view_expander_set_expanded_index(egui_view_t *self, uint8_t item_index)
{
    expander_set_expanded_index_inner(self, item_index, 1);
}

uint8_t egui_view_expander_get_expanded_index(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_expander_t);
    return local->expanded_index;
}

void egui_view_expander_toggle_current(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_expander_t);

    if (local->item_count == 0)
    {
        return;
    }
    expander_toggle_index_inner(self, local->current_index);
}

void egui_view_expander_set_on_selection_changed_listener(egui_view_t *self, egui_view_on_expander_selection_changed_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_expander_t);
    local->on_selection_changed = listener;
}

void egui_view_expander_set_on_expanded_changed_listener(egui_view_t *self, egui_view_on_expander_expanded_changed_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_expander_t);
    local->on_expanded_changed = listener;
}

void egui_view_expander_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_expander_t);
    local->font = font ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    expander_clear_pressed_state(self);
    egui_view_invalidate(self);
}

void egui_view_expander_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_expander_t);
    local->meta_font = font ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    expander_clear_pressed_state(self);
    egui_view_invalidate(self);
}

void egui_view_expander_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_expander_t);
    local->compact_mode = compact_mode ? 1 : 0;
    expander_clear_pressed_state(self);
    egui_view_invalidate(self);
}

void egui_view_expander_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_expander_t);
    local->read_only_mode = read_only_mode ? 1 : 0;
    expander_clear_pressed_state(self);
    egui_view_invalidate(self);
}

void egui_view_expander_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t section_color,
                                    egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t success_color,
                                    egui_color_t warning_color, egui_color_t neutral_color)
{
    EGUI_LOCAL_INIT(egui_view_expander_t);

    local->surface_color = surface_color;
    local->border_color = border_color;
    local->section_color = section_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->neutral_color = neutral_color;
    expander_clear_pressed_state(self);
    egui_view_invalidate(self);
}

static void expander_draw_header(egui_view_t *self, egui_view_expander_t *local, const egui_view_expander_item_t *item, const egui_region_t *region,
                                 uint8_t selected, uint8_t expanded, uint8_t pressed)
{
    egui_region_t text_region;
    char meta_label[16];
    char title_label[32];
    egui_color_t tone_color = expander_tone_color(local, item->tone);
    egui_color_t header_fill = egui_rgb_mix(local->section_color, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(expanded ? 24 : (selected ? 18 : (item->emphasized ? 14 : 8))));
    egui_color_t header_border = egui_rgb_mix(local->border_color, tone_color, EGUI_ALPHA_MAKE(expanded ? 26 : (selected ? 22 : 16)));
    egui_color_t header_text = selected || expanded ? egui_rgb_mix(local->text_color, tone_color, EGUI_ALPHA_MAKE(12)) : local->text_color;
    egui_color_t chevron_color = selected || expanded ? tone_color : egui_rgb_mix(local->muted_text_color, tone_color, EGUI_ALPHA_MAKE(8));
    egui_color_t meta_fill = egui_rgb_mix(local->surface_color, tone_color, EGUI_ALPHA_MAKE(expanded ? 8 : 4));
    egui_color_t meta_border = egui_rgb_mix(local->border_color, tone_color, EGUI_ALPHA_MAKE(expanded ? 16 : 10));
    egui_color_t meta_color = expanded ? tone_color : egui_rgb_mix(local->muted_text_color, tone_color, EGUI_ALPHA_MAKE(10));
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_EXPANDER_COMPACT_HEADER_RADIUS : EGUI_VIEW_EXPANDER_STANDARD_HEADER_RADIUS;
    egui_dim_t glyph_w = local->compact_mode ? EGUI_VIEW_EXPANDER_COMPACT_GLYPH_W : EGUI_VIEW_EXPANDER_STANDARD_GLYPH_W;
    egui_dim_t glyph_h = local->compact_mode ? EGUI_VIEW_EXPANDER_COMPACT_GLYPH_H : EGUI_VIEW_EXPANDER_STANDARD_GLYPH_H;
    egui_dim_t meta_h = expander_get_meta_height(local);
    egui_dim_t inset = local->compact_mode ? 5 : 7;
    egui_dim_t title_x = region->location.x + inset + glyph_w + 6;
    egui_dim_t meta_max_w = region->size.width - (title_x - region->location.x) - inset - (local->compact_mode ? 18 : 34);
    egui_dim_t meta_w;
    egui_dim_t meta_x;

    if (meta_max_w < (local->compact_mode ? 16 : 22))
    {
        meta_max_w = local->compact_mode ? 16 : 22;
    }
    meta_w = expander_meta_width(local->meta_font, item->meta, local->compact_mode, meta_max_w);
    meta_x = region->location.x + region->size.width - inset - meta_w;

    if (pressed)
    {
        header_fill = egui_rgb_mix(header_fill, tone_color, EGUI_ALPHA_MAKE(10));
        header_border = egui_rgb_mix(header_border, tone_color, EGUI_ALPHA_MAKE(12));
    }

    if (local->read_only_mode)
    {
        header_fill = egui_rgb_mix(header_fill, local->surface_color, EGUI_ALPHA_MAKE(24));
        header_border = egui_rgb_mix(header_border, local->muted_text_color, EGUI_ALPHA_MAKE(20));
        header_text = egui_rgb_mix(header_text, local->muted_text_color, EGUI_ALPHA_MAKE(18));
        chevron_color = egui_rgb_mix(chevron_color, local->muted_text_color, EGUI_ALPHA_MAKE(24));
        meta_fill = egui_rgb_mix(meta_fill, local->surface_color, EGUI_ALPHA_MAKE(24));
        meta_border = egui_rgb_mix(meta_border, local->muted_text_color, EGUI_ALPHA_MAKE(18));
        meta_color = egui_rgb_mix(meta_color, local->muted_text_color, EGUI_ALPHA_MAKE(22));
    }

    if (!egui_view_get_enable(self))
    {
        header_fill = expander_mix_disabled(header_fill);
        header_border = expander_mix_disabled(header_border);
        header_text = expander_mix_disabled(header_text);
        chevron_color = expander_mix_disabled(chevron_color);
        meta_fill = expander_mix_disabled(meta_fill);
        meta_border = expander_mix_disabled(meta_border);
        meta_color = expander_mix_disabled(meta_color);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region->location.x, region->location.y, region->size.width, region->size.height, radius, header_fill,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(expanded ? 96 : 92)));

    if (selected || expanded)
    {
        hcw_selection_marker_draw_left(region, radius, radius, tone_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(expanded ? 96 : 86)));
    }
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, region->location.x, region->location.y, region->size.width, region->size.height, radius, 1, header_border,
                                     egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(expanded ? 82 : 72)));

    expander_draw_chevron(self, region->location.x + inset, region->location.y + (region->size.height - glyph_h) / 2, glyph_w, glyph_h, expanded, chevron_color,
                          EGUI_ALPHA_MAKE(pressed ? 100 : 92));

    if (meta_w > 0)
    {
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, meta_x, region->location.y + (region->size.height - meta_h) / 2, meta_w, meta_h, meta_h / 2, meta_fill,
                                              egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(94)));
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, meta_x, region->location.y + (region->size.height - meta_h) / 2, meta_w, meta_h, meta_h / 2, 1, meta_border,
                                         egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(78)));

        text_region.location.x = meta_x;
        text_region.location.y = region->location.y + (region->size.height - meta_h) / 2;
        text_region.size.width = meta_w;
        text_region.size.height = meta_h;
        expander_fit_text_to_width(local->meta_font, item->meta, meta_label, sizeof(meta_label), meta_w - 4, local->compact_mode ? 4 : 5);
        expander_draw_text(local->meta_font, self, meta_label, &text_region, EGUI_ALIGN_CENTER, meta_color);
    }

    text_region.location.x = title_x;
    text_region.location.y = region->location.y;
    text_region.size.width = (meta_w > 0 ? meta_x - 4 : region->location.x + region->size.width - inset) - title_x;
    text_region.size.height = region->size.height;
    expander_fit_text_to_width(local->font, item->title, title_label, sizeof(title_label), text_region.size.width, local->compact_mode ? 4 : 5);
    expander_draw_text(local->font, self, title_label, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, header_text);
}

static void expander_draw_body(egui_view_t *self, egui_view_expander_t *local, const egui_view_expander_item_t *item, const egui_region_t *region)
{
    egui_region_t text_region;
    char eyebrow_label[20];
    char primary_label[48];
    char secondary_label[48];
    char footer_label[32];
    egui_color_t tone_color = expander_tone_color(local, item->tone);
    egui_color_t body_fill = egui_rgb_mix(local->surface_color, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(item->emphasized ? 12 : 8));
    egui_color_t body_border = egui_rgb_mix(local->border_color, tone_color, EGUI_ALPHA_MAKE(item->emphasized ? 18 : 14));
    egui_color_t eyebrow_fill = egui_rgb_mix(local->surface_color, tone_color, EGUI_ALPHA_MAKE(8));
    egui_color_t eyebrow_border = egui_rgb_mix(local->border_color, tone_color, EGUI_ALPHA_MAKE(14));
    egui_color_t eyebrow_color = tone_color;
    egui_color_t primary_color = egui_rgb_mix(local->text_color, tone_color, EGUI_ALPHA_MAKE(10));
    egui_color_t secondary_color = egui_rgb_mix(local->muted_text_color, tone_color, EGUI_ALPHA_MAKE(8));
    egui_color_t footer_fill = egui_rgb_mix(local->section_color, tone_color, EGUI_ALPHA_MAKE(8));
    egui_color_t footer_border = egui_rgb_mix(local->border_color, tone_color, EGUI_ALPHA_MAKE(16));
    egui_color_t footer_color = egui_rgb_mix(local->muted_text_color, tone_color, EGUI_ALPHA_MAKE(10));
    egui_dim_t eyebrow_h = expander_get_eyebrow_height(local);
    egui_dim_t footer_h = expander_get_footer_height(local);
    egui_dim_t body_text_h = expander_get_meta_line_height(local);
    egui_dim_t inner_x = region->location.x + (local->compact_mode ? 5 : 7);
    egui_dim_t inner_y = region->location.y + (local->compact_mode ? 4 : 5);
    egui_dim_t inner_w = region->size.width - (local->compact_mode ? 10 : 14);
    egui_dim_t footer_y = region->location.y + region->size.height - footer_h - (local->compact_mode ? 4 : 5);
    egui_dim_t eyebrow_w = expander_pill_width(local->meta_font, item->eyebrow, local->compact_mode, local->compact_mode ? 16 : 20, inner_w);
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_EXPANDER_COMPACT_BODY_RADIUS : EGUI_VIEW_EXPANDER_STANDARD_BODY_RADIUS;

    if (local->read_only_mode)
    {
        body_fill = egui_rgb_mix(body_fill, local->surface_color, EGUI_ALPHA_MAKE(24));
        body_border = egui_rgb_mix(body_border, local->muted_text_color, EGUI_ALPHA_MAKE(20));
        eyebrow_fill = egui_rgb_mix(eyebrow_fill, local->surface_color, EGUI_ALPHA_MAKE(26));
        eyebrow_border = egui_rgb_mix(eyebrow_border, local->muted_text_color, EGUI_ALPHA_MAKE(20));
        eyebrow_color = egui_rgb_mix(eyebrow_color, local->muted_text_color, EGUI_ALPHA_MAKE(30));
        primary_color = egui_rgb_mix(primary_color, local->muted_text_color, EGUI_ALPHA_MAKE(18));
        secondary_color = egui_rgb_mix(secondary_color, local->muted_text_color, EGUI_ALPHA_MAKE(22));
        footer_fill = egui_rgb_mix(footer_fill, local->surface_color, EGUI_ALPHA_MAKE(24));
        footer_border = egui_rgb_mix(footer_border, local->muted_text_color, EGUI_ALPHA_MAKE(18));
        footer_color = egui_rgb_mix(footer_color, local->muted_text_color, EGUI_ALPHA_MAKE(24));
    }

    if (!egui_view_get_enable(self))
    {
        body_fill = expander_mix_disabled(body_fill);
        body_border = expander_mix_disabled(body_border);
        eyebrow_fill = expander_mix_disabled(eyebrow_fill);
        eyebrow_border = expander_mix_disabled(eyebrow_border);
        eyebrow_color = expander_mix_disabled(eyebrow_color);
        primary_color = expander_mix_disabled(primary_color);
        secondary_color = expander_mix_disabled(secondary_color);
        footer_fill = expander_mix_disabled(footer_fill);
        footer_border = expander_mix_disabled(footer_border);
        footer_color = expander_mix_disabled(footer_color);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region->location.x, region->location.y, region->size.width, region->size.height, radius, body_fill,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(96)));
    hcw_selection_marker_draw_left(region, radius, radius, tone_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->read_only_mode ? 44 : 82)));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, region->location.x, region->location.y, region->size.width, region->size.height, radius, 1, body_border,
                                     egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(80)));

    if (!local->compact_mode && item->eyebrow != NULL && item->eyebrow[0] != '\0')
    {
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, inner_x, inner_y, eyebrow_w, eyebrow_h, eyebrow_h / 2, eyebrow_fill,
                                              egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(94)));
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, inner_x, inner_y, eyebrow_w, eyebrow_h, eyebrow_h / 2, 1, eyebrow_border,
                                         egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(78)));

        text_region.location.x = inner_x;
        text_region.location.y = inner_y;
        text_region.size.width = eyebrow_w;
        text_region.size.height = eyebrow_h;
        expander_fit_text_to_width(local->meta_font, item->eyebrow, eyebrow_label, sizeof(eyebrow_label), eyebrow_w - 4, local->compact_mode ? 4 : 5);
        expander_draw_text(local->meta_font, self, eyebrow_label, &text_region, EGUI_ALIGN_CENTER, eyebrow_color);
    }
    else
    {
        eyebrow_h = 0;
    }

    text_region.location.x = inner_x;
    text_region.location.y = local->compact_mode ? (region->location.y + (region->size.height - body_text_h) / 2) : (inner_y + eyebrow_h + 3);
    text_region.size.width = inner_w;
    text_region.size.height = body_text_h;
    expander_fit_text_to_width(local->meta_font, item->body_primary, primary_label, sizeof(primary_label), text_region.size.width, local->compact_mode ? 4 : 5);
    expander_draw_text(local->meta_font, self, primary_label, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, primary_color);

    if (local->compact_mode)
    {
        return;
    }

    if (item->body_secondary != NULL && item->body_secondary[0] != '\0')
    {
        text_region.location.y += text_region.size.height;
        text_region.size.height = body_text_h;
        expander_fit_text_to_width(local->meta_font, item->body_secondary, secondary_label, sizeof(secondary_label), text_region.size.width, 5);
        expander_draw_text(local->meta_font, self, secondary_label, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, secondary_color);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, inner_x, footer_y, inner_w, footer_h, footer_h / 2, footer_fill,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(90)));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, inner_x, footer_y, inner_w, footer_h, footer_h / 2, 1, footer_border,
                                     egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(78)));

    text_region.location.x = inner_x + (local->compact_mode ? 3 : 5);
    text_region.location.y = footer_y;
    text_region.size.width = inner_w - (local->compact_mode ? 6 : 10);
    text_region.size.height = footer_h;
    expander_fit_text_to_width(local->meta_font, item->footer, footer_label, sizeof(footer_label), text_region.size.width, 5);
    expander_draw_text(local->meta_font, self, footer_label, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, footer_color);
}

static void egui_view_expander_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_expander_t);
    egui_region_t region;
    egui_view_expander_metrics_t metrics;
    const egui_view_expander_item_t *current_item;
    egui_color_t focus_color;
    egui_color_t card_fill;
    egui_color_t card_border;
    uint8_t current_index;
    uint8_t i;

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0 || local->items == NULL || local->item_count == 0)
    {
        return;
    }

    current_index = local->current_index >= local->item_count ? 0 : local->current_index;
    local->current_index = current_index;
    if (local->expanded_index != EGUI_VIEW_EXPANDER_INDEX_NONE && local->expanded_index >= local->item_count)
    {
        local->expanded_index = 0;
    }
    current_item = expander_get_current_item(local);
    if (current_item == NULL)
    {
        return;
    }

    focus_color = expander_tone_color(local, current_item->tone);
    card_fill = egui_rgb_mix(local->surface_color, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(local->compact_mode ? 6 : 8));
    card_border = egui_rgb_mix(local->border_color, focus_color, EGUI_ALPHA_MAKE(local->compact_mode ? 16 : 20));
    if (local->read_only_mode)
    {
        focus_color = egui_rgb_mix(focus_color, local->muted_text_color, EGUI_ALPHA_MAKE(44));
        card_fill = egui_rgb_mix(card_fill, local->surface_color, EGUI_ALPHA_MAKE(18));
        card_border = egui_rgb_mix(card_border, local->muted_text_color, EGUI_ALPHA_MAKE(18));
    }
    if (!egui_view_get_enable(self))
    {
        focus_color = expander_mix_disabled(focus_color);
        card_fill = expander_mix_disabled(card_fill);
        card_border = expander_mix_disabled(card_border);
    }

    expander_get_metrics(local, self, &metrics);

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height,
                                          local->compact_mode ? EGUI_VIEW_EXPANDER_COMPACT_RADIUS : EGUI_VIEW_EXPANDER_STANDARD_RADIUS, card_fill,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(100)));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height,
                                     local->compact_mode ? EGUI_VIEW_EXPANDER_COMPACT_RADIUS : EGUI_VIEW_EXPANDER_STANDARD_RADIUS, 1, card_border,
                                     egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->compact_mode ? 78 : 82)));
    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region.location.x + 2, region.location.y + 2, region.size.width - 4, local->compact_mode ? 3 : 4,
                                          local->compact_mode ? EGUI_VIEW_EXPANDER_COMPACT_RADIUS : EGUI_VIEW_EXPANDER_STANDARD_RADIUS, focus_color,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->read_only_mode ? 20 : 44)));

    for (i = 0; i < local->item_count; ++i)
    {
        expander_draw_header(self, local, &local->items[i], &metrics.header_regions[i], local->current_index == i ? 1 : 0, local->expanded_index == i ? 1 : 0,
                             local->pressed_index == i ? 1 : 0);
        if (local->expanded_index == i && metrics.body_regions[i].size.height > 0)
        {
            expander_draw_body(self, local, &local->items[i], &metrics.body_regions[i]);
        }
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_expander_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_expander_t);
    uint8_t hit_index;
    uint8_t same_target;

    if (local->items == NULL || local->item_count == 0 || !egui_view_get_enable(self) || local->read_only_mode)
    {
        if (expander_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_index = expander_hit_index(local, self, event->location.x, event->location.y);
        if (hit_index == EGUI_VIEW_EXPANDER_INDEX_NONE)
        {
            if (expander_clear_pressed_state(self))
            {
                egui_view_invalidate(self);
            }
            return 0;
        }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        local->pressed_index = hit_index;
        egui_view_set_pressed(self, true);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_index == EGUI_VIEW_EXPANDER_INDEX_NONE)
        {
            return 0;
        }
        hit_index = expander_hit_index(local, self, event->location.x, event->location.y);
        same_target = (uint8_t)(local->pressed_index == hit_index);
        if (same_target)
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
    {
        uint8_t handled;

        hit_index = expander_hit_index(local, self, event->location.x, event->location.y);
        handled = (uint8_t)(local->pressed_index != EGUI_VIEW_EXPANDER_INDEX_NONE || hit_index != EGUI_VIEW_EXPANDER_INDEX_NONE);
        same_target = (uint8_t)(local->pressed_index != EGUI_VIEW_EXPANDER_INDEX_NONE && local->pressed_index == hit_index);
        if (same_target && self->is_pressed)
        {
            expander_toggle_index_inner(self, hit_index);
        }
        if (expander_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return handled;
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (expander_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_expander_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_expander_t);
    uint8_t next_index;

    if (local->items == NULL || local->item_count == 0 || !egui_view_get_enable(self) || local->read_only_mode)
    {
        if (expander_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    if (event->type != EGUI_KEY_EVENT_ACTION_UP)
    {
        switch (event->key_code)
        {
        case EGUI_KEY_CODE_UP:
        case EGUI_KEY_CODE_DOWN:
        case EGUI_KEY_CODE_HOME:
        case EGUI_KEY_CODE_END:
        case EGUI_KEY_CODE_ENTER:
        case EGUI_KEY_CODE_SPACE:
            return 1;
        default:
            return 0;
        }
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_UP:
        next_index = local->current_index > 0 ? (local->current_index - 1) : 0;
        expander_set_current_index_inner(self, next_index, 1);
        return 1;
    case EGUI_KEY_CODE_DOWN:
        next_index = local->current_index + 1 < local->item_count ? (local->current_index + 1) : (local->item_count - 1);
        expander_set_current_index_inner(self, next_index, 1);
        return 1;
    case EGUI_KEY_CODE_HOME:
        expander_set_current_index_inner(self, 0, 1);
        return 1;
    case EGUI_KEY_CODE_END:
        expander_set_current_index_inner(self, local->item_count - 1, 1);
        return 1;
    case EGUI_KEY_CODE_ENTER:
    case EGUI_KEY_CODE_SPACE:
        egui_view_expander_toggle_current(self);
        return 1;
    default:
        return egui_view_on_key_event(self, event);
    }
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_expander_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);

    if (expander_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_expander_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);

    if (expander_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_expander_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_expander_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_expander_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_expander_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_expander_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_expander_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_expander_on_key_event,
#endif
};

void egui_view_expander_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_expander_t);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_expander_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif

    local->items = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->on_selection_changed = NULL;
    local->on_expanded_changed = NULL;
    local->surface_color = HCW_COLOR_SURFACE;
    local->border_color = HCW_COLOR_BORDER;
    local->section_color = HCW_COLOR_SURFACE_SUBTLE;
    local->text_color = HCW_COLOR_TEXT_STRONG;
    local->muted_text_color = HCW_COLOR_TEXT_MUTED;
    local->accent_color = HCW_COLOR_PRIMARY;
    local->success_color = HCW_COLOR_SUCCESS;
    local->warning_color = HCW_COLOR_WARNING;
    local->neutral_color = HCW_COLOR_NEUTRAL;
    local->item_count = 0;
    local->current_index = 0;
    local->expanded_index = EGUI_VIEW_EXPANDER_INDEX_NONE;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_index = EGUI_VIEW_EXPANDER_INDEX_NONE;

    egui_view_set_view_name(self, "egui_view_expander");
}
