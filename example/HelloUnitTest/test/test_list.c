#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_list.h"

#include "../../HelloCustomWidgets/layout/list/egui_view_list.h"
#include "../../HelloCustomWidgets/layout/list/egui_view_list.c"

static egui_view_reference_list_t test_widget;
static egui_view_reference_list_t preview_widget;
static egui_view_api_t preview_api;
static uint8_t changed_count;
static uint8_t last_index;

static const egui_view_reference_list_item_t g_items[] = {
        {"Inbox", "4 tasks waiting", "Live", EGUI_VIEW_REFERENCE_LIST_TONE_ACCENT, 1},
        {"Approvals", "Owner review due", "Due", EGUI_VIEW_REFERENCE_LIST_TONE_WARNING, 0},
        {"Published", "24 files synced", "Ready", EGUI_VIEW_REFERENCE_LIST_TONE_SUCCESS, 0},
        {"Archive", "Cold storage", "Muted", EGUI_VIEW_REFERENCE_LIST_TONE_NEUTRAL, 0},
};

static const egui_view_reference_list_item_t g_overflow_items[] = {
        {"A", "", "1", EGUI_VIEW_REFERENCE_LIST_TONE_ACCENT, 0},
        {"B", "", "2", EGUI_VIEW_REFERENCE_LIST_TONE_SUCCESS, 0},
        {"C", "", "3", EGUI_VIEW_REFERENCE_LIST_TONE_WARNING, 0},
        {"D", "", "4", EGUI_VIEW_REFERENCE_LIST_TONE_NEUTRAL, 0},
        {"E", "", "5", EGUI_VIEW_REFERENCE_LIST_TONE_ACCENT, 0},
        {"F", "", "6", EGUI_VIEW_REFERENCE_LIST_TONE_SUCCESS, 0},
};

static void on_selection_changed(egui_view_t *self, uint8_t index)
{
    EGUI_UNUSED(self);
    changed_count++;
    last_index = index;
}

static void reset_listener_state(void)
{
    changed_count = 0;
    last_index = EGUI_VIEW_REFERENCE_LIST_INDEX_NONE;
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
    handled |= view->api->on_key_event(view, &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    handled |= view->api->on_key_event(view, &event);
    return handled;
}

static void seed_pressed_state(egui_view_reference_list_t *widget, uint8_t index)
{
    widget->pressed_index = index;
    egui_view_set_pressed(EGUI_VIEW_OF(widget), true);
}

static void assert_pressed_cleared(egui_view_reference_list_t *widget)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_REFERENCE_LIST_INDEX_NONE, widget->pressed_index);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
}

static void get_item_center(egui_view_t *view, uint8_t index, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    EGUI_TEST_ASSERT_TRUE(egui_view_reference_list_get_item_region(view, index, &region));
    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
}

static void setup_widget(void)
{
    egui_view_reference_list_init(EGUI_VIEW_OF(&test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_widget), 180, 112);
    egui_view_reference_list_set_items(EGUI_VIEW_OF(&test_widget), g_items, EGUI_ARRAY_SIZE(g_items));
    egui_view_reference_list_set_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_reference_list_set_meta_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_reference_list_set_on_selection_changed_listener(EGUI_VIEW_OF(&test_widget), on_selection_changed);
    reset_listener_state();
}

static void setup_preview_widget(void)
{
    egui_view_reference_list_init(EGUI_VIEW_OF(&preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_widget), 104, 72);
    egui_view_reference_list_set_items(EGUI_VIEW_OF(&preview_widget), g_items, EGUI_ARRAY_SIZE(g_items));
    egui_view_reference_list_set_current_index(EGUI_VIEW_OF(&preview_widget), 2);
    egui_view_reference_list_set_compact_mode(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_reference_list_set_read_only_mode(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_reference_list_set_on_selection_changed_listener(EGUI_VIEW_OF(&preview_widget), on_selection_changed);
    egui_view_reference_list_override_static_preview_api(EGUI_VIEW_OF(&preview_widget), &preview_api);
    reset_listener_state();
}

static void test_list_setters_clamp_getters_and_helpers(void)
{
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);

    setup_widget();

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ARRAY_SIZE(g_items), egui_view_reference_list_get_item_count(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_reference_list_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_FALSE(egui_view_reference_list_get_compact_mode(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_FALSE(egui_view_reference_list_get_read_only_mode(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xFFFFFF).full, test_widget.surface_color.full);

    seed_pressed_state(&test_widget, 1);
    egui_view_reference_list_set_items(EGUI_VIEW_OF(&test_widget), g_overflow_items, EGUI_ARRAY_SIZE(g_overflow_items));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_REFERENCE_LIST_MAX_ITEMS, egui_view_reference_list_get_item_count(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_reference_list_get_current_index(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1);
    egui_view_reference_list_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(&test_widget, 2);
    egui_view_reference_list_set_meta_font(EGUI_VIEW_OF(&test_widget), NULL);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(&test_widget, 2);
    egui_view_reference_list_set_palette(EGUI_VIEW_OF(&test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                         EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162), EGUI_COLOR_HEX(0x707172),
                                         EGUI_COLOR_HEX(0x808182));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x505152).full, test_widget.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x808182).full, test_widget.neutral_color.full);

    seed_pressed_state(&test_widget, 3);
    egui_view_reference_list_set_compact_mode(EGUI_VIEW_OF(&test_widget), 3);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(egui_view_reference_list_get_compact_mode(EGUI_VIEW_OF(&test_widget)));

    seed_pressed_state(&test_widget, 4);
    egui_view_reference_list_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 2);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(egui_view_reference_list_get_read_only_mode(EGUI_VIEW_OF(&test_widget)));

    egui_view_reference_list_set_items(EGUI_VIEW_OF(&test_widget), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_reference_list_get_item_count(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_REFERENCE_LIST_INDEX_NONE, egui_view_reference_list_get_current_index(EGUI_VIEW_OF(&test_widget)));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_REFERENCE_LIST_MAX_ITEMS, egui_view_reference_list_clamp_item_count(9));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_reference_list_text_len(NULL));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_reference_list_text_len("List"));
    EGUI_TEST_ASSERT_EQUAL_INT(33, egui_view_reference_list_badge_width("Due", 0, 40));
    EGUI_TEST_ASSERT_EQUAL_INT(22, egui_view_reference_list_badge_width("RO", 1, 40));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_reference_list_badge_width("", 0, 40));
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_DARK_GREY, 68).full, egui_view_reference_list_mix_disabled(sample).full);
}

static void test_list_selection_regions_and_internal_helpers(void)
{
    egui_dim_t x1;
    egui_dim_t y1;
    egui_dim_t x2;
    egui_dim_t y2;
    egui_region_t region;

    setup_widget();
    layout_view(EGUI_VIEW_OF(&test_widget), 10, 20, 180, 112);

    EGUI_TEST_ASSERT_TRUE(egui_view_reference_list_get_item_region(EGUI_VIEW_OF(&test_widget), 0, &region));
    EGUI_TEST_ASSERT_TRUE(region.size.width > 0);
    EGUI_TEST_ASSERT_FALSE(egui_view_reference_list_get_item_region(EGUI_VIEW_OF(&test_widget), 7, &region));
    EGUI_TEST_ASSERT_TRUE(egui_view_reference_list_get_item(&test_widget, 0) == &g_items[0]);
    EGUI_TEST_ASSERT_NULL(egui_view_reference_list_get_item(&test_widget, 7));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full,
                               egui_view_reference_list_tone_color(&test_widget, EGUI_VIEW_REFERENCE_LIST_TONE_ACCENT).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F7B45).full,
                               egui_view_reference_list_tone_color(&test_widget, EGUI_VIEW_REFERENCE_LIST_TONE_SUCCESS).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xA55A00).full,
                               egui_view_reference_list_tone_color(&test_widget, EGUI_VIEW_REFERENCE_LIST_TONE_WARNING).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x7A8796).full,
                               egui_view_reference_list_tone_color(&test_widget, EGUI_VIEW_REFERENCE_LIST_TONE_NEUTRAL).full);

    get_item_center(EGUI_VIEW_OF(&test_widget), 1, &x1, &y1);
    get_item_center(EGUI_VIEW_OF(&test_widget), 2, &x2, &y2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_reference_list_hit_index(&test_widget, EGUI_VIEW_OF(&test_widget), x1, y1));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_reference_list_hit_index(&test_widget, EGUI_VIEW_OF(&test_widget), x2, y2));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_REFERENCE_LIST_INDEX_NONE,
                               egui_view_reference_list_hit_index(&test_widget, EGUI_VIEW_OF(&test_widget), 0, 0));

    egui_view_reference_list_set_current_index(EGUI_VIEW_OF(&test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_reference_list_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, last_index);

    seed_pressed_state(&test_widget, 2);
    egui_view_reference_list_set_current_index(EGUI_VIEW_OF(&test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    assert_pressed_cleared(&test_widget);

    egui_view_reference_list_set_current_index(EGUI_VIEW_OF(&test_widget), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_reference_list_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
}

static void test_list_touch_same_target_release_and_guards(void)
{
    egui_dim_t x1;
    egui_dim_t y1;
    egui_dim_t x2;
    egui_dim_t y2;

    setup_widget();
    layout_view(EGUI_VIEW_OF(&test_widget), 10, 20, 180, 112);
    get_item_center(EGUI_VIEW_OF(&test_widget), 1, &x1, &y1);
    get_item_center(EGUI_VIEW_OF(&test_widget), 2, &x2, &y2);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, x1, y1));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_MOVE, x2, y2));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_UP, x2, y2));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_reference_list_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, x1, y1));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_MOVE, x2, y2));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_MOVE, x1, y1));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_UP, x1, y1));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_reference_list_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, last_index);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1);
    egui_view_reference_list_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, x2, y2));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_UP, x2, y2));
    assert_pressed_cleared(&test_widget);

    egui_view_reference_list_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 0);
    seed_pressed_state(&test_widget, 1);
    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, x2, y2));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_UP, x2, y2));
    assert_pressed_cleared(&test_widget);
}

static void test_list_keyboard_navigation_enter_space_and_guards(void)
{
    setup_widget();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_UP));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_reference_list_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_reference_list_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, last_index);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_reference_list_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_reference_list_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_reference_list_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(4, changed_count);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1);
    egui_view_reference_list_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_DOWN));
    assert_pressed_cleared(&test_widget);

    egui_view_reference_list_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 0);
    seed_pressed_state(&test_widget, 1);
    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 0);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_DOWN));
    assert_pressed_cleared(&test_widget);
}

static void test_list_static_preview_consumes_input(void)
{
    egui_dim_t x;
    egui_dim_t y;
    uint8_t index_before;

    setup_preview_widget();
    layout_view(EGUI_VIEW_OF(&preview_widget), 10, 20, 104, 72);
    get_item_center(EGUI_VIEW_OF(&preview_widget), 2, &x, &y);
    index_before = egui_view_reference_list_get_current_index(EGUI_VIEW_OF(&preview_widget));

    seed_pressed_state(&preview_widget, 2);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_widget), EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_widget), EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(index_before, egui_view_reference_list_get_current_index(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    assert_pressed_cleared(&preview_widget);

    seed_pressed_state(&preview_widget, 2);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_widget), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(index_before, egui_view_reference_list_get_current_index(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    assert_pressed_cleared(&preview_widget);
}

void test_list_run(void)
{
    EGUI_TEST_SUITE_BEGIN(list);
    EGUI_TEST_RUN(test_list_setters_clamp_getters_and_helpers);
    EGUI_TEST_RUN(test_list_selection_regions_and_internal_helpers);
    EGUI_TEST_RUN(test_list_touch_same_target_release_and_guards);
    EGUI_TEST_RUN(test_list_keyboard_navigation_enter_space_and_guards);
    EGUI_TEST_RUN(test_list_static_preview_consumes_input);
    EGUI_TEST_SUITE_END();
}
