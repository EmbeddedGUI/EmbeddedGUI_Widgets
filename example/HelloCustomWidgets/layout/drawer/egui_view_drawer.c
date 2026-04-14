#include "egui_view_drawer.h"

#include <string.h>

#define DR_STD_RADIUS        12
#define DR_STD_OUTER_GAP     6
#define DR_STD_INNER_GAP     6
#define DR_STD_DRAWER_W      74
#define DR_STD_PAD_X         8
#define DR_STD_PAD_Y         8
#define DR_STD_HEADER_H      24
#define DR_STD_FOOTER_H      16
#define DR_STD_TOGGLE_W      14
#define DR_STD_TOGGLE_H      20
#define DR_STD_CLOSE_SIZE    10
#define DR_STD_STRIP_W       4
#define DR_STD_CARD_H        18
#define DR_STD_CARD_GAP      6
#define DR_STD_PILL_H        10

#define DR_COMPACT_RADIUS     9
#define DR_COMPACT_OUTER_GAP  4
#define DR_COMPACT_INNER_GAP  4
#define DR_COMPACT_DRAWER_W   56
#define DR_COMPACT_PAD_X      6
#define DR_COMPACT_PAD_Y      6
#define DR_COMPACT_HEADER_H   18
#define DR_COMPACT_FOOTER_H   12
#define DR_COMPACT_TOGGLE_W   12
#define DR_COMPACT_TOGGLE_H   16
#define DR_COMPACT_CLOSE_SIZE 8
#define DR_COMPACT_STRIP_W    3
#define DR_COMPACT_CARD_H     12
#define DR_COMPACT_CARD_GAP   4
#define DR_COMPACT_PILL_H     8

typedef struct egui_view_drawer_metrics egui_view_drawer_metrics_t;
struct egui_view_drawer_metrics
{
    egui_region_t region;
    egui_region_t host_region;
    egui_region_t body_region;
    egui_region_t toggle_region;
    egui_region_t drawer_region;
    egui_region_t eyebrow_region;
    egui_region_t title_region;
    egui_region_t close_region;
    egui_region_t body_primary_region;
    egui_region_t body_secondary_region;
    egui_region_t footer_region;
    egui_region_t tag_region;
    egui_region_t footer_text_region;
    uint8_t show_drawer;
    uint8_t show_secondary;
    uint8_t show_footer;
    uint8_t show_tag;
    uint8_t show_close;
};

static const char *drawer_default_text(void)
{
    return "";
}

static const egui_font_t *drawer_default_font(void)
{
    return (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
}

static uint8_t drawer_has_text(const char *text)
{
    return (text != NULL && text[0] != '\0') ? 1 : 0;
}

static uint8_t drawer_text_len(const char *text)
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

static void drawer_zero_region(egui_region_t *region)
{
    region->location.x = 0;
    region->location.y = 0;
    region->size.width = 0;
    region->size.height = 0;
}

static egui_color_t drawer_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 68);
}

static uint8_t drawer_clear_pressed_state(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    uint8_t was_pressed = egui_view_get_pressed(self) ? 1 : 0;
    uint8_t had_pressed = (uint8_t)(was_pressed || local->pressed_part != EGUI_VIEW_DRAWER_PART_NONE);

    if (!had_pressed)
    {
        return 0;
    }

    local->pressed_part = EGUI_VIEW_DRAWER_PART_NONE;
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

static egui_dim_t drawer_radius(uint8_t compact_mode)
{
    return compact_mode ? DR_COMPACT_RADIUS : DR_STD_RADIUS;
}

static egui_dim_t drawer_outer_gap(uint8_t compact_mode)
{
    return compact_mode ? DR_COMPACT_OUTER_GAP : DR_STD_OUTER_GAP;
}

static egui_dim_t drawer_inner_gap(uint8_t compact_mode)
{
    return compact_mode ? DR_COMPACT_INNER_GAP : DR_STD_INNER_GAP;
}

static egui_dim_t drawer_width(uint8_t compact_mode)
{
    return compact_mode ? DR_COMPACT_DRAWER_W : DR_STD_DRAWER_W;
}

static egui_dim_t drawer_pad_x(uint8_t compact_mode)
{
    return compact_mode ? DR_COMPACT_PAD_X : DR_STD_PAD_X;
}

static egui_dim_t drawer_pad_y(uint8_t compact_mode)
{
    return compact_mode ? DR_COMPACT_PAD_Y : DR_STD_PAD_Y;
}

static egui_dim_t drawer_header_h(uint8_t compact_mode)
{
    return compact_mode ? DR_COMPACT_HEADER_H : DR_STD_HEADER_H;
}

static egui_dim_t drawer_footer_h(uint8_t compact_mode)
{
    return compact_mode ? DR_COMPACT_FOOTER_H : DR_STD_FOOTER_H;
}

static egui_dim_t drawer_toggle_w(uint8_t compact_mode)
{
    return compact_mode ? DR_COMPACT_TOGGLE_W : DR_STD_TOGGLE_W;
}

static egui_dim_t drawer_toggle_h(uint8_t compact_mode)
{
    return compact_mode ? DR_COMPACT_TOGGLE_H : DR_STD_TOGGLE_H;
}

static egui_dim_t drawer_close_size(uint8_t compact_mode)
{
    return compact_mode ? DR_COMPACT_CLOSE_SIZE : DR_STD_CLOSE_SIZE;
}

static egui_dim_t drawer_strip_w(uint8_t compact_mode)
{
    return compact_mode ? DR_COMPACT_STRIP_W : DR_STD_STRIP_W;
}

static egui_dim_t drawer_card_h(uint8_t compact_mode)
{
    return compact_mode ? DR_COMPACT_CARD_H : DR_STD_CARD_H;
}

static egui_dim_t drawer_card_gap(uint8_t compact_mode)
{
    return compact_mode ? DR_COMPACT_CARD_GAP : DR_STD_CARD_GAP;
}

static egui_dim_t drawer_pill_h(uint8_t compact_mode)
{
    return compact_mode ? DR_COMPACT_PILL_H : DR_STD_PILL_H;
}

static const egui_font_t *drawer_font(const egui_view_drawer_t *local)
{
    return local->font == NULL ? drawer_default_font() : local->font;
}

static const egui_font_t *drawer_meta_font(const egui_view_drawer_t *local)
{
    return local->meta_font == NULL ? drawer_default_font() : local->meta_font;
}

static egui_dim_t drawer_pill_width(const char *text, uint8_t compact_mode, egui_dim_t max_width)
{
    egui_dim_t width;

    if (!drawer_has_text(text))
    {
        return 0;
    }
    width = (compact_mode ? 14 : 18) + drawer_text_len(text) * (compact_mode ? 4 : 5);
    return width > max_width ? max_width : width;
}

static void drawer_draw_text(egui_view_t *self, const egui_font_t *font, const char *text, const egui_region_t *region, uint8_t align, egui_color_t color,
                             egui_alpha_t alpha)
{
    egui_region_t draw_region;

    if (font == NULL || !drawer_has_text(text) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    draw_region = *region;
    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, egui_color_alpha_mix(self->alpha, alpha));
}

static void drawer_get_metrics(egui_view_drawer_t *local, egui_view_t *self, egui_view_drawer_metrics_t *metrics)
{
    egui_dim_t outer_gap = drawer_outer_gap(local->compact_mode);
    egui_dim_t inner_gap = drawer_inner_gap(local->compact_mode);
    egui_dim_t panel_w = drawer_width(local->compact_mode);
    egui_dim_t toggle_w = drawer_toggle_w(local->compact_mode);
    egui_dim_t toggle_h = drawer_toggle_h(local->compact_mode);
    egui_dim_t pad_x = drawer_pad_x(local->compact_mode);
    egui_dim_t pad_y = drawer_pad_y(local->compact_mode);
    egui_dim_t header_h = drawer_header_h(local->compact_mode);
    egui_dim_t footer_h = drawer_footer_h(local->compact_mode);
    egui_dim_t close_size = drawer_close_size(local->compact_mode);
    egui_dim_t min_body_w = local->compact_mode ? 22 : 34;
    egui_dim_t tag_w;

    memset(metrics, 0, sizeof(*metrics));
    egui_view_get_work_region(self, &metrics->region);
    if (metrics->region.size.width <= 0 || metrics->region.size.height <= 0)
    {
        return;
    }

    metrics->host_region.location.x = metrics->region.location.x + outer_gap;
    metrics->host_region.location.y = metrics->region.location.y + outer_gap;
    metrics->host_region.size.width = metrics->region.size.width - outer_gap * 2;
    metrics->host_region.size.height = metrics->region.size.height - outer_gap * 2;
    if (metrics->host_region.size.width <= 0 || metrics->host_region.size.height <= 0)
    {
        drawer_zero_region(&metrics->host_region);
        return;
    }

    metrics->body_region = metrics->host_region;
    metrics->show_drawer = local->open ? 1 : 0;
    if (metrics->show_drawer)
    {
        if (panel_w > metrics->host_region.size.width - min_body_w)
        {
            panel_w = metrics->host_region.size.width - min_body_w;
        }
        if (panel_w < (local->compact_mode ? 34 : 46))
        {
            panel_w = local->compact_mode ? 34 : 46;
        }

        metrics->drawer_region.size.width = panel_w;
        metrics->drawer_region.size.height = metrics->host_region.size.height;
        metrics->drawer_region.location.y = metrics->host_region.location.y;
        metrics->drawer_region.location.x =
                local->anchor == EGUI_VIEW_DRAWER_ANCHOR_END ? metrics->host_region.location.x + metrics->host_region.size.width - panel_w
                                                             : metrics->host_region.location.x;

        if (local->presentation_mode == EGUI_VIEW_DRAWER_MODE_INLINE)
        {
            if (local->anchor == EGUI_VIEW_DRAWER_ANCHOR_END)
            {
                metrics->body_region.size.width = metrics->drawer_region.location.x - inner_gap - metrics->body_region.location.x;
            }
            else
            {
                metrics->body_region.location.x = metrics->drawer_region.location.x + metrics->drawer_region.size.width + inner_gap;
                metrics->body_region.size.width =
                        metrics->host_region.location.x + metrics->host_region.size.width - metrics->body_region.location.x;
            }
            if (metrics->body_region.size.width < min_body_w)
            {
                metrics->body_region = metrics->host_region;
            }
        }

        metrics->toggle_region.location.x =
                local->anchor == EGUI_VIEW_DRAWER_ANCHOR_END ? metrics->drawer_region.location.x - toggle_w / 2
                                                             : metrics->drawer_region.location.x + metrics->drawer_region.size.width - toggle_w / 2;
        metrics->toggle_region.location.y = metrics->host_region.location.y + (local->compact_mode ? 8 : 10);
    }
    else
    {
        metrics->toggle_region.location.x =
                local->anchor == EGUI_VIEW_DRAWER_ANCHOR_END ? metrics->host_region.location.x + metrics->host_region.size.width - toggle_w - 2
                                                             : metrics->host_region.location.x + 2;
        metrics->toggle_region.location.y = metrics->host_region.location.y + (local->compact_mode ? 8 : 10);
    }

    metrics->toggle_region.size.width = toggle_w;
    metrics->toggle_region.size.height = toggle_h;
    if (metrics->toggle_region.location.x < metrics->region.location.x)
    {
        metrics->toggle_region.location.x = metrics->region.location.x;
    }
    if (metrics->toggle_region.location.x + toggle_w > metrics->region.location.x + metrics->region.size.width)
    {
        metrics->toggle_region.location.x = metrics->region.location.x + metrics->region.size.width - toggle_w;
    }

    if (!metrics->show_drawer)
    {
        return;
    }

    metrics->show_close = metrics->drawer_region.size.width >= (local->compact_mode ? 28 : 36) ? 1 : 0;
    metrics->eyebrow_region.location.x = metrics->drawer_region.location.x + pad_x;
    metrics->eyebrow_region.location.y = metrics->drawer_region.location.y + pad_y;
    metrics->eyebrow_region.size.width = metrics->drawer_region.size.width - pad_x * 2;
    metrics->eyebrow_region.size.height = local->compact_mode ? 7 : 8;

    if (metrics->show_close)
    {
        metrics->close_region.location.x = metrics->drawer_region.location.x + metrics->drawer_region.size.width - pad_x - close_size;
        metrics->close_region.location.y = metrics->drawer_region.location.y + pad_y;
        metrics->close_region.size.width = close_size;
        metrics->close_region.size.height = close_size;
        metrics->eyebrow_region.size.width = metrics->close_region.location.x - 4 - metrics->eyebrow_region.location.x;
    }

    metrics->title_region.location.x = metrics->drawer_region.location.x + pad_x;
    metrics->title_region.location.y = metrics->eyebrow_region.location.y + metrics->eyebrow_region.size.height + (local->compact_mode ? 1 : 2);
    metrics->title_region.size.width = metrics->drawer_region.size.width - pad_x * 2;
    if (metrics->show_close)
    {
        metrics->title_region.size.width = metrics->close_region.location.x - 4 - metrics->title_region.location.x;
    }
    metrics->title_region.size.height = header_h - metrics->eyebrow_region.size.height - (local->compact_mode ? 1 : 2);

    metrics->body_primary_region.location.x = metrics->drawer_region.location.x + pad_x;
    metrics->body_primary_region.location.y = metrics->drawer_region.location.y + pad_y + header_h;
    metrics->body_primary_region.size.width = metrics->drawer_region.size.width - pad_x * 2;
    metrics->body_primary_region.size.height = local->compact_mode ? 18 : 28;

    metrics->footer_region.location.x = metrics->drawer_region.location.x + pad_x;
    metrics->footer_region.location.y = metrics->drawer_region.location.y + metrics->drawer_region.size.height - pad_y - footer_h;
    metrics->footer_region.size.width = metrics->drawer_region.size.width - pad_x * 2;
    metrics->footer_region.size.height = footer_h;

    metrics->body_secondary_region.location.x = metrics->drawer_region.location.x + pad_x;
    metrics->body_secondary_region.location.y = metrics->body_primary_region.location.y + metrics->body_primary_region.size.height + (local->compact_mode ? 4 : 6);
    metrics->body_secondary_region.size.width = metrics->drawer_region.size.width - pad_x * 2;
    metrics->body_secondary_region.size.height = metrics->footer_region.location.y - (local->compact_mode ? 4 : 6) - metrics->body_secondary_region.location.y;
    metrics->show_secondary = (uint8_t)(drawer_has_text(local->body_secondary) && metrics->body_secondary_region.size.height >= (local->compact_mode ? 8 : 12));

    metrics->show_footer = drawer_has_text(local->footer);
    if (!metrics->show_footer)
    {
        return;
    }

    tag_w = drawer_pill_width(local->tag, local->compact_mode, metrics->footer_region.size.width / 2);
    metrics->show_tag = tag_w > 0 ? 1 : 0;
    if (metrics->show_tag)
    {
        metrics->tag_region.location.x = metrics->footer_region.location.x;
        metrics->tag_region.location.y = metrics->footer_region.location.y + (metrics->footer_region.size.height - drawer_pill_h(local->compact_mode)) / 2;
        metrics->tag_region.size.width = tag_w;
        metrics->tag_region.size.height = drawer_pill_h(local->compact_mode);
    }
    metrics->footer_text_region.location.x =
            metrics->show_tag ? metrics->tag_region.location.x + metrics->tag_region.size.width + (local->compact_mode ? 4 : 6) : metrics->footer_region.location.x;
    metrics->footer_text_region.location.y = metrics->footer_region.location.y;
    metrics->footer_text_region.size.width = metrics->footer_region.location.x + metrics->footer_region.size.width - metrics->footer_text_region.location.x;
    metrics->footer_text_region.size.height = metrics->footer_region.size.height;
}

static uint8_t drawer_hit_part(egui_view_drawer_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    egui_view_drawer_metrics_t metrics;

    drawer_get_metrics(local, self, &metrics);
    if (metrics.show_close && egui_region_pt_in_rect(&metrics.close_region, x, y))
    {
        return EGUI_VIEW_DRAWER_PART_CLOSE;
    }
    if (egui_region_pt_in_rect(&metrics.toggle_region, x, y))
    {
        return EGUI_VIEW_DRAWER_PART_TOGGLE;
    }
    return EGUI_VIEW_DRAWER_PART_NONE;
}

static void drawer_set_open_inner(egui_view_t *self, uint8_t is_open, uint8_t notify)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    uint8_t new_open = is_open ? 1 : 0;
    uint8_t had_pressed;

    if (local->open == new_open)
    {
        drawer_clear_pressed_state(self);
        return;
    }
    had_pressed = drawer_clear_pressed_state(self);
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

static void drawer_draw_focus_ring(egui_view_t *self, const egui_region_t *region, egui_dim_t radius)
{
    egui_canvas_draw_round_rectangle(region->location.x - 2, region->location.y - 2, region->size.width + 4, region->size.height + 4, radius + 2, 1,
                                     EGUI_THEME_FOCUS, egui_color_alpha_mix(self->alpha, 68));
}

static void drawer_draw_toggle(egui_view_t *self, egui_view_drawer_t *local, const egui_region_t *region, egui_color_t fill_color, egui_color_t border_color,
                               egui_color_t accent_color, uint8_t pressed)
{
    egui_dim_t cx = region->location.x + region->size.width / 2;
    egui_dim_t cy = region->location.y + region->size.height / 2;
    egui_color_t icon_color = accent_color;
    uint8_t open = local->open ? 1 : 0;
    uint8_t anchor_end = local->anchor == EGUI_VIEW_DRAWER_ANCHOR_END ? 1 : 0;

    if (pressed)
    {
        fill_color = egui_rgb_mix(fill_color, accent_color, 16);
        border_color = egui_rgb_mix(border_color, accent_color, 14);
    }

    egui_canvas_draw_round_rectangle_fill(region->location.x, region->location.y, region->size.width, region->size.height, region->size.height / 2, fill_color,
                                          egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_round_rectangle(region->location.x, region->location.y, region->size.width, region->size.height, region->size.height / 2, 1, border_color,
                                     egui_color_alpha_mix(self->alpha, 54));

    if (open)
    {
        if (anchor_end)
        {
            egui_canvas_draw_line(cx + 2, cy - 3, cx - 1, cy, 1, icon_color, egui_color_alpha_mix(self->alpha, 92));
            egui_canvas_draw_line(cx - 1, cy, cx + 2, cy + 3, 1, icon_color, egui_color_alpha_mix(self->alpha, 92));
        }
        else
        {
            egui_canvas_draw_line(cx - 2, cy - 3, cx + 1, cy, 1, icon_color, egui_color_alpha_mix(self->alpha, 92));
            egui_canvas_draw_line(cx + 1, cy, cx - 2, cy + 3, 1, icon_color, egui_color_alpha_mix(self->alpha, 92));
        }
    }
    else if (anchor_end)
    {
        egui_canvas_draw_line(cx - 2, cy - 3, cx + 1, cy, 1, icon_color, egui_color_alpha_mix(self->alpha, 92));
        egui_canvas_draw_line(cx + 1, cy, cx - 2, cy + 3, 1, icon_color, egui_color_alpha_mix(self->alpha, 92));
    }
    else
    {
        egui_canvas_draw_line(cx + 2, cy - 3, cx - 1, cy, 1, icon_color, egui_color_alpha_mix(self->alpha, 92));
        egui_canvas_draw_line(cx - 1, cy, cx + 2, cy + 3, 1, icon_color, egui_color_alpha_mix(self->alpha, 92));
    }
}

static void drawer_draw_close(egui_view_t *self, const egui_region_t *region, egui_color_t color, uint8_t pressed)
{
    egui_dim_t inset = region->size.width > 8 ? 2 : 1;

    if (pressed)
    {
        egui_canvas_draw_round_rectangle_fill(region->location.x - 1, region->location.y - 1, region->size.width + 2, region->size.height + 2,
                                              region->size.height / 2, EGUI_THEME_PRESS_OVERLAY, EGUI_THEME_PRESS_OVERLAY_ALPHA);
        color = egui_rgb_mix(color, EGUI_COLOR_BLACK, 12);
    }
    egui_canvas_draw_line(region->location.x + inset, region->location.y + inset, region->location.x + region->size.width - inset,
                          region->location.y + region->size.height - inset, 1, color, egui_color_alpha_mix(self->alpha, 92));
    egui_canvas_draw_line(region->location.x + region->size.width - inset, region->location.y + inset, region->location.x + inset,
                          region->location.y + region->size.height - inset, 1, color, egui_color_alpha_mix(self->alpha, 92));
}

static void drawer_draw_host(egui_view_t *self, egui_view_drawer_t *local, const egui_view_drawer_metrics_t *metrics, egui_color_t host_surface,
                             egui_color_t border_color, egui_color_t accent_color, egui_color_t text_color, egui_color_t muted_text_color)
{
    egui_dim_t pad_x = drawer_pad_x(local->compact_mode);
    egui_dim_t pad_y = drawer_pad_y(local->compact_mode);
    egui_dim_t card_h = drawer_card_h(local->compact_mode);
    egui_dim_t card_gap = drawer_card_gap(local->compact_mode);
    egui_region_t accent_region;
    egui_region_t title_region;
    egui_region_t note_region;
    egui_region_t card_region;
    egui_region_t line_region;
    uint8_t i;

    egui_canvas_draw_round_rectangle_fill(metrics->host_region.location.x, metrics->host_region.location.y, metrics->host_region.size.width,
                                          metrics->host_region.size.height, drawer_radius(local->compact_mode), host_surface,
                                          egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_round_rectangle(metrics->host_region.location.x, metrics->host_region.location.y, metrics->host_region.size.width,
                                     metrics->host_region.size.height, drawer_radius(local->compact_mode), 1, border_color,
                                     egui_color_alpha_mix(self->alpha, 42));
    if (metrics->body_region.size.width <= 0 || metrics->body_region.size.height <= 0)
    {
        return;
    }

    accent_region.location.x = metrics->body_region.location.x + pad_x;
    accent_region.location.y = metrics->body_region.location.y + pad_y;
    accent_region.size.width = EGUI_MIN((egui_dim_t)(metrics->body_region.size.width / 3), (egui_dim_t)(local->compact_mode ? 20 : 28));
    accent_region.size.height = 3;
    egui_canvas_draw_round_rectangle_fill(accent_region.location.x, accent_region.location.y, accent_region.size.width, accent_region.size.height, 1,
                                          accent_color, egui_color_alpha_mix(self->alpha, 82));

    title_region.location.x = accent_region.location.x;
    title_region.location.y = accent_region.location.y + (local->compact_mode ? 6 : 8);
    title_region.size.width = metrics->body_region.size.width - pad_x * 2;
    title_region.size.height = local->compact_mode ? 10 : 12;
    drawer_draw_text(self, drawer_font(local), local->presentation_mode == EGUI_VIEW_DRAWER_MODE_OVERLAY ? "Overlay surface" : "Inline surface",
                     &title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text_color, EGUI_ALPHA_100);

    note_region.location.x = accent_region.location.x;
    note_region.location.y = title_region.location.y + title_region.size.height + 2;
    note_region.size.width = metrics->body_region.size.width - pad_x * 2;
    note_region.size.height = local->compact_mode ? 8 : 9;
    drawer_draw_text(self, drawer_meta_font(local),
                     local->anchor == EGUI_VIEW_DRAWER_ANCHOR_END ? "Anchored to the end edge." : "Anchored to the start edge.", &note_region,
                     EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, muted_text_color, EGUI_ALPHA_100);

    card_region.location.x = metrics->body_region.location.x + pad_x;
    card_region.location.y = note_region.location.y + note_region.size.height + card_gap;
    card_region.size.width = metrics->body_region.size.width - pad_x * 2;
    card_region.size.height = card_h;
    for (i = 0; i < 2; ++i)
    {
        if (card_region.location.y + card_region.size.height > metrics->body_region.location.y + metrics->body_region.size.height - pad_y)
        {
            break;
        }
        egui_canvas_draw_round_rectangle_fill(card_region.location.x, card_region.location.y, card_region.size.width, card_region.size.height,
                                              local->compact_mode ? 5 : 7, egui_rgb_mix(host_surface, accent_color, (uint8_t)(i == 0 ? 5 : 3)),
                                              egui_color_alpha_mix(self->alpha, 94));
        egui_canvas_draw_round_rectangle(card_region.location.x, card_region.location.y, card_region.size.width, card_region.size.height,
                                         local->compact_mode ? 5 : 7, 1, egui_rgb_mix(border_color, accent_color, (uint8_t)(i == 0 ? 12 : 7)),
                                         egui_color_alpha_mix(self->alpha, 36));
        line_region.location.x = card_region.location.x + (local->compact_mode ? 5 : 6);
        line_region.location.y = card_region.location.y + (local->compact_mode ? 4 : 5);
        line_region.size.width = EGUI_MIN((egui_dim_t)(card_region.size.width - 12), (egui_dim_t)(local->compact_mode ? 26 : 34));
        line_region.size.height = local->compact_mode ? 8 : 10;
        drawer_draw_text(self, drawer_meta_font(local), i == 0 ? "Pinned summary" : "Context note", &line_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER,
                         i == 0 ? text_color : muted_text_color, EGUI_ALPHA_100);
        card_region.location.y += card_region.size.height + card_gap;
        card_region.size.height = local->compact_mode ? 11 : 14;
    }
}

static void drawer_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    egui_view_drawer_metrics_t metrics;
    egui_color_t surface_color = local->surface_color;
    egui_color_t overlay_color = local->overlay_color;
    egui_color_t border_color = local->border_color;
    egui_color_t section_color = local->section_color;
    egui_color_t text_color = local->text_color;
    egui_color_t muted_text_color = local->muted_text_color;
    egui_color_t accent_color = local->accent_color;
    egui_color_t shadow_color = local->shadow_color;
    egui_region_t accent_strip;
    uint8_t toggle_pressed;
    uint8_t close_pressed;

    drawer_get_metrics(local, self, &metrics);
    if (metrics.region.size.width <= 0 || metrics.region.size.height <= 0)
    {
        return;
    }

    if (local->compact_mode)
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xFBFCFD), 16);
        section_color = egui_rgb_mix(section_color, EGUI_COLOR_HEX(0xFBFCFD), 20);
    }
    if (local->read_only_mode)
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xF7F9FB), 18);
        section_color = egui_rgb_mix(section_color, EGUI_COLOR_HEX(0xF7F9FB), 18);
        border_color = egui_rgb_mix(border_color, muted_text_color, 18);
        text_color = egui_rgb_mix(text_color, muted_text_color, 30);
        accent_color = egui_rgb_mix(accent_color, muted_text_color, 56);
        shadow_color = egui_rgb_mix(shadow_color, section_color, 26);
    }
    if (!egui_view_get_enable(self))
    {
        surface_color = drawer_mix_disabled(surface_color);
        overlay_color = drawer_mix_disabled(overlay_color);
        border_color = drawer_mix_disabled(border_color);
        section_color = drawer_mix_disabled(section_color);
        text_color = drawer_mix_disabled(text_color);
        muted_text_color = drawer_mix_disabled(muted_text_color);
        accent_color = drawer_mix_disabled(accent_color);
        shadow_color = drawer_mix_disabled(shadow_color);
    }

    drawer_draw_host(self, local, &metrics, section_color, border_color, accent_color, text_color, muted_text_color);
    if (local->open && local->presentation_mode == EGUI_VIEW_DRAWER_MODE_OVERLAY)
    {
        egui_canvas_draw_round_rectangle_fill(metrics.host_region.location.x, metrics.host_region.location.y, metrics.host_region.size.width,
                                              metrics.host_region.size.height, drawer_radius(local->compact_mode), overlay_color,
                                              egui_color_alpha_mix(self->alpha, local->compact_mode ? 18 : 24));
    }

    if (self->is_focused && egui_view_get_enable(self))
    {
        drawer_draw_focus_ring(self, &metrics.toggle_region, metrics.toggle_region.size.height / 2);
    }
    toggle_pressed = (uint8_t)(local->pressed_part == EGUI_VIEW_DRAWER_PART_TOGGLE && egui_view_get_pressed(self) && !local->read_only_mode);
    drawer_draw_toggle(self, local, &metrics.toggle_region, egui_rgb_mix(surface_color, accent_color, local->open ? 8 : 4),
                       egui_rgb_mix(border_color, accent_color, local->open ? 22 : 12), accent_color, toggle_pressed);

    if (!metrics.show_drawer)
    {
        return;
    }

    egui_canvas_draw_round_rectangle_fill(metrics.drawer_region.location.x + 1, metrics.drawer_region.location.y + 2, metrics.drawer_region.size.width,
                                          metrics.drawer_region.size.height, drawer_radius(local->compact_mode), shadow_color,
                                          egui_color_alpha_mix(self->alpha, local->presentation_mode == EGUI_VIEW_DRAWER_MODE_OVERLAY ? 18 : 10));
    egui_canvas_draw_round_rectangle_fill(metrics.drawer_region.location.x, metrics.drawer_region.location.y, metrics.drawer_region.size.width,
                                          metrics.drawer_region.size.height, drawer_radius(local->compact_mode),
                                          egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xFFFFFF), local->compact_mode ? 10 : 14),
                                          egui_color_alpha_mix(self->alpha, 98));
    egui_canvas_draw_round_rectangle(metrics.drawer_region.location.x, metrics.drawer_region.location.y, metrics.drawer_region.size.width,
                                     metrics.drawer_region.size.height, drawer_radius(local->compact_mode), 1, border_color,
                                     egui_color_alpha_mix(self->alpha, 44));

    accent_strip.location.y = metrics.drawer_region.location.y + 8;
    accent_strip.size.width = drawer_strip_w(local->compact_mode);
    accent_strip.size.height = metrics.drawer_region.size.height - 16;
    accent_strip.location.x = local->anchor == EGUI_VIEW_DRAWER_ANCHOR_END ? metrics.drawer_region.location.x + metrics.drawer_region.size.width - accent_strip.size.width
                                                                           : metrics.drawer_region.location.x;
    egui_canvas_draw_round_rectangle_fill(accent_strip.location.x, accent_strip.location.y, accent_strip.size.width, accent_strip.size.height,
                                          accent_strip.size.width, accent_color, egui_color_alpha_mix(self->alpha, 78));
    egui_canvas_draw_line(metrics.drawer_region.location.x + drawer_pad_x(local->compact_mode), metrics.body_primary_region.location.y - (local->compact_mode ? 3 : 4),
                          metrics.drawer_region.location.x + metrics.drawer_region.size.width - drawer_pad_x(local->compact_mode),
                          metrics.body_primary_region.location.y - (local->compact_mode ? 3 : 4), 1, border_color, egui_color_alpha_mix(self->alpha, 24));
    if (metrics.show_footer)
    {
        egui_canvas_draw_line(metrics.footer_region.location.x, metrics.footer_region.location.y - (local->compact_mode ? 3 : 4),
                              metrics.footer_region.location.x + metrics.footer_region.size.width, metrics.footer_region.location.y - (local->compact_mode ? 3 : 4), 1,
                              border_color, egui_color_alpha_mix(self->alpha, 22));
    }

    drawer_draw_text(self, drawer_meta_font(local), local->eyebrow, &metrics.eyebrow_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, accent_color, EGUI_ALPHA_100);
    drawer_draw_text(self, drawer_font(local), local->title, &metrics.title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_TOP, text_color, EGUI_ALPHA_100);
    drawer_draw_text(self, drawer_meta_font(local), local->body_primary, &metrics.body_primary_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_TOP, text_color, EGUI_ALPHA_100);
    if (metrics.show_secondary)
    {
        drawer_draw_text(self, drawer_meta_font(local), local->body_secondary, &metrics.body_secondary_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_TOP, muted_text_color,
                         EGUI_ALPHA_100);
    }
    if (metrics.show_tag)
    {
        egui_canvas_draw_round_rectangle_fill(metrics.tag_region.location.x, metrics.tag_region.location.y, metrics.tag_region.size.width, metrics.tag_region.size.height,
                                              metrics.tag_region.size.height / 2, egui_rgb_mix(surface_color, accent_color, local->read_only_mode ? 8 : 14),
                                              egui_color_alpha_mix(self->alpha, 96));
        egui_canvas_draw_round_rectangle(metrics.tag_region.location.x, metrics.tag_region.location.y, metrics.tag_region.size.width, metrics.tag_region.size.height,
                                         metrics.tag_region.size.height / 2, 1, egui_rgb_mix(border_color, accent_color, local->read_only_mode ? 12 : 20),
                                         egui_color_alpha_mix(self->alpha, 42));
        drawer_draw_text(self, drawer_meta_font(local), local->tag, &metrics.tag_region, EGUI_ALIGN_CENTER, accent_color, EGUI_ALPHA_100);
    }
    if (metrics.show_footer)
    {
        drawer_draw_text(self, drawer_meta_font(local), local->footer, &metrics.footer_text_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, muted_text_color,
                         EGUI_ALPHA_100);
    }
    if (metrics.show_close)
    {
        close_pressed = (uint8_t)(local->pressed_part == EGUI_VIEW_DRAWER_PART_CLOSE && egui_view_get_pressed(self) && !local->read_only_mode);
        drawer_draw_close(self, &metrics.close_region, text_color, close_pressed);
    }
}

static void drawer_set_text_field(egui_view_t *self, const char **field, const char *text)
{
    uint8_t had_pressed = drawer_clear_pressed_state(self);

    *field = text != NULL ? text : drawer_default_text();
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_drawer_set_eyebrow(egui_view_t *self, const char *text)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    drawer_set_text_field(self, &local->eyebrow, text);
}

void egui_view_drawer_set_title(egui_view_t *self, const char *text)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    drawer_set_text_field(self, &local->title, text);
}

void egui_view_drawer_set_body_primary(egui_view_t *self, const char *text)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    drawer_set_text_field(self, &local->body_primary, text);
}

void egui_view_drawer_set_body_secondary(egui_view_t *self, const char *text)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    drawer_set_text_field(self, &local->body_secondary, text);
}

void egui_view_drawer_set_footer(egui_view_t *self, const char *text)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    drawer_set_text_field(self, &local->footer, text);
}

void egui_view_drawer_set_tag(egui_view_t *self, const char *text)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    drawer_set_text_field(self, &local->tag, text);
}

void egui_view_drawer_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    uint8_t had_pressed = drawer_clear_pressed_state(self);

    local->font = font != NULL ? font : drawer_default_font();
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_drawer_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    uint8_t had_pressed = drawer_clear_pressed_state(self);

    local->meta_font = font != NULL ? font : drawer_default_font();
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

void egui_view_drawer_set_anchor(egui_view_t *self, uint8_t anchor)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    uint8_t had_pressed = drawer_clear_pressed_state(self);

    local->anchor = anchor == EGUI_VIEW_DRAWER_ANCHOR_END ? EGUI_VIEW_DRAWER_ANCHOR_END : EGUI_VIEW_DRAWER_ANCHOR_START;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

uint8_t egui_view_drawer_get_anchor(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    return local->anchor;
}

void egui_view_drawer_set_presentation_mode(egui_view_t *self, uint8_t presentation_mode)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    uint8_t had_pressed = drawer_clear_pressed_state(self);

    local->presentation_mode =
            presentation_mode == EGUI_VIEW_DRAWER_MODE_OVERLAY ? EGUI_VIEW_DRAWER_MODE_OVERLAY : EGUI_VIEW_DRAWER_MODE_INLINE;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

uint8_t egui_view_drawer_get_presentation_mode(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    return local->presentation_mode;
}

void egui_view_drawer_set_open(egui_view_t *self, uint8_t is_open)
{
    drawer_set_open_inner(self, is_open, 1);
}

uint8_t egui_view_drawer_get_open(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    return local->open;
}

void egui_view_drawer_set_on_open_changed_listener(egui_view_t *self, egui_view_on_drawer_open_changed_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    local->on_open_changed = listener;
}

void egui_view_drawer_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    uint8_t had_pressed = drawer_clear_pressed_state(self);

    local->compact_mode = compact_mode ? 1 : 0;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

uint8_t egui_view_drawer_get_compact_mode(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    return local->compact_mode;
}

void egui_view_drawer_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    uint8_t had_pressed = drawer_clear_pressed_state(self);

    local->read_only_mode = read_only_mode ? 1 : 0;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

uint8_t egui_view_drawer_get_read_only_mode(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    return local->read_only_mode;
}

void egui_view_drawer_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t overlay_color, egui_color_t border_color,
                                  egui_color_t section_color, egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                  egui_color_t shadow_color)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    uint8_t had_pressed = drawer_clear_pressed_state(self);

    local->surface_color = surface_color;
    local->overlay_color = overlay_color;
    local->border_color = border_color;
    local->section_color = section_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    local->shadow_color = shadow_color;
    if (!had_pressed)
    {
        egui_view_invalidate(self);
    }
}

uint8_t egui_view_drawer_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    egui_view_drawer_metrics_t metrics;

    if (region == NULL)
    {
        return 0;
    }
    drawer_get_metrics(local, self, &metrics);
    switch (part)
    {
    case EGUI_VIEW_DRAWER_PART_TOGGLE:
        *region = metrics.toggle_region;
        return metrics.toggle_region.size.width > 0 && metrics.toggle_region.size.height > 0;
    case EGUI_VIEW_DRAWER_PART_CLOSE:
        if (!metrics.show_close)
        {
            memset(region, 0, sizeof(*region));
            return 0;
        }
        *region = metrics.close_region;
        return 1;
    default:
        memset(region, 0, sizeof(*region));
        return 0;
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int drawer_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);
    uint8_t hit_part;
    uint8_t same_target;

    if (!egui_view_get_enable(self) || local->read_only_mode)
    {
        drawer_clear_pressed_state(self);
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_part = drawer_hit_part(local, self, event->location.x, event->location.y);
        if (hit_part == EGUI_VIEW_DRAWER_PART_NONE)
        {
            drawer_clear_pressed_state(self);
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
        if (local->pressed_part == EGUI_VIEW_DRAWER_PART_NONE)
        {
            return 0;
        }
        hit_part = drawer_hit_part(local, self, event->location.x, event->location.y);
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

        hit_part = drawer_hit_part(local, self, event->location.x, event->location.y);
        handled = (uint8_t)((local->pressed_part != EGUI_VIEW_DRAWER_PART_NONE) || hit_part != EGUI_VIEW_DRAWER_PART_NONE);
        same_target = (uint8_t)(local->pressed_part != EGUI_VIEW_DRAWER_PART_NONE && local->pressed_part == hit_part);
        if (same_target && egui_view_get_pressed(self))
        {
            if (hit_part == EGUI_VIEW_DRAWER_PART_CLOSE)
            {
                drawer_set_open_inner(self, 0, 1);
            }
            else
            {
                drawer_set_open_inner(self, local->open ? 0 : 1, 1);
            }
        }
        drawer_clear_pressed_state(self);
        return handled;
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        return drawer_clear_pressed_state(self);
    default:
        return 0;
    }
}

static int drawer_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    drawer_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int drawer_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_drawer_t);

    if (!egui_view_get_enable(self) || local->read_only_mode)
    {
        drawer_clear_pressed_state(self);
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_ENTER:
    case EGUI_KEY_CODE_SPACE:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->pressed_part = EGUI_VIEW_DRAWER_PART_TOGGLE;
            if (!egui_view_get_pressed(self))
            {
                egui_view_set_pressed(self, 1);
            }
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            drawer_set_open_inner(self, local->open ? 0 : 1, 1);
            drawer_clear_pressed_state(self);
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
            drawer_set_open_inner(self, 0, 1);
            return 1;
        }
        drawer_clear_pressed_state(self);
        return 1;
    default:
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            drawer_clear_pressed_state(self);
        }
        return egui_view_on_key_event(self, event);
    }
}

static int drawer_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    drawer_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static void drawer_on_focus_change(egui_view_t *self, int is_focused)
{
    if (!is_focused)
    {
        drawer_clear_pressed_state(self);
    }
    egui_view_invalidate(self);
}
#endif

void egui_view_drawer_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = drawer_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = drawer_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_drawer_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = drawer_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = drawer_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = drawer_on_key_event,
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        .on_focus_changed = drawer_on_focus_change,
#endif
};

void egui_view_drawer_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_drawer_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_drawer_t);
    egui_view_set_padding_all(self, 2);
    egui_view_set_clickable(self, 1);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 1);
#endif

    local->eyebrow = drawer_default_text();
    local->title = drawer_default_text();
    local->body_primary = drawer_default_text();
    local->body_secondary = drawer_default_text();
    local->footer = drawer_default_text();
    local->tag = drawer_default_text();
    local->font = drawer_default_font();
    local->meta_font = drawer_default_font();
    local->on_open_changed = NULL;
    local->anchor = EGUI_VIEW_DRAWER_ANCHOR_START;
    local->presentation_mode = EGUI_VIEW_DRAWER_MODE_INLINE;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->open = 1;
    local->pressed_part = EGUI_VIEW_DRAWER_PART_NONE;

    egui_view_drawer_set_palette(self, EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0x19344D), EGUI_COLOR_HEX(0xD6DEE6), EGUI_COLOR_HEX(0xF5F7F9),
                                 EGUI_COLOR_HEX(0x1F2E3C), EGUI_COLOR_HEX(0x667789), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xD7E1E9));
    egui_view_set_view_name(self, "egui_view_drawer");
}
