#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_tab_strip.h"

#include "../../HelloCustomWidgets/navigation/tab_strip/egui_view_tab_strip.h"
#include "../../HelloCustomWidgets/navigation/tab_strip/egui_view_tab_strip.c"

static egui_view_tab_strip_t test_tab_strip;
static egui_view_tab_strip_t preview_tab_strip;
static egui_view_api_t preview_api;
static uint8_t g_listener_index;
static uint8_t g_listener_count;

static const char *g_tabs_primary[] = {"Overview", "Usage", "Access"};
static const char *g_tabs_preview[] = {"Home", "Logs"};
static const char *g_tabs_overflow[] = {"One", "Two", "Three", "Four", "Five", "Six", "Seven"};
static const char *g_tabs_long[] = {"Documents", "Operations", "Administration"};

static void on_tab_changed(egui_view_t *self, uint8_t index)
{
    EGUI_UNUSED(self);
    g_listener_index = index;
    g_listener_count++;
}

static void reset_listener_state(void)
{
    g_listener_index = 0xFF;
    g_listener_count = 0;
}

static void setup_widget(void)
{
    egui_view_tab_strip_init(EGUI_VIEW_OF(&test_tab_strip));
    egui_view_set_size(EGUI_VIEW_OF(&test_tab_strip), 196, 48);
    egui_view_tab_strip_set_tabs(EGUI_VIEW_OF(&test_tab_strip), g_tabs_primary, 3);
    egui_view_tab_strip_set_on_tab_changed_listener(EGUI_VIEW_OF(&test_tab_strip), on_tab_changed);
    reset_listener_state();
}

static void setup_preview_widget(void)
{
    egui_view_tab_strip_init(EGUI_VIEW_OF(&preview_tab_strip));
    egui_view_set_size(EGUI_VIEW_OF(&preview_tab_strip), 104, 36);
    egui_view_tab_strip_set_tabs(EGUI_VIEW_OF(&preview_tab_strip), g_tabs_preview, 2);
    egui_view_tab_strip_set_current_index(EGUI_VIEW_OF(&preview_tab_strip), 1);
    egui_view_tab_strip_set_compact_mode(EGUI_VIEW_OF(&preview_tab_strip), 1);
    egui_view_tab_strip_set_on_tab_changed_listener(EGUI_VIEW_OF(&preview_tab_strip), on_tab_changed);
    egui_view_tab_strip_override_static_preview_api(EGUI_VIEW_OF(&preview_tab_strip), &preview_api);
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

static void layout_widget(egui_dim_t width)
{
    layout_view(EGUI_VIEW_OF(&test_tab_strip), 10, 20, width, 48);
}

static void layout_preview_widget(void)
{
    layout_view(EGUI_VIEW_OF(&preview_tab_strip), 12, 18, 104, 36);
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

static int send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&test_tab_strip), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_tab_strip), type, x, y);
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_tab_strip), key_code);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_tab_strip), key_code);
}

static uint8_t get_item_center_for_strip(egui_view_tab_strip_t *strip, uint8_t item_index, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t work_region;
    egui_view_tab_strip_layout_item_t items[EGUI_VIEW_TAB_STRIP_MAX_TABS];
    egui_dim_t content_x;
    egui_dim_t content_w;
    uint8_t count;

    egui_view_get_work_region(EGUI_VIEW_OF(strip), &work_region);
    content_x = work_region.location.x +
                (strip->compact_mode ? EGUI_VIEW_TAB_STRIP_COMPACT_CONTENT_PAD_X : EGUI_VIEW_TAB_STRIP_STANDARD_CONTENT_PAD_X);
    content_w = work_region.size.width -
                (strip->compact_mode ? EGUI_VIEW_TAB_STRIP_COMPACT_CONTENT_PAD_X * 2 : EGUI_VIEW_TAB_STRIP_STANDARD_CONTENT_PAD_X * 2);
    if (content_w <= 0)
    {
        return 0;
    }

    count = egui_view_tab_strip_prepare_layout(strip, content_x, content_w, items);
    if (item_index >= count)
    {
        return 0;
    }

    *x = items[item_index].x + items[item_index].width / 2;
    *y = work_region.location.y + work_region.size.height / 2;
    return 1;
}

static void seed_pressed_state(egui_view_tab_strip_t *strip, uint8_t index, uint8_t visual_pressed)
{
    egui_view_set_pressed(EGUI_VIEW_OF(strip), 0);
    strip->pressed_index = index;
    if (visual_pressed)
    {
        egui_view_set_pressed(EGUI_VIEW_OF(strip), 1);
    }
}

static void assert_pressed_cleared(egui_view_tab_strip_t *strip)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_STRIP_INDEX_NONE, strip->pressed_index);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(strip)->is_pressed);
}

static void test_tab_strip_set_tabs_clamps_and_clears_pressed_state(void)
{
    setup_widget();

    test_tab_strip.current_index = 6;
    seed_pressed_state(&test_tab_strip, 4, 1);
    egui_view_tab_strip_set_tabs(EGUI_VIEW_OF(&test_tab_strip), g_tabs_overflow, 7);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_STRIP_MAX_TABS, test_tab_strip.tab_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tab_strip_get_current_index(EGUI_VIEW_OF(&test_tab_strip)));
    assert_pressed_cleared(&test_tab_strip);

    test_tab_strip.current_index = 2;
    seed_pressed_state(&test_tab_strip, 2, 1);
    egui_view_tab_strip_set_tabs(EGUI_VIEW_OF(&test_tab_strip), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_tab_strip.tab_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tab_strip_get_current_index(EGUI_VIEW_OF(&test_tab_strip)));
    assert_pressed_cleared(&test_tab_strip);
}

static void test_tab_strip_current_index_and_setters_clear_pressed_state(void)
{
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);
    egui_view_tab_strip_layout_item_t items[EGUI_VIEW_TAB_STRIP_MAX_TABS];
    egui_dim_t width_standard;
    egui_dim_t width_compact;
    uint8_t count;

    setup_widget();

    egui_view_tab_strip_set_current_index(EGUI_VIEW_OF(&test_tab_strip), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_tab_strip_get_current_index(EGUI_VIEW_OF(&test_tab_strip)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_listener_index);

    seed_pressed_state(&test_tab_strip, 2, 1);
    egui_view_tab_strip_set_current_index(EGUI_VIEW_OF(&test_tab_strip), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);
    assert_pressed_cleared(&test_tab_strip);

    seed_pressed_state(&test_tab_strip, 1, 1);
    egui_view_tab_strip_set_font(EGUI_VIEW_OF(&test_tab_strip), NULL);
    assert_pressed_cleared(&test_tab_strip);
    EGUI_TEST_ASSERT_TRUE(test_tab_strip.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(&test_tab_strip, 1, 0);
    egui_view_tab_strip_set_palette(EGUI_VIEW_OF(&test_tab_strip), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                    EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152));
    assert_pressed_cleared(&test_tab_strip);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_tab_strip.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, test_tab_strip.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, test_tab_strip.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x404142).full, test_tab_strip.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x505152).full, test_tab_strip.accent_color.full);

    seed_pressed_state(&test_tab_strip, 1, 1);
    egui_view_tab_strip_set_compact_mode(EGUI_VIEW_OF(&test_tab_strip), 2);
    assert_pressed_cleared(&test_tab_strip);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_tab_strip.compact_mode);

    seed_pressed_state(&test_tab_strip, 1, 0);
    egui_view_tab_strip_set_read_only_mode(EGUI_VIEW_OF(&test_tab_strip), 3);
    assert_pressed_cleared(&test_tab_strip);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_tab_strip.read_only_mode);

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_STRIP_MAX_TABS, egui_view_tab_strip_clamp_tab_count(9));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tab_strip_text_len(NULL));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_tab_strip_text_len("Usage"));
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_DARK_GREY, 64).full, egui_view_tab_strip_mix_disabled(sample).full);

    memset(items, 0, sizeof(items));
    egui_view_tab_strip_copy_elided(items[0].label, sizeof(items[0].label), "Documents", 6);
    EGUI_TEST_ASSERT_TRUE(strcmp("Doc...", items[0].label) == 0);
    egui_view_tab_strip_copy_elided(items[1].label, sizeof(items[1].label), "Usage", 3);
    EGUI_TEST_ASSERT_TRUE(strcmp("...", items[1].label) == 0);

    width_standard = egui_view_tab_strip_measure_tab_width(0, 1, "Overview");
    width_compact = egui_view_tab_strip_measure_tab_width(1, 0, "Overview");
    EGUI_TEST_ASSERT_TRUE(width_standard >= EGUI_VIEW_TAB_STRIP_STANDARD_MIN_WIDTH);
    EGUI_TEST_ASSERT_TRUE(width_compact >= EGUI_VIEW_TAB_STRIP_COMPACT_MIN_WIDTH);
    EGUI_TEST_ASSERT_TRUE(width_standard > width_compact);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_STRIP_COMPACT_GAP, egui_view_tab_strip_item_gap(1));

    egui_view_tab_strip_set_compact_mode(EGUI_VIEW_OF(&test_tab_strip), 0);
    egui_view_tab_strip_set_read_only_mode(EGUI_VIEW_OF(&test_tab_strip), 0);
    egui_view_tab_strip_set_tabs(EGUI_VIEW_OF(&test_tab_strip), g_tabs_long, 3);
    count = egui_view_tab_strip_prepare_layout(&test_tab_strip, 0, 72, items);
    EGUI_TEST_ASSERT_EQUAL_INT(3, count);
    EGUI_TEST_ASSERT_TRUE(items[0].width == items[1].width);
    EGUI_TEST_ASSERT_TRUE(strcmp(items[0].label, g_tabs_long[0]) != 0);
    EGUI_TEST_ASSERT_TRUE(strcmp(items[1].label, g_tabs_long[1]) != 0);
    EGUI_TEST_ASSERT_TRUE(items[1].x > items[0].x);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_STRIP_INDEX_NONE, egui_view_tab_strip_resolve_hit(&test_tab_strip, EGUI_VIEW_OF(&test_tab_strip), 4));
}

static void test_tab_strip_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t x1;
    egui_dim_t y1;
    egui_dim_t x2;
    egui_dim_t y2;

    setup_widget();
    layout_widget(196);
    EGUI_TEST_ASSERT_TRUE(get_item_center_for_strip(&test_tab_strip, 1, &x1, &y1));
    EGUI_TEST_ASSERT_TRUE(get_item_center_for_strip(&test_tab_strip, 2, &x2, &y2));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x1, y1));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_tab_strip.pressed_index);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tab_strip)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, x2, y2));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_tab_strip.pressed_index);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tab_strip)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x2, y2));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tab_strip_get_current_index(EGUI_VIEW_OF(&test_tab_strip)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_listener_count);
    assert_pressed_cleared(&test_tab_strip);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x1, y1));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, x2, y2));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tab_strip)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, x1, y1));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tab_strip)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x1, y1));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tab_strip_get_current_index(EGUI_VIEW_OF(&test_tab_strip)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_index);
    assert_pressed_cleared(&test_tab_strip);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x2, y2));
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_tab_strip.pressed_index);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, x2, y2));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tab_strip_get_current_index(EGUI_VIEW_OF(&test_tab_strip)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);
    assert_pressed_cleared(&test_tab_strip);
}

static void test_tab_strip_keyboard_navigation(void)
{
    setup_widget();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tab_strip_get_current_index(EGUI_VIEW_OF(&test_tab_strip)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_tab_strip_get_current_index(EGUI_VIEW_OF(&test_tab_strip)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tab_strip_get_current_index(EGUI_VIEW_OF(&test_tab_strip)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tab_strip_get_current_index(EGUI_VIEW_OF(&test_tab_strip)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, g_listener_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_listener_index);
}

static void test_tab_strip_compact_mode_clears_pressed_and_keeps_selection_behavior(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_widget();
    egui_view_tab_strip_set_current_index(EGUI_VIEW_OF(&test_tab_strip), 1);
    reset_listener_state();
    layout_widget(196);
    EGUI_TEST_ASSERT_TRUE(get_item_center_for_strip(&test_tab_strip, 2, &x, &y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_tab_strip.pressed_index);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tab_strip)->is_pressed);

    egui_view_tab_strip_set_compact_mode(EGUI_VIEW_OF(&test_tab_strip), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_tab_strip.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tab_strip_get_current_index(EGUI_VIEW_OF(&test_tab_strip)));
    assert_pressed_cleared(&test_tab_strip);

    layout_widget(196);
    EGUI_TEST_ASSERT_TRUE(get_item_center_for_strip(&test_tab_strip, 0, &x, &y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tab_strip_get_current_index(EGUI_VIEW_OF(&test_tab_strip)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_listener_index);
}

static void test_tab_strip_read_only_and_view_disabled_guards_clear_pressed_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_widget();
    layout_widget(196);
    EGUI_TEST_ASSERT_TRUE(get_item_center_for_strip(&test_tab_strip, 2, &x, &y));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tab_strip)->is_pressed);
    egui_view_tab_strip_set_read_only_mode(EGUI_VIEW_OF(&test_tab_strip), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_tab_strip.read_only_mode);
    assert_pressed_cleared(&test_tab_strip);

    seed_pressed_state(&test_tab_strip, 1, 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(&test_tab_strip);

    seed_pressed_state(&test_tab_strip, 1, 0);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_END));
    assert_pressed_cleared(&test_tab_strip);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tab_strip_get_current_index(EGUI_VIEW_OF(&test_tab_strip)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_listener_count);

    egui_view_tab_strip_set_read_only_mode(EGUI_VIEW_OF(&test_tab_strip), 0);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_tab_strip_get_current_index(EGUI_VIEW_OF(&test_tab_strip)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_listener_index);

    setup_widget();
    layout_widget(196);
    EGUI_TEST_ASSERT_TRUE(get_item_center_for_strip(&test_tab_strip, 2, &x, &y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tab_strip)->is_pressed);
    egui_view_set_enable(EGUI_VIEW_OF(&test_tab_strip), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(&test_tab_strip);

    seed_pressed_state(&test_tab_strip, 1, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_END));
    assert_pressed_cleared(&test_tab_strip);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tab_strip_get_current_index(EGUI_VIEW_OF(&test_tab_strip)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_listener_count);

    egui_view_set_enable(EGUI_VIEW_OF(&test_tab_strip), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_tab_strip_get_current_index(EGUI_VIEW_OF(&test_tab_strip)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_listener_index);
}

static void test_tab_strip_static_preview_consumes_input_and_clears_pressed_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_widget();
    layout_preview_widget();
    EGUI_TEST_ASSERT_TRUE(get_item_center_for_strip(&preview_tab_strip, 1, &x, &y));

    seed_pressed_state(&preview_tab_strip, 1, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(&preview_tab_strip);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tab_strip_get_current_index(EGUI_VIEW_OF(&preview_tab_strip)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_listener_count);

    seed_pressed_state(&preview_tab_strip, 1, 0);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_END));
    assert_pressed_cleared(&preview_tab_strip);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tab_strip_get_current_index(EGUI_VIEW_OF(&preview_tab_strip)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_listener_count);
}

void test_tab_strip_run(void)
{
    EGUI_TEST_SUITE_BEGIN(tab_strip);
    EGUI_TEST_RUN(test_tab_strip_set_tabs_clamps_and_clears_pressed_state);
    EGUI_TEST_RUN(test_tab_strip_current_index_and_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_tab_strip_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_tab_strip_keyboard_navigation);
    EGUI_TEST_RUN(test_tab_strip_compact_mode_clears_pressed_and_keeps_selection_behavior);
    EGUI_TEST_RUN(test_tab_strip_read_only_and_view_disabled_guards_clear_pressed_state);
    EGUI_TEST_RUN(test_tab_strip_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_SUITE_END();
}
