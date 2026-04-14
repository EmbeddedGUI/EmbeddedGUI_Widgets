#include "egui_view_info_label.h"

#include <string.h>

#define HCW_INFO_LABEL_STANDARD_PANEL_RADIUS        10
#define HCW_INFO_LABEL_STANDARD_FILL_ALPHA          92
#define HCW_INFO_LABEL_STANDARD_BORDER_ALPHA        56
#define HCW_INFO_LABEL_STANDARD_PAD_X               10
#define HCW_INFO_LABEL_STANDARD_PAD_Y               8
#define HCW_INFO_LABEL_STANDARD_ROW_HEIGHT          22
#define HCW_INFO_LABEL_STANDARD_ICON_SIZE           18
#define HCW_INFO_LABEL_STANDARD_ICON_RADIUS         6
#define HCW_INFO_LABEL_STANDARD_ICON_GAP            8
#define HCW_INFO_LABEL_STANDARD_BUBBLE_GAP          8
#define HCW_INFO_LABEL_STANDARD_BUBBLE_HEIGHT       38
#define HCW_INFO_LABEL_STANDARD_BUBBLE_RADIUS       8
#define HCW_INFO_LABEL_STANDARD_BUBBLE_PAD_X        10
#define HCW_INFO_LABEL_STANDARD_BUBBLE_PAD_Y        8
#define HCW_INFO_LABEL_STANDARD_ARROW_WIDTH         12
#define HCW_INFO_LABEL_STANDARD_ARROW_HEIGHT        6
#define HCW_INFO_LABEL_STANDARD_ACTIVE_FILL_ALPHA   18
#define HCW_INFO_LABEL_STANDARD_BUBBLE_BORDER_ALPHA 54

#define HCW_INFO_LABEL_COMPACT_PANEL_RADIUS        8
#define HCW_INFO_LABEL_COMPACT_FILL_ALPHA          88
#define HCW_INFO_LABEL_COMPACT_BORDER_ALPHA        50
#define HCW_INFO_LABEL_COMPACT_PAD_X               8
#define HCW_INFO_LABEL_COMPACT_PAD_Y               6
#define HCW_INFO_LABEL_COMPACT_ROW_HEIGHT          18
#define HCW_INFO_LABEL_COMPACT_ICON_SIZE           14
#define HCW_INFO_LABEL_COMPACT_ICON_RADIUS         5
#define HCW_INFO_LABEL_COMPACT_ICON_GAP            6
#define HCW_INFO_LABEL_COMPACT_BUBBLE_GAP          5
#define HCW_INFO_LABEL_COMPACT_BUBBLE_HEIGHT       22
#define HCW_INFO_LABEL_COMPACT_BUBBLE_RADIUS       6
#define HCW_INFO_LABEL_COMPACT_BUBBLE_PAD_X        8
#define HCW_INFO_LABEL_COMPACT_BUBBLE_PAD_Y        6
#define HCW_INFO_LABEL_COMPACT_ARROW_WIDTH         8
#define HCW_INFO_LABEL_COMPACT_ARROW_HEIGHT        4
#define HCW_INFO_LABEL_COMPACT_ACTIVE_FILL_ALPHA   14
#define HCW_INFO_LABEL_COMPACT_BUBBLE_BORDER_ALPHA 48

typedef struct hcw_info_label_metrics hcw_info_label_metrics_t;
struct hcw_info_label_metrics
{
    egui_region_t region;
    egui_region_t row_region;
    egui_region_t label_region;
    egui_region_t icon_region;
    egui_region_t bubble_region;
    egui_region_t title_region;
    egui_region_t body_region;
    egui_dim_t arrow_center_x;
    uint8_t show_bubble;
};

static const char *hcw_info_label_default_text(void)
{
    return "";
}

static const egui_font_t *hcw_info_label_default_font(void)
{
    return (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
}

static const egui_font_t *hcw_info_label_default_icon_font(void)
{
    return EGUI_FONT_ICON_MS_16;
}

static egui_color_t hcw_info_label_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x7D8894), 58);
}

static uint8_t hcw_info_label_clear_pressed_state(egui_view_t *self)
{
    EGUI_LOCAL_INIT(hcw_info_label_t);
    uint8_t was_pressed = egui_view_get_pressed(self) ? 1 : 0;
    uint8_t had_pressed = (uint8_t)(was_pressed || local->pressed_part != HCW_INFO_LABEL_PART_NONE);

    if (!had_pressed)
    {
        return 0;
    }

    local->pressed_part = HCW_INFO_LABEL_PART_NONE;
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

static egui_dim_t hcw_info_label_pad_x(uint8_t compact_mode)
{
    return compact_mode ? HCW_INFO_LABEL_COMPACT_PAD_X : HCW_INFO_LABEL_STANDARD_PAD_X;
}

static egui_dim_t hcw_info_label_pad_y(uint8_t compact_mode)
{
    return compact_mode ? HCW_INFO_LABEL_COMPACT_PAD_Y : HCW_INFO_LABEL_STANDARD_PAD_Y;
}

static egui_dim_t hcw_info_label_row_height(uint8_t compact_mode)
{
    return compact_mode ? HCW_INFO_LABEL_COMPACT_ROW_HEIGHT : HCW_INFO_LABEL_STANDARD_ROW_HEIGHT;
}

static egui_dim_t hcw_info_label_icon_size(uint8_t compact_mode)
{
    return compact_mode ? HCW_INFO_LABEL_COMPACT_ICON_SIZE : HCW_INFO_LABEL_STANDARD_ICON_SIZE;
}

static egui_dim_t hcw_info_label_icon_radius(uint8_t compact_mode)
{
    return compact_mode ? HCW_INFO_LABEL_COMPACT_ICON_RADIUS : HCW_INFO_LABEL_STANDARD_ICON_RADIUS;
}

static egui_dim_t hcw_info_label_icon_gap(uint8_t compact_mode)
{
    return compact_mode ? HCW_INFO_LABEL_COMPACT_ICON_GAP : HCW_INFO_LABEL_STANDARD_ICON_GAP;
}

static egui_dim_t hcw_info_label_bubble_gap(uint8_t compact_mode)
{
    return compact_mode ? HCW_INFO_LABEL_COMPACT_BUBBLE_GAP : HCW_INFO_LABEL_STANDARD_BUBBLE_GAP;
}

static egui_dim_t hcw_info_label_bubble_height(uint8_t compact_mode)
{
    return compact_mode ? HCW_INFO_LABEL_COMPACT_BUBBLE_HEIGHT : HCW_INFO_LABEL_STANDARD_BUBBLE_HEIGHT;
}

static egui_dim_t hcw_info_label_bubble_radius(uint8_t compact_mode)
{
    return compact_mode ? HCW_INFO_LABEL_COMPACT_BUBBLE_RADIUS : HCW_INFO_LABEL_STANDARD_BUBBLE_RADIUS;
}

static egui_dim_t hcw_info_label_bubble_pad_x(uint8_t compact_mode)
{
    return compact_mode ? HCW_INFO_LABEL_COMPACT_BUBBLE_PAD_X : HCW_INFO_LABEL_STANDARD_BUBBLE_PAD_X;
}

static egui_dim_t hcw_info_label_bubble_pad_y(uint8_t compact_mode)
{
    return compact_mode ? HCW_INFO_LABEL_COMPACT_BUBBLE_PAD_Y : HCW_INFO_LABEL_STANDARD_BUBBLE_PAD_Y;
}

static egui_dim_t hcw_info_label_arrow_width(uint8_t compact_mode)
{
    return compact_mode ? HCW_INFO_LABEL_COMPACT_ARROW_WIDTH : HCW_INFO_LABEL_STANDARD_ARROW_WIDTH;
}

static egui_dim_t hcw_info_label_arrow_height(uint8_t compact_mode)
{
    return compact_mode ? HCW_INFO_LABEL_COMPACT_ARROW_HEIGHT : HCW_INFO_LABEL_STANDARD_ARROW_HEIGHT;
}

static egui_alpha_t hcw_info_label_fill_alpha(uint8_t compact_mode)
{
    return compact_mode ? HCW_INFO_LABEL_COMPACT_FILL_ALPHA : HCW_INFO_LABEL_STANDARD_FILL_ALPHA;
}

static egui_alpha_t hcw_info_label_border_alpha(uint8_t compact_mode)
{
    return compact_mode ? HCW_INFO_LABEL_COMPACT_BORDER_ALPHA : HCW_INFO_LABEL_STANDARD_BORDER_ALPHA;
}

static egui_alpha_t hcw_info_label_active_fill_alpha(uint8_t compact_mode)
{
    return compact_mode ? HCW_INFO_LABEL_COMPACT_ACTIVE_FILL_ALPHA : HCW_INFO_LABEL_STANDARD_ACTIVE_FILL_ALPHA;
}

static egui_alpha_t hcw_info_label_bubble_border_alpha(uint8_t compact_mode)
{
    return compact_mode ? HCW_INFO_LABEL_COMPACT_BUBBLE_BORDER_ALPHA : HCW_INFO_LABEL_STANDARD_BUBBLE_BORDER_ALPHA;
}

static egui_dim_t hcw_info_label_panel_radius(uint8_t compact_mode)
{
    return compact_mode ? HCW_INFO_LABEL_COMPACT_PANEL_RADIUS : HCW_INFO_LABEL_STANDARD_PANEL_RADIUS;
}

static const egui_font_t *hcw_info_label_get_font(const hcw_info_label_t *local)
{
    return local->font == NULL ? hcw_info_label_default_font() : local->font;
}

static const egui_font_t *hcw_info_label_get_meta_font(const hcw_info_label_t *local)
{
    return local->meta_font == NULL ? hcw_info_label_default_font() : local->meta_font;
}

static const egui_font_t *hcw_info_label_get_icon_font(const hcw_info_label_t *local)
{
    return local->icon_font == NULL ? hcw_info_label_default_icon_font() : local->icon_font;
}

static void hcw_info_label_draw_text(egui_view_t *self, const egui_font_t *font, const char *text, const egui_region_t *region, uint8_t align,
                                     egui_color_t color, egui_alpha_t alpha)
{
    egui_region_t draw_region;

    if (font == NULL || text == NULL || text[0] == '\0' || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    draw_region = *region;
    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, egui_color_alpha_mix(self->alpha, alpha));
}

static void hcw_info_label_get_metrics(hcw_info_label_t *local, egui_view_t *self, hcw_info_label_metrics_t *metrics)
{
    egui_dim_t pad_x = hcw_info_label_pad_x(local->compact_mode);
    egui_dim_t pad_y = hcw_info_label_pad_y(local->compact_mode);
    egui_dim_t row_height = hcw_info_label_row_height(local->compact_mode);
    egui_dim_t icon_size = hcw_info_label_icon_size(local->compact_mode);
    egui_dim_t icon_gap = hcw_info_label_icon_gap(local->compact_mode);
    egui_dim_t bubble_gap = hcw_info_label_bubble_gap(local->compact_mode);
    egui_dim_t bubble_height = hcw_info_label_bubble_height(local->compact_mode);
    egui_dim_t bubble_pad_x = hcw_info_label_bubble_pad_x(local->compact_mode);
    egui_dim_t bubble_pad_y = hcw_info_label_bubble_pad_y(local->compact_mode);
    egui_dim_t min_label_width = local->compact_mode ? 28 : 36;
    egui_dim_t remaining_height;

    egui_view_get_work_region(self, &metrics->region);
    metrics->row_region.location.x = metrics->region.location.x + pad_x;
    metrics->row_region.location.y = metrics->region.location.y + pad_y;
    metrics->row_region.size.width = metrics->region.size.width - pad_x * 2;
    metrics->row_region.size.height = row_height;

    if (metrics->row_region.size.width < icon_size + icon_gap + min_label_width)
    {
        metrics->row_region.size.width = icon_size + icon_gap + min_label_width;
    }

    metrics->icon_region.size.width = icon_size;
    metrics->icon_region.size.height = icon_size;
    metrics->icon_region.location.x = metrics->row_region.location.x + metrics->row_region.size.width - icon_size;
    metrics->icon_region.location.y = metrics->row_region.location.y + (row_height - icon_size) / 2;

    metrics->label_region.location.x = metrics->row_region.location.x;
    metrics->label_region.location.y = metrics->row_region.location.y;
    metrics->label_region.size.width = metrics->icon_region.location.x - metrics->row_region.location.x - icon_gap;
    if (metrics->label_region.size.width < min_label_width)
    {
        metrics->label_region.size.width = min_label_width;
    }
    metrics->label_region.size.height = row_height;

    metrics->show_bubble = local->open ? 1 : 0;
    metrics->bubble_region.location.x = metrics->region.location.x + pad_x;
    metrics->bubble_region.location.y = metrics->row_region.location.y + row_height + bubble_gap;
    metrics->bubble_region.size.width = metrics->region.size.width - pad_x * 2;
    remaining_height = metrics->region.location.y + metrics->region.size.height - metrics->bubble_region.location.y - pad_y;
    metrics->bubble_region.size.height = remaining_height < bubble_height ? remaining_height : bubble_height;
    if (metrics->bubble_region.size.height < (local->compact_mode ? 12 : 18))
    {
        metrics->show_bubble = 0;
    }

    metrics->title_region.location.x = metrics->bubble_region.location.x + bubble_pad_x;
    metrics->title_region.location.y = metrics->bubble_region.location.y + bubble_pad_y;
    metrics->title_region.size.width = metrics->bubble_region.size.width - bubble_pad_x * 2;
    metrics->title_region.size.height = local->compact_mode ? 10 : 12;

    metrics->body_region.location.x = metrics->bubble_region.location.x + bubble_pad_x;
    metrics->body_region.location.y = metrics->title_region.location.y + metrics->title_region.size.height + (local->compact_mode ? 2 : 4);
    metrics->body_region.size.width = metrics->bubble_region.size.width - bubble_pad_x * 2;
    metrics->body_region.size.height =
            metrics->bubble_region.location.y + metrics->bubble_region.size.height - metrics->body_region.location.y - bubble_pad_y;

    metrics->arrow_center_x = metrics->icon_region.location.x + metrics->icon_region.size.width / 2;
    if (metrics->arrow_center_x < metrics->bubble_region.location.x + 10)
    {
        metrics->arrow_center_x = metrics->bubble_region.location.x + 10;
    }
    if (metrics->arrow_center_x > metrics->bubble_region.location.x + metrics->bubble_region.size.width - 10)
    {
        metrics->arrow_center_x = metrics->bubble_region.location.x + metrics->bubble_region.size.width - 10;
    }
}

static uint8_t hcw_info_label_hit_part(hcw_info_label_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    hcw_info_label_metrics_t metrics;

    hcw_info_label_get_metrics(local, self, &metrics);
    if (egui_region_pt_in_rect(&metrics.icon_region, x, y))
    {
        return HCW_INFO_LABEL_PART_ICON;
    }
    return HCW_INFO_LABEL_PART_NONE;
}

static void hcw_info_label_set_open_inner(egui_view_t *self, uint8_t is_open, uint8_t notify)
{
    EGUI_LOCAL_INIT(hcw_info_label_t);
    uint8_t new_open = is_open ? 1 : 0;
    uint8_t had_pressed;

    if (local->open == new_open)
    {
        hcw_info_label_clear_pressed_state(self);
        return;
    }

    had_pressed = hcw_info_label_clear_pressed_state(self);
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

static void hcw_info_label_draw_focus_ring(egui_view_t *self, const egui_region_t *region, egui_dim_t radius, egui_color_t color, egui_alpha_t alpha)
{
    egui_canvas_draw_round_rectangle(region->location.x - 2, region->location.y - 2, region->size.width + 4, region->size.height + 4, radius + 2, 1, color,
                                     egui_color_alpha_mix(self->alpha, alpha));
}

static void hcw_info_label_draw_bubble_arrow(egui_view_t *self, hcw_info_label_t *local, const hcw_info_label_metrics_t *metrics, egui_color_t fill_color,
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

    arrow_w = hcw_info_label_arrow_width(local->compact_mode);
    arrow_h = hcw_info_label_arrow_height(local->compact_mode);
    center_x = metrics->arrow_center_x;
    top_y = metrics->bubble_region.location.y;

    egui_canvas_draw_triangle_fill(center_x - arrow_w / 2, top_y + 1, center_x + arrow_w / 2, top_y + 1, center_x, top_y - arrow_h, fill_color,
                                   egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_triangle(center_x - arrow_w / 2, top_y + 1, center_x + arrow_w / 2, top_y + 1, center_x, top_y - arrow_h, border_color,
                              egui_color_alpha_mix(self->alpha, 42));
}

static void hcw_info_label_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(hcw_info_label_t);
    hcw_info_label_metrics_t metrics;
    egui_color_t surface_color = local->surface_color;
    egui_color_t border_color = local->border_color;
    egui_color_t text_color = local->text_color;
    egui_color_t muted_text_color = local->muted_text_color;
    egui_color_t accent_color = local->accent_color;
    egui_color_t bubble_surface_color = local->bubble_surface_color;
    egui_color_t shadow_color = local->shadow_color;
    egui_color_t icon_fill;
    egui_color_t icon_border;
    egui_color_t icon_text;
    egui_dim_t panel_radius;
    egui_dim_t icon_radius;

    hcw_info_label_get_metrics(local, self, &metrics);
    if (egui_region_is_empty(&metrics.region))
    {
        return;
    }

    if (local->compact_mode)
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xFBFCFD), 18);
        bubble_surface_color = egui_rgb_mix(bubble_surface_color, EGUI_COLOR_HEX(0xFCFDFE), 12);
    }
    else
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xFFFFFF), 16);
        bubble_surface_color = egui_rgb_mix(bubble_surface_color, EGUI_COLOR_HEX(0xFFFFFF), 14);
    }

    if (local->read_only_mode)
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xF7F9FB), 16);
        bubble_surface_color = egui_rgb_mix(bubble_surface_color, EGUI_COLOR_HEX(0xF7F9FB), 18);
        border_color = egui_rgb_mix(border_color, muted_text_color, 14);
        text_color = egui_rgb_mix(text_color, muted_text_color, 38);
        muted_text_color = egui_rgb_mix(muted_text_color, border_color, 12);
        accent_color = egui_rgb_mix(accent_color, muted_text_color, 54);
        shadow_color = egui_rgb_mix(shadow_color, bubble_surface_color, 34);
    }
    if (!egui_view_get_enable(self))
    {
        surface_color = hcw_info_label_mix_disabled(surface_color);
        bubble_surface_color = hcw_info_label_mix_disabled(bubble_surface_color);
        border_color = hcw_info_label_mix_disabled(border_color);
        text_color = hcw_info_label_mix_disabled(text_color);
        muted_text_color = hcw_info_label_mix_disabled(muted_text_color);
        accent_color = hcw_info_label_mix_disabled(accent_color);
        shadow_color = hcw_info_label_mix_disabled(shadow_color);
    }

    panel_radius = hcw_info_label_panel_radius(local->compact_mode);
    icon_radius = hcw_info_label_icon_radius(local->compact_mode);
    icon_fill = egui_rgb_mix(surface_color, accent_color, local->open ? 7 : 4);
    icon_border = egui_rgb_mix(border_color, accent_color, local->open ? 14 : 8);
    icon_text = egui_rgb_mix(accent_color, text_color, local->open ? 6 : 14);

    egui_canvas_draw_round_rectangle_fill(metrics.region.location.x, metrics.region.location.y, metrics.region.size.width, metrics.region.size.height,
                                          panel_radius, surface_color, egui_color_alpha_mix(self->alpha, hcw_info_label_fill_alpha(local->compact_mode)));
    egui_canvas_draw_round_rectangle(metrics.region.location.x, metrics.region.location.y, metrics.region.size.width, metrics.region.size.height, panel_radius, 1,
                                     border_color, egui_color_alpha_mix(self->alpha, hcw_info_label_border_alpha(local->compact_mode)));

    if (local->open)
    {
        egui_canvas_draw_line(metrics.row_region.location.x, metrics.row_region.location.y + metrics.row_region.size.height + 2,
                              metrics.row_region.location.x + metrics.row_region.size.width, metrics.row_region.location.y + metrics.row_region.size.height + 2, 1,
                              border_color, egui_color_alpha_mix(self->alpha, 18));
    }

    hcw_info_label_draw_text(self, hcw_info_label_get_font(local), local->label, &metrics.label_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text_color,
                             EGUI_ALPHA_100);

    if (self->is_focused && egui_view_get_enable(self))
    {
        hcw_info_label_draw_focus_ring(self, &metrics.icon_region, icon_radius, EGUI_THEME_FOCUS, 68);
    }

    if (egui_view_get_enable(self) && !local->read_only_mode && local->pressed_part == HCW_INFO_LABEL_PART_ICON)
    {
        egui_canvas_draw_round_rectangle_fill(metrics.icon_region.location.x, metrics.icon_region.location.y, metrics.icon_region.size.width,
                                              metrics.icon_region.size.height, icon_radius, EGUI_THEME_PRESS_OVERLAY, EGUI_THEME_PRESS_OVERLAY_ALPHA);
    }
    else
    {
        egui_canvas_draw_round_rectangle_fill(metrics.icon_region.location.x, metrics.icon_region.location.y, metrics.icon_region.size.width,
                                              metrics.icon_region.size.height, icon_radius, icon_fill,
                                              egui_color_alpha_mix(self->alpha, hcw_info_label_active_fill_alpha(local->compact_mode)));
    }
    egui_canvas_draw_round_rectangle(metrics.icon_region.location.x, metrics.icon_region.location.y, metrics.icon_region.size.width, metrics.icon_region.size.height,
                                     icon_radius, 1, icon_border, egui_color_alpha_mix(self->alpha, 56));
    hcw_info_label_draw_text(self, hcw_info_label_get_icon_font(local), EGUI_ICON_MS_INFO, &metrics.icon_region, EGUI_ALIGN_CENTER, icon_text, EGUI_ALPHA_100);

    if (!metrics.show_bubble)
    {
        return;
    }

    egui_canvas_draw_round_rectangle_fill(metrics.bubble_region.location.x + 1, metrics.bubble_region.location.y + 2, metrics.bubble_region.size.width,
                                          metrics.bubble_region.size.height, hcw_info_label_bubble_radius(local->compact_mode), shadow_color,
                                          egui_color_alpha_mix(self->alpha, local->compact_mode ? 10 : 16));
    hcw_info_label_draw_bubble_arrow(self, local, &metrics, bubble_surface_color, border_color);
    egui_canvas_draw_round_rectangle_fill(metrics.bubble_region.location.x, metrics.bubble_region.location.y, metrics.bubble_region.size.width,
                                          metrics.bubble_region.size.height, hcw_info_label_bubble_radius(local->compact_mode), bubble_surface_color,
                                          egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_round_rectangle(metrics.bubble_region.location.x, metrics.bubble_region.location.y, metrics.bubble_region.size.width,
                                     metrics.bubble_region.size.height, hcw_info_label_bubble_radius(local->compact_mode), 1, border_color,
                                     egui_color_alpha_mix(self->alpha, hcw_info_label_bubble_border_alpha(local->compact_mode)));
    egui_canvas_draw_round_rectangle_fill(metrics.bubble_region.location.x + hcw_info_label_bubble_pad_x(local->compact_mode),
                                          metrics.bubble_region.location.y + 6, local->compact_mode ? 12 : 16, 2, 1, accent_color,
                                          egui_color_alpha_mix(self->alpha, local->compact_mode ? 56 : 70));

    hcw_info_label_draw_text(self, hcw_info_label_get_font(local), local->info_title, &metrics.title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text_color,
                             EGUI_ALPHA_100);
    hcw_info_label_draw_text(self, hcw_info_label_get_meta_font(local), local->info_body, &metrics.body_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_TOP,
                             muted_text_color, EGUI_ALPHA_100);
}

static void hcw_info_label_apply_style(egui_view_t *self, uint8_t compact_mode, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(hcw_info_label_t);

    hcw_info_label_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    local->read_only_mode = read_only_mode ? 1 : 0;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xD4DDE5);
    local->text_color = EGUI_COLOR_HEX(0x1B2835);
    local->muted_text_color = EGUI_COLOR_HEX(0x697A8B);
    local->accent_color = compact_mode ? EGUI_COLOR_HEX(0x0C7C73) : EGUI_COLOR_HEX(0x0F6CBD);
    local->bubble_surface_color = EGUI_COLOR_HEX(0xF9FBFD);
    local->shadow_color = EGUI_COLOR_HEX(0xDCE4EA);
    if (read_only_mode)
    {
        local->accent_color = EGUI_COLOR_HEX(0x9AA6B2);
        local->bubble_surface_color = EGUI_COLOR_HEX(0xFBFCFD);
    }
    egui_view_invalidate(self);
}

void hcw_info_label_apply_standard_style(egui_view_t *self)
{
    hcw_info_label_apply_style(self, 0, 0);
}

void hcw_info_label_apply_compact_style(egui_view_t *self)
{
    hcw_info_label_apply_style(self, 1, 0);
}

void hcw_info_label_apply_read_only_style(egui_view_t *self)
{
    hcw_info_label_apply_style(self, 0, 1);
}

void hcw_info_label_set_text(egui_view_t *self, const char *label)
{
    EGUI_LOCAL_INIT(hcw_info_label_t);
    uint8_t had_pressed = hcw_info_label_clear_pressed_state(self);

    local->label = label != NULL ? label : hcw_info_label_default_text();
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_info_label_set_info_title(egui_view_t *self, const char *title)
{
    EGUI_LOCAL_INIT(hcw_info_label_t);
    uint8_t had_pressed = hcw_info_label_clear_pressed_state(self);

    local->info_title = title != NULL ? title : hcw_info_label_default_text();
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_info_label_set_info_body(egui_view_t *self, const char *body)
{
    EGUI_LOCAL_INIT(hcw_info_label_t);
    uint8_t had_pressed = hcw_info_label_clear_pressed_state(self);

    local->info_body = body != NULL ? body : hcw_info_label_default_text();
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_info_label_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(hcw_info_label_t);
    uint8_t had_pressed = hcw_info_label_clear_pressed_state(self);

    local->font = font != NULL ? font : hcw_info_label_default_font();
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_info_label_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(hcw_info_label_t);
    uint8_t had_pressed = hcw_info_label_clear_pressed_state(self);

    local->meta_font = font != NULL ? font : hcw_info_label_default_font();
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_info_label_set_icon_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(hcw_info_label_t);
    uint8_t had_pressed = hcw_info_label_clear_pressed_state(self);

    local->icon_font = font != NULL ? font : hcw_info_label_default_icon_font();
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_info_label_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t bubble_surface_color,
                                egui_color_t shadow_color)
{
    EGUI_LOCAL_INIT(hcw_info_label_t);
    uint8_t had_pressed = hcw_info_label_clear_pressed_state(self);

    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->bubble_surface_color = bubble_surface_color;
    local->shadow_color = shadow_color;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_info_label_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(hcw_info_label_t);
    uint8_t had_pressed = hcw_info_label_clear_pressed_state(self);

    local->compact_mode = compact_mode ? 1 : 0;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_info_label_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(hcw_info_label_t);
    uint8_t had_pressed = hcw_info_label_clear_pressed_state(self);

    local->read_only_mode = read_only_mode ? 1 : 0;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void hcw_info_label_set_open(egui_view_t *self, uint8_t is_open)
{
    hcw_info_label_set_open_inner(self, is_open, 1);
}

uint8_t hcw_info_label_get_open(egui_view_t *self)
{
    EGUI_LOCAL_INIT(hcw_info_label_t);
    return local->open;
}

void hcw_info_label_set_on_open_changed_listener(egui_view_t *self, hcw_info_label_on_open_changed_listener_t listener)
{
    EGUI_LOCAL_INIT(hcw_info_label_t);
    local->on_open_changed = listener;
}

uint8_t hcw_info_label_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region)
{
    EGUI_LOCAL_INIT(hcw_info_label_t);
    hcw_info_label_metrics_t metrics;

    if (region == NULL)
    {
        return 0;
    }

    hcw_info_label_get_metrics(local, self, &metrics);
    switch (part)
    {
    case HCW_INFO_LABEL_PART_ICON:
        *region = metrics.icon_region;
        return 1;
    case HCW_INFO_LABEL_PART_BUBBLE:
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
static int hcw_info_label_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(hcw_info_label_t);
    uint8_t hit_part;
    uint8_t same_target;

    if (!egui_view_get_enable(self) || local->read_only_mode)
    {
        hcw_info_label_clear_pressed_state(self);
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_part = hcw_info_label_hit_part(local, self, event->location.x, event->location.y);
        if (hit_part != HCW_INFO_LABEL_PART_ICON)
        {
            hcw_info_label_clear_pressed_state(self);
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
        if (local->pressed_part == HCW_INFO_LABEL_PART_NONE)
        {
            return 0;
        }
        hit_part = hcw_info_label_hit_part(local, self, event->location.x, event->location.y);
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

        hit_part = hcw_info_label_hit_part(local, self, event->location.x, event->location.y);
        handled = (uint8_t)((local->pressed_part != HCW_INFO_LABEL_PART_NONE) || hit_part != HCW_INFO_LABEL_PART_NONE);
        same_target = (uint8_t)(local->pressed_part != HCW_INFO_LABEL_PART_NONE && local->pressed_part == hit_part);
        if (same_target && egui_view_get_pressed(self))
        {
            hcw_info_label_set_open_inner(self, local->open ? 0 : 1, 1);
        }
        hcw_info_label_clear_pressed_state(self);
        return handled;
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        return hcw_info_label_clear_pressed_state(self);
    default:
        return 0;
    }
}

static int hcw_info_label_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_info_label_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_info_label_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(hcw_info_label_t);

    if (!egui_view_get_enable(self) || local->read_only_mode)
    {
        hcw_info_label_clear_pressed_state(self);
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_ENTER:
    case EGUI_KEY_CODE_SPACE:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->pressed_part = HCW_INFO_LABEL_PART_ICON;
            if (!egui_view_get_pressed(self))
            {
                egui_view_set_pressed(self, 1);
            }
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            hcw_info_label_set_open_inner(self, local->open ? 0 : 1, 1);
            hcw_info_label_clear_pressed_state(self);
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
            hcw_info_label_set_open_inner(self, 0, 1);
            return 1;
        }
        hcw_info_label_clear_pressed_state(self);
        return 1;
    default:
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            hcw_info_label_clear_pressed_state(self);
        }
        return egui_view_on_key_event(self, event);
    }
}

static int hcw_info_label_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_info_label_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static void hcw_info_label_on_focus_change(egui_view_t *self, int is_focused)
{
    if (!is_focused)
    {
        hcw_info_label_clear_pressed_state(self);
    }
    egui_view_invalidate(self);
}
#endif

void hcw_info_label_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_info_label_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_info_label_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(hcw_info_label_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = hcw_info_label_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = hcw_info_label_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = hcw_info_label_on_key_event,
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        .on_focus_changed = hcw_info_label_on_focus_change,
#endif
};

void hcw_info_label_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(hcw_info_label_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(hcw_info_label_t);
    egui_view_set_padding_all(self, 2);
    egui_view_set_clickable(self, 1);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 1);
#endif

    local->label = hcw_info_label_default_text();
    local->info_title = hcw_info_label_default_text();
    local->info_body = hcw_info_label_default_text();
    local->font = hcw_info_label_default_font();
    local->meta_font = hcw_info_label_default_font();
    local->icon_font = hcw_info_label_default_icon_font();
    local->on_open_changed = NULL;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->open = 0;
    local->pressed_part = HCW_INFO_LABEL_PART_NONE;

    hcw_info_label_apply_standard_style(self);
    egui_view_set_view_name(self, "hcw_info_label");
}
