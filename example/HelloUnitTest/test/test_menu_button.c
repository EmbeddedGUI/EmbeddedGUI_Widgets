#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_menu_button.h"

#include "../../HelloCustomWidgets/input/menu_button/egui_view_menu_button.h"
#include "../../HelloCustomWidgets/input/menu_button/egui_view_menu_button.c"

typedef struct menu_button_preview_snapshot menu_button_preview_snapshot_t;
struct menu_button_preview_snapshot
{
    egui_region_t region_screen;
    egui_view_menu_button_item_t items[EGUI_VIEW_MENU_BUTTON_MAX_ITEMS];
    const char *button_label;
    const char *button_icon;
    const char *menu_title;
    const egui_font_t *label_font;
    const egui_font_t *meta_font;
    const egui_font_t *icon_font;
    const egui_view_api_t *api;
    egui_color_t surface_color;
    egui_color_t menu_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t success_color;
    egui_color_t warning_color;
    egui_color_t danger_color;
    egui_color_t neutral_color;
    uint8_t item_count;
    uint8_t selected_index;
    uint8_t focus_index;
    uint8_t active_target;
    uint8_t is_open;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_pressed;
    uint8_t is_focused;
};

static egui_view_menu_button_t test_menu;
static egui_view_menu_button_t preview_menu;
static egui_view_api_t preview_api;
static uint8_t g_action_count;
static uint8_t g_last_action_index;

static const egui_view_menu_button_item_t default_items[] = {
        {"New page", EGUI_ICON_MS_ADD, "N", EGUI_VIEW_MENU_BUTTON_TONE_ACCENT, 1, 0},
        {"Duplicate", EGUI_ICON_MS_EDIT, "D", EGUI_VIEW_MENU_BUTTON_TONE_NEUTRAL, 0, 0},
        {"Export", EGUI_ICON_MS_DOWNLOAD, "E", EGUI_VIEW_MENU_BUTTON_TONE_SUCCESS, 0, 0},
        {"Archive", EGUI_ICON_MS_LOCK, "A", EGUI_VIEW_MENU_BUTTON_TONE_WARNING, 0, 0},
};

static const egui_view_menu_button_item_t disabled_items[] = {
        {"New page", EGUI_ICON_MS_ADD, "N", EGUI_VIEW_MENU_BUTTON_TONE_ACCENT, 1, 0},
        {"Duplicate", EGUI_ICON_MS_EDIT, "D", EGUI_VIEW_MENU_BUTTON_TONE_NEUTRAL, 0, 1},
        {"Export", EGUI_ICON_MS_DOWNLOAD, "E", EGUI_VIEW_MENU_BUTTON_TONE_SUCCESS, 0, 0},
};

static const egui_view_menu_button_item_t preview_items[] = {
        {"More", EGUI_ICON_MS_SETTINGS, "", EGUI_VIEW_MENU_BUTTON_TONE_ACCENT, 1, 0},
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
    g_last_action_index = EGUI_VIEW_MENU_BUTTON_INDEX_NONE;
}

static void setup_menu(void)
{
    egui_view_menu_button_init(EGUI_VIEW_OF(&test_menu));
    egui_view_set_size(EGUI_VIEW_OF(&test_menu), 206, 104);
    egui_view_menu_button_set_button(EGUI_VIEW_OF(&test_menu), "Page actions", EGUI_ICON_MS_SETTINGS);
    egui_view_menu_button_set_menu_title(EGUI_VIEW_OF(&test_menu), "Page menu");
    egui_view_menu_button_set_items(EGUI_VIEW_OF(&test_menu), default_items, (uint8_t)EGUI_ARRAY_SIZE(default_items));
    egui_view_menu_button_set_on_action_listener(EGUI_VIEW_OF(&test_menu), on_action);
    reset_action_state();
}

static void setup_preview_menu(void)
{
    egui_view_menu_button_init(EGUI_VIEW_OF(&preview_menu));
    egui_view_set_size(EGUI_VIEW_OF(&preview_menu), 104, 42);
    egui_view_menu_button_set_button(EGUI_VIEW_OF(&preview_menu), "More", EGUI_ICON_MS_SETTINGS);
    egui_view_menu_button_set_items(EGUI_VIEW_OF(&preview_menu), preview_items, (uint8_t)EGUI_ARRAY_SIZE(preview_items));
    egui_view_menu_button_set_compact_mode(EGUI_VIEW_OF(&preview_menu), 1);
    egui_view_menu_button_override_static_preview_api(EGUI_VIEW_OF(&preview_menu), &preview_api);
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

static void layout_menu(void)
{
    layout_view(EGUI_VIEW_OF(&test_menu), 10, 20, 206, 104);
}

static void layout_preview_menu(void)
{
    layout_view(EGUI_VIEW_OF(&preview_menu), 12, 18, 104, 42);
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

static int send_key(uint8_t key_code)
{
    int handled = 0;

    handled |= send_key_to_view(EGUI_VIEW_OF(&test_menu), EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= send_key_to_view(EGUI_VIEW_OF(&test_menu), EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static int send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&test_menu), type, x, y);
}

static uint8_t get_trigger_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!egui_view_menu_button_get_trigger_region(view, &region))
    {
        return 0;
    }
    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
    return 1;
}

static uint8_t get_item_center(egui_view_t *view, uint8_t index, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!egui_view_menu_button_get_item_region(view, index, &region))
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

static void seed_active_state(egui_view_menu_button_t *menu, uint8_t target, uint8_t visual_pressed)
{
    menu->active_target = target;
    egui_view_set_pressed(EGUI_VIEW_OF(menu), visual_pressed ? 1 : 0);
}

static void assert_active_cleared(egui_view_menu_button_t *menu)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MENU_BUTTON_INDEX_NONE, menu->active_target);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(menu)->is_pressed);
}

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void capture_preview_snapshot(menu_button_preview_snapshot_t *snapshot)
{
    uint8_t index;

    snapshot->region_screen = EGUI_VIEW_OF(&preview_menu)->region_screen;
    for (index = 0; index < EGUI_VIEW_MENU_BUTTON_MAX_ITEMS; ++index)
    {
        snapshot->items[index] = preview_menu.items[index];
    }
    snapshot->button_label = preview_menu.button_label;
    snapshot->button_icon = preview_menu.button_icon;
    snapshot->menu_title = preview_menu.menu_title;
    snapshot->label_font = preview_menu.label_font;
    snapshot->meta_font = preview_menu.meta_font;
    snapshot->icon_font = preview_menu.icon_font;
    snapshot->api = EGUI_VIEW_OF(&preview_menu)->api;
    snapshot->surface_color = preview_menu.surface_color;
    snapshot->menu_color = preview_menu.menu_color;
    snapshot->border_color = preview_menu.border_color;
    snapshot->text_color = preview_menu.text_color;
    snapshot->muted_text_color = preview_menu.muted_text_color;
    snapshot->accent_color = preview_menu.accent_color;
    snapshot->success_color = preview_menu.success_color;
    snapshot->warning_color = preview_menu.warning_color;
    snapshot->danger_color = preview_menu.danger_color;
    snapshot->neutral_color = preview_menu.neutral_color;
    snapshot->item_count = preview_menu.item_count;
    snapshot->selected_index = preview_menu.selected_index;
    snapshot->focus_index = preview_menu.focus_index;
    snapshot->active_target = preview_menu.active_target;
    snapshot->is_open = preview_menu.is_open;
    snapshot->compact_mode = preview_menu.compact_mode;
    snapshot->read_only_mode = preview_menu.read_only_mode;
    snapshot->alpha = EGUI_VIEW_OF(&preview_menu)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_menu));
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_menu)->is_pressed;
    snapshot->is_focused = EGUI_VIEW_OF(&preview_menu)->is_focused;
}

static void assert_preview_state_unchanged(const menu_button_preview_snapshot_t *snapshot)
{
    uint8_t index;

    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_menu)->region_screen);
    for (index = 0; index < EGUI_VIEW_MENU_BUTTON_MAX_ITEMS; ++index)
    {
        EGUI_TEST_ASSERT_TRUE(snapshot->items[index].label == preview_menu.items[index].label);
        EGUI_TEST_ASSERT_TRUE(snapshot->items[index].icon == preview_menu.items[index].icon);
        EGUI_TEST_ASSERT_TRUE(snapshot->items[index].shortcut == preview_menu.items[index].shortcut);
        EGUI_TEST_ASSERT_EQUAL_INT(snapshot->items[index].tone, preview_menu.items[index].tone);
        EGUI_TEST_ASSERT_EQUAL_INT(snapshot->items[index].checked, preview_menu.items[index].checked);
        EGUI_TEST_ASSERT_EQUAL_INT(snapshot->items[index].disabled, preview_menu.items[index].disabled);
    }
    EGUI_TEST_ASSERT_TRUE(snapshot->button_label == preview_menu.button_label);
    EGUI_TEST_ASSERT_TRUE(snapshot->button_icon == preview_menu.button_icon);
    EGUI_TEST_ASSERT_TRUE(snapshot->menu_title == preview_menu.menu_title);
    EGUI_TEST_ASSERT_TRUE(snapshot->label_font == preview_menu.label_font);
    EGUI_TEST_ASSERT_TRUE(snapshot->meta_font == preview_menu.meta_font);
    EGUI_TEST_ASSERT_TRUE(snapshot->icon_font == preview_menu.icon_font);
    EGUI_TEST_ASSERT_TRUE(snapshot->api == EGUI_VIEW_OF(&preview_menu)->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_menu.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->menu_color.full, preview_menu.menu_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_menu.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_menu.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_menu.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_menu.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->success_color.full, preview_menu.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->warning_color.full, preview_menu.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->danger_color.full, preview_menu.danger_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->neutral_color.full, preview_menu.neutral_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->item_count, preview_menu.item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->selected_index, preview_menu.selected_index);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->focus_index, preview_menu.focus_index);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->active_target, preview_menu.active_target);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_open, preview_menu.is_open);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_menu.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_menu.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_menu)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_menu)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_menu)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_menu)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
}

static void test_menu_button_setters_clear_active_and_normalize(void)
{
    setup_menu();
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_button_get_selected_index(EGUI_VIEW_OF(&test_menu)));
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_SURFACE.full, test_menu.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_SURFACE_SUBTLE.full, test_menu.menu_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_BORDER_STRONG.full, test_menu.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_TEXT_STRONG.full, test_menu.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_TEXT_SOFT.full, test_menu.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY_DARK.full, test_menu.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_TEXT_SOFT.full, test_menu.neutral_color.full);

    seed_active_state(&test_menu, EGUI_VIEW_MENU_BUTTON_TARGET_TRIGGER, 1);
    egui_view_menu_button_set_button(EGUI_VIEW_OF(&test_menu), "More actions", EGUI_ICON_MS_ADD);
    EGUI_TEST_ASSERT_TRUE(test_menu.button_label == (const char *)"More actions");
    assert_active_cleared(&test_menu);

    seed_active_state(&test_menu, EGUI_VIEW_MENU_BUTTON_TARGET_TRIGGER, 1);
    egui_view_menu_button_set_menu_title(EGUI_VIEW_OF(&test_menu), "Commands");
    EGUI_TEST_ASSERT_TRUE(test_menu.menu_title == (const char *)"Commands");
    assert_active_cleared(&test_menu);

    seed_active_state(&test_menu, EGUI_VIEW_MENU_BUTTON_TARGET_TRIGGER, 1);
    egui_view_menu_button_set_items(EGUI_VIEW_OF(&test_menu), disabled_items, (uint8_t)EGUI_ARRAY_SIZE(disabled_items));
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_menu.selected_index);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_menu.focus_index);
    EGUI_TEST_ASSERT_EQUAL_INT(3, test_menu.item_count);
    assert_active_cleared(&test_menu);

    seed_active_state(&test_menu, EGUI_VIEW_MENU_BUTTON_TARGET_TRIGGER, 1);
    egui_view_menu_button_set_selected_index(EGUI_VIEW_OF(&test_menu), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_menu.selected_index);
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_menu.focus_index);
    assert_active_cleared(&test_menu);

    seed_active_state(&test_menu, EGUI_VIEW_MENU_BUTTON_TARGET_TRIGGER, 1);
    egui_view_menu_button_set_fonts(EGUI_VIEW_OF(&test_menu), (const egui_font_t *)&egui_res_font_montserrat_8_4,
                                    (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_FONT_ICON_MS_20);
    EGUI_TEST_ASSERT_TRUE(test_menu.icon_font == EGUI_FONT_ICON_MS_20);
    assert_active_cleared(&test_menu);

    seed_active_state(&test_menu, EGUI_VIEW_MENU_BUTTON_TARGET_TRIGGER, 1);
    egui_view_menu_button_set_palette(EGUI_VIEW_OF(&test_menu), HCW_COLOR_SURFACE, EGUI_COLOR_HEX(0xF8FAFC),
                                      EGUI_COLOR_HEX(0x010203), EGUI_COLOR_HEX(0x111213), EGUI_COLOR_HEX(0x212223),
                                      EGUI_COLOR_HEX(0x313233), EGUI_COLOR_HEX(0x414243), EGUI_COLOR_HEX(0x515253),
                                      EGUI_COLOR_HEX(0x616263), EGUI_COLOR_HEX(0x717273));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x010203).full, test_menu.border_color.full);
    assert_active_cleared(&test_menu);

    egui_view_menu_button_set_open(EGUI_VIEW_OF(&test_menu), 1);
    EGUI_TEST_ASSERT_TRUE(egui_view_menu_button_get_open(EGUI_VIEW_OF(&test_menu)));
    egui_view_menu_button_set_compact_mode(EGUI_VIEW_OF(&test_menu), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_menu.compact_mode);
    EGUI_TEST_ASSERT_FALSE(egui_view_menu_button_get_open(EGUI_VIEW_OF(&test_menu)));

    egui_view_menu_button_set_compact_mode(EGUI_VIEW_OF(&test_menu), 0);
    egui_view_menu_button_set_open(EGUI_VIEW_OF(&test_menu), 1);
    egui_view_menu_button_set_read_only_mode(EGUI_VIEW_OF(&test_menu), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_menu.read_only_mode);
    EGUI_TEST_ASSERT_FALSE(egui_view_menu_button_get_open(EGUI_VIEW_OF(&test_menu)));
}

static void test_menu_button_activate_listener_and_disabled_guard(void)
{
    setup_menu();

    EGUI_TEST_ASSERT_TRUE(egui_view_menu_button_activate_item(EGUI_VIEW_OF(&test_menu), 1));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_last_action_index);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_menu.selected_index);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_menu.items[1].checked);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_menu.items[0].checked);

    egui_view_menu_button_set_items(EGUI_VIEW_OF(&test_menu), disabled_items, (uint8_t)EGUI_ARRAY_SIZE(disabled_items));
    reset_action_state();
    EGUI_TEST_ASSERT_FALSE(egui_view_menu_button_activate_item(EGUI_VIEW_OF(&test_menu), 1));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);

    egui_view_menu_button_set_on_action_listener(EGUI_VIEW_OF(&test_menu), NULL);
    EGUI_TEST_ASSERT_TRUE(egui_view_menu_button_activate_item(EGUI_VIEW_OF(&test_menu), 2));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_menu.selected_index);
}

static void test_menu_button_trigger_same_target_release_and_outside_close(void)
{
    egui_dim_t trigger_x;
    egui_dim_t trigger_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_menu();
    layout_menu();
    EGUI_TEST_ASSERT_TRUE(get_trigger_center(EGUI_VIEW_OF(&test_menu), &trigger_x, &trigger_y));
    get_view_outside_point(EGUI_VIEW_OF(&test_menu), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, trigger_x, trigger_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_menu)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_menu)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(test_menu.is_open);
    assert_active_cleared(&test_menu);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, trigger_x, trigger_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, trigger_x, trigger_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, trigger_x, trigger_y));
    EGUI_TEST_ASSERT_TRUE(test_menu.is_open);
    assert_active_cleared(&test_menu);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(test_menu.is_open);
}

static void test_menu_button_item_same_target_release_and_cancel(void)
{
    egui_dim_t item_x;
    egui_dim_t item_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_menu();
    layout_menu();
    egui_view_menu_button_set_open(EGUI_VIEW_OF(&test_menu), 1);
    EGUI_TEST_ASSERT_TRUE(get_item_center(EGUI_VIEW_OF(&test_menu), 1, &item_x, &item_y));
    get_view_outside_point(EGUI_VIEW_OF(&test_menu), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_menu.active_target);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_menu)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    EGUI_TEST_ASSERT_TRUE(test_menu.is_open);
    assert_active_cleared(&test_menu);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, item_x, item_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, item_x, item_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_last_action_index);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_menu.selected_index);
    EGUI_TEST_ASSERT_FALSE(test_menu.is_open);
    assert_active_cleared(&test_menu);

    reset_action_state();
    egui_view_menu_button_set_open(EGUI_VIEW_OF(&test_menu), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, item_x, item_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    assert_active_cleared(&test_menu);
}

static void test_menu_button_keyboard_navigation_and_activate(void)
{
    setup_menu();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(test_menu.is_open);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_menu.focus_index);

    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_menu), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_menu)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_menu), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_last_action_index);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_menu.selected_index);
    EGUI_TEST_ASSERT_FALSE(test_menu.is_open);
    assert_active_cleared(&test_menu);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_TRUE(test_menu.is_open);
    EGUI_TEST_ASSERT_EQUAL_INT(3, test_menu.focus_index);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_FALSE(test_menu.is_open);

    seed_active_state(&test_menu, EGUI_VIEW_MENU_BUTTON_TARGET_TRIGGER, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_TAB));
    assert_active_cleared(&test_menu);
}

static void test_menu_button_read_only_and_disabled_guards(void)
{
    egui_dim_t trigger_x;
    egui_dim_t trigger_y;

    setup_menu();
    layout_menu();
    EGUI_TEST_ASSERT_TRUE(get_trigger_center(EGUI_VIEW_OF(&test_menu), &trigger_x, &trigger_y));

    egui_view_menu_button_set_read_only_mode(EGUI_VIEW_OF(&test_menu), 1);
    seed_active_state(&test_menu, EGUI_VIEW_MENU_BUTTON_TARGET_TRIGGER, 1);
    EGUI_TEST_ASSERT_FALSE(egui_view_menu_button_activate_item(EGUI_VIEW_OF(&test_menu), 1));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, trigger_x, trigger_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(test_menu.is_open);
    assert_active_cleared(&test_menu);

    egui_view_menu_button_set_read_only_mode(EGUI_VIEW_OF(&test_menu), 0);
    egui_view_set_enable(EGUI_VIEW_OF(&test_menu), 0);
    seed_active_state(&test_menu, EGUI_VIEW_MENU_BUTTON_TARGET_TRIGGER, 1);
    EGUI_TEST_ASSERT_FALSE(egui_view_menu_button_activate_item(EGUI_VIEW_OF(&test_menu), 1));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, trigger_x, trigger_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_FALSE(test_menu.is_open);
    assert_active_cleared(&test_menu);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
}

static void test_menu_button_static_preview_consumes_input_and_keeps_state(void)
{
    menu_button_preview_snapshot_t initial_snapshot;
    egui_dim_t trigger_x;
    egui_dim_t trigger_y;

    setup_preview_menu();
    layout_preview_menu();
    EGUI_TEST_ASSERT_TRUE(get_trigger_center(EGUI_VIEW_OF(&preview_menu), &trigger_x, &trigger_y));
    capture_preview_snapshot(&initial_snapshot);

    seed_active_state(&preview_menu, EGUI_VIEW_MENU_BUTTON_TARGET_TRIGGER, 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_menu), EGUI_MOTION_EVENT_ACTION_DOWN, trigger_x, trigger_y));
    assert_preview_state_unchanged(&initial_snapshot);

    seed_active_state(&preview_menu, EGUI_VIEW_MENU_BUTTON_TARGET_TRIGGER, 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_menu), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_menu), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_menu_button_run(void)
{
    EGUI_TEST_SUITE_BEGIN(menu_button);
    EGUI_TEST_RUN(test_menu_button_setters_clear_active_and_normalize);
    EGUI_TEST_RUN(test_menu_button_activate_listener_and_disabled_guard);
    EGUI_TEST_RUN(test_menu_button_trigger_same_target_release_and_outside_close);
    EGUI_TEST_RUN(test_menu_button_item_same_target_release_and_cancel);
    EGUI_TEST_RUN(test_menu_button_keyboard_navigation_and_activate);
    EGUI_TEST_RUN(test_menu_button_read_only_and_disabled_guards);
    EGUI_TEST_RUN(test_menu_button_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
