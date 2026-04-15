#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_parallax_view.h"

#include "../../HelloCustomWidgets/layout/parallax_view/egui_view_parallax_view.h"
#include "../../HelloCustomWidgets/layout/parallax_view/egui_view_parallax_view.c"

static egui_view_parallax_view_t test_parallax_view;
static egui_view_parallax_view_t preview_parallax_view;
static egui_view_api_t preview_api;
static egui_dim_t last_offset;
static uint8_t last_active_row;
static uint8_t changed_count;

typedef struct
{
    egui_view_parallax_view_metrics_t metrics;
    egui_dim_t offset;
    egui_dim_t content_length;
    egui_dim_t viewport_length;
    egui_dim_t vertical_shift;
    egui_dim_t line_step;
    egui_dim_t page_step;
    uint8_t active_row;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t row_count;
} parallax_view_preview_snapshot_t;

static const egui_view_parallax_view_row_t test_rows[] = {
        {"Hero Banner", "Top", 0, EGUI_VIEW_PARALLAX_VIEW_TONE_ACCENT},
        {"Pinned Deck", "Mid", 180, EGUI_VIEW_PARALLAX_VIEW_TONE_SUCCESS},
        {"Quiet Layer", "Hold", 360, EGUI_VIEW_PARALLAX_VIEW_TONE_NEUTRAL},
        {"System Cards", "Tail", 560, EGUI_VIEW_PARALLAX_VIEW_TONE_WARNING},
};

static void on_parallax_changed(egui_view_t *self, egui_dim_t offset, uint8_t active_row)
{
    EGUI_UNUSED(self);
    changed_count++;
    last_offset = offset;
    last_active_row = active_row;
}

static void reset_listener_state(void)
{
    last_offset = 0;
    last_active_row = EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE;
    changed_count = 0;
}

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void setup_parallax_view(void)
{
    egui_view_parallax_view_init(EGUI_VIEW_OF(&test_parallax_view));
    egui_view_set_size(EGUI_VIEW_OF(&test_parallax_view), 150, 100);
    egui_view_parallax_view_set_rows(EGUI_VIEW_OF(&test_parallax_view), test_rows, 4);
    egui_view_parallax_view_set_content_metrics(EGUI_VIEW_OF(&test_parallax_view), 720, 160);
    egui_view_parallax_view_set_step_size(EGUI_VIEW_OF(&test_parallax_view), 60, 180);
    egui_view_parallax_view_set_vertical_shift(EGUI_VIEW_OF(&test_parallax_view), 18);
    egui_view_parallax_view_set_on_changed_listener(EGUI_VIEW_OF(&test_parallax_view), on_parallax_changed);
    reset_listener_state();
}

static void setup_preview_parallax_view(void)
{
    egui_view_parallax_view_init(EGUI_VIEW_OF(&preview_parallax_view));
    egui_view_set_size(EGUI_VIEW_OF(&preview_parallax_view), 106, 82);
    egui_view_parallax_view_set_rows(EGUI_VIEW_OF(&preview_parallax_view), test_rows, 4);
    egui_view_parallax_view_set_content_metrics(EGUI_VIEW_OF(&preview_parallax_view), 720, 160);
    egui_view_parallax_view_set_step_size(EGUI_VIEW_OF(&preview_parallax_view), 60, 180);
    egui_view_parallax_view_set_vertical_shift(EGUI_VIEW_OF(&preview_parallax_view), 18);
    egui_view_parallax_view_set_offset(EGUI_VIEW_OF(&preview_parallax_view), 180);
    egui_view_parallax_view_set_compact_mode(EGUI_VIEW_OF(&preview_parallax_view), 1);
    egui_view_parallax_view_set_on_changed_listener(EGUI_VIEW_OF(&preview_parallax_view), on_parallax_changed);
    egui_view_parallax_view_override_static_preview_api(EGUI_VIEW_OF(&preview_parallax_view), &preview_api);
    reset_listener_state();
}

static void layout_parallax_view(void)
{
    egui_region_t region;

    region.location.x = 10;
    region.location.y = 20;
    region.size.width = 150;
    region.size.height = 100;
    egui_view_layout(EGUI_VIEW_OF(&test_parallax_view), &region);
    egui_region_copy(&EGUI_VIEW_OF(&test_parallax_view)->region_screen, &region);
}

static void layout_preview_parallax_view(void)
{
    egui_region_t region;

    region.location.x = 10;
    region.location.y = 20;
    region.size.width = 106;
    region.size.height = 82;
    egui_view_layout(EGUI_VIEW_OF(&preview_parallax_view), &region);
    egui_region_copy(&EGUI_VIEW_OF(&preview_parallax_view)->region_screen, &region);
}

static int send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return EGUI_VIEW_OF(&test_parallax_view)->api->on_touch_event(EGUI_VIEW_OF(&test_parallax_view), &event);
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

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return EGUI_VIEW_OF(&preview_parallax_view)->api->on_touch_event(EGUI_VIEW_OF(&preview_parallax_view), &event);
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_parallax_view), key_code);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_parallax_view), key_code);
}

static void capture_preview_snapshot(parallax_view_preview_snapshot_t *snapshot)
{
    parallax_view_get_metrics(&preview_parallax_view, EGUI_VIEW_OF(&preview_parallax_view), &snapshot->metrics);
    snapshot->offset = egui_view_parallax_view_get_offset(EGUI_VIEW_OF(&preview_parallax_view));
    snapshot->content_length = egui_view_parallax_view_get_content_length(EGUI_VIEW_OF(&preview_parallax_view));
    snapshot->viewport_length = egui_view_parallax_view_get_viewport_length(EGUI_VIEW_OF(&preview_parallax_view));
    snapshot->vertical_shift = egui_view_parallax_view_get_vertical_shift(EGUI_VIEW_OF(&preview_parallax_view));
    snapshot->line_step = preview_parallax_view.line_step;
    snapshot->page_step = preview_parallax_view.page_step;
    snapshot->active_row = egui_view_parallax_view_get_active_row(EGUI_VIEW_OF(&preview_parallax_view));
    snapshot->compact_mode = preview_parallax_view.compact_mode;
    snapshot->read_only_mode = preview_parallax_view.read_only_mode;
    snapshot->row_count = preview_parallax_view.row_count;
}

static void assert_preview_state_unchanged(const parallax_view_preview_snapshot_t *snapshot)
{
    egui_view_parallax_view_metrics_t metrics;
    uint8_t index;

    parallax_view_get_metrics(&preview_parallax_view, EGUI_VIEW_OF(&preview_parallax_view), &metrics);
    assert_region_equal(&snapshot->metrics.content_region, &metrics.content_region);
    assert_region_equal(&snapshot->metrics.hero_region, &metrics.hero_region);
    assert_region_equal(&snapshot->metrics.title_region, &metrics.title_region);
    assert_region_equal(&snapshot->metrics.subtitle_region, &metrics.subtitle_region);
    assert_region_equal(&snapshot->metrics.progress_region, &metrics.progress_region);
    assert_region_equal(&snapshot->metrics.footer_region, &metrics.footer_region);
    for (index = 0; index < EGUI_VIEW_PARALLAX_VIEW_MAX_ROWS; index++)
    {
        assert_region_equal(&snapshot->metrics.row_regions[index], &metrics.row_regions[index]);
    }
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->offset, egui_view_parallax_view_get_offset(EGUI_VIEW_OF(&preview_parallax_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->content_length, egui_view_parallax_view_get_content_length(EGUI_VIEW_OF(&preview_parallax_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->viewport_length, egui_view_parallax_view_get_viewport_length(EGUI_VIEW_OF(&preview_parallax_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->vertical_shift, egui_view_parallax_view_get_vertical_shift(EGUI_VIEW_OF(&preview_parallax_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->line_step, preview_parallax_view.line_step);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->page_step, preview_parallax_view.page_step);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->active_row, egui_view_parallax_view_get_active_row(EGUI_VIEW_OF(&preview_parallax_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_parallax_view.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_parallax_view.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->row_count, preview_parallax_view.row_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, last_offset);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE, last_active_row);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_parallax_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE, preview_parallax_view.pressed_row);
}

static void test_parallax_view_font_setters_clear_pressed_state(void)
{
    setup_parallax_view();

    EGUI_VIEW_OF(&test_parallax_view)->is_pressed = true;
    test_parallax_view.pressed_row = 1;
    egui_view_parallax_view_set_font(EGUI_VIEW_OF(&test_parallax_view), NULL);
    EGUI_TEST_ASSERT_TRUE(test_parallax_view.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_parallax_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE, test_parallax_view.pressed_row);

    EGUI_VIEW_OF(&test_parallax_view)->is_pressed = true;
    test_parallax_view.pressed_row = 2;
    egui_view_parallax_view_set_meta_font(EGUI_VIEW_OF(&test_parallax_view), NULL);
    EGUI_TEST_ASSERT_TRUE(test_parallax_view.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_parallax_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE, test_parallax_view.pressed_row);
}

static void test_parallax_view_clamps_metrics_and_offset(void)
{
    setup_parallax_view();

    egui_view_parallax_view_set_content_metrics(EGUI_VIEW_OF(&test_parallax_view), 300, 400);
    EGUI_TEST_ASSERT_EQUAL_INT(300, egui_view_parallax_view_get_content_length(EGUI_VIEW_OF(&test_parallax_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(400, egui_view_parallax_view_get_viewport_length(EGUI_VIEW_OF(&test_parallax_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_parallax_view_get_max_offset(EGUI_VIEW_OF(&test_parallax_view)));
    egui_view_parallax_view_set_offset(EGUI_VIEW_OF(&test_parallax_view), 80);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_parallax_view_get_offset(EGUI_VIEW_OF(&test_parallax_view)));

    reset_listener_state();
    egui_view_parallax_view_set_content_metrics(EGUI_VIEW_OF(&test_parallax_view), 720, 160);
    egui_view_parallax_view_set_offset(EGUI_VIEW_OF(&test_parallax_view), 999);
    EGUI_TEST_ASSERT_EQUAL_INT(560, egui_view_parallax_view_get_offset(EGUI_VIEW_OF(&test_parallax_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(560, last_offset);
    EGUI_TEST_ASSERT_EQUAL_INT(3, last_active_row);
}

static void test_parallax_view_active_row_tracks_offset(void)
{
    setup_parallax_view();

    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_parallax_view_get_active_row(EGUI_VIEW_OF(&test_parallax_view)));
    egui_view_parallax_view_set_offset(EGUI_VIEW_OF(&test_parallax_view), 200);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_parallax_view_get_active_row(EGUI_VIEW_OF(&test_parallax_view)));
    egui_view_parallax_view_set_offset(EGUI_VIEW_OF(&test_parallax_view), 400);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_parallax_view_get_active_row(EGUI_VIEW_OF(&test_parallax_view)));
    egui_view_parallax_view_set_offset(EGUI_VIEW_OF(&test_parallax_view), 560);
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_parallax_view_get_active_row(EGUI_VIEW_OF(&test_parallax_view)));
}

static void test_parallax_view_keyboard_navigation(void)
{
    setup_parallax_view();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(60, egui_view_parallax_view_get_offset(EGUI_VIEW_OF(&test_parallax_view)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_UP));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_parallax_view_get_offset(EGUI_VIEW_OF(&test_parallax_view)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_PLUS));
    EGUI_TEST_ASSERT_EQUAL_INT(180, egui_view_parallax_view_get_offset(EGUI_VIEW_OF(&test_parallax_view)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_MINUS));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_parallax_view_get_offset(EGUI_VIEW_OF(&test_parallax_view)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(560, egui_view_parallax_view_get_offset(EGUI_VIEW_OF(&test_parallax_view)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_parallax_view_get_offset(EGUI_VIEW_OF(&test_parallax_view)));
}

static void test_parallax_view_touch_selects_anchor(void)
{
    egui_region_t region;

    setup_parallax_view();
    layout_parallax_view();
    EGUI_TEST_ASSERT_TRUE(egui_view_parallax_view_get_row_region(EGUI_VIEW_OF(&test_parallax_view), 2, &region));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(360, egui_view_parallax_view_get_offset(EGUI_VIEW_OF(&test_parallax_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_parallax_view_get_active_row(EGUI_VIEW_OF(&test_parallax_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(360, last_offset);
}

static void test_parallax_view_release_on_different_target_and_cancel_do_not_commit(void)
{
    egui_region_t region_a;
    egui_region_t region_b;

    setup_parallax_view();
    layout_parallax_view();
    EGUI_TEST_ASSERT_TRUE(egui_view_parallax_view_get_row_region(EGUI_VIEW_OF(&test_parallax_view), 1, &region_a));
    EGUI_TEST_ASSERT_TRUE(egui_view_parallax_view_get_row_region(EGUI_VIEW_OF(&test_parallax_view), 2, &region_b));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region_a.location.x + region_a.size.width / 2,
                                     region_a.location.y + region_a.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_parallax_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_parallax_view.pressed_row);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region_b.location.x + region_b.size.width / 2,
                                     region_b.location.y + region_b.size.height / 2));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_parallax_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE, test_parallax_view.pressed_row);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_parallax_view_get_offset(EGUI_VIEW_OF(&test_parallax_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region_b.location.x + region_b.size.width / 2,
                                     region_b.location.y + region_b.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_parallax_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_parallax_view.pressed_row);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, region_b.location.x + region_b.size.width / 2,
                                     region_b.location.y + region_b.size.height / 2));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_parallax_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE, test_parallax_view.pressed_row);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_parallax_view_get_offset(EGUI_VIEW_OF(&test_parallax_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
}

static void test_parallax_view_compact_mode_clears_pressed_and_ignores_input(void)
{
    egui_region_t region;

    setup_parallax_view();
    layout_parallax_view();
    EGUI_TEST_ASSERT_TRUE(egui_view_parallax_view_get_row_region(EGUI_VIEW_OF(&test_parallax_view), 1, &region));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_parallax_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_parallax_view.pressed_row);

    egui_view_parallax_view_set_compact_mode(EGUI_VIEW_OF(&test_parallax_view), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_parallax_view.compact_mode);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_parallax_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE, test_parallax_view.pressed_row);

    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_parallax_view_get_offset(EGUI_VIEW_OF(&test_parallax_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_parallax_view_set_compact_mode(EGUI_VIEW_OF(&test_parallax_view), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_parallax_view.compact_mode);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(180, egui_view_parallax_view_get_offset(EGUI_VIEW_OF(&test_parallax_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
}

static void test_parallax_view_read_only_mode_ignores_input(void)
{
    egui_region_t region;

    setup_parallax_view();
    egui_view_parallax_view_set_read_only_mode(EGUI_VIEW_OF(&test_parallax_view), 1);
    layout_parallax_view();
    EGUI_TEST_ASSERT_TRUE(egui_view_parallax_view_get_row_region(EGUI_VIEW_OF(&test_parallax_view), 1, &region));

    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_PLUS));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_parallax_view_get_offset(EGUI_VIEW_OF(&test_parallax_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
}

static void test_parallax_view_read_only_mode_clears_pressed_state(void)
{
    egui_region_t region;

    setup_parallax_view();
    layout_parallax_view();
    EGUI_TEST_ASSERT_TRUE(egui_view_parallax_view_get_row_region(EGUI_VIEW_OF(&test_parallax_view), 1, &region));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_parallax_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_parallax_view.pressed_row);

    egui_view_parallax_view_set_read_only_mode(EGUI_VIEW_OF(&test_parallax_view), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_parallax_view.read_only_mode);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_parallax_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE, test_parallax_view.pressed_row);

    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    test_parallax_view.pressed_row = 2;
    EGUI_VIEW_OF(&test_parallax_view)->is_pressed = true;
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_parallax_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE, test_parallax_view.pressed_row);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_parallax_view_get_offset(EGUI_VIEW_OF(&test_parallax_view)));

    egui_view_parallax_view_set_read_only_mode(EGUI_VIEW_OF(&test_parallax_view), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_parallax_view.read_only_mode);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(180, egui_view_parallax_view_get_offset(EGUI_VIEW_OF(&test_parallax_view)));
}

static void test_parallax_view_disabled_ignores_input_and_clears_pressed_state(void)
{
    egui_region_t region;

    setup_parallax_view();
    layout_parallax_view();
    EGUI_TEST_ASSERT_TRUE(egui_view_parallax_view_get_row_region(EGUI_VIEW_OF(&test_parallax_view), 2, &region));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_parallax_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_parallax_view.pressed_row);

    egui_view_set_enable(EGUI_VIEW_OF(&test_parallax_view), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    test_parallax_view.pressed_row = 1;
    EGUI_VIEW_OF(&test_parallax_view)->is_pressed = true;
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_parallax_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PARALLAX_VIEW_INDEX_NONE, test_parallax_view.pressed_row);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_parallax_view_get_offset(EGUI_VIEW_OF(&test_parallax_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_set_enable(EGUI_VIEW_OF(&test_parallax_view), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(360, egui_view_parallax_view_get_offset(EGUI_VIEW_OF(&test_parallax_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
}

static void test_parallax_view_row_region_is_exposed(void)
{
    egui_region_t region;

    setup_parallax_view();
    layout_parallax_view();
    EGUI_TEST_ASSERT_TRUE(egui_view_parallax_view_get_row_region(EGUI_VIEW_OF(&test_parallax_view), 0, &region));
    EGUI_TEST_ASSERT_TRUE(region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(region.size.height > 0);
    EGUI_TEST_ASSERT_FALSE(egui_view_parallax_view_get_row_region(EGUI_VIEW_OF(&test_parallax_view), 4, &region));
}

static void test_parallax_view_static_preview_consumes_input_and_keeps_state(void)
{
    egui_dim_t x;
    egui_dim_t y;
    parallax_view_preview_snapshot_t initial_snapshot;

    setup_preview_parallax_view();
    layout_preview_parallax_view();
    capture_preview_snapshot(&initial_snapshot);
    x = initial_snapshot.metrics.row_regions[2].location.x + initial_snapshot.metrics.row_regions[2].size.width / 2;
    y = initial_snapshot.metrics.row_regions[2].location.y + initial_snapshot.metrics.row_regions[2].size.height / 2;

    EGUI_VIEW_OF(&preview_parallax_view)->is_pressed = true;
    preview_parallax_view.pressed_row = 2;
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_preview_state_unchanged(&initial_snapshot);

    EGUI_VIEW_OF(&preview_parallax_view)->is_pressed = true;
    preview_parallax_view.pressed_row = 2;
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    assert_preview_state_unchanged(&initial_snapshot);

    EGUI_VIEW_OF(&preview_parallax_view)->is_pressed = true;
    preview_parallax_view.pressed_row = 1;
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_END));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_parallax_view_run(void)
{
    EGUI_TEST_SUITE_BEGIN(parallax_view);
    EGUI_TEST_RUN(test_parallax_view_font_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_parallax_view_clamps_metrics_and_offset);
    EGUI_TEST_RUN(test_parallax_view_active_row_tracks_offset);
    EGUI_TEST_RUN(test_parallax_view_keyboard_navigation);
    EGUI_TEST_RUN(test_parallax_view_touch_selects_anchor);
    EGUI_TEST_RUN(test_parallax_view_release_on_different_target_and_cancel_do_not_commit);
    EGUI_TEST_RUN(test_parallax_view_compact_mode_clears_pressed_and_ignores_input);
    EGUI_TEST_RUN(test_parallax_view_read_only_mode_ignores_input);
    EGUI_TEST_RUN(test_parallax_view_read_only_mode_clears_pressed_state);
    EGUI_TEST_RUN(test_parallax_view_disabled_ignores_input_and_clears_pressed_state);
    EGUI_TEST_RUN(test_parallax_view_row_region_is_exposed);
    EGUI_TEST_RUN(test_parallax_view_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
