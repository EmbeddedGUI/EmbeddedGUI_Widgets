#include "egui_view_persona.h"

#include <string.h>

typedef struct egui_view_persona_metrics egui_view_persona_metrics_t;
struct egui_view_persona_metrics
{
    egui_region_t panel_region;
    egui_region_t section_region;
    egui_region_t avatar_region;
    egui_region_t content_region;
    egui_region_t title_region;
    egui_region_t secondary_region;
    egui_region_t tertiary_region;
    egui_region_t quaternary_region;
    egui_region_t presence_region;
    uint8_t show_secondary;
    uint8_t show_tertiary;
    uint8_t show_quaternary;
    uint8_t show_presence;
};

static const egui_font_t *egui_view_persona_default_font(void)
{
    return (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
}

static const egui_font_t *egui_view_persona_default_icon_font(egui_dim_t avatar_size)
{
    if (avatar_size <= 28)
    {
        return EGUI_FONT_ICON_MS_16;
    }
    if (avatar_size <= 40)
    {
        return EGUI_FONT_ICON_MS_20;
    }
    return EGUI_FONT_ICON_MS_24;
}

static uint8_t egui_view_persona_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static egui_color_t egui_view_persona_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x83909D), 54);
}

static uint8_t egui_view_persona_clamp_tone(uint8_t tone)
{
    if (tone > EGUI_VIEW_PERSONA_TONE_NEUTRAL)
    {
        return EGUI_VIEW_PERSONA_TONE_NEUTRAL;
    }
    return tone;
}

static uint8_t egui_view_persona_clamp_status(uint8_t status)
{
    if (status > EGUI_VIEW_PERSONA_STATUS_OFFLINE)
    {
        return EGUI_VIEW_PERSONA_STATUS_NONE;
    }
    return status;
}

static char egui_view_persona_normalize_initial_char(char c)
{
    if (c >= 'a' && c <= 'z')
    {
        return (char)(c - ('a' - 'A'));
    }
    if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
    {
        return c;
    }
    return '\0';
}

static uint8_t egui_view_persona_is_separator(char c)
{
    return (uint8_t)(c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '-' || c == '_' || c == '/' || c == '.');
}

static void egui_view_persona_copy_initials(const char *source, char *buffer)
{
    uint8_t len = 0;
    uint8_t i = 0;

    if (source != NULL)
    {
        while (source[i] != '\0' && len < 2)
        {
            char normalized = egui_view_persona_normalize_initial_char(source[i]);

            if (normalized != '\0')
            {
                buffer[len++] = normalized;
            }
            i++;
        }
    }

    buffer[len] = '\0';
}

static void egui_view_persona_derive_initials(const char *display_name, char *buffer)
{
    char first = '\0';
    char second = '\0';
    char fallback_second = '\0';
    uint8_t word_count = 0;
    uint8_t in_word = 0;
    uint8_t i = 0;

    if (display_name == NULL)
    {
        buffer[0] = '\0';
        return;
    }

    while (display_name[i] != '\0')
    {
        char c = display_name[i];
        char normalized = egui_view_persona_normalize_initial_char(c);

        if (egui_view_persona_is_separator(c))
        {
            in_word = 0;
            i++;
            continue;
        }

        if (!in_word)
        {
            in_word = 1;
            if (normalized != '\0')
            {
                if (word_count == 0)
                {
                    first = normalized;
                }
                else if (word_count == 1)
                {
                    second = normalized;
                    break;
                }
                word_count++;
            }
        }
        else if (word_count == 1 && fallback_second == '\0' && normalized != '\0')
        {
            fallback_second = normalized;
        }

        i++;
    }

    if (first == '\0')
    {
        buffer[0] = '\0';
        return;
    }

    buffer[0] = first;
    if (second != '\0')
    {
        buffer[1] = second;
        buffer[2] = '\0';
        return;
    }
    if (fallback_second != '\0')
    {
        buffer[1] = fallback_second;
        buffer[2] = '\0';
        return;
    }
    buffer[1] = '\0';
}

static void egui_view_persona_resolve_initials(const char *initials, const char *display_name, char *buffer)
{
    egui_view_persona_copy_initials(initials, buffer);
    if (buffer[0] == '\0')
    {
        egui_view_persona_derive_initials(display_name, buffer);
    }
}

static const egui_font_t *egui_view_persona_get_font(const egui_view_persona_t *local)
{
    return local->font == NULL ? egui_view_persona_default_font() : local->font;
}

static const egui_font_t *egui_view_persona_get_meta_font(const egui_view_persona_t *local)
{
    return local->meta_font == NULL ? egui_view_persona_default_font() : local->meta_font;
}

static egui_color_t egui_view_persona_tone_color(const egui_view_persona_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_PERSONA_TONE_ACCENT:
        return local->accent_color;
    case EGUI_VIEW_PERSONA_TONE_SUCCESS:
        return local->success_color;
    case EGUI_VIEW_PERSONA_TONE_WARNING:
        return local->warning_color;
    default:
        return local->neutral_color;
    }
}

static egui_color_t egui_view_persona_status_color(const egui_view_persona_t *local, uint8_t status)
{
    switch (status)
    {
    case EGUI_VIEW_PERSONA_STATUS_BUSY:
        return EGUI_COLOR_HEX(0xC4314B);
    case EGUI_VIEW_PERSONA_STATUS_AWAY:
        return EGUI_COLOR_HEX(0xC17C00);
    case EGUI_VIEW_PERSONA_STATUS_DO_NOT_DISTURB:
        return EGUI_COLOR_HEX(0xC4314B);
    case EGUI_VIEW_PERSONA_STATUS_OFFLINE:
        return local->neutral_color;
    default:
        return EGUI_COLOR_HEX(0x107C41);
    }
}

static uint8_t egui_view_persona_should_draw_presence(const egui_view_persona_t *local)
{
    return (uint8_t)(local->status != EGUI_VIEW_PERSONA_STATUS_NONE);
}

static void egui_view_persona_measure_text(const egui_font_t *font, const char *text, egui_dim_t *width, egui_dim_t *height)
{
    egui_dim_t measured_width = 0;
    egui_dim_t measured_height = 0;

    if (font != NULL && font->api != NULL && font->api->get_str_size != NULL && text != NULL && text[0] != '\0')
    {
        font->api->get_str_size(font, text, 0, 0, &measured_width, &measured_height);
    }
    if (measured_height <= 0)
    {
        measured_height = 10;
    }
    if (width != NULL)
    {
        *width = measured_width;
    }
    if (height != NULL)
    {
        *height = measured_height;
    }
}

static void egui_view_persona_local_region_to_screen(egui_view_t *self, const egui_region_t *local_region, egui_region_t *screen_region)
{
    screen_region->location.x = self->region_screen.location.x + local_region->location.x;
    screen_region->location.y = self->region_screen.location.y + local_region->location.y;
    screen_region->size.width = local_region->size.width;
    screen_region->size.height = local_region->size.height;
}

static void egui_view_persona_draw_text(egui_view_t *self, const egui_font_t *font, const char *text, const egui_region_t *region, uint8_t align,
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

static void egui_view_persona_get_metrics(egui_view_persona_t *local, egui_view_t *self, egui_view_persona_metrics_t *metrics)
{
    egui_dim_t pad_x = local->compact_mode ? 8 : 10;
    egui_dim_t pad_y = local->compact_mode ? 8 : 10;
    egui_dim_t gap = local->compact_mode ? 8 : 10;
    egui_dim_t avatar_size = local->compact_mode ? 30 : 44;
    egui_dim_t section_extra = local->compact_mode ? 8 : 12;
    egui_dim_t min_content_w = local->compact_mode ? 28 : 44;
    egui_dim_t available_w;
    egui_dim_t available_h;
    egui_dim_t section_w;
    egui_dim_t title_height;
    egui_dim_t meta_height;
    egui_dim_t line_gap = local->compact_mode ? 2 : 3;
    egui_dim_t total_height;
    egui_dim_t cursor_y;

    memset(metrics, 0, sizeof(*metrics));
    egui_view_get_work_region(self, &metrics->panel_region);
    if (egui_region_is_empty(&metrics->panel_region))
    {
        return;
    }

    available_w = metrics->panel_region.size.width - pad_x * 2;
    available_h = metrics->panel_region.size.height - pad_y * 2;
    if (available_w <= 0 || available_h <= 0)
    {
        return;
    }

    if (avatar_size > available_h)
    {
        avatar_size = available_h;
    }
    if (available_w < avatar_size + gap + min_content_w)
    {
        if (available_w > gap + min_content_w)
        {
            avatar_size = available_w - gap - min_content_w;
        }
        else
        {
            avatar_size = available_w / 2;
        }
    }
    if (avatar_size < 18)
    {
        avatar_size = available_w > 0 ? EGUI_MAX(available_w / 3, 12) : 0;
    }
    if (avatar_size > available_h)
    {
        avatar_size = available_h;
    }
    if (avatar_size <= 0)
    {
        return;
    }

    section_w = avatar_size + section_extra;
    if (available_w - section_w - gap < min_content_w)
    {
        section_w = available_w - gap - min_content_w;
    }
    if (section_w < avatar_size)
    {
        section_w = avatar_size;
    }
    if (section_w > available_w)
    {
        section_w = available_w;
    }

    metrics->section_region.location.x = metrics->panel_region.location.x + pad_x;
    metrics->section_region.location.y = metrics->panel_region.location.y + pad_y;
    metrics->section_region.size.width = section_w;
    metrics->section_region.size.height = available_h;

    metrics->avatar_region.size.width = avatar_size;
    metrics->avatar_region.size.height = avatar_size;
    metrics->avatar_region.location.x = metrics->section_region.location.x + (metrics->section_region.size.width - avatar_size) / 2;
    metrics->avatar_region.location.y = metrics->section_region.location.y + (metrics->section_region.size.height - avatar_size) / 2;

    metrics->content_region.location.x = metrics->section_region.location.x + metrics->section_region.size.width + gap;
    metrics->content_region.location.y = metrics->panel_region.location.y + pad_y;
    metrics->content_region.size.width =
            metrics->panel_region.location.x + metrics->panel_region.size.width - pad_x - metrics->content_region.location.x;
    metrics->content_region.size.height = available_h;
    if (metrics->content_region.size.width <= 0)
    {
        return;
    }

    metrics->show_secondary = (uint8_t)(local->secondary_text != NULL && local->secondary_text[0] != '\0');
    metrics->show_tertiary = (uint8_t)(!local->compact_mode && local->tertiary_text != NULL && local->tertiary_text[0] != '\0');
    metrics->show_quaternary = (uint8_t)(!local->compact_mode && local->quaternary_text != NULL && local->quaternary_text[0] != '\0');
    metrics->show_presence = egui_view_persona_should_draw_presence(local);

    egui_view_persona_measure_text(egui_view_persona_get_font(local), "Ag", NULL, &title_height);
    egui_view_persona_measure_text(egui_view_persona_get_meta_font(local), "Ag", NULL, &meta_height);
    if (title_height < (local->compact_mode ? 10 : 12))
    {
        title_height = local->compact_mode ? 10 : 12;
    }
    if (meta_height < (local->compact_mode ? 8 : 10))
    {
        meta_height = local->compact_mode ? 8 : 10;
    }

    total_height = title_height;
    if (metrics->show_secondary)
    {
        total_height += line_gap + meta_height;
    }
    if (metrics->show_tertiary)
    {
        total_height += line_gap + meta_height;
    }
    if (metrics->show_quaternary)
    {
        total_height += line_gap + meta_height;
    }
    if (total_height > metrics->content_region.size.height)
    {
        metrics->show_quaternary = 0;
        total_height = title_height;
        if (metrics->show_secondary)
        {
            total_height += line_gap + meta_height;
        }
        if (metrics->show_tertiary)
        {
            total_height += line_gap + meta_height;
        }
        if (total_height > metrics->content_region.size.height)
        {
            metrics->show_tertiary = 0;
            total_height = title_height;
            if (metrics->show_secondary)
            {
                total_height += line_gap + meta_height;
            }
        }
    }

    cursor_y = metrics->content_region.location.y + (metrics->content_region.size.height - total_height) / 2;
    if (cursor_y < metrics->content_region.location.y)
    {
        cursor_y = metrics->content_region.location.y;
    }

    metrics->title_region.location.x = metrics->content_region.location.x;
    metrics->title_region.location.y = cursor_y;
    metrics->title_region.size.width = metrics->content_region.size.width;
    metrics->title_region.size.height = title_height;

    cursor_y += title_height;
    if (metrics->show_secondary)
    {
        cursor_y += line_gap;
        metrics->secondary_region.location.x = metrics->content_region.location.x;
        metrics->secondary_region.location.y = cursor_y;
        metrics->secondary_region.size.width = metrics->content_region.size.width;
        metrics->secondary_region.size.height = meta_height;
        cursor_y += meta_height;
    }
    if (metrics->show_tertiary)
    {
        cursor_y += line_gap;
        metrics->tertiary_region.location.x = metrics->content_region.location.x;
        metrics->tertiary_region.location.y = cursor_y;
        metrics->tertiary_region.size.width = metrics->content_region.size.width;
        metrics->tertiary_region.size.height = meta_height;
        cursor_y += meta_height;
    }
    if (metrics->show_quaternary)
    {
        cursor_y += line_gap;
        metrics->quaternary_region.location.x = metrics->content_region.location.x;
        metrics->quaternary_region.location.y = cursor_y;
        metrics->quaternary_region.size.width = metrics->content_region.size.width;
        metrics->quaternary_region.size.height = meta_height;
    }

    if (metrics->show_presence)
    {
        egui_dim_t dot_size = metrics->avatar_region.size.width / (local->compact_mode ? 4 : 3);
        egui_dim_t margin = local->compact_mode ? 1 : 2;

        if (dot_size < (local->compact_mode ? 6 : 8))
        {
            dot_size = local->compact_mode ? 6 : 8;
        }
        if (dot_size > metrics->avatar_region.size.width / 2)
        {
            dot_size = metrics->avatar_region.size.width / 2;
        }
        metrics->presence_region.size.width = dot_size;
        metrics->presence_region.size.height = dot_size;
        metrics->presence_region.location.x = metrics->avatar_region.location.x + metrics->avatar_region.size.width - dot_size - margin;
        metrics->presence_region.location.y = metrics->avatar_region.location.y + metrics->avatar_region.size.height - dot_size - margin;
    }
}

static void egui_view_persona_draw_do_not_disturb_glyph(egui_view_t *self, const egui_region_t *region, egui_color_t glyph_color)
{
    egui_dim_t center_y;
    egui_dim_t start_x;
    egui_dim_t end_x;
    egui_dim_t stroke_width;
    egui_dim_t horizontal_inset;

    if (region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    horizontal_inset = region->size.width / 4;
    if (horizontal_inset < 2)
    {
        horizontal_inset = 2;
    }
    if (horizontal_inset * 2 >= region->size.width)
    {
        horizontal_inset = region->size.width / 3;
    }

    start_x = region->location.x + horizontal_inset;
    end_x = region->location.x + region->size.width - horizontal_inset;
    center_y = region->location.y + region->size.height / 2;
    stroke_width = region->size.height >= 14 ? 2 : 1;

    egui_canvas_draw_line(start_x, center_y, end_x, center_y, stroke_width, glyph_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
}

static void egui_view_persona_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);
    egui_view_persona_metrics_t metrics;
    egui_color_t tone_color;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t section_color;
    egui_color_t title_color;
    egui_color_t meta_color;
    egui_color_t avatar_fill;
    egui_color_t avatar_text_color;
    egui_color_t avatar_border;
    egui_color_t presence_color;
    egui_color_t presence_outline;
    egui_color_t shadow_color;
    egui_dim_t panel_radius;
    egui_dim_t section_radius;
    egui_dim_t avatar_center_x;
    egui_dim_t avatar_center_y;
    egui_dim_t avatar_radius;
    char resolved_initials[3];
    const char *title_text;

    egui_view_persona_get_metrics(local, self, &metrics);
    if (metrics.panel_region.size.width <= 0 || metrics.panel_region.size.height <= 0 || metrics.avatar_region.size.width <= 0)
    {
        return;
    }

    tone_color = egui_view_persona_tone_color(local, local->tone);
    surface_color = local->surface_color;
    border_color = egui_rgb_mix(local->border_color, tone_color, local->compact_mode ? 12 : 16);
    section_color = egui_rgb_mix(local->section_color, tone_color, local->compact_mode ? 12 : 16);
    title_color = local->text_color;
    meta_color = local->muted_text_color;
    avatar_fill = tone_color;
    avatar_text_color = EGUI_COLOR_WHITE;
    avatar_border = egui_rgb_mix(tone_color, local->surface_color, 24);
    presence_color = egui_view_persona_status_color(local, local->status);
    presence_outline = local->surface_color;
    shadow_color = egui_rgb_mix(local->border_color, EGUI_COLOR_HEX(0xAAB7C4), 48);
    panel_radius = local->compact_mode ? 10 : 12;
    section_radius = local->compact_mode ? 8 : 10;

    if (local->read_only_mode)
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xF6F8FA), 24);
        border_color = egui_rgb_mix(border_color, local->muted_text_color, 34);
        section_color = egui_rgb_mix(section_color, surface_color, 42);
        title_color = egui_rgb_mix(title_color, local->muted_text_color, 28);
        meta_color = egui_rgb_mix(meta_color, surface_color, 20);
        avatar_fill = egui_rgb_mix(avatar_fill, surface_color, 36);
        avatar_text_color = egui_rgb_mix(avatar_text_color, local->muted_text_color, 46);
        avatar_border = egui_rgb_mix(avatar_border, local->muted_text_color, 44);
        presence_color = egui_rgb_mix(presence_color, local->muted_text_color, 46);
        presence_outline = egui_rgb_mix(presence_outline, local->muted_text_color, 16);
        shadow_color = egui_rgb_mix(shadow_color, surface_color, 48);
    }
    if (!egui_view_get_enable(self))
    {
        surface_color = egui_view_persona_mix_disabled(surface_color);
        border_color = egui_view_persona_mix_disabled(border_color);
        section_color = egui_view_persona_mix_disabled(section_color);
        title_color = egui_view_persona_mix_disabled(title_color);
        meta_color = egui_view_persona_mix_disabled(meta_color);
        avatar_fill = egui_view_persona_mix_disabled(avatar_fill);
        avatar_text_color = egui_view_persona_mix_disabled(avatar_text_color);
        avatar_border = egui_view_persona_mix_disabled(avatar_border);
        presence_color = egui_view_persona_mix_disabled(presence_color);
        presence_outline = egui_view_persona_mix_disabled(presence_outline);
        shadow_color = egui_view_persona_mix_disabled(shadow_color);
    }

    if (!local->compact_mode && metrics.panel_region.size.height >= 34)
    {
        egui_canvas_draw_round_rectangle_fill(metrics.panel_region.location.x, metrics.panel_region.location.y + 2, metrics.panel_region.size.width,
                                              metrics.panel_region.size.height, panel_radius, shadow_color, egui_color_alpha_mix(self->alpha, 22));
    }
    egui_canvas_draw_round_rectangle_fill(metrics.panel_region.location.x, metrics.panel_region.location.y, metrics.panel_region.size.width,
                                          metrics.panel_region.size.height, panel_radius, surface_color, egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_round_rectangle(metrics.panel_region.location.x, metrics.panel_region.location.y, metrics.panel_region.size.width,
                                     metrics.panel_region.size.height, panel_radius, 1, border_color,
                                     egui_color_alpha_mix(self->alpha, local->compact_mode ? 40 : 52));

    egui_canvas_draw_round_rectangle_fill(metrics.section_region.location.x, metrics.section_region.location.y, metrics.section_region.size.width,
                                          metrics.section_region.size.height, section_radius, section_color, egui_color_alpha_mix(self->alpha, 82));
    if (metrics.section_region.size.height > 10)
    {
        egui_dim_t indicator_h = metrics.section_region.size.height - (local->compact_mode ? 12 : 14);
        egui_dim_t indicator_y = metrics.section_region.location.y + (metrics.section_region.size.height - indicator_h) / 2;

        if (indicator_h < 6)
        {
            indicator_h = 6;
            indicator_y = metrics.section_region.location.y + (metrics.section_region.size.height - indicator_h) / 2;
        }
        egui_canvas_draw_round_rectangle_fill(metrics.section_region.location.x + 4, indicator_y, local->compact_mode ? 3 : 4, indicator_h, 1, tone_color,
                                              egui_color_alpha_mix(self->alpha, 62));
    }

    avatar_center_x = metrics.avatar_region.location.x + metrics.avatar_region.size.width / 2;
    avatar_center_y = metrics.avatar_region.location.y + metrics.avatar_region.size.height / 2;
    avatar_radius = metrics.avatar_region.size.width / 2;
    egui_canvas_draw_circle_fill_basic(avatar_center_x, avatar_center_y, avatar_radius, avatar_fill, egui_color_alpha_mix(self->alpha, 92));
    egui_canvas_draw_circle_basic(avatar_center_x, avatar_center_y, avatar_radius, 1, avatar_border, egui_color_alpha_mix(self->alpha, 36));

    egui_view_persona_resolve_initials(local->initials, local->display_name, resolved_initials);
    if (resolved_initials[0] != '\0')
    {
        egui_view_persona_draw_text(self, egui_view_persona_get_font(local), resolved_initials, &metrics.avatar_region, EGUI_ALIGN_CENTER, avatar_text_color,
                                    EGUI_ALPHA_100);
    }
    else
    {
        egui_view_persona_draw_text(self, egui_view_persona_default_icon_font(metrics.avatar_region.size.width), EGUI_ICON_MS_PERSON, &metrics.avatar_region,
                                    EGUI_ALIGN_CENTER, avatar_text_color, EGUI_ALPHA_100);
    }

    if (metrics.show_presence && metrics.presence_region.size.width > 0)
    {
        egui_dim_t presence_center_x = metrics.presence_region.location.x + metrics.presence_region.size.width / 2;
        egui_dim_t presence_center_y = metrics.presence_region.location.y + metrics.presence_region.size.height / 2;
        egui_dim_t presence_radius = metrics.presence_region.size.width / 2;

        egui_canvas_draw_circle_fill_basic(presence_center_x, presence_center_y, presence_radius, presence_outline,
                                           egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
        if (local->status == EGUI_VIEW_PERSONA_STATUS_OFFLINE)
        {
            if (presence_radius > 1)
            {
                egui_canvas_draw_circle_basic(presence_center_x, presence_center_y, presence_radius - 1, 1, presence_color,
                                              egui_color_alpha_mix(self->alpha, 92));
            }
        }
        else if (presence_radius > 1)
        {
            egui_canvas_draw_circle_fill_basic(presence_center_x, presence_center_y, presence_radius - 1, presence_color,
                                               egui_color_alpha_mix(self->alpha, 92));
            if (local->status == EGUI_VIEW_PERSONA_STATUS_DO_NOT_DISTURB)
            {
                egui_view_persona_draw_do_not_disturb_glyph(self, &metrics.presence_region, EGUI_COLOR_WHITE);
            }
        }
    }

    title_text = (local->display_name != NULL && local->display_name[0] != '\0') ? local->display_name : resolved_initials;
    egui_view_persona_draw_text(self, egui_view_persona_get_font(local), title_text, &metrics.title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color,
                                EGUI_ALPHA_100);
    if (metrics.show_secondary)
    {
        egui_view_persona_draw_text(self, egui_view_persona_get_meta_font(local), local->secondary_text, &metrics.secondary_region,
                                    EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, meta_color, EGUI_ALPHA_100);
    }
    if (metrics.show_tertiary)
    {
        egui_view_persona_draw_text(self, egui_view_persona_get_meta_font(local), local->tertiary_text, &metrics.tertiary_region,
                                    EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, meta_color, 88);
    }
    if (metrics.show_quaternary)
    {
        egui_view_persona_draw_text(self, egui_view_persona_get_meta_font(local), local->quaternary_text, &metrics.quaternary_region,
                                    EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, meta_color, 74);
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_persona_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_persona_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_persona_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_persona_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_persona_set_display_name(egui_view_t *self, const char *display_name)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    egui_view_persona_clear_pressed_state(self);
    local->display_name = display_name;
    egui_view_invalidate(self);
}

const char *egui_view_persona_get_display_name(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    return local->display_name;
}

void egui_view_persona_set_secondary_text(egui_view_t *self, const char *secondary_text)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    egui_view_persona_clear_pressed_state(self);
    local->secondary_text = secondary_text;
    egui_view_invalidate(self);
}

const char *egui_view_persona_get_secondary_text(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    return local->secondary_text;
}

void egui_view_persona_set_tertiary_text(egui_view_t *self, const char *tertiary_text)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    egui_view_persona_clear_pressed_state(self);
    local->tertiary_text = tertiary_text;
    egui_view_invalidate(self);
}

const char *egui_view_persona_get_tertiary_text(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    return local->tertiary_text;
}

void egui_view_persona_set_quaternary_text(egui_view_t *self, const char *quaternary_text)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    egui_view_persona_clear_pressed_state(self);
    local->quaternary_text = quaternary_text;
    egui_view_invalidate(self);
}

const char *egui_view_persona_get_quaternary_text(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    return local->quaternary_text;
}

void egui_view_persona_set_initials(egui_view_t *self, const char *initials)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    egui_view_persona_clear_pressed_state(self);
    local->initials = initials;
    egui_view_invalidate(self);
}

const char *egui_view_persona_get_initials(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    return local->initials;
}

void egui_view_persona_set_status(egui_view_t *self, uint8_t status)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    egui_view_persona_clear_pressed_state(self);
    local->status = egui_view_persona_clamp_status(status);
    egui_view_invalidate(self);
}

uint8_t egui_view_persona_get_status(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    return local->status;
}

void egui_view_persona_set_tone(egui_view_t *self, uint8_t tone)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    egui_view_persona_clear_pressed_state(self);
    local->tone = egui_view_persona_clamp_tone(tone);
    egui_view_invalidate(self);
}

uint8_t egui_view_persona_get_tone(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    return local->tone;
}

void egui_view_persona_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    egui_view_persona_clear_pressed_state(self);
    local->font = font != NULL ? font : egui_view_persona_default_font();
    egui_view_invalidate(self);
}

void egui_view_persona_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    egui_view_persona_clear_pressed_state(self);
    local->meta_font = font != NULL ? font : egui_view_persona_default_font();
    egui_view_invalidate(self);
}

void egui_view_persona_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    egui_view_persona_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_persona_get_compact_mode(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    return local->compact_mode;
}

void egui_view_persona_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    egui_view_persona_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_persona_get_read_only_mode(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    return local->read_only_mode;
}

void egui_view_persona_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t section_color,
                                   egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t success_color,
                                   egui_color_t warning_color, egui_color_t neutral_color)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);

    egui_view_persona_clear_pressed_state(self);
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

uint8_t egui_view_persona_get_avatar_region(egui_view_t *self, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);
    egui_view_persona_metrics_t metrics;

    if (region == NULL)
    {
        return 0;
    }

    egui_view_persona_get_metrics(local, self, &metrics);
    if (metrics.avatar_region.size.width <= 0 || metrics.avatar_region.size.height <= 0)
    {
        return 0;
    }

    egui_view_persona_local_region_to_screen(self, &metrics.avatar_region, region);
    return 1;
}

uint8_t egui_view_persona_get_presence_region(egui_view_t *self, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_persona_t);
    egui_view_persona_metrics_t metrics;

    if (region == NULL)
    {
        return 0;
    }

    egui_view_persona_get_metrics(local, self, &metrics);
    if (!metrics.show_presence || metrics.presence_region.size.width <= 0 || metrics.presence_region.size.height <= 0)
    {
        return 0;
    }

    egui_view_persona_local_region_to_screen(self, &metrics.presence_region, region);
    return 1;
}

void egui_view_persona_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_persona_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_persona_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_persona_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
        .on_touch_event = egui_view_on_touch_event,
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_persona_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

void egui_view_persona_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_persona_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_persona_t);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);
    egui_view_set_focusable(self, 0);

    local->display_name = NULL;
    local->secondary_text = NULL;
    local->tertiary_text = NULL;
    local->quaternary_text = NULL;
    local->initials = NULL;
    local->font = egui_view_persona_default_font();
    local->meta_font = egui_view_persona_default_font();
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xD2DBE3);
    local->section_color = EGUI_COLOR_HEX(0xEEF3F7);
    local->text_color = EGUI_COLOR_HEX(0x1A2734);
    local->muted_text_color = EGUI_COLOR_HEX(0x6B7A89);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->success_color = EGUI_COLOR_HEX(0x0F7B45);
    local->warning_color = EGUI_COLOR_HEX(0x9D5D00);
    local->neutral_color = EGUI_COLOR_HEX(0x7A8796);
    local->tone = EGUI_VIEW_PERSONA_TONE_NEUTRAL;
    local->status = EGUI_VIEW_PERSONA_STATUS_NONE;
    local->compact_mode = 0;
    local->read_only_mode = 0;

    egui_view_set_view_name(self, "egui_view_persona");
}
