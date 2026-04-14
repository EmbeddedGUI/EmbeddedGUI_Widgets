#include "egui.h"
#include "egui_view_items_repeater.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define ITEMS_REPEATER_ROOT_WIDTH        224
#define ITEMS_REPEATER_ROOT_HEIGHT       244
#define ITEMS_REPEATER_PRIMARY_WIDTH     196
#define ITEMS_REPEATER_PRIMARY_HEIGHT    124
#define ITEMS_REPEATER_PREVIEW_WIDTH     104
#define ITEMS_REPEATER_PREVIEW_HEIGHT    78
#define ITEMS_REPEATER_BOTTOM_ROW_WIDTH  216
#define ITEMS_REPEATER_BOTTOM_ROW_HEIGHT 78
#define ITEMS_REPEATER_RECORD_WAIT       90
#define ITEMS_REPEATER_RECORD_FRAME_WAIT 180
#define ITEMS_REPEATER_RECORD_FINAL_WAIT 280

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)(sizeof(primary_snapshots) / sizeof(primary_snapshots[0])))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_items_repeater_t wrap_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_items_repeater_t wrap_compact;
static egui_view_items_repeater_t wrap_read_only;
static egui_view_api_t wrap_compact_api;
static egui_view_api_t wrap_read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Items Repeater";

static const egui_view_items_repeater_item_t primary_items_0[] = {
        {"OPS", "Inbox", "Live", EGUI_VIEW_ITEMS_REPEATER_TONE_ACCENT, 1, EGUI_VIEW_ITEMS_REPEATER_WIDTH_BALANCED},
        {"QA", "Audit", "5", EGUI_VIEW_ITEMS_REPEATER_TONE_WARNING, 0, EGUI_VIEW_ITEMS_REPEATER_WIDTH_COMPACT},
        {"REL", "Ship", "Ready", EGUI_VIEW_ITEMS_REPEATER_TONE_SUCCESS, 1, EGUI_VIEW_ITEMS_REPEATER_WIDTH_PROMINENT},
        {"DOC", "Notes", "3", EGUI_VIEW_ITEMS_REPEATER_TONE_NEUTRAL, 0, EGUI_VIEW_ITEMS_REPEATER_WIDTH_COMPACT},
        {"NAV", "Routes", "12", EGUI_VIEW_ITEMS_REPEATER_TONE_ACCENT, 0, EGUI_VIEW_ITEMS_REPEATER_WIDTH_BALANCED},
        {"OBS", "Health", "98%", EGUI_VIEW_ITEMS_REPEATER_TONE_SUCCESS, 1, EGUI_VIEW_ITEMS_REPEATER_WIDTH_PROMINENT},
};

static const egui_view_items_repeater_item_t primary_items_1[] = {
        {"UX", "Tokens", "Dense", EGUI_VIEW_ITEMS_REPEATER_TONE_ACCENT, 1, EGUI_VIEW_ITEMS_REPEATER_WIDTH_PROMINENT},
        {"IA", "Filter", "Tag", EGUI_VIEW_ITEMS_REPEATER_TONE_NEUTRAL, 0, EGUI_VIEW_ITEMS_REPEATER_WIDTH_BALANCED},
        {"OPS", "Queue", "2", EGUI_VIEW_ITEMS_REPEATER_TONE_WARNING, 0, EGUI_VIEW_ITEMS_REPEATER_WIDTH_COMPACT},
        {"REL", "Gate", "Open", EGUI_VIEW_ITEMS_REPEATER_TONE_SUCCESS, 1, EGUI_VIEW_ITEMS_REPEATER_WIDTH_BALANCED},
        {"DOC", "Spec", "4", EGUI_VIEW_ITEMS_REPEATER_TONE_NEUTRAL, 0, EGUI_VIEW_ITEMS_REPEATER_WIDTH_COMPACT},
        {"APP", "Owners", "3", EGUI_VIEW_ITEMS_REPEATER_TONE_ACCENT, 0, EGUI_VIEW_ITEMS_REPEATER_WIDTH_BALANCED},
};

static const egui_view_items_repeater_item_t primary_items_2[] = {
        {"UI", "Compact", "8px", EGUI_VIEW_ITEMS_REPEATER_TONE_ACCENT, 1, EGUI_VIEW_ITEMS_REPEATER_WIDTH_PROMINENT},
        {"SEC", "Guard", "2", EGUI_VIEW_ITEMS_REPEATER_TONE_WARNING, 0, EGUI_VIEW_ITEMS_REPEATER_WIDTH_COMPACT},
        {"CD", "Stages", "4", EGUI_VIEW_ITEMS_REPEATER_TONE_SUCCESS, 1, EGUI_VIEW_ITEMS_REPEATER_WIDTH_BALANCED},
        {"LOG", "Review", "6", EGUI_VIEW_ITEMS_REPEATER_TONE_NEUTRAL, 0, EGUI_VIEW_ITEMS_REPEATER_WIDTH_BALANCED},
        {"EXP", "Wrap", "Auto", EGUI_VIEW_ITEMS_REPEATER_TONE_ACCENT, 0, EGUI_VIEW_ITEMS_REPEATER_WIDTH_PROMINENT},
        {"CAL", "Order", "Keep", EGUI_VIEW_ITEMS_REPEATER_TONE_SUCCESS, 0, EGUI_VIEW_ITEMS_REPEATER_WIDTH_BALANCED},
};

static const egui_view_items_repeater_snapshot_t primary_snapshots[] = {
        {"OPS", "Wrap repeater", "Preset layout stays lightweight", "Wrap preset", primary_items_0, 6, 1, EGUI_VIEW_ITEMS_REPEATER_LAYOUT_WRAP},
        {"REL", "Stack repeater", "Same template switches to a vertical lane", "Stack preset", primary_items_1, 6, 0, EGUI_VIEW_ITEMS_REPEATER_LAYOUT_STACK},
        {"LAY", "Strip repeater", "Compact strip reuses the same item template", "Strip preset", primary_items_2, 6, 2, EGUI_VIEW_ITEMS_REPEATER_LAYOUT_STRIP},
};

static const egui_view_items_repeater_item_t compact_items_0[] = {
        {"A", "Ship", "2", EGUI_VIEW_ITEMS_REPEATER_TONE_ACCENT, 1, EGUI_VIEW_ITEMS_REPEATER_WIDTH_BALANCED},
        {"B", "QA", "5", EGUI_VIEW_ITEMS_REPEATER_TONE_WARNING, 0, EGUI_VIEW_ITEMS_REPEATER_WIDTH_COMPACT},
        {"C", "Docs", "3", EGUI_VIEW_ITEMS_REPEATER_TONE_NEUTRAL, 0, EGUI_VIEW_ITEMS_REPEATER_WIDTH_BALANCED},
        {"D", "Live", "1", EGUI_VIEW_ITEMS_REPEATER_TONE_SUCCESS, 0, EGUI_VIEW_ITEMS_REPEATER_WIDTH_COMPACT},
};

static const egui_view_items_repeater_snapshot_t compact_snapshot = {
        "OPS", "Compact wrap", "", "", compact_items_0, 4, 0, EGUI_VIEW_ITEMS_REPEATER_LAYOUT_WRAP};

static const egui_view_items_repeater_item_t read_only_items[] = {
        {"L1", "Locked", "View", EGUI_VIEW_ITEMS_REPEATER_TONE_NEUTRAL, 0, EGUI_VIEW_ITEMS_REPEATER_WIDTH_BALANCED},
        {"L2", "Muted", "Read", EGUI_VIEW_ITEMS_REPEATER_TONE_WARNING, 1, EGUI_VIEW_ITEMS_REPEATER_WIDTH_COMPACT},
        {"L3", "Done", "Pass", EGUI_VIEW_ITEMS_REPEATER_TONE_SUCCESS, 0, EGUI_VIEW_ITEMS_REPEATER_WIDTH_BALANCED},
        {"L4", "Queue", "Idle", EGUI_VIEW_ITEMS_REPEATER_TONE_ACCENT, 0, EGUI_VIEW_ITEMS_REPEATER_WIDTH_COMPACT},
};

static const egui_view_items_repeater_snapshot_t read_only_snapshot = {
        "LOCK", "Read only stack", "", "", read_only_items, 4, 1, EGUI_VIEW_ITEMS_REPEATER_LAYOUT_STACK};

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_items_repeater_set_current_snapshot(EGUI_VIEW_OF(&wrap_primary), index % PRIMARY_SNAPSHOT_COUNT);
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(0);
}

static void apply_preview_states(void)
{
    egui_view_items_repeater_set_current_snapshot(EGUI_VIEW_OF(&wrap_compact), 0);
    egui_view_items_repeater_set_current_snapshot(EGUI_VIEW_OF(&wrap_read_only), 0);
}

#if EGUI_CONFIG_RECORDING_TEST
static void request_page_snapshot(void)
{
    egui_view_invalidate(EGUI_VIEW_OF(&root_layout));
    recording_request_snapshot();
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), ITEMS_REPEATER_ROOT_WIDTH, ITEMS_REPEATER_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), ITEMS_REPEATER_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_items_repeater_init(EGUI_VIEW_OF(&wrap_primary));
    egui_view_set_size(EGUI_VIEW_OF(&wrap_primary), ITEMS_REPEATER_PRIMARY_WIDTH, ITEMS_REPEATER_PRIMARY_HEIGHT);
    egui_view_items_repeater_set_snapshots(EGUI_VIEW_OF(&wrap_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    egui_view_items_repeater_set_font(EGUI_VIEW_OF(&wrap_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_items_repeater_set_meta_font(EGUI_VIEW_OF(&wrap_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_items_repeater_set_palette(EGUI_VIEW_OF(&wrap_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xF4F7FA), EGUI_COLOR_HEX(0xD6DEE6),
                                     EGUI_COLOR_HEX(0x1B2834), EGUI_COLOR_HEX(0x6D7C8A), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x127A43),
                                     EGUI_COLOR_HEX(0xA15D00), EGUI_COLOR_HEX(0x7A8796));
    egui_view_set_margin(EGUI_VIEW_OF(&wrap_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&wrap_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), ITEMS_REPEATER_BOTTOM_ROW_WIDTH, ITEMS_REPEATER_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_items_repeater_init(EGUI_VIEW_OF(&wrap_compact));
    egui_view_set_size(EGUI_VIEW_OF(&wrap_compact), ITEMS_REPEATER_PREVIEW_WIDTH, ITEMS_REPEATER_PREVIEW_HEIGHT);
    egui_view_items_repeater_set_snapshots(EGUI_VIEW_OF(&wrap_compact), &compact_snapshot, 1);
    egui_view_items_repeater_set_font(EGUI_VIEW_OF(&wrap_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_items_repeater_set_meta_font(EGUI_VIEW_OF(&wrap_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_items_repeater_set_compact_mode(EGUI_VIEW_OF(&wrap_compact), 1);
    egui_view_items_repeater_set_palette(EGUI_VIEW_OF(&wrap_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xF3FBF8), EGUI_COLOR_HEX(0xD0E3DE),
                                     EGUI_COLOR_HEX(0x18312F), EGUI_COLOR_HEX(0x5B7A73), EGUI_COLOR_HEX(0x0D9488), EGUI_COLOR_HEX(0x0B8454),
                                     EGUI_COLOR_HEX(0x957000), EGUI_COLOR_HEX(0x6D7F8D));
    egui_view_items_repeater_override_static_preview_api(EGUI_VIEW_OF(&wrap_compact), &wrap_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&wrap_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&wrap_compact));

    egui_view_items_repeater_init(EGUI_VIEW_OF(&wrap_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&wrap_read_only), ITEMS_REPEATER_PREVIEW_WIDTH, ITEMS_REPEATER_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&wrap_read_only), 8, 0, 0, 0);
    egui_view_items_repeater_set_snapshots(EGUI_VIEW_OF(&wrap_read_only), &read_only_snapshot, 1);
    egui_view_items_repeater_set_font(EGUI_VIEW_OF(&wrap_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_items_repeater_set_meta_font(EGUI_VIEW_OF(&wrap_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_items_repeater_set_compact_mode(EGUI_VIEW_OF(&wrap_read_only), 1);
    egui_view_items_repeater_set_read_only_mode(EGUI_VIEW_OF(&wrap_read_only), 1);
    egui_view_items_repeater_set_palette(EGUI_VIEW_OF(&wrap_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xF7F9FB), EGUI_COLOR_HEX(0xD9E1E8),
                                     EGUI_COLOR_HEX(0x4F5E6D), EGUI_COLOR_HEX(0x8895A1), EGUI_COLOR_HEX(0xA1AEBA), EGUI_COLOR_HEX(0xA9B8C0),
                                     EGUI_COLOR_HEX(0xB9B0A6), EGUI_COLOR_HEX(0xB0BAC6));
    egui_view_items_repeater_override_static_preview_api(EGUI_VIEW_OF(&wrap_read_only), &wrap_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&wrap_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&wrap_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&wrap_primary));
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
            egui_view_request_focus(EGUI_VIEW_OF(&wrap_primary));
#endif
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, ITEMS_REPEATER_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, ITEMS_REPEATER_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, ITEMS_REPEATER_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, ITEMS_REPEATER_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, ITEMS_REPEATER_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&wrap_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, ITEMS_REPEATER_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, ITEMS_REPEATER_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif

