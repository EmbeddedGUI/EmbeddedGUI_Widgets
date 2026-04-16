#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_scroll_bar.h"

#include "../../HelloCustomWidgets/input/scroll_bar/egui_view_scroll_bar.h"
#include "../../HelloCustomWidgets/input/scroll_bar/egui_view_scroll_bar.c"

typedef struct scroll_bar_preview_snapshot scroll_bar_preview_snapshot_t;
struct scroll_bar_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    egui_view_on_scroll_bar_changed_listener_t on_changed;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    const char *label;
    const char *helper;
    const egui_view_api_t *api;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t preview_color;
    egui_dim_t content_length;
    egui_dim_t viewport_length;
    egui_dim_t offset;
    egui_dim_t line_step;
    egui_dim_t page_step;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t current_part;
    uint8_t pressed_part;
    uint8_t pressed_track_direction;
    uint8_t thumb_dragging;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_focused;
    uint8_t is_pressed;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_scroll_bar_t test_scroll_bar;
static egui_view_scroll_bar_t preview_scroll_bar;
static egui_view_api_t preview_api;

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void setup_scroll_bar(egui_dim_t content_length, egui_dim_t viewport_length, egui_dim_t offset, egui_dim_t line_step, egui_dim_t page_step)
{
    egui_view_scroll_bar_init(EGUI_VIEW_OF(&test_scroll_bar));
    egui_view_set_size(EGUI_VIEW_OF(&test_scroll_bar), 112, 132);
    egui_view_scroll_bar_set_content_metrics(EGUI_VIEW_OF(&test_scroll_bar), content_length, viewport_length);
    egui_view_scroll_bar_set_step_size(EGUI_VIEW_OF(&test_scroll_bar), line_step, page_step);
    egui_view_scroll_bar_set_offset(EGUI_VIEW_OF(&test_scroll_bar), offset);
    egui_view_scroll_bar_set_current_part(EGUI_VIEW_OF(&test_scroll_bar), EGUI_VIEW_SCROLL_BAR_PART_THUMB);
}

static void layout_view(egui_view_t *view, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(view, &region);
    egui_region_copy(&view->region_screen, &region);
}

static void layout_scroll_bar(egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    layout_view(EGUI_VIEW_OF(&test_scroll_bar), x, y, width, height);
}

static int dispatch_touch_event_to_view(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->dispatch_touch_event(view, &event);
}

static int dispatch_key_event_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->dispatch_key_event(view, &event);
}

static int send_touch_event(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return dispatch_touch_event_to_view(EGUI_VIEW_OF(&test_scroll_bar), type, x, y);
}

static int send_key_to_view(egui_view_t *view, uint8_t key_code)
{
    int handled = 0;

    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_scroll_bar), key_code);
}

static void setup_preview_scroll_bar(void)
{
    egui_view_scroll_bar_init(EGUI_VIEW_OF(&preview_scroll_bar));
    egui_view_set_size(EGUI_VIEW_OF(&preview_scroll_bar), 104, 52);
    egui_view_scroll_bar_set_font(EGUI_VIEW_OF(&preview_scroll_bar), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_scroll_bar_set_meta_font(EGUI_VIEW_OF(&preview_scroll_bar), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_scroll_bar_set_label(EGUI_VIEW_OF(&preview_scroll_bar), "Compact rail");
    egui_view_scroll_bar_set_helper(EGUI_VIEW_OF(&preview_scroll_bar), "Static preview");
    egui_view_scroll_bar_set_palette(EGUI_VIEW_OF(&preview_scroll_bar), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD2DDDA), EGUI_COLOR_HEX(0x17302A),
                                     EGUI_COLOR_HEX(0x57756C), EGUI_COLOR_HEX(0x0D9488), EGUI_COLOR_HEX(0x3BC7B3));
    egui_view_scroll_bar_set_content_metrics(EGUI_VIEW_OF(&preview_scroll_bar), 540, 160);
    egui_view_scroll_bar_set_step_size(EGUI_VIEW_OF(&preview_scroll_bar), 16, 96);
    egui_view_scroll_bar_set_offset(EGUI_VIEW_OF(&preview_scroll_bar), 96);
    egui_view_scroll_bar_set_current_part(EGUI_VIEW_OF(&preview_scroll_bar), EGUI_VIEW_SCROLL_BAR_PART_THUMB);
    egui_view_scroll_bar_set_compact_mode(EGUI_VIEW_OF(&preview_scroll_bar), 1);
    egui_view_scroll_bar_override_static_preview_api(EGUI_VIEW_OF(&preview_scroll_bar), &preview_api);
}

static void layout_preview_scroll_bar(void)
{
    layout_view(EGUI_VIEW_OF(&preview_scroll_bar), 10, 20, 104, 52);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return dispatch_touch_event_to_view(EGUI_VIEW_OF(&preview_scroll_bar), type, x, y);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_scroll_bar), key_code);
}

static void capture_preview_snapshot(scroll_bar_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_scroll_bar)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_scroll_bar)->background;
    snapshot->on_changed = preview_scroll_bar.on_changed;
    snapshot->font = preview_scroll_bar.font;
    snapshot->meta_font = preview_scroll_bar.meta_font;
    snapshot->label = preview_scroll_bar.label;
    snapshot->helper = preview_scroll_bar.helper;
    snapshot->api = EGUI_VIEW_OF(&preview_scroll_bar)->api;
    snapshot->surface_color = preview_scroll_bar.surface_color;
    snapshot->border_color = preview_scroll_bar.border_color;
    snapshot->text_color = preview_scroll_bar.text_color;
    snapshot->muted_text_color = preview_scroll_bar.muted_text_color;
    snapshot->accent_color = preview_scroll_bar.accent_color;
    snapshot->preview_color = preview_scroll_bar.preview_color;
    snapshot->content_length = preview_scroll_bar.content_length;
    snapshot->viewport_length = preview_scroll_bar.viewport_length;
    snapshot->offset = preview_scroll_bar.offset;
    snapshot->line_step = preview_scroll_bar.line_step;
    snapshot->page_step = preview_scroll_bar.page_step;
    snapshot->compact_mode = preview_scroll_bar.compact_mode;
    snapshot->read_only_mode = preview_scroll_bar.read_only_mode;
    snapshot->current_part = preview_scroll_bar.current_part;
    snapshot->pressed_part = preview_scroll_bar.pressed_part;
    snapshot->pressed_track_direction = preview_scroll_bar.pressed_track_direction;
    snapshot->thumb_dragging = preview_scroll_bar.thumb_dragging;
    snapshot->alpha = EGUI_VIEW_OF(&preview_scroll_bar)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_scroll_bar));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_scroll_bar)->is_focused;
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_scroll_bar)->is_pressed;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_scroll_bar)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_scroll_bar)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_scroll_bar)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_scroll_bar)->padding.bottom;
}

static void assert_preview_state_unchanged(const scroll_bar_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_scroll_bar)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_scroll_bar)->background == snapshot->background);
    EGUI_TEST_ASSERT_TRUE(preview_scroll_bar.on_changed == snapshot->on_changed);
    EGUI_TEST_ASSERT_TRUE(preview_scroll_bar.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_scroll_bar.meta_font == snapshot->meta_font);
    EGUI_TEST_ASSERT_TRUE(preview_scroll_bar.label == snapshot->label);
    EGUI_TEST_ASSERT_TRUE(preview_scroll_bar.helper == snapshot->helper);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_scroll_bar)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_scroll_bar.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_scroll_bar.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_scroll_bar.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_scroll_bar.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_scroll_bar.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->preview_color.full, preview_scroll_bar.preview_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->content_length, preview_scroll_bar.content_length);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->viewport_length, preview_scroll_bar.viewport_length);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->offset, preview_scroll_bar.offset);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->line_step, preview_scroll_bar.line_step);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->page_step, preview_scroll_bar.page_step);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_scroll_bar.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_scroll_bar.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_part, preview_scroll_bar.current_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->pressed_part, preview_scroll_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->pressed_track_direction, preview_scroll_bar.pressed_track_direction);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->thumb_dragging, preview_scroll_bar.thumb_dragging);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_scroll_bar)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_scroll_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_scroll_bar)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_scroll_bar)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_scroll_bar)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_scroll_bar)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_scroll_bar)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_scroll_bar)->padding.bottom);
}

static void seed_preview_pressed_state(uint8_t pressed_part, uint8_t track_direction, uint8_t thumb_dragging)
{
    preview_scroll_bar.pressed_part = pressed_part;
    preview_scroll_bar.pressed_track_direction = track_direction;
    preview_scroll_bar.thumb_dragging = thumb_dragging;
    egui_view_set_pressed(EGUI_VIEW_OF(&preview_scroll_bar), 1);
}

static void test_scroll_bar_setters_clear_pressed_state_and_clamp(void)
{
    setup_scroll_bar(800, 200, 280, 20, 80);

    test_scroll_bar.pressed_part = EGUI_VIEW_SCROLL_BAR_PART_DECREASE;
    test_scroll_bar.pressed_track_direction = 1;
    test_scroll_bar.thumb_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_scroll_bar), 1);
    egui_view_scroll_bar_set_content_metrics(EGUI_VIEW_OF(&test_scroll_bar), 200, 80);
    EGUI_TEST_ASSERT_EQUAL_INT(200, egui_view_scroll_bar_get_content_length(EGUI_VIEW_OF(&test_scroll_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(80, egui_view_scroll_bar_get_viewport_length(EGUI_VIEW_OF(&test_scroll_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(120, egui_view_scroll_bar_get_max_offset(EGUI_VIEW_OF(&test_scroll_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(120, egui_view_scroll_bar_get_offset(EGUI_VIEW_OF(&test_scroll_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_NONE, test_scroll_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.pressed_track_direction);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.thumb_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_scroll_bar)->is_pressed);

    test_scroll_bar.pressed_part = EGUI_VIEW_SCROLL_BAR_PART_TRACK;
    test_scroll_bar.pressed_track_direction = 2;
    test_scroll_bar.thumb_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_scroll_bar), 1);
    egui_view_scroll_bar_set_step_size(EGUI_VIEW_OF(&test_scroll_bar), 0, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_scroll_bar.line_step);
    EGUI_TEST_ASSERT_EQUAL_INT(80, test_scroll_bar.page_step);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_NONE, test_scroll_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.pressed_track_direction);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.thumb_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_scroll_bar)->is_pressed);

    test_scroll_bar.pressed_part = EGUI_VIEW_SCROLL_BAR_PART_THUMB;
    test_scroll_bar.thumb_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_scroll_bar), 1);
    egui_view_scroll_bar_set_offset(EGUI_VIEW_OF(&test_scroll_bar), 120);
    EGUI_TEST_ASSERT_EQUAL_INT(120, egui_view_scroll_bar_get_offset(EGUI_VIEW_OF(&test_scroll_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_NONE, test_scroll_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.pressed_track_direction);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.thumb_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_scroll_bar)->is_pressed);

    test_scroll_bar.pressed_part = EGUI_VIEW_SCROLL_BAR_PART_DECREASE;
    test_scroll_bar.pressed_track_direction = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_scroll_bar), 1);
    egui_view_scroll_bar_set_current_part(EGUI_VIEW_OF(&test_scroll_bar), EGUI_VIEW_SCROLL_BAR_PART_INCREASE);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_INCREASE, egui_view_scroll_bar_get_current_part(EGUI_VIEW_OF(&test_scroll_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_NONE, test_scroll_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.pressed_track_direction);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_scroll_bar)->is_pressed);

    test_scroll_bar.pressed_part = EGUI_VIEW_SCROLL_BAR_PART_INCREASE;
    test_scroll_bar.pressed_track_direction = 2;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_scroll_bar), 1);
    egui_view_scroll_bar_set_current_part(EGUI_VIEW_OF(&test_scroll_bar), EGUI_VIEW_SCROLL_BAR_PART_INCREASE);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_INCREASE, egui_view_scroll_bar_get_current_part(EGUI_VIEW_OF(&test_scroll_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_NONE, test_scroll_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.pressed_track_direction);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_scroll_bar)->is_pressed);
}

static void test_scroll_bar_tab_cycles_parts(void)
{
    setup_scroll_bar(800, 200, 120, 20, 80);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_THUMB, egui_view_scroll_bar_get_current_part(EGUI_VIEW_OF(&test_scroll_bar)));
    EGUI_TEST_ASSERT_TRUE(egui_view_scroll_bar_handle_navigation_key(EGUI_VIEW_OF(&test_scroll_bar), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_INCREASE, egui_view_scroll_bar_get_current_part(EGUI_VIEW_OF(&test_scroll_bar)));
    EGUI_TEST_ASSERT_TRUE(egui_view_scroll_bar_handle_navigation_key(EGUI_VIEW_OF(&test_scroll_bar), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_DECREASE, egui_view_scroll_bar_get_current_part(EGUI_VIEW_OF(&test_scroll_bar)));
    EGUI_TEST_ASSERT_TRUE(egui_view_scroll_bar_handle_navigation_key(EGUI_VIEW_OF(&test_scroll_bar), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_THUMB, egui_view_scroll_bar_get_current_part(EGUI_VIEW_OF(&test_scroll_bar)));
}

static void test_scroll_bar_thumb_keyboard_step(void)
{
    setup_scroll_bar(800, 200, 120, 20, 80);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(140, egui_view_scroll_bar_get_offset(EGUI_VIEW_OF(&test_scroll_bar)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_UP));
    EGUI_TEST_ASSERT_EQUAL_INT(120, egui_view_scroll_bar_get_offset(EGUI_VIEW_OF(&test_scroll_bar)));
}

static void test_scroll_bar_plus_minus_page_step(void)
{
    setup_scroll_bar(800, 200, 120, 20, 80);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_PLUS));
    EGUI_TEST_ASSERT_EQUAL_INT(200, egui_view_scroll_bar_get_offset(EGUI_VIEW_OF(&test_scroll_bar)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_MINUS));
    EGUI_TEST_ASSERT_EQUAL_INT(120, egui_view_scroll_bar_get_offset(EGUI_VIEW_OF(&test_scroll_bar)));
}

static void test_scroll_bar_home_end(void)
{
    setup_scroll_bar(800, 200, 120, 20, 80);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_bar_get_offset(EGUI_VIEW_OF(&test_scroll_bar)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(600, egui_view_scroll_bar_get_offset(EGUI_VIEW_OF(&test_scroll_bar)));
}

static void test_scroll_bar_touch_decrease_button(void)
{
    egui_region_t region;

    setup_scroll_bar(800, 200, 120, 20, 80);
    layout_scroll_bar(10, 20, 112, 132);
    EGUI_TEST_ASSERT_TRUE(egui_view_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_scroll_bar), EGUI_VIEW_SCROLL_BAR_PART_DECREASE, &region));
    EGUI_TEST_ASSERT_TRUE(
            send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(100, egui_view_scroll_bar_get_offset(EGUI_VIEW_OF(&test_scroll_bar)));
}

static void test_scroll_bar_touch_track_pages(void)
{
    egui_region_t track_region;
    egui_region_t thumb_region;
    egui_dim_t x;
    egui_dim_t y;

    setup_scroll_bar(800, 200, 120, 20, 80);
    layout_scroll_bar(10, 20, 112, 132);
    EGUI_TEST_ASSERT_TRUE(egui_view_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_scroll_bar), EGUI_VIEW_SCROLL_BAR_PART_TRACK, &track_region));
    EGUI_TEST_ASSERT_TRUE(egui_view_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_scroll_bar), EGUI_VIEW_SCROLL_BAR_PART_THUMB, &thumb_region));
    x = track_region.location.x + track_region.size.width / 2;
    y = thumb_region.location.y + thumb_region.size.height + 6;
    if (y >= track_region.location.y + track_region.size.height)
    {
        y = track_region.location.y + track_region.size.height - 2;
    }

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(200, egui_view_scroll_bar_get_offset(EGUI_VIEW_OF(&test_scroll_bar)));
}

static void test_scroll_bar_thumb_drag_reaches_end(void)
{
    egui_region_t track_region;
    egui_region_t thumb_region;
    egui_dim_t start_x;
    egui_dim_t start_y;
    egui_dim_t end_y;

    setup_scroll_bar(800, 200, 120, 20, 80);
    layout_scroll_bar(10, 20, 112, 132);
    EGUI_TEST_ASSERT_TRUE(egui_view_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_scroll_bar), EGUI_VIEW_SCROLL_BAR_PART_TRACK, &track_region));
    EGUI_TEST_ASSERT_TRUE(egui_view_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_scroll_bar), EGUI_VIEW_SCROLL_BAR_PART_THUMB, &thumb_region));
    start_x = thumb_region.location.x + thumb_region.size.width / 2;
    start_y = thumb_region.location.y + thumb_region.size.height / 2;
    end_y = track_region.location.y + track_region.size.height - thumb_region.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, start_x, start_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_MOVE, start_x, end_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, start_x, end_y));
    EGUI_TEST_ASSERT_EQUAL_INT(600, egui_view_scroll_bar_get_offset(EGUI_VIEW_OF(&test_scroll_bar)));
}

static void test_scroll_bar_touch_cancel_clears_pressed_state(void)
{
    egui_region_t thumb_region;
    egui_dim_t x;
    egui_dim_t y;

    setup_scroll_bar(800, 200, 120, 20, 80);
    layout_scroll_bar(10, 20, 112, 132);
    EGUI_TEST_ASSERT_TRUE(egui_view_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_scroll_bar), EGUI_VIEW_SCROLL_BAR_PART_THUMB, &thumb_region));
    x = thumb_region.location.x + thumb_region.size.width / 2;
    y = thumb_region.location.y + thumb_region.size.height / 2;
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_THUMB, test_scroll_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_scroll_bar.thumb_dragging);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_scroll_bar)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_CANCEL, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_NONE, test_scroll_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.pressed_track_direction);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.thumb_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_scroll_bar)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_CANCEL, x, y));
}

static void test_scroll_bar_compact_mode_clears_pressed_and_ignores_input(void)
{
    egui_region_t decrease_region;

    setup_scroll_bar(800, 200, 120, 20, 80);
    layout_scroll_bar(10, 20, 112, 132);
    EGUI_TEST_ASSERT_TRUE(egui_view_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_scroll_bar), EGUI_VIEW_SCROLL_BAR_PART_DECREASE, &decrease_region));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, decrease_region.location.x + decrease_region.size.width / 2,
                                           decrease_region.location.y + decrease_region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_DECREASE, test_scroll_bar.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_scroll_bar)->is_pressed);

    egui_view_scroll_bar_set_compact_mode(EGUI_VIEW_OF(&test_scroll_bar), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_scroll_bar.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(120, egui_view_scroll_bar_get_offset(EGUI_VIEW_OF(&test_scroll_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_NONE, test_scroll_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.pressed_track_direction);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.thumb_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_scroll_bar)->is_pressed);

    test_scroll_bar.pressed_part = EGUI_VIEW_SCROLL_BAR_PART_THUMB;
    test_scroll_bar.pressed_track_direction = 2;
    test_scroll_bar.thumb_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_scroll_bar), 1);
    egui_view_scroll_bar_set_compact_mode(EGUI_VIEW_OF(&test_scroll_bar), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_NONE, test_scroll_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.pressed_track_direction);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.thumb_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_scroll_bar)->is_pressed);

    test_scroll_bar.pressed_part = EGUI_VIEW_SCROLL_BAR_PART_THUMB;
    test_scroll_bar.pressed_track_direction = 1;
    test_scroll_bar.thumb_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_scroll_bar), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_NONE, test_scroll_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.pressed_track_direction);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.thumb_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_scroll_bar)->is_pressed);

    layout_scroll_bar(10, 20, 112, 132);
    EGUI_TEST_ASSERT_TRUE(egui_view_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_scroll_bar), EGUI_VIEW_SCROLL_BAR_PART_DECREASE, &decrease_region));
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, decrease_region.location.x + decrease_region.size.width / 2,
                                            decrease_region.location.y + decrease_region.size.height / 2));
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, decrease_region.location.x + decrease_region.size.width / 2,
                                            decrease_region.location.y + decrease_region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(120, egui_view_scroll_bar_get_offset(EGUI_VIEW_OF(&test_scroll_bar)));

    egui_view_scroll_bar_set_compact_mode(EGUI_VIEW_OF(&test_scroll_bar), 0);
    layout_scroll_bar(10, 20, 112, 132);
    EGUI_TEST_ASSERT_TRUE(egui_view_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_scroll_bar), EGUI_VIEW_SCROLL_BAR_PART_DECREASE, &decrease_region));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, decrease_region.location.x + decrease_region.size.width / 2,
                                           decrease_region.location.y + decrease_region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, decrease_region.location.x + decrease_region.size.width / 2,
                                           decrease_region.location.y + decrease_region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(100, egui_view_scroll_bar_get_offset(EGUI_VIEW_OF(&test_scroll_bar)));
}

static void test_scroll_bar_read_only_mode_clears_pressed_and_ignores_input(void)
{
    egui_region_t decrease_region;

    setup_scroll_bar(800, 200, 120, 20, 80);
    layout_scroll_bar(10, 20, 112, 132);
    EGUI_TEST_ASSERT_TRUE(egui_view_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_scroll_bar), EGUI_VIEW_SCROLL_BAR_PART_DECREASE, &decrease_region));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, decrease_region.location.x + decrease_region.size.width / 2,
                                           decrease_region.location.y + decrease_region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_DECREASE, test_scroll_bar.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_scroll_bar)->is_pressed);

    egui_view_scroll_bar_set_read_only_mode(EGUI_VIEW_OF(&test_scroll_bar), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_scroll_bar.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(120, egui_view_scroll_bar_get_offset(EGUI_VIEW_OF(&test_scroll_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_NONE, test_scroll_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.pressed_track_direction);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.thumb_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_scroll_bar)->is_pressed);

    test_scroll_bar.pressed_part = EGUI_VIEW_SCROLL_BAR_PART_THUMB;
    test_scroll_bar.pressed_track_direction = 2;
    test_scroll_bar.thumb_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_scroll_bar), 1);
    egui_view_scroll_bar_set_read_only_mode(EGUI_VIEW_OF(&test_scroll_bar), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_NONE, test_scroll_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.pressed_track_direction);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.thumb_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_scroll_bar)->is_pressed);

    test_scroll_bar.pressed_part = EGUI_VIEW_SCROLL_BAR_PART_TRACK;
    test_scroll_bar.pressed_track_direction = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_scroll_bar), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_NONE, test_scroll_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.pressed_track_direction);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.thumb_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_scroll_bar)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, decrease_region.location.x + decrease_region.size.width / 2,
                                            decrease_region.location.y + decrease_region.size.height / 2));
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, decrease_region.location.x + decrease_region.size.width / 2,
                                            decrease_region.location.y + decrease_region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(120, egui_view_scroll_bar_get_offset(EGUI_VIEW_OF(&test_scroll_bar)));

    egui_view_scroll_bar_set_read_only_mode(EGUI_VIEW_OF(&test_scroll_bar), 0);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(140, egui_view_scroll_bar_get_offset(EGUI_VIEW_OF(&test_scroll_bar)));
}

static void test_scroll_bar_disabled_ignores_input_and_clears_pressed_state(void)
{
    egui_region_t decrease_region;

    setup_scroll_bar(800, 200, 120, 20, 80);
    layout_scroll_bar(10, 20, 112, 132);
    EGUI_TEST_ASSERT_TRUE(egui_view_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_scroll_bar), EGUI_VIEW_SCROLL_BAR_PART_DECREASE, &decrease_region));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, decrease_region.location.x + decrease_region.size.width / 2,
                                           decrease_region.location.y + decrease_region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_DECREASE, test_scroll_bar.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_scroll_bar)->is_pressed);

    egui_view_set_enable(EGUI_VIEW_OF(&test_scroll_bar), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, decrease_region.location.x + decrease_region.size.width / 2,
                                            decrease_region.location.y + decrease_region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_NONE, test_scroll_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.pressed_track_direction);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.thumb_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_scroll_bar)->is_pressed);

    test_scroll_bar.pressed_part = EGUI_VIEW_SCROLL_BAR_PART_THUMB;
    test_scroll_bar.pressed_track_direction = 1;
    test_scroll_bar.thumb_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_scroll_bar), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_BAR_PART_NONE, test_scroll_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.pressed_track_direction);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_scroll_bar.thumb_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_scroll_bar)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, decrease_region.location.x + decrease_region.size.width / 2,
                                            decrease_region.location.y + decrease_region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(120, egui_view_scroll_bar_get_offset(EGUI_VIEW_OF(&test_scroll_bar)));

    egui_view_set_enable(EGUI_VIEW_OF(&test_scroll_bar), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, decrease_region.location.x + decrease_region.size.width / 2,
                                           decrease_region.location.y + decrease_region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, decrease_region.location.x + decrease_region.size.width / 2,
                                           decrease_region.location.y + decrease_region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(100, egui_view_scroll_bar_get_offset(EGUI_VIEW_OF(&test_scroll_bar)));
}

static void test_scroll_bar_clamps_metrics_and_offset(void)
{
    setup_scroll_bar(10, 5, 3, 0, 0);
    egui_view_scroll_bar_set_content_metrics(EGUI_VIEW_OF(&test_scroll_bar), 0, 0);
    egui_view_scroll_bar_set_offset(EGUI_VIEW_OF(&test_scroll_bar), 90);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_scroll_bar_get_content_length(EGUI_VIEW_OF(&test_scroll_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_scroll_bar_get_viewport_length(EGUI_VIEW_OF(&test_scroll_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_bar_get_max_offset(EGUI_VIEW_OF(&test_scroll_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_bar_get_offset(EGUI_VIEW_OF(&test_scroll_bar)));
}

static void test_scroll_bar_static_preview_consumes_input_and_keeps_state(void)
{
    scroll_bar_preview_snapshot_t initial_snapshot;
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_scroll_bar();
    layout_preview_scroll_bar();
    x = EGUI_VIEW_OF(&preview_scroll_bar)->region_screen.location.x + EGUI_VIEW_OF(&preview_scroll_bar)->region_screen.size.width / 2;
    y = EGUI_VIEW_OF(&preview_scroll_bar)->region_screen.location.y + EGUI_VIEW_OF(&preview_scroll_bar)->region_screen.size.height / 2;
    capture_preview_snapshot(&initial_snapshot);

    seed_preview_pressed_state(EGUI_VIEW_SCROLL_BAR_PART_THUMB, 2, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    assert_preview_state_unchanged(&initial_snapshot);

    seed_preview_pressed_state(EGUI_VIEW_SCROLL_BAR_PART_DECREASE, 1, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_scroll_bar_run(void)
{
    EGUI_TEST_SUITE_BEGIN(scroll_bar);
    EGUI_TEST_RUN(test_scroll_bar_setters_clear_pressed_state_and_clamp);
    EGUI_TEST_RUN(test_scroll_bar_tab_cycles_parts);
    EGUI_TEST_RUN(test_scroll_bar_thumb_keyboard_step);
    EGUI_TEST_RUN(test_scroll_bar_plus_minus_page_step);
    EGUI_TEST_RUN(test_scroll_bar_home_end);
    EGUI_TEST_RUN(test_scroll_bar_touch_decrease_button);
    EGUI_TEST_RUN(test_scroll_bar_touch_track_pages);
    EGUI_TEST_RUN(test_scroll_bar_thumb_drag_reaches_end);
    EGUI_TEST_RUN(test_scroll_bar_touch_cancel_clears_pressed_state);
    EGUI_TEST_RUN(test_scroll_bar_compact_mode_clears_pressed_and_ignores_input);
    EGUI_TEST_RUN(test_scroll_bar_read_only_mode_clears_pressed_and_ignores_input);
    EGUI_TEST_RUN(test_scroll_bar_disabled_ignores_input_and_clears_pressed_state);
    EGUI_TEST_RUN(test_scroll_bar_clamps_metrics_and_offset);
    EGUI_TEST_RUN(test_scroll_bar_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
