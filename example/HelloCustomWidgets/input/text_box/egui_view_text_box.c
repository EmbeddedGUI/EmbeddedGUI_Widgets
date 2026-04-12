#include <string.h>

#include "egui.h"
#include "egui_view_text_box.h"

#define HCW_TEXT_BOX_STANDARD_RADIUS 10
#define HCW_TEXT_BOX_COMPACT_RADIUS  8
#define HCW_TEXT_BOX_CURSOR_WIDTH    1

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_text_box_standard_bg_normal_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100,
                                                        HCW_TEXT_BOX_STANDARD_RADIUS, 1, EGUI_COLOR_HEX(0xD5DCE4), EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_text_box_standard_bg_pressed_param, EGUI_COLOR_HEX(0xF8FBFE), EGUI_ALPHA_100,
                                                        HCW_TEXT_BOX_STANDARD_RADIUS, 1, EGUI_COLOR_HEX(0xC4D5E7), EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_text_box_standard_bg_disabled_param, EGUI_COLOR_HEX(0xF1F4F7), EGUI_ALPHA_100,
                                                        HCW_TEXT_BOX_STANDARD_RADIUS, 1, EGUI_COLOR_HEX(0xD8E0E7), EGUI_ALPHA_100);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_text_box_standard_bg_focused_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100,
                                                        HCW_TEXT_BOX_STANDARD_RADIUS, 2, EGUI_COLOR_HEX(0x0F6CBD), EGUI_ALPHA_100);
EGUI_BACKGROUND_PARAM_INIT_WITH_FOCUS(hcw_text_box_standard_bg_params, &hcw_text_box_standard_bg_normal_param, &hcw_text_box_standard_bg_pressed_param,
                                      &hcw_text_box_standard_bg_disabled_param, &hcw_text_box_standard_bg_focused_param);
#else
EGUI_BACKGROUND_PARAM_INIT(hcw_text_box_standard_bg_params, &hcw_text_box_standard_bg_normal_param, &hcw_text_box_standard_bg_pressed_param,
                           &hcw_text_box_standard_bg_disabled_param);
#endif
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(hcw_text_box_standard_background, &hcw_text_box_standard_bg_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_text_box_compact_bg_normal_param, EGUI_COLOR_HEX(0xF7FBFB), EGUI_ALPHA_100,
                                                        HCW_TEXT_BOX_COMPACT_RADIUS, 1, EGUI_COLOR_HEX(0xC9D9D7), EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_text_box_compact_bg_pressed_param, EGUI_COLOR_HEX(0xEEF7F5), EGUI_ALPHA_100,
                                                        HCW_TEXT_BOX_COMPACT_RADIUS, 1, EGUI_COLOR_HEX(0xBDD0CD), EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_text_box_compact_bg_disabled_param, EGUI_COLOR_HEX(0xEDF4F3), EGUI_ALPHA_100,
                                                        HCW_TEXT_BOX_COMPACT_RADIUS, 1, EGUI_COLOR_HEX(0xD4DFDE), EGUI_ALPHA_100);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_text_box_compact_bg_focused_param, EGUI_COLOR_HEX(0xF7FBFB), EGUI_ALPHA_100,
                                                        HCW_TEXT_BOX_COMPACT_RADIUS, 2, EGUI_COLOR_HEX(0x0C7C73), EGUI_ALPHA_100);
EGUI_BACKGROUND_PARAM_INIT_WITH_FOCUS(hcw_text_box_compact_bg_params, &hcw_text_box_compact_bg_normal_param, &hcw_text_box_compact_bg_pressed_param,
                                      &hcw_text_box_compact_bg_disabled_param, &hcw_text_box_compact_bg_focused_param);
#else
EGUI_BACKGROUND_PARAM_INIT(hcw_text_box_compact_bg_params, &hcw_text_box_compact_bg_normal_param, &hcw_text_box_compact_bg_pressed_param,
                           &hcw_text_box_compact_bg_disabled_param);
#endif
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(hcw_text_box_compact_background, &hcw_text_box_compact_bg_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_text_box_read_only_bg_normal_param, EGUI_COLOR_HEX(0xF5F7FA), EGUI_ALPHA_100,
                                                        HCW_TEXT_BOX_COMPACT_RADIUS, 1, EGUI_COLOR_HEX(0xD7DEE6), EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_text_box_read_only_bg_pressed_param, EGUI_COLOR_HEX(0xF5F7FA), EGUI_ALPHA_100,
                                                        HCW_TEXT_BOX_COMPACT_RADIUS, 1, EGUI_COLOR_HEX(0xD7DEE6), EGUI_ALPHA_100);
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_text_box_read_only_bg_disabled_param, EGUI_COLOR_HEX(0xF5F7FA), EGUI_ALPHA_100,
                                                        HCW_TEXT_BOX_COMPACT_RADIUS, 1, EGUI_COLOR_HEX(0xD7DEE6), EGUI_ALPHA_100);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE_STROKE(hcw_text_box_read_only_bg_focused_param, EGUI_COLOR_HEX(0xF5F7FA), EGUI_ALPHA_100,
                                                        HCW_TEXT_BOX_COMPACT_RADIUS, 1, EGUI_COLOR_HEX(0xD7DEE6), EGUI_ALPHA_100);
EGUI_BACKGROUND_PARAM_INIT_WITH_FOCUS(hcw_text_box_read_only_bg_params, &hcw_text_box_read_only_bg_normal_param, &hcw_text_box_read_only_bg_pressed_param,
                                      &hcw_text_box_read_only_bg_disabled_param, &hcw_text_box_read_only_bg_focused_param);
#else
EGUI_BACKGROUND_PARAM_INIT(hcw_text_box_read_only_bg_params, &hcw_text_box_read_only_bg_normal_param, &hcw_text_box_read_only_bg_pressed_param,
                           &hcw_text_box_read_only_bg_disabled_param);
#endif
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(hcw_text_box_read_only_background, &hcw_text_box_read_only_bg_params);

static egui_dim_t hcw_text_box_get_text_width_to_pos(const egui_view_textinput_t *local, uint8_t pos)
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

static int hcw_text_box_get_cursor_region(egui_view_t *self, egui_view_textinput_t *local, egui_region_t *cursor_region)
{
    egui_region_t work_region;
    egui_dim_t cursor_x;
    egui_dim_t dummy_width = 0;
    egui_dim_t cursor_height = 0;

    if (local->font == NULL || cursor_region == NULL)
    {
        return 0;
    }

    egui_view_get_work_region(self, &work_region);
    cursor_x = work_region.location.x + hcw_text_box_get_text_width_to_pos(local, local->cursor_pos) - local->scroll_offset_x;
    local->font->api->get_str_size(local->font, "A", 0, 0, &dummy_width, &cursor_height);

    cursor_region->location.x = cursor_x;
    cursor_region->location.y = work_region.location.y + (work_region.size.height - cursor_height) / 2;
    cursor_region->size.width = HCW_TEXT_BOX_CURSOR_WIDTH;
    cursor_region->size.height = cursor_height;
    return !egui_region_is_empty(cursor_region);
}

static void hcw_text_box_local_region_to_screen(egui_view_t *self, const egui_region_t *local_region, egui_region_t *screen_region)
{
    screen_region->location.x = self->region_screen.location.x + local_region->location.x;
    screen_region->location.y = self->region_screen.location.y + local_region->location.y;
    screen_region->size.width = local_region->size.width;
    screen_region->size.height = local_region->size.height;
}

static void hcw_text_box_apply_style(egui_view_t *self, egui_background_t *background, egui_color_t text_color, egui_color_t placeholder_color,
                                     egui_color_t cursor_color, egui_dim_t pad_x, egui_dim_t pad_y, int is_enable)
{
    egui_view_textinput_t *local = (egui_view_textinput_t *)self;

    egui_view_set_shadow(self, NULL);
    egui_view_set_background(self, background);
    egui_view_set_padding(self, pad_x, pad_x, pad_y, pad_y);
    egui_view_textinput_set_text_color(self, text_color, EGUI_ALPHA_100);
    egui_view_textinput_set_placeholder_color(self, placeholder_color, EGUI_ALPHA_100);
    egui_view_textinput_set_cursor_color(self, cursor_color);
    local->align_type = EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER;
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (!is_enable)
    {
        egui_view_clear_focus(self);
        local->cursor_visible = 0;
    }
#endif
    egui_view_set_enable(self, is_enable);
}

void hcw_text_box_apply_standard_style(egui_view_t *self)
{
    hcw_text_box_apply_style(self, EGUI_BG_OF(&hcw_text_box_standard_background), EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89),
                             EGUI_COLOR_HEX(0x0F6CBD), 10, 8, 1);
}

void hcw_text_box_apply_compact_style(egui_view_t *self)
{
    hcw_text_box_apply_style(self, EGUI_BG_OF(&hcw_text_box_compact_background), EGUI_COLOR_HEX(0x183235), EGUI_COLOR_HEX(0x66817E),
                             EGUI_COLOR_HEX(0x0C7C73), 8, 6, 1);
}

void hcw_text_box_apply_read_only_style(egui_view_t *self)
{
    hcw_text_box_apply_style(self, EGUI_BG_OF(&hcw_text_box_read_only_background), EGUI_COLOR_HEX(0x7A8796), EGUI_COLOR_HEX(0x95A1AE),
                             EGUI_COLOR_HEX(0x7A8796), 8, 6, 0);
}

void hcw_text_box_set_text(egui_view_t *self, const char *text)
{
    egui_view_textinput_set_text(self, text);
}

void hcw_text_box_set_placeholder(egui_view_t *self, const char *placeholder)
{
    egui_view_textinput_set_placeholder(self, placeholder);
}

void hcw_text_box_set_font(egui_view_t *self, const egui_font_t *font)
{
    egui_view_textinput_set_font(self, font);
}

void hcw_text_box_set_max_length(egui_view_t *self, uint8_t max_length)
{
    egui_view_textinput_set_max_length(self, max_length);
}

static void hcw_text_box_on_draw(egui_view_t *self)
{
    egui_view_textinput_t *local = (egui_view_textinput_t *)self;
    egui_region_t work_region;
    egui_region_t text_screen_region;
    egui_dim_t text_width = 0;
    egui_dim_t text_height = 0;
    egui_dim_t text_x;
    egui_dim_t text_y;

    if (local->font == NULL)
    {
        return;
    }

    egui_view_get_work_region(self, &work_region);
    hcw_text_box_local_region_to_screen(self, &work_region, &text_screen_region);

    if (!egui_canvas_is_region_active(&text_screen_region))
    {
        return;
    }

    if (local->text_len == 0 && !self->is_focused && local->placeholder != NULL)
    {
        egui_canvas_draw_text_in_rect(local->font, local->placeholder, &work_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, local->placeholder_color,
                                      local->placeholder_alpha);
    }
    else if (local->text_len > 0)
    {
        local->font->api->get_str_size(local->font, local->text, 0, 0, &text_width, &text_height);
        text_x = work_region.location.x - local->scroll_offset_x;
        text_y = work_region.location.y + (work_region.size.height - text_height) / 2;
        egui_canvas_draw_text(local->font, local->text, text_x, text_y, local->text_color, local->text_alpha);
    }

    if (self->is_enable && self->is_focused && local->cursor_visible)
    {
        egui_region_t cursor_region;
        egui_region_t cursor_screen_region;

        if (hcw_text_box_get_cursor_region(self, local, &cursor_region))
        {
            hcw_text_box_local_region_to_screen(self, &cursor_region, &cursor_screen_region);
            if (egui_canvas_is_region_active(&cursor_screen_region))
            {
                egui_canvas_draw_rectangle_fill(cursor_region.location.x, cursor_region.location.y, cursor_region.size.width, cursor_region.size.height,
                                                local->cursor_color, EGUI_ALPHA_100);
            }
        }
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int hcw_text_box_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);
    EGUI_UNUSED(event);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int hcw_text_box_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(self);
    EGUI_UNUSED(event);
    return 1;
}
#endif

void hcw_text_box_override_interaction_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
    api->on_draw = hcw_text_box_on_draw;
}

void hcw_text_box_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
    api->on_draw = hcw_text_box_on_draw;
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = hcw_text_box_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = hcw_text_box_on_static_key_event;
#endif
}
