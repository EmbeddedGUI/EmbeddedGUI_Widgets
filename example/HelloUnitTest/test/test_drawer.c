#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_drawer.h"

#include "../../HelloCustomWidgets/layout/drawer/egui_view_drawer.h"
#include "../../HelloCustomWidgets/layout/drawer/egui_view_drawer.c"

static egui_view_drawer_t test_widget;
static egui_view_drawer_t preview_widget;
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

static int send_touch(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->on_touch_event(view, &event);
}

static int send_key(egui_view_t *view, uint8_t key_code)
{
    egui_key_event_t event;
    int handled = 0;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    handled |= view->api->dispatch_key_event(view, &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    handled |= view->api->dispatch_key_event(view, &event);
    return handled;
}

static void seed_pressed_state(egui_view_drawer_t *widget, uint8_t part)
{
    widget->pressed_part = part;
    egui_view_set_pressed(EGUI_VIEW_OF(widget), 1);
}

static void assert_pressed_cleared(egui_view_drawer_t *widget)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DRAWER_PART_NONE, widget->pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
}

static void get_part_center(egui_view_t *view, uint8_t part, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    EGUI_TEST_ASSERT_TRUE(egui_view_drawer_get_part_region(view, part, &region));
    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
}

static void setup_widget(void)
{
    egui_view_drawer_init(EGUI_VIEW_OF(&test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_widget), 180, 112);
    egui_view_drawer_set_eyebrow(EGUI_VIEW_OF(&test_widget), "Filters");
    egui_view_drawer_set_title(EGUI_VIEW_OF(&test_widget), "Task filters");
    egui_view_drawer_set_body_primary(EGUI_VIEW_OF(&test_widget), "Inline drawer content.");
    egui_view_drawer_set_body_secondary(EGUI_VIEW_OF(&test_widget), "Secondary line.");
    egui_view_drawer_set_footer(EGUI_VIEW_OF(&test_widget), "Inline / start");
    egui_view_drawer_set_tag(EGUI_VIEW_OF(&test_widget), "Inline");
    egui_view_drawer_set_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_drawer_set_meta_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_drawer_set_on_open_changed_listener(EGUI_VIEW_OF(&test_widget), on_open_changed);
    reset_listener_state();
}

static void setup_preview_widget(void)
{
    egui_view_drawer_init(EGUI_VIEW_OF(&preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_widget), 104, 72);
    egui_view_drawer_set_eyebrow(EGUI_VIEW_OF(&preview_widget), "Compact");
    egui_view_drawer_set_title(EGUI_VIEW_OF(&preview_widget), "Quick rail");
    egui_view_drawer_set_body_primary(EGUI_VIEW_OF(&preview_widget), "Static preview.");
    egui_view_drawer_set_footer(EGUI_VIEW_OF(&preview_widget), "Preview");
    egui_view_drawer_set_tag(EGUI_VIEW_OF(&preview_widget), "Compact");
    egui_view_drawer_set_compact_mode(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_drawer_set_presentation_mode(EGUI_VIEW_OF(&preview_widget), EGUI_VIEW_DRAWER_MODE_OVERLAY);
    egui_view_drawer_set_open(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_drawer_set_on_open_changed_listener(EGUI_VIEW_OF(&preview_widget), on_open_changed);
    egui_view_drawer_override_static_preview_api(EGUI_VIEW_OF(&preview_widget), &preview_api);
    reset_listener_state();
}

static void test_drawer_defaults_and_setters_clear_pressed_state(void)
{
    setup_widget();

    EGUI_TEST_ASSERT_TRUE(egui_view_drawer_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DRAWER_ANCHOR_START, egui_view_drawer_get_anchor(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DRAWER_MODE_INLINE, egui_view_drawer_get_presentation_mode(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_FALSE(egui_view_drawer_get_compact_mode(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_FALSE(egui_view_drawer_get_read_only_mode(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xFFFFFF).full, test_widget.surface_color.full);

    seed_pressed_state(&test_widget, EGUI_VIEW_DRAWER_PART_TOGGLE);
    egui_view_drawer_set_title(EGUI_VIEW_OF(&test_widget), "Review notes");
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, EGUI_VIEW_DRAWER_PART_TOGGLE);
    egui_view_drawer_set_anchor(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_DRAWER_ANCHOR_END);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DRAWER_ANCHOR_END, egui_view_drawer_get_anchor(EGUI_VIEW_OF(&test_widget)));

    seed_pressed_state(&test_widget, EGUI_VIEW_DRAWER_PART_TOGGLE);
    egui_view_drawer_set_presentation_mode(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_DRAWER_MODE_OVERLAY);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DRAWER_MODE_OVERLAY, egui_view_drawer_get_presentation_mode(EGUI_VIEW_OF(&test_widget)));

    seed_pressed_state(&test_widget, EGUI_VIEW_DRAWER_PART_TOGGLE);
    egui_view_drawer_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(&test_widget, EGUI_VIEW_DRAWER_PART_TOGGLE);
    egui_view_drawer_set_meta_font(EGUI_VIEW_OF(&test_widget), NULL);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(&test_widget, EGUI_VIEW_DRAWER_PART_TOGGLE);
    egui_view_drawer_set_palette(EGUI_VIEW_OF(&test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                 EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162), EGUI_COLOR_HEX(0x707172),
                                 EGUI_COLOR_HEX(0x808182));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x707172).full, test_widget.accent_color.full);

    seed_pressed_state(&test_widget, EGUI_VIEW_DRAWER_PART_TOGGLE);
    egui_view_drawer_set_compact_mode(EGUI_VIEW_OF(&test_widget), 1);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(egui_view_drawer_get_compact_mode(EGUI_VIEW_OF(&test_widget)));

    seed_pressed_state(&test_widget, EGUI_VIEW_DRAWER_PART_TOGGLE);
    egui_view_drawer_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 1);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(egui_view_drawer_get_read_only_mode(EGUI_VIEW_OF(&test_widget)));
}

static void test_drawer_touch_same_target_release_for_toggle_and_close(void)
{
    egui_dim_t toggle_x;
    egui_dim_t toggle_y;
    egui_dim_t close_x;
    egui_dim_t close_y;
    egui_dim_t outside_x;

    setup_widget();
    egui_view_drawer_set_open(EGUI_VIEW_OF(&test_widget), 0);
    reset_listener_state();
    layout_view(EGUI_VIEW_OF(&test_widget), 10, 16, 180, 112);
    get_part_center(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_DRAWER_PART_TOGGLE, &toggle_x, &toggle_y);
    outside_x = toggle_x + 36;

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, toggle_x, toggle_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, toggle_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_UP, outside_x, toggle_y));
    EGUI_TEST_ASSERT_FALSE(egui_view_drawer_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_count);
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, toggle_x, toggle_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, toggle_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_MOVE, toggle_x, toggle_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_UP, toggle_x, toggle_y));
    EGUI_TEST_ASSERT_TRUE(egui_view_drawer_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_open_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_open_state);
    assert_pressed_cleared(&test_widget);

    get_part_center(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_DRAWER_PART_CLOSE, &close_x, &close_y);
    outside_x = close_x - 28;
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, close_x, close_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, close_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_UP, outside_x, close_y));
    EGUI_TEST_ASSERT_TRUE(egui_view_drawer_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_open_count);
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, close_x, close_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_UP, close_x, close_y));
    EGUI_TEST_ASSERT_FALSE(egui_view_drawer_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_open_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_state);
    assert_pressed_cleared(&test_widget);
}

static void test_drawer_keyboard_toggle_and_escape_close(void)
{
    setup_widget();
    egui_view_drawer_set_open(EGUI_VIEW_OF(&test_widget), 0);
    reset_listener_state();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(egui_view_drawer_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_open_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_open_state);
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_FALSE(egui_view_drawer_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_open_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_state);
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_FALSE(egui_view_drawer_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, g_open_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_state);
    assert_pressed_cleared(&test_widget);
}

static void test_drawer_static_preview_consumes_input(void)
{
    egui_dim_t toggle_x;
    egui_dim_t toggle_y;
    egui_dim_t close_x;
    egui_dim_t close_y;

    setup_preview_widget();
    layout_view(EGUI_VIEW_OF(&preview_widget), 12, 18, 104, 72);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DRAWER_ANCHOR_START, egui_view_drawer_get_anchor(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DRAWER_MODE_OVERLAY, egui_view_drawer_get_presentation_mode(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_TRUE(egui_view_drawer_get_open(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_TRUE(egui_view_drawer_get_compact_mode(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_FALSE(egui_view_drawer_get_read_only_mode(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_count);
    get_part_center(EGUI_VIEW_OF(&preview_widget), EGUI_VIEW_DRAWER_PART_TOGGLE, &toggle_x, &toggle_y);
    get_part_center(EGUI_VIEW_OF(&preview_widget), EGUI_VIEW_DRAWER_PART_CLOSE, &close_x, &close_y);

    seed_pressed_state(&preview_widget, EGUI_VIEW_DRAWER_PART_TOGGLE);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_widget), EGUI_MOTION_EVENT_ACTION_DOWN, toggle_x, toggle_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_widget), EGUI_MOTION_EVENT_ACTION_UP, toggle_x, toggle_y));
    EGUI_TEST_ASSERT_TRUE(egui_view_drawer_get_open(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_count);
    assert_pressed_cleared(&preview_widget);

    seed_pressed_state(&preview_widget, EGUI_VIEW_DRAWER_PART_CLOSE);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_widget), EGUI_MOTION_EVENT_ACTION_DOWN, close_x, close_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_widget), EGUI_MOTION_EVENT_ACTION_UP, close_x, close_y));
    EGUI_TEST_ASSERT_TRUE(egui_view_drawer_get_open(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_count);
    assert_pressed_cleared(&preview_widget);

    seed_pressed_state(&preview_widget, EGUI_VIEW_DRAWER_PART_TOGGLE);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_widget), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(egui_view_drawer_get_open(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_count);
    assert_pressed_cleared(&preview_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DRAWER_ANCHOR_START, egui_view_drawer_get_anchor(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DRAWER_MODE_OVERLAY, egui_view_drawer_get_presentation_mode(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_TRUE(egui_view_drawer_get_open(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_TRUE(egui_view_drawer_get_compact_mode(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_FALSE(egui_view_drawer_get_read_only_mode(EGUI_VIEW_OF(&preview_widget)));
}

static void test_drawer_read_only_and_disabled_guards_clear_pressed_state(void)
{
    egui_dim_t toggle_x;
    egui_dim_t toggle_y;

    setup_widget();
    layout_view(EGUI_VIEW_OF(&test_widget), 10, 16, 180, 112);
    get_part_center(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_DRAWER_PART_TOGGLE, &toggle_x, &toggle_y);

    egui_view_drawer_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 1);
    seed_pressed_state(&test_widget, EGUI_VIEW_DRAWER_PART_TOGGLE);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, toggle_x, toggle_y));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(egui_view_drawer_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_count);

    seed_pressed_state(&test_widget, EGUI_VIEW_DRAWER_PART_TOGGLE);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(egui_view_drawer_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_count);

    egui_view_drawer_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 0);
    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 0);

    seed_pressed_state(&test_widget, EGUI_VIEW_DRAWER_PART_TOGGLE);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, toggle_x, toggle_y));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(egui_view_drawer_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_count);

    seed_pressed_state(&test_widget, EGUI_VIEW_DRAWER_PART_TOGGLE);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(egui_view_drawer_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_count);

    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_FALSE(egui_view_drawer_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_open_count);
}

void test_drawer_run(void)
{
    EGUI_TEST_SUITE_BEGIN(drawer);
    EGUI_TEST_RUN(test_drawer_defaults_and_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_drawer_touch_same_target_release_for_toggle_and_close);
    EGUI_TEST_RUN(test_drawer_keyboard_toggle_and_escape_close);
    EGUI_TEST_RUN(test_drawer_static_preview_consumes_input);
    EGUI_TEST_RUN(test_drawer_read_only_and_disabled_guards_clear_pressed_state);
    EGUI_TEST_SUITE_END();
}
