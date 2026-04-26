#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_status_bar.h"

#include "../../HelloCustomWidgets/display/status_bar/egui_view_status_bar.h"
#include "../../HelloCustomWidgets/display/status_bar/egui_view_status_bar.c"

typedef struct status_bar_preview_snapshot status_bar_preview_snapshot_t;
struct status_bar_preview_snapshot
{
    egui_region_t region_screen;
    const egui_view_api_t *api;
    egui_view_status_bar_item_t items[EGUI_VIEW_STATUS_BAR_MAX_ITEMS];
    const egui_font_t *label_font;
    const egui_font_t *value_font;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t separator_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t ok_color;
    egui_color_t warn_color;
    egui_alpha_t alpha;
    uint8_t item_count;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t enable;
    uint8_t is_pressed;
    uint8_t is_focused;
    egui_dim_margin_padding_t padding_left;
    egui_dim_margin_padding_t padding_right;
    egui_dim_margin_padding_t padding_top;
    egui_dim_margin_padding_t padding_bottom;
};

static egui_view_status_bar_t test_control;
static egui_view_status_bar_t preview_control;
static egui_view_api_t preview_api;

static const egui_view_status_bar_item_t standard_items[] = {
        {"Sync", "Ready", 2, EGUI_VIEW_STATUS_BAR_STATE_OK, 1},
        {"Line", "124", 1, EGUI_VIEW_STATUS_BAR_STATE_INFO, 0},
        {"Mode", "Edit", 1, EGUI_VIEW_STATUS_BAR_STATE_NORMAL, 0},
        {"Warn", "2", 1, EGUI_VIEW_STATUS_BAR_STATE_WARN, 1},
        {"Extra", "Ignored", 1, EGUI_VIEW_STATUS_BAR_STATE_INFO, 0},
};

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void assert_string_equal(const char *expected, const char *actual)
{
    EGUI_TEST_ASSERT_TRUE(expected != NULL);
    EGUI_TEST_ASSERT_TRUE(actual != NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(expected, actual));
}

static void setup_status_bar(void)
{
    egui_view_status_bar_init(EGUI_VIEW_OF(&test_control));
    egui_view_set_size(EGUI_VIEW_OF(&test_control), 180, 32);
}

static void setup_preview_control(void)
{
    egui_view_status_bar_init(EGUI_VIEW_OF(&preview_control));
    egui_view_set_size(EGUI_VIEW_OF(&preview_control), 94, 28);
    egui_view_status_bar_apply_compact_style(EGUI_VIEW_OF(&preview_control));
    egui_view_status_bar_set_items(EGUI_VIEW_OF(&preview_control), standard_items, 2);
    egui_view_status_bar_override_static_preview_api(EGUI_VIEW_OF(&preview_control), &preview_api);
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

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void capture_preview_snapshot(status_bar_preview_snapshot_t *snapshot)
{
    uint8_t index;

    snapshot->region_screen = EGUI_VIEW_OF(&preview_control)->region_screen;
    snapshot->api = EGUI_VIEW_OF(&preview_control)->api;
    for (index = 0; index < EGUI_VIEW_STATUS_BAR_MAX_ITEMS; ++index)
    {
        snapshot->items[index] = preview_control.items[index];
    }
    snapshot->label_font = preview_control.label_font;
    snapshot->value_font = preview_control.value_font;
    snapshot->surface_color = preview_control.surface_color;
    snapshot->border_color = preview_control.border_color;
    snapshot->separator_color = preview_control.separator_color;
    snapshot->text_color = preview_control.text_color;
    snapshot->muted_text_color = preview_control.muted_text_color;
    snapshot->accent_color = preview_control.accent_color;
    snapshot->ok_color = preview_control.ok_color;
    snapshot->warn_color = preview_control.warn_color;
    snapshot->alpha = EGUI_VIEW_OF(&preview_control)->alpha;
    snapshot->item_count = preview_control.item_count;
    snapshot->compact_mode = preview_control.compact_mode;
    snapshot->read_only_mode = preview_control.read_only_mode;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_control));
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_control)->is_pressed;
    snapshot->is_focused = EGUI_VIEW_OF(&preview_control)->is_focused;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_control)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_control)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_control)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_control)->padding.bottom;
}

static void assert_preview_state_unchanged(const status_bar_preview_snapshot_t *snapshot)
{
    uint8_t index;

    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_control)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_control)->api == snapshot->api);
    for (index = 0; index < EGUI_VIEW_STATUS_BAR_MAX_ITEMS; ++index)
    {
        assert_string_equal(snapshot->items[index].label, preview_control.items[index].label);
        assert_string_equal(snapshot->items[index].value, preview_control.items[index].value);
        EGUI_TEST_ASSERT_EQUAL_INT(snapshot->items[index].weight, preview_control.items[index].weight);
        EGUI_TEST_ASSERT_EQUAL_INT(snapshot->items[index].state, preview_control.items[index].state);
        EGUI_TEST_ASSERT_EQUAL_INT(snapshot->items[index].emphasized, preview_control.items[index].emphasized);
    }
    EGUI_TEST_ASSERT_TRUE(preview_control.label_font == snapshot->label_font);
    EGUI_TEST_ASSERT_TRUE(preview_control.value_font == snapshot->value_font);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_control.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->separator_color.full, preview_control.separator_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_control.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_control.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_control.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->ok_color.full, preview_control.ok_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->warn_color.full, preview_control.warn_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_control)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->item_count, preview_control.item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_control.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_control.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_control)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_control)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_control)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_control)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_control)->padding.bottom);
}

static void test_status_bar_init_defaults(void)
{
    setup_status_bar();

    EGUI_TEST_ASSERT_EQUAL_INT(0, test_control.item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_status_bar_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_status_bar_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_TRUE(test_control.label_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_TRUE(test_control.value_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xFFFFFF).full, test_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xCCD6E0).full, test_control.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, test_control.accent_color.full);
#if EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_control)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_control)->padding.right);
#endif
}

static void test_status_bar_items_and_regions(void)
{
    egui_region_t first_region;
    egui_region_t second_region;
    egui_region_t fourth_region;
    egui_view_status_bar_item_t replacement = {NULL, "Muted", 0, 99, 1};

    setup_status_bar();
    egui_view_status_bar_set_items(EGUI_VIEW_OF(&test_control), standard_items, (uint8_t)EGUI_ARRAY_SIZE(standard_items));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_STATUS_BAR_MAX_ITEMS, test_control.item_count);
    assert_string_equal("Sync", test_control.items[0].label);
    assert_string_equal("Ready", test_control.items[0].value);
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_control.items[0].weight);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_STATUS_BAR_STATE_WARN, test_control.items[3].state);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_control), 1);
    egui_view_status_bar_set_item(EGUI_VIEW_OF(&test_control), 2, &replacement);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_control)->is_pressed);
    assert_string_equal("", test_control.items[2].label);
    assert_string_equal("Muted", test_control.items[2].value);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_control.items[2].weight);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_STATUS_BAR_STATE_NORMAL, test_control.items[2].state);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_control.items[2].emphasized);

    layout_view(EGUI_VIEW_OF(&test_control), 10, 20, 180, 32);
    EGUI_TEST_ASSERT_TRUE(egui_view_status_bar_get_item_region(EGUI_VIEW_OF(&test_control), 0, &first_region));
    EGUI_TEST_ASSERT_TRUE(egui_view_status_bar_get_item_region(EGUI_VIEW_OF(&test_control), 1, &second_region));
    EGUI_TEST_ASSERT_TRUE(egui_view_status_bar_get_item_region(EGUI_VIEW_OF(&test_control), 3, &fourth_region));
    EGUI_TEST_ASSERT_TRUE(first_region.size.width > second_region.size.width);
    EGUI_TEST_ASSERT_TRUE(first_region.location.x < second_region.location.x);
    EGUI_TEST_ASSERT_TRUE(second_region.location.x < fourth_region.location.x);
    EGUI_TEST_ASSERT_FALSE(egui_view_status_bar_get_item_region(EGUI_VIEW_OF(&test_control), 4, &fourth_region));
}

static void test_status_bar_styles_palette_and_fonts(void)
{
    setup_status_bar();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_control), 1);
    egui_view_status_bar_apply_accent_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_status_bar_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_status_bar_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xF7FBFF).full, test_control.surface_color.full);

    egui_view_status_bar_apply_compact_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_status_bar_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_status_bar_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0C7C73).full, test_control.accent_color.full);

    egui_view_status_bar_apply_read_only_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_status_bar_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_status_bar_get_read_only_mode(EGUI_VIEW_OF(&test_control)));

    egui_view_status_bar_set_fonts(EGUI_VIEW_OF(&test_control), (const egui_font_t *)&egui_res_font_montserrat_8_4,
                                   (const egui_font_t *)&egui_res_font_montserrat_10_4);
    EGUI_TEST_ASSERT_TRUE(test_control.label_font == (const egui_font_t *)&egui_res_font_montserrat_8_4);
    EGUI_TEST_ASSERT_TRUE(test_control.value_font == (const egui_font_t *)&egui_res_font_montserrat_10_4);

    egui_view_status_bar_set_palette(EGUI_VIEW_OF(&test_control), EGUI_COLOR_HEX(0x010203), EGUI_COLOR_HEX(0x111213),
                                     EGUI_COLOR_HEX(0x212223), EGUI_COLOR_HEX(0x313233), EGUI_COLOR_HEX(0x414243),
                                     EGUI_COLOR_HEX(0x515253), EGUI_COLOR_HEX(0x616263), EGUI_COLOR_HEX(0x717273));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x010203).full, test_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x515253).full, test_control.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x717273).full, test_control.warn_color.full);
}

static void test_status_bar_static_preview_consumes_input_and_keeps_state(void)
{
    status_bar_preview_snapshot_t initial_snapshot;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_control();
    layout_view(EGUI_VIEW_OF(&preview_control), 12, 18, 94, 28);
    get_view_center(EGUI_VIEW_OF(&preview_control), &center_x, &center_y);
    capture_preview_snapshot(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_control), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_control), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_control), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    assert_preview_state_unchanged(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_control), 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_control), EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
}

static void test_status_bar_text_helpers(void)
{
    char label[16];
    char value[24];

    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_status_bar_text_len(NULL));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_status_bar_text_len("Ready"));
    egui_view_status_bar_copy_elided(label, sizeof(label), "Synchronization", 8);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp("Synch...", label));
    egui_view_status_bar_fit_text_to_width(NULL, "Disconnected", value, sizeof(value), 24, 4);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp("Dis...", value));
}

void test_status_bar_run(void)
{
    EGUI_TEST_SUITE_BEGIN(status_bar);
    EGUI_TEST_RUN(test_status_bar_init_defaults);
    EGUI_TEST_RUN(test_status_bar_items_and_regions);
    EGUI_TEST_RUN(test_status_bar_styles_palette_and_fonts);
    EGUI_TEST_RUN(test_status_bar_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_RUN(test_status_bar_text_helpers);
    EGUI_TEST_SUITE_END();
}
