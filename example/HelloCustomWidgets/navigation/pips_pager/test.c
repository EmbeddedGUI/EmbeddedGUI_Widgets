#include <string.h>

#include "egui.h"
#include "egui_view_pips_pager.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define PIPS_PAGER_ROOT_WIDTH        224
#define PIPS_PAGER_ROOT_HEIGHT       198
#define PIPS_PAGER_PRIMARY_WIDTH     196
#define PIPS_PAGER_PRIMARY_HEIGHT    92
#define PIPS_PAGER_PREVIEW_WIDTH     104
#define PIPS_PAGER_PREVIEW_HEIGHT    58
#define PIPS_PAGER_BOTTOM_ROW_WIDTH  216
#define PIPS_PAGER_BOTTOM_ROW_HEIGHT 58
#define PIPS_PAGER_RECORD_WAIT       110
#define PIPS_PAGER_RECORD_FRAME_WAIT 150
#define PIPS_PAGER_RECORD_FINAL_WAIT 420

typedef struct pager_snapshot pager_snapshot_t;
struct pager_snapshot
{
    const char *title;
    const char *helper;
    uint8_t total_count;
    uint8_t current_index;
    uint8_t visible_count;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_pips_pager_t pager_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_pips_pager_t pager_compact;
static egui_view_pips_pager_t pager_read_only;
static egui_view_api_t pager_compact_api;
static egui_view_api_t pager_read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Pips Pager";
static uint8_t primary_snapshot_index = 0;

static const pager_snapshot_t primary_snapshots[] = {
        {"Onboarding", "Discrete pips and next-step paging", 7, 1, 5},
        {"Gallery", "Center the active page in a short rail", 9, 4, 5},
        {"Report deck", "Keep long decks compact without tabs", 12, 8, 5},
};

static const pager_snapshot_t compact_snapshots[] = {
        {"Compact", "", 6, 2, 4},
};

static const pager_snapshot_t read_only_snapshot = {"Read only", "", 7, 3, 4};

static void apply_snapshot(egui_view_t *view, const pager_snapshot_t *snapshot)
{
    egui_view_pips_pager_set_title(view, snapshot->title);
    egui_view_pips_pager_set_helper(view, snapshot->helper);
    egui_view_pips_pager_set_page_metrics(view, snapshot->total_count, snapshot->current_index, snapshot->visible_count);
    egui_view_pips_pager_set_current_part(view, EGUI_VIEW_PIPS_PAGER_PART_PIP);
}

static void apply_primary_snapshot(uint8_t index)
{
    const pager_snapshot_t *snapshot = &primary_snapshots[index % (sizeof(primary_snapshots) / sizeof(primary_snapshots[0]))];

    primary_snapshot_index = index % (sizeof(primary_snapshots) / sizeof(primary_snapshots[0]));
    apply_snapshot(EGUI_VIEW_OF(&pager_primary), snapshot);
}

static void apply_preview_states(void)
{
    apply_snapshot(EGUI_VIEW_OF(&pager_compact), &compact_snapshots[0]);
    apply_snapshot(EGUI_VIEW_OF(&pager_read_only), &read_only_snapshot);
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), PIPS_PAGER_ROOT_WIDTH, PIPS_PAGER_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), PIPS_PAGER_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_pips_pager_init(EGUI_VIEW_OF(&pager_primary));
    egui_view_set_size(EGUI_VIEW_OF(&pager_primary), PIPS_PAGER_PRIMARY_WIDTH, PIPS_PAGER_PRIMARY_HEIGHT);
    egui_view_pips_pager_set_font(EGUI_VIEW_OF(&pager_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_pips_pager_set_meta_font(EGUI_VIEW_OF(&pager_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_pips_pager_set_palette(EGUI_VIEW_OF(&pager_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD2DBE3), EGUI_COLOR_HEX(0x1A2734),
                                     EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xAEB9C4), EGUI_COLOR_HEX(0xB9CCE0));
    egui_view_set_margin(EGUI_VIEW_OF(&pager_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&pager_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), PIPS_PAGER_BOTTOM_ROW_WIDTH, PIPS_PAGER_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_pips_pager_init(EGUI_VIEW_OF(&pager_compact));
    egui_view_set_size(EGUI_VIEW_OF(&pager_compact), PIPS_PAGER_PREVIEW_WIDTH, PIPS_PAGER_PREVIEW_HEIGHT);
    egui_view_pips_pager_set_font(EGUI_VIEW_OF(&pager_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_pips_pager_set_meta_font(EGUI_VIEW_OF(&pager_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_pips_pager_set_compact_mode(EGUI_VIEW_OF(&pager_compact), 1);
    egui_view_pips_pager_set_palette(EGUI_VIEW_OF(&pager_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD2DBE3), EGUI_COLOR_HEX(0x1A2734),
                                     EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xAEB9C4), EGUI_COLOR_HEX(0xB9CCE0));
    egui_view_pips_pager_override_static_preview_api(EGUI_VIEW_OF(&pager_compact), &pager_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&pager_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&pager_compact));

    egui_view_pips_pager_init(EGUI_VIEW_OF(&pager_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&pager_read_only), PIPS_PAGER_PREVIEW_WIDTH, PIPS_PAGER_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&pager_read_only), 8, 0, 0, 0);
    egui_view_pips_pager_set_font(EGUI_VIEW_OF(&pager_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_pips_pager_set_meta_font(EGUI_VIEW_OF(&pager_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_pips_pager_set_compact_mode(EGUI_VIEW_OF(&pager_read_only), 1);
    egui_view_pips_pager_set_read_only_mode(EGUI_VIEW_OF(&pager_read_only), 1);
    egui_view_pips_pager_set_palette(EGUI_VIEW_OF(&pager_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xD8DFE6), EGUI_COLOR_HEX(0x536474),
                                     EGUI_COLOR_HEX(0x8896A4), EGUI_COLOR_HEX(0xA7B4C1), EGUI_COLOR_HEX(0xB8C3CD), EGUI_COLOR_HEX(0xC5D2DE));
    egui_view_pips_pager_override_static_preview_api(EGUI_VIEW_OF(&pager_read_only), &pager_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&pager_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&pager_read_only));

    apply_primary_snapshot(0);
    apply_preview_states();

    {
        hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);
    }

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    EGUI_VIEW_OF(&pager_primary)->api->on_key_event(EGUI_VIEW_OF(&pager_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&pager_primary)->api->on_key_event(EGUI_VIEW_OF(&pager_primary), &event);
}

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
            apply_preview_states();
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PIPS_PAGER_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_RIGHT);
        }
        EGUI_SIM_SET_WAIT(p_action, PIPS_PAGER_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PIPS_PAGER_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_END);
        }
        EGUI_SIM_SET_WAIT(p_action, PIPS_PAGER_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PIPS_PAGER_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_snapshot((uint8_t)(primary_snapshot_index + 1));
        }
        EGUI_SIM_SET_WAIT(p_action, PIPS_PAGER_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PIPS_PAGER_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_snapshot(0);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PIPS_PAGER_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
