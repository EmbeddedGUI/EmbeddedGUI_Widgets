#include "egui_view_inline_ui_container.h"
#include "../../hcw_text_center.h"

#include <string.h>

#define EGUI_VIEW_INLINE_UI_CONTAINER_TEXT_SLOT_MIN 18
#define EGUI_VIEW_INLINE_UI_CONTAINER_TEXT_SLOT_MAX 76
#define EGUI_VIEW_INLINE_UI_CONTAINER_GAP_MAX       14
#define EGUI_VIEW_INLINE_UI_CONTAINER_BASELINE_MIN  (-8)
#define EGUI_VIEW_INLINE_UI_CONTAINER_BASELINE_MAX  8
#define EGUI_VIEW_INLINE_UI_CONTAINER_RADIUS_MAX    14
#define EGUI_VIEW_INLINE_UI_CONTAINER_HOST_PAD_X    6
#define EGUI_VIEW_INLINE_UI_CONTAINER_HOST_PAD_Y    3
#define EGUI_VIEW_INLINE_UI_CONTAINER_HOST_MIN_W    42
#define EGUI_VIEW_INLINE_UI_CONTAINER_HOST_MIN_H    18
#define EGUI_VIEW_INLINE_UI_CONTAINER_TEXT_INSET_X  6

static egui_view_inline_ui_container_t *egui_view_inline_ui_container_local(egui_view_t *self)
{
    return (egui_view_inline_ui_container_t *)self;
}

static uint8_t egui_view_inline_ui_container_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static uint8_t egui_view_inline_ui_container_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static void egui_view_inline_ui_container_copy_text(char *dst, uint8_t capacity, const char *src)
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

static egui_dim_t egui_view_inline_ui_container_clamp_dim(egui_dim_t value, egui_dim_t min_value, egui_dim_t max_value)
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

static egui_color_t egui_view_inline_ui_container_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_MAKE(38));
}

static egui_dim_t egui_view_inline_ui_container_child_width(egui_view_inline_ui_container_t *local)
{
    if (local->child == NULL || local->child->region.size.width <= 0)
    {
        return EGUI_VIEW_INLINE_UI_CONTAINER_HOST_MIN_W;
    }
    return local->child->region.size.width + EGUI_VIEW_INLINE_UI_CONTAINER_HOST_PAD_X * 2;
}

static egui_dim_t egui_view_inline_ui_container_child_height(egui_view_inline_ui_container_t *local)
{
    if (local->child == NULL || local->child->region.size.height <= 0)
    {
        return EGUI_VIEW_INLINE_UI_CONTAINER_HOST_MIN_H;
    }
    return local->child->region.size.height + EGUI_VIEW_INLINE_UI_CONTAINER_HOST_PAD_Y * 2;
}

static void egui_view_inline_ui_container_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region,
                                                   uint8_t align, egui_color_t color, egui_alpha_t alpha)
{
    egui_region_t draw_region = *region;

    if (font == NULL || !egui_view_inline_ui_container_has_text(text) || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    if (draw_region.size.width > EGUI_VIEW_INLINE_UI_CONTAINER_TEXT_INSET_X * 2)
    {
        draw_region.location.x += EGUI_VIEW_INLINE_UI_CONTAINER_TEXT_INSET_X;
        draw_region.size.width -= EGUI_VIEW_INLINE_UI_CONTAINER_TEXT_INSET_X * 2;
    }
    draw_region.location.y += hcw_text_center_get_delta(font, text, &draw_region, align);
    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, align, color, egui_color_alpha_mix(self->alpha, alpha));
}

void egui_view_inline_ui_container_get_regions(egui_view_t *self, egui_region_t *prefix_region, egui_region_t *host_region,
                                               egui_region_t *suffix_region)
{
    egui_view_inline_ui_container_t *local = egui_view_inline_ui_container_local(self);
    egui_region_t work_region;
    egui_dim_t gap = local->inline_gap;
    egui_dim_t prefix_width = local->text_slot_width;
    egui_dim_t host_width = egui_view_inline_ui_container_child_width(local);
    egui_dim_t host_height = egui_view_inline_ui_container_child_height(local);
    egui_dim_t available_width;
    egui_dim_t host_x;
    egui_dim_t host_y;
    egui_dim_t suffix_x;

    egui_view_get_work_region(self, &work_region);
    if (work_region.size.width < 0)
    {
        work_region.size.width = 0;
    }
    if (work_region.size.height < 0)
    {
        work_region.size.height = 0;
    }

    if (prefix_width > work_region.size.width)
    {
        prefix_width = work_region.size.width;
    }
    available_width = work_region.size.width - prefix_width;
    if (available_width < 0)
    {
        available_width = 0;
    }
    if (gap * 2 > available_width)
    {
        gap = available_width / 2;
    }
    if (host_width + gap * 2 > available_width)
    {
        host_width = available_width - gap * 2;
        if (host_width < 0)
        {
            host_width = 0;
        }
    }
    if (host_height > work_region.size.height)
    {
        host_height = work_region.size.height;
    }

    host_x = work_region.location.x + prefix_width + gap;
    host_y = work_region.location.y + (work_region.size.height - host_height) / 2 + local->baseline_offset;
    if (host_y < work_region.location.y)
    {
        host_y = work_region.location.y;
    }
    if (host_y + host_height > work_region.location.y + work_region.size.height)
    {
        host_y = work_region.location.y + work_region.size.height - host_height;
    }
    suffix_x = host_x + host_width + gap;

    if (prefix_region != NULL)
    {
        prefix_region->location.x = work_region.location.x;
        prefix_region->location.y = work_region.location.y;
        prefix_region->size.width = prefix_width;
        prefix_region->size.height = work_region.size.height;
    }
    if (host_region != NULL)
    {
        host_region->location.x = host_x;
        host_region->location.y = host_y;
        host_region->size.width = host_width;
        host_region->size.height = host_height;
    }
    if (suffix_region != NULL)
    {
        suffix_region->location.x = suffix_x;
        suffix_region->location.y = work_region.location.y;
        suffix_region->size.width = work_region.location.x + work_region.size.width - suffix_x;
        suffix_region->size.height = work_region.size.height;
        if (suffix_region->size.width < 0)
        {
            suffix_region->size.width = 0;
        }
    }
}

static void egui_view_inline_ui_container_on_draw(egui_view_t *self)
{
    egui_view_inline_ui_container_t *local = egui_view_inline_ui_container_local(self);
    egui_region_t work_region;
    egui_region_t prefix_region;
    egui_region_t host_region;
    egui_region_t suffix_region;
    egui_color_t surface_color = local->surface_color;
    egui_color_t border_color = local->border_color;
    egui_color_t text_color = local->text_color;
    egui_color_t host_surface_color = local->host_surface_color;
    egui_color_t host_border_color = local->host_border_color;
    egui_dim_t radius = local->corner_radius;
    egui_alpha_t text_alpha = EGUI_ALPHA_100;
    egui_alpha_t host_alpha = EGUI_ALPHA_MAKE(local->compact_mode ? 82 : 94);

    egui_view_get_work_region(self, &work_region);
    if (work_region.size.width <= 0 || work_region.size.height <= 0)
    {
        return;
    }

    if (local->read_only_mode)
    {
        surface_color = egui_rgb_mix(surface_color, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(34));
        border_color = egui_rgb_mix(border_color, HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_MAKE(34));
        text_color = egui_rgb_mix(text_color, HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_MAKE(30));
        host_surface_color = egui_rgb_mix(host_surface_color, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(32));
        host_border_color = egui_rgb_mix(host_border_color, HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_MAKE(36));
        text_alpha = EGUI_ALPHA_MAKE(92);
        host_alpha = EGUI_ALPHA_MAKE(76);
    }
    if (!egui_view_get_enable(self))
    {
        surface_color = egui_view_inline_ui_container_mix_disabled(surface_color);
        border_color = egui_view_inline_ui_container_mix_disabled(border_color);
        text_color = egui_view_inline_ui_container_mix_disabled(text_color);
        host_surface_color = egui_view_inline_ui_container_mix_disabled(host_surface_color);
        host_border_color = egui_view_inline_ui_container_mix_disabled(host_border_color);
        text_alpha = EGUI_ALPHA_MAKE(78);
        host_alpha = EGUI_ALPHA_MAKE(64);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, work_region.location.x, work_region.location.y, work_region.size.width,
                                          work_region.size.height, radius + 2, surface_color,
                                          egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->compact_mode ? 68 : 88)));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, work_region.location.x, work_region.location.y, work_region.size.width,
                                     work_region.size.height, radius + 2, 1, border_color,
                                     egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(local->compact_mode ? 28 : 38)));

    egui_view_inline_ui_container_get_regions(self, &prefix_region, &host_region, &suffix_region);
    egui_view_inline_ui_container_draw_text(local->text_font, self, local->prefix_text, &prefix_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text_color,
                                           text_alpha);
    egui_view_inline_ui_container_draw_text(local->text_font, self, local->suffix_text, &suffix_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, text_color,
                                           text_alpha);

    if (host_region.size.width > 0 && host_region.size.height > 0)
    {
        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, host_region.location.x, host_region.location.y, host_region.size.width,
                                              host_region.size.height, radius, host_surface_color, egui_color_alpha_mix(self->alpha, host_alpha));
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, host_region.location.x, host_region.location.y, host_region.size.width,
                                         host_region.size.height, radius, 1, host_border_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(74)));
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_inline_ui_container_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_inline_ui_container_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_inline_ui_container_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_inline_ui_container_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_inline_ui_container_set_child(egui_view_t *self, egui_view_t *child)
{
    egui_view_inline_ui_container_t *local = egui_view_inline_ui_container_local(self);

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
        egui_view_inline_ui_container_layout_child(self);
    }
    egui_view_invalidate(self);
}

egui_view_t *egui_view_inline_ui_container_get_child(egui_view_t *self)
{
    return egui_view_inline_ui_container_local(self)->child;
}

void egui_view_inline_ui_container_layout_child(egui_view_t *self)
{
    egui_view_inline_ui_container_t *local = egui_view_inline_ui_container_local(self);
    egui_region_t host_region;
    egui_region_t child_region;

    if (local->child == NULL)
    {
        return;
    }

    egui_view_inline_ui_container_get_regions(self, NULL, &host_region, NULL);
    child_region.size.width = local->child->region.size.width;
    child_region.size.height = local->child->region.size.height;
    child_region.location.x = host_region.location.x + (host_region.size.width - child_region.size.width) / 2;
    child_region.location.y = host_region.location.y + (host_region.size.height - child_region.size.height) / 2;
    if (child_region.location.x < host_region.location.x)
    {
        child_region.location.x = host_region.location.x;
    }
    if (child_region.location.y < host_region.location.y)
    {
        child_region.location.y = host_region.location.y;
    }
    egui_view_layout(local->child, &child_region);
}

void egui_view_inline_ui_container_set_text(egui_view_t *self, const char *prefix_text, const char *suffix_text)
{
    egui_view_inline_ui_container_t *local = egui_view_inline_ui_container_local(self);

    egui_view_inline_ui_container_clear_pressed_state(self);
    egui_view_inline_ui_container_copy_text(local->prefix_text, sizeof(local->prefix_text), prefix_text);
    egui_view_inline_ui_container_copy_text(local->suffix_text, sizeof(local->suffix_text), suffix_text);
    egui_view_invalidate(self);
}

const char *egui_view_inline_ui_container_get_prefix_text(egui_view_t *self)
{
    return egui_view_inline_ui_container_local(self)->prefix_text;
}

const char *egui_view_inline_ui_container_get_suffix_text(egui_view_t *self)
{
    return egui_view_inline_ui_container_local(self)->suffix_text;
}

void egui_view_inline_ui_container_set_font(egui_view_t *self, const egui_font_t *font)
{
    egui_view_inline_ui_container_t *local = egui_view_inline_ui_container_local(self);

    egui_view_inline_ui_container_clear_pressed_state(self);
    local->text_font = font == NULL ? (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT : font;
    egui_view_invalidate(self);
}

void egui_view_inline_ui_container_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color,
                                               egui_color_t text_color, egui_color_t host_surface_color,
                                               egui_color_t host_border_color, egui_color_t accent_color)
{
    egui_view_inline_ui_container_t *local = egui_view_inline_ui_container_local(self);

    egui_view_inline_ui_container_clear_pressed_state(self);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->host_surface_color = host_surface_color;
    local->host_border_color = host_border_color;
    local->accent_color = accent_color;
    egui_view_invalidate(self);
}

void egui_view_inline_ui_container_set_metrics(egui_view_t *self, egui_dim_t text_slot_width, egui_dim_t inline_gap,
                                               egui_dim_t baseline_offset, egui_dim_t corner_radius)
{
    egui_view_inline_ui_container_t *local = egui_view_inline_ui_container_local(self);

    egui_view_inline_ui_container_clear_pressed_state(self);
    local->text_slot_width = egui_view_inline_ui_container_clamp_dim(text_slot_width, EGUI_VIEW_INLINE_UI_CONTAINER_TEXT_SLOT_MIN,
                                                                     EGUI_VIEW_INLINE_UI_CONTAINER_TEXT_SLOT_MAX);
    local->inline_gap = egui_view_inline_ui_container_clamp_dim(inline_gap, 0, EGUI_VIEW_INLINE_UI_CONTAINER_GAP_MAX);
    local->baseline_offset = egui_view_inline_ui_container_clamp_dim(baseline_offset, EGUI_VIEW_INLINE_UI_CONTAINER_BASELINE_MIN,
                                                                     EGUI_VIEW_INLINE_UI_CONTAINER_BASELINE_MAX);
    local->corner_radius = egui_view_inline_ui_container_clamp_dim(corner_radius, 0, EGUI_VIEW_INLINE_UI_CONTAINER_RADIUS_MAX);
    egui_view_inline_ui_container_layout_child(self);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_inline_ui_container_get_text_slot_width(egui_view_t *self)
{
    return egui_view_inline_ui_container_local(self)->text_slot_width;
}

egui_dim_t egui_view_inline_ui_container_get_inline_gap(egui_view_t *self)
{
    return egui_view_inline_ui_container_local(self)->inline_gap;
}

egui_dim_t egui_view_inline_ui_container_get_baseline_offset(egui_view_t *self)
{
    return egui_view_inline_ui_container_local(self)->baseline_offset;
}

egui_dim_t egui_view_inline_ui_container_get_corner_radius(egui_view_t *self)
{
    return egui_view_inline_ui_container_local(self)->corner_radius;
}

void egui_view_inline_ui_container_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    egui_view_inline_ui_container_t *local = egui_view_inline_ui_container_local(self);

    egui_view_inline_ui_container_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_inline_ui_container_get_compact_mode(egui_view_t *self)
{
    return egui_view_inline_ui_container_local(self)->compact_mode;
}

void egui_view_inline_ui_container_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    egui_view_inline_ui_container_t *local = egui_view_inline_ui_container_local(self);

    egui_view_inline_ui_container_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_inline_ui_container_get_read_only_mode(egui_view_t *self)
{
    return egui_view_inline_ui_container_local(self)->read_only_mode;
}

void egui_view_inline_ui_container_apply_standard_style(egui_view_t *self)
{
    egui_view_inline_ui_container_set_palette(self, HCW_COLOR_SURFACE, HCW_COLOR_BORDER, HCW_COLOR_TEXT,
                                             HCW_COLOR_SURFACE_SUBTLE, HCW_COLOR_PRIMARY_SOFT, HCW_COLOR_PRIMARY);
    egui_view_inline_ui_container_set_metrics(self, 48, 6, 0, 8);
    egui_view_inline_ui_container_set_compact_mode(self, 0);
    egui_view_inline_ui_container_set_read_only_mode(self, 0);
}

void egui_view_inline_ui_container_apply_accent_style(egui_view_t *self)
{
    egui_view_inline_ui_container_set_palette(self, HCW_COLOR_SURFACE_SUBTLE, HCW_COLOR_PRIMARY_SOFT, HCW_COLOR_TEXT,
                                             HCW_COLOR_PRIMARY_TINT, HCW_COLOR_PRIMARY_LIGHT, HCW_COLOR_PRIMARY);
    egui_view_inline_ui_container_set_metrics(self, 48, 6, -2, 8);
    egui_view_inline_ui_container_set_compact_mode(self, 0);
    egui_view_inline_ui_container_set_read_only_mode(self, 0);
}

void egui_view_inline_ui_container_apply_compact_style(egui_view_t *self)
{
    egui_view_inline_ui_container_set_palette(self, HCW_COLOR_PANEL, HCW_COLOR_BORDER, HCW_COLOR_TEXT,
                                             HCW_COLOR_PRIMARY_TINT, HCW_COLOR_PRIMARY_SOFT, HCW_COLOR_PRIMARY);
    egui_view_inline_ui_container_set_metrics(self, 36, 4, 0, 6);
    egui_view_inline_ui_container_set_compact_mode(self, 1);
    egui_view_inline_ui_container_set_read_only_mode(self, 0);
}

void egui_view_inline_ui_container_apply_read_only_style(egui_view_t *self)
{
    egui_view_inline_ui_container_set_palette(self, HCW_COLOR_SURFACE_SUBTLE, HCW_COLOR_BORDER_STRONG, HCW_COLOR_TEXT_SOFT,
                                             HCW_COLOR_SURFACE_SUBTLE, HCW_COLOR_BORDER_STRONG, HCW_COLOR_TEXT_SOFT);
    egui_view_inline_ui_container_set_metrics(self, 36, 4, 1, 6);
    egui_view_inline_ui_container_set_compact_mode(self, 1);
    egui_view_inline_ui_container_set_read_only_mode(self, 1);
}

void egui_view_inline_ui_container_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_inline_ui_container_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_inline_ui_container_on_static_key_event;
#endif
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_inline_ui_container_t) = {
        .dispatch_touch_event = egui_view_group_dispatch_touch_event,
        .on_touch_event = egui_view_group_on_touch_event,
        .on_intercept_touch_event = egui_view_group_on_intercept_touch_event,
        .compute_scroll = egui_view_group_compute_scroll,
        .calculate_layout = egui_view_group_calculate_layout,
        .request_layout = egui_view_group_request_layout,
        .draw = egui_view_group_draw,
        .on_attach_to_window = egui_view_group_on_attach_to_window,
        .on_draw = egui_view_inline_ui_container_on_draw,
        .on_detach_from_window = egui_view_group_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_group_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

void egui_view_inline_ui_container_init(egui_view_t *self)
{
    egui_view_inline_ui_container_t *local = egui_view_inline_ui_container_local(self);

    egui_view_group_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_inline_ui_container_t);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 3);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->child = NULL;
    local->text_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_inline_ui_container_copy_text(local->prefix_text, sizeof(local->prefix_text), "Before");
    egui_view_inline_ui_container_copy_text(local->suffix_text, sizeof(local->suffix_text), "after");
    local->compact_mode = 0;
    local->read_only_mode = 0;
    egui_view_inline_ui_container_apply_standard_style(self);
    egui_view_set_view_name(self, "egui_view_inline_ui_container");
}
