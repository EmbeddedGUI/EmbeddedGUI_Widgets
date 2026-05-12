#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_auto_suggest_box.h"

#include "../../HelloCustomWidgets/input/auto_suggest_box/egui_view_auto_suggest_box.h"
#include "../../HelloCustomWidgets/input/auto_suggest_box/egui_view_auto_suggest_box.c"

static egui_view_auto_suggest_box_t test_box;
static egui_view_auto_suggest_box_t preview_box;
static egui_view_api_t preview_api;
static uint8_t g_selected_count;
static uint8_t g_last_selected;

typedef struct
{
    egui_region_t region_screen;
    const char **suggestions;
    const char *query;
    const char *placeholder;
    const egui_font_t *font;
    const egui_font_t *icon_font;
    uint8_t suggestion_count;
    uint8_t current_index;
    uint8_t max_visible_items;
    uint8_t is_expanded;
    uint8_t is_enable;
    egui_color_t popup_color;
    egui_color_t highlight_color;
    egui_color_t icon_color;
    egui_dim_t collapsed_height;
    egui_dim_t item_height;
    egui_dim_t icon_text_gap;
} auto_suggest_box_preview_snapshot_t;

static const char *g_people[] = {"Alice Chen", "Alicia Gomez", "Allen Park", "Amelia Stone"};
static const char *g_commands[] = {"Deploy API", "Deploy Docs", "Deploy Worker"};
static const char *g_preview[] = {"Recent", "Reminder"};

static void on_selected(egui_view_t *self, uint8_t index)
{
    EGUI_UNUSED(self);
    g_selected_count++;
    g_last_selected = index;
}

static void reset_listener_state(void)
{
    g_selected_count = 0;
    g_last_selected = 0xFF;
}

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void setup_box(void)
{
    egui_view_auto_suggest_box_init(EGUI_VIEW_OF(&test_box), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&test_box), 180, 34);
    egui_view_auto_suggest_box_set_font(EGUI_VIEW_OF(&test_box), (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    egui_view_auto_suggest_box_set_suggestions(EGUI_VIEW_OF(&test_box), g_people, EGUI_ARRAY_SIZE(g_people));
    egui_view_auto_suggest_box_set_query(EGUI_VIEW_OF(&test_box), "Ali");
    egui_view_auto_suggest_box_set_on_selected_listener(EGUI_VIEW_OF(&test_box), on_selected);
    hcw_auto_suggest_box_apply_standard_style(EGUI_VIEW_OF(&test_box));
    egui_view_auto_suggest_box_set_placeholder(EGUI_VIEW_OF(&test_box), "Search");
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&test_box), 1);
#endif
    reset_listener_state();
}

static void layout_box(egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(EGUI_VIEW_OF(&test_box), &region);
    egui_region_copy(&EGUI_VIEW_OF(&test_box)->region_screen, &region);
}

static void setup_preview_box(void)
{
    egui_view_auto_suggest_box_init(EGUI_VIEW_OF(&preview_box), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&preview_box), 104, 28);
    egui_view_auto_suggest_box_set_font(EGUI_VIEW_OF(&preview_box), (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    hcw_auto_suggest_box_set_suggestions(EGUI_VIEW_OF(&preview_box), g_preview, EGUI_ARRAY_SIZE(g_preview));
    hcw_auto_suggest_box_set_current_index(EGUI_VIEW_OF(&preview_box), 0);
    egui_view_auto_suggest_box_set_on_selected_listener(EGUI_VIEW_OF(&preview_box), on_selected);
    hcw_auto_suggest_box_apply_compact_style(EGUI_VIEW_OF(&preview_box));
    hcw_auto_suggest_box_override_static_preview_api(EGUI_VIEW_OF(&preview_box), &preview_api);
    reset_listener_state();
}

static void layout_preview_box(void)
{
    egui_region_t region;

    region.location.x = 10;
    region.location.y = 20;
    region.size.width = 104;
    region.size.height = 28;
    egui_view_layout(EGUI_VIEW_OF(&preview_box), &region);
    egui_region_copy(&EGUI_VIEW_OF(&preview_box)->region_screen, &region);
}

static int send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return EGUI_VIEW_OF(&test_box)->api->on_touch_event(EGUI_VIEW_OF(&test_box), &event);
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

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_box), key_code);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return EGUI_VIEW_OF(&preview_box)->api->on_touch_event(EGUI_VIEW_OF(&preview_box), &event);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_box), key_code);
}

static void get_view_center(egui_dim_t *x, egui_dim_t *y)
{
    egui_view_auto_suggest_box_t *local = &test_box;

    *x = EGUI_VIEW_OF(&test_box)->region_screen.location.x + EGUI_VIEW_OF(&test_box)->region_screen.size.width / 2;
    *y = EGUI_VIEW_OF(&test_box)->region_screen.location.y + local->collapsed_height / 2;
}

static void get_dropdown_item_center(uint8_t filtered_row, egui_dim_t *x, egui_dim_t *y)
{
    egui_view_auto_suggest_box_t *local = &test_box;
    uint8_t visible_count = auto_suggest_box_get_current_visible_count(EGUI_VIEW_OF(&test_box), local);
    uint8_t start_row = auto_suggest_box_get_visible_start_row(EGUI_VIEW_OF(&test_box), local, visible_count);
    uint8_t row = filtered_row;

    EGUI_TEST_ASSERT_TRUE(local->is_expanded);
    EGUI_TEST_ASSERT_TRUE(visible_count > 0);

    if (row < start_row)
    {
        row = 0;
    }
    else if (row >= (uint8_t)(start_row + visible_count))
    {
        row = (uint8_t)(visible_count - 1);
    }
    else
    {
        row = (uint8_t)(row - start_row);
    }

    *x = EGUI_VIEW_OF(&test_box)->region_screen.location.x + EGUI_VIEW_OF(&test_box)->region_screen.size.width / 2;
    *y = EGUI_VIEW_OF(&test_box)->region_screen.location.y + local->collapsed_height + row * local->item_height + local->item_height / 2;
}

static void capture_preview_snapshot(auto_suggest_box_preview_snapshot_t *snapshot)
{
    egui_view_auto_suggest_box_t *local = &preview_box;

    snapshot->region_screen = EGUI_VIEW_OF(&preview_box)->region_screen;
    snapshot->suggestions = local->suggestions;
    snapshot->query = egui_view_auto_suggest_box_get_query(EGUI_VIEW_OF(&preview_box));
    snapshot->placeholder = local->textinput.placeholder;
    snapshot->font = local->textinput.font;
    snapshot->icon_font = local->icon_font;
    snapshot->suggestion_count = local->suggestion_count;
    snapshot->current_index = local->current_index;
    snapshot->max_visible_items = local->max_visible_items;
    snapshot->is_expanded = local->is_expanded;
    snapshot->is_enable = EGUI_VIEW_OF(&preview_box)->is_enable;
    snapshot->popup_color = local->popup_color;
    snapshot->highlight_color = local->highlight_color;
    snapshot->icon_color = local->icon_color;
    snapshot->collapsed_height = local->collapsed_height;
    snapshot->item_height = local->item_height;
    snapshot->icon_text_gap = local->icon_text_gap;
}

static void assert_preview_state_unchanged(const auto_suggest_box_preview_snapshot_t *snapshot)
{
    egui_view_auto_suggest_box_t *local = &preview_box;

    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_box)->region_screen);
    EGUI_TEST_ASSERT_TRUE(local->suggestions == snapshot->suggestions);
    EGUI_TEST_ASSERT_TRUE(strcmp(snapshot->query, egui_view_auto_suggest_box_get_query(EGUI_VIEW_OF(&preview_box))) == 0);
    EGUI_TEST_ASSERT_TRUE(local->textinput.placeholder == snapshot->placeholder);
    EGUI_TEST_ASSERT_TRUE(local->textinput.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(local->icon_font == snapshot->icon_font);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->suggestion_count, local->suggestion_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_index, local->current_index);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->max_visible_items, local->max_visible_items);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_expanded, local->is_expanded);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_enable, EGUI_VIEW_OF(&preview_box)->is_enable);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->popup_color.full, local->popup_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->highlight_color.full, local->highlight_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->icon_color.full, local->icon_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->collapsed_height, local->collapsed_height);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->item_height, local->item_height);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->icon_text_gap, local->icon_text_gap);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selected_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0xFF, g_last_selected);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_AUTO_SUGGEST_BOX_PART_NONE, local->pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE, local->pressed_row);
}

static void test_auto_suggest_box_query_filter_and_current_index(void)
{
    setup_box();

    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_auto_suggest_box_get_suggestion_count(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_TRUE(strcmp("Ali", egui_view_auto_suggest_box_get_query(EGUI_VIEW_OF(&test_box))) == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_auto_suggest_box_get_filtered_count(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_auto_suggest_box_get_current_index(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_TRUE(strcmp("Alice Chen", egui_view_auto_suggest_box_get_current_text(EGUI_VIEW_OF(&test_box))) == 0);

    egui_view_auto_suggest_box_set_current_index(EGUI_VIEW_OF(&test_box), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_auto_suggest_box_get_current_index(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_TRUE(strcmp("Alicia Gomez", egui_view_auto_suggest_box_get_current_text(EGUI_VIEW_OF(&test_box))) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp("Alicia Gomez", egui_view_auto_suggest_box_get_query(EGUI_VIEW_OF(&test_box))) == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selected_count);

    egui_view_auto_suggest_box_set_query(EGUI_VIEW_OF(&test_box), "Am");
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_auto_suggest_box_get_filtered_count(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_auto_suggest_box_get_current_index(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_TRUE(strcmp("Amelia Stone", egui_view_auto_suggest_box_get_current_text(EGUI_VIEW_OF(&test_box))) == 0);

    egui_view_auto_suggest_box_set_suggestions(EGUI_VIEW_OF(&test_box), g_commands, EGUI_ARRAY_SIZE(g_commands));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_auto_suggest_box_get_suggestion_count(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_auto_suggest_box_get_filtered_count(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE, egui_view_auto_suggest_box_get_current_index(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_NULL(egui_view_auto_suggest_box_get_current_text(EGUI_VIEW_OF(&test_box)));

    egui_view_auto_suggest_box_set_query(EGUI_VIEW_OF(&test_box), "Dep");
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_auto_suggest_box_get_filtered_count(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_auto_suggest_box_get_current_index(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_TRUE(strcmp("Deploy API", egui_view_auto_suggest_box_get_current_text(EGUI_VIEW_OF(&test_box))) == 0);
}

static void test_auto_suggest_box_style_helpers_and_params(void)
{
    egui_view_auto_suggest_box_t params_box;
    const egui_font_t *font_before;
    egui_view_auto_suggest_box_params_t params = {
            .region = {{1, 2}, {96, 28}},
            .suggestions = g_commands,
            .suggestion_count = 3,
            .current_index = 0,
            .query = "Dep",
            .placeholder = "Search",
    };
    egui_view_auto_suggest_box_params_t init_params = {
            .region = {{4, 5}, {120, 30}},
            .suggestions = g_commands,
            .suggestion_count = 3,
            .current_index = 2,
            .query = NULL,
            .placeholder = "Run",
    };

    setup_box();

    hcw_auto_suggest_box_apply_standard_style(EGUI_VIEW_OF(&test_box));
    EGUI_TEST_ASSERT_EQUAL_INT(34, test_box.collapsed_height);
    EGUI_TEST_ASSERT_EQUAL_INT(24, test_box.item_height);
    EGUI_TEST_ASSERT_EQUAL_INT(4, test_box.max_visible_items);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_BORDER_STRONG.full, test_box.popup_border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY_DARK.full, test_box.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_TEXT_STRONG.full, test_box.textinput.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_TEXT_SOFT.full, test_box.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_TEXT_SOFT.full, test_box.icon_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY_DARK.full, test_box.highlight_color.full);

    hcw_auto_suggest_box_apply_compact_style(EGUI_VIEW_OF(&test_box));
    EGUI_TEST_ASSERT_EQUAL_INT(28, test_box.collapsed_height);
    EGUI_TEST_ASSERT_EQUAL_INT(21, test_box.item_height);
    EGUI_TEST_ASSERT_EQUAL_INT(3, test_box.max_visible_items);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY_DARK.full, test_box.highlight_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_BORDER_STRONG.full, test_box.popup_border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_TEXT_SOFT.full, test_box.icon_color.full);

    hcw_auto_suggest_box_apply_read_only_style(EGUI_VIEW_OF(&test_box));
    EGUI_TEST_ASSERT_FALSE(egui_view_get_enable(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_TEXT_STRONG.full, test_box.textinput.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_TRACK_STRONG.full, test_box.popup_border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_TEXT.full, test_box.icon_color.full);

    font_before = test_box.textinput.font;
    egui_view_auto_suggest_box_set_font(EGUI_VIEW_OF(&test_box), NULL);
    EGUI_TEST_ASSERT_TRUE(test_box.textinput.font == font_before);
    egui_view_auto_suggest_box_set_font(EGUI_VIEW_OF(&test_box), (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_TRUE(test_box.textinput.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    egui_view_auto_suggest_box_set_icon_font(EGUI_VIEW_OF(&test_box), EGUI_FONT_ICON_MS_16);
    EGUI_TEST_ASSERT_TRUE(test_box.icon_font == EGUI_FONT_ICON_MS_16);
    egui_view_auto_suggest_box_set_max_visible_items(EGUI_VIEW_OF(&test_box), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_box.max_visible_items);

    egui_view_auto_suggest_box_apply_params(EGUI_VIEW_OF(&test_box), &params);
    EGUI_TEST_ASSERT_EQUAL_INT(1, EGUI_VIEW_OF(&test_box)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_box)->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(96, EGUI_VIEW_OF(&test_box)->region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(28, EGUI_VIEW_OF(&test_box)->region.size.height);
    EGUI_TEST_ASSERT_TRUE(strcmp("Dep", egui_view_auto_suggest_box_get_query(EGUI_VIEW_OF(&test_box))) == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_auto_suggest_box_get_filtered_count(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_auto_suggest_box_get_current_index(EGUI_VIEW_OF(&test_box)));

    egui_view_auto_suggest_box_init_with_params(EGUI_VIEW_OF(&params_box), uicode_get_core(), &init_params);
    EGUI_TEST_ASSERT_EQUAL_INT(4, EGUI_VIEW_OF(&params_box)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(5, EGUI_VIEW_OF(&params_box)->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(120, EGUI_VIEW_OF(&params_box)->region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(30, EGUI_VIEW_OF(&params_box)->region.size.height);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_auto_suggest_box_get_current_index(EGUI_VIEW_OF(&params_box)));
    EGUI_TEST_ASSERT_TRUE(strcmp("Deploy Worker", egui_view_auto_suggest_box_get_query(EGUI_VIEW_OF(&params_box))) == 0);
}

static void test_auto_suggest_box_wrapper_setters_clear_interaction_state(void)
{
    setup_box();
    layout_box(10, 20, 180, 34);

    egui_view_auto_suggest_box_expand(EGUI_VIEW_OF(&test_box));
    EGUI_VIEW_OF(&test_box)->is_pressed = 1;
    test_box.pressed_part = EGUI_VIEW_AUTO_SUGGEST_BOX_PART_FIELD;
    hcw_auto_suggest_box_set_suggestions(EGUI_VIEW_OF(&test_box), g_commands, EGUI_ARRAY_SIZE(g_commands));
    EGUI_TEST_ASSERT_FALSE(egui_view_auto_suggest_box_is_expanded(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_AUTO_SUGGEST_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_auto_suggest_box_get_filtered_count(EGUI_VIEW_OF(&test_box)));

    egui_view_auto_suggest_box_set_query(EGUI_VIEW_OF(&test_box), "Dep");
    egui_view_auto_suggest_box_expand(EGUI_VIEW_OF(&test_box));
    EGUI_VIEW_OF(&test_box)->is_pressed = 1;
    test_box.pressed_part = EGUI_VIEW_AUTO_SUGGEST_BOX_PART_ITEM;
    test_box.pressed_row = 1;
    hcw_auto_suggest_box_set_current_index(EGUI_VIEW_OF(&test_box), 2);
    EGUI_TEST_ASSERT_FALSE(egui_view_auto_suggest_box_is_expanded(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_AUTO_SUGGEST_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_auto_suggest_box_get_current_index(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_TRUE(strcmp("Deploy Worker", egui_view_auto_suggest_box_get_query(EGUI_VIEW_OF(&test_box))) == 0);

    egui_view_auto_suggest_box_expand(EGUI_VIEW_OF(&test_box));
    EGUI_VIEW_OF(&test_box)->is_pressed = 1;
    test_box.pressed_part = EGUI_VIEW_AUTO_SUGGEST_BOX_PART_FIELD;
    hcw_auto_suggest_box_apply_compact_style(EGUI_VIEW_OF(&test_box));
    EGUI_TEST_ASSERT_FALSE(egui_view_auto_suggest_box_is_expanded(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_AUTO_SUGGEST_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(28, test_box.collapsed_height);
    EGUI_TEST_ASSERT_EQUAL_INT(21, test_box.item_height);
}

static void test_auto_suggest_box_touch_expand_select_and_fit_height(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_box();
    layout_box(10, 20, 180, 34);
    get_view_center(&x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_TRUE(egui_view_auto_suggest_box_is_expanded(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(34 + 2 * 24, EGUI_VIEW_OF(&test_box)->region_screen.size.height);

    get_dropdown_item_center(1, &x, &y);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_FALSE(egui_view_auto_suggest_box_is_expanded(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_auto_suggest_box_get_current_index(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selected_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_last_selected);
    EGUI_TEST_ASSERT_TRUE(strcmp("Alicia Gomez", egui_view_auto_suggest_box_get_query(EGUI_VIEW_OF(&test_box))) == 0);

    setup_box();
    egui_view_auto_suggest_box_set_query(EGUI_VIEW_OF(&test_box), "A");
    layout_box(10, EGUI_CONFIG_SCREEN_HEIGHT - 80, 180, 34);
    get_view_center(&x, &y);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_TRUE(egui_view_auto_suggest_box_is_expanded(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(34 + 24, EGUI_VIEW_OF(&test_box)->region_screen.size.height);
}

static void test_auto_suggest_box_keyboard_query_navigation_and_commit(void)
{
    setup_box();
    egui_view_auto_suggest_box_set_suggestions(EGUI_VIEW_OF(&test_box), g_commands, EGUI_ARRAY_SIZE(g_commands));
    egui_view_auto_suggest_box_set_query(EGUI_VIEW_OF(&test_box), NULL);
    layout_box(10, 20, 180, 34);
    reset_listener_state();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_D));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_E));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_P));
    EGUI_TEST_ASSERT_TRUE(strcmp("dep", egui_view_auto_suggest_box_get_query(EGUI_VIEW_OF(&test_box))) == 0);
    EGUI_TEST_ASSERT_TRUE(egui_view_auto_suggest_box_is_expanded(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_auto_suggest_box_get_filtered_count(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selected_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_auto_suggest_box_get_current_index(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selected_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_auto_suggest_box_get_current_index(EGUI_VIEW_OF(&test_box)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(egui_view_auto_suggest_box_is_expanded(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selected_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_last_selected);
    EGUI_TEST_ASSERT_TRUE(strcmp("Deploy Worker", egui_view_auto_suggest_box_get_query(EGUI_VIEW_OF(&test_box))) == 0);

    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ESCAPE));
}

static void test_auto_suggest_box_disabled_and_empty_guard_input(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_box();
    layout_box(10, 20, 180, 34);
    get_view_center(&x, &y);

    egui_view_set_enable(EGUI_VIEW_OF(&test_box), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_FALSE(egui_view_auto_suggest_box_is_expanded(EGUI_VIEW_OF(&test_box)));

    setup_box();
    egui_view_auto_suggest_box_set_suggestions(EGUI_VIEW_OF(&test_box), NULL, 0);
    layout_box(10, 20, 180, 34);
    get_view_center(&x, &y);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_FALSE(egui_view_auto_suggest_box_is_expanded(EGUI_VIEW_OF(&test_box)));
}

static void test_auto_suggest_box_static_preview_consumes_input_and_keeps_state(void)
{
    auto_suggest_box_preview_snapshot_t initial_snapshot;
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_box();
    layout_preview_box();
    capture_preview_snapshot(&initial_snapshot);

    egui_view_auto_suggest_box_expand(EGUI_VIEW_OF(&preview_box));
    EGUI_VIEW_OF(&preview_box)->is_pressed = 1;
    preview_box.pressed_part = EGUI_VIEW_AUTO_SUGGEST_BOX_PART_ITEM;
    preview_box.pressed_row = 1;
    x = EGUI_VIEW_OF(&preview_box)->region_screen.location.x + EGUI_VIEW_OF(&preview_box)->region_screen.size.width / 2;
    y = EGUI_VIEW_OF(&preview_box)->region_screen.location.y + preview_box.collapsed_height / 2;
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_preview_state_unchanged(&initial_snapshot);

    egui_view_auto_suggest_box_expand(EGUI_VIEW_OF(&preview_box));
    EGUI_VIEW_OF(&preview_box)->is_pressed = 1;
    preview_box.pressed_part = EGUI_VIEW_AUTO_SUGGEST_BOX_PART_FIELD;
    preview_box.pressed_row = EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE;
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_DOWN));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_auto_suggest_box_run(void)
{
    EGUI_TEST_SUITE_BEGIN(auto_suggest_box);
    EGUI_TEST_RUN(test_auto_suggest_box_query_filter_and_current_index);
    EGUI_TEST_RUN(test_auto_suggest_box_style_helpers_and_params);
    EGUI_TEST_RUN(test_auto_suggest_box_wrapper_setters_clear_interaction_state);
    EGUI_TEST_RUN(test_auto_suggest_box_touch_expand_select_and_fit_height);
    EGUI_TEST_RUN(test_auto_suggest_box_keyboard_query_navigation_and_commit);
    EGUI_TEST_RUN(test_auto_suggest_box_disabled_and_empty_guard_input);
    EGUI_TEST_RUN(test_auto_suggest_box_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
