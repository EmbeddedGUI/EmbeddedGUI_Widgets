#include "egui.h"
#include "egui_view_arc.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define ARC_ROOT_WIDTH        224
#define ARC_ROOT_HEIGHT       190
#define ARC_PRIMARY_SIZE      76
#define ARC_PREVIEW_SIZE      42
#define ARC_BOTTOM_ROW_WIDTH  92
#define ARC_BOTTOM_ROW_HEIGHT 42
#define ARC_RECORD_WAIT       90
#define ARC_RECORD_FRAME_WAIT 170
#define ARC_RECORD_FINAL_WAIT 280
#define ARC_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct arc_snapshot arc_snapshot_t;
struct arc_snapshot
{
    uint8_t value;
    egui_color_t track_color;
    egui_color_t active_color;
    const char *value_text;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_arc_t primary_arc;
static egui_view_label_t primary_value_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_arc_t subtle_arc;
static egui_view_arc_t attention_arc;
static egui_view_api_t subtle_arc_api;
static egui_view_api_t attention_arc_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Arc";

static const arc_snapshot_t primary_snapshots[] = {
        {
                32,
                EGUI_COLOR_HEX(0xD9E2EA),
                EGUI_COLOR_HEX(0x0F6CBD),
                "32%",
        },
        {
                58,
                EGUI_COLOR_HEX(0xE9DDC9),
                EGUI_COLOR_HEX(0xA15C00),
                "58%",
        },
        {
                86,
                EGUI_COLOR_HEX(0xD6E8DD),
                EGUI_COLOR_HEX(0x0F7B45),
                "86%",
        },
};

static void layout_page(void);

static void apply_primary_snapshot(uint8_t index)
{
    const arc_snapshot_t *snapshot = &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT];

    egui_view_arc_set_value(EGUI_VIEW_OF(&primary_arc), snapshot->value);
    egui_view_arc_set_palette(EGUI_VIEW_OF(&primary_arc), snapshot->track_color, snapshot->active_color);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_value_label), snapshot->value_text);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_value_label), snapshot->active_color, EGUI_ALPHA_100);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(ARC_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    egui_view_arc_apply_subtle_style(EGUI_VIEW_OF(&subtle_arc));
    egui_view_arc_set_value(EGUI_VIEW_OF(&subtle_arc), 24);

    egui_view_arc_apply_attention_style(EGUI_VIEW_OF(&attention_arc));
    egui_view_arc_set_value(EGUI_VIEW_OF(&attention_arc), 72);

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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), ARC_ROOT_WIDTH, ARC_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), ARC_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_arc_init(EGUI_VIEW_OF(&primary_arc));
    egui_view_set_size(EGUI_VIEW_OF(&primary_arc), ARC_PRIMARY_SIZE, ARC_PRIMARY_SIZE);
    egui_view_arc_apply_standard_style(EGUI_VIEW_OF(&primary_arc));
    egui_view_set_margin(EGUI_VIEW_OF(&primary_arc), 0, 0, 0, 8);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&primary_arc), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_arc));

    egui_view_label_init(EGUI_VIEW_OF(&primary_value_label));
    egui_view_set_size(EGUI_VIEW_OF(&primary_value_label), ARC_ROOT_WIDTH, 14);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_value_label), "32%");
    egui_view_label_set_align_type(EGUI_VIEW_OF(&primary_value_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&primary_value_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_value_label), EGUI_COLOR_HEX(0x0F6CBD), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_value_label), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_value_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), ARC_BOTTOM_ROW_WIDTH, ARC_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_arc_init(EGUI_VIEW_OF(&subtle_arc));
    egui_view_set_size(EGUI_VIEW_OF(&subtle_arc), ARC_PREVIEW_SIZE, ARC_PREVIEW_SIZE);
    egui_view_arc_apply_subtle_style(EGUI_VIEW_OF(&subtle_arc));
    egui_view_arc_override_static_preview_api(EGUI_VIEW_OF(&subtle_arc), &subtle_arc_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&subtle_arc), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&subtle_arc));

    egui_view_arc_init(EGUI_VIEW_OF(&attention_arc));
    egui_view_set_size(EGUI_VIEW_OF(&attention_arc), ARC_PREVIEW_SIZE, ARC_PREVIEW_SIZE);
    egui_view_set_margin(EGUI_VIEW_OF(&attention_arc), 8, 0, 0, 0);
    egui_view_arc_apply_attention_style(EGUI_VIEW_OF(&attention_arc));
    egui_view_arc_override_static_preview_api(EGUI_VIEW_OF(&attention_arc), &attention_arc_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&attention_arc), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&attention_arc));

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
        EGUI_SIM_SET_WAIT(p_action, ARC_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, ARC_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, ARC_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, ARC_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, ARC_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_default_state();
        }
        EGUI_SIM_SET_WAIT(p_action, ARC_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, ARC_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
