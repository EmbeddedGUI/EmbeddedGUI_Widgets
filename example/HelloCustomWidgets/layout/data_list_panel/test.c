#include "egui.h"
#include "egui_view_data_list_panel.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define DATA_LIST_PANEL_ROOT_WIDTH        224
#define DATA_LIST_PANEL_ROOT_HEIGHT       236
#define DATA_LIST_PANEL_PRIMARY_WIDTH     196
#define DATA_LIST_PANEL_PRIMARY_HEIGHT    116
#define DATA_LIST_PANEL_PREVIEW_WIDTH     104
#define DATA_LIST_PANEL_PREVIEW_HEIGHT    80
#define DATA_LIST_PANEL_BOTTOM_ROW_WIDTH  216
#define DATA_LIST_PANEL_BOTTOM_ROW_HEIGHT 80
#define DATA_LIST_PANEL_RECORD_WAIT       90
#define DATA_LIST_PANEL_RECORD_FRAME_WAIT 170

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_data_list_panel_t panel_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_column;
static egui_view_data_list_panel_t panel_compact;
static egui_view_linearlayout_t read_only_column;
static egui_view_data_list_panel_t panel_read_only;
static egui_view_api_t panel_compact_api;
static egui_view_api_t panel_read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Data List";
static uint8_t primary_snapshot_index = 0;
static uint8_t primary_item_index = 0;
static uint8_t compact_snapshot_index = 0;
static uint8_t compact_item_index = 0;

static const egui_view_data_list_panel_item_t primary_items_0[] = {
        {"NS", "Nightly sync", "Ops handoff", "18", "Blocked", EGUI_VIEW_DATA_LIST_PANEL_TONE_ACCENT, 1},
        {"EX", "Exports", "Awaiting review", "12", "Review", EGUI_VIEW_DATA_LIST_PANEL_TONE_WARNING, 0},
        {"AP", "Approvals", "Ready to send", "7", "Ready", EGUI_VIEW_DATA_LIST_PANEL_TONE_SUCCESS, 0},
};

static const egui_view_data_list_panel_item_t primary_items_1[] = {
        {"TH", "Thumb pack", "Fresh uploads", "24", "Fresh", EGUI_VIEW_DATA_LIST_PANEL_TONE_SUCCESS, 1},
        {"MD", "Metadata", "Needs owner", "9", "Review", EGUI_VIEW_DATA_LIST_PANEL_TONE_WARNING, 0},
        {"VC", "Version diff", "Build mismatch", "5", "Watch", EGUI_VIEW_DATA_LIST_PANEL_TONE_ACCENT, 0},
};

static const egui_view_data_list_panel_item_t primary_items_2[] = {
        {"RT", "Retention", "31 day hold", "31", "Watch", EGUI_VIEW_DATA_LIST_PANEL_TONE_WARNING, 1},
        {"CL", "Cleanup", "Queued purge", "8", "Ready", EGUI_VIEW_DATA_LIST_PANEL_TONE_SUCCESS, 0},
        {"RS", "Restore", "Pending owner", "4", "Review", EGUI_VIEW_DATA_LIST_PANEL_TONE_ACCENT, 0},
};

static const egui_view_data_list_panel_snapshot_t primary_snapshots[] = {
        {"QUEUE", "Sync queue", "3 rows tracked", primary_items_0, 3, 0},
        {"ASSET", "Asset review", "3 assets staged", primary_items_1, 3, 0},
        {"ARCHIVE", "Archive sweep", "Retention overview", primary_items_2, 3, 0},
};

static const egui_view_data_list_panel_item_t compact_items_0[] = {
        {"NS", "Nightly", "Ops", "18", "Blocked", EGUI_VIEW_DATA_LIST_PANEL_TONE_ACCENT, 1},
        {"EX", "Exports", "Wait", "12", "Review", EGUI_VIEW_DATA_LIST_PANEL_TONE_WARNING, 0},
        {"AP", "Approve", "Ready", "7", "Ready", EGUI_VIEW_DATA_LIST_PANEL_TONE_SUCCESS, 0},
};

static const egui_view_data_list_panel_item_t compact_items_1[] = {
        {"TH", "Thumbs", "Fresh", "24", "Fresh", EGUI_VIEW_DATA_LIST_PANEL_TONE_SUCCESS, 1},
        {"MD", "Meta", "Owner", "9", "Review", EGUI_VIEW_DATA_LIST_PANEL_TONE_WARNING, 0},
        {"VC", "Diff", "Build", "5", "Watch", EGUI_VIEW_DATA_LIST_PANEL_TONE_ACCENT, 0},
};

static const egui_view_data_list_panel_snapshot_t compact_snapshots[] = {
        {"CMP", "Compact", "Rows staged", compact_items_0, 3, 0},
        {"CMP", "Compact", "Rows checked", compact_items_1, 3, 0},
};

static const egui_view_data_list_panel_item_t read_only_items[] = {
        {"AR", "Archive", "Cold store", "3", "Muted", EGUI_VIEW_DATA_LIST_PANEL_TONE_NEUTRAL, 1},
        {"RS", "Restore", "Pending", "4", "Review", EGUI_VIEW_DATA_LIST_PANEL_TONE_ACCENT, 0},
        {"CL", "Cleanup", "Queued", "8", "Ready", EGUI_VIEW_DATA_LIST_PANEL_TONE_SUCCESS, 0},
};

static const egui_view_data_list_panel_snapshot_t read_only_snapshots[] = {
        {"LOCK", "Read only", "Static preview", read_only_items, 3, 0},
};

static int dismiss_primary_focus_on_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (event->type == EGUI_MOTION_EVENT_ACTION_DOWN)
    {
        egui_view_clear_focus(EGUI_VIEW_OF(&panel_primary));
    }
#else
    EGUI_UNUSED(event);
#endif
    return 1;
}

static uint8_t clamp_item_index(const egui_view_data_list_panel_snapshot_t *snapshot, uint8_t item_index)
{
    if (snapshot == NULL || snapshot->item_count == 0)
    {
        return 0;
    }
    if (item_index >= snapshot->item_count)
    {
        return snapshot->focus_index < snapshot->item_count ? snapshot->focus_index : 0;
    }
    return item_index;
}

static void apply_primary_state(uint8_t snapshot_index, uint8_t item_index)
{
    const egui_view_data_list_panel_snapshot_t *snapshot;

    primary_snapshot_index = (uint8_t)(snapshot_index % EGUI_ARRAY_SIZE(primary_snapshots));
    snapshot = &primary_snapshots[primary_snapshot_index];
    primary_item_index = clamp_item_index(snapshot, item_index);

    egui_view_data_list_panel_set_current_snapshot(EGUI_VIEW_OF(&panel_primary), primary_snapshot_index);
    egui_view_data_list_panel_set_current_index(EGUI_VIEW_OF(&panel_primary), primary_item_index);
}

static void apply_compact_state(uint8_t snapshot_index, uint8_t item_index)
{
    const egui_view_data_list_panel_snapshot_t *snapshot;

    compact_snapshot_index = (uint8_t)(snapshot_index % EGUI_ARRAY_SIZE(compact_snapshots));
    snapshot = &compact_snapshots[compact_snapshot_index];
    compact_item_index = clamp_item_index(snapshot, item_index);

    egui_view_data_list_panel_set_current_snapshot(EGUI_VIEW_OF(&panel_compact), compact_snapshot_index);
    egui_view_data_list_panel_set_current_index(EGUI_VIEW_OF(&panel_compact), compact_item_index);
}

#if EGUI_CONFIG_RECORDING_TEST
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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), DATA_LIST_PANEL_ROOT_WIDTH, DATA_LIST_PANEL_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), DATA_LIST_PANEL_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_data_list_panel_init(EGUI_VIEW_OF(&panel_primary));
    egui_view_set_size(EGUI_VIEW_OF(&panel_primary), DATA_LIST_PANEL_PRIMARY_WIDTH, DATA_LIST_PANEL_PRIMARY_HEIGHT);
    egui_view_data_list_panel_set_snapshots(EGUI_VIEW_OF(&panel_primary), primary_snapshots, EGUI_ARRAY_SIZE(primary_snapshots));
    egui_view_data_list_panel_set_font(EGUI_VIEW_OF(&panel_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_data_list_panel_set_meta_font(EGUI_VIEW_OF(&panel_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_data_list_panel_set_palette(EGUI_VIEW_OF(&panel_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0xEAF0F7),
                                          EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45),
                                          EGUI_COLOR_HEX(0x9D5D00), EGUI_COLOR_HEX(0x7A8796));
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&panel_primary), 1);
#endif
    egui_view_set_margin(EGUI_VIEW_OF(&panel_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&panel_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), DATA_LIST_PANEL_BOTTOM_ROW_WIDTH, DATA_LIST_PANEL_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&compact_column));
    egui_view_set_size(EGUI_VIEW_OF(&compact_column), DATA_LIST_PANEL_PREVIEW_WIDTH, DATA_LIST_PANEL_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&compact_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&compact_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_column));

    egui_view_data_list_panel_init(EGUI_VIEW_OF(&panel_compact));
    egui_view_set_size(EGUI_VIEW_OF(&panel_compact), DATA_LIST_PANEL_PREVIEW_WIDTH, DATA_LIST_PANEL_PREVIEW_HEIGHT);
    egui_view_data_list_panel_set_snapshots(EGUI_VIEW_OF(&panel_compact), compact_snapshots, EGUI_ARRAY_SIZE(compact_snapshots));
    egui_view_data_list_panel_set_compact_mode(EGUI_VIEW_OF(&panel_compact), 1);
    egui_view_data_list_panel_set_font(EGUI_VIEW_OF(&panel_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_data_list_panel_set_meta_font(EGUI_VIEW_OF(&panel_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_data_list_panel_set_palette(EGUI_VIEW_OF(&panel_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0xEAF0F7),
                                          EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45),
                                          EGUI_COLOR_HEX(0x9D5D00), EGUI_COLOR_HEX(0x7A8796));
    egui_view_data_list_panel_override_static_preview_api(EGUI_VIEW_OF(&panel_compact), &panel_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    panel_compact_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&panel_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_column), EGUI_VIEW_OF(&panel_compact));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&read_only_column));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_column), DATA_LIST_PANEL_PREVIEW_WIDTH, DATA_LIST_PANEL_BOTTOM_ROW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_column), 8, 0, 0, 0);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&read_only_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&read_only_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_column));

    egui_view_data_list_panel_init(EGUI_VIEW_OF(&panel_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&panel_read_only), DATA_LIST_PANEL_PREVIEW_WIDTH, DATA_LIST_PANEL_PREVIEW_HEIGHT);
    egui_view_data_list_panel_set_snapshots(EGUI_VIEW_OF(&panel_read_only), read_only_snapshots, EGUI_ARRAY_SIZE(read_only_snapshots));
    egui_view_data_list_panel_set_compact_mode(EGUI_VIEW_OF(&panel_read_only), 1);
    egui_view_data_list_panel_set_read_only_mode(EGUI_VIEW_OF(&panel_read_only), 1);
    egui_view_data_list_panel_set_font(EGUI_VIEW_OF(&panel_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_data_list_panel_set_meta_font(EGUI_VIEW_OF(&panel_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_data_list_panel_set_palette(EGUI_VIEW_OF(&panel_read_only), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0xEAF0F7),
                                          EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45),
                                          EGUI_COLOR_HEX(0x9D5D00), EGUI_COLOR_HEX(0x7A8796));
    egui_view_data_list_panel_override_static_preview_api(EGUI_VIEW_OF(&panel_read_only), &panel_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    panel_read_only_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&panel_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_column), EGUI_VIEW_OF(&panel_read_only));

    apply_primary_state(0, 0);
    apply_compact_state(0, 0);
    egui_view_data_list_panel_set_current_snapshot(EGUI_VIEW_OF(&panel_read_only), 0);
    egui_view_data_list_panel_set_current_index(EGUI_VIEW_OF(&panel_read_only), 0);

    {
        hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);
    }

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&compact_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&read_only_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&panel_primary));
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
            apply_primary_state(0, 0);
            apply_compact_state(0, 0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&panel_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, DATA_LIST_PANEL_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DATA_LIST_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_state(0, 2);
        }
        EGUI_SIM_SET_WAIT(p_action, DATA_LIST_PANEL_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DATA_LIST_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_state(1, 1);
        }
        EGUI_SIM_SET_WAIT(p_action, DATA_LIST_PANEL_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DATA_LIST_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_state(2, 0);
        }
        EGUI_SIM_SET_WAIT(p_action, DATA_LIST_PANEL_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DATA_LIST_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            apply_compact_state(1, 0);
        }
        EGUI_SIM_SET_WAIT(p_action, DATA_LIST_PANEL_RECORD_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DATA_LIST_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 10:
        if (first_call)
        {
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&panel_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, DATA_LIST_PANEL_RECORD_WAIT);
        return true;
    case 11:
        if (first_call)
        {
            set_click_view_center(p_action, EGUI_VIEW_OF(&panel_compact), 220);
        }
        return true;
    case 12:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 520);
        return true;
    default:
        return false;
    }
}
#endif
