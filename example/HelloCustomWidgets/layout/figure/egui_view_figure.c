#include "egui_view_figure.h"

#include <string.h>

#define EGUI_VIEW_FIGURE_WIDTH_MIN      42
#define EGUI_VIEW_FIGURE_WIDTH_MAX      118
#define EGUI_VIEW_FIGURE_HEIGHT_MIN     22
#define EGUI_VIEW_FIGURE_HEIGHT_MAX     58
#define EGUI_VIEW_FIGURE_GAP_MAX        12
#define EGUI_VIEW_FIGURE_RADIUS_MAX     16
#define EGUI_VIEW_FIGURE_TEXT_H         12
#define EGUI_VIEW_FIGURE_TEXT_H_COMPACT 10
#define EGUI_VIEW_FIGURE_CHILD_PAD_X    5
#define EGUI_VIEW_FIGURE_CHILD_PAD_Y    3

static egui_view_figure_t *egui_view_figure_local(egui_view_t *self)
{
    return (egui_view_figure_t *)self;
}

static uint8_t egui_view_figure_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static uint8_t egui_view_figure_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static void egui_view_figure_copy_text(char *dst, uint8_t capacity, const char *src)
{
    size_t length = 0;

    if (dst == NULL || capacity == 0)
    {
        return;
    }
    if (src != NULL)
    {
        length = strlen(src);
        if (length >= capacity)
        {
            length = capacity - 1;
        }
        if (length > 0)
        {
            memcpy(dst, src, length);
        }
    }
    dst[length] = '\0';
}

static egui_dim_t egui_view_figure_clamp_dim(egui_dim_t value, egui_dim_t min_value, egui_dim_t max_value)
{
    if (value < min_value)
    {
        return min_value;
    }
    if (value > max_value)
    {
        return max_value;
    }
    return value;
}

static egui_color_t egui_view_figure_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x8A97A5), 58);
}

static egui_dim_t egui_view_figure_text_height(egui_view_figure_t *local)
{
    return local->compact_mode ? EGUI_VIEW_FIGURE_TEXT_H_COMPACT : EGUI_VIEW_FIGURE_TEXT_H;
}

static void egui_view_figure_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, egui_color_t color,
                                       egui_alpha_t alpha)
{
    egui_region_t draw_region = *region;

    if (font == NULL || !egui_view_figure_has_text(text) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, color,
                                  egui_color_alpha_mix(self->alpha, alpha));
}

void egui_view_figure_get_regions(egui_view_t *self, egui_region_t *leading_region, egui_region_t *figure_region, egui_region_t *wrap_region,
                                  egui_region_t *trailing_region)
{
    egui_view_figure_t *local = egui_view_figure_local(self);
    egui_region_t work_region;
    egui_dim_t text_height = egui_view_figure_text_height(local);
    egui_dim_t gap = local->wrap_gap;
    egui_dim_t middle_y;
    egui_dim_t middle_height;
    egui_dim_t available_height;
    egui_dim_t figure_width = local->figure_width;
    egui_dim_t figure_height = local->figure_height;
    egui_dim_t figure_x;
    egui_dim_t figure_y;
    egui_dim_t right;

    egui_view_get_work_region(self, &work_region);
    if (work_region.size.width < 0)
    {
        work_region.size.width = 0;
    }
    if (work_region.size.height < 0)
    {
        work_region.size.height = 0;
    }

    if (text_height * 2 > work_region.size.height)
    {
        text_height = work_region.size.height / 2;
    }
    available_height = work_region.size.height - text_height * 2;
    if (available_height < 0)
    {
        available_height = 0;
    }
    if (gap * 2 > available_height)
    {
        gap = available_height / 2;
    }
    middle_y = work_region.location.y + text_height + gap;
    middle_height = available_height - gap * 2;
    if (middle_height < 0)
    {
        middle_height = 0;
    }

    if (figure_width > work_region.size.width)
    {
        figure_width = work_region.size.width;
    }
    if (figure_height > middle_height)
    {
        figure_height = middle_height;
    }
    right = work_region.location.x + work_region.size.width;
    if (local->anchor == EGUI_VIEW_FIGURE_ANCHOR_RIGHT)
    {
        figure_x = right - figure_width;
    }
    else if (local->anchor == EGUI_VIEW_FIGURE_ANCHOR_CENTER)
    {
        figure_x = work_region.location.x + (work_region.size.width - figure_width) / 2;
    }
    else
    {
        figure_x = work_region.location.x;
    }
    figure_y = middle_y;

    if (leading_region != NULL)
    {
        leading_region->location.x = work_region.location.x;
        leading_region->location.y = work_region.location.y;
        leading_region->size.width = work_region.size.width;
        leading_region->size.height = text_height;
    }
    if (figure_region != NULL)
    {
        figure_region->location.x = figure_x;
        figure_region->location.y = figure_y;
        figure_region->size.width = figure_width;
        figure_region->size.height = figure_height;
    }
    if (wrap_region != NULL)
    {
        if (local->anchor == EGUI_VIEW_FIGURE_ANCHOR_RIGHT)
        {
            wrap_region->location.x = work_region.location.x;
            wrap_region->location.y = figure_y;
            wrap_region->size.width = figure_x - gap - work_region.location.x;
            wrap_region->size.height = figure_height;
        }
        else if (local->anchor == EGUI_VIEW_FIGURE_ANCHOR_CENTER)
        {
            wrap_region->location.x = work_region.location.x;
            wrap_region->location.y = figure_y + figure_height + gap;
            wrap_region->size.width = work_region.size.width;
            wrap_region->size.height = middle_y + middle_height - wrap_region->location.y;
        }
        else
        {
            wrap_region->location.x = figure_x + figure_width + gap;
            wrap_region->location.y = figure_y;
            wrap_region->size.width = right - wrap_region->location.x;
            wrap_region->size.height = figure_height;
        }
        if (wrap_region->size.width < 0)
        {
            wrap_region->size.width = 0;
        }
        if (wrap_region->size.height < 0)
        {
            wrap_region->size.height = 0;
        }
    }
    if (trailing_region != NULL)
    {
        trailing_region->location.x = work_region.location.x;
        trailing_region->location.y = middle_y + middle_height + gap;
        trailing_region->size.width = work_region.size.width;
        trailing_region->size.height = text_height;
    }
}

static void egui_view_figure_on_draw(egui_view_t *self)
{
    egui_view_figure_t *local = egui_view_figure_local(self);
    egui_region_t work_region;
    egui_region_t leading_region;
    egui_region_t figure_region;
    egui_region_t wrap_region;
    egui_region_t trailing_region;
    egui_color_t surface_color = local->surface_color;
    egui_color_t border_color = local->border_color;
    egui_color_t text_color = local->text_color;
    egui_color_t figure_surface_color = local->figure_surface_color;
    egui_color_t figure_border_color = local->figure_border_color;
    egui_color_t accent_color = local->accent_color;
    egui_alpha_t text_alpha = EGUI_ALPHA_100;
    egui_alpha_t figure_alpha = local->compact_mode ? 80 : 94;

    egui_view_get_work_region(self, &work_region);
    if (work_region.size.width <= 0 || work_region.size.height <= 0)
    {
        return;
    }

    if (local->read_only_mode)
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xF7F9FB), 52);
        border_color = egui_rgb_mix(border_color, EGUI_COLOR_HEX(0xAEB8C2), 54);
        text_color = egui_rgb_mix(text_color, EGUI_COLOR_HEX(0x8A97A5), 48);
        figure_surface_color = egui_rgb_mix(figure_surface_color, EGUI_COLOR_HEX(0xF7F9FB), 50);
        figure_border_color = egui_rgb_mix(figure_border_color, EGUI_COLOR_HEX(0xAEB8C2), 56);
        accent_color = egui_rgb_mix(accent_color, EGUI_COLOR_HEX(0x8A97A5), 58);
        text_alpha = 84;
        figure_alpha = 66;
    }
    if (!egui_view_get_enable(self))
    {
        surface_color = egui_view_figure_mix_disabled(surface_color);
        border_color = egui_view_figure_mix_disabled(border_color);
        text_color = egui_view_figure_mix_disabled(text_color);
        figure_surface_color = egui_view_figure_mix_disabled(figure_surface_color);
        figure_border_color = egui_view_figure_mix_disabled(figure_border_color);
        accent_color = egui_view_figure_mix_disabled(accent_color);
        text_alpha = 66;
        figure_alpha = 52;
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, work_region.location.x, work_region.location.y, work_region.size.width,
                                          work_region.size.height, local->corner_radius + 2, surface_color,
                                          egui_color_alpha_mix(self->alpha, local->compact_mode ? 66 : 88));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, work_region.location.x, work_region.location.y, work_region.size.width,
                                     work_region.size.height, local->corner_radius + 2, 1, border_color,
                                     egui_color_alpha_mix(self->alpha, local->compact_mode ? 28 : 38));

    egui_view_figure_get_regions(self, &leading_region, &figure_region, &wrap_region, &trailing_region);
    egui_view_figure_draw_text(local->text_font, self, local->leading_text, &leading_region, text_color, text_alpha);
    egui_view_figure_draw_text(local->text_font, self, local->wrap_text, &wrap_region, text_color, text_alpha);
    egui_view_figure_draw_text(local->text_font, self, local->trailing_text, &trailing_region, text_color, text_alpha);

    if (figure_region.size.width > 0 && figure_region.size.height > 0)
    {
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, figure_region.location.x, figure_region.location.y, figure_region.size.width,
                                              figure_region.size.height, local->corner_radius, figure_surface_color,
                                              egui_color_alpha_mix(self->alpha, figure_alpha));
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, figure_region.location.x, figure_region.location.y, figure_region.size.width,
                                         figure_region.size.height, local->corner_radius, 1, figure_border_color, egui_color_alpha_mix(self->alpha, 72));
        if (figure_region.size.height > 8)
        {
            egui_canvas_draw_rectangle_fill(&uicode_get_core()->canvas, figure_region.location.x, figure_region.location.y + 4, 2,
                                            figure_region.size.height - 8, accent_color, egui_color_alpha_mix(self->alpha, local->read_only_mode ? 28 : 48));
        }
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_figure_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_figure_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_figure_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_figure_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_figure_set_child(egui_view_t *self, egui_view_t *child)
{
    egui_view_figure_t *local = egui_view_figure_local(self);

    if (local->child == child)
    {
        return;
    }
    if (local->child != NULL)
    {
        egui_view_group_remove_child(self, local->child);
    }
    local->child = child;
    if (child != NULL)
    {
        egui_view_group_add_child(self, child);
        egui_view_figure_layout_child(self);
    }
    egui_view_invalidate(self);
}

egui_view_t *egui_view_figure_get_child(egui_view_t *self)
{
    return egui_view_figure_local(self)->child;
}

void egui_view_figure_layout_child(egui_view_t *self)
{
    egui_view_figure_t *local = egui_view_figure_local(self);
    egui_region_t figure_region;
    egui_region_t content_region;
    egui_region_t child_region;

    if (local->child == NULL)
    {
        return;
    }

    egui_view_figure_get_regions(self, NULL, &figure_region, NULL, NULL);
    content_region.location.x = figure_region.location.x + EGUI_VIEW_FIGURE_CHILD_PAD_X;
    content_region.location.y = figure_region.location.y + EGUI_VIEW_FIGURE_CHILD_PAD_Y;
    content_region.size.width = figure_region.size.width - EGUI_VIEW_FIGURE_CHILD_PAD_X * 2;
    content_region.size.height = figure_region.size.height - EGUI_VIEW_FIGURE_CHILD_PAD_Y * 2;
    if (content_region.size.width < 0)
    {
        content_region.size.width = 0;
    }
    if (content_region.size.height < 0)
    {
        content_region.size.height = 0;
    }

    child_region.size.width = local->child->region.size.width;
    child_region.size.height = local->child->region.size.height;
    child_region.location.x = content_region.location.x + (content_region.size.width - child_region.size.width) / 2;
    child_region.location.y = content_region.location.y + (content_region.size.height - child_region.size.height) / 2;
    if (child_region.location.x < content_region.location.x)
    {
        child_region.location.x = content_region.location.x;
    }
    if (child_region.location.y < content_region.location.y)
    {
        child_region.location.y = content_region.location.y;
    }
    egui_view_layout(local->child, &child_region);
}

void egui_view_figure_set_text(egui_view_t *self, const char *leading_text, const char *wrap_text, const char *trailing_text)
{
    egui_view_figure_t *local = egui_view_figure_local(self);

    egui_view_figure_clear_pressed_state(self);
    egui_view_figure_copy_text(local->leading_text, sizeof(local->leading_text), leading_text);
    egui_view_figure_copy_text(local->wrap_text, sizeof(local->wrap_text), wrap_text);
    egui_view_figure_copy_text(local->trailing_text, sizeof(local->trailing_text), trailing_text);
    egui_view_invalidate(self);
}

const char *egui_view_figure_get_leading_text(egui_view_t *self)
{
    return egui_view_figure_local(self)->leading_text;
}

const char *egui_view_figure_get_wrap_text(egui_view_t *self)
{
    return egui_view_figure_local(self)->wrap_text;
}

const char *egui_view_figure_get_trailing_text(egui_view_t *self)
{
    return egui_view_figure_local(self)->trailing_text;
}

void egui_view_figure_set_font(egui_view_t *self, const egui_font_t *font)
{
    egui_view_figure_t *local = egui_view_figure_local(self);

    egui_view_figure_clear_pressed_state(self);
    local->text_font = font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : font;
    egui_view_invalidate(self);
}

void egui_view_figure_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                  egui_color_t figure_surface_color, egui_color_t figure_border_color, egui_color_t accent_color)
{
    egui_view_figure_t *local = egui_view_figure_local(self);

    egui_view_figure_clear_pressed_state(self);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->figure_surface_color = figure_surface_color;
    local->figure_border_color = figure_border_color;
    local->accent_color = accent_color;
    egui_view_invalidate(self);
}

void egui_view_figure_set_metrics(egui_view_t *self, egui_dim_t figure_width, egui_dim_t figure_height, egui_dim_t wrap_gap, egui_dim_t corner_radius)
{
    egui_view_figure_t *local = egui_view_figure_local(self);

    egui_view_figure_clear_pressed_state(self);
    local->figure_width = egui_view_figure_clamp_dim(figure_width, EGUI_VIEW_FIGURE_WIDTH_MIN, EGUI_VIEW_FIGURE_WIDTH_MAX);
    local->figure_height = egui_view_figure_clamp_dim(figure_height, EGUI_VIEW_FIGURE_HEIGHT_MIN, EGUI_VIEW_FIGURE_HEIGHT_MAX);
    local->wrap_gap = egui_view_figure_clamp_dim(wrap_gap, 0, EGUI_VIEW_FIGURE_GAP_MAX);
    local->corner_radius = egui_view_figure_clamp_dim(corner_radius, 0, EGUI_VIEW_FIGURE_RADIUS_MAX);
    egui_view_figure_layout_child(self);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_figure_get_figure_width(egui_view_t *self)
{
    return egui_view_figure_local(self)->figure_width;
}

egui_dim_t egui_view_figure_get_figure_height(egui_view_t *self)
{
    return egui_view_figure_local(self)->figure_height;
}

egui_dim_t egui_view_figure_get_wrap_gap(egui_view_t *self)
{
    return egui_view_figure_local(self)->wrap_gap;
}

egui_dim_t egui_view_figure_get_corner_radius(egui_view_t *self)
{
    return egui_view_figure_local(self)->corner_radius;
}

void egui_view_figure_set_anchor(egui_view_t *self, uint8_t anchor)
{
    egui_view_figure_t *local = egui_view_figure_local(self);

    egui_view_figure_clear_pressed_state(self);
    if (anchor > EGUI_VIEW_FIGURE_ANCHOR_CENTER)
    {
        anchor = EGUI_VIEW_FIGURE_ANCHOR_LEFT;
    }
    local->anchor = anchor;
    egui_view_figure_layout_child(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_figure_get_anchor(egui_view_t *self)
{
    return egui_view_figure_local(self)->anchor;
}

void egui_view_figure_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    egui_view_figure_t *local = egui_view_figure_local(self);

    egui_view_figure_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_figure_layout_child(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_figure_get_compact_mode(egui_view_t *self)
{
    return egui_view_figure_local(self)->compact_mode;
}

void egui_view_figure_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    egui_view_figure_t *local = egui_view_figure_local(self);

    egui_view_figure_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_figure_get_read_only_mode(egui_view_t *self)
{
    return egui_view_figure_local(self)->read_only_mode;
}

void egui_view_figure_apply_standard_style(egui_view_t *self)
{
    egui_view_figure_set_palette(self, EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DEE8), EGUI_COLOR_HEX(0x243241), EGUI_COLOR_HEX(0xF7FBFF),
                                 EGUI_COLOR_HEX(0xB8D1E7), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_figure_set_metrics(self, 68, 34, 5, 8);
    egui_view_figure_set_anchor(self, EGUI_VIEW_FIGURE_ANCHOR_LEFT);
    egui_view_figure_set_compact_mode(self, 0);
    egui_view_figure_set_read_only_mode(self, 0);
}

void egui_view_figure_apply_accent_style(egui_view_t *self)
{
    egui_view_figure_set_palette(self, EGUI_COLOR_HEX(0xF7FBFF), EGUI_COLOR_HEX(0xB8D1E7), EGUI_COLOR_HEX(0x173247), EGUI_COLOR_HEX(0xEAF4FF),
                                 EGUI_COLOR_HEX(0x78A8D5), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_figure_set_metrics(self, 72, 34, 5, 8);
    egui_view_figure_set_anchor(self, EGUI_VIEW_FIGURE_ANCHOR_RIGHT);
    egui_view_figure_set_compact_mode(self, 0);
    egui_view_figure_set_read_only_mode(self, 0);
}

void egui_view_figure_apply_compact_style(egui_view_t *self)
{
    egui_view_figure_set_palette(self, EGUI_COLOR_HEX(0xF8FBFA), EGUI_COLOR_HEX(0xD3DFDE), EGUI_COLOR_HEX(0x22313C), EGUI_COLOR_HEX(0xEEF8F6),
                                 EGUI_COLOR_HEX(0xA7CCC7), EGUI_COLOR_HEX(0x0C7C73));
    egui_view_figure_set_metrics(self, 56, 26, 4, 6);
    egui_view_figure_set_anchor(self, EGUI_VIEW_FIGURE_ANCHOR_CENTER);
    egui_view_figure_set_compact_mode(self, 1);
    egui_view_figure_set_read_only_mode(self, 0);
}

void egui_view_figure_apply_read_only_style(egui_view_t *self)
{
    egui_view_figure_set_palette(self, EGUI_COLOR_HEX(0xF5F7FA), EGUI_COLOR_HEX(0xD8E0E8), EGUI_COLOR_HEX(0x6B7785), EGUI_COLOR_HEX(0xF7F9FB),
                                 EGUI_COLOR_HEX(0xD0D8E0), EGUI_COLOR_HEX(0x788593));
    egui_view_figure_set_metrics(self, 56, 26, 4, 6);
    egui_view_figure_set_anchor(self, EGUI_VIEW_FIGURE_ANCHOR_CENTER);
    egui_view_figure_set_compact_mode(self, 1);
    egui_view_figure_set_read_only_mode(self, 1);
}

void egui_view_figure_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_figure_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_figure_on_static_key_event;
#endif
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_figure_t) = {
        .dispatch_touch_event = egui_view_group_dispatch_touch_event,
        .on_touch_event = egui_view_group_on_touch_event,
        .on_intercept_touch_event = egui_view_group_on_intercept_touch_event,
        .compute_scroll = egui_view_group_compute_scroll,
        .calculate_layout = egui_view_group_calculate_layout,
        .request_layout = egui_view_group_request_layout,
        .draw = egui_view_group_draw,
        .on_attach_to_window = egui_view_group_on_attach_to_window,
        .on_draw = egui_view_figure_on_draw,
        .on_detach_from_window = egui_view_group_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_group_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

void egui_view_figure_init(egui_view_t *self)
{
    egui_view_figure_t *local = egui_view_figure_local(self);

    egui_view_group_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_figure_t);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 4);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->child = NULL;
    local->text_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_figure_copy_text(local->leading_text, sizeof(local->leading_text), "Intro text");
    egui_view_figure_copy_text(local->wrap_text, sizeof(local->wrap_text), "wrapped context");
    egui_view_figure_copy_text(local->trailing_text, sizeof(local->trailing_text), "after figure");
    local->anchor = EGUI_VIEW_FIGURE_ANCHOR_LEFT;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    egui_view_figure_apply_standard_style(self);
    egui_view_set_view_name(self, "egui_view_figure");
}
