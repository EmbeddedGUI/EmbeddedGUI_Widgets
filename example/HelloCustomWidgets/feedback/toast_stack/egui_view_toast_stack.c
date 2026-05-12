#include <stdlib.h>

#include "egui_view_toast_stack.h"
#include "../../hcw_text_center.h"
#include "../../hcw_selection_marker.h"

#define TOAST_STACK_STANDARD_SHIFT_X 9
#define TOAST_STACK_STANDARD_SHIFT_Y 9
#define TOAST_STACK_COMPACT_SHIFT_X  4
#define TOAST_STACK_COMPACT_SHIFT_Y  4

static uint8_t egui_view_toast_stack_clamp_snapshot_count(uint8_t count)
{
    if (count > EGUI_VIEW_TOAST_STACK_MAX_SNAPSHOTS)
    {
        return EGUI_VIEW_TOAST_STACK_MAX_SNAPSHOTS;
    }
    return count;
}

static egui_color_t egui_view_toast_stack_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(44));
}

static egui_color_t egui_view_toast_stack_severity_color(egui_view_toast_stack_t *local, uint8_t severity)
{
    switch (severity)
    {
    case 1:
        return local->success_color;
    case 2:
        return local->warning_color;
    case 3:
        return local->error_color;
    default:
        return local->info_color;
    }
}

static const char *egui_view_toast_stack_severity_glyph(uint8_t severity)
{
    switch (severity)
    {
    case 1:
        return "+";
    case 2:
        return "!";
    case 3:
        return "x";
    default:
        return "i";
    }
}

static egui_dim_t egui_view_toast_stack_measure_text_width(const egui_font_t *font, const char *text)
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

static uint8_t egui_view_toast_stack_text_len(const char *text)
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

static uint8_t egui_view_toast_stack_is_space_char(char c)
{
    return (uint8_t)(c == ' ' || c == '\t');
}

static uint8_t egui_view_toast_stack_is_break_after_char(char c)
{
    return (uint8_t)(c == '-' || c == '/');
}

static uint8_t egui_view_toast_stack_find_elide_boundary(const char *text, uint8_t visible_chars)
{
    uint8_t index;

    if (text == NULL || visible_chars == 0)
    {
        return 0;
    }

    for (index = visible_chars; index > 0; --index)
    {
        if (egui_view_toast_stack_is_space_char(text[index - 1]))
        {
            return (uint8_t)(index - 1);
        }
    }

    for (index = visible_chars; index > 0; --index)
    {
        if (egui_view_toast_stack_is_break_after_char(text[index - 1]))
        {
            return index;
        }
    }

    return visible_chars;
}

static void egui_view_toast_stack_copy_elided(char *buffer, uint8_t buffer_size, const char *text, uint8_t max_chars)
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

    length = egui_view_toast_stack_text_len(text);
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

    copy_length = egui_view_toast_stack_find_elide_boundary(text, (uint8_t)(max_chars - 3));
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

static void egui_view_toast_stack_fit_text_to_width(const egui_font_t *font, const char *text, char *buffer, uint8_t buffer_size, egui_dim_t max_width,
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

    max_chars = egui_view_toast_stack_text_len(text);
    egui_view_toast_stack_copy_elided(buffer, buffer_size, text, max_chars);
    while (max_chars > 0)
    {
        egui_dim_t text_width = egui_view_toast_stack_measure_text_width(font, buffer);

        if (text_width <= 0)
        {
            text_width = (egui_dim_t)egui_view_toast_stack_text_len(buffer) * fallback_char_width;
        }
        if (text_width <= max_width)
        {
            break;
        }

        max_chars--;
        egui_view_toast_stack_copy_elided(buffer, buffer_size, text, max_chars);
    }
}

static egui_dim_t egui_view_toast_stack_measure_font_line_height(const egui_font_t *font)
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

static egui_dim_t egui_view_toast_stack_resolve_line_height(const egui_font_t *font, egui_dim_t fallback)
{
    egui_dim_t line_height = egui_view_toast_stack_measure_font_line_height(font);

    return line_height > fallback ? line_height : fallback;
}

static uint8_t egui_view_toast_stack_clear_pressed_state(egui_view_t *self)
{
    if (!self->is_pressed)
    {
        return 0;
    }
    egui_view_set_pressed(self, false);
    return 1;
}

void egui_view_toast_stack_set_snapshots(egui_view_t *self, const egui_view_toast_stack_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_toast_stack_t);
    uint8_t had_pressed = egui_view_toast_stack_clear_pressed_state(self);

    local->snapshots = snapshots;
    local->snapshot_count = snapshots == NULL ? 0 : egui_view_toast_stack_clamp_snapshot_count(snapshot_count);
    if (local->current_snapshot >= local->snapshot_count)
    {
        local->current_snapshot = 0;
    }
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

void egui_view_toast_stack_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    EGUI_LOCAL_INIT(egui_view_toast_stack_t);
    if (local->snapshot_count == 0 || snapshot_index >= local->snapshot_count)
    {
        if (egui_view_toast_stack_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return;
    }
    if (local->current_snapshot == snapshot_index)
    {
        if (egui_view_toast_stack_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return;
    }
    local->current_snapshot = snapshot_index;
    egui_view_toast_stack_clear_pressed_state(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_toast_stack_get_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_toast_stack_t);
    return local->current_snapshot;
}

void egui_view_toast_stack_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_toast_stack_t);
    uint8_t had_pressed = egui_view_toast_stack_clear_pressed_state(self);

    local->font = font ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

void egui_view_toast_stack_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_toast_stack_t);
    uint8_t had_pressed = egui_view_toast_stack_clear_pressed_state(self);

    local->meta_font = font ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

void egui_view_toast_stack_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_toast_stack_t);
    uint8_t had_pressed = egui_view_toast_stack_clear_pressed_state(self);

    local->compact_mode = compact_mode ? 1 : 0;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

void egui_view_toast_stack_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_toast_stack_t);
    uint8_t had_pressed = egui_view_toast_stack_clear_pressed_state(self);

    local->read_only_mode = read_only_mode ? 1 : 0;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

void egui_view_toast_stack_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                       egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t info_color, egui_color_t success_color,
                                       egui_color_t warning_color, egui_color_t error_color)
{
    EGUI_LOCAL_INIT(egui_view_toast_stack_t);
    uint8_t had_pressed = egui_view_toast_stack_clear_pressed_state(self);

    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->info_color = info_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->error_color = error_color;
    EGUI_UNUSED(had_pressed);
    egui_view_invalidate(self);
}

static void egui_view_toast_stack_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                            egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (font == NULL || text == NULL || text[0] == '\0' || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    draw_region.location.y += hcw_text_center_get_delta(font, text, region, align);
    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, align, color, self->alpha);
}

static void egui_view_toast_stack_draw_pill(const egui_font_t *font, egui_view_t *self, const char *text, egui_dim_t x, egui_dim_t y, egui_dim_t width,
                                            egui_dim_t height, egui_dim_t radius, egui_color_t fill_color, egui_alpha_t fill_alpha, egui_color_t border_color,
                                            egui_alpha_t border_alpha, egui_color_t text_color, egui_dim_t fallback_char_width)
{
    char pill_text[24];
    egui_region_t text_region;

    if (font == NULL || width <= 0 || height <= 0 || text == NULL || text[0] == '\0')
    {
        return;
    }

    egui_view_toast_stack_fit_text_to_width(font, text, pill_text, sizeof(pill_text), width - (height <= 11 ? 8 : 10), fallback_char_width);

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, x, y, width, height, radius, fill_color, egui_color_alpha_mix(self->alpha, fill_alpha));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, x, y, width, height, radius, 1, border_color, egui_color_alpha_mix(self->alpha, border_alpha));

    text_region.location.x = x;
    text_region.location.y = y;
    text_region.size.width = width;
    text_region.size.height = height;
    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, pill_text, &text_region, EGUI_ALIGN_CENTER, text_color, self->alpha);
}

static void egui_view_toast_stack_draw_back_card(egui_view_t *self, const egui_font_t *font, const egui_region_t *card_region, const char *title,
                                                 egui_color_t fill_color, egui_color_t border_color, egui_color_t strip_color, egui_color_t text_color,
                                                 uint8_t compact_mode)
{
    char title_label[32];
    egui_region_t title_region;
    egui_dim_t radius = compact_mode ? 5 : 6;
    egui_dim_t strip_w = compact_mode ? 3 : 4;
    egui_region_t strip_region = *card_region;
    egui_dim_t footer_w;
    egui_dim_t fallback_char_width = compact_mode ? 4 : 5;

    if (card_region->size.width <= 12 || card_region->size.height <= 12)
    {
        return;
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, card_region->location.x, card_region->location.y, card_region->size.width, card_region->size.height, radius,
                                          fill_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(compact_mode ? 94 : 96)));
    hcw_selection_marker_draw_left(&strip_region, radius, EGUI_MAX(strip_w, radius), strip_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(compact_mode ? 86 : 92)));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, card_region->location.x, card_region->location.y, card_region->size.width, card_region->size.height, radius, 1,
                                     border_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(compact_mode ? 88 : 92)));

    if (!compact_mode)
    {
        title_region.location.x = card_region->location.x + strip_w + 7;
        title_region.location.y = card_region->location.y;
        title_region.size.width = card_region->size.width - strip_w - 14;
        title_region.size.height = card_region->size.height;
        egui_view_toast_stack_fit_text_to_width(font, title, title_label, sizeof(title_label), title_region.size.width, fallback_char_width);
        egui_view_toast_stack_draw_text(font, self, title_label, &title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text_color);
    }

    footer_w = card_region->size.width / (compact_mode ? 2 : 3);
    if (footer_w < 16)
    {
        footer_w = 16;
    }
    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, card_region->location.x + strip_w + (compact_mode ? 5 : 7),
                                          card_region->location.y + card_region->size.height - (compact_mode ? 8 : 10), footer_w, 2, 1, border_color,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(compact_mode ? 36 : 44)));
}

static void egui_view_toast_stack_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_toast_stack_t);
    char title_label[32];
    char body_label[48];
    egui_region_t region;
    egui_region_t front_region;
    egui_region_t mid_region;
    egui_region_t back_region;
    egui_region_t text_region;
    const egui_view_toast_stack_snapshot_t *snapshot;
    egui_color_t severity_color;
    egui_color_t front_fill;
    egui_color_t front_border;
    egui_color_t back_fill;
    egui_color_t back_border;
    egui_color_t title_color;
    egui_color_t body_color;
    egui_color_t meta_color;
    egui_color_t glyph_color;
    egui_color_t shadow_color;
    egui_color_t action_fill;
    egui_color_t action_border;
    egui_color_t action_text;
    uint8_t is_enabled;
    uint8_t show_action;
    uint8_t show_close;
    egui_dim_t shift_x;
    egui_dim_t shift_y;
    egui_dim_t radius;
    egui_dim_t strip_w;
    egui_dim_t content_x;
    egui_dim_t content_y;
    egui_dim_t content_w;
    egui_dim_t content_h;
    egui_dim_t icon_size;
    egui_dim_t close_w;
    egui_dim_t title_x;
    egui_dim_t title_w;
    egui_dim_t title_slot_h;
    egui_dim_t title_h;
    egui_dim_t body_y;
    egui_dim_t body_h;
    egui_dim_t footer_y;
    egui_dim_t action_h;
    egui_dim_t action_w;
    egui_dim_t meta_h;
    egui_dim_t meta_w;
    egui_dim_t footer_gap;
    egui_dim_t fallback_char_width;

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0 || local->snapshots == NULL || local->snapshot_count == 0)
    {
        return;
    }

    snapshot = &local->snapshots[local->current_snapshot];
    shift_x = local->compact_mode ? TOAST_STACK_COMPACT_SHIFT_X : TOAST_STACK_STANDARD_SHIFT_X;
    shift_y = local->compact_mode ? TOAST_STACK_COMPACT_SHIFT_Y : TOAST_STACK_STANDARD_SHIFT_Y;
    radius = local->compact_mode ? 6 : 8;
    strip_w = local->compact_mode ? 3 : 4;

    front_region.location.x = region.location.x;
    front_region.location.y = region.location.y;
    front_region.size.width = region.size.width - shift_x * 2;
    front_region.size.height = region.size.height - shift_y * 2;
    mid_region.location.x = region.location.x + shift_x;
    mid_region.location.y = region.location.y + shift_y;
    mid_region.size.width = front_region.size.width;
    mid_region.size.height = front_region.size.height;
    back_region.location.x = region.location.x + shift_x * 2;
    back_region.location.y = region.location.y + shift_y * 2;
    back_region.size.width = front_region.size.width;
    back_region.size.height = front_region.size.height;

    if (front_region.size.width <= 32 || front_region.size.height <= 30)
    {
        return;
    }

    is_enabled = egui_view_get_enable(self) ? 1 : 0;
    severity_color = egui_view_toast_stack_severity_color(local, snapshot->severity);
    front_fill = HCW_COLOR_PANEL;
    front_border = egui_rgb_mix(local->border_color, severity_color, EGUI_ALPHA_MAKE(local->compact_mode ? 18 : 22));
    back_fill = HCW_COLOR_PANEL;
    back_border = egui_rgb_mix(local->border_color, severity_color, EGUI_ALPHA_MAKE(local->compact_mode ? 14 : 18));
    title_color = local->text_color;
    body_color = egui_rgb_mix(local->muted_text_color, local->text_color, EGUI_ALPHA_MAKE(local->compact_mode ? 22 : 26));
    meta_color = egui_rgb_mix(local->muted_text_color, local->text_color, EGUI_ALPHA_MAKE(local->compact_mode ? 30 : 28));
    glyph_color = local->surface_color;
    shadow_color = egui_rgb_mix(EGUI_COLOR_BLACK, local->border_color, EGUI_ALPHA_MAKE(10));
    action_fill = egui_rgb_mix(local->surface_color, local->accent_color, EGUI_ALPHA_MAKE(8));
    action_border = egui_rgb_mix(local->border_color, local->accent_color, EGUI_ALPHA_MAKE(16));
    action_text = egui_rgb_mix(local->accent_color, local->text_color, EGUI_ALPHA_MAKE(20));

    if (local->read_only_mode)
    {
        severity_color = egui_rgb_mix(severity_color, local->muted_text_color, EGUI_ALPHA_MAKE(42));
        front_fill = HCW_COLOR_PANEL;
        front_border = egui_rgb_mix(front_border, local->muted_text_color, EGUI_ALPHA_MAKE(30));
        back_fill = HCW_COLOR_PANEL;
        back_border = egui_rgb_mix(back_border, local->muted_text_color, EGUI_ALPHA_MAKE(32));
        title_color = egui_rgb_mix(title_color, local->muted_text_color, EGUI_ALPHA_MAKE(16));
        body_color = egui_rgb_mix(body_color, local->text_color, EGUI_ALPHA_MAKE(14));
        meta_color = egui_rgb_mix(meta_color, local->text_color, EGUI_ALPHA_MAKE(10));
        glyph_color = egui_rgb_mix(glyph_color, local->muted_text_color, EGUI_ALPHA_MAKE(22));
        shadow_color = egui_rgb_mix(shadow_color, local->surface_color, EGUI_ALPHA_MAKE(22));
        action_fill = egui_rgb_mix(action_fill, local->surface_color, EGUI_ALPHA_MAKE(40));
        action_border = egui_rgb_mix(action_border, local->muted_text_color, EGUI_ALPHA_MAKE(34));
        action_text = egui_rgb_mix(action_text, local->muted_text_color, EGUI_ALPHA_MAKE(42));
    }

    if (!is_enabled)
    {
        severity_color = egui_view_toast_stack_mix_disabled(severity_color);
        front_fill = egui_view_toast_stack_mix_disabled(front_fill);
        front_border = egui_view_toast_stack_mix_disabled(front_border);
        back_fill = egui_view_toast_stack_mix_disabled(back_fill);
        back_border = egui_view_toast_stack_mix_disabled(back_border);
        title_color = egui_view_toast_stack_mix_disabled(title_color);
        body_color = egui_view_toast_stack_mix_disabled(body_color);
        meta_color = egui_view_toast_stack_mix_disabled(meta_color);
        glyph_color = egui_view_toast_stack_mix_disabled(glyph_color);
        shadow_color = egui_view_toast_stack_mix_disabled(shadow_color);
        action_fill = egui_view_toast_stack_mix_disabled(action_fill);
        action_border = egui_view_toast_stack_mix_disabled(action_border);
        action_text = egui_view_toast_stack_mix_disabled(action_text);
    }

    egui_view_toast_stack_draw_back_card(self, local->meta_font, &back_region, snapshot->back_title ? snapshot->back_title : snapshot->title, back_fill,
                                         back_border, severity_color, meta_color, local->compact_mode);
    egui_view_toast_stack_draw_back_card(self, local->meta_font, &mid_region, snapshot->mid_title ? snapshot->mid_title : snapshot->title,
                                         egui_rgb_mix(back_fill, local->surface_color, EGUI_ALPHA_MAKE(8)),
                                         egui_rgb_mix(back_border, local->border_color, EGUI_ALPHA_MAKE(16)), severity_color, meta_color, local->compact_mode);

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, front_region.location.x + 2, front_region.location.y + 3, front_region.size.width, front_region.size.height, radius,
                                          shadow_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->compact_mode ? 12 : 16)));
    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, front_region.location.x, front_region.location.y, front_region.size.width, front_region.size.height, radius,
                                          front_fill, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->compact_mode ? 92 : 95)));
    hcw_selection_marker_draw_left(&front_region, radius, EGUI_MAX(strip_w, radius), severity_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->read_only_mode ? 72 : 92)));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, front_region.location.x, front_region.location.y, front_region.size.width, front_region.size.height, radius, 1,
                                     front_border, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->compact_mode ? 88 : 92)));

    content_x = front_region.location.x + (local->compact_mode ? 8 : 10);
    content_y = front_region.location.y + (local->compact_mode ? 6 : 8);
    content_w = front_region.size.width - (local->compact_mode ? 14 : 18);
    content_h = front_region.size.height - (local->compact_mode ? 12 : 16);
    icon_size = local->compact_mode ? 10 : 12;
    close_w = (!local->compact_mode && snapshot->closable && !local->read_only_mode && is_enabled) ? 10 : 0;
    show_close = close_w > 0 ? 1 : 0;
    title_x = content_x + icon_size + 6;
    title_w = content_w - icon_size - 6 - (show_close ? 12 : 0);
    title_slot_h = local->compact_mode ? 11 : 12;
    title_h = egui_view_toast_stack_resolve_line_height(local->font, title_slot_h);
    fallback_char_width = local->compact_mode ? 4 : 5;

    egui_canvas_draw_circle_fill(&uicode_get_core()->canvas, content_x + icon_size / 2, content_y + icon_size / 2, icon_size / 2, severity_color,
                                 egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->read_only_mode ? 66 : 94)));
    text_region.location.x = content_x;
    text_region.location.y = content_y;
    text_region.size.width = icon_size;
    text_region.size.height = icon_size;
    egui_view_toast_stack_draw_text(local->meta_font, self, egui_view_toast_stack_severity_glyph(snapshot->severity), &text_region, EGUI_ALIGN_CENTER,
                                    glyph_color);

    text_region.location.x = title_x;
    text_region.location.y = content_y + (title_slot_h - title_h) / 2;
    text_region.size.width = title_w;
    text_region.size.height = title_h;
    egui_view_toast_stack_fit_text_to_width(local->font, snapshot->title, title_label, sizeof(title_label), text_region.size.width, fallback_char_width);
    egui_view_toast_stack_draw_text(local->font, self, title_label, &text_region, EGUI_ALIGN_LEFT, title_color);

    if (show_close)
    {
        egui_dim_t close_x = content_x + content_w - 10;

        egui_canvas_draw_line(&uicode_get_core()->canvas, close_x, content_y + 2, close_x + 4, content_y + 6, 1, body_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(88)));
        egui_canvas_draw_line(&uicode_get_core()->canvas, close_x + 4, content_y + 2, close_x, content_y + 6, 1, body_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(88)));
    }

    body_y = content_y + title_slot_h + (local->compact_mode ? 3 : 5);
    body_h = local->compact_mode ? 16 : 18;
    if (body_y + body_h > content_y + content_h - (local->compact_mode ? 12 : 16))
    {
        body_h = content_y + content_h - body_y - (local->compact_mode ? 12 : 16);
    }
    if (body_h < 10)
    {
        body_h = 10;
    }

    text_region.location.x = title_x;
    text_region.location.y = body_y;
    text_region.size.width = content_w - icon_size - 6;
    text_region.size.height = body_h;
    egui_view_toast_stack_fit_text_to_width(local->font, snapshot->body, body_label, sizeof(body_label), text_region.size.width, fallback_char_width);
    egui_view_toast_stack_draw_text(local->font, self, body_label, &text_region, EGUI_ALIGN_LEFT, body_color);

    footer_gap = local->compact_mode ? 2 : 4;
    action_h = local->compact_mode ? 11 : 13;
    meta_h = egui_view_toast_stack_resolve_line_height(local->meta_font, local->compact_mode ? 10 : 12);
    footer_y = content_y + content_h - action_h;
    show_action = (snapshot->action != NULL && snapshot->action[0] != '\0' && !local->read_only_mode) ? 1 : 0;
    action_w = 18 + egui_view_toast_stack_measure_text_width(local->meta_font, snapshot->action);
    meta_w = 12 + egui_view_toast_stack_measure_text_width(local->meta_font, snapshot->meta);

    if (show_action)
    {
        if (action_w < (local->compact_mode ? 26 : 38))
        {
            action_w = local->compact_mode ? 26 : 38;
        }
        if (action_w > content_w / 2)
        {
            action_w = content_w / 2;
        }
        egui_view_toast_stack_draw_pill(local->meta_font, self, snapshot->action, title_x, footer_y, action_w, action_h, 5, action_fill,
                                        EGUI_ALPHA_MAKE(local->read_only_mode ? (local->compact_mode ? 24 : 28) : (local->compact_mode ? 82 : 90)), action_border,
                                        EGUI_ALPHA_MAKE(local->read_only_mode ? (local->compact_mode ? 30 : 34) : (local->compact_mode ? 90 : 96)), action_text,
                                        fallback_char_width);
    }

    if (snapshot->meta != NULL && snapshot->meta[0] != '\0')
    {
        egui_dim_t meta_x;
        egui_dim_t min_meta_w = local->compact_mode ? 20 : 28;

        if (meta_w < min_meta_w)
        {
            meta_w = min_meta_w;
        }
        if (meta_w > content_w / 2)
        {
            meta_w = content_w / 2;
        }
        meta_x = content_x + content_w - meta_w;
        if (show_action && meta_x < title_x + action_w + footer_gap)
        {
            meta_x = title_x + action_w + footer_gap;
            meta_w = content_x + content_w - meta_x;
        }

        egui_view_toast_stack_draw_pill(local->meta_font, self, snapshot->meta, meta_x, content_y + content_h - meta_h, meta_w, meta_h, 5,
                                        egui_rgb_mix(local->surface_color, local->border_color, EGUI_ALPHA_MAKE(20)),
                                        EGUI_ALPHA_MAKE(local->read_only_mode ? (local->compact_mode ? 38 : 42) : (local->compact_mode ? 74 : 84)),
                                        egui_rgb_mix(local->border_color, severity_color, EGUI_ALPHA_MAKE(26)),
                                        EGUI_ALPHA_MAKE(local->read_only_mode ? (local->compact_mode ? 44 : 48) : (local->compact_mode ? 86 : 92)), meta_color,
                                        fallback_char_width);
    }

    if (local->read_only_mode || !is_enabled)
    {
        egui_canvas_draw_line(&uicode_get_core()->canvas, content_x + 1, content_y + content_h, content_x + content_w - 2, content_y + content_h, 1, front_border,
                              egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(64)));
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_toast_stack_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_toast_stack_t);

    if (local->read_only_mode || !egui_view_get_enable(self))
    {
        if (egui_view_toast_stack_clear_pressed_state(self))
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
static int egui_view_toast_stack_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_toast_stack_t);

    if (local->read_only_mode || !egui_view_get_enable(self))
    {
        if (egui_view_toast_stack_clear_pressed_state(self))
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
static int egui_view_toast_stack_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_toast_stack_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_toast_stack_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_toast_stack_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_toast_stack_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_toast_stack_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_toast_stack_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_toast_stack_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_toast_stack_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_toast_stack_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_toast_stack_on_key_event,
#endif
};

void egui_view_toast_stack_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_toast_stack_t);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_toast_stack_t);
    egui_view_set_padding_all(self, 2);

    local->snapshots = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->surface_color = HCW_COLOR_SURFACE;
    local->border_color = HCW_COLOR_BORDER;
    local->text_color = HCW_COLOR_TEXT_STRONG;
    local->muted_text_color = HCW_COLOR_TEXT_MUTED;
    local->accent_color = HCW_COLOR_PRIMARY;
    local->info_color = HCW_COLOR_PRIMARY;
    local->success_color = HCW_COLOR_SUCCESS;
    local->warning_color = HCW_COLOR_WARNING;
    local->error_color = HCW_COLOR_DANGER;
    local->snapshot_count = 0;
    local->current_snapshot = 0;
    local->compact_mode = 0;
    local->read_only_mode = 0;
}
