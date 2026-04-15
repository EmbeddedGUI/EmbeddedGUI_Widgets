#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_calendar_view.h"

#include "../../HelloCustomWidgets/input/calendar_view/egui_view_calendar_view.h"
#include "../../HelloCustomWidgets/input/calendar_view/egui_view_calendar_view.c"

static egui_view_calendar_view_t test_calendar_view;
static egui_view_calendar_view_t preview_calendar_view;
static egui_view_api_t preview_api;

typedef struct
{
    egui_region_t region_screen;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    const char *label;
    const char *helper;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t today_color;
    uint16_t display_year;
    uint16_t selection_year;
    uint16_t committed_year;
    uint16_t today_year;
    uint8_t display_month;
    uint8_t selection_month;
    uint8_t committed_month;
    uint8_t start_day;
    uint8_t end_day;
    uint8_t committed_start_day;
    uint8_t committed_end_day;
    uint8_t focus_day;
    uint8_t anchor_day;
    uint8_t today_month;
    uint8_t today_day;
    uint8_t first_day_of_week;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t editing_range;
    uint8_t current_part;
} calendar_view_preview_snapshot_t;

static uint8_t calendar_view_test_day_of_week(uint16_t year, uint8_t month, uint8_t day)
{
    static const int offsets[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    uint16_t y = year;

    if (month < 3)
    {
        y--;
    }
    return (uint8_t)((y + y / 4 - y / 100 + y / 400 + offsets[month - 1] + day) % 7);
}

static void setup_calendar_view(uint16_t year, uint8_t month, uint8_t start_day, uint8_t end_day)
{
    egui_view_calendar_view_init(EGUI_VIEW_OF(&test_calendar_view));
    egui_view_set_size(EGUI_VIEW_OF(&test_calendar_view), 196, 144);
    egui_view_calendar_view_set_display_month(EGUI_VIEW_OF(&test_calendar_view), year, month);
    egui_view_calendar_view_set_range(EGUI_VIEW_OF(&test_calendar_view), year, month, start_day, end_day);
}

static void layout_calendar_view(egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(EGUI_VIEW_OF(&test_calendar_view), &region);
    egui_region_copy(&EGUI_VIEW_OF(&test_calendar_view)->region_screen, &region);
}

static void setup_preview_calendar_view(void)
{
    egui_view_calendar_view_init(EGUI_VIEW_OF(&preview_calendar_view));
    egui_view_set_size(EGUI_VIEW_OF(&preview_calendar_view), 104, 50);
    egui_view_calendar_view_set_display_month(EGUI_VIEW_OF(&preview_calendar_view), 2026, 5);
    egui_view_calendar_view_set_range(EGUI_VIEW_OF(&preview_calendar_view), 2026, 5, 5, 8);
    egui_view_calendar_view_set_compact_mode(EGUI_VIEW_OF(&preview_calendar_view), 1);
    egui_view_calendar_view_override_static_preview_api(EGUI_VIEW_OF(&preview_calendar_view), &preview_api);
}

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void layout_preview_calendar_view(void)
{
    egui_region_t region;

    region.location.x = 10;
    region.location.y = 20;
    region.size.width = 104;
    region.size.height = 50;
    egui_view_layout(EGUI_VIEW_OF(&preview_calendar_view), &region);
    egui_region_copy(&EGUI_VIEW_OF(&preview_calendar_view)->region_screen, &region);
}

static int send_touch_event(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return EGUI_VIEW_OF(&test_calendar_view)->api->on_touch_event(EGUI_VIEW_OF(&test_calendar_view), &event);
}

static int dispatch_key_event_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->dispatch_key_event(view, &event);
}

static int send_key_event(uint8_t type, uint8_t key_code)
{
    return dispatch_key_event_to_view(EGUI_VIEW_OF(&test_calendar_view), type, key_code);
}

static int send_preview_touch_event(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return EGUI_VIEW_OF(&preview_calendar_view)->api->on_touch_event(EGUI_VIEW_OF(&preview_calendar_view), &event);
}

static int send_preview_key_event(uint8_t type, uint8_t key_code)
{
    return dispatch_key_event_to_view(EGUI_VIEW_OF(&preview_calendar_view), type, key_code);
}

static void get_day_center(uint8_t day, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t grid_region;
    uint8_t first_dow = calendar_view_test_day_of_week(egui_view_calendar_view_get_display_year(EGUI_VIEW_OF(&test_calendar_view)),
                                                       egui_view_calendar_view_get_display_month(EGUI_VIEW_OF(&test_calendar_view)), 1);
    uint8_t start_cell = (uint8_t)((first_dow - egui_view_calendar_view_get_first_day_of_week(EGUI_VIEW_OF(&test_calendar_view)) + 7) % 7);
    uint8_t pos = (uint8_t)(start_cell + day - 1);
    uint8_t col = (uint8_t)(pos % 7);
    uint8_t row = (uint8_t)(pos / 7);
    egui_dim_t cell_w;
    egui_dim_t cell_h;

    EGUI_TEST_ASSERT_TRUE(egui_view_calendar_view_get_part_region(EGUI_VIEW_OF(&test_calendar_view), EGUI_VIEW_CALENDAR_VIEW_PART_GRID, &grid_region));
    cell_w = grid_region.size.width / 7;
    cell_h = grid_region.size.height / 6;
    *x = grid_region.location.x + col * cell_w + cell_w / 2;
    *y = grid_region.location.y + row * cell_h + cell_h / 2;
}

static void capture_preview_snapshot(calendar_view_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_calendar_view)->region_screen;
    snapshot->font = preview_calendar_view.font;
    snapshot->meta_font = preview_calendar_view.meta_font;
    snapshot->label = preview_calendar_view.label;
    snapshot->helper = preview_calendar_view.helper;
    snapshot->surface_color = preview_calendar_view.surface_color;
    snapshot->border_color = preview_calendar_view.border_color;
    snapshot->text_color = preview_calendar_view.text_color;
    snapshot->muted_text_color = preview_calendar_view.muted_text_color;
    snapshot->accent_color = preview_calendar_view.accent_color;
    snapshot->today_color = preview_calendar_view.today_color;
    snapshot->display_year = preview_calendar_view.display_year;
    snapshot->selection_year = preview_calendar_view.selection_year;
    snapshot->committed_year = preview_calendar_view.committed_year;
    snapshot->today_year = preview_calendar_view.today_year;
    snapshot->display_month = preview_calendar_view.display_month;
    snapshot->selection_month = preview_calendar_view.selection_month;
    snapshot->committed_month = preview_calendar_view.committed_month;
    snapshot->start_day = preview_calendar_view.start_day;
    snapshot->end_day = preview_calendar_view.end_day;
    snapshot->committed_start_day = preview_calendar_view.committed_start_day;
    snapshot->committed_end_day = preview_calendar_view.committed_end_day;
    snapshot->focus_day = preview_calendar_view.focus_day;
    snapshot->anchor_day = preview_calendar_view.anchor_day;
    snapshot->today_month = preview_calendar_view.today_month;
    snapshot->today_day = preview_calendar_view.today_day;
    snapshot->first_day_of_week = preview_calendar_view.first_day_of_week;
    snapshot->compact_mode = preview_calendar_view.compact_mode;
    snapshot->read_only_mode = preview_calendar_view.read_only_mode;
    snapshot->editing_range = preview_calendar_view.editing_range;
    snapshot->current_part = preview_calendar_view.current_part;
}

static void assert_preview_state_unchanged(const calendar_view_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_calendar_view)->region_screen);
    EGUI_TEST_ASSERT_TRUE(preview_calendar_view.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_calendar_view.meta_font == snapshot->meta_font);
    EGUI_TEST_ASSERT_TRUE(preview_calendar_view.label == snapshot->label);
    EGUI_TEST_ASSERT_TRUE(preview_calendar_view.helper == snapshot->helper);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_calendar_view.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_calendar_view.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_calendar_view.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_calendar_view.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_calendar_view.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->today_color.full, preview_calendar_view.today_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->display_year, preview_calendar_view.display_year);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->selection_year, preview_calendar_view.selection_year);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->committed_year, preview_calendar_view.committed_year);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->today_year, preview_calendar_view.today_year);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->display_month, preview_calendar_view.display_month);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->selection_month, preview_calendar_view.selection_month);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->committed_month, preview_calendar_view.committed_month);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->start_day, preview_calendar_view.start_day);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->end_day, preview_calendar_view.end_day);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->committed_start_day, preview_calendar_view.committed_start_day);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->committed_end_day, preview_calendar_view.committed_end_day);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->focus_day, preview_calendar_view.focus_day);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->anchor_day, preview_calendar_view.anchor_day);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->today_month, preview_calendar_view.today_month);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->today_day, preview_calendar_view.today_day);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->first_day_of_week, preview_calendar_view.first_day_of_week);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_calendar_view.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_calendar_view.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->editing_range, preview_calendar_view.editing_range);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_part, preview_calendar_view.current_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_calendar_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CALENDAR_VIEW_PART_NONE, preview_calendar_view.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_calendar_view.pressed_day);
}

static void test_calendar_view_tab_cycles_parts(void)
{
    setup_calendar_view(2026, 3, 9, 11);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CALENDAR_VIEW_PART_GRID, egui_view_calendar_view_get_current_part(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_TRUE(egui_view_calendar_view_handle_navigation_key(EGUI_VIEW_OF(&test_calendar_view), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CALENDAR_VIEW_PART_PREV, egui_view_calendar_view_get_current_part(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_TRUE(egui_view_calendar_view_handle_navigation_key(EGUI_VIEW_OF(&test_calendar_view), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CALENDAR_VIEW_PART_NEXT, egui_view_calendar_view_get_current_part(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_TRUE(egui_view_calendar_view_handle_navigation_key(EGUI_VIEW_OF(&test_calendar_view), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CALENDAR_VIEW_PART_GRID, egui_view_calendar_view_get_current_part(EGUI_VIEW_OF(&test_calendar_view)));
}

static void test_calendar_view_keyboard_range_commit(void)
{
    setup_calendar_view(2026, 3, 9, 11);
    EGUI_TEST_ASSERT_FALSE(egui_view_calendar_view_get_editing_range(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_TRUE(egui_view_calendar_view_handle_navigation_key(EGUI_VIEW_OF(&test_calendar_view), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(egui_view_calendar_view_get_editing_range(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(11, egui_view_calendar_view_get_start_day(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(11, egui_view_calendar_view_get_end_day(EGUI_VIEW_OF(&test_calendar_view)));

    EGUI_TEST_ASSERT_TRUE(egui_view_calendar_view_handle_navigation_key(EGUI_VIEW_OF(&test_calendar_view), EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_TRUE(egui_view_calendar_view_handle_navigation_key(EGUI_VIEW_OF(&test_calendar_view), EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(11, egui_view_calendar_view_get_start_day(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(13, egui_view_calendar_view_get_end_day(EGUI_VIEW_OF(&test_calendar_view)));

    EGUI_TEST_ASSERT_TRUE(egui_view_calendar_view_handle_navigation_key(EGUI_VIEW_OF(&test_calendar_view), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(egui_view_calendar_view_get_editing_range(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(11, egui_view_calendar_view_get_start_day(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(13, egui_view_calendar_view_get_end_day(EGUI_VIEW_OF(&test_calendar_view)));
}

static void test_calendar_view_plus_minus_shift_display_month(void)
{
    setup_calendar_view(2026, 3, 9, 11);
    EGUI_TEST_ASSERT_TRUE(egui_view_calendar_view_handle_navigation_key(EGUI_VIEW_OF(&test_calendar_view), EGUI_KEY_CODE_PLUS));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_calendar_view_get_display_month(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_TRUE(egui_view_calendar_view_handle_navigation_key(EGUI_VIEW_OF(&test_calendar_view), EGUI_KEY_CODE_MINUS));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_calendar_view_get_display_month(EGUI_VIEW_OF(&test_calendar_view)));
}

static void test_calendar_view_touch_two_taps_commits_range(void)
{
    egui_dim_t x1;
    egui_dim_t y1;
    egui_dim_t x2;
    egui_dim_t y2;

    setup_calendar_view(2026, 3, 9, 11);
    layout_calendar_view(10, 20, 196, 144);
    get_day_center(9, &x1, &y1);
    get_day_center(13, &x2, &y2);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x1, y1));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x1, y1));
    EGUI_TEST_ASSERT_TRUE(egui_view_calendar_view_get_editing_range(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(9, egui_view_calendar_view_get_start_day(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(9, egui_view_calendar_view_get_end_day(EGUI_VIEW_OF(&test_calendar_view)));

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x2, y2));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x2, y2));
    EGUI_TEST_ASSERT_FALSE(egui_view_calendar_view_get_editing_range(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(9, egui_view_calendar_view_get_start_day(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(13, egui_view_calendar_view_get_end_day(EGUI_VIEW_OF(&test_calendar_view)));
}

static void test_calendar_view_release_requires_same_day(void)
{
    egui_dim_t x_start;
    egui_dim_t y_start;
    egui_dim_t x_end;
    egui_dim_t y_end;

    setup_calendar_view(2026, 3, 9, 11);
    layout_calendar_view(10, 20, 196, 144);
    get_day_center(10, &x_start, &y_start);
    get_day_center(12, &x_end, &y_end);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x_start, y_start));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CALENDAR_VIEW_PART_GRID, test_calendar_view.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(10, test_calendar_view.pressed_day);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_calendar_view)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_MOVE, x_end, y_end));
    EGUI_TEST_ASSERT_EQUAL_INT(10, test_calendar_view.pressed_day);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_calendar_view)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x_end, y_end));
    EGUI_TEST_ASSERT_FALSE(egui_view_calendar_view_get_editing_range(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(9, egui_view_calendar_view_get_start_day(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(11, egui_view_calendar_view_get_end_day(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CALENDAR_VIEW_PART_NONE, test_calendar_view.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_calendar_view)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x_start, y_start));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_MOVE, x_end, y_end));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_MOVE, x_start, y_start));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_calendar_view)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x_start, y_start));
    EGUI_TEST_ASSERT_TRUE(egui_view_calendar_view_get_editing_range(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(10, egui_view_calendar_view_get_start_day(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(10, egui_view_calendar_view_get_end_day(EGUI_VIEW_OF(&test_calendar_view)));

    get_day_center(13, &x_start, &y_start);
    get_day_center(14, &x_end, &y_end);
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x_start, y_start));
    EGUI_TEST_ASSERT_EQUAL_INT(13, test_calendar_view.pressed_day);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_calendar_view)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_MOVE, x_end, y_end));
    EGUI_TEST_ASSERT_EQUAL_INT(13, test_calendar_view.pressed_day);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_calendar_view)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x_end, y_end));
    EGUI_TEST_ASSERT_TRUE(egui_view_calendar_view_get_editing_range(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(10, egui_view_calendar_view_get_start_day(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(10, egui_view_calendar_view_get_end_day(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CALENDAR_VIEW_PART_NONE, test_calendar_view.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_calendar_view)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x_start, y_start));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_MOVE, x_end, y_end));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_MOVE, x_start, y_start));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_calendar_view)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x_start, y_start));
    EGUI_TEST_ASSERT_FALSE(egui_view_calendar_view_get_editing_range(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(10, egui_view_calendar_view_get_start_day(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(13, egui_view_calendar_view_get_end_day(EGUI_VIEW_OF(&test_calendar_view)));
}

static void test_calendar_view_touch_prev_button_changes_month(void)
{
    egui_region_t prev_region;

    setup_calendar_view(2026, 3, 9, 11);
    layout_calendar_view(10, 20, 196, 144);
    EGUI_TEST_ASSERT_TRUE(egui_view_calendar_view_get_part_region(EGUI_VIEW_OF(&test_calendar_view), EGUI_VIEW_CALENDAR_VIEW_PART_PREV, &prev_region));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, prev_region.location.x + prev_region.size.width / 2,
                                           prev_region.location.y + prev_region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, prev_region.location.x + prev_region.size.width / 2,
                                           prev_region.location.y + prev_region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_calendar_view_get_display_month(EGUI_VIEW_OF(&test_calendar_view)));
}

static void test_calendar_view_setters_clear_pressed_state(void)
{
    setup_calendar_view(2026, 3, 9, 11);
    layout_calendar_view(10, 20, 196, 144);

    EGUI_VIEW_OF(&test_calendar_view)->is_pressed = 1;
    test_calendar_view.pressed_part = EGUI_VIEW_CALENDAR_VIEW_PART_GRID;
    test_calendar_view.pressed_day = 10;
    egui_view_calendar_view_set_range(EGUI_VIEW_OF(&test_calendar_view), 2026, 3, 12, 15);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_calendar_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CALENDAR_VIEW_PART_NONE, test_calendar_view.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_calendar_view.pressed_day);
    EGUI_TEST_ASSERT_EQUAL_INT(12, egui_view_calendar_view_get_start_day(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(15, egui_view_calendar_view_get_end_day(EGUI_VIEW_OF(&test_calendar_view)));

    EGUI_VIEW_OF(&test_calendar_view)->is_pressed = 1;
    test_calendar_view.pressed_part = EGUI_VIEW_CALENDAR_VIEW_PART_PREV;
    test_calendar_view.pressed_day = 0;
    egui_view_calendar_view_set_display_month(EGUI_VIEW_OF(&test_calendar_view), 2026, 4);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_calendar_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CALENDAR_VIEW_PART_NONE, test_calendar_view.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_calendar_view.pressed_day);
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_calendar_view_get_display_month(EGUI_VIEW_OF(&test_calendar_view)));

    EGUI_VIEW_OF(&test_calendar_view)->is_pressed = 1;
    test_calendar_view.pressed_part = EGUI_VIEW_CALENDAR_VIEW_PART_NEXT;
    test_calendar_view.pressed_day = 0;
    egui_view_calendar_view_set_palette(EGUI_VIEW_OF(&test_calendar_view), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                        EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F6CBD));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_calendar_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CALENDAR_VIEW_PART_NONE, test_calendar_view.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_calendar_view.pressed_day);

    EGUI_VIEW_OF(&test_calendar_view)->is_pressed = 1;
    test_calendar_view.pressed_part = EGUI_VIEW_CALENDAR_VIEW_PART_GRID;
    test_calendar_view.pressed_day = 15;
    egui_view_calendar_view_set_current_part(EGUI_VIEW_OF(&test_calendar_view), EGUI_VIEW_CALENDAR_VIEW_PART_PREV);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_calendar_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CALENDAR_VIEW_PART_NONE, test_calendar_view.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_calendar_view.pressed_day);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CALENDAR_VIEW_PART_PREV, egui_view_calendar_view_get_current_part(EGUI_VIEW_OF(&test_calendar_view)));
}

static void test_calendar_view_set_range_clamps_order(void)
{
    setup_calendar_view(2026, 3, 9, 11);
    egui_view_calendar_view_set_range(EGUI_VIEW_OF(&test_calendar_view), 2026, 2, 29, 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2026, egui_view_calendar_view_get_selection_year(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_calendar_view_get_selection_month(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_calendar_view_get_start_day(EGUI_VIEW_OF(&test_calendar_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(28, egui_view_calendar_view_get_end_day(EGUI_VIEW_OF(&test_calendar_view)));
}

static void test_calendar_view_read_only_and_compact_ignore_interaction(void)
{
    egui_region_t grid_region;

    setup_calendar_view(2026, 3, 9, 11);
    egui_view_calendar_view_set_read_only_mode(EGUI_VIEW_OF(&test_calendar_view), 1);
    EGUI_TEST_ASSERT_FALSE(egui_view_calendar_view_handle_navigation_key(EGUI_VIEW_OF(&test_calendar_view), EGUI_KEY_CODE_LEFT));
    EGUI_VIEW_OF(&test_calendar_view)->is_pressed = 1;
    test_calendar_view.pressed_part = EGUI_VIEW_CALENDAR_VIEW_PART_GRID;
    test_calendar_view.pressed_day = 9;
    EGUI_TEST_ASSERT_FALSE(send_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_calendar_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CALENDAR_VIEW_PART_NONE, test_calendar_view.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_calendar_view.pressed_day);

    setup_calendar_view(2026, 3, 9, 11);
    egui_view_calendar_view_set_compact_mode(EGUI_VIEW_OF(&test_calendar_view), 1);
    layout_calendar_view(10, 20, 196, 144);
    EGUI_TEST_ASSERT_FALSE(egui_view_calendar_view_handle_navigation_key(EGUI_VIEW_OF(&test_calendar_view), EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_FALSE(egui_view_calendar_view_get_part_region(EGUI_VIEW_OF(&test_calendar_view), EGUI_VIEW_CALENDAR_VIEW_PART_GRID, &grid_region));
    EGUI_VIEW_OF(&test_calendar_view)->is_pressed = 1;
    test_calendar_view.pressed_part = EGUI_VIEW_CALENDAR_VIEW_PART_GRID;
    test_calendar_view.pressed_day = 9;
    EGUI_TEST_ASSERT_FALSE(send_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_calendar_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CALENDAR_VIEW_PART_NONE, test_calendar_view.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_calendar_view.pressed_day);
}

static void test_calendar_view_static_preview_consumes_input_and_keeps_state(void)
{
    calendar_view_preview_snapshot_t initial_snapshot;
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_calendar_view();
    layout_preview_calendar_view();
    capture_preview_snapshot(&initial_snapshot);

    x = EGUI_VIEW_OF(&preview_calendar_view)->region_screen.location.x + EGUI_VIEW_OF(&preview_calendar_view)->region_screen.size.width / 2;
    y = EGUI_VIEW_OF(&preview_calendar_view)->region_screen.location.y + EGUI_VIEW_OF(&preview_calendar_view)->region_screen.size.height / 2;

    EGUI_VIEW_OF(&preview_calendar_view)->is_pressed = 1;
    preview_calendar_view.pressed_part = EGUI_VIEW_CALENDAR_VIEW_PART_GRID;
    preview_calendar_view.pressed_day = 6;
    EGUI_TEST_ASSERT_TRUE(send_preview_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_preview_state_unchanged(&initial_snapshot);

    EGUI_VIEW_OF(&preview_calendar_view)->is_pressed = 1;
    preview_calendar_view.pressed_part = EGUI_VIEW_CALENDAR_VIEW_PART_PREV;
    preview_calendar_view.pressed_day = 0;
    EGUI_TEST_ASSERT_TRUE(send_preview_key_event(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_TRUE(send_preview_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_LEFT));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_calendar_view_run(void)
{
    EGUI_TEST_SUITE_BEGIN(calendar_view);
    EGUI_TEST_RUN(test_calendar_view_tab_cycles_parts);
    EGUI_TEST_RUN(test_calendar_view_keyboard_range_commit);
    EGUI_TEST_RUN(test_calendar_view_plus_minus_shift_display_month);
    EGUI_TEST_RUN(test_calendar_view_touch_two_taps_commits_range);
    EGUI_TEST_RUN(test_calendar_view_release_requires_same_day);
    EGUI_TEST_RUN(test_calendar_view_touch_prev_button_changes_month);
    EGUI_TEST_RUN(test_calendar_view_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_calendar_view_set_range_clamps_order);
    EGUI_TEST_RUN(test_calendar_view_read_only_and_compact_ignore_interaction);
    EGUI_TEST_RUN(test_calendar_view_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
