#include "egui.h"
#include "egui_view_accordion.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define ACCORDION_ROOT_WIDTH        224
#define ACCORDION_ROOT_HEIGHT       262
#define ACCORDION_PRIMARY_WIDTH     196
#define ACCORDION_PRIMARY_HEIGHT    142
#define ACCORDION_PREVIEW_WIDTH     104
#define ACCORDION_PREVIEW_HEIGHT    76
#define ACCORDION_BOTTOM_ROW_WIDTH  216
#define ACCORDION_BOTTOM_ROW_HEIGHT 76
#define ACCORDION_RECORD_WAIT       90
#define ACCORDION_RECORD_FRAME_WAIT 180
#define ACCORDION_RECORD_FINAL_WAIT 300

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_accordion_t accordion_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_accordion_t accordion_compact;
static egui_view_accordion_t accordion_read_only;
static egui_view_api_t accordion_compact_api;
static egui_view_api_t accordion_read_only_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Accordion";
static void layout_page(void);

static const egui_view_accordion_item_t primary_items[] = {
        {"Workspace", "Review policy and sync cadence.", "Owners approve sync changes here.", "WF", EGUI_VIEW_ACCORDION_TONE_ACCENT, 1},
        {"Identity", "Verify access before publish.", "Two reviewers must confirm identity.", "ID", EGUI_VIEW_ACCORDION_TONE_SUCCESS, 0},
        {"Release", "Stage rollout and rollback notes.", "Pilot rollout waits for sign-off.", "UP", EGUI_VIEW_ACCORDION_TONE_WARNING, 0},
};

static const egui_view_accordion_item_t compact_items[] = {
        {"Sync", "", "Compact detail.", "S", EGUI_VIEW_ACCORDION_TONE_ACCENT, 1},
        {"Audit", "", "Hidden detail.", "A", EGUI_VIEW_ACCORDION_TONE_NEUTRAL, 0},
};

static const egui_view_accordion_item_t read_only_items[] = {
        {"Managed", "", "Preview only.", "M", EGUI_VIEW_ACCORDION_TONE_NEUTRAL, 1},
        {"Locked", "", "No toggle.", "L", EGUI_VIEW_ACCORDION_TONE_NEUTRAL, 0},
};

static void apply_primary_state(uint8_t expanded_index)
{
    egui_view_accordion_set_expanded_index(EGUI_VIEW_OF(&accordion_primary), expanded_index);
    egui_view_accordion_set_focused_index(EGUI_VIEW_OF(&accordion_primary), expanded_index == EGUI_VIEW_ACCORDION_INDEX_NONE ? 0 : expanded_index);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_state(0);
}

static void apply_preview_states(void)
{
    egui_view_accordion_set_expanded_index(EGUI_VIEW_OF(&accordion_compact), 0);
    egui_view_accordion_set_focused_index(EGUI_VIEW_OF(&accordion_compact), 0);
    egui_view_accordion_set_expanded_index(EGUI_VIEW_OF(&accordion_read_only), 0);
    egui_view_accordion_set_focused_index(EGUI_VIEW_OF(&accordion_read_only), 0);
    if (ui_ready)
    {
        layout_page();
    }
}

static void layout_local_views(void)
{
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));
}

static void layout_page(void)
{
    layout_local_views();
    egui_core_layout_childs_user_root_view(uicode_get_core(), EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}

#if EGUI_CONFIG_RECORDING_TEST
static void request_page_snapshot(void)
{
    layout_page();
    egui_view_invalidate(EGUI_VIEW_OF(&root_layout));
    recording_request_snapshot();
}
#endif

void test_init_ui(void)
{
    ui_ready = 0;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), ACCORDION_ROOT_WIDTH, ACCORDION_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&title_label), ACCORDION_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_accordion_init(EGUI_VIEW_OF(&accordion_primary));
    egui_view_set_size(EGUI_VIEW_OF(&accordion_primary), ACCORDION_PRIMARY_WIDTH, ACCORDION_PRIMARY_HEIGHT);
    egui_view_accordion_set_items(EGUI_VIEW_OF(&accordion_primary), primary_items, (uint8_t)(sizeof(primary_items) / sizeof(primary_items[0])));
    egui_view_accordion_set_font(EGUI_VIEW_OF(&accordion_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_accordion_set_meta_font(EGUI_VIEW_OF(&accordion_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_accordion_set_palette(EGUI_VIEW_OF(&accordion_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xF8FAFC), EGUI_COLOR_HEX(0xD4DDE6),
                                    EGUI_COLOR_HEX(0x182331), EGUI_COLOR_HEX(0x647587), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x107C41),
                                    EGUI_COLOR_HEX(0x9A6400), EGUI_COLOR_HEX(0x687484));
    egui_view_set_margin(EGUI_VIEW_OF(&accordion_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&accordion_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), ACCORDION_BOTTOM_ROW_WIDTH, ACCORDION_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_accordion_init(EGUI_VIEW_OF(&accordion_compact));
    egui_view_set_size(EGUI_VIEW_OF(&accordion_compact), ACCORDION_PREVIEW_WIDTH, ACCORDION_PREVIEW_HEIGHT);
    egui_view_accordion_set_items(EGUI_VIEW_OF(&accordion_compact), compact_items, (uint8_t)(sizeof(compact_items) / sizeof(compact_items[0])));
    egui_view_accordion_set_font(EGUI_VIEW_OF(&accordion_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_accordion_set_meta_font(EGUI_VIEW_OF(&accordion_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_accordion_set_compact_mode(EGUI_VIEW_OF(&accordion_compact), 1);
    egui_view_accordion_override_static_preview_api(EGUI_VIEW_OF(&accordion_compact), &accordion_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&accordion_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&accordion_compact));

    egui_view_accordion_init(EGUI_VIEW_OF(&accordion_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&accordion_read_only), ACCORDION_PREVIEW_WIDTH, ACCORDION_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&accordion_read_only), 8, 0, 0, 0);
    egui_view_accordion_set_items(EGUI_VIEW_OF(&accordion_read_only), read_only_items, (uint8_t)(sizeof(read_only_items) / sizeof(read_only_items[0])));
    egui_view_accordion_set_font(EGUI_VIEW_OF(&accordion_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_accordion_set_meta_font(EGUI_VIEW_OF(&accordion_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_accordion_set_compact_mode(EGUI_VIEW_OF(&accordion_read_only), 1);
    egui_view_accordion_set_read_only_mode(EGUI_VIEW_OF(&accordion_read_only), 1);
    egui_view_accordion_set_palette(EGUI_VIEW_OF(&accordion_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xF8FAFB), EGUI_COLOR_HEX(0xD8DFE6),
                                    EGUI_COLOR_HEX(0x8794A2), EGUI_COLOR_HEX(0x98A5B2), EGUI_COLOR_HEX(0xA2AFBA), EGUI_COLOR_HEX(0xA8B7AE),
                                    EGUI_COLOR_HEX(0xB9AD99), EGUI_COLOR_HEX(0xB4BDC8));
    egui_view_accordion_override_static_preview_api(EGUI_VIEW_OF(&accordion_read_only), &accordion_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&accordion_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&accordion_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&accordion_primary));
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
            apply_preview_states();
            apply_primary_default_state();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, ACCORDION_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_state(1);
        }
        EGUI_SIM_SET_WAIT(p_action, ACCORDION_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, ACCORDION_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_state(2);
        }
        EGUI_SIM_SET_WAIT(p_action, ACCORDION_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, ACCORDION_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
        }
        EGUI_SIM_SET_WAIT(p_action, ACCORDION_RECORD_FINAL_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, ACCORDION_RECORD_FINAL_WAIT);
        return true;
    default:
        break;
    }

    return false;
}
#endif
