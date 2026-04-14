#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_card_panel.h"

#include "../../HelloCustomWidgets/display/card_panel/egui_view_card_panel.h"
#include "../../HelloCustomWidgets/display/card_panel/egui_view_card_panel.c"

static egui_view_card_panel_t test_panel;
static egui_view_card_panel_t preview_panel;
static egui_view_api_t preview_api;
static int click_count;

static const egui_view_card_panel_snapshot_t g_snapshots[] = {
        {"OVERVIEW", "Workspace status", "Three flows stay aligned.", "98%", "uptime", "Today", "Two checks wait.", "Footer stays readable.", "Open", 0, 1},
        {"SYNC", "Design review", "New handoff needs approval.", "4", "changes", "Next step", "Confirm spacing tokens.", "Summary stays close.", "Review", 2,
         1},
        {"DEPLOY", "Release notes", "Ready for staged publish.", "6", "items", "Channel", "Internal preview for QA.", "Card stays calm on dense pages.",
         "Publish", 1, 0},
        {"ARCHIVE", "Readback summary", "Older detail stays available.", "12", "pages", "History", "Summary stays visible.", "Read only mode still works.",
         "Browse", 3, 0},
};

static const egui_view_card_panel_snapshot_t g_overflow_snapshots[] = {
        {"A", "A", "A", "1", "a", "A", "A", "A", "A", 0, 0},
        {"B", "B", "B", "2", "b", "B", "B", "B", "B", 1, 1},
        {"C", "C", "C", "3", "c", "C", "C", "C", "C", 2, 0},
        {"D", "D", "D", "4", "d", "D", "D", "D", "D", 3, 1},
        {"E", "E", "E", "5", "e", "E", "E", "E", "E", 0, 0},
        {"F", "F", "F", "6", "f", "F", "F", "F", "F", 1, 1},
        {"G", "G", "G", "7", "g", "G", "G", "G", "G", 2, 0},
};

static void on_panel_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    click_count++;
}

static void reset_click_count(void)
{
    click_count = 0;
}

static void setup_panel(void)
{
    egui_view_card_panel_init(EGUI_VIEW_OF(&test_panel));
    egui_view_set_size(EGUI_VIEW_OF(&test_panel), 196, 122);
    egui_view_card_panel_set_snapshots(EGUI_VIEW_OF(&test_panel), g_snapshots, 4);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&test_panel), on_panel_click);
    reset_click_count();
}

static void setup_preview_panel(void)
{
    egui_view_card_panel_init(EGUI_VIEW_OF(&preview_panel));
    egui_view_set_size(EGUI_VIEW_OF(&preview_panel), 104, 90);
    egui_view_card_panel_set_snapshots(EGUI_VIEW_OF(&preview_panel), g_snapshots, 4);
    egui_view_card_panel_set_current_snapshot(EGUI_VIEW_OF(&preview_panel), 1);
    egui_view_card_panel_set_compact_mode(EGUI_VIEW_OF(&preview_panel), 1);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_panel), on_panel_click);
    egui_view_card_panel_override_static_preview_api(EGUI_VIEW_OF(&preview_panel), &preview_api);
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

static void layout_panel(void)
{
    layout_view(EGUI_VIEW_OF(&test_panel), 10, 20, 196, 122);
}

static void layout_preview_panel(void)
{
    layout_view(EGUI_VIEW_OF(&preview_panel), 12, 18, 104, 90);
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
    return send_touch_to_view(EGUI_VIEW_OF(&test_panel), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_panel), type, x, y);
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_panel), key_code);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_panel), key_code);
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

static void test_card_panel_set_snapshots_clamps_and_clears_pressed_state(void)
{
    setup_panel();

    test_panel.current_snapshot = EGUI_VIEW_CARD_PANEL_MAX_SNAPSHOTS;
    seed_pressed_state(EGUI_VIEW_OF(&test_panel), 1);
    egui_view_card_panel_set_snapshots(EGUI_VIEW_OF(&test_panel), g_overflow_snapshots, 7);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CARD_PANEL_MAX_SNAPSHOTS, test_panel.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_card_panel_get_current_snapshot(EGUI_VIEW_OF(&test_panel)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_panel));

    test_panel.current_snapshot = 2;
    seed_pressed_state(EGUI_VIEW_OF(&test_panel), 1);
    egui_view_card_panel_set_snapshots(EGUI_VIEW_OF(&test_panel), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_panel.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_card_panel_get_current_snapshot(EGUI_VIEW_OF(&test_panel)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_panel));
}

static void test_card_panel_snapshot_and_setters_clear_pressed_state(void)
{
    egui_color_t surface = EGUI_COLOR_HEX(0x101112);
    egui_color_t border = EGUI_COLOR_HEX(0x202122);
    egui_color_t text = EGUI_COLOR_HEX(0x303132);
    egui_color_t muted = EGUI_COLOR_HEX(0x404142);
    egui_color_t accent = EGUI_COLOR_HEX(0x505152);
    egui_color_t success = EGUI_COLOR_HEX(0x606162);
    egui_color_t warning = EGUI_COLOR_HEX(0x707172);
    egui_color_t neutral = EGUI_COLOR_HEX(0x808182);

    setup_panel();

    egui_view_card_panel_set_current_snapshot(EGUI_VIEW_OF(&test_panel), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_card_panel_get_current_snapshot(EGUI_VIEW_OF(&test_panel)));

    seed_pressed_state(EGUI_VIEW_OF(&test_panel), 1);
    egui_view_card_panel_set_current_snapshot(EGUI_VIEW_OF(&test_panel), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_card_panel_get_current_snapshot(EGUI_VIEW_OF(&test_panel)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_panel));

    seed_pressed_state(EGUI_VIEW_OF(&test_panel), 1);
    egui_view_card_panel_set_current_snapshot(EGUI_VIEW_OF(&test_panel), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_card_panel_get_current_snapshot(EGUI_VIEW_OF(&test_panel)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_panel));

    seed_pressed_state(EGUI_VIEW_OF(&test_panel), 1);
    egui_view_card_panel_set_font(EGUI_VIEW_OF(&test_panel), NULL);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_panel));
    EGUI_TEST_ASSERT_TRUE(test_panel.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(EGUI_VIEW_OF(&test_panel), 1);
    egui_view_card_panel_set_meta_font(EGUI_VIEW_OF(&test_panel), NULL);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_panel));
    EGUI_TEST_ASSERT_TRUE(test_panel.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(EGUI_VIEW_OF(&test_panel), 1);
    egui_view_card_panel_set_palette(EGUI_VIEW_OF(&test_panel), surface, border, text, muted, accent, success, warning, neutral);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_panel));
    EGUI_TEST_ASSERT_EQUAL_INT(surface.full, test_panel.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(border.full, test_panel.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(text.full, test_panel.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(muted.full, test_panel.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(accent.full, test_panel.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(success.full, test_panel.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(warning.full, test_panel.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(neutral.full, test_panel.neutral_color.full);

    seed_pressed_state(EGUI_VIEW_OF(&test_panel), 1);
    egui_view_card_panel_set_compact_mode(EGUI_VIEW_OF(&test_panel), 2);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_panel));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_panel.compact_mode);

    seed_pressed_state(EGUI_VIEW_OF(&test_panel), 1);
    egui_view_card_panel_set_read_only_mode(EGUI_VIEW_OF(&test_panel), 3);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_panel));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_panel.read_only_mode);
}

static void test_card_panel_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t inside_x;
    egui_dim_t inside_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_panel();
    layout_panel();
    get_view_center(EGUI_VIEW_OF(&test_panel), &inside_x, &inside_y);
    get_view_outside_point(EGUI_VIEW_OF(&test_panel), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_panel)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_panel)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_panel));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_panel)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_panel)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, inside_x, inside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_panel));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, inside_x, inside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_panel));
}

static void test_card_panel_keyboard_click_listener(void)
{
    setup_panel();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
}

static void test_card_panel_compact_mode_clears_pressed_and_keeps_click_behavior(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_panel();
    layout_panel();
    get_view_center(EGUI_VIEW_OF(&test_panel), &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_panel)->is_pressed);

    egui_view_card_panel_set_compact_mode(EGUI_VIEW_OF(&test_panel), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_panel.compact_mode);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_panel));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
}

static void test_card_panel_read_only_and_disabled_guards_clear_pressed_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_panel();
    layout_panel();
    get_view_center(EGUI_VIEW_OF(&test_panel), &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_panel)->is_pressed);

    egui_view_card_panel_set_read_only_mode(EGUI_VIEW_OF(&test_panel), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_panel.read_only_mode);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_panel));

    seed_pressed_state(EGUI_VIEW_OF(&test_panel), 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_panel));

    seed_pressed_state(EGUI_VIEW_OF(&test_panel), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_panel));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    egui_view_card_panel_set_read_only_mode(EGUI_VIEW_OF(&test_panel), 0);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);

    setup_panel();
    layout_panel();
    get_view_center(EGUI_VIEW_OF(&test_panel), &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_panel)->is_pressed);

    egui_view_set_enable(EGUI_VIEW_OF(&test_panel), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_panel));

    seed_pressed_state(EGUI_VIEW_OF(&test_panel), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_panel));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    egui_view_set_enable(EGUI_VIEW_OF(&test_panel), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
}

static void test_card_panel_static_preview_consumes_input_and_keeps_snapshot(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_panel();
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_card_panel_get_current_snapshot(EGUI_VIEW_OF(&preview_panel)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_panel.compact_mode);
    layout_preview_panel();
    get_view_center(EGUI_VIEW_OF(&preview_panel), &x, &y);

    seed_pressed_state(EGUI_VIEW_OF(&preview_panel), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(EGUI_VIEW_OF(&preview_panel));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_card_panel_get_current_snapshot(EGUI_VIEW_OF(&preview_panel)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    seed_pressed_state(EGUI_VIEW_OF(&preview_panel), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(EGUI_VIEW_OF(&preview_panel));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_card_panel_get_current_snapshot(EGUI_VIEW_OF(&preview_panel)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);
}

static void test_card_panel_internal_helpers_cover_tone_text_and_pill_width(void)
{
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);
    egui_color_t mixed = egui_view_card_panel_mix_disabled(sample);

    setup_panel();
    egui_view_card_panel_set_palette(EGUI_VIEW_OF(&test_panel), EGUI_COLOR_HEX(0x111111), EGUI_COLOR_HEX(0x222222), EGUI_COLOR_HEX(0x333333),
                                     EGUI_COLOR_HEX(0x444444), EGUI_COLOR_HEX(0x555555), EGUI_COLOR_HEX(0x666666), EGUI_COLOR_HEX(0x777777),
                                     EGUI_COLOR_HEX(0x888888));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CARD_PANEL_MAX_SNAPSHOTS, egui_view_card_panel_clamp_snapshot_count(9));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_card_panel_text_len(NULL));
    EGUI_TEST_ASSERT_EQUAL_INT(6, egui_view_card_panel_text_len("Review"));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x555555).full, egui_view_card_panel_tone_color(&test_panel, 0).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x666666).full, egui_view_card_panel_tone_color(&test_panel, 1).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x777777).full, egui_view_card_panel_tone_color(&test_panel, 2).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x888888).full, egui_view_card_panel_tone_color(&test_panel, 3).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x555555).full, egui_view_card_panel_tone_color(&test_panel, 9).full);
    EGUI_TEST_ASSERT_EQUAL_INT(34, egui_view_card_panel_pill_width("", 1, 34, 64));
    EGUI_TEST_ASSERT_EQUAL_INT(52, egui_view_card_panel_pill_width("AB", 0, 42, 78));
    EGUI_TEST_ASSERT_EQUAL_INT(50, egui_view_card_panel_pill_width("Long label", 0, 42, 50));
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_DARK_GREY, 68).full, mixed.full);
}

void test_card_panel_run(void)
{
    EGUI_TEST_SUITE_BEGIN(card_panel);
    EGUI_TEST_RUN(test_card_panel_set_snapshots_clamps_and_clears_pressed_state);
    EGUI_TEST_RUN(test_card_panel_snapshot_and_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_card_panel_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_card_panel_keyboard_click_listener);
    EGUI_TEST_RUN(test_card_panel_compact_mode_clears_pressed_and_keeps_click_behavior);
    EGUI_TEST_RUN(test_card_panel_read_only_and_disabled_guards_clear_pressed_state);
    EGUI_TEST_RUN(test_card_panel_static_preview_consumes_input_and_keeps_snapshot);
    EGUI_TEST_RUN(test_card_panel_internal_helpers_cover_tone_text_and_pill_width);
    EGUI_TEST_SUITE_END();
}
