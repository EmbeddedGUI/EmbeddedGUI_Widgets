#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_toolbar.h"

#include "../../HelloCustomWidgets/input/toolbar/egui_view_toolbar.h"
#include "../../HelloCustomWidgets/input/toolbar/egui_view_toolbar.c"

typedef struct toolbar_preview_snapshot toolbar_preview_snapshot_t;
struct toolbar_preview_snapshot
{
    egui_region_t region_screen;
    egui_view_toolbar_item_t items[EGUI_VIEW_TOOLBAR_MAX_ITEMS];
    const egui_font_t *label_font;
    const egui_font_t *icon_font;
    const egui_view_api_t *api;
    egui_color_t surface_color;
    egui_color_t item_color;
    egui_color_t checked_color;
    egui_color_t pressed_color;
    egui_color_t border_color;
    egui_color_t focus_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t icon_color;
    egui_color_t checked_icon_color;
    uint8_t item_count;
    uint8_t current_index;
    uint8_t pressed_index;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_pressed;
    uint8_t is_focused;
};

static egui_view_toolbar_t test_toolbar;
static egui_view_toolbar_t preview_toolbar;
static egui_view_api_t preview_api;
static uint8_t g_action_count;
static uint8_t g_last_action_index;

static const egui_view_toolbar_item_t default_items[] = {
        {"Edit", EGUI_ICON_MS_EDIT, EGUI_VIEW_TOOLBAR_ITEM_TOGGLE, 1, 0},
        {"Find", EGUI_ICON_MS_SEARCH, EGUI_VIEW_TOOLBAR_ITEM_TOGGLE, 0, 0},
        {"Sync", EGUI_ICON_MS_SYNC, EGUI_VIEW_TOOLBAR_ITEM_TOGGLE, 0, 0},
        {"Done", EGUI_ICON_MS_DONE, EGUI_VIEW_TOOLBAR_ITEM_TOGGLE, 0, 0},
};

static const egui_view_toolbar_item_t short_items[] = {
        {"Edit", EGUI_ICON_MS_EDIT, EGUI_VIEW_TOOLBAR_ITEM_TOGGLE, 1, 0},
        {"Done", EGUI_ICON_MS_DONE, EGUI_VIEW_TOOLBAR_ITEM_TOGGLE, 0, 0},
};

static void on_action(egui_view_t *self, uint8_t index)
{
    EGUI_UNUSED(self);
    g_action_count++;
    g_last_action_index = index;
}

static void reset_action_state(void)
{
    g_action_count = 0;
    g_last_action_index = EGUI_VIEW_TOOLBAR_INDEX_NONE;
}

static void setup_toolbar(void)
{
    egui_view_toolbar_init(EGUI_VIEW_OF(&test_toolbar));
    egui_view_set_size(EGUI_VIEW_OF(&test_toolbar), 212, 48);
    egui_view_toolbar_set_items(EGUI_VIEW_OF(&test_toolbar), default_items, (uint8_t)EGUI_ARRAY_SIZE(default_items));
    egui_view_toolbar_set_fonts(EGUI_VIEW_OF(&test_toolbar), NULL, NULL);
    egui_view_toolbar_set_on_action_listener(EGUI_VIEW_OF(&test_toolbar), on_action);
    reset_action_state();
}

static void setup_preview_toolbar(void)
{
    egui_view_toolbar_init(EGUI_VIEW_OF(&preview_toolbar));
    egui_view_set_size(EGUI_VIEW_OF(&preview_toolbar), 104, 42);
    egui_view_toolbar_set_items(EGUI_VIEW_OF(&preview_toolbar), short_items, (uint8_t)EGUI_ARRAY_SIZE(short_items));
    egui_view_toolbar_set_fonts(EGUI_VIEW_OF(&preview_toolbar), NULL, NULL);
    egui_view_toolbar_set_compact_mode(EGUI_VIEW_OF(&preview_toolbar), 1);
    egui_view_toolbar_override_static_preview_api(EGUI_VIEW_OF(&preview_toolbar), &preview_api);
    reset_action_state();
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

static void layout_toolbar(void)
{
    layout_view(EGUI_VIEW_OF(&test_toolbar), 10, 20, 212, 48);
}

static void layout_preview_toolbar(void)
{
    layout_view(EGUI_VIEW_OF(&preview_toolbar), 12, 18, 104, 42);
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

static int send_key_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->dispatch_key_event(view, &event);
}

static int send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&test_toolbar), type, x, y);
}

static int send_key_action(uint8_t type, uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_toolbar), type, key_code);
}

static int send_key(uint8_t key_code)
{
    int handled = 0;

    handled |= send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= send_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static uint8_t get_item_center(egui_view_t *view, uint8_t index, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!egui_view_toolbar_get_item_region(view, index, &region))
    {
        return 0;
    }
    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
    return 1;
}

static void get_view_outside_point(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x - 5;
    *y = view->region_screen.location.y - 5;
}

static void seed_pressed_state(egui_view_toolbar_t *toolbar, uint8_t index, uint8_t visual_pressed)
{
    toolbar->pressed_index = index;
    egui_view_set_pressed(EGUI_VIEW_OF(toolbar), visual_pressed ? 1 : 0);
}

static void assert_pressed_cleared(egui_view_toolbar_t *toolbar)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOOLBAR_INDEX_NONE, toolbar->pressed_index);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(toolbar)->is_pressed);
}

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void capture_preview_snapshot(toolbar_preview_snapshot_t *snapshot)
{
    uint8_t index;

    snapshot->region_screen = EGUI_VIEW_OF(&preview_toolbar)->region_screen;
    for (index = 0; index < EGUI_VIEW_TOOLBAR_MAX_ITEMS; ++index)
    {
        snapshot->items[index] = preview_toolbar.items[index];
    }
    snapshot->label_font = preview_toolbar.label_font;
    snapshot->icon_font = preview_toolbar.icon_font;
    snapshot->api = EGUI_VIEW_OF(&preview_toolbar)->api;
    snapshot->surface_color = preview_toolbar.surface_color;
    snapshot->item_color = preview_toolbar.item_color;
    snapshot->checked_color = preview_toolbar.checked_color;
    snapshot->pressed_color = preview_toolbar.pressed_color;
    snapshot->border_color = preview_toolbar.border_color;
    snapshot->focus_color = preview_toolbar.focus_color;
    snapshot->text_color = preview_toolbar.text_color;
    snapshot->muted_text_color = preview_toolbar.muted_text_color;
    snapshot->icon_color = preview_toolbar.icon_color;
    snapshot->checked_icon_color = preview_toolbar.checked_icon_color;
    snapshot->item_count = preview_toolbar.item_count;
    snapshot->current_index = preview_toolbar.current_index;
    snapshot->pressed_index = preview_toolbar.pressed_index;
    snapshot->compact_mode = preview_toolbar.compact_mode;
    snapshot->read_only_mode = preview_toolbar.read_only_mode;
    snapshot->alpha = EGUI_VIEW_OF(&preview_toolbar)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_toolbar));
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_toolbar)->is_pressed;
    snapshot->is_focused = EGUI_VIEW_OF(&preview_toolbar)->is_focused;
}

static void assert_preview_state_unchanged(const toolbar_preview_snapshot_t *snapshot)
{
    uint8_t index;

    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_toolbar)->region_screen);
    for (index = 0; index < EGUI_VIEW_TOOLBAR_MAX_ITEMS; ++index)
    {
        EGUI_TEST_ASSERT_TRUE(snapshot->items[index].label == preview_toolbar.items[index].label);
        EGUI_TEST_ASSERT_TRUE(snapshot->items[index].icon == preview_toolbar.items[index].icon);
        EGUI_TEST_ASSERT_EQUAL_INT(snapshot->items[index].kind, preview_toolbar.items[index].kind);
        EGUI_TEST_ASSERT_EQUAL_INT(snapshot->items[index].checked, preview_toolbar.items[index].checked);
        EGUI_TEST_ASSERT_EQUAL_INT(snapshot->items[index].disabled, preview_toolbar.items[index].disabled);
    }
    EGUI_TEST_ASSERT_TRUE(snapshot->label_font == preview_toolbar.label_font);
    EGUI_TEST_ASSERT_TRUE(snapshot->icon_font == preview_toolbar.icon_font);
    EGUI_TEST_ASSERT_TRUE(snapshot->api == EGUI_VIEW_OF(&preview_toolbar)->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_toolbar.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->item_color.full, preview_toolbar.item_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->checked_color.full, preview_toolbar.checked_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->pressed_color.full, preview_toolbar.pressed_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_toolbar.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->focus_color.full, preview_toolbar.focus_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_toolbar.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_toolbar.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->icon_color.full, preview_toolbar.icon_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->checked_icon_color.full, preview_toolbar.checked_icon_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->item_count, preview_toolbar.item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_index, preview_toolbar.current_index);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->pressed_index, preview_toolbar.pressed_index);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_toolbar.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_toolbar.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_toolbar)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_toolbar)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_toolbar)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_toolbar)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
}

static void test_toolbar_internal_helpers_cover_text_fitting(void)
{
    char label[24];

    setup_toolbar();

    EGUI_TEST_ASSERT_FALSE(egui_view_toolbar_has_text(NULL));
    EGUI_TEST_ASSERT_TRUE(egui_view_toolbar_has_text("Edit"));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_toolbar_text_len("Sync"));
    EGUI_TEST_ASSERT_TRUE(egui_view_toolbar_is_space_char(' '));
    EGUI_TEST_ASSERT_FALSE(egui_view_toolbar_is_space_char('x'));
    EGUI_TEST_ASSERT_TRUE(egui_view_toolbar_is_break_after_char('-'));
    EGUI_TEST_ASSERT_TRUE(egui_view_toolbar_is_break_after_char('/'));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_toolbar_find_elide_boundary("Open latest", 7));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_toolbar_find_elide_boundary("scan-first", 5));
    egui_view_toolbar_copy_elided(label, sizeof(label), "Sync tools", 6);
    EGUI_TEST_ASSERT_TRUE(strcmp("Syn...", label) == 0);
    egui_view_toolbar_copy_elided(label, sizeof(label), "Mode", 3);
    EGUI_TEST_ASSERT_TRUE(strcmp("...", label) == 0);
}

static void test_toolbar_setters_clear_pressed_and_update_state(void)
{
    setup_toolbar();

    seed_pressed_state(&test_toolbar, 0, 1);
    egui_view_toolbar_set_current_index(EGUI_VIEW_OF(&test_toolbar), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_toolbar.current_index);
    assert_pressed_cleared(&test_toolbar);

    seed_pressed_state(&test_toolbar, 1, 1);
    egui_view_toolbar_set_item_checked(EGUI_VIEW_OF(&test_toolbar), 2, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_toolbar.current_index);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_toolbar.items[0].checked);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_toolbar.items[2].checked);
    assert_pressed_cleared(&test_toolbar);

    seed_pressed_state(&test_toolbar, 2, 1);
    egui_view_toolbar_set_item_disabled(EGUI_VIEW_OF(&test_toolbar), 2, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_toolbar.items[2].disabled);
    EGUI_TEST_ASSERT_TRUE(test_toolbar.current_index != 2);
    assert_pressed_cleared(&test_toolbar);

    seed_pressed_state(&test_toolbar, 0, 1);
    egui_view_toolbar_set_fonts(EGUI_VIEW_OF(&test_toolbar), (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_FONT_ICON_MS_20);
    EGUI_TEST_ASSERT_TRUE(test_toolbar.label_font == (const egui_font_t *)&egui_res_font_montserrat_8_4);
    EGUI_TEST_ASSERT_TRUE(test_toolbar.icon_font == EGUI_FONT_ICON_MS_20);
    assert_pressed_cleared(&test_toolbar);

    seed_pressed_state(&test_toolbar, 0, 1);
    egui_view_toolbar_set_compact_mode(EGUI_VIEW_OF(&test_toolbar), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_toolbar.compact_mode);
    assert_pressed_cleared(&test_toolbar);

    seed_pressed_state(&test_toolbar, 0, 1);
    egui_view_toolbar_set_read_only_mode(EGUI_VIEW_OF(&test_toolbar), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_toolbar.read_only_mode);
    assert_pressed_cleared(&test_toolbar);

    seed_pressed_state(&test_toolbar, 0, 1);
    egui_view_toolbar_set_palette(EGUI_VIEW_OF(&test_toolbar), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122),
                                  EGUI_COLOR_HEX(0x303132), EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152),
                                  EGUI_COLOR_HEX(0x606162), EGUI_COLOR_HEX(0x707172), EGUI_COLOR_HEX(0x808182),
                                  EGUI_COLOR_HEX(0x909192), EGUI_COLOR_HEX(0xA0A1A2));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_toolbar.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xA0A1A2).full, test_toolbar.checked_icon_color.full);
    assert_pressed_cleared(&test_toolbar);

    seed_pressed_state(&test_toolbar, 0, 1);
    egui_view_toolbar_set_items(EGUI_VIEW_OF(&test_toolbar), short_items, (uint8_t)EGUI_ARRAY_SIZE(short_items));
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_toolbar.item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_toolbar.current_index);
    assert_pressed_cleared(&test_toolbar);
}

static void test_toolbar_activate_listener_and_toggle_group(void)
{
    setup_toolbar();

    EGUI_TEST_ASSERT_TRUE(egui_view_toolbar_activate_item(EGUI_VIEW_OF(&test_toolbar), 1));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_last_action_index);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_toolbar.current_index);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_toolbar.items[0].checked);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_toolbar.items[1].checked);

    egui_view_toolbar_set_item_disabled(EGUI_VIEW_OF(&test_toolbar), 2, 1);
    EGUI_TEST_ASSERT_FALSE(egui_view_toolbar_activate_item(EGUI_VIEW_OF(&test_toolbar), 2));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);

    egui_view_toolbar_set_on_action_listener(EGUI_VIEW_OF(&test_toolbar), NULL);
    EGUI_TEST_ASSERT_TRUE(egui_view_toolbar_activate_item(EGUI_VIEW_OF(&test_toolbar), 3));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(3, test_toolbar.current_index);
}

static void test_toolbar_touch_same_target_release_and_cancel(void)
{
    egui_dim_t item_x;
    egui_dim_t item_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_toolbar();
    layout_toolbar();
    EGUI_TEST_ASSERT_TRUE(get_item_center(EGUI_VIEW_OF(&test_toolbar), 1, &item_x, &item_y));
    get_view_outside_point(EGUI_VIEW_OF(&test_toolbar), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_toolbar.pressed_index);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_toolbar)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_toolbar)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    assert_pressed_cleared(&test_toolbar);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, item_x, item_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, item_x, item_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_last_action_index);
    assert_pressed_cleared(&test_toolbar);

    reset_action_state();
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, item_x, item_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    assert_pressed_cleared(&test_toolbar);
}

static void test_toolbar_keyboard_navigation_and_activate(void)
{
    setup_toolbar();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_toolbar.current_index);

    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_toolbar)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_toolbar.pressed_index);
    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_last_action_index);
    assert_pressed_cleared(&test_toolbar);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(3, test_toolbar.current_index);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(3, g_last_action_index);

    egui_view_toolbar_set_item_disabled(EGUI_VIEW_OF(&test_toolbar), 2, 1);
    egui_view_toolbar_set_current_index(EGUI_VIEW_OF(&test_toolbar), 1);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(3, test_toolbar.current_index);

    seed_pressed_state(&test_toolbar, 3, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_TAB));
    assert_pressed_cleared(&test_toolbar);
}

static void test_toolbar_read_only_and_disabled_guards(void)
{
    egui_dim_t item_x;
    egui_dim_t item_y;

    setup_toolbar();
    layout_toolbar();
    EGUI_TEST_ASSERT_TRUE(get_item_center(EGUI_VIEW_OF(&test_toolbar), 1, &item_x, &item_y));

    egui_view_toolbar_set_read_only_mode(EGUI_VIEW_OF(&test_toolbar), 1);
    seed_pressed_state(&test_toolbar, 1, 1);
    EGUI_TEST_ASSERT_FALSE(egui_view_toolbar_activate_item(EGUI_VIEW_OF(&test_toolbar), 1));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(&test_toolbar);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);

    egui_view_toolbar_set_read_only_mode(EGUI_VIEW_OF(&test_toolbar), 0);
    egui_view_set_enable(EGUI_VIEW_OF(&test_toolbar), 0);
    seed_pressed_state(&test_toolbar, 1, 1);
    EGUI_TEST_ASSERT_FALSE(egui_view_toolbar_activate_item(EGUI_VIEW_OF(&test_toolbar), 1));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_SPACE));
    assert_pressed_cleared(&test_toolbar);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
}

static void test_toolbar_static_preview_consumes_input_and_keeps_state(void)
{
    toolbar_preview_snapshot_t initial_snapshot;
    egui_dim_t item_x;
    egui_dim_t item_y;

    setup_preview_toolbar();
    layout_preview_toolbar();
    EGUI_TEST_ASSERT_TRUE(get_item_center(EGUI_VIEW_OF(&preview_toolbar), 0, &item_x, &item_y));
    capture_preview_snapshot(&initial_snapshot);

    seed_pressed_state(&preview_toolbar, 0, 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_toolbar), EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    assert_preview_state_unchanged(&initial_snapshot);

    seed_pressed_state(&preview_toolbar, 0, 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_toolbar), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_toolbar), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_toolbar_run(void)
{
    EGUI_TEST_SUITE_BEGIN(toolbar);
    EGUI_TEST_RUN(test_toolbar_internal_helpers_cover_text_fitting);
    EGUI_TEST_RUN(test_toolbar_setters_clear_pressed_and_update_state);
    EGUI_TEST_RUN(test_toolbar_activate_listener_and_toggle_group);
    EGUI_TEST_RUN(test_toolbar_touch_same_target_release_and_cancel);
    EGUI_TEST_RUN(test_toolbar_keyboard_navigation_and_activate);
    EGUI_TEST_RUN(test_toolbar_read_only_and_disabled_guards);
    EGUI_TEST_RUN(test_toolbar_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
