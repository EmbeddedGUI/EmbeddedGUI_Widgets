#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_badge_group.h"

#include "../../HelloCustomWidgets/display/badge_group/egui_view_badge_group.h"
#include "../../HelloCustomWidgets/display/badge_group/egui_view_badge_group.c"

static egui_view_badge_group_t test_group;
static egui_view_badge_group_t preview_group;
static egui_view_api_t preview_api;
static int click_count;

static const egui_view_badge_group_item_t g_items_0[] = {
        {"Review", "4", 0, 1, 0},
        {"Ready", "12", 1, 1, 0},
        {"Risk", "1", 2, 0, 1},
        {"Archive", "7", 3, 0, 1},
};

static const egui_view_badge_group_item_t g_items_1[] = {
        {"Online", "8", 1, 1, 0},
        {"Shadow", "2", 3, 0, 1},
        {"Sync", "3", 0, 0, 1},
        {"Alert", "1", 2, 1, 0},
};

static const egui_view_badge_group_item_t g_items_2[] = {
        {"Queued", "5", 3, 0, 1},
        {"Hold", "2", 2, 1, 0},
        {"Owner", "A", 0, 1, 0},
        {"Done", "9", 1, 0, 1},
};

static const egui_view_badge_group_item_t g_items_3[] = {
        {"Pinned", "6", 0, 0, 1},
        {"Calm", "3", 3, 1, 0},
        {"Watch", "2", 2, 0, 1},
        {"Live", "4", 1, 0, 1},
};

static const egui_view_badge_group_item_t g_extra_items[] = {
        {"A", "1", 0, 0, 0}, {"B", "2", 1, 1, 0}, {"C", "3", 2, 0, 1}, {"D", "4", 3, 1, 0},
        {"E", "5", 0, 0, 1}, {"F", "6", 1, 0, 1}, {"G", "7", 2, 1, 0},
};

static const egui_view_badge_group_snapshot_t g_snapshots[] = {
        {"TRIAGE", "Release lanes", "Mixed badges stay aligned.", "Summary follows focus.", g_items_0, 4, 0},
        {"QUEUE", "Ops handoff", "Success tone leads the row.", "Success drives footer.", g_items_1, 4, 0},
        {"RISK", "Change review", "Warning focus stays visible.", "Warning stays visible.", g_items_2, 4, 1},
        {"CALM", "Archive sweep", "Neutral focus softens the card.", "Neutral stays calm.", g_items_3, 4, 1},
};

static const egui_view_badge_group_snapshot_t g_overflow_snapshots[] = {
        {"A", "A", "A", "A", g_extra_items, 7, 0},
        {"B", "B", "B", "B", g_extra_items, 7, 1},
        {"C", "C", "C", "C", g_extra_items, 7, 2},
        {"D", "D", "D", "D", g_extra_items, 7, 3},
        {"E", "E", "E", "E", g_extra_items, 7, 4},
        {"F", "F", "F", "F", g_extra_items, 7, 5},
        {"G", "G", "G", "G", g_extra_items, 7, 6},
};

static const egui_view_badge_group_snapshot_t g_invalid_focus_snapshot = {
        "T", "Title", "Body", "Footer", g_items_0, 4, 9,
};

static void on_group_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    click_count++;
}

static void reset_click_count(void)
{
    click_count = 0;
}

static void setup_group(void)
{
    egui_view_badge_group_init(EGUI_VIEW_OF(&test_group));
    egui_view_set_size(EGUI_VIEW_OF(&test_group), 196, 118);
    egui_view_badge_group_set_snapshots(EGUI_VIEW_OF(&test_group), g_snapshots, 4);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&test_group), on_group_click);
    reset_click_count();
}

static void setup_preview_group(void)
{
    egui_view_badge_group_init(EGUI_VIEW_OF(&preview_group));
    egui_view_set_size(EGUI_VIEW_OF(&preview_group), 104, 84);
    egui_view_badge_group_set_snapshots(EGUI_VIEW_OF(&preview_group), g_snapshots, 4);
    egui_view_badge_group_set_current_snapshot(EGUI_VIEW_OF(&preview_group), 1);
    egui_view_badge_group_set_compact_mode(EGUI_VIEW_OF(&preview_group), 1);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_group), on_group_click);
    egui_view_badge_group_override_static_preview_api(EGUI_VIEW_OF(&preview_group), &preview_api);
    reset_click_count();
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

static void layout_group(void)
{
    layout_view(EGUI_VIEW_OF(&test_group), 10, 20, 196, 118);
}

static void layout_preview_group(void)
{
    layout_view(EGUI_VIEW_OF(&preview_group), 12, 18, 104, 84);
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
    return send_touch_to_view(EGUI_VIEW_OF(&test_group), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_group), type, x, y);
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_group), key_code);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_group), key_code);
}

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void get_view_outside_point(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x - 6;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void seed_pressed_state(egui_view_t *view, uint8_t visual_pressed)
{
    egui_view_set_pressed(view, visual_pressed ? true : false);
}

static void assert_pressed_cleared(egui_view_t *view)
{
    EGUI_TEST_ASSERT_FALSE(view->is_pressed);
}

static void test_badge_group_set_snapshots_clamps_and_clears_pressed_state(void)
{
    setup_group();

    test_group.current_snapshot = EGUI_VIEW_BADGE_GROUP_MAX_SNAPSHOTS;
    seed_pressed_state(EGUI_VIEW_OF(&test_group), 1);
    egui_view_badge_group_set_snapshots(EGUI_VIEW_OF(&test_group), g_overflow_snapshots, 7);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_BADGE_GROUP_MAX_SNAPSHOTS, test_group.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_badge_group_get_current_snapshot(EGUI_VIEW_OF(&test_group)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_group));

    test_group.current_snapshot = 2;
    seed_pressed_state(EGUI_VIEW_OF(&test_group), 1);
    egui_view_badge_group_set_snapshots(EGUI_VIEW_OF(&test_group), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_group.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_badge_group_get_current_snapshot(EGUI_VIEW_OF(&test_group)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_group));
}

static void test_badge_group_snapshot_and_setters_clear_pressed_state(void)
{
    egui_color_t surface = EGUI_COLOR_HEX(0x101112);
    egui_color_t border = EGUI_COLOR_HEX(0x202122);
    egui_color_t text = EGUI_COLOR_HEX(0x303132);
    egui_color_t muted = EGUI_COLOR_HEX(0x404142);
    egui_color_t accent = EGUI_COLOR_HEX(0x505152);
    egui_color_t success = EGUI_COLOR_HEX(0x606162);
    egui_color_t warning = EGUI_COLOR_HEX(0x707172);
    egui_color_t neutral = EGUI_COLOR_HEX(0x808182);

    setup_group();

    egui_view_badge_group_set_current_snapshot(EGUI_VIEW_OF(&test_group), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_badge_group_get_current_snapshot(EGUI_VIEW_OF(&test_group)));

    seed_pressed_state(EGUI_VIEW_OF(&test_group), 1);
    egui_view_badge_group_set_current_snapshot(EGUI_VIEW_OF(&test_group), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_badge_group_get_current_snapshot(EGUI_VIEW_OF(&test_group)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_group));

    seed_pressed_state(EGUI_VIEW_OF(&test_group), 1);
    egui_view_badge_group_set_current_snapshot(EGUI_VIEW_OF(&test_group), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_badge_group_get_current_snapshot(EGUI_VIEW_OF(&test_group)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_group));

    seed_pressed_state(EGUI_VIEW_OF(&test_group), 1);
    egui_view_badge_group_set_font(EGUI_VIEW_OF(&test_group), NULL);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_group));
    EGUI_TEST_ASSERT_TRUE(test_group.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(EGUI_VIEW_OF(&test_group), 1);
    egui_view_badge_group_set_meta_font(EGUI_VIEW_OF(&test_group), NULL);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_group));
    EGUI_TEST_ASSERT_TRUE(test_group.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(EGUI_VIEW_OF(&test_group), 1);
    egui_view_badge_group_set_palette(EGUI_VIEW_OF(&test_group), surface, border, text, muted, accent, success, warning, neutral);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_group));
    EGUI_TEST_ASSERT_EQUAL_INT(surface.full, test_group.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(border.full, test_group.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(text.full, test_group.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(muted.full, test_group.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(accent.full, test_group.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(success.full, test_group.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(warning.full, test_group.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(neutral.full, test_group.neutral_color.full);

    seed_pressed_state(EGUI_VIEW_OF(&test_group), 1);
    egui_view_badge_group_set_compact_mode(EGUI_VIEW_OF(&test_group), 2);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_group));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_group.compact_mode);

    seed_pressed_state(EGUI_VIEW_OF(&test_group), 1);
    egui_view_badge_group_set_read_only_mode(EGUI_VIEW_OF(&test_group), 3);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_group));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_group.read_only_mode);
}

static void test_badge_group_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t inside_x;
    egui_dim_t inside_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_group();
    layout_group();
    get_view_center(EGUI_VIEW_OF(&test_group), &inside_x, &inside_y);
    get_view_outside_point(EGUI_VIEW_OF(&test_group), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_group)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_group)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_group));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_group)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_group)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, inside_x, inside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_group));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, inside_x, inside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_group));
}

static void test_badge_group_keyboard_click_listener(void)
{
    setup_group();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
}

static void test_badge_group_compact_mode_clears_pressed_and_keeps_click_behavior(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_group();
    layout_group();
    get_view_center(EGUI_VIEW_OF(&test_group), &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_group)->is_pressed);

    egui_view_badge_group_set_compact_mode(EGUI_VIEW_OF(&test_group), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_group.compact_mode);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_group));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
}

static void test_badge_group_read_only_and_disabled_guards_clear_pressed_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_group();
    layout_group();
    get_view_center(EGUI_VIEW_OF(&test_group), &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_group)->is_pressed);

    egui_view_badge_group_set_read_only_mode(EGUI_VIEW_OF(&test_group), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_group.read_only_mode);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_group));

    seed_pressed_state(EGUI_VIEW_OF(&test_group), 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_group));

    seed_pressed_state(EGUI_VIEW_OF(&test_group), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_group));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    egui_view_badge_group_set_read_only_mode(EGUI_VIEW_OF(&test_group), 0);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);

    setup_group();
    layout_group();
    get_view_center(EGUI_VIEW_OF(&test_group), &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_group)->is_pressed);

    egui_view_set_enable(EGUI_VIEW_OF(&test_group), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_group));

    seed_pressed_state(EGUI_VIEW_OF(&test_group), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_group));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    egui_view_set_enable(EGUI_VIEW_OF(&test_group), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
}

static void test_badge_group_static_preview_consumes_input_and_clears_pressed_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_group();
    layout_preview_group();
    get_view_center(EGUI_VIEW_OF(&preview_group), &x, &y);

    seed_pressed_state(EGUI_VIEW_OF(&preview_group), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(EGUI_VIEW_OF(&preview_group));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_badge_group_get_current_snapshot(EGUI_VIEW_OF(&preview_group)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    seed_pressed_state(EGUI_VIEW_OF(&preview_group), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(EGUI_VIEW_OF(&preview_group));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_badge_group_get_current_snapshot(EGUI_VIEW_OF(&preview_group)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);
}

static void test_badge_group_internal_helpers_cover_focus_tone_text_and_width(void)
{
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);
    egui_color_t mixed = egui_view_badge_group_mix_disabled(sample);

    setup_group();
    egui_view_badge_group_set_palette(EGUI_VIEW_OF(&test_group), EGUI_COLOR_HEX(0x111111), EGUI_COLOR_HEX(0x222222), EGUI_COLOR_HEX(0x333333),
                                      EGUI_COLOR_HEX(0x444444), EGUI_COLOR_HEX(0x555555), EGUI_COLOR_HEX(0x666666), EGUI_COLOR_HEX(0x777777),
                                      EGUI_COLOR_HEX(0x888888));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_BADGE_GROUP_MAX_SNAPSHOTS, egui_view_badge_group_clamp_snapshot_count(9));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_BADGE_GROUP_MAX_ITEMS, egui_view_badge_group_clamp_item_count(9));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_badge_group_text_len(NULL));
    EGUI_TEST_ASSERT_EQUAL_INT(6, egui_view_badge_group_text_len("Review"));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_badge_group_focus_index(NULL, 1));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_badge_group_focus_index(&g_invalid_focus_snapshot, 4));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_badge_group_focus_index(&g_snapshots[2], 4));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x555555).full, egui_view_badge_group_tone_color(&test_group, 0).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x666666).full, egui_view_badge_group_tone_color(&test_group, 1).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x777777).full, egui_view_badge_group_tone_color(&test_group, 2).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x888888).full, egui_view_badge_group_tone_color(&test_group, 3).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x555555).full, egui_view_badge_group_tone_color(&test_group, 9).full);
    EGUI_TEST_ASSERT_EQUAL_INT(24, egui_view_badge_group_pill_width("", 1, 24, 64));
    EGUI_TEST_ASSERT_EQUAL_INT(38, egui_view_badge_group_pill_width("AB", 0, 28, 50));
    EGUI_TEST_ASSERT_EQUAL_INT(40, egui_view_badge_group_pill_width("Long label", 0, 28, 40));
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_DARK_GREY, 68).full, mixed.full);
}

void test_badge_group_run(void)
{
    EGUI_TEST_SUITE_BEGIN(badge_group);
    EGUI_TEST_RUN(test_badge_group_set_snapshots_clamps_and_clears_pressed_state);
    EGUI_TEST_RUN(test_badge_group_snapshot_and_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_badge_group_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_badge_group_keyboard_click_listener);
    EGUI_TEST_RUN(test_badge_group_compact_mode_clears_pressed_and_keeps_click_behavior);
    EGUI_TEST_RUN(test_badge_group_read_only_and_disabled_guards_clear_pressed_state);
    EGUI_TEST_RUN(test_badge_group_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_RUN(test_badge_group_internal_helpers_cover_focus_tone_text_and_width);
    EGUI_TEST_SUITE_END();
}
