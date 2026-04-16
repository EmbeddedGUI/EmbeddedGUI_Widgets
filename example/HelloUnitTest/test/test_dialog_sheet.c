#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_dialog_sheet.h"

#include "../../HelloCustomWidgets/feedback/dialog_sheet/egui_view_dialog_sheet.h"
#include "../../HelloCustomWidgets/feedback/dialog_sheet/egui_view_dialog_sheet.c"

typedef struct dialog_sheet_preview_snapshot dialog_sheet_preview_snapshot_t;
struct dialog_sheet_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    const egui_view_dialog_sheet_snapshot_t *snapshots;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_dialog_sheet_action_changed_listener_t on_action_changed;
    const egui_view_api_t *api;
    egui_color_t surface_color;
    egui_color_t overlay_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t success_color;
    egui_color_t warning_color;
    egui_color_t error_color;
    egui_color_t neutral_color;
    uint8_t snapshot_count;
    uint8_t current_snapshot;
    uint8_t current_action;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_action;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_focused;
    uint8_t is_pressed;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_dialog_sheet_t test_sheet;
static egui_view_dialog_sheet_t preview_sheet;
static egui_view_api_t preview_api;
static uint8_t changed_count;
static uint8_t last_action;

static const egui_view_dialog_sheet_snapshot_t g_snapshots[] = {
        {"Sync issue", "Reconnect account?", "Resume sync for review.", "Reconnect", "Later", "Sync", "Queue paused", EGUI_VIEW_DIALOG_SHEET_TONE_WARNING, 1, 1,
         EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY},
        {"Delete draft", "Delete unfinished layout?", "Remove local draft.", "Delete", "Cancel", "Draft", "Cannot undo", EGUI_VIEW_DIALOG_SHEET_TONE_ERROR, 1,
         1, EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY},
        {"Template", "Apply starter scene?", "Load base panels.", "Apply", NULL, "Template", "Saved", EGUI_VIEW_DIALOG_SHEET_TONE_ACCENT, 0, 0,
         EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY},
        {"Publishing", "Send build to review?", "Share build now.", "Send", NULL, "Review", "Ready", EGUI_VIEW_DIALOG_SHEET_TONE_SUCCESS, 0, 1,
         EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY},
};

static const egui_view_dialog_sheet_snapshot_t g_overflow_snapshots[] = {
        {"A", "A", "A", "A", "B", "A", "A", EGUI_VIEW_DIALOG_SHEET_TONE_ACCENT, 1, 1, EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY},
        {"B", "B", "B", "B", "C", "B", "B", EGUI_VIEW_DIALOG_SHEET_TONE_SUCCESS, 1, 1, EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY},
        {"C", "C", "C", "C", "D", "C", "C", EGUI_VIEW_DIALOG_SHEET_TONE_WARNING, 1, 1, EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY},
        {"D", "D", "D", "D", "E", "D", "D", EGUI_VIEW_DIALOG_SHEET_TONE_ERROR, 1, 1, EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY},
        {"E", "E", "E", "E", "F", "E", "E", EGUI_VIEW_DIALOG_SHEET_TONE_NEUTRAL, 1, 1, EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY},
        {"F", "F", "F", "F", "G", "F", "F", EGUI_VIEW_DIALOG_SHEET_TONE_ACCENT, 1, 1, EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY},
        {"G", "G", "G", "G", "H", "G", "G", EGUI_VIEW_DIALOG_SHEET_TONE_SUCCESS, 1, 1, EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY},
};

static const egui_view_dialog_sheet_snapshot_t g_secondary_only_snapshot = {
        "Warn",
        "Secondary only",
        "Fallback keeps the right action.",
        NULL,
        "Later",
        "Queue",
        "",
        EGUI_VIEW_DIALOG_SHEET_TONE_WARNING,
        1,
        1,
        EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY,
};

static const egui_view_dialog_sheet_snapshot_t g_no_action_snapshot = {
        "Read", "Read only", "No actions remain.", NULL, NULL, "Read only", "", EGUI_VIEW_DIALOG_SHEET_TONE_NEUTRAL, 0, 0,
        EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY,
};

static const egui_view_dialog_sheet_snapshot_t g_invalid_secondary_snapshot = {
        "Warn",
        "Invalid secondary",
        "Secondary flag without label.",
        "Retry",
        NULL,
        "Sync",
        "",
        EGUI_VIEW_DIALOG_SHEET_TONE_WARNING,
        1,
        1,
        EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY,
};

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void reset_changed_state(void)
{
    changed_count = 0;
    last_action = EGUI_VIEW_DIALOG_SHEET_ACTION_NONE;
}

static void on_action_changed(egui_view_t *self, uint8_t action_index)
{
    EGUI_UNUSED(self);
    changed_count++;
    last_action = action_index;
}

static void setup_sheet(void)
{
    egui_view_dialog_sheet_init(EGUI_VIEW_OF(&test_sheet));
    egui_view_set_size(EGUI_VIEW_OF(&test_sheet), 196, 132);
    egui_view_dialog_sheet_set_snapshots(EGUI_VIEW_OF(&test_sheet), g_snapshots, 4);
    egui_view_dialog_sheet_set_on_action_changed_listener(EGUI_VIEW_OF(&test_sheet), on_action_changed);
    reset_changed_state();
}

static void setup_preview_sheet(void)
{
    egui_view_dialog_sheet_init(EGUI_VIEW_OF(&preview_sheet));
    egui_view_set_size(EGUI_VIEW_OF(&preview_sheet), 104, 86);
    egui_view_dialog_sheet_set_snapshots(EGUI_VIEW_OF(&preview_sheet), g_snapshots, 4);
    egui_view_dialog_sheet_set_current_snapshot(EGUI_VIEW_OF(&preview_sheet), 2);
    egui_view_dialog_sheet_set_compact_mode(EGUI_VIEW_OF(&preview_sheet), 1);
    egui_view_dialog_sheet_set_on_action_changed_listener(EGUI_VIEW_OF(&preview_sheet), on_action_changed);
    egui_view_dialog_sheet_override_static_preview_api(EGUI_VIEW_OF(&preview_sheet), &preview_api);
    reset_changed_state();
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

static void layout_sheet(void)
{
    layout_view(EGUI_VIEW_OF(&test_sheet), 10, 20, 196, 132);
}

static void layout_preview_sheet(void)
{
    layout_view(EGUI_VIEW_OF(&preview_sheet), 12, 18, 104, 86);
}

static int send_touch_to_view(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->dispatch_touch_event(view, &event);
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

static int send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&test_sheet), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_sheet), type, x, y);
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_sheet), key_code);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_sheet), key_code);
}

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void capture_preview_snapshot(dialog_sheet_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_sheet)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_sheet)->background;
    snapshot->snapshots = preview_sheet.snapshots;
    snapshot->font = preview_sheet.font;
    snapshot->meta_font = preview_sheet.meta_font;
    snapshot->on_action_changed = preview_sheet.on_action_changed;
    snapshot->api = EGUI_VIEW_OF(&preview_sheet)->api;
    snapshot->surface_color = preview_sheet.surface_color;
    snapshot->overlay_color = preview_sheet.overlay_color;
    snapshot->border_color = preview_sheet.border_color;
    snapshot->text_color = preview_sheet.text_color;
    snapshot->muted_text_color = preview_sheet.muted_text_color;
    snapshot->accent_color = preview_sheet.accent_color;
    snapshot->success_color = preview_sheet.success_color;
    snapshot->warning_color = preview_sheet.warning_color;
    snapshot->error_color = preview_sheet.error_color;
    snapshot->neutral_color = preview_sheet.neutral_color;
    snapshot->snapshot_count = preview_sheet.snapshot_count;
    snapshot->current_snapshot = preview_sheet.current_snapshot;
    snapshot->current_action = preview_sheet.current_action;
    snapshot->compact_mode = preview_sheet.compact_mode;
    snapshot->read_only_mode = preview_sheet.read_only_mode;
    snapshot->pressed_action = preview_sheet.pressed_action;
    snapshot->alpha = EGUI_VIEW_OF(&preview_sheet)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_sheet));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_sheet)->is_focused;
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_sheet)->is_pressed;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_sheet)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_sheet)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_sheet)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_sheet)->padding.bottom;
}

static void assert_preview_state_unchanged(const dialog_sheet_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_sheet)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_sheet)->background == snapshot->background);
    EGUI_TEST_ASSERT_TRUE(preview_sheet.snapshots == snapshot->snapshots);
    EGUI_TEST_ASSERT_TRUE(preview_sheet.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_sheet.meta_font == snapshot->meta_font);
    EGUI_TEST_ASSERT_TRUE(preview_sheet.on_action_changed == snapshot->on_action_changed);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_sheet)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_sheet.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->overlay_color.full, preview_sheet.overlay_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_sheet.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_sheet.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_sheet.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_sheet.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->success_color.full, preview_sheet.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->warning_color.full, preview_sheet.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->error_color.full, preview_sheet.error_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->neutral_color.full, preview_sheet.neutral_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->snapshot_count, preview_sheet.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_snapshot, preview_sheet.current_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_action, preview_sheet.current_action);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_sheet.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_sheet.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->pressed_action, preview_sheet.pressed_action);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_sheet)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_sheet)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_sheet)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_sheet)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_sheet)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_sheet)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_sheet)->padding.bottom);
}

static void get_metrics(egui_view_dialog_sheet_metrics_t *metrics)
{
    const egui_view_dialog_sheet_snapshot_t *snapshot = egui_view_dialog_sheet_get_snapshot(&test_sheet);
    uint8_t show_secondary;
    uint8_t show_close;

    EGUI_TEST_ASSERT_NOT_NULL(snapshot);
    show_secondary = egui_view_dialog_sheet_has_secondary(snapshot);
    show_close = snapshot->show_close && !test_sheet.compact_mode && !test_sheet.read_only_mode;
    egui_view_dialog_sheet_get_metrics(&test_sheet, EGUI_VIEW_OF(&test_sheet), show_secondary, show_close, metrics);
}

static void seed_pressed_state(egui_view_dialog_sheet_t *sheet, uint8_t action_index)
{
    sheet->pressed_action = action_index;
    egui_view_set_pressed(EGUI_VIEW_OF(sheet), true);
}

static void assert_pressed_cleared(egui_view_dialog_sheet_t *sheet)
{
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(sheet)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_NONE, sheet->pressed_action);
}

static void test_dialog_sheet_set_snapshots_clamp_and_reset_state(void)
{
    setup_sheet();

    egui_view_dialog_sheet_set_snapshots(EGUI_VIEW_OF(&test_sheet), g_overflow_snapshots, 7);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_MAX_SNAPSHOTS, test_sheet.snapshot_count);

    test_sheet.current_snapshot = 5;
    test_sheet.current_action = EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY;
    test_sheet.pressed_action = EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY;
    EGUI_VIEW_OF(&test_sheet)->is_pressed = true;
    egui_view_dialog_sheet_set_snapshots(EGUI_VIEW_OF(&test_sheet), g_snapshots, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_sheet.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_dialog_sheet_get_current_snapshot(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_NONE, test_sheet.pressed_action);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_sheet)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_dialog_sheet_set_snapshots(EGUI_VIEW_OF(&test_sheet), &g_no_action_snapshot, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_NONE, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));

    egui_view_dialog_sheet_set_snapshots(EGUI_VIEW_OF(&test_sheet), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_sheet.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_dialog_sheet_get_current_snapshot(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_NONE, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
}

static void test_dialog_sheet_snapshot_and_action_guards_notify(void)
{
    setup_sheet();

    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_dialog_sheet_get_current_snapshot(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));

    egui_view_dialog_sheet_set_current_action(EGUI_VIEW_OF(&test_sheet), EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY, last_action);

    egui_view_dialog_sheet_set_current_action(EGUI_VIEW_OF(&test_sheet), EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY);
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);

    reset_changed_state();
    test_sheet.pressed_action = EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY;
    EGUI_VIEW_OF(&test_sheet)->is_pressed = true;
    egui_view_dialog_sheet_set_current_snapshot(EGUI_VIEW_OF(&test_sheet), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_dialog_sheet_get_current_snapshot(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_NONE, test_sheet.pressed_action);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_sheet)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    seed_pressed_state(&test_sheet, EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY);
    egui_view_dialog_sheet_set_current_snapshot(EGUI_VIEW_OF(&test_sheet), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_dialog_sheet_get_current_snapshot(EGUI_VIEW_OF(&test_sheet)));
    assert_pressed_cleared(&test_sheet);

    egui_view_dialog_sheet_set_snapshots(EGUI_VIEW_OF(&test_sheet), &g_secondary_only_snapshot, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
    reset_changed_state();
    egui_view_dialog_sheet_set_current_action(EGUI_VIEW_OF(&test_sheet), EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_dialog_sheet_set_snapshots(EGUI_VIEW_OF(&test_sheet), &g_no_action_snapshot, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_NONE, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
    egui_view_dialog_sheet_set_current_action(EGUI_VIEW_OF(&test_sheet), EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_NONE, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
}

static void test_dialog_sheet_font_modes_listener_and_palette(void)
{
    setup_sheet();

    seed_pressed_state(&test_sheet, EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY);
    egui_view_dialog_sheet_set_font(EGUI_VIEW_OF(&test_sheet), NULL);
    assert_pressed_cleared(&test_sheet);

    seed_pressed_state(&test_sheet, EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY);
    egui_view_dialog_sheet_set_meta_font(EGUI_VIEW_OF(&test_sheet), NULL);
    assert_pressed_cleared(&test_sheet);
    EGUI_TEST_ASSERT_TRUE(test_sheet.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_TRUE(test_sheet.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(&test_sheet, EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY);
    egui_view_dialog_sheet_set_compact_mode(EGUI_VIEW_OF(&test_sheet), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_sheet.compact_mode);
    assert_pressed_cleared(&test_sheet);

    seed_pressed_state(&test_sheet, EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY);
    egui_view_dialog_sheet_set_read_only_mode(EGUI_VIEW_OF(&test_sheet), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_sheet.read_only_mode);
    assert_pressed_cleared(&test_sheet);

    egui_view_dialog_sheet_set_read_only_mode(EGUI_VIEW_OF(&test_sheet), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_sheet.read_only_mode);

    seed_pressed_state(&test_sheet, EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY);
    egui_view_dialog_sheet_set_palette(EGUI_VIEW_OF(&test_sheet), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                       EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162), EGUI_COLOR_HEX(0x707172),
                                       EGUI_COLOR_HEX(0x808182), EGUI_COLOR_HEX(0x909192), EGUI_COLOR_HEX(0xA0A1A2));
    assert_pressed_cleared(&test_sheet);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_sheet.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, test_sheet.overlay_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, test_sheet.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x404142).full, test_sheet.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x505152).full, test_sheet.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x606162).full, test_sheet.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x707172).full, test_sheet.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x808182).full, test_sheet.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x909192).full, test_sheet.error_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xA0A1A2).full, test_sheet.neutral_color.full);
}

static void test_dialog_sheet_touch_updates_action_and_hit_testing(void)
{
    egui_view_dialog_sheet_metrics_t metrics;
    egui_dim_t primary_x;
    egui_dim_t primary_y;
    egui_dim_t secondary_x;
    egui_dim_t secondary_y;

    setup_sheet();
    layout_sheet();
    get_metrics(&metrics);

    EGUI_TEST_ASSERT_TRUE(metrics.sheet_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.sheet_region.size.height > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.close_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.primary_action_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.secondary_action_region.size.width > 0);

    primary_x = metrics.primary_action_region.location.x + metrics.primary_action_region.size.width / 2;
    primary_y = metrics.primary_action_region.location.y + metrics.primary_action_region.size.height / 2;
    secondary_x = metrics.secondary_action_region.location.x + metrics.secondary_action_region.size.width / 2;
    secondary_y = metrics.secondary_action_region.location.y + metrics.secondary_action_region.size.height / 2;

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY,
                               egui_view_dialog_sheet_hit_action(&test_sheet, EGUI_VIEW_OF(&test_sheet), primary_x, primary_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY,
                               egui_view_dialog_sheet_hit_action(&test_sheet, EGUI_VIEW_OF(&test_sheet), secondary_x, secondary_y));
    EGUI_TEST_ASSERT_EQUAL_INT(
            EGUI_VIEW_DIALOG_SHEET_ACTION_NONE,
            egui_view_dialog_sheet_hit_action(&test_sheet, EGUI_VIEW_OF(&test_sheet), metrics.sheet_region.location.x, metrics.sheet_region.location.y));

    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, metrics.sheet_region.location.x, metrics.sheet_region.location.y));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, secondary_x, secondary_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_sheet)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY, test_sheet.pressed_action);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, secondary_x, secondary_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_sheet)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_NONE, test_sheet.pressed_action);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY, last_action);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, primary_x, primary_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY, last_action);
}

static void test_dialog_sheet_touch_same_target_release_and_cancel_behavior(void)
{
    egui_view_dialog_sheet_metrics_t metrics;
    egui_dim_t primary_x;
    egui_dim_t primary_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_sheet();
    egui_view_dialog_sheet_set_current_action(EGUI_VIEW_OF(&test_sheet), EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY);
    reset_changed_state();
    layout_sheet();
    get_metrics(&metrics);

    primary_x = metrics.primary_action_region.location.x + metrics.primary_action_region.size.width / 2;
    primary_y = metrics.primary_action_region.location.y + metrics.primary_action_region.size.height / 2;
    outside_x = metrics.sheet_region.location.x;
    outside_y = metrics.sheet_region.location.y;

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_sheet)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_sheet)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    assert_pressed_cleared(&test_sheet);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_sheet)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_sheet)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, primary_x, primary_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY, last_action);
    assert_pressed_cleared(&test_sheet);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, primary_x, primary_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    assert_pressed_cleared(&test_sheet);
}

static void test_dialog_sheet_touch_cancel_clears_pressed_without_selection(void)
{
    egui_view_dialog_sheet_metrics_t metrics;
    egui_dim_t primary_x;
    egui_dim_t primary_y;

    setup_sheet();
    egui_view_dialog_sheet_set_current_action(EGUI_VIEW_OF(&test_sheet), EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY);
    reset_changed_state();
    layout_sheet();
    get_metrics(&metrics);

    primary_x = metrics.primary_action_region.location.x + metrics.primary_action_region.size.width / 2;
    primary_y = metrics.primary_action_region.location.y + metrics.primary_action_region.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_sheet)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY, test_sheet.pressed_action);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, primary_x, primary_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_sheet)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_NONE, test_sheet.pressed_action);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

}

static void test_dialog_sheet_keyboard_navigation_and_guards(void)
{
    setup_sheet();
    reset_changed_state();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY, last_action);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, changed_count);

    egui_view_dialog_sheet_set_current_snapshot(EGUI_VIEW_OF(&test_sheet), 2);
    reset_changed_state();
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

}

static void test_dialog_sheet_read_only_mode_clears_pressed_and_ignores_input(void)
{
    egui_view_dialog_sheet_metrics_t metrics;
    egui_dim_t primary_x;
    egui_dim_t primary_y;

    setup_sheet();
    layout_sheet();
    get_metrics(&metrics);

    primary_x = metrics.primary_action_region.location.x + metrics.primary_action_region.size.width / 2;
    primary_y = metrics.primary_action_region.location.y + metrics.primary_action_region.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_sheet)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY, test_sheet.pressed_action);

    egui_view_dialog_sheet_set_read_only_mode(EGUI_VIEW_OF(&test_sheet), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_sheet.read_only_mode);
    assert_pressed_cleared(&test_sheet);

    seed_pressed_state(&test_sheet, EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    assert_pressed_cleared(&test_sheet);

    seed_pressed_state(&test_sheet, EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_LEFT));
    assert_pressed_cleared(&test_sheet);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_dialog_sheet_set_read_only_mode(EGUI_VIEW_OF(&test_sheet), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_sheet.read_only_mode);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY, egui_view_dialog_sheet_get_current_action(EGUI_VIEW_OF(&test_sheet)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY, last_action);
}

static void test_dialog_sheet_disabled_ignores_input(void)
{
    egui_view_dialog_sheet_metrics_t metrics;
    egui_dim_t primary_x;
    egui_dim_t primary_y;

    setup_sheet();
    layout_sheet();
    get_metrics(&metrics);

    primary_x = metrics.primary_action_region.location.x + metrics.primary_action_region.size.width / 2;
    primary_y = metrics.primary_action_region.location.y + metrics.primary_action_region.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_sheet)->is_pressed);

    egui_view_set_enable(EGUI_VIEW_OF(&test_sheet), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    assert_pressed_cleared(&test_sheet);

    seed_pressed_state(&test_sheet, EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_LEFT));
    assert_pressed_cleared(&test_sheet);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, primary_x, primary_y));
}

static void test_dialog_sheet_static_preview_consumes_input_and_keeps_state(void)
{
    dialog_sheet_preview_snapshot_t initial_snapshot;
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_sheet();
    layout_preview_sheet();
    get_view_center(EGUI_VIEW_OF(&preview_sheet), &x, &y);
    capture_preview_snapshot(&initial_snapshot);

    seed_pressed_state(&preview_sheet, EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    assert_preview_state_unchanged(&initial_snapshot);
    assert_pressed_cleared(&preview_sheet);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    seed_pressed_state(&preview_sheet, EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
    assert_pressed_cleared(&preview_sheet);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
}

static void test_dialog_sheet_internal_helpers_cover_tone_glyph_metrics_and_regions(void)
{
    egui_view_dialog_sheet_metrics_t metrics;
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);
    egui_color_t mixed = egui_view_dialog_sheet_mix_disabled(sample);
    egui_region_t region;

    setup_sheet();
    egui_view_dialog_sheet_set_palette(EGUI_VIEW_OF(&test_sheet), EGUI_COLOR_HEX(0x111111), EGUI_COLOR_HEX(0x222222), EGUI_COLOR_HEX(0x333333),
                                       EGUI_COLOR_HEX(0x444444), EGUI_COLOR_HEX(0x555555), EGUI_COLOR_HEX(0x666666), EGUI_COLOR_HEX(0x777777),
                                       EGUI_COLOR_HEX(0x888888), EGUI_COLOR_HEX(0x999999), EGUI_COLOR_HEX(0xAAAAAA));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_MAX_SNAPSHOTS, egui_view_dialog_sheet_clamp_snapshot_count(9));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_dialog_sheet_text_len(NULL));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_dialog_sheet_text_len("Retry"));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x666666).full, egui_view_dialog_sheet_tone_color(&test_sheet, EGUI_VIEW_DIALOG_SHEET_TONE_ACCENT).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x777777).full, egui_view_dialog_sheet_tone_color(&test_sheet, EGUI_VIEW_DIALOG_SHEET_TONE_SUCCESS).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x888888).full, egui_view_dialog_sheet_tone_color(&test_sheet, EGUI_VIEW_DIALOG_SHEET_TONE_WARNING).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x999999).full, egui_view_dialog_sheet_tone_color(&test_sheet, EGUI_VIEW_DIALOG_SHEET_TONE_ERROR).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xAAAAAA).full, egui_view_dialog_sheet_tone_color(&test_sheet, EGUI_VIEW_DIALOG_SHEET_TONE_NEUTRAL).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x666666).full, egui_view_dialog_sheet_tone_color(&test_sheet, 99).full);
    EGUI_TEST_ASSERT_TRUE(strcmp("i", egui_view_dialog_sheet_tone_glyph(EGUI_VIEW_DIALOG_SHEET_TONE_ACCENT)) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp("+", egui_view_dialog_sheet_tone_glyph(EGUI_VIEW_DIALOG_SHEET_TONE_SUCCESS)) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp("!", egui_view_dialog_sheet_tone_glyph(EGUI_VIEW_DIALOG_SHEET_TONE_WARNING)) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp("x", egui_view_dialog_sheet_tone_glyph(EGUI_VIEW_DIALOG_SHEET_TONE_ERROR)) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp("o", egui_view_dialog_sheet_tone_glyph(EGUI_VIEW_DIALOG_SHEET_TONE_NEUTRAL)) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp("i", egui_view_dialog_sheet_tone_glyph(99)) == 0);
    EGUI_TEST_ASSERT_TRUE(egui_view_dialog_sheet_get_snapshot(&test_sheet) == &g_snapshots[0]);
    test_sheet.current_snapshot = 9;
    EGUI_TEST_ASSERT_TRUE(egui_view_dialog_sheet_get_snapshot(&test_sheet) == NULL);
    test_sheet.current_snapshot = 0;
    EGUI_TEST_ASSERT_TRUE(egui_view_dialog_sheet_has_primary(&g_snapshots[0]));
    EGUI_TEST_ASSERT_TRUE(egui_view_dialog_sheet_has_secondary(&g_snapshots[0]));
    EGUI_TEST_ASSERT_FALSE(egui_view_dialog_sheet_has_secondary(&g_invalid_secondary_snapshot));
    EGUI_TEST_ASSERT_FALSE(egui_view_dialog_sheet_has_primary(&g_no_action_snapshot));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY,
                               egui_view_dialog_sheet_normalize_action(&g_snapshots[1], EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY,
                               egui_view_dialog_sheet_normalize_action(&g_invalid_secondary_snapshot, EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY,
                               egui_view_dialog_sheet_normalize_action(&g_secondary_only_snapshot, EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DIALOG_SHEET_ACTION_NONE,
                               egui_view_dialog_sheet_normalize_action(&g_no_action_snapshot, EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_dialog_sheet_pill_width(NULL, 0, 40));
    EGUI_TEST_ASSERT_EQUAL_INT(28, egui_view_dialog_sheet_pill_width("Go", 0, 40));
    EGUI_TEST_ASSERT_EQUAL_INT(24, egui_view_dialog_sheet_pill_width("Long", 1, 24));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_dialog_sheet_button_width(NULL, 0, 40));
    EGUI_TEST_ASSERT_EQUAL_INT(34, egui_view_dialog_sheet_button_width("Go", 0, 40));
    EGUI_TEST_ASSERT_EQUAL_INT(24, egui_view_dialog_sheet_button_width("Long", 1, 24));
    egui_view_dialog_sheet_zero_region(&region);
    EGUI_TEST_ASSERT_EQUAL_INT(0, region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(0, region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(0, region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(0, region.size.height);
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_DARK_GREY, 68).full, mixed.full);

    layout_sheet();
    get_metrics(&metrics);
    EGUI_TEST_ASSERT_TRUE(metrics.sheet_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.sheet_region.size.height > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.close_region.size.width > 0);

    egui_view_dialog_sheet_set_current_snapshot(EGUI_VIEW_OF(&test_sheet), 2);
    get_metrics(&metrics);
    EGUI_TEST_ASSERT_TRUE(metrics.footer_text_region.size.width > 0);

    egui_view_dialog_sheet_set_compact_mode(EGUI_VIEW_OF(&test_sheet), 1);
    get_metrics(&metrics);
    EGUI_TEST_ASSERT_EQUAL_INT(0, metrics.close_region.size.width);

    egui_view_dialog_sheet_set_compact_mode(EGUI_VIEW_OF(&test_sheet), 0);
    egui_view_dialog_sheet_set_current_snapshot(EGUI_VIEW_OF(&test_sheet), 3);
    egui_view_dialog_sheet_set_read_only_mode(EGUI_VIEW_OF(&test_sheet), 1);
    get_metrics(&metrics);
    EGUI_TEST_ASSERT_EQUAL_INT(0, metrics.close_region.size.width);
}

void test_dialog_sheet_run(void)
{
    EGUI_TEST_SUITE_BEGIN(dialog_sheet);
    EGUI_TEST_RUN(test_dialog_sheet_set_snapshots_clamp_and_reset_state);
    EGUI_TEST_RUN(test_dialog_sheet_snapshot_and_action_guards_notify);
    EGUI_TEST_RUN(test_dialog_sheet_font_modes_listener_and_palette);
    EGUI_TEST_RUN(test_dialog_sheet_touch_updates_action_and_hit_testing);
    EGUI_TEST_RUN(test_dialog_sheet_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_dialog_sheet_touch_cancel_clears_pressed_without_selection);
    EGUI_TEST_RUN(test_dialog_sheet_keyboard_navigation_and_guards);
    EGUI_TEST_RUN(test_dialog_sheet_read_only_mode_clears_pressed_and_ignores_input);
    EGUI_TEST_RUN(test_dialog_sheet_disabled_ignores_input);
    EGUI_TEST_RUN(test_dialog_sheet_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_RUN(test_dialog_sheet_internal_helpers_cover_tone_glyph_metrics_and_regions);
    EGUI_TEST_SUITE_END();
}
