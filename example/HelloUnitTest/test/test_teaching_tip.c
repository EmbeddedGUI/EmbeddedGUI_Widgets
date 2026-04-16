#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_teaching_tip.h"

#include "../../HelloCustomWidgets/feedback/teaching_tip/egui_view_teaching_tip.h"
#include "../../HelloCustomWidgets/feedback/teaching_tip/egui_view_teaching_tip.c"

typedef struct teaching_tip_preview_snapshot teaching_tip_preview_snapshot_t;
struct teaching_tip_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    const egui_view_teaching_tip_snapshot_t *snapshots;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_teaching_tip_part_changed_listener_t on_part_changed;
    const egui_view_api_t *api;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t success_color;
    egui_color_t warning_color;
    egui_color_t neutral_color;
    egui_color_t shadow_color;
    uint8_t snapshot_count;
    uint8_t current_snapshot;
    uint8_t current_part;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_part;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_focused;
    uint8_t is_pressed;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_teaching_tip_t test_tip;
static egui_view_teaching_tip_t preview_tip;
static egui_view_api_t preview_api;
static uint8_t changed_count;
static uint8_t last_part;

static const egui_view_teaching_tip_snapshot_t g_snapshots[] = {
        {"Quick filters", "Coachmark", "Pin today view", "Keep ship dates nearby.", "Pin tip", "Later", "Below target", EGUI_VIEW_TEACHING_TIP_TONE_ACCENT,
         EGUI_VIEW_TEACHING_TIP_PLACEMENT_BOTTOM, 1, 1, 1, 1, EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, -18},
        {"Cmd palette", "Shortcut", "Press slash to search", "Jump to commands fast.", "Got it", "Tips", "Placement above target",
         EGUI_VIEW_TEACHING_TIP_TONE_SUCCESS, EGUI_VIEW_TEACHING_TIP_PLACEMENT_TOP, 1, 1, 1, 1, EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, 22},
        {"Quick filters", "", "Tip hidden", "Tap target to reopen", "", "", "", EGUI_VIEW_TEACHING_TIP_TONE_NEUTRAL, EGUI_VIEW_TEACHING_TIP_PLACEMENT_BOTTOM, 0,
         0, 0, 0, EGUI_VIEW_TEACHING_TIP_PART_TARGET, 0},
};

static const egui_view_teaching_tip_snapshot_t g_preview_snapshots[] = {
        {"Quick tip", "Hint", "Pin filters", "Keep nearby.", "Open", "", "", EGUI_VIEW_TEACHING_TIP_TONE_ACCENT, EGUI_VIEW_TEACHING_TIP_PLACEMENT_BOTTOM, 1,
         1, 0, 0, EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, -8},
};

static const egui_view_teaching_tip_snapshot_t g_overflow_snapshots[] = {
        {"A", "A", "A", "A", "A", "B", "A", EGUI_VIEW_TEACHING_TIP_TONE_ACCENT, EGUI_VIEW_TEACHING_TIP_PLACEMENT_BOTTOM, 1, 1, 1, 1,
         EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, 0},
        {"B", "B", "B", "B", "B", "C", "B", EGUI_VIEW_TEACHING_TIP_TONE_SUCCESS, EGUI_VIEW_TEACHING_TIP_PLACEMENT_TOP, 1, 1, 1, 1,
         EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, 0},
        {"C", "C", "C", "C", "C", "", "C", EGUI_VIEW_TEACHING_TIP_TONE_WARNING, EGUI_VIEW_TEACHING_TIP_PLACEMENT_BOTTOM, 1, 1, 0, 1,
         EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, 0},
        {"D", "", "D", "D", "", "", "", EGUI_VIEW_TEACHING_TIP_TONE_NEUTRAL, EGUI_VIEW_TEACHING_TIP_PLACEMENT_BOTTOM, 0, 0, 0, 0,
         EGUI_VIEW_TEACHING_TIP_PART_TARGET, 0},
        {"E", "E", "E", "E", "E", "F", "E", EGUI_VIEW_TEACHING_TIP_TONE_ACCENT, EGUI_VIEW_TEACHING_TIP_PLACEMENT_BOTTOM, 1, 1, 1, 1,
         EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, 0},
        {"F", "F", "F", "F", "F", "G", "F", EGUI_VIEW_TEACHING_TIP_TONE_SUCCESS, EGUI_VIEW_TEACHING_TIP_PLACEMENT_TOP, 1, 1, 1, 1,
         EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, 0},
        {"G", "G", "G", "G", "G", "H", "G", EGUI_VIEW_TEACHING_TIP_TONE_WARNING, EGUI_VIEW_TEACHING_TIP_PLACEMENT_BOTTOM, 1, 1, 1, 1,
         EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, 0},
};

static const egui_view_teaching_tip_snapshot_t g_secondary_only_snapshot = {
        "Warn",
        "Fallback",
        "Secondary only",
        "Fallback keeps the right action.",
        NULL,
        "Later",
        "",
        EGUI_VIEW_TEACHING_TIP_TONE_WARNING,
        EGUI_VIEW_TEACHING_TIP_PLACEMENT_BOTTOM,
        1,
        0,
        1,
        0,
        EGUI_VIEW_TEACHING_TIP_PART_PRIMARY,
        0,
};

static const egui_view_teaching_tip_snapshot_t g_close_only_snapshot = {
        "Hint",
        "Close only",
        "Dismiss helper",
        "Only the close affordance remains.",
        NULL,
        NULL,
        "",
        EGUI_VIEW_TEACHING_TIP_TONE_ACCENT,
        EGUI_VIEW_TEACHING_TIP_PLACEMENT_BOTTOM,
        1,
        0,
        0,
        1,
        EGUI_VIEW_TEACHING_TIP_PART_PRIMARY,
        0,
};

static const egui_view_teaching_tip_snapshot_t g_closed_snapshot = {
        "Quick filters",
        "",
        "Tip hidden",
        "Tap target to reopen",
        "",
        "",
        "",
        EGUI_VIEW_TEACHING_TIP_TONE_NEUTRAL,
        EGUI_VIEW_TEACHING_TIP_PLACEMENT_BOTTOM,
        0,
        0,
        0,
        0,
        EGUI_VIEW_TEACHING_TIP_PART_PRIMARY,
        0,
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
    last_part = EGUI_VIEW_TEACHING_TIP_PART_NONE;
}

static void on_part_changed(egui_view_t *self, uint8_t part)
{
    EGUI_UNUSED(self);
    changed_count++;
    last_part = part;
}

static void setup_tip(void)
{
    egui_view_teaching_tip_init(EGUI_VIEW_OF(&test_tip));
    egui_view_set_size(EGUI_VIEW_OF(&test_tip), 196, 132);
    egui_view_teaching_tip_set_snapshots(EGUI_VIEW_OF(&test_tip), g_snapshots, 3);
    egui_view_teaching_tip_set_on_part_changed_listener(EGUI_VIEW_OF(&test_tip), on_part_changed);
    reset_changed_state();
}

static void setup_preview_tip(void)
{
    egui_view_teaching_tip_init(EGUI_VIEW_OF(&preview_tip));
    egui_view_set_size(EGUI_VIEW_OF(&preview_tip), 104, 80);
    egui_view_teaching_tip_set_snapshots(EGUI_VIEW_OF(&preview_tip), g_preview_snapshots, 1);
    egui_view_teaching_tip_set_current_snapshot(EGUI_VIEW_OF(&preview_tip), 0);
    egui_view_teaching_tip_set_font(EGUI_VIEW_OF(&preview_tip), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_teaching_tip_set_meta_font(EGUI_VIEW_OF(&preview_tip), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_teaching_tip_set_compact_mode(EGUI_VIEW_OF(&preview_tip), 1);
    egui_view_teaching_tip_set_on_part_changed_listener(EGUI_VIEW_OF(&preview_tip), on_part_changed);
    egui_view_teaching_tip_override_static_preview_api(EGUI_VIEW_OF(&preview_tip), &preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&preview_tip), 0);
#endif
    reset_changed_state();
}

static void layout_tip(void)
{
    egui_region_t region;

    region.location.x = 10;
    region.location.y = 20;
    region.size.width = 196;
    region.size.height = 132;
    egui_view_layout(EGUI_VIEW_OF(&test_tip), &region);
    egui_region_copy(&EGUI_VIEW_OF(&test_tip)->region_screen, &region);
}

static void layout_preview_tip(void)
{
    egui_region_t region;

    region.location.x = 24;
    region.location.y = 28;
    region.size.width = 104;
    region.size.height = 80;
    egui_view_layout(EGUI_VIEW_OF(&preview_tip), &region);
    egui_region_copy(&EGUI_VIEW_OF(&preview_tip)->region_screen, &region);
}

static int dispatch_touch_event_to_view(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
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
    return dispatch_touch_event_to_view(EGUI_VIEW_OF(&test_tip), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return dispatch_touch_event_to_view(EGUI_VIEW_OF(&preview_tip), type, x, y);
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_tip), key_code);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_tip), key_code);
}

static void capture_preview_snapshot(teaching_tip_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_tip)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_tip)->background;
    snapshot->snapshots = preview_tip.snapshots;
    snapshot->font = preview_tip.font;
    snapshot->meta_font = preview_tip.meta_font;
    snapshot->on_part_changed = preview_tip.on_part_changed;
    snapshot->api = EGUI_VIEW_OF(&preview_tip)->api;
    snapshot->surface_color = preview_tip.surface_color;
    snapshot->border_color = preview_tip.border_color;
    snapshot->text_color = preview_tip.text_color;
    snapshot->muted_text_color = preview_tip.muted_text_color;
    snapshot->accent_color = preview_tip.accent_color;
    snapshot->success_color = preview_tip.success_color;
    snapshot->warning_color = preview_tip.warning_color;
    snapshot->neutral_color = preview_tip.neutral_color;
    snapshot->shadow_color = preview_tip.shadow_color;
    snapshot->snapshot_count = preview_tip.snapshot_count;
    snapshot->current_snapshot = preview_tip.current_snapshot;
    snapshot->current_part = preview_tip.current_part;
    snapshot->compact_mode = preview_tip.compact_mode;
    snapshot->read_only_mode = preview_tip.read_only_mode;
    snapshot->pressed_part = preview_tip.pressed_part;
    snapshot->alpha = EGUI_VIEW_OF(&preview_tip)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_tip));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_tip)->is_focused;
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_tip)->is_pressed;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_tip)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_tip)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_tip)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_tip)->padding.bottom;
}

static void assert_preview_state_unchanged(const teaching_tip_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_tip)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_tip)->background == snapshot->background);
    EGUI_TEST_ASSERT_TRUE(preview_tip.snapshots == snapshot->snapshots);
    EGUI_TEST_ASSERT_TRUE(preview_tip.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_tip.meta_font == snapshot->meta_font);
    EGUI_TEST_ASSERT_TRUE(preview_tip.on_part_changed == snapshot->on_part_changed);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_tip)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_tip.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_tip.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_tip.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_tip.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_tip.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->success_color.full, preview_tip.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->warning_color.full, preview_tip.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->neutral_color.full, preview_tip.neutral_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->shadow_color.full, preview_tip.shadow_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->snapshot_count, preview_tip.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_snapshot, preview_tip.current_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_part, preview_tip.current_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_tip.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_tip.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->pressed_part, preview_tip.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_tip)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_tip)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_tip)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_tip)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_tip)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_tip)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_tip)->padding.bottom);
}

static void get_metrics(egui_view_teaching_tip_metrics_t *metrics)
{
    const egui_view_teaching_tip_snapshot_t *snapshot = egui_view_teaching_tip_get_snapshot(&test_tip);

    EGUI_TEST_ASSERT_NOT_NULL(snapshot);
    egui_view_teaching_tip_get_metrics(&test_tip, EGUI_VIEW_OF(&test_tip), snapshot, metrics);
}

static void get_region_center(egui_region_t *region, egui_dim_t *x, egui_dim_t *y)
{
    *x = region->location.x + region->size.width / 2;
    *y = region->location.y + region->size.height / 2;
}

static void seed_pressed_state(egui_view_teaching_tip_t *tip, uint8_t part)
{
    tip->pressed_part = part;
    egui_view_set_pressed(EGUI_VIEW_OF(tip), true);
}

static void assert_pressed_cleared(egui_view_teaching_tip_t *tip)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_NONE, tip->pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(tip)->is_pressed);
}

static void test_teaching_tip_set_snapshots_clamp_and_default_part(void)
{
    setup_tip();

    egui_view_teaching_tip_set_snapshots(EGUI_VIEW_OF(&test_tip), g_overflow_snapshots, 7);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_MAX_SNAPSHOTS, test_tip.snapshot_count);

    test_tip.current_snapshot = 5;
    test_tip.current_part = EGUI_VIEW_TEACHING_TIP_PART_CLOSE;
    test_tip.pressed_part = EGUI_VIEW_TEACHING_TIP_PART_PRIMARY;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_tip), true);
    egui_view_teaching_tip_set_snapshots(EGUI_VIEW_OF(&test_tip), g_snapshots, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_tip.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_teaching_tip_get_current_snapshot(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_NONE, test_tip.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tip)->is_pressed);

    egui_view_teaching_tip_set_snapshots(EGUI_VIEW_OF(&test_tip), &g_secondary_only_snapshot, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));

    egui_view_teaching_tip_set_snapshots(EGUI_VIEW_OF(&test_tip), &g_close_only_snapshot, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_CLOSE, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));

    egui_view_teaching_tip_set_snapshots(EGUI_VIEW_OF(&test_tip), &g_closed_snapshot, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_TARGET, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));

    egui_view_teaching_tip_set_snapshots(EGUI_VIEW_OF(&test_tip), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_tip.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_teaching_tip_get_current_snapshot(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_NONE, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
}

static void test_teaching_tip_snapshot_and_part_guards(void)
{
    setup_tip();

    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_teaching_tip_get_current_snapshot(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));

    test_tip.pressed_part = EGUI_VIEW_TEACHING_TIP_PART_CLOSE;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_tip), true);
    egui_view_teaching_tip_set_current_part(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_PRIMARY);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_NONE, test_tip.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tip)->is_pressed);

    egui_view_teaching_tip_set_current_part(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_SECONDARY);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_teaching_tip_set_current_part(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_NONE);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));

    test_tip.pressed_part = EGUI_VIEW_TEACHING_TIP_PART_CLOSE;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_tip), true);
    egui_view_teaching_tip_set_current_snapshot(EGUI_VIEW_OF(&test_tip), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_teaching_tip_get_current_snapshot(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_NONE, test_tip.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tip)->is_pressed);

    test_tip.pressed_part = EGUI_VIEW_TEACHING_TIP_PART_CLOSE;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_tip), true);
    egui_view_teaching_tip_set_current_snapshot(EGUI_VIEW_OF(&test_tip), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_teaching_tip_get_current_snapshot(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_NONE, test_tip.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tip)->is_pressed);

    seed_pressed_state(&test_tip, EGUI_VIEW_TEACHING_TIP_PART_CLOSE);
    egui_view_teaching_tip_set_current_snapshot(EGUI_VIEW_OF(&test_tip), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_teaching_tip_get_current_snapshot(EGUI_VIEW_OF(&test_tip)));
    assert_pressed_cleared(&test_tip);

    egui_view_teaching_tip_set_snapshots(EGUI_VIEW_OF(&test_tip), &g_secondary_only_snapshot, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
    egui_view_teaching_tip_set_current_part(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_PRIMARY);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));

    egui_view_teaching_tip_set_snapshots(EGUI_VIEW_OF(&test_tip), &g_closed_snapshot, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_TARGET, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
    egui_view_teaching_tip_set_current_part(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_PRIMARY);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_TARGET, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
}

static void test_teaching_tip_font_palette_and_internal_helpers(void)
{
    uint8_t parts[4];
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);
    egui_color_t mixed = egui_view_teaching_tip_mix_disabled(sample);

    setup_tip();

    seed_pressed_state(&test_tip, EGUI_VIEW_TEACHING_TIP_PART_PRIMARY);
    egui_view_teaching_tip_set_font(EGUI_VIEW_OF(&test_tip), NULL);
    EGUI_TEST_ASSERT_TRUE(test_tip.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(&test_tip);

    seed_pressed_state(&test_tip, EGUI_VIEW_TEACHING_TIP_PART_PRIMARY);
    egui_view_teaching_tip_set_meta_font(EGUI_VIEW_OF(&test_tip), NULL);
    EGUI_TEST_ASSERT_TRUE(test_tip.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(&test_tip);

    seed_pressed_state(&test_tip, EGUI_VIEW_TEACHING_TIP_PART_PRIMARY);
    egui_view_teaching_tip_set_compact_mode(EGUI_VIEW_OF(&test_tip), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_tip.compact_mode);
    assert_pressed_cleared(&test_tip);
    egui_view_teaching_tip_set_compact_mode(EGUI_VIEW_OF(&test_tip), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_tip.compact_mode);

    seed_pressed_state(&test_tip, EGUI_VIEW_TEACHING_TIP_PART_SECONDARY);
    egui_view_teaching_tip_set_read_only_mode(EGUI_VIEW_OF(&test_tip), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_tip.read_only_mode);
    assert_pressed_cleared(&test_tip);
    egui_view_teaching_tip_set_read_only_mode(EGUI_VIEW_OF(&test_tip), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_tip.read_only_mode);

    seed_pressed_state(&test_tip, EGUI_VIEW_TEACHING_TIP_PART_CLOSE);
    egui_view_teaching_tip_set_palette(EGUI_VIEW_OF(&test_tip), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                       EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162), EGUI_COLOR_HEX(0x707172),
                                       EGUI_COLOR_HEX(0x808182), EGUI_COLOR_HEX(0x909192));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_tip.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, test_tip.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, test_tip.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x404142).full, test_tip.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x505152).full, test_tip.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x606162).full, test_tip.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x707172).full, test_tip.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x808182).full, test_tip.neutral_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x909192).full, test_tip.shadow_color.full);
    assert_pressed_cleared(&test_tip);

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_MAX_SNAPSHOTS, egui_view_teaching_tip_clamp_snapshot_count(9));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_teaching_tip_text_len(NULL));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_teaching_tip_text_len("Later"));
    EGUI_TEST_ASSERT_FALSE(egui_view_teaching_tip_has_text(NULL));
    EGUI_TEST_ASSERT_FALSE(egui_view_teaching_tip_has_text(""));
    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_has_text("Tip"));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x505152).full, egui_view_teaching_tip_tone_color(&test_tip, EGUI_VIEW_TEACHING_TIP_TONE_ACCENT).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x606162).full, egui_view_teaching_tip_tone_color(&test_tip, EGUI_VIEW_TEACHING_TIP_TONE_SUCCESS).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x707172).full, egui_view_teaching_tip_tone_color(&test_tip, EGUI_VIEW_TEACHING_TIP_TONE_WARNING).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x808182).full, egui_view_teaching_tip_tone_color(&test_tip, EGUI_VIEW_TEACHING_TIP_TONE_NEUTRAL).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x505152).full, egui_view_teaching_tip_tone_color(&test_tip, 99).full);
    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_get_snapshot(&test_tip) == &g_snapshots[0]);
    test_tip.current_snapshot = 9;
    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_get_snapshot(&test_tip) == NULL);
    test_tip.current_snapshot = 0;

    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_part_is_enabled(&test_tip, EGUI_VIEW_OF(&test_tip), &g_snapshots[0], EGUI_VIEW_TEACHING_TIP_PART_TARGET));
    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_part_is_enabled(&test_tip, EGUI_VIEW_OF(&test_tip), &g_snapshots[0], EGUI_VIEW_TEACHING_TIP_PART_PRIMARY));
    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_part_is_enabled(&test_tip, EGUI_VIEW_OF(&test_tip), &g_snapshots[0], EGUI_VIEW_TEACHING_TIP_PART_SECONDARY));
    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_part_is_enabled(&test_tip, EGUI_VIEW_OF(&test_tip), &g_snapshots[0], EGUI_VIEW_TEACHING_TIP_PART_CLOSE));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_SECONDARY,
                               egui_view_teaching_tip_resolve_default_part(&test_tip, EGUI_VIEW_OF(&test_tip), &g_secondary_only_snapshot));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_CLOSE,
                               egui_view_teaching_tip_resolve_default_part(&test_tip, EGUI_VIEW_OF(&test_tip), &g_close_only_snapshot));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_TARGET,
                               egui_view_teaching_tip_resolve_default_part(&test_tip, EGUI_VIEW_OF(&test_tip), &g_closed_snapshot));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_teaching_tip_collect_parts(&test_tip, EGUI_VIEW_OF(&test_tip), &g_snapshots[0], parts, 4));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_TARGET, parts[0]);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, parts[1]);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, parts[2]);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_CLOSE, parts[3]);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_teaching_tip_find_part_index(parts, 4, EGUI_VIEW_TEACHING_TIP_PART_PRIMARY));
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_DARK_GREY, 68).full, mixed.full);

    egui_view_teaching_tip_set_read_only_mode(EGUI_VIEW_OF(&test_tip), 1);
    EGUI_TEST_ASSERT_FALSE(egui_view_teaching_tip_part_is_enabled(&test_tip, EGUI_VIEW_OF(&test_tip), &g_snapshots[0], EGUI_VIEW_TEACHING_TIP_PART_PRIMARY));
    egui_view_teaching_tip_set_read_only_mode(EGUI_VIEW_OF(&test_tip), 0);
    egui_view_set_enable(EGUI_VIEW_OF(&test_tip), 0);
    EGUI_TEST_ASSERT_FALSE(egui_view_teaching_tip_part_is_enabled(&test_tip, EGUI_VIEW_OF(&test_tip), &g_snapshots[0], EGUI_VIEW_TEACHING_TIP_PART_TARGET));
}

static void test_teaching_tip_metrics_and_hit_testing(void)
{
    egui_view_teaching_tip_metrics_t metrics;
    egui_region_t target_region;
    egui_region_t primary_region;
    egui_region_t secondary_region;
    egui_region_t close_region;
    egui_dim_t target_x;
    egui_dim_t target_y;
    egui_dim_t primary_x;
    egui_dim_t primary_y;
    egui_dim_t secondary_x;
    egui_dim_t secondary_y;
    egui_dim_t close_x;
    egui_dim_t close_y;

    setup_tip();
    layout_tip();
    get_metrics(&metrics);

    EGUI_TEST_ASSERT_TRUE(metrics.show_bubble);
    EGUI_TEST_ASSERT_TRUE(metrics.show_primary);
    EGUI_TEST_ASSERT_TRUE(metrics.show_secondary);
    EGUI_TEST_ASSERT_TRUE(metrics.show_close);
    EGUI_TEST_ASSERT_TRUE(metrics.target_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.bubble_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.primary_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.secondary_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.close_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.bubble_region.location.y > metrics.target_region.location.y);

    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_TARGET, &target_region));
    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, &primary_region));
    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, &secondary_region));
    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_CLOSE, &close_region));

    get_region_center(&target_region, &target_x, &target_y);
    get_region_center(&primary_region, &primary_x, &primary_y);
    get_region_center(&secondary_region, &secondary_x, &secondary_y);
    get_region_center(&close_region, &close_x, &close_y);

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_TARGET, egui_view_teaching_tip_hit_part(&test_tip, EGUI_VIEW_OF(&test_tip), target_x, target_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, egui_view_teaching_tip_hit_part(&test_tip, EGUI_VIEW_OF(&test_tip), primary_x, primary_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_SECONDARY,
                               egui_view_teaching_tip_hit_part(&test_tip, EGUI_VIEW_OF(&test_tip), secondary_x, secondary_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_CLOSE, egui_view_teaching_tip_hit_part(&test_tip, EGUI_VIEW_OF(&test_tip), close_x, close_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_NONE,
                               egui_view_teaching_tip_hit_part(&test_tip, EGUI_VIEW_OF(&test_tip), EGUI_VIEW_OF(&test_tip)->region_screen.location.x,
                                                               EGUI_VIEW_OF(&test_tip)->region_screen.location.y));

    egui_view_teaching_tip_set_current_snapshot(EGUI_VIEW_OF(&test_tip), 1);
    get_metrics(&metrics);
    EGUI_TEST_ASSERT_TRUE(metrics.bubble_region.location.y < metrics.target_region.location.y);

    egui_view_teaching_tip_set_compact_mode(EGUI_VIEW_OF(&test_tip), 1);
    get_metrics(&metrics);
    EGUI_TEST_ASSERT_FALSE(metrics.show_secondary);
    EGUI_TEST_ASSERT_FALSE(metrics.show_close);
    EGUI_TEST_ASSERT_FALSE(metrics.show_body);
    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_TARGET, &target_region));
    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, &primary_region));
    EGUI_TEST_ASSERT_FALSE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, &secondary_region));
    EGUI_TEST_ASSERT_FALSE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_CLOSE, &close_region));

    egui_view_teaching_tip_set_compact_mode(EGUI_VIEW_OF(&test_tip), 0);
    egui_view_teaching_tip_set_snapshots(EGUI_VIEW_OF(&test_tip), &g_closed_snapshot, 1);
    get_metrics(&metrics);
    EGUI_TEST_ASSERT_FALSE(metrics.show_bubble);
    EGUI_TEST_ASSERT_TRUE(metrics.show_closed_hint);
    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_TARGET, &target_region));
    EGUI_TEST_ASSERT_FALSE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, &primary_region));
}

static void test_teaching_tip_touch_same_target_release_and_cancel(void)
{
    egui_region_t target_region;
    egui_region_t primary_region;
    egui_region_t secondary_region;
    egui_region_t close_region;
    egui_dim_t target_x;
    egui_dim_t target_y;
    egui_dim_t primary_x;
    egui_dim_t primary_y;
    egui_dim_t secondary_x;
    egui_dim_t secondary_y;
    egui_dim_t close_x;
    egui_dim_t close_y;

    setup_tip();
    layout_tip();

    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_TARGET, &target_region));
    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, &primary_region));
    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, &secondary_region));
    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_CLOSE, &close_region));

    get_region_center(&target_region, &target_x, &target_y);
    get_region_center(&primary_region, &primary_x, &primary_y);
    get_region_center(&secondary_region, &secondary_x, &secondary_y);
    get_region_center(&close_region, &close_x, &close_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, secondary_x, secondary_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tip)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, test_tip.pressed_part);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, target_x, target_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tip)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, target_x, target_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    assert_pressed_cleared(&test_tip);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, secondary_x, secondary_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, target_x, target_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tip)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, secondary_x, secondary_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tip)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, secondary_x, secondary_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, last_part);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, primary_x, primary_y));
    assert_pressed_cleared(&test_tip);
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, close_x, close_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, close_x, close_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_CLOSE, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_CLOSE, last_part);

    egui_view_teaching_tip_set_snapshots(EGUI_VIEW_OF(&test_tip), &g_closed_snapshot, 1);
    layout_tip();
    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_TARGET, &target_region));
    get_region_center(&target_region, &target_x, &target_y);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, target_x, target_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, target_x, target_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_TARGET, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_TARGET, last_part);
}

static void test_teaching_tip_keyboard_navigation_and_guards(void)
{
    setup_tip();
    reset_changed_state();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, last_part);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_CLOSE, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_TARGET, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(5, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_TARGET, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(6, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(7, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_TARGET, last_part);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(8, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_TARGET, last_part);

    egui_view_teaching_tip_set_read_only_mode(EGUI_VIEW_OF(&test_tip), 1);
    reset_changed_state();
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_teaching_tip_set_read_only_mode(EGUI_VIEW_OF(&test_tip), 0);
    egui_view_set_enable(EGUI_VIEW_OF(&test_tip), 0);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_LEFT));
}

static void test_teaching_tip_compact_mode_clears_pressed_and_keeps_input_behavior(void)
{
    egui_region_t target_region;
    egui_region_t secondary_region;
    egui_dim_t target_x;
    egui_dim_t target_y;
    egui_dim_t secondary_x;
    egui_dim_t secondary_y;

    setup_tip();
    layout_tip();

    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, &secondary_region));
    get_region_center(&secondary_region, &secondary_x, &secondary_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, secondary_x, secondary_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tip)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, test_tip.pressed_part);

    egui_view_teaching_tip_set_compact_mode(EGUI_VIEW_OF(&test_tip), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_tip.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_NONE, test_tip.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tip)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_TARGET, &target_region));
    get_region_center(&target_region, &target_x, &target_y);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, target_x, target_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, target_x, target_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_TARGET, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_TARGET, last_part);
}

static void test_teaching_tip_read_only_mode_clears_pressed_and_ignores_input(void)
{
    egui_region_t primary_region;
    egui_dim_t primary_x;
    egui_dim_t primary_y;

    setup_tip();
    layout_tip();

    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, &primary_region));
    get_region_center(&primary_region, &primary_x, &primary_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tip)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, test_tip.pressed_part);

    egui_view_teaching_tip_set_read_only_mode(EGUI_VIEW_OF(&test_tip), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_tip.read_only_mode);
    assert_pressed_cleared(&test_tip);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, primary_x, primary_y));
    seed_pressed_state(&test_tip, EGUI_VIEW_TEACHING_TIP_PART_PRIMARY);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_LEFT));
    assert_pressed_cleared(&test_tip);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_teaching_tip_set_read_only_mode(EGUI_VIEW_OF(&test_tip), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_tip.read_only_mode);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, last_part);
}

static void test_teaching_tip_disabled_ignores_input_and_clears_pressed_state(void)
{
    egui_region_t primary_region;
    egui_region_t target_region;
    egui_dim_t primary_x;
    egui_dim_t primary_y;
    egui_dim_t target_x;
    egui_dim_t target_y;

    setup_tip();
    layout_tip();

    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, &primary_region));
    get_region_center(&primary_region, &primary_x, &primary_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tip)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, test_tip.pressed_part);

    egui_view_set_enable(EGUI_VIEW_OF(&test_tip), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    assert_pressed_cleared(&test_tip);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, primary_x, primary_y));
    seed_pressed_state(&test_tip, EGUI_VIEW_TEACHING_TIP_PART_PRIMARY);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_LEFT));
    assert_pressed_cleared(&test_tip);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_set_enable(EGUI_VIEW_OF(&test_tip), 1);
    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&test_tip), EGUI_VIEW_TEACHING_TIP_PART_TARGET, &target_region));
    get_region_center(&target_region, &target_x, &target_y);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, target_x, target_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, target_x, target_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_TARGET, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&test_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_TARGET, last_part);
}

static void test_teaching_tip_static_preview_consumes_input_and_keeps_state(void)
{
    teaching_tip_preview_snapshot_t initial_snapshot;
    egui_region_t primary_region;
    egui_dim_t primary_x;
    egui_dim_t primary_y;

    setup_preview_tip();
    layout_preview_tip();
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_teaching_tip_get_current_snapshot(EGUI_VIEW_OF(&preview_tip)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, egui_view_teaching_tip_get_current_part(EGUI_VIEW_OF(&preview_tip)));
    capture_preview_snapshot(&initial_snapshot);

    EGUI_TEST_ASSERT_TRUE(egui_view_teaching_tip_get_part_region(EGUI_VIEW_OF(&preview_tip), EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, &primary_region));
    get_region_center(&primary_region, &primary_x, &primary_y);

    seed_pressed_state(&preview_tip, EGUI_VIEW_TEACHING_TIP_PART_PRIMARY);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_UP, primary_x, primary_y));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    seed_pressed_state(&preview_tip, EGUI_VIEW_TEACHING_TIP_PART_PRIMARY);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
}

void test_teaching_tip_run(void)
{
    EGUI_TEST_SUITE_BEGIN(teaching_tip);
    EGUI_TEST_RUN(test_teaching_tip_set_snapshots_clamp_and_default_part);
    EGUI_TEST_RUN(test_teaching_tip_snapshot_and_part_guards);
    EGUI_TEST_RUN(test_teaching_tip_font_palette_and_internal_helpers);
    EGUI_TEST_RUN(test_teaching_tip_metrics_and_hit_testing);
    EGUI_TEST_RUN(test_teaching_tip_touch_same_target_release_and_cancel);
    EGUI_TEST_RUN(test_teaching_tip_compact_mode_clears_pressed_and_keeps_input_behavior);
    EGUI_TEST_RUN(test_teaching_tip_read_only_mode_clears_pressed_and_ignores_input);
    EGUI_TEST_RUN(test_teaching_tip_disabled_ignores_input_and_clears_pressed_state);
    EGUI_TEST_RUN(test_teaching_tip_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_RUN(test_teaching_tip_keyboard_navigation_and_guards);
    EGUI_TEST_SUITE_END();
}
