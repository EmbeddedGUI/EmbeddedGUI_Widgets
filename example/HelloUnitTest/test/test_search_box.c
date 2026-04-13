#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_search_box.h"

#include "../../HelloCustomWidgets/input/search_box/egui_view_search_box.h"
#include "../../HelloCustomWidgets/input/search_box/egui_view_search_box.c"

static egui_view_search_box_t test_search_box;
static egui_view_search_box_t preview_search_box;
static egui_view_api_t preview_api;
static char submitted_text[EGUI_CONFIG_TEXTINPUT_MAX_LENGTH + 1];
static uint8_t submit_count;

static void on_submit(egui_view_t *self, const char *text)
{
    EGUI_UNUSED(self);

    strncpy(submitted_text, text, sizeof(submitted_text) - 1);
    submitted_text[sizeof(submitted_text) - 1] = '\0';
    submit_count++;
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

static void setup_search_box(const char *text)
{
    egui_view_search_box_init(EGUI_VIEW_OF(&test_search_box));
    egui_view_set_size(EGUI_VIEW_OF(&test_search_box), 196, 40);
    egui_view_search_box_set_font(EGUI_VIEW_OF(&test_search_box), (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    egui_view_search_box_apply_standard_style(EGUI_VIEW_OF(&test_search_box));
    egui_view_search_box_set_text(EGUI_VIEW_OF(&test_search_box), text);
    egui_view_textinput_set_on_submit(EGUI_VIEW_OF(&test_search_box), on_submit);
    submitted_text[0] = '\0';
    submit_count = 0;
    layout_view(EGUI_VIEW_OF(&test_search_box), 10, 20, 196, 40);
}

static void setup_preview_search_box(const char *text)
{
    egui_view_search_box_init(EGUI_VIEW_OF(&preview_search_box));
    egui_view_set_size(EGUI_VIEW_OF(&preview_search_box), 104, 32);
    egui_view_search_box_set_font(EGUI_VIEW_OF(&preview_search_box), (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    egui_view_search_box_apply_compact_style(EGUI_VIEW_OF(&preview_search_box));
    egui_view_search_box_set_text(EGUI_VIEW_OF(&preview_search_box), text);
    egui_view_search_box_override_static_preview_api(EGUI_VIEW_OF(&preview_search_box), &preview_api);
    layout_view(EGUI_VIEW_OF(&preview_search_box), 12, 18, 104, 32);
}

static int send_key_event(uint8_t type, uint8_t key_code, uint8_t is_shift)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    event.is_shift = is_shift ? 1 : 0;
    return EGUI_VIEW_OF(&test_search_box)->api->on_key_event(EGUI_VIEW_OF(&test_search_box), &event);
}

static int send_preview_key_event(uint8_t type, uint8_t key_code, uint8_t is_shift)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    event.is_shift = is_shift ? 1 : 0;
    return EGUI_VIEW_OF(&preview_search_box)->api->on_key_event(EGUI_VIEW_OF(&preview_search_box), &event);
}

static int send_touch_to_view(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->dispatch_touch_event(view, &event);
}

static int send_preview_touch_event(uint8_t type)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_search_box), type, EGUI_VIEW_OF(&preview_search_box)->region_screen.location.x + 8,
                              EGUI_VIEW_OF(&preview_search_box)->region_screen.location.y + 8);
}

static int press_key(uint8_t key_code, uint8_t is_shift)
{
    int handled = 0;

    handled |= send_key_event(EGUI_KEY_EVENT_ACTION_DOWN, key_code, is_shift);
    handled |= send_key_event(EGUI_KEY_EVENT_ACTION_UP, key_code, is_shift);
    return handled;
}

static void test_search_box_style_helpers_apply_expected_state(void)
{
    egui_view_search_box_init(EGUI_VIEW_OF(&test_search_box));

    egui_view_search_box_apply_standard_style(EGUI_VIEW_OF(&test_search_box));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_search_box)->background != NULL);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_search_box)->is_enable);
#if EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    EGUI_TEST_ASSERT_EQUAL_INT(28, EGUI_VIEW_OF(&test_search_box)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(26, EGUI_VIEW_OF(&test_search_box)->padding.right);
#endif
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x5C6B79).full, test_search_box.icon_color.full);

    egui_view_search_box_apply_compact_style(EGUI_VIEW_OF(&test_search_box));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_search_box)->is_enable);
#if EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    EGUI_TEST_ASSERT_EQUAL_INT(24, EGUI_VIEW_OF(&test_search_box)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(22, EGUI_VIEW_OF(&test_search_box)->padding.right);
#endif
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x55716D).full, test_search_box.icon_color.full);

    egui_view_search_box_apply_read_only_style(EGUI_VIEW_OF(&test_search_box));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_search_box)->is_enable);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x8A97A5).full, test_search_box.icon_color.full);
}

static void test_search_box_insert_delete_and_submit_via_keyboard(void)
{
    setup_search_box("Roadmap");
    EGUI_TEST_ASSERT_TRUE(press_key(EGUI_KEY_CODE_BACKSPACE, 0));
    EGUI_TEST_ASSERT_TRUE(press_key(EGUI_KEY_CODE_S, 0));
    EGUI_TEST_ASSERT_TRUE(strcmp("Roadmas", egui_view_textinput_get_text(EGUI_VIEW_OF(&test_search_box))) == 0);
    EGUI_TEST_ASSERT_TRUE(press_key(EGUI_KEY_CODE_ENTER, 0));
    EGUI_TEST_ASSERT_EQUAL_INT(1, submit_count);
    EGUI_TEST_ASSERT_TRUE(strcmp("Roadmas", submitted_text) == 0);
}

static void test_search_box_clear_button_requires_same_target_release(void)
{
    egui_region_t clear_region;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_search_box("Assets");
    EGUI_TEST_ASSERT_TRUE(egui_view_search_box_get_clear_region(EGUI_VIEW_OF(&test_search_box), &clear_region));

    outside_x = clear_region.location.x - 6;
    outside_y = clear_region.location.y - 6;

    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_search_box), EGUI_MOTION_EVENT_ACTION_DOWN,
                                             clear_region.location.x + clear_region.size.width / 2,
                                             clear_region.location.y + clear_region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_search_box), EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_search_box), EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_TRUE(strcmp("Assets", egui_view_textinput_get_text(EGUI_VIEW_OF(&test_search_box))) == 0);

    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_search_box), EGUI_MOTION_EVENT_ACTION_DOWN,
                                             clear_region.location.x + clear_region.size.width / 2,
                                             clear_region.location.y + clear_region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_search_box), EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_search_box), EGUI_MOTION_EVENT_ACTION_MOVE,
                                             clear_region.location.x + clear_region.size.width / 2,
                                             clear_region.location.y + clear_region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_search_box), EGUI_MOTION_EVENT_ACTION_UP,
                                             clear_region.location.x + clear_region.size.width / 2,
                                             clear_region.location.y + clear_region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(strcmp("", egui_view_textinput_get_text(EGUI_VIEW_OF(&test_search_box))) == 0);
    EGUI_TEST_ASSERT_FALSE(test_search_box.clear_pressed);
    EGUI_TEST_ASSERT_FALSE(egui_view_search_box_get_clear_region(EGUI_VIEW_OF(&test_search_box), &clear_region));
}

static void test_search_box_read_only_style_blocks_input_and_hides_clear(void)
{
    egui_region_t clear_region;

    setup_search_box("Locked");
    egui_view_search_box_apply_read_only_style(EGUI_VIEW_OF(&test_search_box));
    EGUI_TEST_ASSERT_FALSE(press_key(EGUI_KEY_CODE_A, 0));
    EGUI_TEST_ASSERT_TRUE(strcmp("Locked", egui_view_textinput_get_text(EGUI_VIEW_OF(&test_search_box))) == 0);
    EGUI_TEST_ASSERT_FALSE(egui_view_search_box_get_clear_region(EGUI_VIEW_OF(&test_search_box), &clear_region));
}

static void test_search_box_static_preview_consumes_input(void)
{
    setup_preview_search_box("Assets");
    EGUI_TEST_ASSERT_TRUE(send_preview_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN));
    EGUI_TEST_ASSERT_TRUE(send_preview_touch_event(EGUI_MOTION_EVENT_ACTION_UP));
    EGUI_TEST_ASSERT_TRUE(send_preview_key_event(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_A, 0));
    EGUI_TEST_ASSERT_TRUE(send_preview_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_A, 0));
    EGUI_TEST_ASSERT_TRUE(strcmp("Assets", egui_view_textinput_get_text(EGUI_VIEW_OF(&preview_search_box))) == 0);
}

void test_search_box_run(void)
{
    EGUI_TEST_SUITE_BEGIN(search_box);
    EGUI_TEST_RUN(test_search_box_style_helpers_apply_expected_state);
    EGUI_TEST_RUN(test_search_box_insert_delete_and_submit_via_keyboard);
    EGUI_TEST_RUN(test_search_box_clear_button_requires_same_target_release);
    EGUI_TEST_RUN(test_search_box_read_only_style_blocks_input_and_hides_clear);
    EGUI_TEST_RUN(test_search_box_static_preview_consumes_input);
    EGUI_TEST_SUITE_END();
}
