#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_date_picker.h"

#include "../../HelloCustomWidgets/input/date_picker/egui_view_date_picker.h"
#include "../../HelloCustomWidgets/input/date_picker/egui_view_date_picker.c"

typedef struct date_picker_preview_snapshot date_picker_preview_snapshot_t;
struct date_picker_preview_snapshot
{
    egui_region_t region_screen;
    egui_view_on_date_picker_changed_listener_t on_date_changed;
    egui_view_on_date_picker_open_changed_listener_t on_open_changed;
    egui_view_on_date_picker_display_month_changed_listener_t on_display_month_changed;
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
    uint16_t year;
    uint16_t panel_year;
    uint16_t today_year;
    uint8_t month;
    uint8_t panel_month;
    uint8_t day;
    uint8_t today_month;
    uint8_t today_day;
    uint8_t first_day_of_week;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t open_mode;
    uint8_t preserve_display_month_on_open;
    uint8_t pressed_part;
    uint8_t pressed_day;
    egui_alpha_t alpha;
};

static egui_view_date_picker_t test_date_picker;
static egui_view_date_picker_t preview_date_picker;
static egui_view_api_t preview_api;
static uint8_t g_date_changed_count;
static uint16_t g_last_year;
static uint8_t g_last_month;
static uint8_t g_last_day;
static uint8_t g_open_changed_count;
static uint8_t g_last_opened;
static uint8_t g_display_changed_count;
static uint16_t g_last_display_year;
static uint8_t g_last_display_month;

static void on_date_changed(egui_view_t *self, uint16_t year, uint8_t month, uint8_t day)
{
    EGUI_UNUSED(self);
    g_date_changed_count++;
    g_last_year = year;
    g_last_month = month;
    g_last_day = day;
}

static void on_open_changed(egui_view_t *self, uint8_t opened)
{
    EGUI_UNUSED(self);
    g_open_changed_count++;
    g_last_opened = opened;
}

static void on_display_month_changed(egui_view_t *self, uint16_t year, uint8_t month)
{
    EGUI_UNUSED(self);
    g_display_changed_count++;
    g_last_display_year = year;
    g_last_display_month = month;
}

static void reset_listener_state(void)
{
    g_date_changed_count = 0;
    g_last_year = 0;
    g_last_month = 0;
    g_last_day = 0;
    g_open_changed_count = 0;
    g_last_opened = 0xFF;
    g_display_changed_count = 0;
    g_last_display_year = 0;
    g_last_display_month = 0;
}

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void setup_date_picker(void)
{
    egui_view_date_picker_init(EGUI_VIEW_OF(&test_date_picker));
    egui_view_set_size(EGUI_VIEW_OF(&test_date_picker), 194, 180);
    egui_view_date_picker_set_label(EGUI_VIEW_OF(&test_date_picker), "Ship date");
    egui_view_date_picker_set_helper(EGUI_VIEW_OF(&test_date_picker), "Mon first week, tap days");
    egui_view_date_picker_set_today(EGUI_VIEW_OF(&test_date_picker), 2026, 3, 15);
    egui_view_date_picker_set_first_day_of_week(EGUI_VIEW_OF(&test_date_picker), 1);
    egui_view_date_picker_set_on_date_changed_listener(EGUI_VIEW_OF(&test_date_picker), on_date_changed);
    egui_view_date_picker_set_on_open_changed_listener(EGUI_VIEW_OF(&test_date_picker), on_open_changed);
    egui_view_date_picker_set_on_display_month_changed_listener(EGUI_VIEW_OF(&test_date_picker), on_display_month_changed);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&test_date_picker), 1);
#endif
    reset_listener_state();
}

static void setup_preview_date_picker(void)
{
    egui_view_date_picker_init(EGUI_VIEW_OF(&preview_date_picker));
    egui_view_set_size(EGUI_VIEW_OF(&preview_date_picker), 104, 48);
    egui_view_date_picker_set_font(EGUI_VIEW_OF(&preview_date_picker), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_date_picker_set_meta_font(EGUI_VIEW_OF(&preview_date_picker), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_date_picker_set_label(EGUI_VIEW_OF(&preview_date_picker), "Ship date");
    egui_view_date_picker_set_helper(EGUI_VIEW_OF(&preview_date_picker), "Tap day or use +/-");
    egui_view_date_picker_set_today(EGUI_VIEW_OF(&preview_date_picker), 2026, 3, 15);
    egui_view_date_picker_set_first_day_of_week(EGUI_VIEW_OF(&preview_date_picker), 1);
    egui_view_date_picker_set_date(EGUI_VIEW_OF(&preview_date_picker), 2026, 4, 5);
    egui_view_date_picker_set_display_month(EGUI_VIEW_OF(&preview_date_picker), 2026, 4);
    egui_view_date_picker_set_compact_mode(EGUI_VIEW_OF(&preview_date_picker), 1);
    egui_view_date_picker_set_palette(EGUI_VIEW_OF(&preview_date_picker), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                      EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_date_picker_set_on_date_changed_listener(EGUI_VIEW_OF(&preview_date_picker), on_date_changed);
    egui_view_date_picker_set_on_open_changed_listener(EGUI_VIEW_OF(&preview_date_picker), on_open_changed);
    egui_view_date_picker_set_on_display_month_changed_listener(EGUI_VIEW_OF(&preview_date_picker), on_display_month_changed);
    egui_view_date_picker_override_static_preview_api(EGUI_VIEW_OF(&preview_date_picker), &preview_api);
    reset_listener_state();
}

static void layout_date_picker(egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = 10;
    region.location.y = 20;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(EGUI_VIEW_OF(&test_date_picker), &region);
    egui_region_copy(&EGUI_VIEW_OF(&test_date_picker)->region_screen, &region);
}

static void layout_preview_date_picker(void)
{
    egui_region_t region;

    region.location.x = 10;
    region.location.y = 20;
    region.size.width = 104;
    region.size.height = 48;
    egui_view_layout(EGUI_VIEW_OF(&preview_date_picker), &region);
    egui_region_copy(&EGUI_VIEW_OF(&preview_date_picker)->region_screen, &region);
}

static int send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return EGUI_VIEW_OF(&test_date_picker)->api->dispatch_touch_event(EGUI_VIEW_OF(&test_date_picker), &event);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return EGUI_VIEW_OF(&preview_date_picker)->api->dispatch_touch_event(EGUI_VIEW_OF(&preview_date_picker), &event);
}

static int send_key_action(uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return EGUI_VIEW_OF(&test_date_picker)->api->dispatch_key_event(EGUI_VIEW_OF(&test_date_picker), &event);
}

static int send_preview_key_action(uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return EGUI_VIEW_OF(&preview_date_picker)->api->dispatch_key_event(EGUI_VIEW_OF(&preview_date_picker), &event);
}

static int send_key(uint8_t key_code)
{
    int handled = 0;

    handled |= send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= send_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static void get_metrics(egui_view_date_picker_metrics_t *metrics)
{
    date_picker_get_metrics(&test_date_picker, EGUI_VIEW_OF(&test_date_picker), metrics);
}

static void get_field_center(egui_dim_t *x, egui_dim_t *y)
{
    egui_view_date_picker_metrics_t metrics;

    get_metrics(&metrics);
    *x = metrics.field_region.location.x + metrics.field_region.size.width / 2;
    *y = metrics.field_region.location.y + metrics.field_region.size.height / 2;
}

static void get_preview_field_center(egui_dim_t *x, egui_dim_t *y)
{
    egui_view_date_picker_metrics_t metrics;

    date_picker_get_metrics(&preview_date_picker, EGUI_VIEW_OF(&preview_date_picker), &metrics);
    *x = metrics.field_region.location.x + metrics.field_region.size.width / 2;
    *y = metrics.field_region.location.y + metrics.field_region.size.height / 2;
}

static void capture_preview_snapshot(date_picker_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_date_picker)->region_screen;
    snapshot->on_date_changed = preview_date_picker.on_date_changed;
    snapshot->on_open_changed = preview_date_picker.on_open_changed;
    snapshot->on_display_month_changed = preview_date_picker.on_display_month_changed;
    snapshot->font = preview_date_picker.font;
    snapshot->meta_font = preview_date_picker.meta_font;
    snapshot->label = preview_date_picker.label;
    snapshot->helper = preview_date_picker.helper;
    snapshot->surface_color = preview_date_picker.surface_color;
    snapshot->border_color = preview_date_picker.border_color;
    snapshot->text_color = preview_date_picker.text_color;
    snapshot->muted_text_color = preview_date_picker.muted_text_color;
    snapshot->accent_color = preview_date_picker.accent_color;
    snapshot->today_color = preview_date_picker.today_color;
    snapshot->year = preview_date_picker.year;
    snapshot->panel_year = preview_date_picker.panel_year;
    snapshot->today_year = preview_date_picker.today_year;
    snapshot->month = preview_date_picker.month;
    snapshot->panel_month = preview_date_picker.panel_month;
    snapshot->day = preview_date_picker.day;
    snapshot->today_month = preview_date_picker.today_month;
    snapshot->today_day = preview_date_picker.today_day;
    snapshot->first_day_of_week = preview_date_picker.first_day_of_week;
    snapshot->compact_mode = preview_date_picker.compact_mode;
    snapshot->read_only_mode = preview_date_picker.read_only_mode;
    snapshot->open_mode = preview_date_picker.open_mode;
    snapshot->preserve_display_month_on_open = preview_date_picker.preserve_display_month_on_open;
    snapshot->pressed_part = preview_date_picker.pressed_part;
    snapshot->pressed_day = preview_date_picker.pressed_day;
    snapshot->alpha = EGUI_VIEW_OF(&preview_date_picker)->alpha;
}

static void assert_preview_state_unchanged(const date_picker_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_date_picker)->region_screen);
    EGUI_TEST_ASSERT_TRUE(preview_date_picker.on_date_changed == snapshot->on_date_changed);
    EGUI_TEST_ASSERT_TRUE(preview_date_picker.on_open_changed == snapshot->on_open_changed);
    EGUI_TEST_ASSERT_TRUE(preview_date_picker.on_display_month_changed == snapshot->on_display_month_changed);
    EGUI_TEST_ASSERT_TRUE(preview_date_picker.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_date_picker.meta_font == snapshot->meta_font);
    EGUI_TEST_ASSERT_TRUE(preview_date_picker.label == snapshot->label);
    EGUI_TEST_ASSERT_TRUE(preview_date_picker.helper == snapshot->helper);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_date_picker.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_date_picker.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_date_picker.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_date_picker.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_date_picker.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->today_color.full, preview_date_picker.today_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->year, preview_date_picker.year);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->panel_year, preview_date_picker.panel_year);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->today_year, preview_date_picker.today_year);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->month, preview_date_picker.month);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->panel_month, preview_date_picker.panel_month);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->day, preview_date_picker.day);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->today_month, preview_date_picker.today_month);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->today_day, preview_date_picker.today_day);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->first_day_of_week, preview_date_picker.first_day_of_week);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_date_picker.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_date_picker.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->open_mode, preview_date_picker.open_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->preserve_display_month_on_open, preview_date_picker.preserve_display_month_on_open);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->pressed_part, preview_date_picker.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->pressed_day, preview_date_picker.pressed_day);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_date_picker)->alpha);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_date_picker)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_date_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_display_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_last_year);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_last_month);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_last_day);
    EGUI_TEST_ASSERT_EQUAL_INT(0xFF, g_last_opened);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_last_display_year);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_last_display_month);
}

static void get_prev_center(egui_dim_t *x, egui_dim_t *y)
{
    egui_view_date_picker_metrics_t metrics;

    get_metrics(&metrics);
    *x = metrics.panel_prev_region.location.x + metrics.panel_prev_region.size.width / 2;
    *y = metrics.panel_prev_region.location.y + metrics.panel_prev_region.size.height / 2;
}

static void get_next_center(egui_dim_t *x, egui_dim_t *y)
{
    egui_view_date_picker_metrics_t metrics;

    get_metrics(&metrics);
    *x = metrics.panel_next_region.location.x + metrics.panel_next_region.size.width / 2;
    *y = metrics.panel_next_region.location.y + metrics.panel_next_region.size.height / 2;
}

static void get_day_center(uint8_t day, egui_dim_t *x, egui_dim_t *y)
{
    egui_view_date_picker_metrics_t metrics;
    uint8_t start_cell;
    uint8_t pos;
    uint8_t col;
    uint8_t row;
    egui_dim_t cell_w;
    egui_dim_t cell_h;

    get_metrics(&metrics);
    start_cell = date_picker_get_start_cell(&test_date_picker);
    pos = (uint8_t)(start_cell + day - 1);
    col = (uint8_t)(pos % 7);
    row = (uint8_t)(pos / 7);
    cell_w = metrics.panel_grid_region.size.width / 7;
    cell_h = metrics.panel_grid_region.size.height / 6;
    *x = metrics.panel_grid_region.location.x + col * cell_w + cell_w / 2;
    *y = metrics.panel_grid_region.location.y + row * cell_h + cell_h / 2;
}

static void test_date_picker_setters_and_listener_guards(void)
{
    setup_date_picker();

    egui_view_date_picker_set_date(EGUI_VIEW_OF(&test_date_picker), 2026, 2, 30);
    EGUI_TEST_ASSERT_EQUAL_INT(2026, egui_view_date_picker_get_year(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_date_picker_get_month(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(28, egui_view_date_picker_get_day(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_date_changed_count);

    egui_view_date_picker_set_today(EGUI_VIEW_OF(&test_date_picker), 2026, 2, 30);
    EGUI_TEST_ASSERT_EQUAL_INT(2026, test_date_picker.today_year);
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_date_picker.today_month);
    EGUI_TEST_ASSERT_EQUAL_INT(28, test_date_picker.today_day);

    egui_view_date_picker_set_first_day_of_week(EGUI_VIEW_OF(&test_date_picker), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_date_picker_get_first_day_of_week(EGUI_VIEW_OF(&test_date_picker)));
    egui_view_date_picker_set_first_day_of_week(EGUI_VIEW_OF(&test_date_picker), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_date_picker_get_first_day_of_week(EGUI_VIEW_OF(&test_date_picker)));

    egui_view_date_picker_set_display_month(EGUI_VIEW_OF(&test_date_picker), 2027, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_display_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2027, g_last_display_year);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_last_display_month);
    EGUI_TEST_ASSERT_EQUAL_INT(2027, egui_view_date_picker_get_display_year(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_date_picker_get_display_month(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_date_picker.preserve_display_month_on_open);

    egui_view_date_picker_set_opened(EGUI_VIEW_OF(&test_date_picker), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_date_picker_get_opened(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(2027, egui_view_date_picker_get_display_year(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_date_picker_get_display_month(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_date_picker.preserve_display_month_on_open);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_changed_count);

    egui_view_date_picker_set_opened(EGUI_VIEW_OF(&test_date_picker), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_date_picker_get_opened(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(2026, egui_view_date_picker_get_display_year(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_date_picker_get_display_month(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_changed_count);
}

static void test_date_picker_setters_clear_pressed_state(void)
{
    setup_date_picker();
    layout_date_picker(194, 180);

    EGUI_VIEW_OF(&test_date_picker)->is_pressed = 1;
    test_date_picker.pressed_part = EGUI_VIEW_DATE_PICKER_PART_FIELD;
    test_date_picker.pressed_day = 18;
    egui_view_date_picker_set_date(EGUI_VIEW_OF(&test_date_picker), 2026, 4, 22);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_date_picker)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_NONE, test_date_picker.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_date_picker.pressed_day);
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_date_picker_get_month(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(22, egui_view_date_picker_get_day(EGUI_VIEW_OF(&test_date_picker)));

    EGUI_VIEW_OF(&test_date_picker)->is_pressed = 1;
    test_date_picker.pressed_part = EGUI_VIEW_DATE_PICKER_PART_PREV;
    test_date_picker.pressed_day = 0;
    egui_view_date_picker_set_display_month(EGUI_VIEW_OF(&test_date_picker), 2026, 5);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_date_picker)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_NONE, test_date_picker.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_date_picker.pressed_day);
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_date_picker_get_display_month(EGUI_VIEW_OF(&test_date_picker)));

    EGUI_VIEW_OF(&test_date_picker)->is_pressed = 1;
    test_date_picker.pressed_part = EGUI_VIEW_DATE_PICKER_PART_DAY;
    test_date_picker.pressed_day = 22;
    egui_view_date_picker_set_first_day_of_week(EGUI_VIEW_OF(&test_date_picker), 0);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_date_picker)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_NONE, test_date_picker.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_date_picker.pressed_day);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_date_picker_get_first_day_of_week(EGUI_VIEW_OF(&test_date_picker)));

    EGUI_VIEW_OF(&test_date_picker)->is_pressed = 1;
    test_date_picker.pressed_part = EGUI_VIEW_DATE_PICKER_PART_NEXT;
    test_date_picker.pressed_day = 0;
    egui_view_date_picker_set_palette(EGUI_VIEW_OF(&test_date_picker), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                      EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_date_picker)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_NONE, test_date_picker.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_date_picker.pressed_day);

    setup_date_picker();
    layout_date_picker(194, 180);
    date_picker_set_open_inner(EGUI_VIEW_OF(&test_date_picker), 1, 1);
    EGUI_VIEW_OF(&test_date_picker)->is_pressed = 1;
    test_date_picker.pressed_part = EGUI_VIEW_DATE_PICKER_PART_DAY;
    test_date_picker.pressed_day = 18;
    egui_view_date_picker_set_compact_mode(EGUI_VIEW_OF(&test_date_picker), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_date_picker.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_date_picker_get_opened(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_date_picker)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_NONE, test_date_picker.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_date_picker.pressed_day);

    setup_date_picker();
    layout_date_picker(194, 180);
    date_picker_set_open_inner(EGUI_VIEW_OF(&test_date_picker), 1, 1);
    EGUI_VIEW_OF(&test_date_picker)->is_pressed = 1;
    test_date_picker.pressed_part = EGUI_VIEW_DATE_PICKER_PART_DAY;
    test_date_picker.pressed_day = 18;
    egui_view_date_picker_set_read_only_mode(EGUI_VIEW_OF(&test_date_picker), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_date_picker.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_date_picker_get_opened(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_date_picker)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_NONE, test_date_picker.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_date_picker.pressed_day);
}

static void test_date_picker_font_palette_and_internal_helpers(void)
{
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);
    uint16_t year;
    uint8_t month;
    uint8_t day;
    char buffer[16];

    setup_date_picker();

    egui_view_date_picker_set_font(EGUI_VIEW_OF(&test_date_picker), NULL);
    egui_view_date_picker_set_meta_font(EGUI_VIEW_OF(&test_date_picker), NULL);
    EGUI_TEST_ASSERT_TRUE(test_date_picker.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_TRUE(test_date_picker.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    egui_view_date_picker_set_palette(EGUI_VIEW_OF(&test_date_picker), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                      EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_date_picker.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, test_date_picker.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, test_date_picker.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x404142).full, test_date_picker.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x505152).full, test_date_picker.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x606162).full, test_date_picker.today_color.full);

    EGUI_TEST_ASSERT_EQUAL_INT(29, date_picker_days_in_month(2024, 2));
    EGUI_TEST_ASSERT_EQUAL_INT(28, date_picker_days_in_month(2025, 2));
    EGUI_TEST_ASSERT_EQUAL_INT(0, date_picker_day_of_week(2026, 3, 1));

    year = 2026;
    month = 13;
    day = 40;
    date_picker_normalize_date(&year, &month, &day);
    EGUI_TEST_ASSERT_EQUAL_INT(2026, year);
    EGUI_TEST_ASSERT_EQUAL_INT(12, month);
    EGUI_TEST_ASSERT_EQUAL_INT(31, day);

    year = 0;
    month = 0;
    date_picker_normalize_display_month(&year, &month);
    EGUI_TEST_ASSERT_EQUAL_INT(1, year);
    EGUI_TEST_ASSERT_EQUAL_INT(1, month);

    date_picker_format_date_field(2026, 3, 18, 0, buffer);
    EGUI_TEST_ASSERT_TRUE(strcmp(buffer, "2026-03-18") == 0);
    date_picker_format_date_field(2026, 3, 18, 1, buffer);
    EGUI_TEST_ASSERT_TRUE(strcmp(buffer, "Mar 18") == 0);
    date_picker_format_month_title(2027, 1, buffer);
    EGUI_TEST_ASSERT_TRUE(strcmp(buffer, "Jan 2027") == 0);

    test_date_picker.year = 2026;
    test_date_picker.month = 3;
    test_date_picker.day = 31;
    test_date_picker.panel_year = 2026;
    test_date_picker.panel_month = 4;
    test_date_picker.first_day_of_week = 1;
    EGUI_TEST_ASSERT_EQUAL_INT(2, date_picker_get_start_cell(&test_date_picker));
    EGUI_TEST_ASSERT_EQUAL_INT(30, date_picker_get_display_anchor_day(&test_date_picker));
    test_date_picker.panel_month = 3;
    EGUI_TEST_ASSERT_EQUAL_INT(0, date_picker_get_display_anchor_day(&test_date_picker));

    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_DARK_GREY, 68).full, date_picker_mix_disabled(sample).full);
}

static void test_date_picker_metrics_and_hit_testing(void)
{
    egui_view_date_picker_metrics_t metrics;
    egui_dim_t x;
    egui_dim_t y;

    setup_date_picker();
    egui_view_date_picker_set_opened(EGUI_VIEW_OF(&test_date_picker), 1);
    layout_date_picker(194, 180);
    get_metrics(&metrics);

    EGUI_TEST_ASSERT_EQUAL_INT(1, metrics.show_label);
    EGUI_TEST_ASSERT_EQUAL_INT(1, metrics.show_helper);
    EGUI_TEST_ASSERT_EQUAL_INT(1, metrics.show_panel);
    EGUI_TEST_ASSERT_TRUE(metrics.field_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.panel_grid_region.size.height > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.chevron_region.size.width > 0);

    get_field_center(&x, &y);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_FIELD, date_picker_hit_part(&test_date_picker, EGUI_VIEW_OF(&test_date_picker), x, y));

    get_prev_center(&x, &y);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_PREV, date_picker_hit_part(&test_date_picker, EGUI_VIEW_OF(&test_date_picker), x, y));

    get_next_center(&x, &y);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_NEXT, date_picker_hit_part(&test_date_picker, EGUI_VIEW_OF(&test_date_picker), x, y));

    get_day_center(18, &x, &y);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_DAY, date_picker_hit_part(&test_date_picker, EGUI_VIEW_OF(&test_date_picker), x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(18, date_picker_get_hit_day(&test_date_picker, EGUI_VIEW_OF(&test_date_picker), x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, date_picker_get_hit_day(&test_date_picker, EGUI_VIEW_OF(&test_date_picker), metrics.panel_grid_region.location.x + 1,
                                                          metrics.panel_grid_region.location.y + 1));

    egui_view_date_picker_set_read_only_mode(EGUI_VIEW_OF(&test_date_picker), 1);
    layout_date_picker(194, 180);
    get_metrics(&metrics);
    EGUI_TEST_ASSERT_EQUAL_INT(0, metrics.show_panel);
    EGUI_TEST_ASSERT_EQUAL_INT(0, metrics.chevron_region.size.width);

    egui_view_date_picker_set_read_only_mode(EGUI_VIEW_OF(&test_date_picker), 0);
    egui_view_date_picker_set_compact_mode(EGUI_VIEW_OF(&test_date_picker), 1);
    egui_view_set_size(EGUI_VIEW_OF(&test_date_picker), 106, 48);
    layout_date_picker(106, 48);
    get_metrics(&metrics);
    EGUI_TEST_ASSERT_EQUAL_INT(0, metrics.show_label);
    EGUI_TEST_ASSERT_EQUAL_INT(0, metrics.show_helper);
    EGUI_TEST_ASSERT_EQUAL_INT(0, metrics.show_panel);
}

static void test_date_picker_touch_toggle_and_day_selection(void)
{
    egui_dim_t x;
    egui_dim_t y;
    egui_dim_t x2;
    egui_dim_t y2;

    setup_date_picker();
    layout_date_picker(194, 180);
    get_field_center(&x, &y);

    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, 0, 0));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_FIELD, test_date_picker.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_date_picker)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_date_picker_get_opened(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_open_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_last_opened);

    get_prev_center(&x, &y);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_date_picker_get_display_month(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_display_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2026, g_last_display_year);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_last_display_month);

    get_next_center(&x, &y);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_date_picker_get_display_month(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_display_changed_count);

    get_day_center(24, &x, &y);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_DAY, test_date_picker.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(24, test_date_picker.pressed_day);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(24, egui_view_date_picker_get_day(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_date_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2026, g_last_year);
    EGUI_TEST_ASSERT_EQUAL_INT(3, g_last_month);
    EGUI_TEST_ASSERT_EQUAL_INT(24, g_last_day);

    get_day_center(25, &x, &y);
    get_day_center(26, &x2, &y2);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(25, test_date_picker.pressed_day);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_date_picker)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, x2, y2));
    EGUI_TEST_ASSERT_EQUAL_INT(25, test_date_picker.pressed_day);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_date_picker)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x2, y2));
    EGUI_TEST_ASSERT_EQUAL_INT(24, egui_view_date_picker_get_day(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_date_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_NONE, test_date_picker.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_date_picker)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, x2, y2));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_date_picker)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(25, egui_view_date_picker_get_day(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_date_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(25, g_last_day);

    get_day_center(26, &x, &y);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, 0, 0));
    EGUI_TEST_ASSERT_EQUAL_INT(26, test_date_picker.pressed_day);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_date_picker)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, 0, 0));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_NONE, test_date_picker.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_date_picker)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_date_changed_count);

    get_field_center(&x, &y);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_date_picker_get_opened(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_open_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_last_opened);
}

static void test_date_picker_keyboard_navigation_and_browse_commit(void)
{
    setup_date_picker();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_EQUAL_INT(17, egui_view_date_picker_get_day(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_date_changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(18, egui_view_date_picker_get_day(EGUI_VIEW_OF(&test_date_picker)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_UP));
    EGUI_TEST_ASSERT_EQUAL_INT(11, egui_view_date_picker_get_day(EGUI_VIEW_OF(&test_date_picker)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(18, egui_view_date_picker_get_day(EGUI_VIEW_OF(&test_date_picker)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_date_picker_get_opened(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_open_changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_PLUS));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_date_picker_get_display_month(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_display_changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(2026, egui_view_date_picker_get_year(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_date_picker_get_month(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(18, egui_view_date_picker_get_day(EGUI_VIEW_OF(&test_date_picker)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_date_picker_get_day(EGUI_VIEW_OF(&test_date_picker)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(30, egui_view_date_picker_get_day(EGUI_VIEW_OF(&test_date_picker)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_date_picker_get_opened(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_open_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_last_opened);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_date_picker_get_opened(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_date_picker_get_opened(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_FALSE(send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ESCAPE));
}

static void test_date_picker_compact_read_only_disabled_and_focus_guards(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_date_picker();
    layout_date_picker(194, 180);
    get_field_center(&x, &y);

    egui_view_date_picker_set_read_only_mode(EGUI_VIEW_OF(&test_date_picker), 1);
    EGUI_VIEW_OF(&test_date_picker)->is_pressed = 1;
    test_date_picker.pressed_part = EGUI_VIEW_DATE_PICKER_PART_FIELD;
    test_date_picker.pressed_day = 18;
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_date_picker)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_NONE, test_date_picker.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_date_picker.pressed_day);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_VIEW_OF(&test_date_picker)->is_pressed = 1;
    test_date_picker.pressed_part = EGUI_VIEW_DATE_PICKER_PART_FIELD;
    test_date_picker.pressed_day = 18;
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_date_picker)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_NONE, test_date_picker.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_date_picker.pressed_day);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_date_picker_get_opened(EGUI_VIEW_OF(&test_date_picker)));

    setup_date_picker();
    egui_view_date_picker_set_compact_mode(EGUI_VIEW_OF(&test_date_picker), 1);
    egui_view_set_size(EGUI_VIEW_OF(&test_date_picker), 106, 48);
    layout_date_picker(106, 48);
    get_field_center(&x, &y);
    EGUI_VIEW_OF(&test_date_picker)->is_pressed = 1;
    test_date_picker.pressed_part = EGUI_VIEW_DATE_PICKER_PART_FIELD;
    test_date_picker.pressed_day = 18;
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_date_picker)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_NONE, test_date_picker.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_date_picker.pressed_day);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_VIEW_OF(&test_date_picker)->is_pressed = 1;
    test_date_picker.pressed_part = EGUI_VIEW_DATE_PICKER_PART_FIELD;
    test_date_picker.pressed_day = 18;
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_date_picker)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_NONE, test_date_picker.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_date_picker.pressed_day);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_date_picker_get_opened(EGUI_VIEW_OF(&test_date_picker)));
    egui_view_date_picker_set_opened(EGUI_VIEW_OF(&test_date_picker), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_date_picker_get_opened(EGUI_VIEW_OF(&test_date_picker)));

    setup_date_picker();
    layout_date_picker(194, 180);
    get_field_center(&x, &y);
    egui_view_set_enable(EGUI_VIEW_OF(&test_date_picker), 0);
    EGUI_VIEW_OF(&test_date_picker)->is_pressed = 1;
    test_date_picker.pressed_part = EGUI_VIEW_DATE_PICKER_PART_FIELD;
    test_date_picker.pressed_day = 18;
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_date_picker)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_NONE, test_date_picker.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_date_picker.pressed_day);
    EGUI_VIEW_OF(&test_date_picker)->is_pressed = 1;
    test_date_picker.pressed_part = EGUI_VIEW_DATE_PICKER_PART_FIELD;
    test_date_picker.pressed_day = 18;
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_date_picker)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_NONE, test_date_picker.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_date_picker.pressed_day);
    egui_view_date_picker_set_opened(EGUI_VIEW_OF(&test_date_picker), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_date_picker_get_opened(EGUI_VIEW_OF(&test_date_picker)));

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    setup_date_picker();
    date_picker_set_open_inner(EGUI_VIEW_OF(&test_date_picker), 1, 1);
    test_date_picker.pressed_part = EGUI_VIEW_DATE_PICKER_PART_FIELD;
    test_date_picker.pressed_day = 18;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_date_picker), true);
    egui_view_date_picker_on_focus_change(EGUI_VIEW_OF(&test_date_picker), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_date_picker_get_opened(EGUI_VIEW_OF(&test_date_picker)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATE_PICKER_PART_NONE, test_date_picker.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_date_picker.pressed_day);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_date_picker)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_open_changed_count);
#endif
}

static void test_date_picker_static_preview_consumes_input_and_keeps_state(void)
{
    date_picker_preview_snapshot_t initial_snapshot;
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_date_picker();
    layout_preview_date_picker();
    get_preview_field_center(&x, &y);
    capture_preview_snapshot(&initial_snapshot);

    EGUI_VIEW_OF(&preview_date_picker)->is_pressed = 1;
    preview_date_picker.pressed_part = EGUI_VIEW_DATE_PICKER_PART_FIELD;
    preview_date_picker.pressed_day = 0;
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_preview_state_unchanged(&initial_snapshot);

    EGUI_VIEW_OF(&preview_date_picker)->is_pressed = 1;
    preview_date_picker.pressed_part = EGUI_VIEW_DATE_PICKER_PART_DAY;
    preview_date_picker.pressed_day = 5;
    EGUI_TEST_ASSERT_TRUE(send_preview_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_preview_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_date_picker_run(void)
{
    EGUI_TEST_SUITE_BEGIN(date_picker);
    EGUI_TEST_RUN(test_date_picker_setters_and_listener_guards);
    EGUI_TEST_RUN(test_date_picker_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_date_picker_font_palette_and_internal_helpers);
    EGUI_TEST_RUN(test_date_picker_metrics_and_hit_testing);
    EGUI_TEST_RUN(test_date_picker_touch_toggle_and_day_selection);
    EGUI_TEST_RUN(test_date_picker_keyboard_navigation_and_browse_commit);
    EGUI_TEST_RUN(test_date_picker_compact_read_only_disabled_and_focus_guards);
    EGUI_TEST_RUN(test_date_picker_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
