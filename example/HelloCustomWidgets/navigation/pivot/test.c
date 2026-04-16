#include "egui.h"
#include "egui_view_pivot.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define PIVOT_ROOT_WIDTH        224
#define PIVOT_ROOT_HEIGHT       206
#define PIVOT_PRIMARY_WIDTH     196
#define PIVOT_PRIMARY_HEIGHT    108
#define PIVOT_PREVIEW_WIDTH     104
#define PIVOT_PREVIEW_HEIGHT    72
#define PIVOT_BOTTOM_ROW_WIDTH  216
#define PIVOT_BOTTOM_ROW_HEIGHT 72
#define PIVOT_RECORD_WAIT       90
#define PIVOT_RECORD_FRAME_WAIT 170
#define PIVOT_RECORD_FINAL_WAIT 280
#define PIVOT_DEFAULT_INDEX     0

#define PRIMARY_ITEM_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_items))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static hcw_pivot_t primary_pivot;
static egui_view_linearlayout_t bottom_row;
static hcw_pivot_t compact_pivot;
static hcw_pivot_t read_only_pivot;
static egui_view_api_t compact_pivot_api;
static egui_view_api_t read_only_pivot_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Pivot";

static const hcw_pivot_item_t primary_items[] = {
        {"Overview", "Core view", "Project overview", "Goals, owner and next step.", "Section 1 of 3", HCW_PIVOT_TONE_ACCENT},
        {"Activity", "Daily feed", "Recent activity", "Ship notes and open reviews.", "Section 2 of 3", HCW_PIVOT_TONE_WARM},
        {"History", "Past work", "Change history", "Milestones and archived updates.", "Section 3 of 3", HCW_PIVOT_TONE_SUCCESS},
};

static const hcw_pivot_item_t compact_items[] = {
        {"Home", "Quick", "Home", "Pinned work", "2 tabs", HCW_PIVOT_TONE_ACCENT},
        {"Queue", "Next", "Queue", "Ready items", "2 tabs", HCW_PIVOT_TONE_NEUTRAL},
};

static const hcw_pivot_item_t read_only_items[] = {
        {"Usage", "Static", "Usage", "Frozen shell", "Locked", HCW_PIVOT_TONE_NEUTRAL},
        {"Audit", "Static", "Audit", "Read only", "Locked", HCW_PIVOT_TONE_WARM},
};

static void layout_page(void);

static void init_text_label(egui_view_label_t *label, egui_dim_t width, egui_dim_t height, const char *text, const egui_font_t *font, egui_color_t color,
                            uint8_t align_type)
{
    egui_view_label_init(EGUI_VIEW_OF(label));
    egui_view_set_size(EGUI_VIEW_OF(label), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(label), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(label), align_type);
    egui_view_label_set_font(EGUI_VIEW_OF(label), font);
    egui_view_label_set_font_color(EGUI_VIEW_OF(label), color, EGUI_ALPHA_100);
}

static void apply_primary_index(uint8_t index)
{
    hcw_pivot_set_current_index(EGUI_VIEW_OF(&primary_pivot), index % PRIMARY_ITEM_COUNT);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_index(PIVOT_DEFAULT_INDEX);
}

static void apply_preview_states(void)
{
    hcw_pivot_set_current_index(EGUI_VIEW_OF(&compact_pivot), 0);
    hcw_pivot_set_compact_mode(EGUI_VIEW_OF(&compact_pivot), 1);
    hcw_pivot_set_read_only_mode(EGUI_VIEW_OF(&compact_pivot), 0);

    hcw_pivot_set_current_index(EGUI_VIEW_OF(&read_only_pivot), 1);
    hcw_pivot_set_compact_mode(EGUI_VIEW_OF(&read_only_pivot), 1);
    hcw_pivot_set_read_only_mode(EGUI_VIEW_OF(&read_only_pivot), 1);

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
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), PIVOT_ROOT_WIDTH, PIVOT_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, PIVOT_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    hcw_pivot_init(EGUI_VIEW_OF(&primary_pivot));
    egui_view_set_size(EGUI_VIEW_OF(&primary_pivot), PIVOT_PRIMARY_WIDTH, PIVOT_PRIMARY_HEIGHT);
    hcw_pivot_set_items(EGUI_VIEW_OF(&primary_pivot), primary_items, PRIMARY_ITEM_COUNT);
    hcw_pivot_set_font(EGUI_VIEW_OF(&primary_pivot), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_pivot_set_meta_font(EGUI_VIEW_OF(&primary_pivot), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_pivot_set_palette(EGUI_VIEW_OF(&primary_pivot), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD4DCE4), EGUI_COLOR_HEX(0x17212C),
                          EGUI_COLOR_HEX(0x667585), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xF7F9FB));
    egui_view_set_margin(EGUI_VIEW_OF(&primary_pivot), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_pivot));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), PIVOT_BOTTOM_ROW_WIDTH, PIVOT_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    hcw_pivot_init(EGUI_VIEW_OF(&compact_pivot));
    egui_view_set_size(EGUI_VIEW_OF(&compact_pivot), PIVOT_PREVIEW_WIDTH, PIVOT_PREVIEW_HEIGHT);
    hcw_pivot_set_items(EGUI_VIEW_OF(&compact_pivot), compact_items, EGUI_ARRAY_SIZE(compact_items));
    hcw_pivot_set_font(EGUI_VIEW_OF(&compact_pivot), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_pivot_set_meta_font(EGUI_VIEW_OF(&compact_pivot), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_pivot_apply_compact_style(EGUI_VIEW_OF(&compact_pivot));
    hcw_pivot_set_palette(EGUI_VIEW_OF(&compact_pivot), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD6DEE6), EGUI_COLOR_HEX(0x22303C),
                          EGUI_COLOR_HEX(0x73808C), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xF7F9FB));
    hcw_pivot_override_static_preview_api(EGUI_VIEW_OF(&compact_pivot), &compact_pivot_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_pivot), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_pivot));

    hcw_pivot_init(EGUI_VIEW_OF(&read_only_pivot));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_pivot), PIVOT_PREVIEW_WIDTH, PIVOT_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_pivot), 8, 0, 0, 0);
    hcw_pivot_set_items(EGUI_VIEW_OF(&read_only_pivot), read_only_items, EGUI_ARRAY_SIZE(read_only_items));
    hcw_pivot_set_font(EGUI_VIEW_OF(&read_only_pivot), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_pivot_set_meta_font(EGUI_VIEW_OF(&read_only_pivot), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_pivot_apply_compact_style(EGUI_VIEW_OF(&read_only_pivot));
    hcw_pivot_set_palette(EGUI_VIEW_OF(&read_only_pivot), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xD9E1E8), EGUI_COLOR_HEX(0x566675),
                          EGUI_COLOR_HEX(0x8895A1), EGUI_COLOR_HEX(0xB8C4CF), EGUI_COLOR_HEX(0xF7F9FB));
    hcw_pivot_set_read_only_mode(EGUI_VIEW_OF(&read_only_pivot), 1);
    hcw_pivot_override_static_preview_api(EGUI_VIEW_OF(&read_only_pivot), &read_only_pivot_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&read_only_pivot), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_pivot));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
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
        EGUI_SIM_SET_WAIT(p_action, PIVOT_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_index(1);
        }
        EGUI_SIM_SET_WAIT(p_action, PIVOT_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PIVOT_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_index(2);
        }
        EGUI_SIM_SET_WAIT(p_action, PIVOT_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PIVOT_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PIVOT_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
