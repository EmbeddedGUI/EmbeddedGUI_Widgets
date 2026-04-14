#include "egui.h"
#include "egui_view_virtualizing_stack_panel.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define VIRTUALIZING_STACK_PANEL_ROOT_WIDTH        224
#define VIRTUALIZING_STACK_PANEL_ROOT_HEIGHT       232
#define VIRTUALIZING_STACK_PANEL_PRIMARY_WIDTH     196
#define VIRTUALIZING_STACK_PANEL_PRIMARY_HEIGHT    108
#define VIRTUALIZING_STACK_PANEL_PREVIEW_WIDTH     104
#define VIRTUALIZING_STACK_PANEL_PREVIEW_HEIGHT    82
#define VIRTUALIZING_STACK_PANEL_BOTTOM_ROW_WIDTH  216
#define VIRTUALIZING_STACK_PANEL_BOTTOM_ROW_HEIGHT 82
#define VIRTUALIZING_STACK_PANEL_RECORD_WAIT       90
#define VIRTUALIZING_STACK_PANEL_RECORD_FRAME_WAIT 180
#define VIRTUALIZING_STACK_PANEL_RECORD_FINAL_WAIT 280

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)(sizeof(primary_snapshots) / sizeof(primary_snapshots[0])))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_virtualizing_stack_panel_t stack_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_virtualizing_stack_panel_t stack_compact;
static egui_view_virtualizing_stack_panel_t stack_read_only;
static egui_view_api_t stack_compact_api;
static egui_view_api_t stack_read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Virtualizing Stack Panel";

static const egui_view_virtualizing_stack_panel_item_t primary_items_0[] = {
        {"OPS", "Inbox queue", "14 items waiting for owner review", "14", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_ACCENT, 1},
        {"QA", "Audit lane", "Blocked by release checklist", "09", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_WARNING, 0},
        {"REL", "Release gate", "Ready after smoke check", "03", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_SUCCESS, 1},
        {"DOC", "Spec notes", "Author requested one more pass", "07", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_NEUTRAL, 0},
        {"NAV", "Route map", "Pending navigation owner", "05", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_ACCENT, 0},
        {"OBS", "Health watch", "Signals collected for triage", "12", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_SUCCESS, 1},
        {"SEC", "Guard rules", "Two warnings still unresolved", "02", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_WARNING, 0},
        {"APP", "Owner sync", "Cross-team review in progress", "04", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_NEUTRAL, 0},
        {"UX", "Token atlas", "Preview pack generated", "08", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_ACCENT, 1},
        {"IA", "Filter mode", "Selection narrowed to one lane", "11", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_SUCCESS, 0},
        {"CD", "Stage board", "Canary branch still muted", "06", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_WARNING, 1},
        {"CAL", "Order watch", "Window anchor follows focus", "10", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_NEUTRAL, 0},
};

static const egui_view_virtualizing_stack_panel_item_t primary_items_1[] = {
        {"REL", "Review flow", "Anchor follows the active row", "A1", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_ACCENT, 1},
        {"OPS", "Queue depth", "Compact view keeps density", "A2", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_WARNING, 0},
        {"DOC", "Spec mirror", "Documentation copy aligned", "A3", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_NEUTRAL, 0},
        {"UX", "Density mode", "Preview stays static", "A4", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_SUCCESS, 1},
        {"LOG", "Review tape", "Recording shows page jump", "A5", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_ACCENT, 0},
        {"APP", "Owner panel", "Read only keeps muted tone", "A6", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_SUCCESS, 0},
        {"EXP", "Window rail", "Thumb marks hidden rows", "A7", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_WARNING, 1},
        {"QA", "Check stage", "Same-target release only", "A8", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_NEUTRAL, 0},
        {"SEC", "Policy view", "Key focus returns on demand", "A9", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_ACCENT, 0},
        {"REL", "Gate shift", "Move to next snapshot with tab", "B1", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_SUCCESS, 1},
        {"NAV", "Flow track", "Window scroll stays bounded", "B2", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_WARNING, 0},
        {"OBS", "Trace scan", "Visible rows only expose regions", "B3", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_NEUTRAL, 0},
};

static const egui_view_virtualizing_stack_panel_snapshot_t primary_snapshots[] = {
        {"OPS", "Operations queue", "Only the current window is rendered", "Ctrl+Down jumps the window", primary_items_0, 12, 1, 0},
        {"REL", "Release review", "Anchor follows the current row", "12 items / windowed stack", primary_items_1, 12, 4, 2},
};

static const egui_view_virtualizing_stack_panel_item_t compact_items_0[] = {
        {"A", "Ship", "Next release lane", "1", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_ACCENT, 1},
        {"B", "QA", "Audit queue", "2", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_WARNING, 0},
        {"C", "Docs", "Spec mirror", "3", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_NEUTRAL, 0},
        {"D", "Live", "Deploy rail", "4", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_SUCCESS, 0},
        {"E", "Guard", "Muted policy", "5", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_ACCENT, 0},
        {"F", "Flow", "Owner sync", "6", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_SUCCESS, 1},
        {"G", "Owner", "Trace lane", "7", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_WARNING, 0},
        {"H", "Trace", "Read preview", "8", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_NEUTRAL, 0},
};

static const egui_view_virtualizing_stack_panel_snapshot_t compact_snapshot = {
        "OPS", "Compact window", "", "", compact_items_0, 8, 0, 0};

static const egui_view_virtualizing_stack_panel_item_t read_only_items[] = {
        {"L1", "Locked", "Static rows stay muted", "1", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_NEUTRAL, 0},
        {"L2", "Muted", "Read only blocks input", "2", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_WARNING, 1},
        {"L3", "Done", "Focus remains on primary", "3", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_SUCCESS, 0},
        {"L4", "Queue", "Compact shell stays calm", "4", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_ACCENT, 0},
        {"L5", "Flow", "Visible rows only", "5", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_NEUTRAL, 0},
        {"L6", "Anchor", "Thumb hints hidden rows", "6", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_WARNING, 0},
        {"L7", "Thumb", "No action commit", "7", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_SUCCESS, 0},
        {"L8", "Preview", "Static compare", "8", EGUI_VIEW_VIRTUALIZING_STACK_PANEL_TONE_ACCENT, 0},
};

static const egui_view_virtualizing_stack_panel_snapshot_t read_only_snapshot = {
        "LOCK", "Read only window", "", "", read_only_items, 8, 3, 1};

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_virtualizing_stack_panel_set_current_snapshot(EGUI_VIEW_OF(&stack_primary), index % PRIMARY_SNAPSHOT_COUNT);
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(0);
}

static void apply_preview_states(void)
{
    egui_view_virtualizing_stack_panel_set_current_snapshot(EGUI_VIEW_OF(&stack_compact), 0);
    egui_view_virtualizing_stack_panel_set_current_snapshot(EGUI_VIEW_OF(&stack_read_only), 0);
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_primary_key(uint8_t key_code, uint8_t is_ctrl)
{
    egui_key_event_t event = {0};

    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    event.is_ctrl = is_ctrl ? 1 : 0;
    EGUI_VIEW_OF(&stack_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&stack_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&stack_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&stack_primary), &event);
}

static void request_page_snapshot(void)
{
    egui_view_invalidate(EGUI_VIEW_OF(&root_layout));
    recording_request_snapshot();
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), VIRTUALIZING_STACK_PANEL_ROOT_WIDTH, VIRTUALIZING_STACK_PANEL_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), VIRTUALIZING_STACK_PANEL_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_virtualizing_stack_panel_init(EGUI_VIEW_OF(&stack_primary));
    egui_view_set_size(EGUI_VIEW_OF(&stack_primary), VIRTUALIZING_STACK_PANEL_PRIMARY_WIDTH, VIRTUALIZING_STACK_PANEL_PRIMARY_HEIGHT);
    egui_view_virtualizing_stack_panel_set_snapshots(EGUI_VIEW_OF(&stack_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    egui_view_virtualizing_stack_panel_set_font(EGUI_VIEW_OF(&stack_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_virtualizing_stack_panel_set_meta_font(EGUI_VIEW_OF(&stack_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_virtualizing_stack_panel_set_palette(EGUI_VIEW_OF(&stack_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xF4F7FA), EGUI_COLOR_HEX(0xD6DEE6),
                                                   EGUI_COLOR_HEX(0x1B2834), EGUI_COLOR_HEX(0x6D7C8A), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x127A43),
                                                   EGUI_COLOR_HEX(0xA15D00), EGUI_COLOR_HEX(0x7A8796));
    egui_view_set_margin(EGUI_VIEW_OF(&stack_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&stack_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), VIRTUALIZING_STACK_PANEL_BOTTOM_ROW_WIDTH, VIRTUALIZING_STACK_PANEL_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_virtualizing_stack_panel_init(EGUI_VIEW_OF(&stack_compact));
    egui_view_set_size(EGUI_VIEW_OF(&stack_compact), VIRTUALIZING_STACK_PANEL_PREVIEW_WIDTH, VIRTUALIZING_STACK_PANEL_PREVIEW_HEIGHT);
    egui_view_virtualizing_stack_panel_set_snapshots(EGUI_VIEW_OF(&stack_compact), &compact_snapshot, 1);
    egui_view_virtualizing_stack_panel_set_font(EGUI_VIEW_OF(&stack_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_virtualizing_stack_panel_set_meta_font(EGUI_VIEW_OF(&stack_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_virtualizing_stack_panel_set_compact_mode(EGUI_VIEW_OF(&stack_compact), 1);
    egui_view_virtualizing_stack_panel_set_palette(EGUI_VIEW_OF(&stack_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xF3FBF8), EGUI_COLOR_HEX(0xD0E3DE),
                                                   EGUI_COLOR_HEX(0x18312F), EGUI_COLOR_HEX(0x5B7A73), EGUI_COLOR_HEX(0x0D9488), EGUI_COLOR_HEX(0x0B8454),
                                                   EGUI_COLOR_HEX(0x957000), EGUI_COLOR_HEX(0x6D7F8D));
    egui_view_virtualizing_stack_panel_override_static_preview_api(EGUI_VIEW_OF(&stack_compact), &stack_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&stack_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&stack_compact));

    egui_view_virtualizing_stack_panel_init(EGUI_VIEW_OF(&stack_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&stack_read_only), VIRTUALIZING_STACK_PANEL_PREVIEW_WIDTH, VIRTUALIZING_STACK_PANEL_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&stack_read_only), 8, 0, 0, 0);
    egui_view_virtualizing_stack_panel_set_snapshots(EGUI_VIEW_OF(&stack_read_only), &read_only_snapshot, 1);
    egui_view_virtualizing_stack_panel_set_font(EGUI_VIEW_OF(&stack_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_virtualizing_stack_panel_set_meta_font(EGUI_VIEW_OF(&stack_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_virtualizing_stack_panel_set_compact_mode(EGUI_VIEW_OF(&stack_read_only), 1);
    egui_view_virtualizing_stack_panel_set_read_only_mode(EGUI_VIEW_OF(&stack_read_only), 1);
    egui_view_virtualizing_stack_panel_set_palette(EGUI_VIEW_OF(&stack_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xF7F9FB), EGUI_COLOR_HEX(0xD9E1E8),
                                                   EGUI_COLOR_HEX(0x4F5E6D), EGUI_COLOR_HEX(0x8895A1), EGUI_COLOR_HEX(0xA1AEBA), EGUI_COLOR_HEX(0xA9B8C0),
                                                   EGUI_COLOR_HEX(0xB9B0A6), EGUI_COLOR_HEX(0xB0BAC6));
    egui_view_virtualizing_stack_panel_override_static_preview_api(EGUI_VIEW_OF(&stack_read_only), &stack_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&stack_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&stack_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&stack_primary));
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
            apply_primary_default_state();
            apply_preview_states();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&stack_primary));
#endif
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, VIRTUALIZING_STACK_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_DOWN, 1);
        }
        EGUI_SIM_SET_WAIT(p_action, VIRTUALIZING_STACK_PANEL_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, VIRTUALIZING_STACK_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, VIRTUALIZING_STACK_PANEL_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, VIRTUALIZING_STACK_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&stack_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, VIRTUALIZING_STACK_PANEL_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, VIRTUALIZING_STACK_PANEL_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
