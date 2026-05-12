#include <string.h>

#include "egui.h"
#include "egui_view_search_box.h"
#include "../../hcw_text_center.h"

#include "../../../../sdk/EmbeddedGUI/src/resource/egui_icon_material_symbols.h"
#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_icon_font.h"

#define HCW_SEARCH_BOX_STANDARD_RADIUS 10
#define HCW_SEARCH_BOX_COMPACT_RADIUS  8
#define HCW_SEARCH_BOX_CURSOR_WIDTH    1

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_search_box_standard_bg_normal_param, HCW_COLOR_SURFACE, EGUI_ALPHA_100,
                                                        HCW_SEARCH_BOX_STANDARD_RADIUS, 1, HCW_COLOR_BORDER_STRONG, EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_search_box_standard_bg_pressed_param, HCW_COLOR_SURFACE_PRESS, EGUI_ALPHA_100,
                                                        HCW_SEARCH_BOX_STANDARD_RADIUS, 1, HCW_COLOR_PRIMARY_DARK, EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_search_box_standard_bg_disabled_param, HCW_COLOR_SURFACE_DISABLED, EGUI_ALPHA_100,
                                                        HCW_SEARCH_BOX_STANDARD_RADIUS, 1, HCW_COLOR_BORDER_STRONG, EGUI_ALPHA_100);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_search_box_standard_bg_focused_param, HCW_COLOR_SURFACE, EGUI_ALPHA_100,
                                                        HCW_SEARCH_BOX_STANDARD_RADIUS, 2, HCW_COLOR_PRIMARY_DARK, EGUI_ALPHA_100);
EGUI_BACKGROUND_PARAM_INIT_WITH_FOCUS(hcw_search_box_standard_bg_params, &hcw_search_box_standard_bg_normal_param,
                                      &hcw_search_box_standard_bg_pressed_param, &hcw_search_box_standard_bg_disabled_param,
                                      &hcw_search_box_standard_bg_focused_param);
#else
EGUI_BACKGROUND_PARAM_INIT(hcw_search_box_standard_bg_params, &hcw_search_box_standard_bg_normal_param, &hcw_search_box_standard_bg_pressed_param,
                           &hcw_search_box_standard_bg_disabled_param);
#endif
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(hcw_search_box_standard_background, &hcw_search_box_standard_bg_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_search_box_compact_bg_normal_param, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_100,
                                                        HCW_SEARCH_BOX_COMPACT_RADIUS, 1, HCW_COLOR_BORDER_STRONG, EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_search_box_compact_bg_pressed_param, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_100,
                                                        HCW_SEARCH_BOX_COMPACT_RADIUS, 1, HCW_COLOR_BORDER_STRONG, EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_search_box_compact_bg_disabled_param, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_100,
                                                        HCW_SEARCH_BOX_COMPACT_RADIUS, 1, HCW_COLOR_BORDER_STRONG, EGUI_ALPHA_100);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_search_box_compact_bg_focused_param, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_100,
                                                        HCW_SEARCH_BOX_COMPACT_RADIUS, 2, HCW_COLOR_PRIMARY_DARK, EGUI_ALPHA_100);
EGUI_BACKGROUND_PARAM_INIT_WITH_FOCUS(hcw_search_box_compact_bg_params, &hcw_search_box_compact_bg_normal_param, &hcw_search_box_compact_bg_pressed_param,
                                      &hcw_search_box_compact_bg_disabled_param, &hcw_search_box_compact_bg_focused_param);
#else
EGUI_BACKGROUND_PARAM_INIT(hcw_search_box_compact_bg_params, &hcw_search_box_compact_bg_normal_param, &hcw_search_box_compact_bg_pressed_param,
                           &hcw_search_box_compact_bg_disabled_param);
#endif
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(hcw_search_box_compact_background, &hcw_search_box_compact_bg_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_search_box_read_only_bg_normal_param, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_100,
                                                        HCW_SEARCH_BOX_COMPACT_RADIUS, 1, HCW_COLOR_BORDER_STRONG, EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_search_box_read_only_bg_pressed_param, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_100,
                                                        HCW_SEARCH_BOX_COMPACT_RADIUS, 1, HCW_COLOR_BORDER_STRONG, EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_search_box_read_only_bg_disabled_param, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_100,
                                                        HCW_SEARCH_BOX_COMPACT_RADIUS, 1, HCW_COLOR_BORDER_STRONG, EGUI_ALPHA_100);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_search_box_read_only_bg_focused_param, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_100,
                                                        HCW_SEARCH_BOX_COMPACT_RADIUS, 1, HCW_COLOR_BORDER_STRONG, EGUI_ALPHA_100);
EGUI_BACKGROUND_PARAM_INIT_WITH_FOCUS(hcw_search_box_read_only_bg_params, &hcw_search_box_read_only_bg_normal_param,
                                      &hcw_search_box_read_only_bg_pressed_param, &hcw_search_box_read_only_bg_disabled_param,
                                      &hcw_search_box_read_only_bg_focused_param);
#else
EGUI_BACKGROUND_PARAM_INIT(hcw_search_box_read_only_bg_params, &hcw_search_box_read_only_bg_normal_param,
                           &hcw_search_box_read_only_bg_pressed_param, &hcw_search_box_read_only_bg_disabled_param);
#endif
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(hcw_search_box_read_only_background, &hcw_search_box_read_only_bg_params);

static egui_dim_t hcw_search_box_get_effective_pad_left(egui_view_t *self)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    return self->padding.left;
#else
    egui_view_search_box_t *local = (egui_view_search_box_t *)self;
    return local->content_pad_left;
#endif
}

static egui_dim_t hcw_search_box_get_effective_pad_right(egui_view_t *self)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    return self->padding.right;
#else
    egui_view_search_box_t *local = (egui_view_search_box_t *)self;
    return local->content_pad_right;
#endif
}

static void hcw_search_box_get_text_region(egui_view_t *self, egui_region_t *region)
{
    egui_view_get_work_region(self, region);

#if !EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    {
        egui_view_search_box_t *local = (egui_view_search_box_t *)self;
        egui_dim_t pad_left = local->content_pad_left;
        egui_dim_t pad_right = local->content_pad_right;
        egui_dim_t pad_y = local->content_pad_y;

        if (region->size.width > pad_left + pad_right)
        {
            region->location.x += pad_left;
            region->size.width -= pad_left + pad_right;
        }
        else
        {
            region->size.width = 0;
        }
        if (region->size.height > pad_y * 2)
        {
            region->location.y += pad_y;
            region->size.height -= pad_y * 2;
        }
        else
        {
            region->size.height = 0;
        }
    }
#endif
}

static egui_dim_t hcw_search_box_get_text_width_to_pos(const egui_view_textinput_t *local, uint8_t pos)
{
    char tmp[EGUI_CONFIG_TEXTINPUT_MAX_LENGTH + 1];
    egui_dim_t width = 0;
    egui_dim_t height = 0;

    if (local->font == NULL || pos == 0)
    {
        return 0;
    }

    if (pos > local->text_len)
    {
        pos = local->text_len;
    }

    memcpy(tmp, local->text, pos);
    tmp[pos] = '\0';
    local->font->api->get_str_size(local->font, tmp, 0, 0, &width, &height);
    return width;
}

static int hcw_search_box_get_cursor_region(egui_view_t *self, egui_view_textinput_t *local, egui_region_t *cursor_region)
{
    egui_region_t work_region;
    egui_dim_t cursor_x;
    egui_dim_t dummy_width = 0;
    egui_dim_t cursor_height = 0;

    if (local->font == NULL || cursor_region == NULL)
    {
        return 0;
    }

    hcw_search_box_get_text_region(self, &work_region);
    cursor_x = work_region.location.x + hcw_search_box_get_text_width_to_pos(local, local->cursor_pos) - local->scroll_offset_x;
    local->font->api->get_str_size(local->font, "A", 0, 0, &dummy_width, &cursor_height);

    cursor_region->location.x = cursor_x;
    cursor_region->location.y = work_region.location.y + (work_region.size.height - cursor_height) / 2;
    cursor_region->size.width = HCW_SEARCH_BOX_CURSOR_WIDTH;
    cursor_region->size.height = cursor_height;
    return !egui_region_is_empty(cursor_region);
}

static void hcw_search_box_local_region_to_screen(egui_view_t *self, const egui_region_t *local_region, egui_region_t *screen_region)
{
    screen_region->location.x = self->region_screen.location.x + local_region->location.x;
    screen_region->location.y = self->region_screen.location.y + local_region->location.y;
    screen_region->size.width = local_region->size.width;
    screen_region->size.height = local_region->size.height;
}

static void hcw_search_box_get_search_icon_region(egui_view_t *self, egui_region_t *region)
{
    egui_dim_t pad_left = hcw_search_box_get_effective_pad_left(self);
    egui_dim_t size = EGUI_MIN(pad_left - 8, self->region.size.height - 10);

    if (size < 12)
    {
        size = 12;
    }

    region->size.width = size;
    region->size.height = size;
    region->location.x = 8;
    region->location.y = (self->region.size.height - size) / 2;
}

static uint8_t hcw_search_box_can_show_clear_button(egui_view_t *self)
{
    egui_view_search_box_t *local = (egui_view_search_box_t *)self;

    return self->is_enable && local->textinput.text_len > 0;
}

static void hcw_search_box_get_clear_region_local(egui_view_t *self, egui_region_t *region)
{
    egui_dim_t pad_right = hcw_search_box_get_effective_pad_right(self);
    egui_dim_t size = EGUI_MIN(pad_right - 8, self->region.size.height - 10);

    if (size < 12)
    {
        size = 12;
    }

    region->size.width = size;
    region->size.height = size;
    region->location.x = self->region.size.width - pad_right + (pad_right - size) / 2;
    region->location.y = (self->region.size.height - size) / 2;
}

static uint8_t hcw_search_box_clear_pressed_state(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_search_box_t);
    egui_region_t region;

    if (!local->clear_pressed)
    {
        return 0;
    }

    local->clear_pressed = 0;
    hcw_search_box_get_clear_region_local(self, &region);
    egui_view_invalidate_region(self, &region);
    return 1;
}

static void hcw_search_box_apply_style(egui_view_t *self, egui_background_t *background, egui_color_t text_color, egui_color_t placeholder_color,
                                       egui_color_t cursor_color, egui_color_t icon_color, egui_color_t clear_fill_color,
                                       egui_color_t clear_fill_pressed_color, egui_color_t clear_icon_color, egui_dim_t pad_left,
                                       egui_dim_t pad_right, egui_dim_t pad_y, int is_enable)
{
    egui_view_search_box_t *local = (egui_view_search_box_t *)self;
    egui_view_textinput_t *input = &local->textinput;

    hcw_search_box_clear_pressed_state(self);
    egui_view_set_shadow(self, NULL);
    egui_view_set_background(self, background);
    egui_view_set_padding(self, pad_left, pad_right, pad_y, pad_y);
    local->content_pad_left = pad_left;
    local->content_pad_right = pad_right;
    local->content_pad_y = pad_y;
    egui_view_textinput_set_text_color(self, text_color, EGUI_ALPHA_100);
    egui_view_textinput_set_placeholder_color(self, placeholder_color, EGUI_ALPHA_100);
    egui_view_textinput_set_cursor_color(self, cursor_color);
    input->align_type = EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER;
    local->icon_color = icon_color;
    local->clear_fill_color = clear_fill_color;
    local->clear_fill_pressed_color = clear_fill_pressed_color;
    local->clear_icon_color = clear_icon_color;
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (!is_enable)
    {
        egui_view_clear_focus(self);
        input->cursor_visible = 0;
    }
#endif
    egui_view_set_enable(self, is_enable);
}

void egui_view_search_box_apply_standard_style(egui_view_t *self)
{
    hcw_search_box_apply_style(self, EGUI_BG_OF(&hcw_search_box_standard_background), HCW_COLOR_TEXT_STRONG, HCW_COLOR_TEXT_SOFT,
                               HCW_COLOR_PRIMARY_DARK, HCW_COLOR_TEXT_SOFT, HCW_COLOR_PANEL, HCW_COLOR_PRIMARY_DARK,
                               HCW_COLOR_TEXT_SOFT, 28, 26, 8, 1);
}

void egui_view_search_box_apply_compact_style(egui_view_t *self)
{
    hcw_search_box_apply_style(self, EGUI_BG_OF(&hcw_search_box_compact_background), HCW_COLOR_TEXT_STRONG, HCW_COLOR_TEXT_SOFT,
                               HCW_COLOR_PRIMARY_DARK, HCW_COLOR_TEXT_SOFT, HCW_COLOR_PANEL, HCW_COLOR_PRIMARY_DARK,
                               HCW_COLOR_TEXT_SOFT, 24, 22, 6, 1);
}

void egui_view_search_box_apply_read_only_style(egui_view_t *self)
{
    hcw_search_box_apply_style(self, EGUI_BG_OF(&hcw_search_box_read_only_background), HCW_COLOR_TEXT_STRONG, HCW_COLOR_TEXT_SOFT,
                               HCW_COLOR_TEXT_SOFT, HCW_COLOR_TEXT, HCW_COLOR_SURFACE_DISABLED, HCW_COLOR_SURFACE_DISABLED,
                               HCW_COLOR_TEXT, 24, 18, 6, 0);
}

void egui_view_search_box_set_text(egui_view_t *self, const char *text)
{
    hcw_search_box_clear_pressed_state(self);
    egui_view_textinput_set_text(self, text);
}

void egui_view_search_box_set_placeholder(egui_view_t *self, const char *placeholder)
{
    hcw_search_box_clear_pressed_state(self);
    egui_view_textinput_set_placeholder(self, placeholder);
}

void egui_view_search_box_set_font(egui_view_t *self, const egui_font_t *font)
{
    hcw_search_box_clear_pressed_state(self);
    egui_view_textinput_set_font(self, font);
}

void egui_view_search_box_set_icon_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_search_box_t);

    if (local->icon_font == font)
    {
        return;
    }

    hcw_search_box_clear_pressed_state(self);
    local->icon_font = font;
    egui_view_invalidate(self);
}

void egui_view_search_box_set_max_length(egui_view_t *self, uint8_t max_length)
{
    hcw_search_box_clear_pressed_state(self);
    egui_view_textinput_set_max_length(self, max_length);
}

uint8_t egui_view_search_box_get_clear_region(egui_view_t *self, egui_region_t *region)
{
    egui_region_t local_region;

    if (region == NULL || !hcw_search_box_can_show_clear_button(self))
    {
        return 0;
    }

    hcw_search_box_get_clear_region_local(self, &local_region);
    hcw_search_box_local_region_to_screen(self, &local_region, region);
    return 1;
}

static void hcw_search_box_draw_icon(egui_view_t *self, const egui_region_t *region, const char *icon, egui_color_t color)
{
    EGUI_LOCAL_INIT(egui_view_search_box_t);
    egui_region_t screen_region;
    egui_region_t draw_region;
    const egui_font_t *icon_font;

    if (!EGUI_VIEW_ICON_TEXT_VALID(icon))
    {
        return;
    }

    hcw_search_box_local_region_to_screen(self, region, &screen_region);
    if (!egui_canvas_is_region_active(&uicode_get_core()->canvas, &screen_region))
    {
        return;
    }

    icon_font = EGUI_VIEW_ICON_FONT_RESOLVE(local->icon_font, EGUI_MIN(region->size.width, region->size.height), 18, 20);
    if (icon_font == NULL)
    {
        return;
    }

    draw_region = *region;
    draw_region.location.y += hcw_text_center_get_delta(icon_font, icon, region, EGUI_ALIGN_CENTER);
    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, icon_font, icon, &draw_region, EGUI_ALIGN_CENTER, color, EGUI_ALPHA_100);
}

static void hcw_search_box_on_draw(egui_view_t *self)
{
    egui_view_search_box_t *local = (egui_view_search_box_t *)self;
    egui_view_textinput_t *input = &local->textinput;
    egui_region_t work_region;
    egui_region_t text_screen_region;
    egui_region_t search_icon_region;
    egui_region_t clear_region;
    egui_dim_t text_width = 0;
    egui_dim_t text_height = 0;
    egui_dim_t text_x;
    egui_dim_t text_y;

    if (input->font == NULL)
    {
        return;
    }

    hcw_search_box_get_search_icon_region(self, &search_icon_region);
    hcw_search_box_draw_icon(self, &search_icon_region, EGUI_ICON_MS_SEARCH, local->icon_color);

    if (hcw_search_box_can_show_clear_button(self))
    {
        hcw_search_box_get_clear_region_local(self, &clear_region);
        {
            egui_dim_t radius = EGUI_MAX(EGUI_MIN(clear_region.size.width, clear_region.size.height) / 2 - 1, 4);
            egui_color_t fill_color = local->clear_pressed ? local->clear_fill_pressed_color : local->clear_fill_color;
            egui_alpha_t fill_alpha = local->clear_pressed ? 78 : 52;

            egui_canvas_draw_circle_fill_basic(&uicode_get_core()->canvas, clear_region.location.x + clear_region.size.width / 2, clear_region.location.y + clear_region.size.height / 2,
                                               radius, fill_color, fill_alpha);
        }
        hcw_search_box_draw_icon(self, &clear_region, EGUI_ICON_MS_CLOSE, local->clear_icon_color);
    }

    hcw_search_box_get_text_region(self, &work_region);
    hcw_search_box_local_region_to_screen(self, &work_region, &text_screen_region);
    if (!egui_canvas_is_region_active(&uicode_get_core()->canvas, &text_screen_region))
    {
        return;
    }

    if (input->text_len == 0 && !self->is_focused && input->placeholder != NULL)
    {
        egui_region_t placeholder_region = work_region;

        placeholder_region.location.y += hcw_text_center_get_delta(input->font, input->placeholder, &work_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
        egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, input->font, input->placeholder, &placeholder_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, input->placeholder_color,
                                      input->placeholder_alpha);
    }
    else if (input->text_len > 0)
    {
        input->font->api->get_str_size(input->font, input->text, 0, 0, &text_width, &text_height);
        text_x = work_region.location.x - input->scroll_offset_x;
        text_y = work_region.location.y + (work_region.size.height - text_height) / 2;
        text_y += hcw_text_center_get_delta(input->font, input->text, &work_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
        egui_canvas_draw_text(&uicode_get_core()->canvas, input->font, input->text, text_x, text_y, input->text_color, input->text_alpha);
    }

    if (self->is_enable && self->is_focused && input->cursor_visible)
    {
        egui_region_t cursor_region;
        egui_region_t cursor_screen_region;

        if (hcw_search_box_get_cursor_region(self, input, &cursor_region))
        {
            hcw_search_box_local_region_to_screen(self, &cursor_region, &cursor_screen_region);
            if (egui_canvas_is_region_active(&uicode_get_core()->canvas, &cursor_screen_region))
            {
                egui_canvas_draw_rectangle_fill(&uicode_get_core()->canvas, cursor_region.location.x, cursor_region.location.y, cursor_region.size.width, cursor_region.size.height,
                                                input->cursor_color, EGUI_ALPHA_100);
            }
        }
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_search_box_on_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_search_box_t);
    egui_region_t clear_region;
    uint8_t inside_clear = 0;
    uint8_t had_pressed = local->clear_pressed;

    if (!hcw_search_box_can_show_clear_button(self))
    {
        return had_pressed ? hcw_search_box_clear_pressed_state(self) : 0;
    }

    hcw_search_box_get_clear_region_local(self, &clear_region);
    inside_clear = egui_region_pt_in_rect(&clear_region, event->location.x - self->region_screen.location.x, event->location.y - self->region_screen.location.y);

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        if (inside_clear)
        {
            local->clear_pressed = 1;
            egui_view_invalidate_region(self, &clear_region);
            return 1;
        }
        break;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (had_pressed != inside_clear)
        {
            local->clear_pressed = inside_clear;
            egui_view_invalidate_region(self, &clear_region);
        }
        return had_pressed ? 1 : 0;
    case EGUI_MOTION_EVENT_ACTION_UP:
        if (had_pressed)
        {
            local->clear_pressed = 0;
            egui_view_invalidate_region(self, &clear_region);
            if (inside_clear)
            {
                egui_view_textinput_clear(self);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
                egui_view_request_focus(self);
#endif
            }
            return 1;
        }
        break;
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (had_pressed)
        {
            local->clear_pressed = 0;
            egui_view_invalidate_region(self, &clear_region);
            return 1;
        }
        break;
    default:
        break;
    }

    return 0;
}

static int hcw_search_box_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_search_box_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_search_box_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    hcw_search_box_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_search_box_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
    api->on_draw = hcw_search_box_on_draw;
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_search_box_on_static_touch_event;
    api->on_touch = NULL;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_search_box_on_static_key_event;
    api->on_key = NULL;
#endif
}

void egui_view_search_box_init(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_search_box_t);

    egui_view_textinput_init(self, uicode_get_core());
    egui_view_copy_api(self, &local->api);
    local->api.on_draw = hcw_search_box_on_draw;
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    local->api.on_touch = hcw_search_box_on_touch;
#endif
    local->icon_font = NULL;
    local->icon_color = HCW_COLOR_TEXT_SOFT;
    local->clear_fill_color = HCW_COLOR_SURFACE_DISABLED;
    local->clear_fill_pressed_color = HCW_COLOR_PRIMARY_DARK;
    local->clear_icon_color = HCW_COLOR_TEXT_SOFT;
    local->content_pad_left = 0;
    local->content_pad_right = 0;
    local->content_pad_y = 0;
    local->clear_pressed = 0;
    egui_view_set_view_name(self, "egui_view_search_box");
}
