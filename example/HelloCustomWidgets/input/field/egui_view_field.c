#include "egui_view_field.h"

#include <string.h>

#define HCW_FIELD_STANDARD_FIELD_RADIUS         10
#define HCW_FIELD_STANDARD_TOP_PAD              6
#define HCW_FIELD_STANDARD_SIDE_PAD             2
#define HCW_FIELD_STANDARD_LABEL_HEIGHT         12
#define HCW_FIELD_STANDARD_LABEL_GAP            6
#define HCW_FIELD_STANDARD_FIELD_HEIGHT         34
#define HCW_FIELD_STANDARD_FIELD_PAD_X          10
#define HCW_FIELD_STANDARD_FIELD_GAP            5
#define HCW_FIELD_STANDARD_INFO_SIZE            18
#define HCW_FIELD_STANDARD_INFO_RADIUS          6
#define HCW_FIELD_STANDARD_INFO_GAP            10
#define HCW_FIELD_STANDARD_BUBBLE_HEIGHT        36
#define HCW_FIELD_STANDARD_BUBBLE_MIN_HEIGHT    24
#define HCW_FIELD_STANDARD_BUBBLE_RADIUS        8
#define HCW_FIELD_STANDARD_BUBBLE_PAD_X         10
#define HCW_FIELD_STANDARD_BUBBLE_PAD_Y         8
#define HCW_FIELD_STANDARD_BUBBLE_GAP           6
#define HCW_FIELD_STANDARD_BUBBLE_ARROW_WIDTH   12
#define HCW_FIELD_STANDARD_BUBBLE_ARROW_HEIGHT  6
#define HCW_FIELD_STANDARD_HELPER_HEIGHT        10
#define HCW_FIELD_STANDARD_VALIDATION_HEIGHT    10
#define HCW_FIELD_STANDARD_VALIDATION_GAP       3
#define HCW_FIELD_STANDARD_VALIDATION_DOT_SIZE  5

#define HCW_FIELD_COMPACT_FIELD_RADIUS         8
#define HCW_FIELD_COMPACT_TOP_PAD              4
#define HCW_FIELD_COMPACT_SIDE_PAD             1
#define HCW_FIELD_COMPACT_LABEL_HEIGHT         10
#define HCW_FIELD_COMPACT_LABEL_GAP            4
#define HCW_FIELD_COMPACT_FIELD_HEIGHT         28
#define HCW_FIELD_COMPACT_FIELD_PAD_X          8
#define HCW_FIELD_COMPACT_FIELD_GAP            4
#define HCW_FIELD_COMPACT_INFO_SIZE            14
#define HCW_FIELD_COMPACT_INFO_RADIUS          5
#define HCW_FIELD_COMPACT_INFO_GAP             8
#define HCW_FIELD_COMPACT_BUBBLE_HEIGHT        26
#define HCW_FIELD_COMPACT_BUBBLE_MIN_HEIGHT    18
#define HCW_FIELD_COMPACT_BUBBLE_RADIUS        6
#define HCW_FIELD_COMPACT_BUBBLE_PAD_X         8
#define HCW_FIELD_COMPACT_BUBBLE_PAD_Y         6
#define HCW_FIELD_COMPACT_BUBBLE_GAP           4
#define HCW_FIELD_COMPACT_BUBBLE_ARROW_WIDTH   8
#define HCW_FIELD_COMPACT_BUBBLE_ARROW_HEIGHT  4
#define HCW_FIELD_COMPACT_HELPER_HEIGHT        9
#define HCW_FIELD_COMPACT_VALIDATION_HEIGHT    9
#define HCW_FIELD_COMPACT_VALIDATION_GAP       2
#define HCW_FIELD_COMPACT_VALIDATION_DOT_SIZE  4

typedef struct hcw_field_metrics hcw_field_metrics_t;
struct hcw_field_metrics
{
    egui_region_t region;
    egui_region_t label_row_region;
    egui_region_t label_region;
    egui_region_t info_region;
    egui_region_t bubble_region;
    egui_region_t bubble_title_region;
    egui_region_t bubble_body_region;
    egui_region_t field_region;
    egui_region_t field_text_region;
    egui_region_t helper_region;
    egui_region_t validation_region;
    egui_region_t validation_dot_region;
    egui_dim_t required_x;
    egui_dim_t arrow_center_x;
    uint8_t show_info;
    uint8_t show_bubble;
    uint8_t show_helper;
    uint8_t show_validation;
};

static const char *hcw_field_default_text(void)
{
    return "";
}

static const egui_font_t *hcw_field_default_font(void)
{
    return (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
}

static const egui_font_t *hcw_field_default_icon_font(void)
{
    return EGUI_FONT_ICON_MS_16;
}

static uint8_t hcw_field_has_text(const char *text)
{
    return text != NULL && text[0] != '\0';
}

static uint8_t hcw_field_show_info(const hcw_field_t *local)
{
    return (uint8_t)(hcw_field_has_text(local->info_title) || hcw_field_has_text(local->info_body));
}

static egui_color_t hcw_field_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x8A96A3), 58);
}

static egui_color_t hcw_field_validation_tone(hcw_field_t *local)
{
    switch (local->validation_state)
    {
    case HCW_FIELD_VALIDATION_SUCCESS:
        return local->success_color;
    case HCW_FIELD_VALIDATION_WARNING:
        return local->warning_color;
    case HCW_FIELD_VALIDATION_ERROR:
        return local->error_color;
    default:
        return local->muted_text_color;
    }
}

static uint8_t hcw_field_clear_pressed_state(egui_view_t *self)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    uint8_t was_pressed = egui_view_get_pressed(self) ? 1 : 0;
    uint8_t had_pressed = (uint8_t)(was_pressed || local->pressed_part != HCW_FIELD_PART_NONE);

    if (!had_pressed)
    {
        return 0;
    }

    local->pressed_part = HCW_FIELD_PART_NONE;
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

static egui_dim_t hcw_field_top_pad(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_TOP_PAD : HCW_FIELD_STANDARD_TOP_PAD;
}

static egui_dim_t hcw_field_side_pad(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_SIDE_PAD : HCW_FIELD_STANDARD_SIDE_PAD;
}

static egui_dim_t hcw_field_label_height(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_LABEL_HEIGHT : HCW_FIELD_STANDARD_LABEL_HEIGHT;
}

static egui_dim_t hcw_field_label_gap(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_LABEL_GAP : HCW_FIELD_STANDARD_LABEL_GAP;
}

static egui_dim_t hcw_field_field_height(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_FIELD_HEIGHT : HCW_FIELD_STANDARD_FIELD_HEIGHT;
}

static egui_dim_t hcw_field_field_pad_x(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_FIELD_PAD_X : HCW_FIELD_STANDARD_FIELD_PAD_X;
}

static egui_dim_t hcw_field_field_gap(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_FIELD_GAP : HCW_FIELD_STANDARD_FIELD_GAP;
}

static egui_dim_t hcw_field_info_size(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_INFO_SIZE : HCW_FIELD_STANDARD_INFO_SIZE;
}

static egui_dim_t hcw_field_info_radius(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_INFO_RADIUS : HCW_FIELD_STANDARD_INFO_RADIUS;
}

static egui_dim_t hcw_field_info_gap(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_INFO_GAP : HCW_FIELD_STANDARD_INFO_GAP;
}

static egui_dim_t hcw_field_bubble_height(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_BUBBLE_HEIGHT : HCW_FIELD_STANDARD_BUBBLE_HEIGHT;
}

static egui_dim_t hcw_field_bubble_min_height(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_BUBBLE_MIN_HEIGHT : HCW_FIELD_STANDARD_BUBBLE_MIN_HEIGHT;
}

static egui_dim_t hcw_field_bubble_radius(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_BUBBLE_RADIUS : HCW_FIELD_STANDARD_BUBBLE_RADIUS;
}

static egui_dim_t hcw_field_bubble_pad_x(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_BUBBLE_PAD_X : HCW_FIELD_STANDARD_BUBBLE_PAD_X;
}

static egui_dim_t hcw_field_bubble_pad_y(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_BUBBLE_PAD_Y : HCW_FIELD_STANDARD_BUBBLE_PAD_Y;
}

static egui_dim_t hcw_field_bubble_gap(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_BUBBLE_GAP : HCW_FIELD_STANDARD_BUBBLE_GAP;
}

static egui_dim_t hcw_field_bubble_arrow_width(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_BUBBLE_ARROW_WIDTH : HCW_FIELD_STANDARD_BUBBLE_ARROW_WIDTH;
}

static egui_dim_t hcw_field_bubble_arrow_height(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_BUBBLE_ARROW_HEIGHT : HCW_FIELD_STANDARD_BUBBLE_ARROW_HEIGHT;
}

static egui_dim_t hcw_field_helper_height(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_HELPER_HEIGHT : HCW_FIELD_STANDARD_HELPER_HEIGHT;
}

static egui_dim_t hcw_field_validation_height(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_VALIDATION_HEIGHT : HCW_FIELD_STANDARD_VALIDATION_HEIGHT;
}

static egui_dim_t hcw_field_validation_gap(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_VALIDATION_GAP : HCW_FIELD_STANDARD_VALIDATION_GAP;
}

static egui_dim_t hcw_field_validation_dot_size(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_VALIDATION_DOT_SIZE : HCW_FIELD_STANDARD_VALIDATION_DOT_SIZE;
}

static egui_dim_t hcw_field_radius(uint8_t compact_mode)
{
    return compact_mode ? HCW_FIELD_COMPACT_FIELD_RADIUS : HCW_FIELD_STANDARD_FIELD_RADIUS;
}

static const egui_font_t *hcw_field_get_font(const hcw_field_t *local)
{
    return local->font == NULL ? hcw_field_default_font() : local->font;
}

static const egui_font_t *hcw_field_get_meta_font(const hcw_field_t *local)
{
    return local->meta_font == NULL ? hcw_field_default_font() : local->meta_font;
}

static const egui_font_t *hcw_field_get_icon_font(const hcw_field_t *local)
{
    return local->icon_font == NULL ? hcw_field_default_icon_font() : local->icon_font;
}

static void hcw_field_measure_text(const egui_font_t *font, const char *text, egui_dim_t *width, egui_dim_t *height)
{
    egui_dim_t out_width = 0;
    egui_dim_t out_height = 0;

    if (font != NULL && hcw_field_has_text(text))
    {
        font->api->get_str_size(font, text, 0, 0, &out_width, &out_height);
    }

    if (width != NULL)
    {
        *width = out_width;
    }
    if (height != NULL)
    {
        *height = out_height;
    }
}

static void hcw_field_draw_text(egui_view_t *self, const egui_font_t *font, const char *text, const egui_region_t *region, uint8_t align, egui_color_t color,
                                egui_alpha_t alpha)
{
    egui_region_t draw_region;

    if (font == NULL || !hcw_field_has_text(text) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    draw_region = *region;
    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, egui_color_alpha_mix(self->alpha, alpha));
}

static void hcw_field_get_metrics(hcw_field_t *local, egui_view_t *self, hcw_field_metrics_t *metrics)
{
    egui_dim_t side_pad = hcw_field_side_pad(local->compact_mode);
    egui_dim_t top_pad = hcw_field_top_pad(local->compact_mode);
    egui_dim_t label_height = hcw_field_label_height(local->compact_mode);
    egui_dim_t label_gap = hcw_field_label_gap(local->compact_mode);
    egui_dim_t field_height = hcw_field_field_height(local->compact_mode);
    egui_dim_t field_pad_x = hcw_field_field_pad_x(local->compact_mode);
    egui_dim_t field_gap = hcw_field_field_gap(local->compact_mode);
    egui_dim_t info_size = hcw_field_info_size(local->compact_mode);
    egui_dim_t info_gap = hcw_field_info_gap(local->compact_mode);
    egui_dim_t bubble_height = hcw_field_bubble_height(local->compact_mode);
    egui_dim_t bubble_gap = hcw_field_bubble_gap(local->compact_mode);
    egui_dim_t bubble_pad_x = hcw_field_bubble_pad_x(local->compact_mode);
    egui_dim_t bubble_pad_y = hcw_field_bubble_pad_y(local->compact_mode);
    egui_dim_t helper_height = hcw_field_helper_height(local->compact_mode);
    egui_dim_t validation_height = hcw_field_validation_height(local->compact_mode);
    egui_dim_t validation_gap = hcw_field_validation_gap(local->compact_mode);
    egui_dim_t validation_dot_size = hcw_field_validation_dot_size(local->compact_mode);
    egui_dim_t y;
    egui_dim_t x;
    egui_dim_t width;
    egui_dim_t region_bottom;
    egui_dim_t footer_height = 0;
    egui_dim_t available_for_bubble;
    egui_dim_t available_for_field;
    egui_dim_t label_text_width = 0;

    memset(metrics, 0, sizeof(*metrics));
    egui_view_get_work_region(self, &metrics->region);

    x = metrics->region.location.x + side_pad;
    y = metrics->region.location.y + top_pad;
    width = metrics->region.size.width - side_pad * 2;
    region_bottom = metrics->region.location.y + metrics->region.size.height - top_pad;
    if (width < 20)
    {
        width = 20;
    }

    metrics->show_info = hcw_field_show_info(local);
    metrics->show_helper = hcw_field_has_text(local->helper_text);
    metrics->show_validation = hcw_field_has_text(local->validation_text);

    if (metrics->show_helper)
    {
        footer_height += helper_height;
    }
    if (metrics->show_validation)
    {
        if (footer_height > 0)
        {
            footer_height += validation_gap;
        }
        footer_height += validation_height;
    }

    if (hcw_field_has_text(local->label) || metrics->show_info)
    {
        egui_dim_t row_height = label_height;

        if (row_height < info_size)
        {
            row_height = info_size;
        }

        metrics->label_row_region.location.x = x;
        metrics->label_row_region.location.y = y;
        metrics->label_row_region.size.width = width;
        metrics->label_row_region.size.height = row_height;

        metrics->label_region.location.x = x;
        metrics->label_region.location.y = y;
        metrics->label_region.size.width = width;
        metrics->label_region.size.height = row_height;
        if (metrics->show_info)
        {
            metrics->info_region.location.x = x + width - info_size;
            metrics->info_region.location.y = y + (row_height - info_size) / 2;
            metrics->info_region.size.width = info_size;
            metrics->info_region.size.height = info_size;

            metrics->label_region.size.width -= info_size + info_gap;
            if (metrics->label_region.size.width < 24)
            {
                metrics->label_region.size.width = 24;
            }
        }

        hcw_field_measure_text(hcw_field_get_font(local), local->label, &label_text_width, NULL);
        metrics->required_x = metrics->label_region.location.x + label_text_width + 2;
        if (metrics->required_x > metrics->label_region.location.x + metrics->label_region.size.width - 6)
        {
            metrics->required_x = metrics->label_region.location.x + metrics->label_region.size.width - 6;
        }

        y += row_height + label_gap;
    }

    if (metrics->show_info && local->open)
    {
        available_for_bubble = region_bottom - y - field_height - (footer_height > 0 ? (field_gap + footer_height) : 0) - label_gap;
        if (available_for_bubble > bubble_height)
        {
            available_for_bubble = bubble_height;
        }
        if (available_for_bubble >= hcw_field_bubble_min_height(local->compact_mode))
        {
            metrics->show_bubble = 1;
            metrics->bubble_region.location.x = x;
            metrics->bubble_region.location.y = y;
            metrics->bubble_region.size.width = width;
            metrics->bubble_region.size.height = available_for_bubble;

            metrics->bubble_title_region.location.x = x + bubble_pad_x;
            metrics->bubble_title_region.location.y = y + bubble_pad_y;
            metrics->bubble_title_region.size.width = width - bubble_pad_x * 2;
            metrics->bubble_title_region.size.height = local->compact_mode ? 10 : 12;

            metrics->bubble_body_region.location.x = x + bubble_pad_x;
            metrics->bubble_body_region.location.y =
                    metrics->bubble_title_region.location.y + metrics->bubble_title_region.size.height + (local->compact_mode ? 2 : 4);
            metrics->bubble_body_region.size.width = width - bubble_pad_x * 2;
            metrics->bubble_body_region.size.height =
                    y + available_for_bubble - bubble_pad_y - metrics->bubble_body_region.location.y;

            metrics->arrow_center_x = metrics->info_region.location.x + metrics->info_region.size.width / 2;
            if (metrics->arrow_center_x < metrics->bubble_region.location.x + 10)
            {
                metrics->arrow_center_x = metrics->bubble_region.location.x + 10;
            }
            if (metrics->arrow_center_x > metrics->bubble_region.location.x + metrics->bubble_region.size.width - 10)
            {
                metrics->arrow_center_x = metrics->bubble_region.location.x + metrics->bubble_region.size.width - 10;
            }

            y += available_for_bubble + bubble_gap;
        }
    }

    available_for_field = region_bottom - y - (footer_height > 0 ? (field_gap + footer_height) : 0);
    if (available_for_field < 0)
    {
        available_for_field = 0;
    }
    if (available_for_field < field_height)
    {
        field_height = available_for_field;
    }

    metrics->field_region.location.x = x;
    metrics->field_region.location.y = y;
    metrics->field_region.size.width = width;
    metrics->field_region.size.height = field_height;

    metrics->field_text_region.location.x = x + field_pad_x;
    metrics->field_text_region.location.y = y;
    metrics->field_text_region.size.width = width - field_pad_x * 2;
    metrics->field_text_region.size.height = field_height;
    if (metrics->field_text_region.size.width < 8)
    {
        metrics->field_text_region.size.width = 8;
    }

    y += field_height;
    if (footer_height > 0)
    {
        y += field_gap;
    }

    if (metrics->show_helper)
    {
        metrics->helper_region.location.x = x;
        metrics->helper_region.location.y = y;
        metrics->helper_region.size.width = width;
        metrics->helper_region.size.height = helper_height;
        y += helper_height;
        if (metrics->show_validation)
        {
            y += validation_gap;
        }
    }

    if (metrics->show_validation)
    {
        metrics->validation_dot_region.location.x = x + 1;
        metrics->validation_dot_region.location.y = y + (validation_height - validation_dot_size) / 2;
        metrics->validation_dot_region.size.width = validation_dot_size;
        metrics->validation_dot_region.size.height = validation_dot_size;

        metrics->validation_region.location.x = x + validation_dot_size + 6;
        metrics->validation_region.location.y = y - 1;
        metrics->validation_region.size.width = width - validation_dot_size - 6;
        metrics->validation_region.size.height = validation_height + 2;
        if (metrics->validation_region.size.width < 8)
        {
            metrics->validation_region.size.width = 8;
        }
    }
}

static uint8_t hcw_field_hit_part(hcw_field_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    hcw_field_metrics_t metrics;

    hcw_field_get_metrics(local, self, &metrics);
    if (metrics.show_info && egui_region_pt_in_rect(&metrics.info_region, x, y))
    {
        return HCW_FIELD_PART_INFO_BUTTON;
    }
    return HCW_FIELD_PART_NONE;
}

static void hcw_field_set_open_inner(egui_view_t *self, uint8_t is_open, uint8_t notify)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    uint8_t new_open = is_open ? 1 : 0;
    uint8_t had_pressed;

    if (!hcw_field_show_info(local))
    {
        new_open = 0;
    }

    if (local->open == new_open)
    {
        hcw_field_clear_pressed_state(self);
        return;
    }

    had_pressed = hcw_field_clear_pressed_state(self);
    local->open = new_open;
    if (notify && local->on_open_changed != NULL)
    {
        local->on_open_changed(self, new_open);
    }
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

static void hcw_field_draw_focus_ring(egui_view_t *self, const egui_region_t *region, egui_dim_t radius, egui_color_t color, egui_alpha_t alpha)
{
    egui_canvas_draw_round_rectangle(region->location.x - 2, region->location.y - 2, region->size.width + 4, region->size.height + 4, radius + 2, 1, color,
                                     egui_color_alpha_mix(self->alpha, alpha));
}

static void hcw_field_draw_bubble_arrow(egui_view_t *self, hcw_field_t *local, const hcw_field_metrics_t *metrics, egui_color_t fill_color,
                                        egui_color_t border_color)
{
    egui_dim_t arrow_w;
    egui_dim_t arrow_h;
    egui_dim_t center_x;
    egui_dim_t top_y;

    if (!metrics->show_bubble)
    {
        return;
    }

    arrow_w = hcw_field_bubble_arrow_width(local->compact_mode);
    arrow_h = hcw_field_bubble_arrow_height(local->compact_mode);
    center_x = metrics->arrow_center_x;
    top_y = metrics->bubble_region.location.y;

    egui_canvas_draw_triangle_fill(center_x - arrow_w / 2, top_y + 1, center_x + arrow_w / 2, top_y + 1, center_x, top_y - arrow_h, fill_color,
                                   egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_triangle(center_x - arrow_w / 2, top_y + 1, center_x + arrow_w / 2, top_y + 1, center_x, top_y - arrow_h, border_color,
                              egui_color_alpha_mix(self->alpha, 42));
}

static void hcw_field_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    hcw_field_metrics_t metrics;
    egui_color_t field_fill = local->surface_color;
    egui_color_t field_border = local->border_color;
    egui_color_t text_color = local->text_color;
    egui_color_t muted_text_color = local->muted_text_color;
    egui_color_t accent_color = local->accent_color;
    egui_color_t validation_color = hcw_field_validation_tone(local);
    egui_color_t bubble_surface_color = local->bubble_surface_color;
    egui_color_t shadow_color = local->shadow_color;
    egui_color_t info_fill;
    egui_color_t info_border;
    egui_color_t info_text;
    egui_color_t required_color;
    egui_dim_t field_radius;
    egui_dim_t info_radius;
    uint8_t placeholder_active;

    hcw_field_get_metrics(local, self, &metrics);
    if (egui_region_is_empty(&metrics.region))
    {
        return;
    }

    if (local->compact_mode)
    {
        field_fill = egui_rgb_mix(field_fill, EGUI_COLOR_HEX(0xFBFCFD), 20);
        bubble_surface_color = egui_rgb_mix(bubble_surface_color, EGUI_COLOR_HEX(0xFCFDFE), 14);
    }
    else
    {
        field_fill = egui_rgb_mix(field_fill, EGUI_COLOR_HEX(0xFFFFFF), 18);
        bubble_surface_color = egui_rgb_mix(bubble_surface_color, EGUI_COLOR_HEX(0xFFFFFF), 12);
    }

    if (metrics.show_validation && local->validation_state != HCW_FIELD_VALIDATION_NONE)
    {
        field_border = egui_rgb_mix(field_border, validation_color, local->compact_mode ? 24 : 36);
        field_fill = egui_rgb_mix(field_fill, validation_color, local->compact_mode ? 2 : 4);
    }
    if (metrics.show_info && local->open)
    {
        field_border = egui_rgb_mix(field_border, accent_color, local->compact_mode ? 10 : 14);
    }

    if (local->read_only_mode)
    {
        field_fill = egui_rgb_mix(field_fill, EGUI_COLOR_HEX(0xF7F9FB), 42);
        field_border = egui_rgb_mix(field_border, muted_text_color, 26);
        text_color = egui_rgb_mix(text_color, muted_text_color, 38);
        muted_text_color = egui_rgb_mix(muted_text_color, field_border, 10);
        accent_color = egui_rgb_mix(accent_color, muted_text_color, 54);
        validation_color = egui_rgb_mix(validation_color, muted_text_color, 34);
        bubble_surface_color = egui_rgb_mix(bubble_surface_color, EGUI_COLOR_HEX(0xFBFCFD), 16);
        shadow_color = egui_rgb_mix(shadow_color, bubble_surface_color, 34);
    }
    if (!egui_view_get_enable(self))
    {
        field_fill = hcw_field_mix_disabled(field_fill);
        field_border = hcw_field_mix_disabled(field_border);
        text_color = hcw_field_mix_disabled(text_color);
        muted_text_color = hcw_field_mix_disabled(muted_text_color);
        accent_color = hcw_field_mix_disabled(accent_color);
        validation_color = hcw_field_mix_disabled(validation_color);
        bubble_surface_color = hcw_field_mix_disabled(bubble_surface_color);
        shadow_color = hcw_field_mix_disabled(shadow_color);
    }

    required_color = egui_rgb_mix(local->error_color, text_color, 22);
    field_radius = hcw_field_radius(local->compact_mode);
    info_radius = hcw_field_info_radius(local->compact_mode);
    info_fill = egui_rgb_mix(field_fill, accent_color, local->open ? 7 : 4);
    info_border = egui_rgb_mix(field_border, accent_color, local->open ? 16 : 10);
    info_text = egui_rgb_mix(accent_color, text_color, local->open ? 6 : 14);
    placeholder_active = hcw_field_has_text(local->field_text) ? 0 : 1;

    hcw_field_draw_text(self, hcw_field_get_font(local), local->label, &metrics.label_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text_color,
                        EGUI_ALPHA_100);

    if (local->required && hcw_field_has_text(local->label))
    {
        egui_region_t required_region;

        required_region.location.x = metrics.required_x;
        required_region.location.y = metrics.label_region.location.y;
        required_region.size.width = 8;
        required_region.size.height = metrics.label_region.size.height;
        hcw_field_draw_text(self, hcw_field_get_font(local), "*", &required_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, required_color, EGUI_ALPHA_100);
    }

    if (metrics.show_info)
    {
        if (self->is_focused && egui_view_get_enable(self))
        {
            hcw_field_draw_focus_ring(self, &metrics.info_region, info_radius, EGUI_THEME_FOCUS, 68);
        }

        if (egui_view_get_enable(self) && !local->read_only_mode && local->pressed_part == HCW_FIELD_PART_INFO_BUTTON)
        {
            egui_canvas_draw_round_rectangle_fill(metrics.info_region.location.x, metrics.info_region.location.y, metrics.info_region.size.width,
                                                  metrics.info_region.size.height, info_radius, EGUI_THEME_PRESS_OVERLAY, EGUI_THEME_PRESS_OVERLAY_ALPHA);
        }
        else
        {
            egui_canvas_draw_round_rectangle_fill(metrics.info_region.location.x, metrics.info_region.location.y, metrics.info_region.size.width,
                                                  metrics.info_region.size.height, info_radius, info_fill, egui_color_alpha_mix(self->alpha, 18));
        }
        egui_canvas_draw_round_rectangle(metrics.info_region.location.x, metrics.info_region.location.y, metrics.info_region.size.width,
                                         metrics.info_region.size.height, info_radius, 1, info_border, egui_color_alpha_mix(self->alpha, 58));
        hcw_field_draw_text(self, hcw_field_get_icon_font(local), EGUI_ICON_MS_INFO, &metrics.info_region, EGUI_ALIGN_CENTER, info_text, EGUI_ALPHA_100);
    }

    if (metrics.show_bubble)
    {
        egui_canvas_draw_round_rectangle_fill(metrics.bubble_region.location.x + 1, metrics.bubble_region.location.y + 2, metrics.bubble_region.size.width,
                                              metrics.bubble_region.size.height, hcw_field_bubble_radius(local->compact_mode), shadow_color,
                                              egui_color_alpha_mix(self->alpha, local->compact_mode ? 10 : 16));
        hcw_field_draw_bubble_arrow(self, local, &metrics, bubble_surface_color, field_border);
        egui_canvas_draw_round_rectangle_fill(metrics.bubble_region.location.x, metrics.bubble_region.location.y, metrics.bubble_region.size.width,
                                              metrics.bubble_region.size.height, hcw_field_bubble_radius(local->compact_mode), bubble_surface_color,
                                              egui_color_alpha_mix(self->alpha, 96));
        egui_canvas_draw_round_rectangle(metrics.bubble_region.location.x, metrics.bubble_region.location.y, metrics.bubble_region.size.width,
                                         metrics.bubble_region.size.height, hcw_field_bubble_radius(local->compact_mode), 1, field_border,
                                         egui_color_alpha_mix(self->alpha, 44));
        egui_canvas_draw_round_rectangle_fill(metrics.bubble_region.location.x + hcw_field_bubble_pad_x(local->compact_mode),
                                              metrics.bubble_region.location.y + (local->compact_mode ? 5 : 6), local->compact_mode ? 12 : 16, 2, 1, accent_color,
                                              egui_color_alpha_mix(self->alpha, local->compact_mode ? 52 : 68));
        hcw_field_draw_text(self, hcw_field_get_font(local), local->info_title, &metrics.bubble_title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text_color,
                            EGUI_ALPHA_100);
        hcw_field_draw_text(self, hcw_field_get_meta_font(local), local->info_body, &metrics.bubble_body_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_TOP,
                            muted_text_color, EGUI_ALPHA_100);
    }

    if (metrics.field_region.size.width > 0 && metrics.field_region.size.height > 0)
    {
        egui_canvas_draw_round_rectangle_fill(metrics.field_region.location.x, metrics.field_region.location.y, metrics.field_region.size.width,
                                              metrics.field_region.size.height, field_radius, field_fill, egui_color_alpha_mix(self->alpha, 96));
        egui_canvas_draw_round_rectangle(metrics.field_region.location.x, metrics.field_region.location.y, metrics.field_region.size.width,
                                         metrics.field_region.size.height, field_radius, 1, field_border, egui_color_alpha_mix(self->alpha, 72));
        if (local->read_only_mode || !egui_view_get_enable(self))
        {
            egui_canvas_draw_line(metrics.field_region.location.x + 8, metrics.field_region.location.y + metrics.field_region.size.height - 6,
                                  metrics.field_region.location.x + metrics.field_region.size.width - 8,
                                  metrics.field_region.location.y + metrics.field_region.size.height - 6, 1, field_border,
                                  egui_color_alpha_mix(self->alpha, 24));
        }

        if (placeholder_active)
        {
            hcw_field_draw_text(self, hcw_field_get_font(local), local->placeholder, &metrics.field_text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                                muted_text_color, EGUI_ALPHA_100);
        }
        else
        {
            hcw_field_draw_text(self, hcw_field_get_font(local), local->field_text, &metrics.field_text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text_color,
                                EGUI_ALPHA_100);
        }
    }

    if (metrics.show_helper)
    {
        hcw_field_draw_text(self, hcw_field_get_meta_font(local), local->helper_text, &metrics.helper_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                            muted_text_color, EGUI_ALPHA_100);
    }

    if (metrics.show_validation)
    {
        egui_canvas_draw_circle_fill(metrics.validation_dot_region.location.x + metrics.validation_dot_region.size.width / 2,
                                     metrics.validation_dot_region.location.y + metrics.validation_dot_region.size.height / 2,
                                     metrics.validation_dot_region.size.width / 2, validation_color, egui_color_alpha_mix(self->alpha, 82));
        hcw_field_draw_text(self, hcw_field_get_meta_font(local), local->validation_text, &metrics.validation_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                            validation_color, EGUI_ALPHA_100);
    }
}

static void hcw_field_apply_style(egui_view_t *self, uint8_t compact_mode, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(hcw_field_t);

    hcw_field_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    local->read_only_mode = read_only_mode ? 1 : 0;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xD4DDE5);
    local->text_color = EGUI_COLOR_HEX(0x1B2835);
    local->muted_text_color = EGUI_COLOR_HEX(0x697A8B);
    local->accent_color = compact_mode ? EGUI_COLOR_HEX(0x0C7C73) : EGUI_COLOR_HEX(0x0F6CBD);
    local->success_color = EGUI_COLOR_HEX(0x0F9D58);
    local->warning_color = EGUI_COLOR_HEX(0xC27C12);
    local->error_color = EGUI_COLOR_HEX(0xC93C37);
    local->bubble_surface_color = compact_mode ? EGUI_COLOR_HEX(0xF7FBFA) : EGUI_COLOR_HEX(0xF9FBFD);
    local->shadow_color = compact_mode ? EGUI_COLOR_HEX(0xD7E7E4) : EGUI_COLOR_HEX(0xDCE4EA);
    if (read_only_mode)
    {
        local->accent_color = EGUI_COLOR_HEX(0xA8B6C2);
        local->bubble_surface_color = EGUI_COLOR_HEX(0xFBFCFD);
    }
    egui_view_invalidate(self);
}

void hcw_field_apply_standard_style(egui_view_t *self)
{
    hcw_field_apply_style(self, 0, 0);
}

void hcw_field_apply_compact_style(egui_view_t *self)
{
    hcw_field_apply_style(self, 1, 0);
}

void hcw_field_apply_read_only_style(egui_view_t *self)
{
    hcw_field_apply_style(self, 0, 1);
}

void hcw_field_set_label(egui_view_t *self, const char *label)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    uint8_t had_pressed = hcw_field_clear_pressed_state(self);

    local->label = label != NULL ? label : hcw_field_default_text();
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_field_set_field_text(egui_view_t *self, const char *field_text)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    uint8_t had_pressed = hcw_field_clear_pressed_state(self);

    local->field_text = field_text != NULL ? field_text : hcw_field_default_text();
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_field_set_placeholder(egui_view_t *self, const char *placeholder)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    uint8_t had_pressed = hcw_field_clear_pressed_state(self);

    local->placeholder = placeholder != NULL ? placeholder : hcw_field_default_text();
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_field_set_helper_text(egui_view_t *self, const char *helper_text)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    uint8_t had_pressed = hcw_field_clear_pressed_state(self);

    local->helper_text = helper_text != NULL ? helper_text : hcw_field_default_text();
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_field_set_validation_text(egui_view_t *self, const char *validation_text)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    uint8_t had_pressed = hcw_field_clear_pressed_state(self);

    local->validation_text = validation_text != NULL ? validation_text : hcw_field_default_text();
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_field_set_validation_state(egui_view_t *self, uint8_t validation_state)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    uint8_t had_pressed = hcw_field_clear_pressed_state(self);

    if (validation_state > HCW_FIELD_VALIDATION_ERROR)
    {
        validation_state = HCW_FIELD_VALIDATION_NONE;
    }
    local->validation_state = validation_state;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_field_set_required(egui_view_t *self, uint8_t required)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    uint8_t had_pressed = hcw_field_clear_pressed_state(self);

    local->required = required ? 1 : 0;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_field_set_info_title(egui_view_t *self, const char *title)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    uint8_t had_pressed = hcw_field_clear_pressed_state(self);

    local->info_title = title != NULL ? title : hcw_field_default_text();
    if (!hcw_field_show_info(local))
    {
        local->open = 0;
    }
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_field_set_info_body(egui_view_t *self, const char *body)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    uint8_t had_pressed = hcw_field_clear_pressed_state(self);

    local->info_body = body != NULL ? body : hcw_field_default_text();
    if (!hcw_field_show_info(local))
    {
        local->open = 0;
    }
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_field_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    uint8_t had_pressed = hcw_field_clear_pressed_state(self);

    local->font = font != NULL ? font : hcw_field_default_font();
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_field_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    uint8_t had_pressed = hcw_field_clear_pressed_state(self);

    local->meta_font = font != NULL ? font : hcw_field_default_font();
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_field_set_icon_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    uint8_t had_pressed = hcw_field_clear_pressed_state(self);

    local->icon_font = font != NULL ? font : hcw_field_default_icon_font();
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_field_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color, egui_color_t muted_text_color,
                           egui_color_t accent_color, egui_color_t success_color, egui_color_t warning_color, egui_color_t error_color,
                           egui_color_t bubble_surface_color, egui_color_t shadow_color)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    uint8_t had_pressed = hcw_field_clear_pressed_state(self);

    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->error_color = error_color;
    local->bubble_surface_color = bubble_surface_color;
    local->shadow_color = shadow_color;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_field_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    uint8_t had_pressed = hcw_field_clear_pressed_state(self);

    local->compact_mode = compact_mode ? 1 : 0;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_field_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    uint8_t had_pressed = hcw_field_clear_pressed_state(self);

    local->read_only_mode = read_only_mode ? 1 : 0;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_field_set_open(egui_view_t *self, uint8_t is_open)
{
    hcw_field_set_open_inner(self, is_open, 1);
}

uint8_t hcw_field_get_open(egui_view_t *self)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    return local->open;
}

void hcw_field_set_on_open_changed_listener(egui_view_t *self, hcw_field_on_open_changed_listener_t listener)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    local->on_open_changed = listener;
}

uint8_t hcw_field_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    hcw_field_metrics_t metrics;

    if (region == NULL)
    {
        return 0;
    }

    hcw_field_get_metrics(local, self, &metrics);
    switch (part)
    {
    case HCW_FIELD_PART_INFO_BUTTON:
        if (!metrics.show_info)
        {
            memset(region, 0, sizeof(*region));
            return 0;
        }
        *region = metrics.info_region;
        return 1;
    case HCW_FIELD_PART_FIELD:
        if (metrics.field_region.size.width <= 0 || metrics.field_region.size.height <= 0)
        {
            memset(region, 0, sizeof(*region));
            return 0;
        }
        *region = metrics.field_region;
        return 1;
    case HCW_FIELD_PART_BUBBLE:
        if (!metrics.show_bubble)
        {
            memset(region, 0, sizeof(*region));
            return 0;
        }
        *region = metrics.bubble_region;
        return 1;
    default:
        memset(region, 0, sizeof(*region));
        return 0;
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_field_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(hcw_field_t);
    uint8_t hit_part;
    uint8_t same_target;

    if (!egui_view_get_enable(self) || local->read_only_mode)
    {
        hcw_field_clear_pressed_state(self);
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_part = hcw_field_hit_part(local, self, event->location.x, event->location.y);
        if (hit_part != HCW_FIELD_PART_INFO_BUTTON)
        {
            hcw_field_clear_pressed_state(self);
            return 0;
        }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        same_target = (uint8_t)(local->pressed_part == hit_part);
        if (same_target && egui_view_get_pressed(self))
        {
            return 1;
        }
        local->pressed_part = hit_part;
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
        if (local->pressed_part == HCW_FIELD_PART_NONE)
        {
            return 0;
        }
        hit_part = hcw_field_hit_part(local, self, event->location.x, event->location.y);
        if (hit_part == local->pressed_part)
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

        hit_part = hcw_field_hit_part(local, self, event->location.x, event->location.y);
        handled = (uint8_t)((local->pressed_part != HCW_FIELD_PART_NONE) || hit_part != HCW_FIELD_PART_NONE);
        same_target = (uint8_t)(local->pressed_part != HCW_FIELD_PART_NONE && local->pressed_part == hit_part);
        if (same_target && egui_view_get_pressed(self))
        {
            hcw_field_set_open_inner(self, local->open ? 0 : 1, 1);
        }
        hcw_field_clear_pressed_state(self);
        return handled;
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        return hcw_field_clear_pressed_state(self);
    default:
        return 0;
    }
}

static int hcw_field_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_field_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_field_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(hcw_field_t);

    if (!egui_view_get_enable(self) || local->read_only_mode || !hcw_field_show_info(local))
    {
        hcw_field_clear_pressed_state(self);
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_ENTER:
    case EGUI_KEY_CODE_SPACE:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->pressed_part = HCW_FIELD_PART_INFO_BUTTON;
            if (!egui_view_get_pressed(self))
            {
                egui_view_set_pressed(self, 1);
            }
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            hcw_field_set_open_inner(self, local->open ? 0 : 1, 1);
            hcw_field_clear_pressed_state(self);
            return 1;
        }
        return 0;
    case EGUI_KEY_CODE_ESCAPE:
        if (event->type != EGUI_KEY_EVENT_ACTION_UP)
        {
            return 0;
        }
        if (local->open)
        {
            hcw_field_set_open_inner(self, 0, 1);
            return 1;
        }
        hcw_field_clear_pressed_state(self);
        return 1;
    default:
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            hcw_field_clear_pressed_state(self);
        }
        return egui_view_on_key_event(self, event);
    }
}

static int hcw_field_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_field_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static void hcw_field_on_focus_change(egui_view_t *self, int is_focused)
{
    if (!is_focused)
    {
        hcw_field_clear_pressed_state(self);
    }
    egui_view_invalidate(self);
}
#endif

void hcw_field_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_field_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_field_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(hcw_field_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = hcw_field_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = hcw_field_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = hcw_field_on_key_event,
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        .on_focus_changed = hcw_field_on_focus_change,
#endif
};

void hcw_field_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(hcw_field_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(hcw_field_t);
    egui_view_set_padding_all(self, 2);
    egui_view_set_clickable(self, 1);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 1);
#endif

    local->label = hcw_field_default_text();
    local->field_text = hcw_field_default_text();
    local->placeholder = hcw_field_default_text();
    local->helper_text = hcw_field_default_text();
    local->validation_text = hcw_field_default_text();
    local->info_title = hcw_field_default_text();
    local->info_body = hcw_field_default_text();
    local->font = hcw_field_default_font();
    local->meta_font = hcw_field_default_font();
    local->icon_font = hcw_field_default_icon_font();
    local->on_open_changed = NULL;
    local->required = 0;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->open = 0;
    local->validation_state = HCW_FIELD_VALIDATION_NONE;
    local->pressed_part = HCW_FIELD_PART_NONE;

    hcw_field_apply_standard_style(self);
    egui_view_set_view_name(self, "hcw_field");
}
