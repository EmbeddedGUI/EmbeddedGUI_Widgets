#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_radio_button.h"

#include "../../HelloCustomWidgets/input/radio_button/egui_view_radio_button.h"
#include "../../HelloCustomWidgets/input/radio_button/egui_view_radio_button.c"

static egui_view_radio_group_t test_group;
static egui_view_radio_button_t test_buttons[2];
static egui_view_api_t test_button_api[2];
static egui_view_radio_group_t preview_group;
static egui_view_radio_button_t preview_buttons[2];
static egui_view_api_t preview_button_api[2];
static uint8_t g_changed_count;
static int g_last_index;

static void on_group_changed(egui_view_t *self, int index)
{
    EGUI_UNUSED(self);
    g_changed_count++;
    g_last_index = index;
}

static void reset_listener_state(void)
{
    g_changed_count = 0;
    g_last_index = -1;
}

static void setup_group(uint8_t selected_index)
{
    uint8_t i;
    static const char *texts[] = {"Home", "Alerts"};

    egui_view_radio_group_init(&test_group);
    egui_view_radio_group_set_on_changed_listener(&test_group, on_group_changed);
    for (i = 0; i < EGUI_ARRAY_SIZE(test_buttons); ++i)
    {
        egui_view_radio_button_init(EGUI_VIEW_OF(&test_buttons[i]));
        egui_view_set_size(EGUI_VIEW_OF(&test_buttons[i]), 180, 34);
        hcw_radio_button_set_font(EGUI_VIEW_OF(&test_buttons[i]), (const egui_font_t *)&egui_res_font_montserrat_10_4);
        hcw_radio_button_apply_standard_style(EGUI_VIEW_OF(&test_buttons[i]));
        hcw_radio_button_set_text(EGUI_VIEW_OF(&test_buttons[i]), texts[i]);
        hcw_radio_button_override_interaction_api(EGUI_VIEW_OF(&test_buttons[i]), &test_button_api[i]);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        egui_view_set_focusable(EGUI_VIEW_OF(&test_buttons[i]), 1);
#endif
        egui_view_radio_group_add(&test_group, EGUI_VIEW_OF(&test_buttons[i]));
    }

    if (selected_index < EGUI_ARRAY_SIZE(test_buttons))
    {
        hcw_radio_button_set_checked(EGUI_VIEW_OF(&test_buttons[selected_index]), 1);
    }
    reset_listener_state();
}

static void setup_preview_group(uint8_t selected_index)
{
    uint8_t i;
    static const char *texts[] = {"Auto", "Manual"};

    egui_view_radio_group_init(&preview_group);
    for (i = 0; i < EGUI_ARRAY_SIZE(preview_buttons); ++i)
    {
        egui_view_radio_button_init(EGUI_VIEW_OF(&preview_buttons[i]));
        egui_view_set_size(EGUI_VIEW_OF(&preview_buttons[i]), 104, 24);
        hcw_radio_button_set_font(EGUI_VIEW_OF(&preview_buttons[i]), (const egui_font_t *)&egui_res_font_montserrat_10_4);
        hcw_radio_button_apply_compact_style(EGUI_VIEW_OF(&preview_buttons[i]));
        hcw_radio_button_set_text(EGUI_VIEW_OF(&preview_buttons[i]), texts[i]);
        hcw_radio_button_override_static_preview_api(EGUI_VIEW_OF(&preview_buttons[i]), &preview_button_api[i]);
        egui_view_radio_group_add(&preview_group, EGUI_VIEW_OF(&preview_buttons[i]));
    }

    if (selected_index < EGUI_ARRAY_SIZE(preview_buttons))
    {
        hcw_radio_button_set_checked(EGUI_VIEW_OF(&preview_buttons[selected_index]), 1);
    }
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

static int send_touch(egui_view_radio_button_t *button, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return EGUI_VIEW_OF(button)->api->on_touch_event(EGUI_VIEW_OF(button), &event);
}

static int send_key(egui_view_radio_button_t *button, uint8_t key_code)
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

static int send_preview_key_action(egui_view_radio_button_t *button, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return EGUI_VIEW_OF(button)->api->on_key_event(EGUI_VIEW_OF(button), &event);
}

static void get_indicator_center(egui_view_radio_button_t *button, egui_dim_t *x, egui_dim_t *y)
{
    *x = EGUI_VIEW_OF(button)->region_screen.location.x + EGUI_VIEW_OF(button)->region_screen.size.height / 2;
    *y = EGUI_VIEW_OF(button)->region_screen.location.y + EGUI_VIEW_OF(button)->region_screen.size.height / 2;
}

static void test_radio_button_style_helpers_update_palette_and_clear_pressed_state(void)
{
    egui_view_radio_button_t *local;

    setup_group(0);
    local = &test_buttons[0];

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    hcw_radio_button_apply_compact_style(EGUI_VIEW_OF(local));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xC7D8CE).full, local->circle_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0C7C73).full, local->dot_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x21303F).full, local->text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(6, local->text_gap);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    hcw_radio_button_apply_read_only_style(EGUI_VIEW_OF(local));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xD8E0E8).full, local->circle_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xAFB8C3).full, local->dot_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x546474).full, local->text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RADIO_BUTTON_MARK_STYLE_DOT, local->mark_style);
}

static void test_radio_button_text_and_icon_setters_clear_pressed_state(void)
{
    egui_view_radio_button_t *local;

    setup_group(0);
    local = &test_buttons[0];

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    hcw_radio_button_set_text(EGUI_VIEW_OF(local), "Offline");
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(strcmp("Offline", local->text) == 0);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    hcw_radio_button_set_font(EGUI_VIEW_OF(local), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->font == (const egui_font_t *)&egui_res_font_montserrat_12_4);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    hcw_radio_button_set_text_color(EGUI_VIEW_OF(local), EGUI_COLOR_HEX(0x345678));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x345678).full, local->text_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    hcw_radio_button_set_mark_style(EGUI_VIEW_OF(local), EGUI_VIEW_RADIO_BUTTON_MARK_STYLE_ICON);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RADIO_BUTTON_MARK_STYLE_ICON, local->mark_style);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    hcw_radio_button_set_mark_icon(EGUI_VIEW_OF(local), "X");
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(strcmp("X", local->mark_icon) == 0);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    hcw_radio_button_set_icon_font(EGUI_VIEW_OF(local), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->icon_font == (const egui_font_t *)&egui_res_font_montserrat_10_4);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    hcw_radio_button_set_icon_text_gap(EGUI_VIEW_OF(local), 10);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(10, local->text_gap);
}

static void test_radio_button_checked_setter_updates_group_and_clears_pressed_state(void)
{
    setup_group(0);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_buttons[1]), 1);
    hcw_radio_button_set_checked(EGUI_VIEW_OF(&test_buttons[1]), 1);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_buttons[1])->is_pressed);
    EGUI_TEST_ASSERT_FALSE(test_buttons[0].is_checked);
    EGUI_TEST_ASSERT_TRUE(test_buttons[1].is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_last_index);

    hcw_radio_button_set_checked(EGUI_VIEW_OF(&test_buttons[1]), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_changed_count);
}

static void test_radio_button_touch_same_target_release_selects_once(void)
{
    egui_dim_t inside_x;
    egui_dim_t inside_y;
    egui_dim_t outside_x;

    setup_group(0);
    layout_button(EGUI_VIEW_OF(&test_buttons[0]), 10, 20, 180, 34);
    layout_button(EGUI_VIEW_OF(&test_buttons[1]), 10, 60, 180, 34);
    get_indicator_center(&test_buttons[1], &inside_x, &inside_y);
    outside_x = EGUI_VIEW_OF(&test_buttons[1])->region_screen.location.x + EGUI_VIEW_OF(&test_buttons[1])->region_screen.size.width + 12;

    EGUI_TEST_ASSERT_TRUE(send_touch(&test_buttons[1], EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_buttons[1])->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_buttons[1], EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_buttons[1])->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_buttons[1], EGUI_MOTION_EVENT_ACTION_UP, outside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(test_buttons[0].is_checked);
    EGUI_TEST_ASSERT_FALSE(test_buttons[1].is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);

    EGUI_TEST_ASSERT_TRUE(send_touch(&test_buttons[1], EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_buttons[1], EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_buttons[1], EGUI_MOTION_EVENT_ACTION_MOVE, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_buttons[1])->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_buttons[1], EGUI_MOTION_EVENT_ACTION_UP, inside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_buttons[1])->is_pressed);
    EGUI_TEST_ASSERT_FALSE(test_buttons[0].is_checked);
    EGUI_TEST_ASSERT_TRUE(test_buttons[1].is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_last_index);
}

static void test_radio_button_keyboard_space_and_enter_select_targets(void)
{
    setup_group(0);

    EGUI_TEST_ASSERT_TRUE(send_key(&test_buttons[1], EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_FALSE(test_buttons[0].is_checked);
    EGUI_TEST_ASSERT_TRUE(test_buttons[1].is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_last_index);

    EGUI_TEST_ASSERT_TRUE(send_key(&test_buttons[1], EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_changed_count);
    EGUI_TEST_ASSERT_TRUE(test_buttons[1].is_checked);

    EGUI_TEST_ASSERT_TRUE(send_key(&test_buttons[0], EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(test_buttons[0].is_checked);
    EGUI_TEST_ASSERT_FALSE(test_buttons[1].is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_last_index);
}

static void test_radio_button_disabled_input_does_not_select(void)
{
    egui_dim_t inside_x;
    egui_dim_t inside_y;

    setup_group(0);
    layout_button(EGUI_VIEW_OF(&test_buttons[1]), 10, 60, 180, 34);
    get_indicator_center(&test_buttons[1], &inside_x, &inside_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_buttons[1]), 1);
    egui_view_set_enable(EGUI_VIEW_OF(&test_buttons[1]), 0);
    EGUI_TEST_ASSERT_FALSE(send_key(&test_buttons[1], EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_buttons[1])->is_pressed);
    EGUI_TEST_ASSERT_TRUE(test_buttons[0].is_checked);
    EGUI_TEST_ASSERT_FALSE(test_buttons[1].is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_buttons[1]), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_buttons[1], EGUI_MOTION_EVENT_ACTION_UP, inside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_buttons[1])->is_pressed);
    EGUI_TEST_ASSERT_TRUE(test_buttons[0].is_checked);
    EGUI_TEST_ASSERT_FALSE(test_buttons[1].is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_radio_button_static_preview_consumes_input_and_clears_pressed_state(void)
{
    egui_dim_t inside_x;
    egui_dim_t inside_y;

    setup_preview_group(0);
    layout_button(EGUI_VIEW_OF(&preview_buttons[1]), 10, 20, 104, 24);
    get_indicator_center(&preview_buttons[1], &inside_x, &inside_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_buttons[1]), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(&preview_buttons[1], EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_buttons[1])->is_pressed);
    EGUI_TEST_ASSERT_TRUE(preview_buttons[0].is_checked);
    EGUI_TEST_ASSERT_FALSE(preview_buttons[1].is_checked);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_buttons[1]), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key_action(&preview_buttons[1], EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_TRUE(send_preview_key_action(&preview_buttons[1], EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_buttons[1])->is_pressed);
    EGUI_TEST_ASSERT_TRUE(preview_buttons[0].is_checked);
    EGUI_TEST_ASSERT_FALSE(preview_buttons[1].is_checked);
}

void test_radio_button_run(void)
{
    EGUI_TEST_SUITE_BEGIN(radio_button);
    EGUI_TEST_RUN(test_radio_button_style_helpers_update_palette_and_clear_pressed_state);
    EGUI_TEST_RUN(test_radio_button_text_and_icon_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_radio_button_checked_setter_updates_group_and_clears_pressed_state);
    EGUI_TEST_RUN(test_radio_button_touch_same_target_release_selects_once);
    EGUI_TEST_RUN(test_radio_button_keyboard_space_and_enter_select_targets);
    EGUI_TEST_RUN(test_radio_button_disabled_input_does_not_select);
    EGUI_TEST_RUN(test_radio_button_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_SUITE_END();
}
