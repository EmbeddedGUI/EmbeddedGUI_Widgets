#include "egui.h"
#include "egui_view_number_box.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define NUMBER_BOX_ROOT_WIDTH        224
#define NUMBER_BOX_ROOT_HEIGHT       154
#define NUMBER_BOX_PRIMARY_WIDTH     196
#define NUMBER_BOX_PRIMARY_HEIGHT    70
#define NUMBER_BOX_PREVIEW_WIDTH     104
#define NUMBER_BOX_PREVIEW_HEIGHT    44
#define NUMBER_BOX_BOTTOM_ROW_WIDTH  216
#define NUMBER_BOX_BOTTOM_ROW_HEIGHT 44
#define NUMBER_BOX_RECORD_WAIT       90
#define NUMBER_BOX_RECORD_FRAME_WAIT 170
#define NUMBER_BOX_RECORD_FINAL_WAIT 280
#define NUMBER_BOX_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct number_box_snapshot number_box_snapshot_t;
struct number_box_snapshot
{
    int16_t value;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_number_box_t box_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_number_box_t box_compact;
static egui_view_number_box_t box_read_only;
static egui_view_api_t box_compact_api;
static egui_view_api_t box_read_only_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Number Box";

static const number_box_snapshot_t primary_snapshots[] = {
        {24},
        {28},
        {32},
};

static const number_box_snapshot_t compact_snapshot = {12};
static const number_box_snapshot_t read_only_snapshot = {16};

static void layout_page(void);

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_number_box_set_value(EGUI_VIEW_OF(&box_primary), primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT].value);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(NUMBER_BOX_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    egui_view_number_box_set_value(EGUI_VIEW_OF(&box_compact), compact_snapshot.value);

    egui_view_number_box_set_value(EGUI_VIEW_OF(&box_read_only), read_only_snapshot.value);
    egui_view_number_box_set_read_only_mode(EGUI_VIEW_OF(&box_read_only), 1);

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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), NUMBER_BOX_ROOT_WIDTH, NUMBER_BOX_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), NUMBER_BOX_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 6, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_number_box_init(EGUI_VIEW_OF(&box_primary));
    egui_view_set_size(EGUI_VIEW_OF(&box_primary), NUMBER_BOX_PRIMARY_WIDTH, NUMBER_BOX_PRIMARY_HEIGHT);
    egui_view_number_box_set_font(EGUI_VIEW_OF(&box_primary), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_number_box_set_meta_font(EGUI_VIEW_OF(&box_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_number_box_set_label(EGUI_VIEW_OF(&box_primary), "Spacing");
    egui_view_number_box_set_suffix(EGUI_VIEW_OF(&box_primary), "px");
    egui_view_number_box_set_helper(EGUI_VIEW_OF(&box_primary), "0 to 64, step 4");
    egui_view_number_box_set_range(EGUI_VIEW_OF(&box_primary), 0, 64);
    egui_view_number_box_set_step(EGUI_VIEW_OF(&box_primary), 4);
    egui_view_number_box_set_palette(EGUI_VIEW_OF(&box_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD6DDE5), EGUI_COLOR_HEX(0x1A2734),
                                     EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_set_margin(EGUI_VIEW_OF(&box_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&box_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), NUMBER_BOX_BOTTOM_ROW_WIDTH, NUMBER_BOX_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_number_box_init(EGUI_VIEW_OF(&box_compact));
    egui_view_set_size(EGUI_VIEW_OF(&box_compact), NUMBER_BOX_PREVIEW_WIDTH, NUMBER_BOX_PREVIEW_HEIGHT);
    egui_view_number_box_set_font(EGUI_VIEW_OF(&box_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_number_box_set_meta_font(EGUI_VIEW_OF(&box_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_number_box_set_suffix(EGUI_VIEW_OF(&box_compact), "ms");
    egui_view_number_box_set_range(EGUI_VIEW_OF(&box_compact), 0, 24);
    egui_view_number_box_set_step(EGUI_VIEW_OF(&box_compact), 2);
    egui_view_number_box_set_compact_mode(EGUI_VIEW_OF(&box_compact), 1);
    egui_view_number_box_set_palette(EGUI_VIEW_OF(&box_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD8DFE6), EGUI_COLOR_HEX(0x1A2734),
                                     EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x2F76B7));
    egui_view_number_box_override_static_preview_api(EGUI_VIEW_OF(&box_compact), &box_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&box_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&box_compact));

    egui_view_number_box_init(EGUI_VIEW_OF(&box_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&box_read_only), NUMBER_BOX_PREVIEW_WIDTH, NUMBER_BOX_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&box_read_only), 8, 0, 0, 0);
    egui_view_number_box_set_font(EGUI_VIEW_OF(&box_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_number_box_set_meta_font(EGUI_VIEW_OF(&box_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_number_box_set_suffix(EGUI_VIEW_OF(&box_read_only), "px");
    egui_view_number_box_set_compact_mode(EGUI_VIEW_OF(&box_read_only), 1);
    egui_view_number_box_set_read_only_mode(EGUI_VIEW_OF(&box_read_only), 1);
    egui_view_number_box_set_palette(EGUI_VIEW_OF(&box_read_only), EGUI_COLOR_HEX(0xFCFDFE), EGUI_COLOR_HEX(0xDEE4EA), EGUI_COLOR_HEX(0x364452),
                                     EGUI_COLOR_HEX(0x7A8793), EGUI_COLOR_HEX(0x909CAA));
    egui_view_number_box_override_static_preview_api(EGUI_VIEW_OF(&box_read_only), &box_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&box_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&box_read_only));

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
        EGUI_SIM_SET_WAIT(p_action, NUMBER_BOX_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, NUMBER_BOX_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, NUMBER_BOX_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, NUMBER_BOX_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, NUMBER_BOX_RECORD_FRAME_WAIT);
        return true;
    case 5:
        EGUI_SIM_SET_WAIT(p_action, NUMBER_BOX_RECORD_FINAL_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, NUMBER_BOX_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
