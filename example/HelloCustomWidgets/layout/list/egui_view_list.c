#include "egui_view_list.h"

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

static egui_color_t egui_view_reference_list_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 68);
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
    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, self->alpha);
}

static egui_dim_t egui_view_reference_list_badge_width(const char *text, uint8_t compact_mode, egui_dim_t max_w)
{
    egui_dim_t width = (compact_mode ? 14 : 18) + egui_view_reference_list_text_len(text) * (compact_mode ? 4 : 5);
    egui_dim_t min_w = compact_mode ? 18 : 22;

    if (text == NULL || text[0] == '\0')
    {
        return 0;
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
    egui_dim_t row_h = local->compact_mode ? EGUI_VIEW_REFERENCE_LIST_COMPACT_ROW_H : EGUI_VIEW_REFERENCE_LIST_STANDARD_ROW_H;
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
        row_h = local->compact_mode ? 9 : 16;
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
    egui_region_t text_region;
    egui_color_t tone_color = egui_view_reference_list_tone_color(local, item->tone);
    egui_color_t row_fill = egui_rgb_mix(local->surface_color, tone_color, selected ? (local->compact_mode ? 16 : 12) : (item->emphasized ? 5 : 2));
    egui_color_t row_border = egui_rgb_mix(local->border_color, tone_color, selected ? 24 : (item->emphasized ? 12 : 8));
    egui_color_t title_color = selected ? egui_rgb_mix(local->text_color, tone_color, 8) : local->text_color;
    egui_color_t meta_color = selected ? egui_rgb_mix(local->muted_text_color, tone_color, 18) : local->muted_text_color;
    egui_color_t badge_fill = egui_rgb_mix(local->surface_color, tone_color, local->compact_mode ? 12 : 16);
    egui_color_t badge_border = egui_rgb_mix(local->border_color, tone_color, selected ? 24 : 16);
    egui_color_t badge_text = selected ? tone_color : egui_rgb_mix(local->muted_text_color, tone_color, 18);
    egui_color_t divider_color = egui_rgb_mix(local->border_color, tone_color, 10);
    egui_dim_t dot_size = (local->compact_mode ? 3 : 4) + (item->emphasized ? 1 : 0);
    egui_dim_t dot_x = region->location.x + (local->compact_mode ? 7 : 8);
    egui_dim_t dot_y = region->location.y + (region->size.height - dot_size) / 2;
    egui_dim_t text_x = dot_x + dot_size + (local->compact_mode ? 5 : 7);
    egui_dim_t inset_right = local->compact_mode ? 4 : 6;
    egui_dim_t badge_h = local->compact_mode ? 8 : 10;
    egui_dim_t badge_w = egui_view_reference_list_badge_width(item->badge, local->compact_mode, region->size.width / 3);
    egui_dim_t badge_x = region->location.x + region->size.width - badge_w - inset_right;
    egui_dim_t badge_y = region->location.y + (region->size.height - badge_h) / 2;
    egui_dim_t indicator_w = local->compact_mode ? 2 : 3;

    if (local->read_only_mode)
    {
        row_fill = egui_rgb_mix(row_fill, local->surface_color, 24);
        row_border = egui_rgb_mix(row_border, local->muted_text_color, 18);
        title_color = egui_rgb_mix(title_color, local->muted_text_color, 14);
        meta_color = egui_rgb_mix(meta_color, local->muted_text_color, 18);
        badge_fill = egui_rgb_mix(badge_fill, local->surface_color, 24);
        badge_border = egui_rgb_mix(badge_border, local->muted_text_color, 18);
        badge_text = egui_rgb_mix(badge_text, local->muted_text_color, 24);
        divider_color = egui_rgb_mix(divider_color, local->muted_text_color, 16);
        tone_color = egui_rgb_mix(tone_color, local->muted_text_color, 24);
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

    egui_canvas_draw_round_rectangle_fill(region->location.x, region->location.y, region->size.width, region->size.height, local->compact_mode ? 6 : 7,
                                          row_fill, egui_color_alpha_mix(self->alpha, pressed ? 92 : (selected ? 88 : (local->compact_mode ? 66 : 56))));
    egui_canvas_draw_round_rectangle(region->location.x, region->location.y, region->size.width, region->size.height, local->compact_mode ? 6 : 7, 1,
                                     row_border, egui_color_alpha_mix(self->alpha, selected ? 40 : (local->compact_mode ? 18 : 12)));

    if (!selected && !pressed && !item->emphasized && !last)
    {
        egui_canvas_draw_line(region->location.x + 12, region->location.y + region->size.height, region->location.x + region->size.width - 10,
                              region->location.y + region->size.height, 1, divider_color, egui_color_alpha_mix(self->alpha, 16));
    }

    if (selected)
    {
        egui_canvas_draw_round_rectangle_fill(region->location.x + 1, region->location.y + 2, indicator_w, region->size.height - 4, 1, tone_color,
                                              egui_color_alpha_mix(self->alpha, 100));
    }

    egui_canvas_draw_round_rectangle_fill(dot_x, dot_y, dot_size, dot_size, dot_size / 2, tone_color, egui_color_alpha_mix(self->alpha, 100));

    if (badge_w > 0)
    {
        text_region.location.x = badge_x;
        text_region.location.y = badge_y;
        text_region.size.width = badge_w;
        text_region.size.height = badge_h;
        egui_canvas_draw_round_rectangle_fill(badge_x, badge_y, badge_w, badge_h, badge_h / 2, badge_fill, egui_color_alpha_mix(self->alpha, 98));
        egui_canvas_draw_round_rectangle(badge_x, badge_y, badge_w, badge_h, badge_h / 2, 1, badge_border, egui_color_alpha_mix(self->alpha, 30));
        egui_view_reference_list_draw_text(local->meta_font, self, item->badge, &text_region, EGUI_ALIGN_CENTER, badge_text);
    }

    if (local->compact_mode)
    {
        text_region.location.x = text_x;
        text_region.location.y = region->location.y;
        text_region.size.width = (badge_w > 0 ? badge_x : region->location.x + region->size.width - inset_right) - text_x - 4;
        text_region.size.height = region->size.height;
        egui_view_reference_list_draw_text(local->font, self, item->title, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);
        if (badge_w == 0 && item->meta != NULL && item->meta[0] != '\0')
        {
            text_region.location.x = region->location.x + region->size.width / 2;
            text_region.size.width = region->size.width / 2 - inset_right;
            egui_view_reference_list_draw_text(local->meta_font, self, item->meta, &text_region, EGUI_ALIGN_RIGHT | EGUI_ALIGN_VCENTER, meta_color);
        }
        return;
    }

    text_region.location.x = text_x;
    text_region.location.y = region->location.y + 2;
    text_region.size.width = (badge_w > 0 ? badge_x : region->location.x + region->size.width - inset_right) - text_x - 4;
    text_region.size.height = 8;
    egui_view_reference_list_draw_text(local->font, self, item->title, &text_region, EGUI_ALIGN_LEFT, title_color);

    text_region.location.y = region->location.y + region->size.height - 8;
    text_region.size.height = 7;
    egui_view_reference_list_draw_text(local->meta_font, self, item->meta, &text_region, EGUI_ALIGN_LEFT, meta_color);
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

    card_fill = egui_rgb_mix(local->surface_color, EGUI_COLOR_HEX(0xF8FAFC), local->compact_mode ? 18 : 12);
    card_border = egui_rgb_mix(local->border_color, local->surface_color, local->compact_mode ? 6 : 10);
    if (local->read_only_mode)
    {
        card_fill = egui_rgb_mix(card_fill, local->surface_color, 18);
        card_border = egui_rgb_mix(card_border, local->muted_text_color, 18);
    }
    if (!egui_view_get_enable(self))
    {
        card_fill = egui_view_reference_list_mix_disabled(card_fill);
        card_border = egui_view_reference_list_mix_disabled(card_border);
    }

    egui_canvas_draw_round_rectangle_fill(work_region.location.x, work_region.location.y, work_region.size.width, work_region.size.height,
                                          local->compact_mode ? EGUI_VIEW_REFERENCE_LIST_COMPACT_RADIUS : EGUI_VIEW_REFERENCE_LIST_STANDARD_RADIUS, card_fill,
                                          egui_color_alpha_mix(self->alpha, local->compact_mode ? 90 : 96));
    egui_canvas_draw_round_rectangle(work_region.location.x, work_region.location.y, work_region.size.width, work_region.size.height,
                                     local->compact_mode ? EGUI_VIEW_REFERENCE_LIST_COMPACT_RADIUS : EGUI_VIEW_REFERENCE_LIST_STANDARD_RADIUS, 1, card_border,
                                     egui_color_alpha_mix(self->alpha, local->compact_mode ? 28 : 40));

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

        egui_canvas_draw_round_rectangle(work_region.location.x, work_region.location.y, work_region.size.width, work_region.size.height,
                                         local->compact_mode ? EGUI_VIEW_REFERENCE_LIST_COMPACT_RADIUS : EGUI_VIEW_REFERENCE_LIST_STANDARD_RADIUS, 2,
                                         EGUI_THEME_FOCUS, egui_color_alpha_mix(self->alpha, 96));
        egui_canvas_draw_round_rectangle(focus_x, focus_y, focus_w, focus_h, radius > 0 ? radius : 0, 1, EGUI_THEME_FOCUS,
                                         egui_color_alpha_mix(self->alpha, 48));
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

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_reference_list_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif

    local->items = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->on_selection_changed = NULL;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xD7DEE6);
    local->text_color = EGUI_COLOR_HEX(0x1A2733);
    local->muted_text_color = EGUI_COLOR_HEX(0x6E7E8C);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->success_color = EGUI_COLOR_HEX(0x0F7B45);
    local->warning_color = EGUI_COLOR_HEX(0xA55A00);
    local->neutral_color = EGUI_COLOR_HEX(0x7A8796);
    local->item_count = 0;
    local->current_index = EGUI_VIEW_REFERENCE_LIST_INDEX_NONE;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_index = EGUI_VIEW_REFERENCE_LIST_INDEX_NONE;

    egui_view_set_view_name(self, "egui_view_reference_list");
}
