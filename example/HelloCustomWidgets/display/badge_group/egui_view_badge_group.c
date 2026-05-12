#include "egui_view_badge_group.h"
#include "../../hcw_text_center.h"
#include "../../hcw_top_accent.h"

static uint8_t egui_view_badge_group_clamp_snapshot_count(uint8_t count)
{
    if (count > EGUI_VIEW_BADGE_GROUP_MAX_SNAPSHOTS)
    {
        return EGUI_VIEW_BADGE_GROUP_MAX_SNAPSHOTS;
    }
    return count;
}

static uint8_t egui_view_badge_group_clamp_item_count(uint8_t count)
{
    if (count > EGUI_VIEW_BADGE_GROUP_MAX_ITEMS)
    {
        return EGUI_VIEW_BADGE_GROUP_MAX_ITEMS;
    }
    return count;
}

static uint8_t egui_view_badge_group_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static uint8_t egui_view_badge_group_text_len(const char *text)
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

static egui_dim_t egui_view_badge_group_measure_font_line_height(const egui_font_t *font)
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

static egui_dim_t egui_view_badge_group_measure_text_width(const egui_font_t *font, const char *text)
{
    egui_dim_t text_width = 0;
    egui_dim_t dummy_height = 0;

    if (!egui_view_badge_group_has_text(text) || font == NULL || font->api == NULL || font->api->get_str_size == NULL)
    {
        return 0;
    }

    font->api->get_str_size(font, text, 0, 0, &text_width, &dummy_height);
    return text_width;
}

static void egui_view_badge_group_copy_elided(char *buffer, uint8_t buffer_size, const char *text, uint8_t max_chars)
{
    uint8_t capacity;
    uint8_t allowed_chars;
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

    capacity = buffer_size - 1;
    length = egui_view_badge_group_text_len(text);
    if (length <= max_chars && length <= capacity)
    {
        for (index = 0; index < length; ++index)
        {
            buffer[index] = text[index];
        }
        buffer[length] = '\0';
        return;
    }

    allowed_chars = max_chars;
    if (allowed_chars > capacity)
    {
        allowed_chars = capacity;
    }
    if (allowed_chars == 0)
    {
        return;
    }

    if (allowed_chars <= 3)
    {
        for (index = 0; index < allowed_chars; ++index)
        {
            buffer[index] = '.';
        }
        buffer[allowed_chars] = '\0';
        return;
    }

    copy_length = allowed_chars - 3;
    for (index = 0; index < copy_length; ++index)
    {
        buffer[index] = text[index];
    }
    buffer[copy_length] = '.';
    buffer[copy_length + 1] = '.';
    buffer[copy_length + 2] = '.';
    buffer[copy_length + 3] = '\0';
}

static void egui_view_badge_group_fit_text_to_width(const egui_font_t *font, const char *text, char *buffer, uint8_t buffer_size, egui_dim_t max_width,
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

    max_chars = egui_view_badge_group_text_len(text);
    egui_view_badge_group_copy_elided(buffer, buffer_size, text, max_chars);
    while (max_chars > 0)
    {
        egui_dim_t text_width = egui_view_badge_group_measure_text_width(font, buffer);

        if (text_width <= 0)
        {
            text_width = (egui_dim_t)egui_view_badge_group_text_len(buffer) * fallback_char_width;
        }
        if (text_width <= max_width)
        {
            break;
        }

        max_chars--;
        egui_view_badge_group_copy_elided(buffer, buffer_size, text, max_chars);
    }
}

static uint8_t egui_view_badge_group_should_use_dense_layout(const egui_region_t *work_region)
{
    return (uint8_t)(work_region->size.width <= 132 || work_region->size.height <= 90);
}

static egui_dim_t egui_view_badge_group_get_badge_height(egui_view_badge_group_t *local, uint8_t dense_layout)
{
    egui_dim_t badge_h = egui_view_badge_group_measure_font_line_height(local->meta_font);
    egui_dim_t min_h = dense_layout ? 11 : 13;

    return badge_h > min_h ? badge_h : min_h;
}

static egui_dim_t egui_view_badge_group_get_title_height(egui_view_badge_group_t *local, uint8_t dense_layout)
{
    egui_dim_t title_h = egui_view_badge_group_measure_font_line_height(local->font);
    egui_dim_t min_h = dense_layout ? 11 : 13;

    return title_h > min_h ? title_h : min_h;
}

static egui_dim_t egui_view_badge_group_get_body_height(egui_view_badge_group_t *local, uint8_t dense_layout)
{
    egui_dim_t body_h;

    if (dense_layout)
    {
        return 0;
    }

    body_h = egui_view_badge_group_measure_font_line_height(local->font);
    return body_h > 11 ? body_h : 11;
}

static egui_dim_t egui_view_badge_group_get_footer_height(egui_view_badge_group_t *local, uint8_t dense_layout)
{
    egui_dim_t footer_h = egui_view_badge_group_measure_font_line_height(local->meta_font);
    egui_dim_t min_h = dense_layout ? 12 : 16;

    return footer_h > min_h ? footer_h : min_h;
}

static uint8_t egui_view_badge_group_focus_index(const egui_view_badge_group_snapshot_t *snapshot, uint8_t item_count)
{
    if (snapshot == NULL || item_count == 0 || snapshot->focus_index >= item_count)
    {
        return 0;
    }
    return snapshot->focus_index;
}

static egui_color_t egui_view_badge_group_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_MAKE(32));
}

static uint8_t egui_view_badge_group_clear_pressed_state(egui_view_t *self)
{
    if (!self->is_pressed)
    {
        return 0;
    }
    egui_view_set_pressed(self, false);
    return 1;
}

static egui_color_t egui_view_badge_group_tone_color(egui_view_badge_group_t *local, uint8_t tone)
{
    switch (tone)
    {
    case 1:
        return local->success_color;
    case 2:
        return local->warning_color;
    case 3:
        return local->neutral_color;
    default:
        return local->accent_color;
    }
}

static void egui_view_badge_group_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                            egui_color_t color)
{
    egui_region_t draw_region;

    if (font == NULL || text == NULL || text[0] == '\0' || region == NULL || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    draw_region = *region;
    draw_region.location.y += hcw_text_center_get_delta(font, text, region, align);
    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, align, color, self->alpha);
}

static egui_dim_t egui_view_badge_group_pill_width(const egui_font_t *font, const char *text, egui_dim_t min_w, egui_dim_t max_w)
{
    egui_dim_t width = min_w;

    if (egui_view_badge_group_has_text(text))
    {
        width += egui_view_badge_group_measure_text_width(font, text);
        if (width <= min_w)
        {
            width = min_w + egui_view_badge_group_text_len(text) * 5;
        }
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

static void egui_view_badge_group_draw_badge(egui_view_t *self, egui_view_badge_group_t *local, const egui_view_badge_group_item_t *item, egui_dim_t x,
                                             egui_dim_t y, egui_dim_t width, egui_dim_t height, uint8_t focused, uint8_t dense_layout)
{
    char label_text[16];
    char meta_text_buffer[8];
    egui_region_t text_region;
    egui_color_t tone_color;
    egui_color_t fill_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t meta_fill;
    egui_color_t meta_text;
    egui_dim_t radius;
    egui_dim_t meta_w;
    uint8_t is_enabled;

    tone_color = egui_view_badge_group_tone_color(local, item->tone);
    fill_color = item->outlined ? HCW_COLOR_PANEL : egui_rgb_mix(HCW_COLOR_PANEL, tone_color, EGUI_ALPHA_MAKE(item->emphasized ? 6 : 3));
    border_color = egui_rgb_mix(local->border_color, tone_color, EGUI_ALPHA_MAKE(focused ? 12 : (item->outlined ? 8 : 6)));
    text_color = focused ? egui_rgb_mix(local->text_color, tone_color, EGUI_ALPHA_MAKE(18)) : local->text_color;
    meta_fill = HCW_COLOR_PANEL;
    meta_text = egui_rgb_mix(local->text_color, tone_color, EGUI_ALPHA_MAKE(18));
    radius = height / 2;
    is_enabled = egui_view_get_enable(self) ? 1 : 0;

    if (!is_enabled)
    {
        fill_color = HCW_COLOR_PANEL;
        border_color = egui_rgb_mix(local->border_color, local->muted_text_color, EGUI_ALPHA_MAKE(22));
        text_color = egui_view_badge_group_mix_disabled(text_color);
        meta_fill = HCW_COLOR_PANEL;
        meta_text = local->muted_text_color;
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, x, y, width, height, radius, fill_color,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(item->outlined ? 74 : (focused ? 96 : 90))));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, x, y, width, height, radius, 1, border_color,
                                     egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(focused ? 76 : 64)));

    meta_w = 0;
    if (egui_view_badge_group_has_text(item->meta))
    {
        meta_w = 10 + egui_view_badge_group_measure_text_width(local->meta_font, item->meta);
        if (meta_w <= 10)
        {
            meta_w = 10 + egui_view_badge_group_text_len(item->meta) * 4;
        }
        if (meta_w < (dense_layout ? 16 : 18))
        {
            meta_w = dense_layout ? 16 : 18;
        }
        if (meta_w > width / 2)
        {
            meta_w = width / 2;
        }

        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, x + width - meta_w - 3, y + 2, meta_w, height - 4, (height - 4) / 2, meta_fill,
                                              egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(focused ? 88 : 82)));

        text_region.location.x = x + width - meta_w - 3;
        text_region.location.y = y;
        text_region.size.width = meta_w;
        text_region.size.height = height;
        egui_view_badge_group_fit_text_to_width(local->meta_font, item->meta, meta_text_buffer, sizeof(meta_text_buffer), text_region.size.width - 4, 4);
        egui_view_badge_group_draw_text(local->meta_font, self, meta_text_buffer, &text_region, EGUI_ALIGN_CENTER, meta_text);
    }

    text_region.location.x = x + (dense_layout ? 6 : 8);
    text_region.location.y = y;
    text_region.size.width = width - meta_w - (dense_layout ? 9 : 13);
    text_region.size.height = height;
    if (text_region.size.width < 0)
    {
        text_region.size.width = 0;
    }
    egui_view_badge_group_fit_text_to_width(local->meta_font, item->label, label_text, sizeof(label_text), text_region.size.width, dense_layout ? 4 : 5);
    egui_view_badge_group_draw_text(local->meta_font, self, label_text, &text_region, EGUI_ALIGN_CENTER, text_color);
}

void egui_view_badge_group_set_snapshots(egui_view_t *self, const egui_view_badge_group_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_badge_group_t);
    uint8_t had_pressed = egui_view_badge_group_clear_pressed_state(self);

    local->snapshots = snapshots;
    local->snapshot_count = snapshots == NULL ? 0 : egui_view_badge_group_clamp_snapshot_count(snapshot_count);
    if (local->current_snapshot >= local->snapshot_count)
    {
        local->current_snapshot = 0;
    }
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

void egui_view_badge_group_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    EGUI_LOCAL_INIT(egui_view_badge_group_t);
    if (local->snapshot_count == 0 || snapshot_index >= local->snapshot_count)
    {
        if (egui_view_badge_group_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return;
    }
    if (local->current_snapshot == snapshot_index)
    {
        if (egui_view_badge_group_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return;
    }
    local->current_snapshot = snapshot_index;
    egui_view_badge_group_clear_pressed_state(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_badge_group_get_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_badge_group_t);
    return local->current_snapshot;
}

void egui_view_badge_group_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_badge_group_t);
    uint8_t had_pressed = egui_view_badge_group_clear_pressed_state(self);

    local->font = font ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

void egui_view_badge_group_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_badge_group_t);
    uint8_t had_pressed = egui_view_badge_group_clear_pressed_state(self);

    local->meta_font = font ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

void egui_view_badge_group_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                       egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t success_color, egui_color_t warning_color,
                                       egui_color_t neutral_color)
{
    EGUI_LOCAL_INIT(egui_view_badge_group_t);
    uint8_t had_pressed = egui_view_badge_group_clear_pressed_state(self);

    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->neutral_color = neutral_color;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

static void egui_view_badge_group_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_badge_group_t);
    char eyebrow_text[16];
    char title_text[24];
    char body_text[32];
    char focus_label[16];
    char footer_text[32];
    egui_region_t region;
    egui_region_t text_region;
    const egui_view_badge_group_snapshot_t *snapshot;
    const egui_view_badge_group_item_t *focus_item;
    egui_color_t focus_color;
    egui_color_t card_fill;
    egui_color_t card_border;
    egui_color_t title_color;
    egui_color_t body_color;
    egui_color_t footer_color;
    egui_color_t footer_fill;
    egui_color_t footer_border;
    egui_color_t eyebrow_fill;
    egui_color_t eyebrow_border;
    egui_color_t focus_fill;
    egui_color_t focus_border;
    egui_dim_t x;
    egui_dim_t y;
    egui_dim_t w;
    egui_dim_t h;
    egui_dim_t radius;
    egui_dim_t accent_h;
    egui_dim_t content_y;
    egui_dim_t padding;
    egui_dim_t badge_h;
    egui_dim_t title_h;
    egui_dim_t body_h;
    egui_dim_t title_y;
    egui_dim_t body_y;
    egui_dim_t badges_y;
    egui_dim_t badges_w;
    egui_dim_t footer_y;
    egui_dim_t footer_h;
    egui_dim_t cursor_x;
    egui_dim_t cursor_y;
    egui_dim_t row_gap;
    egui_dim_t badge_gap;
    egui_dim_t eyebrow_w;
    egui_dim_t badge_w;
    egui_dim_t focus_pill_h;
    egui_dim_t focus_pill_w;
    egui_dim_t focus_pill_x;
    egui_dim_t focus_pill_y;
    uint8_t item_count;
    uint8_t focus_index;
    uint8_t dense_layout;
    uint8_t i;

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0 || local->snapshots == NULL || local->snapshot_count == 0)
    {
        return;
    }

    snapshot = &local->snapshots[local->current_snapshot];
    item_count = egui_view_badge_group_clamp_item_count(snapshot->item_count);
    if (item_count == 0)
    {
        return;
    }

    focus_index = egui_view_badge_group_focus_index(snapshot, item_count);
    focus_item = &snapshot->items[focus_index];
    dense_layout = egui_view_badge_group_should_use_dense_layout(&region);
    focus_color = egui_view_badge_group_tone_color(local, focus_item->tone);
    card_fill = HCW_COLOR_PANEL;
    card_border = egui_rgb_mix(local->border_color, focus_color, EGUI_ALPHA_MAKE(dense_layout ? 5 : 6));
    title_color = local->text_color;
    body_color = egui_rgb_mix(local->muted_text_color, local->text_color, EGUI_ALPHA_MAKE(dense_layout ? 18 : 24));
    footer_color = egui_rgb_mix(local->muted_text_color, focus_color, EGUI_ALPHA_MAKE(dense_layout ? 8 : 10));
    footer_fill = HCW_COLOR_PANEL;
    footer_border = egui_rgb_mix(local->border_color, focus_color, EGUI_ALPHA_MAKE(dense_layout ? 5 : 6));
    eyebrow_fill = egui_rgb_mix(HCW_COLOR_PANEL, focus_color, EGUI_ALPHA_MAKE(dense_layout ? 4 : 5));
    eyebrow_border = egui_rgb_mix(local->border_color, focus_color, EGUI_ALPHA_MAKE(dense_layout ? 6 : 8));
    focus_fill = egui_rgb_mix(HCW_COLOR_PANEL, focus_color, EGUI_ALPHA_MAKE(dense_layout ? 5 : 6));
    focus_border = egui_rgb_mix(local->border_color, focus_color, EGUI_ALPHA_MAKE(dense_layout ? 6 : 8));

    if (!egui_view_get_enable(self))
    {
        focus_color = local->muted_text_color;
        card_fill = HCW_COLOR_PANEL;
        card_border = egui_rgb_mix(local->border_color, local->muted_text_color, EGUI_ALPHA_MAKE(22));
        title_color = egui_view_badge_group_mix_disabled(title_color);
        body_color = local->muted_text_color;
        footer_color = local->muted_text_color;
        footer_fill = HCW_COLOR_PANEL;
        footer_border = egui_rgb_mix(local->border_color, local->muted_text_color, EGUI_ALPHA_MAKE(22));
        eyebrow_fill = HCW_COLOR_PANEL;
        eyebrow_border = egui_rgb_mix(local->border_color, local->muted_text_color, EGUI_ALPHA_MAKE(20));
        focus_fill = HCW_COLOR_PANEL;
        focus_border = egui_rgb_mix(local->border_color, local->muted_text_color, EGUI_ALPHA_MAKE(20));
    }

    x = region.location.x;
    y = region.location.y;
    w = region.size.width;
    h = region.size.height;
    radius = dense_layout ? 8 : 10;
    accent_h = dense_layout ? 3 : 4;
    padding = dense_layout ? 7 : 9;
    content_y = y + hcw_top_accent_content_offset(&region, radius, accent_h, dense_layout ? 2 : 3);
    badge_h = egui_view_badge_group_get_badge_height(local, dense_layout);
    title_h = egui_view_badge_group_get_title_height(local, dense_layout);
    body_h = egui_view_badge_group_get_body_height(local, dense_layout);
    row_gap = dense_layout ? 3 : 4;
    badge_gap = dense_layout ? 3 : 4;
    footer_h = egui_view_badge_group_get_footer_height(local, dense_layout);
    footer_y = y + h - padding - footer_h;
    title_y = content_y + badge_h + 4;
    body_y = title_y + title_h + 2;
    badges_y = body_y;
    if (body_h > 0)
    {
        badges_y += body_h + 3;
    }
    badges_w = w - padding * 2;
    eyebrow_w = egui_view_badge_group_pill_width(local->meta_font, snapshot->eyebrow, dense_layout ? 24 : 28, badges_w);
    focus_pill_h = badge_h;
    if (focus_pill_h < (dense_layout ? 10 : 12))
    {
        focus_pill_h = dense_layout ? 10 : 12;
    }
    if (focus_pill_h > footer_h - 2)
    {
        focus_pill_h = footer_h - 2;
    }
    focus_pill_w = egui_view_badge_group_pill_width(local->meta_font, focus_item->label, dense_layout ? 20 : 24, badges_w / 2);
    focus_pill_x = x + padding + 4;
    focus_pill_y = footer_y + (footer_h - focus_pill_h) / 2;

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, x, y, w, h, radius, card_fill,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(100)));
    {
        egui_region_t card_region;

        card_region.location.x = x;
        card_region.location.y = y;
        card_region.size.width = w;
        card_region.size.height = h;
        hcw_top_accent_draw(&card_region, radius, accent_h, focus_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(24)));
    }
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, x, y, w, h, radius, 1, card_border,
                                     egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(76)));

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, x + padding, content_y, eyebrow_w, badge_h, badge_h / 2, eyebrow_fill,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(88)));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, x + padding, content_y, eyebrow_w, badge_h, badge_h / 2, 1, eyebrow_border,
                                     egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(76)));

    text_region.location.x = x + padding;
    text_region.location.y = content_y;
    text_region.size.width = eyebrow_w;
    text_region.size.height = badge_h;
    egui_view_badge_group_fit_text_to_width(local->meta_font, snapshot->eyebrow, eyebrow_text, sizeof(eyebrow_text), text_region.size.width - 4, 4);
    egui_view_badge_group_draw_text(local->meta_font, self, eyebrow_text, &text_region, EGUI_ALIGN_CENTER, focus_color);

    text_region.location.x = x + padding;
    text_region.location.y = title_y;
    text_region.size.width = w - padding * 2;
    text_region.size.height = title_h;
    egui_view_badge_group_fit_text_to_width(local->font, snapshot->title, title_text, sizeof(title_text), text_region.size.width, dense_layout ? 4 : 5);
    egui_view_badge_group_draw_text(local->font, self, title_text, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);

    if (body_h > 0)
    {
        text_region.location.x = x + padding;
        text_region.location.y = body_y;
        text_region.size.width = w - padding * 2;
        text_region.size.height = body_h;
        egui_view_badge_group_fit_text_to_width(local->font, snapshot->body, body_text, sizeof(body_text), text_region.size.width, dense_layout ? 4 : 5);
        egui_view_badge_group_draw_text(local->font, self, body_text, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, body_color);
    }

    cursor_x = x + padding;
    cursor_y = badges_y;
    for (i = 0; i < item_count; i++)
    {
        badge_w = 18 + egui_view_badge_group_measure_text_width(local->meta_font, snapshot->items[i].label);
        if (egui_view_badge_group_has_text(snapshot->items[i].meta))
        {
            badge_w += 12 + egui_view_badge_group_measure_text_width(local->meta_font, snapshot->items[i].meta);
        }

        if (badge_w < (dense_layout ? 34 : 42))
        {
            badge_w = dense_layout ? 34 : 42;
        }
        if (badge_w > badges_w)
        {
            badge_w = badges_w;
        }

        if (cursor_x + badge_w > x + padding + badges_w)
        {
            cursor_x = x + padding;
            cursor_y += badge_h + row_gap;
        }
        if (cursor_y + badge_h > footer_y - 2)
        {
            break;
        }

        egui_view_badge_group_draw_badge(self, local, &snapshot->items[i], cursor_x, cursor_y, badge_w, badge_h, i == focus_index ? 1 : 0, dense_layout);
        cursor_x += badge_w + badge_gap;
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, x + padding, footer_y, w - padding * 2, footer_h, dense_layout ? 6 : 8, footer_fill,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(dense_layout ? 54 : 62)));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, x + padding, footer_y, w - padding * 2, footer_h, dense_layout ? 6 : 8, 1, footer_border,
                                     egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(dense_layout ? 70 : 76)));

    if (!dense_layout)
    {
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, focus_pill_x, focus_pill_y, focus_pill_w, focus_pill_h, focus_pill_h / 2, focus_fill,
                                              egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(88)));
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, focus_pill_x, focus_pill_y, focus_pill_w, focus_pill_h, focus_pill_h / 2, 1, focus_border,
                                         egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(76)));

        text_region.location.x = focus_pill_x;
        text_region.location.y = focus_pill_y;
        text_region.size.width = focus_pill_w;
        text_region.size.height = focus_pill_h;
        egui_view_badge_group_fit_text_to_width(local->meta_font, focus_item->label, focus_label, sizeof(focus_label), text_region.size.width - 4, 4);
        egui_view_badge_group_draw_text(local->meta_font, self, focus_label, &text_region, EGUI_ALIGN_CENTER, focus_color);

        text_region.location.x = focus_pill_x + focus_pill_w + 4;
        text_region.location.y = footer_y;
        text_region.size.width = x + w - padding - 3 - text_region.location.x;
        text_region.size.height = footer_h;
        egui_view_badge_group_fit_text_to_width(local->meta_font, snapshot->footer, footer_text, sizeof(footer_text), text_region.size.width, 4);
        egui_view_badge_group_draw_text(local->meta_font, self, footer_text, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, footer_color);
    }
    else
    {
        text_region.location.x = x + padding + 4;
        text_region.location.y = footer_y;
        text_region.size.width = w - padding * 2 - 8;
        text_region.size.height = footer_h;
        egui_view_badge_group_fit_text_to_width(local->meta_font, snapshot->footer, footer_text, sizeof(footer_text), text_region.size.width, 4);
        egui_view_badge_group_draw_text(local->meta_font, self, footer_text, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, footer_color);
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_badge_group_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    if (!egui_view_get_enable(self))
    {
        if (egui_view_badge_group_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        EGUI_UNUSED(event);
        return 0;
    }

    return egui_view_on_touch_event(self, event);
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_badge_group_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    if (!egui_view_get_enable(self))
    {
        if (egui_view_badge_group_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        EGUI_UNUSED(event);
        return 0;
    }

    return egui_view_on_key_event(self, event);
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_badge_group_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_badge_group_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_badge_group_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_badge_group_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_badge_group_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_badge_group_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_badge_group_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_badge_group_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
        .on_touch_event = egui_view_badge_group_on_touch_event,
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_badge_group_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_badge_group_on_key_event,
#endif
};

void egui_view_badge_group_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_badge_group_t);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_badge_group_t);
    egui_view_set_padding_all(self, 2);

    local->snapshots = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->surface_color = HCW_COLOR_SURFACE;
    local->border_color = HCW_COLOR_BORDER;
    local->text_color = HCW_COLOR_TEXT;
    local->muted_text_color = HCW_COLOR_TEXT_MUTED;
    local->accent_color = HCW_COLOR_PRIMARY;
    local->success_color = HCW_COLOR_SUCCESS;
    local->warning_color = HCW_COLOR_WARNING;
    local->neutral_color = HCW_COLOR_NEUTRAL;
    local->snapshot_count = 0;
    local->current_snapshot = 0;
}
