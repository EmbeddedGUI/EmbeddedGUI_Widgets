#include "egui_view_two_pane_view.h"
#include "../../hcw_selection_marker.h"

#define TPV_HIT_NONE        0xFF
#define TPV_HIT_LAYOUT_BASE 0
#define TPV_HIT_PANE_BASE   16

#define TPV_STD_PAD_X      7
#define TPV_STD_PAD_Y      7
#define TPV_STD_RADIUS     10
#define TPV_STD_CONTROL_H  15
#define TPV_STD_CONTROL_W  31
#define TPV_STD_PANE_BTN_W 19
#define TPV_STD_GAP        5
#define TPV_STD_PANE_PAD_X 7
#define TPV_STD_PANE_PAD_Y 6
#define TPV_STD_PANE_RAD   8
#define TPV_STD_ACTION_H   13

#define TPV_COMPACT_PAD_X      5
#define TPV_COMPACT_PAD_Y      5
#define TPV_COMPACT_RADIUS     8
#define TPV_COMPACT_CONTROL_H  12
#define TPV_COMPACT_CONTROL_W  20
#define TPV_COMPACT_PANE_BTN_W 16
#define TPV_COMPACT_GAP        3
#define TPV_COMPACT_PANE_PAD_X 5
#define TPV_COMPACT_PANE_PAD_Y 4
#define TPV_COMPACT_PANE_RAD   6
#define TPV_COMPACT_ACTION_H   10

typedef struct egui_view_two_pane_view_metrics egui_view_two_pane_view_metrics_t;
struct egui_view_two_pane_view_metrics
{
    egui_region_t content;
    egui_region_t layout_tabs[EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_COUNT];
    egui_region_t pane_buttons[2];
    egui_region_t pane_area;
    egui_region_t first_pane;
    egui_region_t second_pane;
    egui_region_t divider;
    uint8_t show_first_pane;
    uint8_t show_second_pane;
};

static const char *tpv_layout_label(uint8_t layout_mode, uint8_t compact_mode)
{
    switch (layout_mode)
    {
    case EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_TALL:
        return compact_mode ? "T" : "Tall";
    case EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_SINGLE:
        return compact_mode ? "S" : "Single";
    default:
        return compact_mode ? "W" : "Wide";
    }
}

static uint8_t tpv_clamp_layout(uint8_t layout_mode)
{
    return layout_mode < EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_COUNT ? layout_mode : EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_WIDE;
}

static uint8_t tpv_clamp_pane(uint8_t pane)
{
    return pane == EGUI_VIEW_TWO_PANE_VIEW_PANE_SECOND ? EGUI_VIEW_TWO_PANE_VIEW_PANE_SECOND : EGUI_VIEW_TWO_PANE_VIEW_PANE_FIRST;
}

static uint8_t tpv_text_len(const char *text)
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

static void tpv_copy_elided(char *buffer, uint8_t buffer_size, const char *text, uint8_t max_chars)
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

    length = tpv_text_len(text);
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

static egui_dim_t tpv_measure_text_width(const egui_font_t *font, const char *text)
{
    egui_dim_t text_width = 0;
    egui_dim_t text_height = 0;

    if (font == NULL || font->api == NULL || font->api->get_str_size == NULL || text == NULL || text[0] == '\0')
    {
        return 0;
    }

    font->api->get_str_size(font, text, 0, 0, &text_width, &text_height);
    return text_width;
}

static void tpv_fit_text_to_width(const egui_font_t *font, const char *text, char *buffer, uint8_t buffer_size, egui_dim_t max_width,
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

    max_chars = tpv_text_len(text);
    tpv_copy_elided(buffer, buffer_size, text, max_chars);
    while (max_chars > 0)
    {
        egui_dim_t text_width = tpv_measure_text_width(font, buffer);

        if (text_width <= 0)
        {
            text_width = (egui_dim_t)tpv_text_len(buffer) * fallback_char_width;
        }
        if (text_width <= max_width)
        {
            break;
        }
        max_chars--;
        tpv_copy_elided(buffer, buffer_size, text, max_chars);
    }
}

static egui_color_t tpv_tone_color(egui_view_two_pane_view_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_TWO_PANE_VIEW_TONE_SUCCESS:
        return local->success_color;
    case EGUI_VIEW_TWO_PANE_VIEW_TONE_WARNING:
        return local->warning_color;
    case EGUI_VIEW_TWO_PANE_VIEW_TONE_NEUTRAL:
        return local->neutral_color;
    default:
        return local->accent_color;
    }
}

static egui_color_t tpv_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(44));
}

static uint8_t tpv_clear_pressed_state(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_two_pane_view_t);
    uint8_t had_pressed = self->is_pressed || local->pressed_target != TPV_HIT_NONE;

    local->pressed_target = TPV_HIT_NONE;
    egui_view_set_pressed(self, false);
    return had_pressed;
}

static uint8_t tpv_region_contains(const egui_region_t *region, egui_dim_t x, egui_dim_t y)
{
    if (region->size.width <= 0 || region->size.height <= 0)
    {
        return 0;
    }
    return x >= region->location.x && y >= region->location.y && x < region->location.x + region->size.width &&
           y < region->location.y + region->size.height;
}

static void tpv_empty_region(egui_region_t *region)
{
    region->location.x = 0;
    region->location.y = 0;
    region->size.width = 0;
    region->size.height = 0;
}

static void tpv_get_metrics(egui_view_two_pane_view_t *local, egui_view_t *self, egui_view_two_pane_view_metrics_t *metrics)
{
    egui_region_t work;
    egui_dim_t pad_x = local->compact_mode ? TPV_COMPACT_PAD_X : TPV_STD_PAD_X;
    egui_dim_t pad_y = local->compact_mode ? TPV_COMPACT_PAD_Y : TPV_STD_PAD_Y;
    egui_dim_t control_h = local->compact_mode ? TPV_COMPACT_CONTROL_H : TPV_STD_CONTROL_H;
    egui_dim_t control_w = local->compact_mode ? TPV_COMPACT_CONTROL_W : TPV_STD_CONTROL_W;
    egui_dim_t pane_btn_w = local->compact_mode ? TPV_COMPACT_PANE_BTN_W : TPV_STD_PANE_BTN_W;
    egui_dim_t gap = local->compact_mode ? TPV_COMPACT_GAP : TPV_STD_GAP;
    egui_dim_t pane_y;
    egui_dim_t pane_h;
    egui_dim_t pane_w;
    uint8_t i;

    egui_view_get_work_region(self, &work);
    metrics->content.location.x = work.location.x + pad_x;
    metrics->content.location.y = work.location.y + pad_y;
    metrics->content.size.width = work.size.width - pad_x * 2;
    metrics->content.size.height = work.size.height - pad_y * 2;
    if (metrics->content.size.width < 0)
    {
        metrics->content.size.width = 0;
    }
    if (metrics->content.size.height < 0)
    {
        metrics->content.size.height = 0;
    }

    for (i = 0; i < EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_COUNT; i++)
    {
        metrics->layout_tabs[i].location.x = metrics->content.location.x + (control_w + 1) * i;
        metrics->layout_tabs[i].location.y = metrics->content.location.y;
        metrics->layout_tabs[i].size.width = control_w;
        metrics->layout_tabs[i].size.height = control_h;
    }

    metrics->pane_buttons[1].location.x = metrics->content.location.x + metrics->content.size.width - pane_btn_w;
    metrics->pane_buttons[1].location.y = metrics->content.location.y;
    metrics->pane_buttons[1].size.width = pane_btn_w;
    metrics->pane_buttons[1].size.height = control_h;
    metrics->pane_buttons[0].location.x = metrics->pane_buttons[1].location.x - pane_btn_w - 1;
    metrics->pane_buttons[0].location.y = metrics->content.location.y;
    metrics->pane_buttons[0].size.width = pane_btn_w;
    metrics->pane_buttons[0].size.height = control_h;

    pane_y = metrics->content.location.y + control_h + gap;
    pane_h = metrics->content.size.height - control_h - gap;
    if (pane_h < 0)
    {
        pane_h = 0;
    }
    metrics->pane_area.location.x = metrics->content.location.x;
    metrics->pane_area.location.y = pane_y;
    metrics->pane_area.size.width = metrics->content.size.width;
    metrics->pane_area.size.height = pane_h;

    tpv_empty_region(&metrics->first_pane);
    tpv_empty_region(&metrics->second_pane);
    tpv_empty_region(&metrics->divider);
    metrics->show_first_pane = 0;
    metrics->show_second_pane = 0;

    if (pane_h <= 0 || metrics->content.size.width <= 0)
    {
        return;
    }

    switch (local->layout_mode)
    {
    case EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_TALL:
        pane_h = (metrics->pane_area.size.height - gap) / 2;
        metrics->first_pane = metrics->pane_area;
        metrics->first_pane.size.height = pane_h;
        metrics->second_pane = metrics->pane_area;
        metrics->second_pane.location.y += pane_h + gap;
        metrics->second_pane.size.height = metrics->pane_area.size.height - pane_h - gap;
        metrics->divider.location.x = metrics->pane_area.location.x;
        metrics->divider.location.y = metrics->first_pane.location.y + metrics->first_pane.size.height + gap / 2;
        metrics->divider.size.width = metrics->pane_area.size.width;
        metrics->divider.size.height = 1;
        metrics->show_first_pane = 1;
        metrics->show_second_pane = metrics->second_pane.size.height > 0 ? 1 : 0;
        break;
    case EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_SINGLE:
        if (local->single_pane == EGUI_VIEW_TWO_PANE_VIEW_PANE_SECOND)
        {
            metrics->second_pane = metrics->pane_area;
            metrics->show_second_pane = 1;
        }
        else
        {
            metrics->first_pane = metrics->pane_area;
            metrics->show_first_pane = 1;
        }
        break;
    default:
        pane_w = (metrics->pane_area.size.width - gap) / 2;
        metrics->first_pane = metrics->pane_area;
        metrics->first_pane.size.width = pane_w;
        metrics->second_pane = metrics->pane_area;
        metrics->second_pane.location.x += pane_w + gap;
        metrics->second_pane.size.width = metrics->pane_area.size.width - pane_w - gap;
        metrics->divider.location.x = metrics->first_pane.location.x + metrics->first_pane.size.width + gap / 2;
        metrics->divider.location.y = metrics->pane_area.location.y;
        metrics->divider.size.width = 1;
        metrics->divider.size.height = metrics->pane_area.size.height;
        metrics->show_first_pane = 1;
        metrics->show_second_pane = metrics->second_pane.size.width > 0 ? 1 : 0;
        break;
    }
}

static void tpv_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align, egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (font == NULL || text == NULL || text[0] == '\0' || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, align, color, self->alpha);
}

static void tpv_draw_chip(egui_view_t *self, egui_view_two_pane_view_t *local, const egui_region_t *region, const char *label, uint8_t selected,
                          uint8_t pressed)
{
    egui_color_t fill = selected ? local->accent_color : HCW_COLOR_PANEL;
    egui_color_t border = selected ? local->accent_color : local->border_color;
    egui_color_t text = selected ? EGUI_COLOR_WHITE : local->muted_text_color;
    egui_alpha_t alpha = EGUI_ALPHA_MAKE(selected ? 96 : 96);

    if (local->read_only_mode || !egui_view_get_enable(self))
    {
        fill = tpv_mix_disabled(fill);
        border = tpv_mix_disabled(border);
        text = tpv_mix_disabled(text);
        alpha = EGUI_ALPHA_MAKE(selected ? 42 : 54);
    }
    if (pressed)
    {
        fill = egui_rgb_mix(fill, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(16));
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region->location.x, region->location.y, region->size.width, region->size.height,
                                          region->size.height / 2, fill, egui_color_alpha_mix(self->alpha, alpha));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, region->location.x, region->location.y, region->size.width, region->size.height,
                                     region->size.height / 2, 1, border, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(selected ? 78 : 42)));
    tpv_draw_text(local->meta_font, self, label, region, EGUI_ALIGN_CENTER, text);
}

static void tpv_draw_pane(egui_view_t *self, egui_view_two_pane_view_t *local, const egui_view_two_pane_view_pane_t *pane, const egui_region_t *region,
                          uint8_t active_pane)
{
    egui_dim_t pad_x = local->compact_mode ? TPV_COMPACT_PANE_PAD_X : TPV_STD_PANE_PAD_X;
    egui_dim_t pad_y = local->compact_mode ? TPV_COMPACT_PANE_PAD_Y : TPV_STD_PANE_PAD_Y;
    egui_dim_t action_h = local->compact_mode ? TPV_COMPACT_ACTION_H : TPV_STD_ACTION_H;
    egui_dim_t radius = local->compact_mode ? TPV_COMPACT_PANE_RAD : TPV_STD_PANE_RAD;
    egui_dim_t marker_pad_x;
    egui_dim_t fallback_char_width = local->compact_mode ? 4 : 5;
    egui_color_t tone;
    egui_color_t fill;
    egui_color_t border;
    egui_color_t text;
    egui_color_t muted;
    egui_region_t text_region;
    egui_dim_t cursor_y;
    egui_dim_t body_h;
    egui_dim_t action_w;
    char label[36];
    uint8_t tiny_compact_pane;

    if (pane == NULL || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    tiny_compact_pane = (uint8_t)(local->compact_mode && region->size.height < 50);

    tone = tpv_tone_color(local, pane->tone);
    fill = HCW_COLOR_PANEL;
    border = pane->emphasized || active_pane ? egui_rgb_mix(local->border_color, tone, EGUI_ALPHA_MAKE(28)) : local->border_color;
    text = local->text_color;
    muted = local->muted_text_color;

    if (local->read_only_mode || !egui_view_get_enable(self))
    {
        tone = tpv_mix_disabled(tone);
        fill = tpv_mix_disabled(fill);
        border = tpv_mix_disabled(border);
        text = tpv_mix_disabled(text);
        muted = tpv_mix_disabled(muted);
    }
    marker_pad_x = radius + (local->compact_mode ? 3 : 4);
    if (pad_x < marker_pad_x)
    {
        pad_x = marker_pad_x;
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region->location.x, region->location.y, region->size.width, region->size.height,
                                          radius, fill, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(98)));
    hcw_selection_marker_draw_left(region, radius, radius, tone, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(active_pane ? 96 : 82)));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, region->location.x, region->location.y, region->size.width, region->size.height, radius, 1,
                                     border, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(78)));

    text_region.location.x = region->location.x + pad_x;
    text_region.location.y = region->location.y + pad_y;
    text_region.size.width = region->size.width - pad_x * 2;
    text_region.size.height = local->compact_mode ? 8 : 10;
    tpv_fit_text_to_width(local->meta_font, pane->eyebrow, label, sizeof(label), text_region.size.width, fallback_char_width);
    tpv_draw_text(local->meta_font, self, label, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, muted);

    cursor_y = text_region.location.y + text_region.size.height + (local->compact_mode ? 1 : 2);
    text_region.location.y = cursor_y;
    text_region.size.height = local->compact_mode ? 10 : 12;
    tpv_fit_text_to_width(local->font, pane->title, label, sizeof(label), text_region.size.width, fallback_char_width);
    tpv_draw_text(local->font, self, label, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text);

    cursor_y += text_region.size.height + (local->compact_mode ? 1 : 2);
    text_region.location.y = cursor_y;
    text_region.size.height = local->compact_mode ? 8 : 9;
    if (tiny_compact_pane)
    {
        return;
    }
    tpv_fit_text_to_width(local->meta_font, pane->meta, label, sizeof(label), text_region.size.width, fallback_char_width);
    tpv_draw_text(local->meta_font, self, label, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, muted);

    cursor_y += text_region.size.height + (local->compact_mode ? 2 : 3);
    body_h = local->compact_mode ? 8 : 9;
    if (cursor_y + body_h < region->location.y + region->size.height - action_h - pad_y)
    {
        text_region.location.y = cursor_y;
        text_region.size.height = body_h;
        tpv_fit_text_to_width(local->meta_font, pane->body_primary, label, sizeof(label), text_region.size.width, fallback_char_width);
        tpv_draw_text(local->meta_font, self, label, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text);
    }

    if (!local->compact_mode && cursor_y + body_h * 2 + 2 < region->location.y + region->size.height - action_h - pad_y)
    {
        text_region.location.y = cursor_y + body_h + 2;
        text_region.size.height = body_h;
        tpv_fit_text_to_width(local->meta_font, pane->body_secondary, label, sizeof(label), text_region.size.width, fallback_char_width);
        tpv_draw_text(local->meta_font, self, label, &text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text);
    }

    action_w = tpv_measure_text_width(local->meta_font, pane->action) + (local->compact_mode ? 10 : 14);
    if (action_w < (local->compact_mode ? 28 : 36))
    {
        action_w = local->compact_mode ? 28 : 36;
    }
    if (action_w > region->size.width - pad_x * 2)
    {
        action_w = region->size.width - pad_x * 2;
    }

    text_region.location.x = region->location.x + pad_x;
    text_region.location.y = region->location.y + region->size.height - pad_y - action_h;
    text_region.size.width = action_w;
    text_region.size.height = action_h;
    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, text_region.location.x, text_region.location.y, text_region.size.width,
                                          text_region.size.height, text_region.size.height / 2, egui_rgb_mix(HCW_COLOR_PANEL, tone, EGUI_ALPHA_MAKE(10)),
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(96)));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, text_region.location.x, text_region.location.y, text_region.size.width,
                                     text_region.size.height, text_region.size.height / 2, 1, egui_rgb_mix(local->border_color, tone, EGUI_ALPHA_MAKE(22)),
                                     egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(78)));
    tpv_fit_text_to_width(local->meta_font, pane->action, label, sizeof(label), text_region.size.width - 4, fallback_char_width);
    tpv_draw_text(local->meta_font, self, label, &text_region, EGUI_ALIGN_CENTER, tone);
}

static void egui_view_two_pane_view_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_two_pane_view_t);
    egui_view_two_pane_view_metrics_t metrics;
    egui_region_t work;
    egui_color_t outer_fill = local->surface_color;
    egui_color_t outer_border = local->border_color;
    egui_color_t divider_color = egui_rgb_mix(local->border_color, local->muted_text_color, EGUI_ALPHA_MAKE(18));
    uint8_t radius = local->compact_mode ? TPV_COMPACT_RADIUS : TPV_STD_RADIUS;
    uint8_t i;

    if (local->read_only_mode || !egui_view_get_enable(self))
    {
        outer_fill = tpv_mix_disabled(outer_fill);
        outer_border = tpv_mix_disabled(outer_border);
        divider_color = tpv_mix_disabled(divider_color);
    }

    tpv_get_metrics(local, self, &metrics);
    egui_view_get_work_region(self, &work);

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, work.location.x, work.location.y, work.size.width, work.size.height, radius, outer_fill,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(98)));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, work.location.x, work.location.y, work.size.width, work.size.height, radius, 1, outer_border,
                                     egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(78)));

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (self->is_focused)
    {
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, work.location.x + 1, work.location.y + 1, work.size.width - 2, work.size.height - 2,
                                         radius, 1, local->accent_color,
                                         egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(78)));
    }
#endif

    for (i = 0; i < EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_COUNT; i++)
    {
        tpv_draw_chip(self, local, &metrics.layout_tabs[i], tpv_layout_label(i, local->compact_mode), i == local->layout_mode,
                      local->pressed_target == (TPV_HIT_LAYOUT_BASE + i));
    }

    tpv_draw_chip(self, local, &metrics.pane_buttons[0], local->compact_mode ? "1" : "P1", local->single_pane == EGUI_VIEW_TWO_PANE_VIEW_PANE_FIRST,
                  local->pressed_target == TPV_HIT_PANE_BASE);
    tpv_draw_chip(self, local, &metrics.pane_buttons[1], local->compact_mode ? "2" : "P2", local->single_pane == EGUI_VIEW_TWO_PANE_VIEW_PANE_SECOND,
                  local->pressed_target == TPV_HIT_PANE_BASE + 1);

    if (metrics.divider.size.width > 0 && metrics.divider.size.height > 0)
    {
        egui_canvas_draw_rectangle_fill(&uicode_get_core()->canvas, metrics.divider.location.x, metrics.divider.location.y, metrics.divider.size.width,
                                        metrics.divider.size.height, divider_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(76)));
    }

    if (metrics.show_first_pane)
    {
        tpv_draw_pane(self, local, local->first_pane, &metrics.first_pane, local->layout_mode == EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_SINGLE);
    }
    if (metrics.show_second_pane)
    {
        tpv_draw_pane(self, local, local->second_pane, &metrics.second_pane, local->layout_mode == EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_SINGLE);
    }
}

static uint8_t tpv_hit_target(egui_view_two_pane_view_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    egui_view_two_pane_view_metrics_t metrics;
    uint8_t i;

    tpv_get_metrics(local, self, &metrics);
    for (i = 0; i < EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_COUNT; i++)
    {
        if (tpv_region_contains(&metrics.layout_tabs[i], x, y))
        {
            return TPV_HIT_LAYOUT_BASE + i;
        }
    }
    for (i = 0; i < 2; i++)
    {
        if (tpv_region_contains(&metrics.pane_buttons[i], x, y))
        {
            return TPV_HIT_PANE_BASE + i;
        }
    }
    return TPV_HIT_NONE;
}

static void egui_view_two_pane_view_set_layout_mode_inner(egui_view_t *self, uint8_t layout_mode, uint8_t notify)
{
    EGUI_LOCAL_INIT(egui_view_two_pane_view_t);

    layout_mode = tpv_clamp_layout(layout_mode);
    if (local->layout_mode == layout_mode)
    {
        return;
    }

    local->layout_mode = layout_mode;
    if (notify && local->on_layout_changed != NULL)
    {
        local->on_layout_changed(self, local->layout_mode);
    }
    egui_view_invalidate(self);
}

static void egui_view_two_pane_view_set_single_pane_inner(egui_view_t *self, uint8_t pane, uint8_t notify)
{
    EGUI_LOCAL_INIT(egui_view_two_pane_view_t);

    pane = tpv_clamp_pane(pane);
    if (local->single_pane == pane)
    {
        return;
    }

    local->single_pane = pane;
    if (notify && local->on_pane_changed != NULL)
    {
        local->on_pane_changed(self, local->single_pane);
    }
    egui_view_invalidate(self);
}

static void tpv_activate_target(egui_view_t *self, uint8_t target)
{
    if (target < TPV_HIT_LAYOUT_BASE + EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_COUNT)
    {
        egui_view_two_pane_view_set_layout_mode_inner(self, target - TPV_HIT_LAYOUT_BASE, 1);
    }
    else if (target == TPV_HIT_PANE_BASE || target == TPV_HIT_PANE_BASE + 1)
    {
        egui_view_two_pane_view_set_single_pane_inner(self, target - TPV_HIT_PANE_BASE, 1);
        egui_view_two_pane_view_set_layout_mode_inner(self, EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_SINGLE, 1);
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_two_pane_view_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_two_pane_view_t);
    egui_dim_t local_x = event->location.x - self->region_screen.location.x;
    egui_dim_t local_y = event->location.y - self->region_screen.location.y;
    uint8_t hit_target;
    uint8_t had_pressed;

    if (!egui_view_get_enable(self) || local->read_only_mode)
    {
        if (tpv_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_target = tpv_hit_target(local, self, local_x, local_y);
        if (hit_target == TPV_HIT_NONE)
        {
            return 0;
        }
        local->pressed_target = hit_target;
        egui_view_set_pressed(self, true);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_target == TPV_HIT_NONE)
        {
            return 0;
        }
        hit_target = tpv_hit_target(local, self, local_x, local_y);
        egui_view_set_pressed(self, hit_target == local->pressed_target ? true : false);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
        had_pressed = local->pressed_target != TPV_HIT_NONE;
        hit_target = tpv_hit_target(local, self, local_x, local_y);
        if (hit_target == local->pressed_target && hit_target != TPV_HIT_NONE)
        {
            tpv_activate_target(self, hit_target);
        }
        tpv_clear_pressed_state(self);
        egui_view_invalidate(self);
        return had_pressed || hit_target != TPV_HIT_NONE;
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (tpv_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return 1;
    default:
        return 0;
    }
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_two_pane_view_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_two_pane_view_t);
    uint8_t next_layout;

    if (!egui_view_get_enable(self) || local->read_only_mode)
    {
        if (tpv_clear_pressed_state(self))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }
    if (event->type != EGUI_KEY_EVENT_ACTION_UP)
    {
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_LEFT:
    case EGUI_KEY_CODE_UP:
        next_layout = local->layout_mode > 0 ? local->layout_mode - 1 : 0;
        egui_view_two_pane_view_set_layout_mode_inner(self, next_layout, 1);
        return 1;
    case EGUI_KEY_CODE_RIGHT:
    case EGUI_KEY_CODE_DOWN:
        next_layout = local->layout_mode + 1 < EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_COUNT ? local->layout_mode + 1 :
                                                                                EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_COUNT - 1;
        egui_view_two_pane_view_set_layout_mode_inner(self, next_layout, 1);
        return 1;
    case EGUI_KEY_CODE_HOME:
        egui_view_two_pane_view_set_layout_mode_inner(self, EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_WIDE, 1);
        return 1;
    case EGUI_KEY_CODE_END:
        egui_view_two_pane_view_set_layout_mode_inner(self, EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_SINGLE, 1);
        return 1;
    case EGUI_KEY_CODE_TAB:
        next_layout = local->layout_mode + 1;
        if (next_layout >= EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_COUNT)
        {
            next_layout = 0;
        }
        egui_view_two_pane_view_set_layout_mode_inner(self, next_layout, 1);
        return 1;
    case EGUI_KEY_CODE_ENTER:
    case EGUI_KEY_CODE_SPACE:
        egui_view_two_pane_view_set_layout_mode_inner(self, EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_SINGLE, 1);
        egui_view_two_pane_view_set_single_pane_inner(self, local->single_pane == EGUI_VIEW_TWO_PANE_VIEW_PANE_FIRST ?
                                                                    EGUI_VIEW_TWO_PANE_VIEW_PANE_SECOND :
                                                                    EGUI_VIEW_TWO_PANE_VIEW_PANE_FIRST,
                                                      1);
        return 1;
    default:
        return egui_view_on_key_event(self, event);
    }
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_two_pane_view_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (tpv_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_two_pane_view_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (tpv_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_two_pane_view_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_two_pane_view_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_two_pane_view_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_two_pane_view_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_two_pane_view_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_two_pane_view_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_two_pane_view_on_key_event,
#endif
};

void egui_view_two_pane_view_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_two_pane_view_t);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_two_pane_view_t);
    egui_view_set_padding_all(self, 2);

    local->first_pane = NULL;
    local->second_pane = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->on_layout_changed = NULL;
    local->on_pane_changed = NULL;
    local->surface_color = HCW_COLOR_SURFACE;
    local->border_color = HCW_COLOR_BORDER;
    local->section_color = HCW_COLOR_SURFACE_SUBTLE;
    local->text_color = HCW_COLOR_TEXT;
    local->muted_text_color = HCW_COLOR_NEUTRAL;
    local->accent_color = HCW_COLOR_PRIMARY;
    local->success_color = HCW_COLOR_SUCCESS;
    local->warning_color = HCW_COLOR_WARNING;
    local->neutral_color = HCW_COLOR_TEXT_MUTED;
    local->layout_mode = EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_WIDE;
    local->single_pane = EGUI_VIEW_TWO_PANE_VIEW_PANE_FIRST;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_target = TPV_HIT_NONE;

    egui_view_set_view_name(self, "egui_view_two_pane_view");
}

void egui_view_two_pane_view_set_panes(egui_view_t *self, const egui_view_two_pane_view_pane_t *first_pane,
                                       const egui_view_two_pane_view_pane_t *second_pane)
{
    EGUI_LOCAL_INIT(egui_view_two_pane_view_t);

    local->first_pane = first_pane;
    local->second_pane = second_pane;
    tpv_clear_pressed_state(self);
    egui_view_invalidate(self);
}

void egui_view_two_pane_view_set_layout_mode(egui_view_t *self, uint8_t layout_mode)
{
    egui_view_two_pane_view_set_layout_mode_inner(self, layout_mode, 0);
}

uint8_t egui_view_two_pane_view_get_layout_mode(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_two_pane_view_t);

    return local->layout_mode;
}

void egui_view_two_pane_view_set_single_pane(egui_view_t *self, uint8_t pane)
{
    egui_view_two_pane_view_set_single_pane_inner(self, pane, 0);
}

uint8_t egui_view_two_pane_view_get_single_pane(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_two_pane_view_t);

    return local->single_pane;
}

void egui_view_two_pane_view_toggle_single_pane(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_two_pane_view_t);

    egui_view_two_pane_view_set_single_pane_inner(self, local->single_pane == EGUI_VIEW_TWO_PANE_VIEW_PANE_FIRST ?
                                                               EGUI_VIEW_TWO_PANE_VIEW_PANE_SECOND :
                                                               EGUI_VIEW_TWO_PANE_VIEW_PANE_FIRST,
                                                 0);
}

void egui_view_two_pane_view_set_on_layout_changed_listener(egui_view_t *self, egui_view_on_two_pane_view_layout_changed_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_two_pane_view_t);

    local->on_layout_changed = listener;
}

void egui_view_two_pane_view_set_on_pane_changed_listener(egui_view_t *self, egui_view_on_two_pane_view_pane_changed_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_two_pane_view_t);

    local->on_pane_changed = listener;
}

void egui_view_two_pane_view_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_two_pane_view_t);

    local->font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_invalidate(self);
}

void egui_view_two_pane_view_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_two_pane_view_t);

    local->meta_font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_invalidate(self);
}

void egui_view_two_pane_view_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_two_pane_view_t);

    compact_mode = compact_mode ? 1 : 0;
    if (local->compact_mode == compact_mode)
    {
        return;
    }
    local->compact_mode = compact_mode;
    tpv_clear_pressed_state(self);
    egui_view_invalidate(self);
}

void egui_view_two_pane_view_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_two_pane_view_t);

    read_only_mode = read_only_mode ? 1 : 0;
    if (local->read_only_mode == read_only_mode)
    {
        return;
    }
    local->read_only_mode = read_only_mode;
    tpv_clear_pressed_state(self);
    egui_view_invalidate(self);
}

void egui_view_two_pane_view_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t section_color,
                                         egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t success_color,
                                         egui_color_t warning_color, egui_color_t neutral_color)
{
    EGUI_LOCAL_INIT(egui_view_two_pane_view_t);

    local->surface_color = surface_color;
    local->border_color = border_color;
    local->section_color = section_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->neutral_color = neutral_color;
    egui_view_invalidate(self);
}
