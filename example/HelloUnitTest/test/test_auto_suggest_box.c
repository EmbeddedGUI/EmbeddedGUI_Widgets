#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_auto_suggest_box.h"

#include "../../HelloCustomWidgets/input/auto_suggest_box/egui_view_auto_suggest_box.h"
#include "../../HelloCustomWidgets/input/auto_suggest_box/egui_view_auto_suggest_box.c"

static egui_view_autocomplete_t test_box;
static egui_view_autocomplete_t preview_box;
static egui_view_api_t preview_api;
static uint8_t g_selected_count;
static uint8_t g_last_selected;

typedef struct
{
    egui_region_t region_screen;
    const char **items;
    const char **item_icons;
    const char *current_text;
    const egui_font_t *font;
    const egui_font_t *icon_font;
    const char *expand_icon;
    const char *collapse_icon;
    uint8_t item_count;
    uint8_t current_index;
    uint8_t is_expanded;
    uint8_t max_visible_items;
    egui_alpha_t alpha;
    egui_color_t text_color;
    egui_color_t bg_color;
    egui_color_t border_color;
    egui_color_t highlight_color;
    egui_color_t arrow_color;
    egui_dim_t collapsed_height;
    egui_dim_t item_height;
    egui_dim_t icon_text_gap;
} auto_suggest_box_preview_snapshot_t;

static const char *g_people[] = {"Alice Chen", "Alicia Gomez", "Allen Park", "Amelia Stone"};
static const char *g_people_icons[] = {"A", "B", "C", "D"};
static const char *g_commands[] = {"Deploy API", "Deploy Docs"};

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
    egui_view_autocomplete_init(EGUI_VIEW_OF(&test_box));
    egui_view_set_size(EGUI_VIEW_OF(&test_box), 180, 34);
    egui_view_autocomplete_set_suggestions(EGUI_VIEW_OF(&test_box), g_people, 4);
    egui_view_autocomplete_set_on_selected_listener(EGUI_VIEW_OF(&test_box), on_selected);
    hcw_auto_suggest_box_apply_standard_style(EGUI_VIEW_OF(&test_box));
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
    egui_view_autocomplete_init(EGUI_VIEW_OF(&preview_box));
    egui_view_set_size(EGUI_VIEW_OF(&preview_box), 104, 28);
    hcw_auto_suggest_box_set_suggestions(EGUI_VIEW_OF(&preview_box), g_commands, 2);
    hcw_auto_suggest_box_set_current_index(EGUI_VIEW_OF(&preview_box), 1);
    hcw_auto_suggest_box_apply_compact_style(EGUI_VIEW_OF(&preview_box));
    egui_view_autocomplete_set_on_selected_listener(EGUI_VIEW_OF(&preview_box), on_selected);
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

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_box), key_code);
}

static void get_view_center(egui_dim_t *x, egui_dim_t *y)
{
    *x = EGUI_VIEW_OF(&test_box)->region_screen.location.x + EGUI_VIEW_OF(&test_box)->region_screen.size.width / 2;
    *y = EGUI_VIEW_OF(&test_box)->region_screen.location.y + ((egui_view_combobox_t *)EGUI_VIEW_OF(&test_box))->collapsed_height / 2;
}

static void capture_preview_snapshot(auto_suggest_box_preview_snapshot_t *snapshot)
{
    egui_view_combobox_t *local = (egui_view_combobox_t *)EGUI_VIEW_OF(&preview_box);

    snapshot->region_screen = EGUI_VIEW_OF(&preview_box)->region_screen;
    snapshot->items = local->items;
    snapshot->item_icons = local->item_icons;
    snapshot->current_text = egui_view_autocomplete_get_current_text(EGUI_VIEW_OF(&preview_box));
    snapshot->font = local->font;
    snapshot->icon_font = local->icon_font;
    snapshot->expand_icon = local->expand_icon;
    snapshot->collapse_icon = local->collapse_icon;
    snapshot->item_count = local->item_count;
    snapshot->current_index = local->current_index;
    snapshot->is_expanded = local->is_expanded;
    snapshot->max_visible_items = local->max_visible_items;
    snapshot->alpha = local->alpha;
    snapshot->text_color = local->text_color;
    snapshot->bg_color = local->bg_color;
    snapshot->border_color = local->border_color;
    snapshot->highlight_color = local->highlight_color;
    snapshot->arrow_color = local->arrow_color;
    snapshot->collapsed_height = local->collapsed_height;
    snapshot->item_height = local->item_height;
    snapshot->icon_text_gap = local->icon_text_gap;
}

static void assert_preview_state_unchanged(const auto_suggest_box_preview_snapshot_t *snapshot)
{
    egui_view_combobox_t *local = (egui_view_combobox_t *)EGUI_VIEW_OF(&preview_box);
    const char *current_text = egui_view_autocomplete_get_current_text(EGUI_VIEW_OF(&preview_box));

    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_box)->region_screen);
    EGUI_TEST_ASSERT_TRUE(local->items == snapshot->items);
    EGUI_TEST_ASSERT_TRUE(local->item_icons == snapshot->item_icons);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->item_count, local->item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_index, local->current_index);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_expanded, local->is_expanded);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->max_visible_items, local->max_visible_items);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, local->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, local->text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->bg_color.full, local->bg_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, local->border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->highlight_color.full, local->highlight_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->arrow_color.full, local->arrow_color.full);
    EGUI_TEST_ASSERT_TRUE(local->font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(local->icon_font == snapshot->icon_font);
    EGUI_TEST_ASSERT_TRUE(strcmp(snapshot->expand_icon, local->expand_icon) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp(snapshot->collapse_icon, local->collapse_icon) == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->collapsed_height, local->collapsed_height);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->item_height, local->item_height);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->icon_text_gap, local->icon_text_gap);
    EGUI_TEST_ASSERT_TRUE(strcmp(snapshot->current_text, current_text) == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selected_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0xFF, g_last_selected);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMBOBOX_PRESSED_NONE, local->pressed_index);
    EGUI_TEST_ASSERT_EQUAL_INT(0, local->pressed_is_header);
}

static uint8_t get_visible_count(void)
{
    egui_view_combobox_t *local = (egui_view_combobox_t *)EGUI_VIEW_OF(&test_box);
    uint8_t visible_count = local->item_count;
    egui_dim_t item_space;
    uint8_t fit_count;

    if (visible_count > local->max_visible_items)
    {
        visible_count = local->max_visible_items;
    }
    if (!local->is_expanded || visible_count == 0 || local->item_height <= 0 || EGUI_VIEW_OF(&test_box)->region_screen.size.height <= local->collapsed_height)
    {
        return visible_count;
    }

    item_space = EGUI_VIEW_OF(&test_box)->region_screen.size.height - local->collapsed_height;
    fit_count = (uint8_t)(item_space / local->item_height);
    if (fit_count < visible_count)
    {
        visible_count = fit_count;
    }

    return visible_count;
}

static uint8_t get_visible_start_index(uint8_t visible_count)
{
    egui_view_combobox_t *local = (egui_view_combobox_t *)EGUI_VIEW_OF(&test_box);
    uint8_t start_index = 0;

    if (visible_count == 0 || local->item_count <= visible_count)
    {
        return 0;
    }
    if (local->current_index >= visible_count)
    {
        start_index = (uint8_t)(local->current_index + 1 - visible_count);
    }
    if ((uint16_t)start_index + visible_count > local->item_count)
    {
        start_index = (uint8_t)(local->item_count - visible_count);
    }

    return start_index;
}

static void get_dropdown_item_center(uint8_t item_index, egui_dim_t *x, egui_dim_t *y)
{
    egui_view_combobox_t *local = (egui_view_combobox_t *)EGUI_VIEW_OF(&test_box);
    uint8_t visible_count = get_visible_count();
    uint8_t start_index = get_visible_start_index(visible_count);
    uint8_t row_index;

    EGUI_TEST_ASSERT_TRUE(local->is_expanded);
    EGUI_TEST_ASSERT_TRUE(visible_count > 0);

    if (item_index < start_index)
    {
        row_index = 0;
    }
    else if (item_index >= (uint8_t)(start_index + visible_count))
    {
        row_index = (uint8_t)(visible_count - 1);
    }
    else
    {
        row_index = (uint8_t)(item_index - start_index);
    }

    *x = EGUI_VIEW_OF(&test_box)->region_screen.location.x + EGUI_VIEW_OF(&test_box)->region_screen.size.width / 2;
    *y = EGUI_VIEW_OF(&test_box)->region_screen.location.y + local->collapsed_height + row_index * local->item_height + local->item_height / 2;
}

static void test_auto_suggest_box_suggestions_current_index_and_text(void)
{
    setup_box();

    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_autocomplete_get_suggestion_count(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_autocomplete_get_current_index(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_TRUE(strcmp("Alice Chen", egui_view_autocomplete_get_current_text(EGUI_VIEW_OF(&test_box))) == 0);

    egui_view_autocomplete_set_current_index(EGUI_VIEW_OF(&test_box), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_autocomplete_get_current_index(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_TRUE(strcmp("Allen Park", egui_view_autocomplete_get_current_text(EGUI_VIEW_OF(&test_box))) == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selected_count);

    egui_view_autocomplete_set_current_index(EGUI_VIEW_OF(&test_box), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_autocomplete_get_current_index(EGUI_VIEW_OF(&test_box)));

    egui_view_autocomplete_set_suggestions(EGUI_VIEW_OF(&test_box), g_commands, 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_autocomplete_get_suggestion_count(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_autocomplete_get_current_index(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_TRUE(strcmp("Deploy API", egui_view_autocomplete_get_current_text(EGUI_VIEW_OF(&test_box))) == 0);

    egui_view_autocomplete_set_suggestions(EGUI_VIEW_OF(&test_box), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_autocomplete_get_suggestion_count(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_autocomplete_get_current_index(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_NULL(egui_view_autocomplete_get_current_text(EGUI_VIEW_OF(&test_box)));
}

static void test_auto_suggest_box_style_helpers_and_params(void)
{
    egui_view_autocomplete_t params_box;
    egui_view_combobox_t *local = (egui_view_combobox_t *)EGUI_VIEW_OF(&test_box);
    const egui_font_t *font_before;
    egui_view_autocomplete_params_t params = {
            .region = {{1, 2}, {96, 28}},
            .suggestions = g_commands,
            .suggestion_count = 2,
            .current_index = 1,
    };
    egui_view_autocomplete_params_t init_params = {
            .region = {{4, 5}, {120, 30}},
            .suggestions = g_commands,
            .suggestion_count = 2,
            .current_index = 9,
    };

    setup_box();

    hcw_auto_suggest_box_apply_standard_style(EGUI_VIEW_OF(&test_box));
    EGUI_TEST_ASSERT_EQUAL_INT(34, local->collapsed_height);
    EGUI_TEST_ASSERT_EQUAL_INT(24, local->item_height);
    EGUI_TEST_ASSERT_EQUAL_INT(4, local->max_visible_items);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xD5DCE4).full, local->border_color.full);

    hcw_auto_suggest_box_apply_compact_style(EGUI_VIEW_OF(&test_box));
    EGUI_TEST_ASSERT_EQUAL_INT(28, local->collapsed_height);
    EGUI_TEST_ASSERT_EQUAL_INT(21, local->item_height);
    EGUI_TEST_ASSERT_EQUAL_INT(3, local->max_visible_items);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xEAF3FB).full, local->highlight_color.full);

    hcw_auto_suggest_box_apply_read_only_style(EGUI_VIEW_OF(&test_box));
    EGUI_TEST_ASSERT_EQUAL_INT(28, local->collapsed_height);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xFFFFFF).full, local->bg_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x7A8796).full, local->arrow_color.full);

    font_before = local->font;
    egui_view_autocomplete_set_font(EGUI_VIEW_OF(&test_box), NULL);
    EGUI_TEST_ASSERT_TRUE(local->font == font_before);
    egui_view_autocomplete_set_font(EGUI_VIEW_OF(&test_box), (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_TRUE(local->font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    egui_view_autocomplete_set_max_visible_items(EGUI_VIEW_OF(&test_box), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(1, local->max_visible_items);
    egui_view_combobox_set_item_icons(EGUI_VIEW_OF(&test_box), g_people_icons);
    EGUI_TEST_ASSERT_TRUE(local->item_icons == g_people_icons);
    egui_view_combobox_set_icon_font(EGUI_VIEW_OF(&test_box), EGUI_FONT_ICON_MS_16);
    EGUI_TEST_ASSERT_TRUE(local->icon_font == EGUI_FONT_ICON_MS_16);
    egui_view_combobox_set_arrow_icons(EGUI_VIEW_OF(&test_box), ">", "<");
    EGUI_TEST_ASSERT_TRUE(strcmp(">", local->expand_icon) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp("<", local->collapse_icon) == 0);
    egui_view_combobox_set_arrow_icons(EGUI_VIEW_OF(&test_box), NULL, NULL);
    egui_view_combobox_set_icon_font(EGUI_VIEW_OF(&test_box), EGUI_FONT_ICON_MS_16);
    EGUI_TEST_ASSERT_TRUE(strcmp(local->expand_icon, EGUI_ICON_MS_EXPAND_MORE) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp(local->collapse_icon, EGUI_ICON_MS_EXPAND_LESS) == 0);
    egui_view_combobox_set_icon_text_gap(EGUI_VIEW_OF(&test_box), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(3, local->icon_text_gap);

    egui_view_autocomplete_apply_params(EGUI_VIEW_OF(&test_box), &params);
    EGUI_TEST_ASSERT_EQUAL_INT(1, EGUI_VIEW_OF(&test_box)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_box)->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(96, EGUI_VIEW_OF(&test_box)->region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(28, EGUI_VIEW_OF(&test_box)->region.size.height);
    EGUI_TEST_ASSERT_EQUAL_INT(28, local->collapsed_height);
    EGUI_TEST_ASSERT_EQUAL_INT(2, local->item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, local->current_index);
    EGUI_TEST_ASSERT_TRUE(strcmp("Deploy Docs", egui_view_autocomplete_get_current_text(EGUI_VIEW_OF(&test_box))) == 0);

    egui_view_autocomplete_init_with_params(EGUI_VIEW_OF(&params_box), &init_params);
    EGUI_TEST_ASSERT_EQUAL_INT(4, EGUI_VIEW_OF(&params_box)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(5, EGUI_VIEW_OF(&params_box)->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(120, EGUI_VIEW_OF(&params_box)->region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(30, EGUI_VIEW_OF(&params_box)->region.size.height);
    EGUI_TEST_ASSERT_EQUAL_INT(30, params_box.combobox.collapsed_height);
    EGUI_TEST_ASSERT_EQUAL_INT(2, params_box.combobox.item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, params_box.combobox.current_index);
    EGUI_TEST_ASSERT_TRUE(strcmp("Deploy API", egui_view_autocomplete_get_current_text(EGUI_VIEW_OF(&params_box))) == 0);
}

static void test_auto_suggest_box_wrapper_setters_clear_interaction_state(void)
{
    egui_view_combobox_t *local;

    setup_box();
    layout_box(10, 20, 180, 34);
    local = (egui_view_combobox_t *)EGUI_VIEW_OF(&test_box);

    egui_view_autocomplete_expand(EGUI_VIEW_OF(&test_box));
    EGUI_VIEW_OF(&test_box)->is_pressed = 1;
    local->pressed_is_header = 1;
    local->pressed_index = EGUI_VIEW_COMBOBOX_PRESSED_NONE;
    hcw_auto_suggest_box_set_suggestions(EGUI_VIEW_OF(&test_box), g_commands, 2);
    EGUI_TEST_ASSERT_FALSE(egui_view_autocomplete_is_expanded(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMBOBOX_PRESSED_NONE, local->pressed_index);
    EGUI_TEST_ASSERT_EQUAL_INT(0, local->pressed_is_header);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_autocomplete_get_suggestion_count(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_autocomplete_get_current_index(EGUI_VIEW_OF(&test_box)));

    egui_view_autocomplete_expand(EGUI_VIEW_OF(&test_box));
    EGUI_VIEW_OF(&test_box)->is_pressed = 1;
    local->pressed_is_header = 0;
    local->pressed_index = 1;
    hcw_auto_suggest_box_set_current_index(EGUI_VIEW_OF(&test_box), 1);
    EGUI_TEST_ASSERT_FALSE(egui_view_autocomplete_is_expanded(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMBOBOX_PRESSED_NONE, local->pressed_index);
    EGUI_TEST_ASSERT_EQUAL_INT(0, local->pressed_is_header);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_autocomplete_get_current_index(EGUI_VIEW_OF(&test_box)));

    egui_view_autocomplete_expand(EGUI_VIEW_OF(&test_box));
    EGUI_VIEW_OF(&test_box)->is_pressed = 1;
    local->pressed_is_header = 1;
    local->pressed_index = EGUI_VIEW_COMBOBOX_PRESSED_NONE;
    hcw_auto_suggest_box_apply_compact_style(EGUI_VIEW_OF(&test_box));
    EGUI_TEST_ASSERT_FALSE(egui_view_autocomplete_is_expanded(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMBOBOX_PRESSED_NONE, local->pressed_index);
    EGUI_TEST_ASSERT_EQUAL_INT(0, local->pressed_is_header);
    EGUI_TEST_ASSERT_EQUAL_INT(28, local->collapsed_height);
    EGUI_TEST_ASSERT_EQUAL_INT(21, local->item_height);
}

static void test_auto_suggest_box_touch_expand_select_and_fit_height(void)
{
    egui_view_combobox_t *local = (egui_view_combobox_t *)EGUI_VIEW_OF(&test_box);
    egui_dim_t x;
    egui_dim_t y;

    setup_box();
    layout_box(10, 20, 180, 34);
    get_view_center(&x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_TRUE(egui_view_autocomplete_is_expanded(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(34 + 4 * 24, EGUI_VIEW_OF(&test_box)->region_screen.size.height);

    get_dropdown_item_center(2, &x, &y);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_FALSE(egui_view_autocomplete_is_expanded(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_autocomplete_get_current_index(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selected_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_last_selected);
    EGUI_TEST_ASSERT_EQUAL_INT(local->collapsed_height, EGUI_VIEW_OF(&test_box)->region_screen.size.height);

    setup_box();
    layout_box(10, EGUI_CONFIG_SCEEN_HEIGHT - 80, 180, 34);
    get_view_center(&x, &y);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_TRUE(egui_view_autocomplete_is_expanded(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(34 + 24, EGUI_VIEW_OF(&test_box)->region_screen.size.height);
}

static void test_auto_suggest_box_keyboard_navigation_and_commit(void)
{
    setup_box();
    layout_box(10, 20, 180, 34);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_TRUE(egui_view_autocomplete_is_expanded(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_autocomplete_get_current_index(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selected_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_autocomplete_get_current_index(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selected_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_last_selected);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_autocomplete_get_current_index(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_selected_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_autocomplete_get_current_index(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, g_selected_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_FALSE(egui_view_autocomplete_is_expanded(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, g_selected_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(egui_view_autocomplete_is_expanded(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, g_selected_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_FALSE(egui_view_autocomplete_is_expanded(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, g_selected_count);
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
    EGUI_TEST_ASSERT_FALSE(egui_view_autocomplete_is_expanded(EGUI_VIEW_OF(&test_box)));

    setup_box();
    egui_view_autocomplete_set_suggestions(EGUI_VIEW_OF(&test_box), NULL, 0);
    layout_box(10, 20, 180, 34);
    get_view_center(&x, &y);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_FALSE(egui_view_autocomplete_is_expanded(EGUI_VIEW_OF(&test_box)));
}

static void test_auto_suggest_box_static_preview_consumes_input_and_keeps_state(void)
{
    auto_suggest_box_preview_snapshot_t initial_snapshot;
    egui_view_combobox_t *local;
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_box();
    layout_preview_box();
    capture_preview_snapshot(&initial_snapshot);
    local = (egui_view_combobox_t *)EGUI_VIEW_OF(&preview_box);

    egui_view_autocomplete_expand(EGUI_VIEW_OF(&preview_box));
    EGUI_VIEW_OF(&preview_box)->is_pressed = 1;
    local->pressed_is_header = 0;
    local->pressed_index = 1;
    x = EGUI_VIEW_OF(&preview_box)->region_screen.location.x + EGUI_VIEW_OF(&preview_box)->region_screen.size.width / 2;
    y = EGUI_VIEW_OF(&preview_box)->region_screen.location.y + local->collapsed_height / 2;
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_preview_state_unchanged(&initial_snapshot);

    egui_view_autocomplete_expand(EGUI_VIEW_OF(&preview_box));
    EGUI_VIEW_OF(&preview_box)->is_pressed = 1;
    local->pressed_is_header = 1;
    local->pressed_index = EGUI_VIEW_COMBOBOX_PRESSED_NONE;
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_DOWN));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_auto_suggest_box_run(void)
{
    EGUI_TEST_SUITE_BEGIN(auto_suggest_box);
    EGUI_TEST_RUN(test_auto_suggest_box_suggestions_current_index_and_text);
    EGUI_TEST_RUN(test_auto_suggest_box_style_helpers_and_params);
    EGUI_TEST_RUN(test_auto_suggest_box_wrapper_setters_clear_interaction_state);
    EGUI_TEST_RUN(test_auto_suggest_box_touch_expand_select_and_fit_height);
    EGUI_TEST_RUN(test_auto_suggest_box_keyboard_navigation_and_commit);
    EGUI_TEST_RUN(test_auto_suggest_box_disabled_and_empty_guard_input);
    EGUI_TEST_RUN(test_auto_suggest_box_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
