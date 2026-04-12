#include <string.h>

#include "egui.h"
#include "resource/egui_icon_material_symbols.h"
#include "test/egui_test.h"
#include "test_selector_bar.h"

#include "../../HelloCustomWidgets/navigation/selector_bar/egui_view_selector_bar.h"
#include "../../HelloCustomWidgets/navigation/selector_bar/egui_view_selector_bar.c"

static egui_view_selector_bar_t test_selector_bar;
static egui_view_selector_bar_t preview_selector_bar;
static egui_view_api_t preview_api;
static uint8_t g_listener_index;
static uint8_t g_listener_count;

static const char *g_items_primary[] = {"Recent", "Search", "Saved"};
static const char *g_icons_primary[] = {EGUI_ICON_MS_SCHEDULE, EGUI_ICON_MS_SEARCH, EGUI_ICON_MS_FAVORITE};
static const char *g_icons_preview[] = {EGUI_ICON_MS_HOME, EGUI_ICON_MS_SEARCH, EGUI_ICON_MS_SETTINGS};
static const char *g_items_overflow[] = {"One", "Two", "Three", "Four", "Five", "Six", "Seven"};
static const char *g_icons_overflow[] = {EGUI_ICON_MS_HOME, EGUI_ICON_MS_SEARCH, EGUI_ICON_MS_SETTINGS, EGUI_ICON_MS_HOME, EGUI_ICON_MS_SEARCH,
                                         EGUI_ICON_MS_SETTINGS, EGUI_ICON_MS_HOME};

static void on_selector_changed(egui_view_t *self, uint8_t index)
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
    egui_view_selector_bar_init(EGUI_VIEW_OF(&test_selector_bar));
    egui_view_set_size(EGUI_VIEW_OF(&test_selector_bar), 196, 56);
    egui_view_selector_bar_set_items(EGUI_VIEW_OF(&test_selector_bar), g_items_primary, g_icons_primary, 3);
    egui_view_selector_bar_set_font(EGUI_VIEW_OF(&test_selector_bar), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_selector_bar_set_on_selection_changed_listener(EGUI_VIEW_OF(&test_selector_bar), on_selector_changed);
    egui_view_selector_bar_set_current_index(EGUI_VIEW_OF(&test_selector_bar), 0);
    reset_listener_state();
}

static void setup_preview_widget(void)
{
    egui_view_selector_bar_init(EGUI_VIEW_OF(&preview_selector_bar));
    egui_view_set_size(EGUI_VIEW_OF(&preview_selector_bar), 84, 42);
    egui_view_selector_bar_set_items(EGUI_VIEW_OF(&preview_selector_bar), NULL, g_icons_preview, 3);
    egui_view_selector_bar_set_compact_mode(EGUI_VIEW_OF(&preview_selector_bar), 1);
    egui_view_selector_bar_set_current_index(EGUI_VIEW_OF(&preview_selector_bar), 1);
    egui_view_selector_bar_set_on_selection_changed_listener(EGUI_VIEW_OF(&preview_selector_bar), on_selector_changed);
    egui_view_selector_bar_override_static_preview_api(EGUI_VIEW_OF(&preview_selector_bar), &preview_api);
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

static void layout_widget(void)
{
    layout_view(EGUI_VIEW_OF(&test_selector_bar), 10, 20, 196, 56);
}

static void layout_preview_widget(void)
{
    layout_view(EGUI_VIEW_OF(&preview_selector_bar), 12, 18, 84, 42);
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
    return send_touch_to_view(EGUI_VIEW_OF(&test_selector_bar), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_selector_bar), type, x, y);
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_selector_bar), key_code);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_selector_bar), key_code);
}

static uint8_t get_item_center_for_bar(egui_view_selector_bar_t *bar, uint8_t item_index, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t work_region;
    egui_view_selector_bar_layout_item_t items[EGUI_VIEW_SELECTOR_BAR_MAX_ITEMS];
    egui_dim_t content_x;
    egui_dim_t content_w;
    uint8_t count;

    egui_view_get_work_region(EGUI_VIEW_OF(bar), &work_region);
    content_x = work_region.location.x +
                (bar->compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_CONTENT_PAD_X : EGUI_VIEW_SELECTOR_BAR_STANDARD_CONTENT_PAD_X);
    content_w = work_region.size.width -
                (bar->compact_mode ? EGUI_VIEW_SELECTOR_BAR_COMPACT_CONTENT_PAD_X * 2 : EGUI_VIEW_SELECTOR_BAR_STANDARD_CONTENT_PAD_X * 2);
    if (content_w <= 0)
    {
        return 0;
    }

    count = egui_view_selector_bar_prepare_layout(bar, content_x, content_w, items);
    if (item_index >= count)
    {
        return 0;
    }

    *x = items[item_index].x + items[item_index].width / 2;
    *y = work_region.location.y + work_region.size.height / 2;
    return 1;
}

static void seed_pressed_state(egui_view_selector_bar_t *bar, uint8_t index, uint8_t visual_pressed)
{
    egui_view_set_pressed(EGUI_VIEW_OF(bar), 0);
    bar->pressed_index = index;
    if (visual_pressed)
    {
        egui_view_set_pressed(EGUI_VIEW_OF(bar), 1);
    }
}

static void assert_pressed_cleared(egui_view_selector_bar_t *bar)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SELECTOR_BAR_INDEX_NONE, bar->pressed_index);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(bar)->is_pressed);
}

static void test_selector_bar_set_items_and_setters_clear_pressed_state(void)
{
    setup_widget();

    test_selector_bar.current_index = EGUI_VIEW_SELECTOR_BAR_INDEX_NONE;
    seed_pressed_state(&test_selector_bar, 4, 1);
    egui_view_selector_bar_set_items(EGUI_VIEW_OF(&test_selector_bar), g_items_overflow, g_icons_overflow, 7);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SELECTOR_BAR_MAX_ITEMS, test_selector_bar.item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SELECTOR_BAR_INDEX_NONE, egui_view_selector_bar_get_current_index(EGUI_VIEW_OF(&test_selector_bar)));
    assert_pressed_cleared(&test_selector_bar);

    egui_view_selector_bar_set_current_index(EGUI_VIEW_OF(&test_selector_bar), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_selector_bar_get_current_index(EGUI_VIEW_OF(&test_selector_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_listener_index);

    seed_pressed_state(&test_selector_bar, 2, 1);
    egui_view_selector_bar_set_current_index(EGUI_VIEW_OF(&test_selector_bar), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);
    assert_pressed_cleared(&test_selector_bar);

    seed_pressed_state(&test_selector_bar, 1, 1);
    egui_view_selector_bar_set_font(EGUI_VIEW_OF(&test_selector_bar), NULL);
    assert_pressed_cleared(&test_selector_bar);
    EGUI_TEST_ASSERT_TRUE(test_selector_bar.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(&test_selector_bar, 1, 0);
    egui_view_selector_bar_set_icon_font(EGUI_VIEW_OF(&test_selector_bar), EGUI_FONT_ICON_MS_16);
    assert_pressed_cleared(&test_selector_bar);
    EGUI_TEST_ASSERT_TRUE(test_selector_bar.icon_font == EGUI_FONT_ICON_MS_16);

    seed_pressed_state(&test_selector_bar, 1, 1);
    egui_view_selector_bar_set_compact_mode(EGUI_VIEW_OF(&test_selector_bar), 2);
    assert_pressed_cleared(&test_selector_bar);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_selector_bar.compact_mode);

    seed_pressed_state(&test_selector_bar, 1, 0);
    egui_view_selector_bar_set_palette(EGUI_VIEW_OF(&test_selector_bar), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                       EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152));
    assert_pressed_cleared(&test_selector_bar);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_selector_bar.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, test_selector_bar.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, test_selector_bar.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x404142).full, test_selector_bar.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x505152).full, test_selector_bar.accent_color.full);

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SELECTOR_BAR_MAX_ITEMS, egui_view_selector_bar_clamp_count(7));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_selector_bar_text_len(NULL));
    EGUI_TEST_ASSERT_EQUAL_INT(6, egui_view_selector_bar_text_len("Recent"));
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(EGUI_COLOR_HEX(0x123456), EGUI_COLOR_DARK_GREY, 64).full,
                               egui_view_selector_bar_mix_disabled(EGUI_COLOR_HEX(0x123456)).full);
}

static void test_selector_bar_focus_and_keyboard_navigation(void)
{
    setup_widget();

    egui_view_selector_bar_set_current_index(EGUI_VIEW_OF(&test_selector_bar), EGUI_VIEW_SELECTOR_BAR_INDEX_NONE);
    reset_listener_state();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    EGUI_VIEW_OF(&test_selector_bar)->api->on_focus_changed(EGUI_VIEW_OF(&test_selector_bar), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_selector_bar_get_current_index(EGUI_VIEW_OF(&test_selector_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_listener_index);
    reset_listener_state();
#endif

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_selector_bar_get_current_index(EGUI_VIEW_OF(&test_selector_bar)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_selector_bar_get_current_index(EGUI_VIEW_OF(&test_selector_bar)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_selector_bar_get_current_index(EGUI_VIEW_OF(&test_selector_bar)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_selector_bar_get_current_index(EGUI_VIEW_OF(&test_selector_bar)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_selector_bar_get_current_index(EGUI_VIEW_OF(&test_selector_bar)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(5, g_listener_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_index);
}

static void test_selector_bar_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t x1;
    egui_dim_t y1;
    egui_dim_t x2;
    egui_dim_t y2;

    setup_widget();
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_item_center_for_bar(&test_selector_bar, 1, &x1, &y1));
    EGUI_TEST_ASSERT_TRUE(get_item_center_for_bar(&test_selector_bar, 2, &x2, &y2));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x1, y1));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_selector_bar.pressed_index);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_selector_bar)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, x2, y2));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_selector_bar)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x2, y2));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_selector_bar_get_current_index(EGUI_VIEW_OF(&test_selector_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_listener_count);
    assert_pressed_cleared(&test_selector_bar);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x1, y1));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, x2, y2));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_selector_bar)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, x1, y1));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_selector_bar)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x1, y1));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_selector_bar_get_current_index(EGUI_VIEW_OF(&test_selector_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_index);
    assert_pressed_cleared(&test_selector_bar);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x2, y2));
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_selector_bar.pressed_index);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, x2, y2));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_selector_bar_get_current_index(EGUI_VIEW_OF(&test_selector_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);
    assert_pressed_cleared(&test_selector_bar);
}

static void test_selector_bar_static_preview_consumes_input_and_preserves_selection(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_widget();
    layout_preview_widget();
    EGUI_TEST_ASSERT_TRUE(get_item_center_for_bar(&preview_selector_bar, 1, &x, &y));

    seed_pressed_state(&preview_selector_bar, 1, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(&preview_selector_bar);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_selector_bar_get_current_index(EGUI_VIEW_OF(&preview_selector_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_listener_count);

    seed_pressed_state(&preview_selector_bar, 1, 0);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_END));
    assert_pressed_cleared(&preview_selector_bar);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_selector_bar_get_current_index(EGUI_VIEW_OF(&preview_selector_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_listener_count);
}

void test_selector_bar_run(void)
{
    EGUI_TEST_SUITE_BEGIN(selector_bar);
    EGUI_TEST_RUN(test_selector_bar_set_items_and_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_selector_bar_focus_and_keyboard_navigation);
    EGUI_TEST_RUN(test_selector_bar_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_selector_bar_static_preview_consumes_input_and_preserves_selection);
    EGUI_TEST_SUITE_END();
}
