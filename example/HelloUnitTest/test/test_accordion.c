#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_accordion.h"

#include "../../HelloCustomWidgets/layout/accordion/egui_view_accordion.h"
#include "../../HelloCustomWidgets/layout/accordion/egui_view_accordion.c"

typedef struct accordion_preview_snapshot accordion_preview_snapshot_t;
struct accordion_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    const egui_view_accordion_item_t *items;
    egui_view_accordion_action_listener_t on_action;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    const egui_view_api_t *api;
    egui_color_t surface_color;
    egui_color_t section_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t success_color;
    egui_color_t warning_color;
    egui_color_t neutral_color;
    uint8_t item_count;
    uint8_t expanded_index;
    uint8_t focused_index;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_index;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_focused;
    uint8_t is_pressed;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_accordion_t test_widget;
static egui_view_accordion_t preview_widget;
static egui_view_api_t preview_api;
static uint8_t g_action_count;
static uint8_t g_action_index;
static uint8_t g_action_expanded;

static const egui_view_accordion_item_t g_items[] = {
        {"Workspace", "Review policy and sync cadence.", "Owners approve sync changes here.", "WF", EGUI_VIEW_ACCORDION_TONE_ACCENT, 1},
        {"Identity", "Verify access before publish.", "Two reviewers must confirm identity.", "ID", EGUI_VIEW_ACCORDION_TONE_SUCCESS, 0},
        {"Release", "Stage rollout and rollback notes.", "Pilot rollout waits for sign-off.", "UP", EGUI_VIEW_ACCORDION_TONE_WARNING, 0},
};

static const egui_view_accordion_item_t g_overflow_items[] = {
        {"A", "A", "A", "A", EGUI_VIEW_ACCORDION_TONE_ACCENT, 0},
        {"B", "B", "B", "B", EGUI_VIEW_ACCORDION_TONE_SUCCESS, 1},
        {"C", "C", "C", "C", EGUI_VIEW_ACCORDION_TONE_WARNING, 0},
        {"D", "D", "D", "D", EGUI_VIEW_ACCORDION_TONE_NEUTRAL, 0},
        {"E", "E", "E", "E", EGUI_VIEW_ACCORDION_TONE_ACCENT, 0},
        {"F", "F", "F", "F", EGUI_VIEW_ACCORDION_TONE_SUCCESS, 0},
};

static const egui_view_accordion_item_t g_preview_items[] = {
        {"Sync", "", "Compact detail.", "S", EGUI_VIEW_ACCORDION_TONE_ACCENT, 1},
        {"Audit", "", "Hidden detail.", "A", EGUI_VIEW_ACCORDION_TONE_NEUTRAL, 0},
};

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void on_action(egui_view_t *self, uint8_t item_index, uint8_t expanded)
{
    EGUI_UNUSED(self);
    g_action_count++;
    g_action_index = item_index;
    g_action_expanded = expanded;
}

static void reset_action_state(void)
{
    g_action_count = 0;
    g_action_index = EGUI_VIEW_ACCORDION_INDEX_NONE;
    g_action_expanded = 0xFF;
}

static void setup_widget(const egui_view_accordion_item_t *items, uint8_t item_count)
{
    egui_view_accordion_init(EGUI_VIEW_OF(&test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_widget), 196, 142);
    egui_view_accordion_set_items(EGUI_VIEW_OF(&test_widget), items, item_count);
    egui_view_accordion_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    egui_view_accordion_set_meta_font(EGUI_VIEW_OF(&test_widget), NULL);
    egui_view_accordion_set_on_action_listener(EGUI_VIEW_OF(&test_widget), on_action);
    reset_action_state();
}

static void setup_preview_widget(void)
{
    egui_view_accordion_init(EGUI_VIEW_OF(&preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_widget), 104, 76);
    egui_view_accordion_set_items(EGUI_VIEW_OF(&preview_widget), g_preview_items, (uint8_t)(sizeof(g_preview_items) / sizeof(g_preview_items[0])));
    egui_view_accordion_set_font(EGUI_VIEW_OF(&preview_widget), NULL);
    egui_view_accordion_set_meta_font(EGUI_VIEW_OF(&preview_widget), NULL);
    egui_view_accordion_set_compact_mode(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_accordion_override_static_preview_api(EGUI_VIEW_OF(&preview_widget), &preview_api);
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

static void layout_widget(void)
{
    layout_view(EGUI_VIEW_OF(&test_widget), 10, 20, 196, 142);
}

static void layout_preview_widget(void)
{
    layout_view(EGUI_VIEW_OF(&preview_widget), 12, 18, 104, 76);
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
    return send_touch_to_view(EGUI_VIEW_OF(&test_widget), type, x, y);
}

static int send_key_action(uint8_t type, uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_widget), type, key_code);
}

static int send_key(uint8_t key_code)
{
    int handled = 0;

    handled |= send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= send_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static uint8_t get_item_center(egui_view_t *view, uint8_t item_index, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!egui_view_accordion_get_item_region(view, item_index, &region))
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

static void seed_pressed_state(egui_view_accordion_t *widget, uint8_t item_index, uint8_t visual_pressed)
{
    widget->pressed_index = item_index;
    egui_view_set_pressed(EGUI_VIEW_OF(widget), visual_pressed ? 1 : 0);
}

static void assert_pressed_cleared(egui_view_accordion_t *widget)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ACCORDION_INDEX_NONE, widget->pressed_index);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
}

static void capture_preview_snapshot(accordion_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_widget)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_widget)->background;
    snapshot->items = preview_widget.items;
    snapshot->on_action = preview_widget.on_action;
    snapshot->font = preview_widget.font;
    snapshot->meta_font = preview_widget.meta_font;
    snapshot->api = EGUI_VIEW_OF(&preview_widget)->api;
    snapshot->surface_color = preview_widget.surface_color;
    snapshot->section_color = preview_widget.section_color;
    snapshot->border_color = preview_widget.border_color;
    snapshot->text_color = preview_widget.text_color;
    snapshot->muted_text_color = preview_widget.muted_text_color;
    snapshot->accent_color = preview_widget.accent_color;
    snapshot->success_color = preview_widget.success_color;
    snapshot->warning_color = preview_widget.warning_color;
    snapshot->neutral_color = preview_widget.neutral_color;
    snapshot->item_count = preview_widget.item_count;
    snapshot->expanded_index = preview_widget.expanded_index;
    snapshot->focused_index = preview_widget.focused_index;
    snapshot->compact_mode = preview_widget.compact_mode;
    snapshot->read_only_mode = preview_widget.read_only_mode;
    snapshot->pressed_index = preview_widget.pressed_index;
    snapshot->alpha = EGUI_VIEW_OF(&preview_widget)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_widget));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_widget)->is_focused;
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_widget)->is_pressed;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_widget)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_widget)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_widget)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_widget)->padding.bottom;
}

static void assert_preview_state_unchanged(const accordion_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_widget)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_widget)->background == snapshot->background);
    EGUI_TEST_ASSERT_TRUE(preview_widget.items == snapshot->items);
    EGUI_TEST_ASSERT_TRUE(preview_widget.on_action == snapshot->on_action);
    EGUI_TEST_ASSERT_TRUE(preview_widget.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_widget.meta_font == snapshot->meta_font);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_widget)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->section_color.full, preview_widget.section_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_widget.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_widget.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_widget.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_widget.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->success_color.full, preview_widget.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->warning_color.full, preview_widget.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->neutral_color.full, preview_widget.neutral_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->item_count, preview_widget.item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->expanded_index, preview_widget.expanded_index);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->focused_index, preview_widget.focused_index);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->pressed_index, preview_widget.pressed_index);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_widget)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_widget)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_widget)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_widget)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_widget)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_widget)->padding.bottom);
}

static void test_accordion_internal_helpers_cover_text_fitting_and_tones(void)
{
    char label[24];
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);
    egui_color_t mixed = egui_view_accordion_mix_disabled(sample);

    setup_widget(g_items, (uint8_t)(sizeof(g_items) / sizeof(g_items[0])));
    egui_view_accordion_set_palette(EGUI_VIEW_OF(&test_widget), EGUI_COLOR_HEX(0x111111), EGUI_COLOR_HEX(0x222222), EGUI_COLOR_HEX(0x333333),
                                    EGUI_COLOR_HEX(0x444444), EGUI_COLOR_HEX(0x555555), EGUI_COLOR_HEX(0x666666), EGUI_COLOR_HEX(0x777777),
                                    EGUI_COLOR_HEX(0x888888), EGUI_COLOR_HEX(0x999999));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ACCORDION_MAX_ITEMS, egui_view_accordion_clamp_item_count(9));
    EGUI_TEST_ASSERT_FALSE(egui_view_accordion_has_text(NULL));
    EGUI_TEST_ASSERT_TRUE(egui_view_accordion_has_text("Ready"));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_accordion_text_len(NULL));
    EGUI_TEST_ASSERT_EQUAL_INT(6, egui_view_accordion_text_len("Review"));
    EGUI_TEST_ASSERT_TRUE(egui_view_accordion_is_space_char(' '));
    EGUI_TEST_ASSERT_FALSE(egui_view_accordion_is_space_char('x'));
    EGUI_TEST_ASSERT_TRUE(egui_view_accordion_is_break_after_char('-'));
    EGUI_TEST_ASSERT_TRUE(egui_view_accordion_is_break_after_char('/'));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_accordion_find_elide_boundary("Open latest release notes.", 7));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_accordion_find_elide_boundary("scan-first review", 5));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_accordion_measure_text_width(NULL, "Ready"));
    egui_view_accordion_copy_elided(label, sizeof(label), "Documents", 6);
    EGUI_TEST_ASSERT_TRUE(strcmp("Doc...", label) == 0);
    egui_view_accordion_copy_elided(label, sizeof(label), "Open latest release notes.", 8);
    EGUI_TEST_ASSERT_TRUE(strcmp("Open...", label) == 0);
    egui_view_accordion_fit_text_to_width(NULL, "scan-first review", label, sizeof(label), 32, 4);
    EGUI_TEST_ASSERT_TRUE(strcmp("scan-...", label) == 0);
    egui_view_accordion_fit_text_to_width(NULL, "Ready", label, sizeof(label), 20, 4);
    EGUI_TEST_ASSERT_TRUE(strcmp("Ready", label) == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x666666).full, egui_view_accordion_tone_color(&test_widget, EGUI_VIEW_ACCORDION_TONE_ACCENT).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x777777).full, egui_view_accordion_tone_color(&test_widget, EGUI_VIEW_ACCORDION_TONE_SUCCESS).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x888888).full, egui_view_accordion_tone_color(&test_widget, EGUI_VIEW_ACCORDION_TONE_WARNING).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x999999).full, egui_view_accordion_tone_color(&test_widget, EGUI_VIEW_ACCORDION_TONE_NEUTRAL).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x666666).full, egui_view_accordion_tone_color(&test_widget, 99).full);
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(44)).full, mixed.full);
}

static void test_accordion_set_items_clamp_and_defaults(void)
{
    setup_widget(g_overflow_items, (uint8_t)(sizeof(g_overflow_items) / sizeof(g_overflow_items[0])));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ACCORDION_MAX_ITEMS, test_widget.item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_accordion_get_expanded_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_accordion_get_focused_index(EGUI_VIEW_OF(&test_widget)));

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_accordion_set_items(EGUI_VIEW_OF(&test_widget), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ACCORDION_INDEX_NONE, egui_view_accordion_get_expanded_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ACCORDION_INDEX_NONE, egui_view_accordion_get_focused_index(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);
}

static void test_accordion_setters_clear_pressed_and_update_state(void)
{
    setup_widget(g_items, (uint8_t)(sizeof(g_items) / sizeof(g_items[0])));

    seed_pressed_state(&test_widget, 0, 1);
    egui_view_accordion_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 0, 1);
    egui_view_accordion_set_meta_font(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(test_widget.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 0, 1);
    egui_view_accordion_set_compact_mode(EGUI_VIEW_OF(&test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.compact_mode);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 0, 1);
    egui_view_accordion_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.read_only_mode);
    assert_pressed_cleared(&test_widget);
    egui_view_accordion_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 0);

    seed_pressed_state(&test_widget, 0, 1);
    egui_view_accordion_set_palette(EGUI_VIEW_OF(&test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                    EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162), EGUI_COLOR_HEX(0x707172),
                                    EGUI_COLOR_HEX(0x808182), EGUI_COLOR_HEX(0x909192));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, test_widget.section_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, test_widget.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x404142).full, test_widget.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x505152).full, test_widget.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x606162).full, test_widget.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x707172).full, test_widget.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x808182).full, test_widget.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x909192).full, test_widget.neutral_color.full);
    assert_pressed_cleared(&test_widget);

    egui_view_accordion_set_expanded_index(EGUI_VIEW_OF(&test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_accordion_get_expanded_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_accordion_get_focused_index(EGUI_VIEW_OF(&test_widget)));
    egui_view_accordion_set_expanded_index(EGUI_VIEW_OF(&test_widget), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_accordion_get_expanded_index(EGUI_VIEW_OF(&test_widget)));
    egui_view_accordion_set_focused_index(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_accordion_get_focused_index(EGUI_VIEW_OF(&test_widget)));
}

static void test_accordion_activate_listener_and_single_expand(void)
{
    setup_widget(g_items, (uint8_t)(sizeof(g_items) / sizeof(g_items[0])));

    egui_view_accordion_set_focused_index(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_TRUE(egui_view_accordion_activate_focused(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_accordion_get_expanded_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_index);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_expanded);

    EGUI_TEST_ASSERT_TRUE(egui_view_accordion_activate_focused(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ACCORDION_INDEX_NONE, egui_view_accordion_get_expanded_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_expanded);

    reset_action_state();
    egui_view_accordion_set_on_action_listener(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(egui_view_accordion_activate_focused(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
}

static void test_accordion_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t item_x;
    egui_dim_t item_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_widget(g_items, (uint8_t)(sizeof(g_items) / sizeof(g_items[0])));
    layout_widget();
    egui_view_accordion_set_expanded_index(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_ACCORDION_INDEX_NONE);
    EGUI_TEST_ASSERT_TRUE(get_item_center(EGUI_VIEW_OF(&test_widget), 1, &item_x, &item_y));
    get_view_outside_point(EGUI_VIEW_OF(&test_widget), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.pressed_index);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ACCORDION_INDEX_NONE, egui_view_accordion_get_expanded_index(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, item_x, item_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, item_x, item_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_accordion_get_expanded_index(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    reset_action_state();
    EGUI_TEST_ASSERT_TRUE(get_item_center(EGUI_VIEW_OF(&test_widget), 1, &item_x, &item_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, item_x, item_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    assert_pressed_cleared(&test_widget);
}

static void test_accordion_key_navigation_activate_and_escape(void)
{
    setup_widget(g_items, (uint8_t)(sizeof(g_items) / sizeof(g_items[0])));
    egui_view_accordion_set_expanded_index(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_ACCORDION_INDEX_NONE);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_accordion_get_focused_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_UP));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_accordion_get_focused_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_accordion_get_focused_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_accordion_get_focused_index(EGUI_VIEW_OF(&test_widget)));

    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_widget.pressed_index);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_accordion_get_expanded_index(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ACCORDION_INDEX_NONE, egui_view_accordion_get_expanded_index(EGUI_VIEW_OF(&test_widget)));
}

static void test_accordion_read_only_and_disabled_guards(void)
{
    egui_dim_t item_x;
    egui_dim_t item_y;
    uint8_t initial_expanded;
    uint8_t initial_focused;

    setup_widget(g_items, (uint8_t)(sizeof(g_items) / sizeof(g_items[0])));
    layout_widget();
    egui_view_accordion_set_expanded_index(EGUI_VIEW_OF(&test_widget), 2);
    egui_view_accordion_set_focused_index(EGUI_VIEW_OF(&test_widget), 2);
    initial_expanded = egui_view_accordion_get_expanded_index(EGUI_VIEW_OF(&test_widget));
    initial_focused = egui_view_accordion_get_focused_index(EGUI_VIEW_OF(&test_widget));
    EGUI_TEST_ASSERT_TRUE(get_item_center(EGUI_VIEW_OF(&test_widget), 2, &item_x, &item_y));

    seed_pressed_state(&test_widget, 2, 1);
    egui_view_accordion_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 1);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    seed_pressed_state(&test_widget, 2, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(initial_expanded, egui_view_accordion_get_expanded_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_focused, egui_view_accordion_get_focused_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);

    egui_view_accordion_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 0);
    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 0);
    seed_pressed_state(&test_widget, 2, 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    seed_pressed_state(&test_widget, 2, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(initial_expanded, egui_view_accordion_get_expanded_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_focused, egui_view_accordion_get_focused_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
}

static void test_accordion_static_preview_consumes_input_and_keeps_state(void)
{
    accordion_preview_snapshot_t initial_snapshot;
    egui_dim_t item_x;
    egui_dim_t item_y;

    setup_preview_widget();
    layout_preview_widget();
    EGUI_TEST_ASSERT_TRUE(get_item_center(EGUI_VIEW_OF(&preview_widget), 0, &item_x, &item_y));
    capture_preview_snapshot(&initial_snapshot);

    seed_pressed_state(&preview_widget, 0, 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_widget), EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);

    seed_pressed_state(&preview_widget, 0, 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_widget), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_widget), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
}

void test_accordion_run(void)
{
    EGUI_TEST_SUITE_BEGIN(accordion);
    EGUI_TEST_RUN(test_accordion_internal_helpers_cover_text_fitting_and_tones);
    EGUI_TEST_RUN(test_accordion_set_items_clamp_and_defaults);
    EGUI_TEST_RUN(test_accordion_setters_clear_pressed_and_update_state);
    EGUI_TEST_RUN(test_accordion_activate_listener_and_single_expand);
    EGUI_TEST_RUN(test_accordion_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_accordion_key_navigation_activate_and_escape);
    EGUI_TEST_RUN(test_accordion_read_only_and_disabled_guards);
    EGUI_TEST_RUN(test_accordion_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
