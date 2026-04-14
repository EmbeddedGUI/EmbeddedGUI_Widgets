#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_info_label.h"

#include "../../HelloCustomWidgets/display/info_label/egui_view_info_label.h"
#include "../../HelloCustomWidgets/display/info_label/egui_view_info_label.c"

static hcw_info_label_t test_widget;
static hcw_info_label_t preview_widget;
static egui_view_api_t preview_api;
static uint8_t g_open_state;
static uint8_t g_open_count;

static void on_open_changed(egui_view_t *self, uint8_t is_open)
{
    EGUI_UNUSED(self);
    g_open_state = is_open;
    g_open_count++;
}

static void reset_listener_state(void)
{
    g_open_state = 0xFF;
    g_open_count = 0;
}

static void setup_widget(void)
{
    hcw_info_label_init(EGUI_VIEW_OF(&test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_widget), 148, 96);
    hcw_info_label_set_text(EGUI_VIEW_OF(&test_widget), "Project policy");
    hcw_info_label_set_info_title(EGUI_VIEW_OF(&test_widget), "Versioning");
    hcw_info_label_set_info_body(EGUI_VIEW_OF(&test_widget), "Keep release notes aligned.");
    hcw_info_label_set_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_info_label_set_meta_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_info_label_set_on_open_changed_listener(EGUI_VIEW_OF(&test_widget), on_open_changed);
    reset_listener_state();
}

static void setup_preview_widget(void)
{
    hcw_info_label_init(EGUI_VIEW_OF(&preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_widget), 96, 54);
    hcw_info_label_apply_compact_style(EGUI_VIEW_OF(&preview_widget));
    hcw_info_label_set_text(EGUI_VIEW_OF(&preview_widget), "Compact help");
    hcw_info_label_set_info_title(EGUI_VIEW_OF(&preview_widget), "Inline note");
    hcw_info_label_set_info_body(EGUI_VIEW_OF(&preview_widget), "Static preview.");
    hcw_info_label_set_font(EGUI_VIEW_OF(&preview_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_info_label_set_meta_font(EGUI_VIEW_OF(&preview_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_info_label_set_open(EGUI_VIEW_OF(&preview_widget), 1);
    hcw_info_label_set_on_open_changed_listener(EGUI_VIEW_OF(&preview_widget), on_open_changed);
    hcw_info_label_override_static_preview_api(EGUI_VIEW_OF(&preview_widget), &preview_api);
    reset_listener_state();
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

static int send_touch_to_view(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->on_touch_event(view, &event);
}

static int send_key_to_view(egui_view_t *view, uint8_t key_code)
{
    egui_key_event_t event;
    int handled = 0;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    handled |= view->api->on_key_event(view, &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    handled |= view->api->on_key_event(view, &event);
    return handled;
}

static void seed_pressed_state(hcw_info_label_t *widget, uint8_t visual_pressed)
{
    widget->pressed_part = HCW_INFO_LABEL_PART_ICON;
    egui_view_set_pressed(EGUI_VIEW_OF(widget), visual_pressed ? 1 : 0);
}

static void assert_pressed_cleared(hcw_info_label_t *widget)
{
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_INFO_LABEL_PART_NONE, widget->pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
}

static void get_icon_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    EGUI_TEST_ASSERT_TRUE(hcw_info_label_get_part_region(view, HCW_INFO_LABEL_PART_ICON, &region));
    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
}

static void test_info_label_style_helpers_and_setters_clear_pressed_state(void)
{
    egui_region_t bubble_region;

    setup_widget();

    EGUI_TEST_ASSERT_EQUAL_INT(0, hcw_info_label_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_FALSE(test_widget.compact_mode);
    EGUI_TEST_ASSERT_FALSE(test_widget.read_only_mode);

    seed_pressed_state(&test_widget, 1);
    hcw_info_label_apply_compact_style(EGUI_VIEW_OF(&test_widget));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.compact_mode);
    EGUI_TEST_ASSERT_FALSE(test_widget.read_only_mode);

    seed_pressed_state(&test_widget, 0);
    hcw_info_label_apply_read_only_style(EGUI_VIEW_OF(&test_widget));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_FALSE(test_widget.compact_mode);
    EGUI_TEST_ASSERT_TRUE(test_widget.read_only_mode);

    seed_pressed_state(&test_widget, 1);
    hcw_info_label_set_text(EGUI_VIEW_OF(&test_widget), "Export guidance");
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(test_widget.label, "Export guidance"));

    seed_pressed_state(&test_widget, 1);
    hcw_info_label_set_info_title(EGUI_VIEW_OF(&test_widget), "Sensitive content");
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(test_widget.info_title, "Sensitive content"));

    seed_pressed_state(&test_widget, 0);
    hcw_info_label_set_info_body(EGUI_VIEW_OF(&test_widget), "Mask personal data.");
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(test_widget.info_body, "Mask personal data."));

    seed_pressed_state(&test_widget, 1);
    hcw_info_label_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(&test_widget, 0);
    hcw_info_label_set_meta_font(EGUI_VIEW_OF(&test_widget), NULL);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(&test_widget, 1);
    hcw_info_label_set_icon_font(EGUI_VIEW_OF(&test_widget), NULL);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.icon_font == EGUI_FONT_ICON_MS_16);

    seed_pressed_state(&test_widget, 1);
    hcw_info_label_set_palette(EGUI_VIEW_OF(&test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                               EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162), EGUI_COLOR_HEX(0x707172));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x505152).full, test_widget.accent_color.full);

    seed_pressed_state(&test_widget, 0);
    hcw_info_label_set_compact_mode(EGUI_VIEW_OF(&test_widget), 1);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.compact_mode);

    seed_pressed_state(&test_widget, 1);
    hcw_info_label_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 1);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.read_only_mode);

    layout_view(EGUI_VIEW_OF(&test_widget), 12, 18, 148, 96);
    EGUI_TEST_ASSERT_FALSE(hcw_info_label_get_part_region(EGUI_VIEW_OF(&test_widget), HCW_INFO_LABEL_PART_BUBBLE, &bubble_region));
    hcw_info_label_set_open(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_TRUE(hcw_info_label_get_part_region(EGUI_VIEW_OF(&test_widget), HCW_INFO_LABEL_PART_BUBBLE, &bubble_region));
}

static void test_info_label_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t icon_x;
    egui_dim_t icon_y;
    egui_dim_t outside_x;

    setup_widget();
    layout_view(EGUI_VIEW_OF(&test_widget), 12, 18, 148, 96);
    get_icon_center(EGUI_VIEW_OF(&test_widget), &icon_x, &icon_y);
    outside_x = icon_x - 42;

    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, icon_x, icon_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, icon_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_UP, outside_x, icon_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, hcw_info_label_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_count);
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, icon_x, icon_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, icon_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_MOVE, icon_x, icon_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_UP, icon_x, icon_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_info_label_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_open_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_open_state);
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, icon_x, icon_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_CANCEL, icon_x, icon_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_info_label_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_open_count);
    assert_pressed_cleared(&test_widget);
}

static void test_info_label_keyboard_toggle_and_escape_close(void)
{
    setup_widget();

    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_info_label_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_open_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_open_state);
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(0, hcw_info_label_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_open_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_state);
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_info_label_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, g_open_count);

    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_EQUAL_INT(0, hcw_info_label_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, g_open_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_state);
    assert_pressed_cleared(&test_widget);
}

static void test_info_label_static_preview_consumes_input(void)
{
    egui_dim_t icon_x;
    egui_dim_t icon_y;

    setup_preview_widget();
    layout_view(EGUI_VIEW_OF(&preview_widget), 12, 18, 96, 54);
    get_icon_center(EGUI_VIEW_OF(&preview_widget), &icon_x, &icon_y);

    seed_pressed_state(&preview_widget, 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_widget), EGUI_MOTION_EVENT_ACTION_DOWN, icon_x, icon_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_widget), EGUI_MOTION_EVENT_ACTION_UP, icon_x, icon_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_info_label_get_open(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_count);
    assert_pressed_cleared(&preview_widget);

    seed_pressed_state(&preview_widget, 0);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_widget), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_info_label_get_open(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_count);
    assert_pressed_cleared(&preview_widget);
}

void test_info_label_run(void)
{
    EGUI_TEST_SUITE_BEGIN(info_label);
    EGUI_TEST_RUN(test_info_label_style_helpers_and_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_info_label_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_info_label_keyboard_toggle_and_escape_close);
    EGUI_TEST_RUN(test_info_label_static_preview_consumes_input);
    EGUI_TEST_SUITE_END();
}
