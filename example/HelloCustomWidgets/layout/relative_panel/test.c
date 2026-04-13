#include "egui.h"
#include "egui_view_relative_panel.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define RELATIVE_PANEL_ROOT_WIDTH        224
#define RELATIVE_PANEL_ROOT_HEIGHT       284
#define RELATIVE_PANEL_PRIMARY_WIDTH     196
#define RELATIVE_PANEL_PRIMARY_HEIGHT    160
#define RELATIVE_PANEL_PREVIEW_WIDTH     104
#define RELATIVE_PANEL_PREVIEW_HEIGHT    88
#define RELATIVE_PANEL_BOTTOM_ROW_WIDTH  216
#define RELATIVE_PANEL_BOTTOM_ROW_HEIGHT 88
#define RELATIVE_PANEL_RECORD_WAIT       90
#define RELATIVE_PANEL_RECORD_FRAME_WAIT 170
#define RELATIVE_PANEL_RECORD_FINAL_WAIT 280

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)(sizeof(primary_snapshots) / sizeof(primary_snapshots[0])))
#define COMPACT_SNAPSHOT_COUNT ((uint8_t)(sizeof(compact_snapshots) / sizeof(compact_snapshots[0])))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_relative_panel_t relative_panel_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_relative_panel_t relative_panel_compact;
static egui_view_relative_panel_t relative_panel_read_only;
static egui_view_api_t relative_panel_compact_api;
static egui_view_api_t relative_panel_read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Relative Panel";

static const egui_view_relative_panel_item_t summary_items[] = {
        {"SUM", "Summary anchor", "Top-left cluster", "Align left + top", 0, 0, 96, 40, EGUI_VIEW_RELATIVE_PANEL_TONE_ACCENT, 1,
         EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE},
        {"STAT", "Right rail", "Right of summary", "Right of Summary", 116, 6, 74, 54, EGUI_VIEW_RELATIVE_PANEL_TONE_SUCCESS, 0, 0},
        {"DET", "Detail block", "Below summary", "Below Summary", 10, 64, 128, 48, EGUI_VIEW_RELATIVE_PANEL_TONE_WARNING, 0, 0},
        {"AUX", "Assist strip", "Align right to detail", "Align right to Detail", 102, 120, 90, 26, EGUI_VIEW_RELATIVE_PANEL_TONE_NEUTRAL, 0, 2},
};

static const egui_view_relative_panel_item_t rail_items[] = {
        {"HUB", "Hero anchor", "Intro block", "Align left + top", 0, 0, 104, 38, EGUI_VIEW_RELATIVE_PANEL_TONE_ACCENT, 1,
         EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE},
        {"OPS", "Status rail", "Right-side stack", "Right of Hero", 126, 8, 66, 48, EGUI_VIEW_RELATIVE_PANEL_TONE_SUCCESS, 0, 0},
        {"DOC", "Detail shelf", "Below the hero", "Below Hero", 18, 62, 118, 50, EGUI_VIEW_RELATIVE_PANEL_TONE_NEUTRAL, 0, 0},
        {"QA", "Review pin", "Attach to rail", "Below Status rail", 142, 82, 50, 40, EGUI_VIEW_RELATIVE_PANEL_TONE_WARNING, 0, 1},
};

static const egui_view_relative_panel_item_t dense_items[] = {
        {"MAP", "Canvas anchor", "Reference card", "Align left + top", 0, 0, 88, 34, EGUI_VIEW_RELATIVE_PANEL_TONE_ACCENT, 1,
         EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE},
        {"REL", "Signal lane", "Right aligned", "Right of Canvas", 104, 0, 88, 38, EGUI_VIEW_RELATIVE_PANEL_TONE_SUCCESS, 0, 0},
        {"ENG", "Bottom detail", "Pinned below", "Below Signal lane", 74, 58, 118, 52, EGUI_VIEW_RELATIVE_PANEL_TONE_WARNING, 0, 1},
        {"PM", "Footer note", "Support card", "Align left with Detail", 0, 118, 98, 28, EGUI_VIEW_RELATIVE_PANEL_TONE_NEUTRAL, 0, 2},
};

static const egui_view_relative_panel_snapshot_t primary_snapshots[] = {
        {"OPS", "Anchored summary", "Cards stay attached through explicit right-of and below rules.", "Rule highlight mirrors the focused card.",
         summary_items, 4, 0, 192, 152},
        {"REL", "Status rail", "The right rail and lower review card share a compact relationship shell.", "Tab to the rule pill, then cycle the preset.",
         rail_items, 4, 1, 192, 152},
        {"QA", "Dense detail", "The lower detail block becomes the active rule while the footer note keeps its anchor.", "Compact preview keeps the same graph.",
         dense_items, 4, 2, 192, 152},
};

static const egui_view_relative_panel_snapshot_t compact_snapshots[] = {
        {"UI", "Compact anchor", "", "Compact preview", summary_items, 4, 0, 192, 152},
        {"QA", "Compact dense", "", "Dense preview", dense_items, 4, 2, 192, 152},
};

static const egui_view_relative_panel_snapshot_t read_only_snapshot = {
        "LOCK", "Read only anchor", "", "Static preview", rail_items, 4, 1, 192, 152};

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_relative_panel_set_current_snapshot(EGUI_VIEW_OF(&relative_panel_primary), index % PRIMARY_SNAPSHOT_COUNT);
}

static void apply_compact_snapshot(uint8_t index)
{
    egui_view_relative_panel_set_current_snapshot(EGUI_VIEW_OF(&relative_panel_compact), index % COMPACT_SNAPSHOT_COUNT);
}

static void dismiss_primary_relative_panel_focus(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_clear_focus(EGUI_VIEW_OF(&relative_panel_primary));
#endif
}

static int dismiss_primary_focus_on_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

    if (event->type == EGUI_MOTION_EVENT_ACTION_DOWN)
    {
        dismiss_primary_relative_panel_focus();
    }
    return 1;
}

#if EGUI_CONFIG_RECORDING_TEST
static void request_page_snapshot(void)
{
    egui_view_invalidate(EGUI_VIEW_OF(&root_layout));
    recording_request_snapshot();
}

static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event = {0};

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&relative_panel_primary));
#endif
    event.key_code = key_code;
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    egui_view_dispatch_key_event(EGUI_VIEW_OF(&relative_panel_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    egui_view_dispatch_key_event(EGUI_VIEW_OF(&relative_panel_primary), &event);
}

static void set_click_view_center(egui_sim_action_t *p_action, egui_view_t *view, int interval_ms)
{
    p_action->type = EGUI_SIM_ACTION_CLICK;
    p_action->x1 = view->region_screen.location.x + view->region_screen.size.width / 2;
    p_action->y1 = view->region_screen.location.y + view->region_screen.size.height / 2;
    p_action->x2 = 0;
    p_action->y2 = 0;
    p_action->steps = 0;
    p_action->interval_ms = interval_ms;
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), RELATIVE_PANEL_ROOT_WIDTH, RELATIVE_PANEL_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), RELATIVE_PANEL_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_relative_panel_init(EGUI_VIEW_OF(&relative_panel_primary));
    egui_view_set_size(EGUI_VIEW_OF(&relative_panel_primary), RELATIVE_PANEL_PRIMARY_WIDTH, RELATIVE_PANEL_PRIMARY_HEIGHT);
    egui_view_relative_panel_set_snapshots(EGUI_VIEW_OF(&relative_panel_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    egui_view_relative_panel_set_font(EGUI_VIEW_OF(&relative_panel_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_relative_panel_set_meta_font(EGUI_VIEW_OF(&relative_panel_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_relative_panel_set_palette(EGUI_VIEW_OF(&relative_panel_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xF7FAFD),
                                         EGUI_COLOR_HEX(0xD4DDE6), EGUI_COLOR_HEX(0x1B2834), EGUI_COLOR_HEX(0x6D7C8A), EGUI_COLOR_HEX(0x0F6CBD),
                                         EGUI_COLOR_HEX(0x6AA8FF));
    egui_view_set_margin(EGUI_VIEW_OF(&relative_panel_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&relative_panel_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), RELATIVE_PANEL_BOTTOM_ROW_WIDTH, RELATIVE_PANEL_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_relative_panel_init(EGUI_VIEW_OF(&relative_panel_compact));
    egui_view_set_size(EGUI_VIEW_OF(&relative_panel_compact), RELATIVE_PANEL_PREVIEW_WIDTH, RELATIVE_PANEL_PREVIEW_HEIGHT);
    egui_view_relative_panel_set_snapshots(EGUI_VIEW_OF(&relative_panel_compact), compact_snapshots, COMPACT_SNAPSHOT_COUNT);
    egui_view_relative_panel_set_font(EGUI_VIEW_OF(&relative_panel_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_relative_panel_set_meta_font(EGUI_VIEW_OF(&relative_panel_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_relative_panel_set_compact_mode(EGUI_VIEW_OF(&relative_panel_compact), 1);
    egui_view_relative_panel_set_palette(EGUI_VIEW_OF(&relative_panel_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xF3FBF9),
                                         EGUI_COLOR_HEX(0xCEE2DC), EGUI_COLOR_HEX(0x173330), EGUI_COLOR_HEX(0x5F7B74), EGUI_COLOR_HEX(0x0D9488),
                                         EGUI_COLOR_HEX(0x67D4C6));
    egui_view_relative_panel_override_static_preview_api(EGUI_VIEW_OF(&relative_panel_compact), &relative_panel_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    relative_panel_compact_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&relative_panel_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&relative_panel_compact));

    egui_view_relative_panel_init(EGUI_VIEW_OF(&relative_panel_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&relative_panel_read_only), RELATIVE_PANEL_PREVIEW_WIDTH, RELATIVE_PANEL_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&relative_panel_read_only), 8, 0, 0, 0);
    egui_view_relative_panel_set_snapshots(EGUI_VIEW_OF(&relative_panel_read_only), &read_only_snapshot, 1);
    egui_view_relative_panel_set_font(EGUI_VIEW_OF(&relative_panel_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_relative_panel_set_meta_font(EGUI_VIEW_OF(&relative_panel_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_relative_panel_set_compact_mode(EGUI_VIEW_OF(&relative_panel_read_only), 1);
    egui_view_relative_panel_set_read_only_mode(EGUI_VIEW_OF(&relative_panel_read_only), 1);
    egui_view_relative_panel_set_palette(EGUI_VIEW_OF(&relative_panel_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xF7F9FB),
                                         EGUI_COLOR_HEX(0xD9E1E8), EGUI_COLOR_HEX(0x556575), EGUI_COLOR_HEX(0x8997A4), EGUI_COLOR_HEX(0xA3B2BE),
                                         EGUI_COLOR_HEX(0xC4D2DE));
    egui_view_relative_panel_override_static_preview_api(EGUI_VIEW_OF(&relative_panel_read_only), &relative_panel_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    relative_panel_read_only_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&relative_panel_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&relative_panel_read_only));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&relative_panel_primary));
#endif
}

#if EGUI_CONFIG_RECORDING_TEST
bool egui_port_get_recording_action(int action_index, egui_sim_action_t *p_action)
{
    static int last_action = -1;
    int first_call = action_index != last_action;

    last_action = action_index;

    switch (action_index)
    {
    case 0:
        if (first_call)
        {
            apply_primary_snapshot(0);
            apply_compact_snapshot(0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&relative_panel_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, RELATIVE_PANEL_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RELATIVE_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_RIGHT);
        }
        EGUI_SIM_SET_WAIT(p_action, RELATIVE_PANEL_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RELATIVE_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_DOWN);
        }
        EGUI_SIM_SET_WAIT(p_action, RELATIVE_PANEL_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RELATIVE_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_TAB);
            apply_primary_key(EGUI_KEY_CODE_ENTER);
            apply_compact_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, RELATIVE_PANEL_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RELATIVE_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 8:
        set_click_view_center(p_action, EGUI_VIEW_OF(&relative_panel_compact), RELATIVE_PANEL_RECORD_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RELATIVE_PANEL_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
