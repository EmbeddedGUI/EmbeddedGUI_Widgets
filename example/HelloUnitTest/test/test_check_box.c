#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_check_box.h"

#include "../../HelloCustomWidgets/input/check_box/egui_view_check_box.h"
#include "../../HelloCustomWidgets/input/check_box/egui_view_check_box.c"

typedef struct
{
    egui_region_t region_screen;
    egui_background_t *background;
    const char *text;
    const egui_font_t *font;
    const egui_font_t *icon_font;
    uint8_t is_checked;
    uint8_t mark_style;
    egui_alpha_t alpha;
    egui_color_t box_color;
    egui_color_t box_fill_color;
    egui_color_t check_color;
    egui_color_t text_color;
    egui_dim_t text_gap;
} check_box_preview_snapshot_t;

static egui_view_checkbox_t test_box;
static egui_view_checkbox_t preview_box;
static egui_view_api_t test_box_api;
static egui_view_api_t preview_api;
static uint8_t g_checked_count;
static uint8_t g_last_checked;

static void on_checked(egui_view_t *self, int is_checked)
{
    EGUI_UNUSED(self);
    g_checked_count++;
    g_last_checked = is_checked;
}

static void reset_listener_state(void)
{
    g_checked_count = 0;
    g_last_checked = 0xFF;
}

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void setup_box(uint8_t checked)
{
    egui_view_checkbox_init(EGUI_VIEW_OF(&test_box));
    egui_view_set_size(EGUI_VIEW_OF(&test_box), 180, 34);
    egui_view_checkbox_set_font(EGUI_VIEW_OF(&test_box), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_check_box_apply_standard_style(EGUI_VIEW_OF(&test_box));
    hcw_check_box_set_text(EGUI_VIEW_OF(&test_box), "Email alerts");
    hcw_check_box_set_checked(EGUI_VIEW_OF(&test_box), checked);
    egui_view_checkbox_set_on_checked_listener(EGUI_VIEW_OF(&test_box), on_checked);
    hcw_check_box_override_interaction_api(EGUI_VIEW_OF(&test_box), &test_box_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&test_box), 1);
#endif
    reset_listener_state();
}

static void setup_preview_box(uint8_t checked)
{
    egui_view_checkbox_init(EGUI_VIEW_OF(&preview_box));
    egui_view_set_size(EGUI_VIEW_OF(&preview_box), 104, 28);
    egui_view_checkbox_set_font(EGUI_VIEW_OF(&preview_box), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_check_box_apply_compact_style(EGUI_VIEW_OF(&preview_box));
    hcw_check_box_set_text(EGUI_VIEW_OF(&preview_box), "Auto");
    hcw_check_box_set_checked(EGUI_VIEW_OF(&preview_box), checked);
    egui_view_checkbox_set_on_checked_listener(EGUI_VIEW_OF(&preview_box), on_checked);
    hcw_check_box_override_static_preview_api(EGUI_VIEW_OF(&preview_box), &preview_api);
    reset_listener_state();
}

static void layout_box(egui_view_t *view, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(view, &region);
    egui_region_copy(&view->region_screen, &region);
}

static int send_touch(egui_view_checkbox_t *box, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return EGUI_VIEW_OF(box)->api->on_touch_event(EGUI_VIEW_OF(box), &event);
}

static int dispatch_key_event_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->dispatch_key_event(view, &event);
}

static int send_key_to_view(egui_view_t *view, uint8_t key_code)
{
    int handled = 0;

    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static int send_key(egui_view_checkbox_t *box, uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(box), key_code);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_box), key_code);
}

static void get_indicator_center(egui_view_checkbox_t *box, egui_dim_t *x, egui_dim_t *y)
{
    *x = EGUI_VIEW_OF(box)->region_screen.location.x + EGUI_VIEW_OF(box)->region_screen.size.height / 2;
    *y = EGUI_VIEW_OF(box)->region_screen.location.y + EGUI_VIEW_OF(box)->region_screen.size.height / 2;
}

static void capture_preview_snapshot(check_box_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_box)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_box)->background;
    snapshot->text = preview_box.text;
    snapshot->font = preview_box.font;
    snapshot->icon_font = preview_box.icon_font;
    snapshot->is_checked = preview_box.is_checked;
    snapshot->mark_style = preview_box.mark_style;
    snapshot->alpha = preview_box.alpha;
    snapshot->box_color = preview_box.box_color;
    snapshot->box_fill_color = preview_box.box_fill_color;
    snapshot->check_color = preview_box.check_color;
    snapshot->text_color = preview_box.text_color;
    snapshot->text_gap = preview_box.text_gap;
}

static void assert_preview_state_unchanged(const check_box_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_box)->region_screen);
    EGUI_TEST_ASSERT_TRUE(snapshot->background == EGUI_VIEW_OF(&preview_box)->background);
    EGUI_TEST_ASSERT_TRUE(strcmp(snapshot->text, preview_box.text) == 0);
    EGUI_TEST_ASSERT_TRUE(snapshot->font == preview_box.font);
    EGUI_TEST_ASSERT_TRUE(snapshot->icon_font == preview_box.icon_font);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_checked, preview_box.is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->mark_style, preview_box.mark_style);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, preview_box.alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->box_color.full, preview_box.box_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->box_fill_color.full, preview_box.box_fill_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->check_color.full, preview_box.check_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_box.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_gap, preview_box.text_gap);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_checked_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0xFF, g_last_checked);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_box)->is_pressed);
}

static void test_check_box_style_helpers_update_palette_and_clear_pressed_state(void)
{
    egui_view_checkbox_t *local;

    setup_box(0);
    local = (egui_view_checkbox_t *)EGUI_VIEW_OF(&test_box);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_box), 1);
    hcw_check_box_apply_compact_style(EGUI_VIEW_OF(&test_box));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xC7D8CE).full, local->box_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0C7C73).full, local->box_fill_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x21303F).full, local->text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(6, local->text_gap);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_box), 1);
    hcw_check_box_apply_read_only_style(EGUI_VIEW_OF(&test_box));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xD8E0E8).full, local->box_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xAFB8C3).full, local->box_fill_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x546474).full, local->text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xF7F9FB).full, local->check_color.full);
}

static void test_check_box_text_and_mark_setters_clear_pressed_state(void)
{
    egui_view_checkbox_t *local;

    setup_box(0);
    local = (egui_view_checkbox_t *)EGUI_VIEW_OF(&test_box);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_box), 1);
    hcw_check_box_set_text(EGUI_VIEW_OF(&test_box), "Offline sync");
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(strcmp("Offline sync", local->text) == 0);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_box), 1);
    hcw_check_box_set_mark_style(EGUI_VIEW_OF(&test_box), EGUI_VIEW_CHECKBOX_MARK_STYLE_ICON);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CHECKBOX_MARK_STYLE_ICON, local->mark_style);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_box), 1);
    hcw_check_box_set_mark_icon(EGUI_VIEW_OF(&test_box), "X");
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(strcmp("X", local->mark_icon) == 0);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_box), 1);
    hcw_check_box_set_icon_font(EGUI_VIEW_OF(&test_box), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->icon_font == (const egui_font_t *)&egui_res_font_montserrat_10_4);
}

static void test_check_box_checked_setter_notifies_and_clears_pressed_state(void)
{
    egui_view_checkbox_t *local;

    setup_box(0);
    local = (egui_view_checkbox_t *)EGUI_VIEW_OF(&test_box);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_box), 1);
    hcw_check_box_set_checked(EGUI_VIEW_OF(&test_box), 1);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, local->is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_checked_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_last_checked);

    hcw_check_box_set_checked(EGUI_VIEW_OF(&test_box), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_checked_count);
}

static void test_check_box_touch_same_target_release_toggles_once(void)
{
    egui_view_checkbox_t *local;
    egui_dim_t inside_x;
    egui_dim_t inside_y;
    egui_dim_t outside_x;

    setup_box(0);
    layout_box(EGUI_VIEW_OF(&test_box), 10, 20, 180, 34);
    local = (egui_view_checkbox_t *)EGUI_VIEW_OF(&test_box);
    get_indicator_center(&test_box, &inside_x, &inside_y);
    outside_x = EGUI_VIEW_OF(&test_box)->region_screen.location.x + EGUI_VIEW_OF(&test_box)->region_screen.size.width + 12;

    EGUI_TEST_ASSERT_TRUE(send_touch(&test_box, EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_box, EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_box, EGUI_MOTION_EVENT_ACTION_UP, outside_x, inside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, local->is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_checked_count);

    EGUI_TEST_ASSERT_TRUE(send_touch(&test_box, EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_box, EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_box, EGUI_MOTION_EVENT_ACTION_MOVE, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_box, EGUI_MOTION_EVENT_ACTION_UP, inside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, local->is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_checked_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_last_checked);
}

static void test_check_box_keyboard_space_and_enter_toggle(void)
{
    egui_view_checkbox_t *local;

    setup_box(0);
    local = (egui_view_checkbox_t *)EGUI_VIEW_OF(&test_box);

    EGUI_TEST_ASSERT_TRUE(send_key(&test_box, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(1, local->is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_checked_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_last_checked);

    EGUI_TEST_ASSERT_TRUE(send_key(&test_box, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(0, local->is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_checked_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_last_checked);
}

static void test_check_box_disabled_input_does_not_toggle(void)
{
    egui_view_checkbox_t *local;
    egui_dim_t inside_x;
    egui_dim_t inside_y;

    setup_box(1);
    layout_box(EGUI_VIEW_OF(&test_box), 10, 20, 180, 34);
    local = (egui_view_checkbox_t *)EGUI_VIEW_OF(&test_box);
    get_indicator_center(&test_box, &inside_x, &inside_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_box), 1);
    egui_view_set_enable(EGUI_VIEW_OF(&test_box), 0);
    EGUI_TEST_ASSERT_FALSE(send_key(&test_box, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, local->is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_checked_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_box), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_box, EGUI_MOTION_EVENT_ACTION_UP, inside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, local->is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_checked_count);
}

static void test_check_box_static_preview_consumes_input_and_keeps_state(void)
{
    check_box_preview_snapshot_t initial_snapshot;
    egui_dim_t inside_x;
    egui_dim_t inside_y;

    setup_preview_box(1);
    layout_box(EGUI_VIEW_OF(&preview_box), 10, 20, 104, 28);
    get_indicator_center(&preview_box, &inside_x, &inside_y);
    capture_preview_snapshot(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_box), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(&preview_box, EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    assert_preview_state_unchanged(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_box), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_SPACE));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_check_box_run(void)
{
    EGUI_TEST_SUITE_BEGIN(check_box);
    EGUI_TEST_RUN(test_check_box_style_helpers_update_palette_and_clear_pressed_state);
    EGUI_TEST_RUN(test_check_box_text_and_mark_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_check_box_checked_setter_notifies_and_clears_pressed_state);
    EGUI_TEST_RUN(test_check_box_touch_same_target_release_toggles_once);
    EGUI_TEST_RUN(test_check_box_keyboard_space_and_enter_toggle);
    EGUI_TEST_RUN(test_check_box_disabled_input_does_not_toggle);
    EGUI_TEST_RUN(test_check_box_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
