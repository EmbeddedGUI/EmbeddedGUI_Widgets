#include "egui.h"
#include "egui_view_list.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define LIST_ROOT_WIDTH        224
#define LIST_ROOT_HEIGHT       224
#define LIST_PRIMARY_WIDTH     196
#define LIST_PRIMARY_HEIGHT    112
#define LIST_PREVIEW_WIDTH     104
#define LIST_PREVIEW_HEIGHT    72
#define LIST_BOTTOM_ROW_WIDTH  216
#define LIST_BOTTOM_ROW_HEIGHT 72
#define LIST_RECORD_WAIT       90
#define LIST_RECORD_FRAME_WAIT 170
#define LIST_RECORD_FINAL_WAIT 280

#define LIST_DEFAULT_SNAPSHOT  0
#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct
{
    const egui_view_reference_list_item_t *items;
    uint8_t item_count;
    uint8_t current_index;
} list_demo_state_t;

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_reference_list_t primary_list;
static egui_view_linearlayout_t bottom_row;
static egui_view_reference_list_t compact_list;
static egui_view_reference_list_t read_only_list;
static egui_view_api_t compact_api;
static egui_view_api_t read_only_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "List";

static const egui_view_reference_list_item_t primary_items_0[] = {
        {"Inbox", "4 tasks waiting", "Live", EGUI_VIEW_REFERENCE_LIST_TONE_ACCENT, 1},
        {"Approvals", "Owner review due", "Due", EGUI_VIEW_REFERENCE_LIST_TONE_WARNING, 0},
        {"Published", "24 files synced", "Ready", EGUI_VIEW_REFERENCE_LIST_TONE_SUCCESS, 0},
        {"Archive", "Cold storage", "Muted", EGUI_VIEW_REFERENCE_LIST_TONE_NEUTRAL, 0},
};

static const egui_view_reference_list_item_t primary_items_1[] = {
        {"Review", "Delta check queued", "Watch", EGUI_VIEW_REFERENCE_LIST_TONE_WARNING, 1},
        {"Variants", "Compact preview ready", "2", EGUI_VIEW_REFERENCE_LIST_TONE_ACCENT, 0},
        {"Exports", "Nightly package passed", "OK", EGUI_VIEW_REFERENCE_LIST_TONE_SUCCESS, 0},
        {"Notes", "Read only example", "Static", EGUI_VIEW_REFERENCE_LIST_TONE_NEUTRAL, 0},
};

static const egui_view_reference_list_item_t primary_items_2[] = {
        {"Archive", "Retention snapshot", "31", EGUI_VIEW_REFERENCE_LIST_TONE_WARNING, 1},
        {"Cleanup", "Batch queued", "8", EGUI_VIEW_REFERENCE_LIST_TONE_SUCCESS, 0},
        {"Restore", "Owner needed", "4", EGUI_VIEW_REFERENCE_LIST_TONE_ACCENT, 0},
        {"Audit", "History locked", "RO", EGUI_VIEW_REFERENCE_LIST_TONE_NEUTRAL, 0},
};

static const list_demo_state_t primary_snapshots[] = {
        {primary_items_0, EGUI_ARRAY_SIZE(primary_items_0), 0},
        {primary_items_1, EGUI_ARRAY_SIZE(primary_items_1), 1},
        {primary_items_2, EGUI_ARRAY_SIZE(primary_items_2), 2},
};

static const egui_view_reference_list_item_t compact_items[] = {
        {"Compact", "Trimmed density", "4", EGUI_VIEW_REFERENCE_LIST_TONE_ACCENT, 1},
        {"Review", "Muted meta", "2", EGUI_VIEW_REFERENCE_LIST_TONE_WARNING, 0},
        {"Ready", "Short rows", "OK", EGUI_VIEW_REFERENCE_LIST_TONE_SUCCESS, 0},
        {"Archive", "", "RO", EGUI_VIEW_REFERENCE_LIST_TONE_NEUTRAL, 0},
};

static const egui_view_reference_list_item_t read_only_items[] = {
        {"Read only", "Static preview", "RO", EGUI_VIEW_REFERENCE_LIST_TONE_NEUTRAL, 1},
        {"Selection", "Input suppressed", "1", EGUI_VIEW_REFERENCE_LIST_TONE_ACCENT, 0},
        {"Status", "Visual only", "Lock", EGUI_VIEW_REFERENCE_LIST_TONE_WARNING, 0},
};

static const list_demo_state_t compact_preview_state = {compact_items, EGUI_ARRAY_SIZE(compact_items), 0};
static const list_demo_state_t read_only_preview_state = {read_only_items, EGUI_ARRAY_SIZE(read_only_items), 0};

static void init_text_label(egui_view_label_t *label, egui_dim_t width, egui_dim_t height, const char *text, const egui_font_t *font, egui_color_t color,
                            uint8_t align)
{
    egui_view_label_init(EGUI_VIEW_OF(label));
    egui_view_set_size(EGUI_VIEW_OF(label), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(label), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(label), align);
    egui_view_label_set_font(EGUI_VIEW_OF(label), font);
    egui_view_label_set_font_color(EGUI_VIEW_OF(label), color, EGUI_ALPHA_100);
}

static uint8_t clamp_index(uint8_t item_count, uint8_t index)
{
    if (item_count == 0)
    {
        return EGUI_VIEW_REFERENCE_LIST_INDEX_NONE;
    }
    if (index >= item_count)
    {
        return 0;
    }
    return index;
}

static void apply_snapshot(egui_view_reference_list_t *view, const list_demo_state_t *snapshot)
{
    egui_view_reference_list_set_items(EGUI_VIEW_OF(view), snapshot->items, snapshot->item_count);
    egui_view_reference_list_set_current_index(EGUI_VIEW_OF(view), clamp_index(snapshot->item_count, snapshot->current_index));
}

static void layout_page(void);

static void apply_primary_snapshot(uint8_t index)
{
    apply_snapshot(&primary_list, &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT]);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(LIST_DEFAULT_SNAPSHOT);
}

static void apply_compact_state(void)
{
    apply_snapshot(&compact_list, &compact_preview_state);
    egui_view_reference_list_set_compact_mode(EGUI_VIEW_OF(&compact_list), 1);
    egui_view_reference_list_set_read_only_mode(EGUI_VIEW_OF(&compact_list), 0);
}

static void apply_read_only_state(void)
{
    apply_snapshot(&read_only_list, &read_only_preview_state);
    egui_view_reference_list_set_compact_mode(EGUI_VIEW_OF(&read_only_list), 1);
    egui_view_reference_list_set_read_only_mode(EGUI_VIEW_OF(&read_only_list), 1);
}

static void apply_preview_states(void)
{
    apply_compact_state();
    apply_read_only_state();
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
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
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

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), LIST_ROOT_WIDTH, LIST_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, LIST_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_reference_list_init(EGUI_VIEW_OF(&primary_list));
    egui_view_set_size(EGUI_VIEW_OF(&primary_list), LIST_PRIMARY_WIDTH, LIST_PRIMARY_HEIGHT);
    egui_view_reference_list_set_font(EGUI_VIEW_OF(&primary_list), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_reference_list_set_meta_font(EGUI_VIEW_OF(&primary_list), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_list), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_list));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), LIST_BOTTOM_ROW_WIDTH, LIST_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_reference_list_init(EGUI_VIEW_OF(&compact_list));
    egui_view_set_size(EGUI_VIEW_OF(&compact_list), LIST_PREVIEW_WIDTH, LIST_PREVIEW_HEIGHT);
    egui_view_reference_list_set_font(EGUI_VIEW_OF(&compact_list), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_reference_list_set_meta_font(EGUI_VIEW_OF(&compact_list), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_reference_list_override_static_preview_api(EGUI_VIEW_OF(&compact_list), &compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_list), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_list));

    egui_view_reference_list_init(EGUI_VIEW_OF(&read_only_list));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_list), LIST_PREVIEW_WIDTH, LIST_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_list), 8, 0, 0, 0);
    egui_view_reference_list_set_font(EGUI_VIEW_OF(&read_only_list), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_reference_list_set_meta_font(EGUI_VIEW_OF(&read_only_list), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_reference_list_override_static_preview_api(EGUI_VIEW_OF(&read_only_list), &read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&read_only_list), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_list));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);
    layout_local_views();

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&primary_list));
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
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, LIST_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, LIST_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, LIST_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, LIST_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, LIST_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
        }
        EGUI_SIM_SET_WAIT(p_action, LIST_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, LIST_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
