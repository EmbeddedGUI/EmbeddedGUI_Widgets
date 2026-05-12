#include "egui_view_block_ui_container.h"
#include "../../hcw_selection_marker.h"

#include <string.h>

#define EGUI_VIEW_BLOCK_UI_CONTAINER_PAD_X_MIN      2
#define EGUI_VIEW_BLOCK_UI_CONTAINER_PAD_X_MAX      16
#define EGUI_VIEW_BLOCK_UI_CONTAINER_PAD_Y_MIN      1
#define EGUI_VIEW_BLOCK_UI_CONTAINER_PAD_Y_MAX      12
#define EGUI_VIEW_BLOCK_UI_CONTAINER_GAP_MAX        12
#define EGUI_VIEW_BLOCK_UI_CONTAINER_RADIUS_MAX     16
#define EGUI_VIEW_BLOCK_UI_CONTAINER_TEXT_H         12
#define EGUI_VIEW_BLOCK_UI_CONTAINER_TEXT_H_COMPACT 10
#define EGUI_VIEW_BLOCK_UI_CONTAINER_TEXT_INSET_X   3

static egui_view_block_ui_container_t *egui_view_block_ui_container_local(egui_view_t *self)
{
    return (egui_view_block_ui_container_t *)self;
}

static uint8_t egui_view_block_ui_container_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static uint8_t egui_view_block_ui_container_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static void egui_view_block_ui_container_copy_text(char *dst, uint8_t capacity, const char *src)
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

static egui_dim_t egui_view_block_ui_container_clamp_dim(egui_dim_t value, egui_dim_t min_value, egui_dim_t max_value)
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

static egui_color_t egui_view_block_ui_container_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_MAKE(38));
}

static egui_dim_t egui_view_block_ui_container_text_height(egui_view_block_ui_container_t *local)
{
    return local->compact_mode ? EGUI_VIEW_BLOCK_UI_CONTAINER_TEXT_H_COMPACT : EGUI_VIEW_BLOCK_UI_CONTAINER_TEXT_H;
}

static void egui_view_block_ui_container_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region,
                                                   egui_color_t color, egui_alpha_t alpha)
{
    egui_region_t draw_region = *region;

    if (font == NULL || !egui_view_block_ui_container_has_text(text) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    if (draw_region.size.width > EGUI_VIEW_BLOCK_UI_CONTAINER_TEXT_INSET_X * 2)
    {
        draw_region.location.x += EGUI_VIEW_BLOCK_UI_CONTAINER_TEXT_INSET_X;
        draw_region.size.width -= EGUI_VIEW_BLOCK_UI_CONTAINER_TEXT_INSET_X * 2;
    }
    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, color,
                                  egui_color_alpha_mix(self->alpha, alpha));
}

void egui_view_block_ui_container_get_regions(egui_view_t *self, egui_region_t *leading_region, egui_region_t *host_region, egui_region_t *trailing_region)
{
    egui_view_block_ui_container_t *local = egui_view_block_ui_container_local(self);
    egui_region_t work_region;
    egui_dim_t text_height = egui_view_block_ui_container_text_height(local);
    egui_dim_t gap = local->block_gap;
    egui_dim_t host_height;
    egui_dim_t available_height;

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
    host_height = available_height - gap * 2;
    if (host_height < 0)
    {
        host_height = 0;
    }

    if (leading_region != NULL)
    {
        leading_region->location.x = work_region.location.x;
        leading_region->location.y = work_region.location.y;
        leading_region->size.width = work_region.size.width;
        leading_region->size.height = text_height;
    }
    if (host_region != NULL)
    {
        host_region->location.x = work_region.location.x;
        host_region->location.y = work_region.location.y + text_height + gap;
        host_region->size.width = work_region.size.width;
        host_region->size.height = host_height;
    }
    if (trailing_region != NULL)
    {
        trailing_region->location.x = work_region.location.x;
        trailing_region->location.y = work_region.location.y + text_height + gap + host_height + gap;
        trailing_region->size.width = work_region.size.width;
        trailing_region->size.height = text_height;
    }
}

static void egui_view_block_ui_container_on_draw(egui_view_t *self)
{
    egui_view_block_ui_container_t *local = egui_view_block_ui_container_local(self);
    egui_region_t work_region;
    egui_region_t leading_region;
    egui_region_t host_region;
    egui_region_t trailing_region;
    egui_color_t surface_color = local->surface_color;
    egui_color_t border_color = local->border_color;
    egui_color_t text_color = local->text_color;
    egui_color_t host_surface_color = local->host_surface_color;
    egui_color_t host_border_color = local->host_border_color;
    egui_color_t accent_color = local->accent_color;
    egui_alpha_t text_alpha = EGUI_ALPHA_100;
    egui_alpha_t host_alpha = EGUI_ALPHA_MAKE(local->compact_mode ? 90 : 98);

    egui_view_get_work_region(self, &work_region);
    if (work_region.size.width <= 0 || work_region.size.height <= 0)
    {
        return;
    }

    if (local->read_only_mode)
    {
        surface_color = egui_rgb_mix(surface_color, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(34));
        border_color = egui_rgb_mix(border_color, HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_MAKE(22));
        text_color = egui_rgb_mix(text_color, HCW_COLOR_TEXT, EGUI_ALPHA_MAKE(12));
        host_surface_color = egui_rgb_mix(host_surface_color, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(32));
        host_border_color = egui_rgb_mix(host_border_color, HCW_COLOR_BORDER_STRONG, EGUI_ALPHA_MAKE(22));
        accent_color = egui_rgb_mix(accent_color, HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_MAKE(18));
        text_alpha = EGUI_ALPHA_100;
        host_alpha = EGUI_ALPHA_MAKE(84);
    }
    if (!egui_view_get_enable(self))
    {
        surface_color = egui_view_block_ui_container_mix_disabled(surface_color);
        border_color = egui_view_block_ui_container_mix_disabled(border_color);
        text_color = egui_view_block_ui_container_mix_disabled(text_color);
        host_surface_color = egui_view_block_ui_container_mix_disabled(host_surface_color);
        host_border_color = egui_view_block_ui_container_mix_disabled(host_border_color);
        accent_color = egui_view_block_ui_container_mix_disabled(accent_color);
        text_alpha = EGUI_ALPHA_MAKE(78);
        host_alpha = EGUI_ALPHA_MAKE(64);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, work_region.location.x, work_region.location.y, work_region.size.width,
                                          work_region.size.height, local->corner_radius + 2, surface_color,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->compact_mode ? 76 : 94)));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, work_region.location.x, work_region.location.y, work_region.size.width,
                                     work_region.size.height, local->corner_radius + 2, 1, border_color,
                                     egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->compact_mode ? 46 : 58)));

    egui_view_block_ui_container_get_regions(self, &leading_region, &host_region, &trailing_region);
    egui_view_block_ui_container_draw_text(local->text_font, self, local->leading_text, &leading_region, text_color, text_alpha);
    egui_view_block_ui_container_draw_text(local->text_font, self, local->trailing_text, &trailing_region, text_color, text_alpha);

    if (host_region.size.width > 0 && host_region.size.height > 0)
    {
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, host_region.location.x, host_region.location.y, host_region.size.width,
                                              host_region.size.height, local->corner_radius, host_surface_color, egui_color_alpha_mix(self->alpha, host_alpha));
        hcw_selection_marker_draw_left(&host_region, local->corner_radius, local->corner_radius, accent_color,
                                       egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->read_only_mode ? 38 : 76)));
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, host_region.location.x, host_region.location.y, host_region.size.width,
                                         host_region.size.height, local->corner_radius, 1, host_border_color,
                                         egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->read_only_mode ? 72 : 92)));
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_block_ui_container_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_block_ui_container_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_block_ui_container_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_block_ui_container_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_block_ui_container_set_child(egui_view_t *self, egui_view_t *child)
{
    egui_view_block_ui_container_t *local = egui_view_block_ui_container_local(self);

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
        egui_view_block_ui_container_layout_child(self);
    }
    egui_view_invalidate(self);
}

egui_view_t *egui_view_block_ui_container_get_child(egui_view_t *self)
{
    return egui_view_block_ui_container_local(self)->child;
}

void egui_view_block_ui_container_layout_child(egui_view_t *self)
{
    egui_view_block_ui_container_t *local = egui_view_block_ui_container_local(self);
    egui_region_t host_region;
    egui_region_t content_region;
    egui_region_t child_region;

    if (local->child == NULL)
    {
        return;
    }

    egui_view_block_ui_container_get_regions(self, NULL, &host_region, NULL);
    content_region.location.x = host_region.location.x + local->host_padding_x;
    content_region.location.y = host_region.location.y + local->host_padding_y;
    content_region.size.width = host_region.size.width - local->host_padding_x * 2;
    content_region.size.height = host_region.size.height - local->host_padding_y * 2;
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

void egui_view_block_ui_container_set_text(egui_view_t *self, const char *leading_text, const char *trailing_text)
{
    egui_view_block_ui_container_t *local = egui_view_block_ui_container_local(self);

    egui_view_block_ui_container_clear_pressed_state(self);
    egui_view_block_ui_container_copy_text(local->leading_text, sizeof(local->leading_text), leading_text);
    egui_view_block_ui_container_copy_text(local->trailing_text, sizeof(local->trailing_text), trailing_text);
    egui_view_invalidate(self);
}

const char *egui_view_block_ui_container_get_leading_text(egui_view_t *self)
{
    return egui_view_block_ui_container_local(self)->leading_text;
}

const char *egui_view_block_ui_container_get_trailing_text(egui_view_t *self)
{
    return egui_view_block_ui_container_local(self)->trailing_text;
}

void egui_view_block_ui_container_set_font(egui_view_t *self, const egui_font_t *font)
{
    egui_view_block_ui_container_t *local = egui_view_block_ui_container_local(self);

    egui_view_block_ui_container_clear_pressed_state(self);
    local->text_font = font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : font;
    egui_view_invalidate(self);
}

void egui_view_block_ui_container_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t text_color,
                                              egui_color_t host_surface_color, egui_color_t host_border_color, egui_color_t accent_color)
{
    egui_view_block_ui_container_t *local = egui_view_block_ui_container_local(self);

    egui_view_block_ui_container_clear_pressed_state(self);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->host_surface_color = host_surface_color;
    local->host_border_color = host_border_color;
    local->accent_color = accent_color;
    egui_view_invalidate(self);
}

void egui_view_block_ui_container_set_metrics(egui_view_t *self, egui_dim_t host_padding_x, egui_dim_t host_padding_y, egui_dim_t block_gap,
                                              egui_dim_t corner_radius)
{
    egui_view_block_ui_container_t *local = egui_view_block_ui_container_local(self);

    egui_view_block_ui_container_clear_pressed_state(self);
    local->host_padding_x =
            egui_view_block_ui_container_clamp_dim(host_padding_x, EGUI_VIEW_BLOCK_UI_CONTAINER_PAD_X_MIN, EGUI_VIEW_BLOCK_UI_CONTAINER_PAD_X_MAX);
    local->host_padding_y =
            egui_view_block_ui_container_clamp_dim(host_padding_y, EGUI_VIEW_BLOCK_UI_CONTAINER_PAD_Y_MIN, EGUI_VIEW_BLOCK_UI_CONTAINER_PAD_Y_MAX);
    local->block_gap = egui_view_block_ui_container_clamp_dim(block_gap, 0, EGUI_VIEW_BLOCK_UI_CONTAINER_GAP_MAX);
    local->corner_radius = egui_view_block_ui_container_clamp_dim(corner_radius, 0, EGUI_VIEW_BLOCK_UI_CONTAINER_RADIUS_MAX);
    egui_view_block_ui_container_layout_child(self);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_block_ui_container_get_host_padding_x(egui_view_t *self)
{
    return egui_view_block_ui_container_local(self)->host_padding_x;
}

egui_dim_t egui_view_block_ui_container_get_host_padding_y(egui_view_t *self)
{
    return egui_view_block_ui_container_local(self)->host_padding_y;
}

egui_dim_t egui_view_block_ui_container_get_block_gap(egui_view_t *self)
{
    return egui_view_block_ui_container_local(self)->block_gap;
}

egui_dim_t egui_view_block_ui_container_get_corner_radius(egui_view_t *self)
{
    return egui_view_block_ui_container_local(self)->corner_radius;
}

void egui_view_block_ui_container_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    egui_view_block_ui_container_t *local = egui_view_block_ui_container_local(self);

    egui_view_block_ui_container_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_block_ui_container_layout_child(self);
    egui_view_invalidate(self);
}

uint8_t egui_view_block_ui_container_get_compact_mode(egui_view_t *self)
{
    return egui_view_block_ui_container_local(self)->compact_mode;
}

void egui_view_block_ui_container_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    egui_view_block_ui_container_t *local = egui_view_block_ui_container_local(self);

    egui_view_block_ui_container_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_block_ui_container_get_read_only_mode(egui_view_t *self)
{
    return egui_view_block_ui_container_local(self)->read_only_mode;
}

void egui_view_block_ui_container_apply_standard_style(egui_view_t *self)
{
    egui_view_block_ui_container_set_palette(self, HCW_COLOR_SURFACE, HCW_COLOR_BORDER, HCW_COLOR_TEXT, HCW_COLOR_SURFACE_SUBTLE,
                                             HCW_COLOR_PRIMARY_SOFT, HCW_COLOR_PRIMARY);
    egui_view_block_ui_container_set_metrics(self, 8, 6, 5, 8);
    egui_view_block_ui_container_set_compact_mode(self, 0);
    egui_view_block_ui_container_set_read_only_mode(self, 0);
}

void egui_view_block_ui_container_apply_accent_style(egui_view_t *self)
{
    egui_view_block_ui_container_set_palette(self, HCW_COLOR_SURFACE_SUBTLE, HCW_COLOR_PRIMARY_SOFT, HCW_COLOR_TEXT, HCW_COLOR_PRIMARY_TINT,
                                             HCW_COLOR_PRIMARY_LIGHT, HCW_COLOR_PRIMARY);
    egui_view_block_ui_container_set_metrics(self, 8, 6, 4, 8);
    egui_view_block_ui_container_set_compact_mode(self, 0);
    egui_view_block_ui_container_set_read_only_mode(self, 0);
}

void egui_view_block_ui_container_apply_compact_style(egui_view_t *self)
{
    egui_view_block_ui_container_set_palette(self, HCW_COLOR_PANEL, HCW_COLOR_BORDER, HCW_COLOR_TEXT, HCW_COLOR_PRIMARY_TINT,
                                             HCW_COLOR_PRIMARY_SOFT, HCW_COLOR_PRIMARY);
    egui_view_block_ui_container_set_metrics(self, 6, 4, 3, 6);
    egui_view_block_ui_container_set_compact_mode(self, 1);
    egui_view_block_ui_container_set_read_only_mode(self, 0);
}

void egui_view_block_ui_container_apply_read_only_style(egui_view_t *self)
{
    egui_view_block_ui_container_set_palette(self, HCW_COLOR_SURFACE_SUBTLE, HCW_COLOR_BORDER_STRONG, HCW_COLOR_TEXT,
                                             HCW_COLOR_SURFACE_SUBTLE, HCW_COLOR_BORDER_STRONG, HCW_COLOR_TEXT_SOFT);
    egui_view_block_ui_container_set_metrics(self, 6, 4, 3, 6);
    egui_view_block_ui_container_set_compact_mode(self, 1);
    egui_view_block_ui_container_set_read_only_mode(self, 1);
}

void egui_view_block_ui_container_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_block_ui_container_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_block_ui_container_on_static_key_event;
#endif
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_block_ui_container_t) = {
        .dispatch_touch_event = egui_view_group_dispatch_touch_event,
        .on_touch_event = egui_view_group_on_touch_event,
        .on_intercept_touch_event = egui_view_group_on_intercept_touch_event,
        .compute_scroll = egui_view_group_compute_scroll,
        .calculate_layout = egui_view_group_calculate_layout,
        .request_layout = egui_view_group_request_layout,
        .draw = egui_view_group_draw,
        .on_attach_to_window = egui_view_group_on_attach_to_window,
        .on_draw = egui_view_block_ui_container_on_draw,
        .on_detach_from_window = egui_view_group_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_group_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

void egui_view_block_ui_container_init(egui_view_t *self)
{
    egui_view_block_ui_container_t *local = egui_view_block_ui_container_local(self);

    egui_view_group_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_block_ui_container_t);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 4);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->child = NULL;
    local->text_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_block_ui_container_copy_text(local->leading_text, sizeof(local->leading_text), "Before block");
    egui_view_block_ui_container_copy_text(local->trailing_text, sizeof(local->trailing_text), "After block");
    local->compact_mode = 0;
    local->read_only_mode = 0;
    egui_view_block_ui_container_apply_standard_style(self);
    egui_view_set_view_name(self, "egui_view_block_ui_container");
}
