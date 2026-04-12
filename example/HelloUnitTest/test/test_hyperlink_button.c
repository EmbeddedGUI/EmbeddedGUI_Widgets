#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_hyperlink_button.h"

#include "../../HelloCustomWidgets/input/hyperlink_button/egui_view_hyperlink_button.h"
#include "../../HelloCustomWidgets/input/hyperlink_button/egui_view_hyperlink_button.c"

static egui_view_button_t test_button;
static egui_view_button_t preview_button;
static egui_view_api_t test_button_api;
static egui_view_api_t preview_button_api;
static uint8_t g_click_count;

static void on_button_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    g_click_count++;
}

static void setup_button(void)
{
    egui_view_button_init(EGUI_VIEW_OF(&test_button));
    egui_view_set_size(EGUI_VIEW_OF(&test_button), 132, 24);
    hcw_hyperlink_button_apply_standard_style(EGUI_VIEW_OF(&test_button));
    hcw_hyperlink_button_set_text(EGUI_VIEW_OF(&test_button), "Open release notes");
    hcw_hyperlink_button_set_font(EGUI_VIEW_OF(&test_button), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&test_button), on_button_click);
    hcw_hyperlink_button_override_interaction_api(EGUI_VIEW_OF(&test_button), &test_button_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&test_button), 1);
#endif
    g_click_count = 0;
}

static void setup_preview_button(void)
{
    egui_view_button_init(EGUI_VIEW_OF(&preview_button));
    egui_view_set_size(EGUI_VIEW_OF(&preview_button), 96, 24);
    hcw_hyperlink_button_apply_inline_style(EGUI_VIEW_OF(&preview_button));
    hcw_hyperlink_button_set_text(EGUI_VIEW_OF(&preview_button), "Inline article");
    hcw_hyperlink_button_set_font(EGUI_VIEW_OF(&preview_button), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_button), on_button_click);
    hcw_hyperlink_button_override_static_preview_api(EGUI_VIEW_OF(&preview_button), &preview_button_api);
    g_click_count = 0;
}

static void layout_button(egui_view_t *view, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(view, &region);
    egui_region_copy(&view->region_screen, &region);
}

static int send_touch(egui_view_button_t *button, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return EGUI_VIEW_OF(button)->api->on_touch_event(EGUI_VIEW_OF(button), &event);
}

static int send_key(egui_view_button_t *button, uint8_t key_code)
{
    egui_key_event_t event;
    int handled = 0;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    handled |= EGUI_VIEW_OF(button)->api->dispatch_key_event(EGUI_VIEW_OF(button), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    handled |= EGUI_VIEW_OF(button)->api->dispatch_key_event(EGUI_VIEW_OF(button), &event);
    return handled;
}

static int send_preview_key_action(egui_view_button_t *button, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return EGUI_VIEW_OF(button)->api->on_key_event(EGUI_VIEW_OF(button), &event);
}

static void get_button_center(egui_view_button_t *button, egui_dim_t *x, egui_dim_t *y)
{
    *x = EGUI_VIEW_OF(button)->region_screen.location.x + EGUI_VIEW_OF(button)->region_screen.size.width / 2;
    *y = EGUI_VIEW_OF(button)->region_screen.location.y + EGUI_VIEW_OF(button)->region_screen.size.height / 2;
}

static void test_hyperlink_button_style_helpers_update_background_and_clear_pressed_state(void)
{
    egui_view_button_t *local;

    setup_button();
    local = &test_button;

    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(local)->background == EGUI_BG_OF(&hcw_hyperlink_button_standard_background));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, local->base.color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, local->base.align_type);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    hcw_hyperlink_button_apply_inline_style(EGUI_VIEW_OF(local));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(local)->background == EGUI_BG_OF(&hcw_hyperlink_button_inline_background));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x24527A).full, local->base.color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    hcw_hyperlink_button_apply_disabled_style(EGUI_VIEW_OF(local));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(local)->background == EGUI_BG_OF(&hcw_hyperlink_button_disabled_background));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x8A96A3).full, local->base.color.full);
}

static void test_hyperlink_button_setters_clear_pressed_state_and_update_content(void)
{
    egui_view_button_t *local;

    setup_button();
    local = &test_button;

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    hcw_hyperlink_button_set_text(EGUI_VIEW_OF(local), "Review change summary");
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(strcmp(local->base.text, "Review change summary") == 0);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    hcw_hyperlink_button_set_font(EGUI_VIEW_OF(local), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->base.font == (const egui_font_t *)&egui_res_font_montserrat_12_4);
}

static void test_hyperlink_button_touch_same_target_release_clicks_once(void)
{
    egui_dim_t inside_x;
    egui_dim_t inside_y;
    egui_dim_t outside_x;

    setup_button();
    layout_button(EGUI_VIEW_OF(&test_button), 10, 20, 132, 24);
    get_button_center(&test_button, &inside_x, &inside_y);
    outside_x = EGUI_VIEW_OF(&test_button)->region_screen.location.x + EGUI_VIEW_OF(&test_button)->region_screen.size.width + 12;

    EGUI_TEST_ASSERT_TRUE(send_touch(&test_button, EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_button, EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_button, EGUI_MOTION_EVENT_ACTION_UP, outside_x, inside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    EGUI_TEST_ASSERT_TRUE(send_touch(&test_button, EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_button, EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_button, EGUI_MOTION_EVENT_ACTION_MOVE, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_button, EGUI_MOTION_EVENT_ACTION_UP, inside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_click_count);
}

static void test_hyperlink_button_keyboard_space_and_enter_click(void)
{
    setup_button();

    EGUI_TEST_ASSERT_TRUE(send_key(&test_button, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_click_count);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_key(&test_button, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_click_count);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
}

static void test_hyperlink_button_disabled_input_does_not_click(void)
{
    egui_dim_t inside_x;
    egui_dim_t inside_y;

    setup_button();
    layout_button(EGUI_VIEW_OF(&test_button), 10, 20, 132, 24);
    get_button_center(&test_button, &inside_x, &inside_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), 1);
    egui_view_set_enable(EGUI_VIEW_OF(&test_button), 0);
    EGUI_TEST_ASSERT_FALSE(send_key(&test_button, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_button, EGUI_MOTION_EVENT_ACTION_UP, inside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

static void test_hyperlink_button_unhandled_key_clears_pressed_state(void)
{
    setup_button();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(&test_button, EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

static void test_hyperlink_button_static_preview_consumes_input_and_clears_pressed_state(void)
{
    egui_dim_t inside_x;
    egui_dim_t inside_y;

    setup_preview_button();
    layout_button(EGUI_VIEW_OF(&preview_button), 10, 20, 96, 24);
    get_button_center(&preview_button, &inside_x, &inside_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_button), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(&preview_button, EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_button), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key_action(&preview_button, EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_preview_key_action(&preview_button, EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

void test_hyperlink_button_run(void)
{
    EGUI_TEST_SUITE_BEGIN(hyperlink_button);
    EGUI_TEST_RUN(test_hyperlink_button_style_helpers_update_background_and_clear_pressed_state);
    EGUI_TEST_RUN(test_hyperlink_button_setters_clear_pressed_state_and_update_content);
    EGUI_TEST_RUN(test_hyperlink_button_touch_same_target_release_clicks_once);
    EGUI_TEST_RUN(test_hyperlink_button_keyboard_space_and_enter_click);
    EGUI_TEST_RUN(test_hyperlink_button_disabled_input_does_not_click);
    EGUI_TEST_RUN(test_hyperlink_button_unhandled_key_clears_pressed_state);
    EGUI_TEST_RUN(test_hyperlink_button_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_SUITE_END();
}
