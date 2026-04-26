#include "egui.h"
#include "egui_view_two_pane_view.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define TWO_PANE_VIEW_ROOT_W           224
#define TWO_PANE_VIEW_ROOT_H           216
#define TWO_PANE_VIEW_PRIMARY_W        196
#define TWO_PANE_VIEW_PRIMARY_H        106
#define TWO_PANE_VIEW_PREVIEW_W        104
#define TWO_PANE_VIEW_PREVIEW_H        74
#define TWO_PANE_VIEW_BOTTOM_W         216
#define TWO_PANE_VIEW_BOTTOM_H         74
#define TWO_PANE_VIEW_RECORD_WAIT      90
#define TWO_PANE_VIEW_RECORD_FRAME_WAIT 170
#define TWO_PANE_VIEW_RECORD_FINAL_WAIT 280
#define TWO_PANE_VIEW_DEFAULT_STATE    0

typedef struct
{
    uint8_t layout_mode;
    uint8_t single_pane;
} two_pane_view_state_t;

#define PRIMARY_STATE_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_states))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_two_pane_view_t panel_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_column;
static egui_view_two_pane_view_t panel_compact;
static egui_view_linearlayout_t read_only_column;
static egui_view_two_pane_view_t panel_read_only;
static egui_view_api_t panel_compact_api;
static egui_view_api_t panel_read_only_api;
static uint8_t ui_ready;

static void layout_page(void);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Two Pane View";

static const two_pane_view_state_t primary_states[] = {
        {EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_WIDE, EGUI_VIEW_TWO_PANE_VIEW_PANE_FIRST},
        {EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_TALL, EGUI_VIEW_TWO_PANE_VIEW_PANE_FIRST},
        {EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_SINGLE, EGUI_VIEW_TWO_PANE_VIEW_PANE_SECOND},
        {EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_WIDE, EGUI_VIEW_TWO_PANE_VIEW_PANE_SECOND},
};

static const egui_view_two_pane_view_pane_t primary_first_pane = {
        "Primary pane",
        "Inbox timeline",
        "Wide / Tall",
        "Messages and filters stay visible",
        "The left pane keeps the scan path stable",
        "Open",
        EGUI_VIEW_TWO_PANE_VIEW_TONE_ACCENT,
        1,
};

static const egui_view_two_pane_view_pane_t primary_second_pane = {
        "Secondary pane",
        "Reading surface",
        "Single pane ready",
        "Selected content can take the full area",
        "The second pane keeps context nearby",
        "Read",
        EGUI_VIEW_TWO_PANE_VIEW_TONE_SUCCESS,
        0,
};

static const egui_view_two_pane_view_pane_t compact_first_pane = {
        "Pane 1",
        "Folders",
        "Compact",
        "Navigation stays brief",
        "Second pane can be promoted",
        "P1",
        EGUI_VIEW_TWO_PANE_VIEW_TONE_ACCENT,
        1,
};

static const egui_view_two_pane_view_pane_t compact_second_pane = {
        "Pane 2",
        "Preview",
        "Compact",
        "Content uses the single slot",
        "Priority remains explicit",
        "P2",
        EGUI_VIEW_TWO_PANE_VIEW_TONE_WARNING,
        0,
};

static const egui_view_two_pane_view_pane_t read_only_first_pane = {
        "Pane 1",
        "Roster",
        "Read only",
        "Names stay visible",
        "No touch edits allowed",
        "View",
        EGUI_VIEW_TWO_PANE_VIEW_TONE_NEUTRAL,
        0,
};

static const egui_view_two_pane_view_pane_t read_only_second_pane = {
        "Pane 2",
        "Details",
        "Read only",
        "Selection remains fixed",
        "State can be mirrored safely",
        "View",
        EGUI_VIEW_TWO_PANE_VIEW_TONE_SUCCESS,
        0,
};

static void apply_primary_state(uint8_t index)
{
    const two_pane_view_state_t *state = &primary_states[index % PRIMARY_STATE_COUNT];

    egui_view_two_pane_view_set_single_pane(EGUI_VIEW_OF(&panel_primary), state->single_pane);
    egui_view_two_pane_view_set_layout_mode(EGUI_VIEW_OF(&panel_primary), state->layout_mode);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_state(TWO_PANE_VIEW_DEFAULT_STATE);
}

static void apply_preview_states(void)
{
    egui_view_two_pane_view_set_single_pane(EGUI_VIEW_OF(&panel_compact), EGUI_VIEW_TWO_PANE_VIEW_PANE_SECOND);
    egui_view_two_pane_view_set_layout_mode(EGUI_VIEW_OF(&panel_compact), EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_SINGLE);
    egui_view_two_pane_view_set_single_pane(EGUI_VIEW_OF(&panel_read_only), EGUI_VIEW_TWO_PANE_VIEW_PANE_FIRST);
    egui_view_two_pane_view_set_layout_mode(EGUI_VIEW_OF(&panel_read_only), EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_TALL);
    if (ui_ready)
    {
        layout_page();
    }
}

static void layout_local_views(void)
{
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&compact_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&read_only_column));
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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), TWO_PANE_VIEW_ROOT_W, TWO_PANE_VIEW_ROOT_H);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&title_label), TWO_PANE_VIEW_ROOT_W, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_two_pane_view_init(EGUI_VIEW_OF(&panel_primary));
    egui_view_set_size(EGUI_VIEW_OF(&panel_primary), TWO_PANE_VIEW_PRIMARY_W, TWO_PANE_VIEW_PRIMARY_H);
    egui_view_two_pane_view_set_font(EGUI_VIEW_OF(&panel_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_two_pane_view_set_meta_font(EGUI_VIEW_OF(&panel_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_two_pane_view_set_panes(EGUI_VIEW_OF(&panel_primary), &primary_first_pane, &primary_second_pane);
    egui_view_two_pane_view_set_palette(EGUI_VIEW_OF(&panel_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0xEAF0F7),
                                        EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45),
                                        EGUI_COLOR_HEX(0x9D5D00), EGUI_COLOR_HEX(0x7A8796));
    egui_view_set_margin(EGUI_VIEW_OF(&panel_primary), 0, 0, 0, 8);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&panel_primary), true);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&panel_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), TWO_PANE_VIEW_BOTTOM_W, TWO_PANE_VIEW_BOTTOM_H);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&compact_column), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&compact_column), TWO_PANE_VIEW_PREVIEW_W, TWO_PANE_VIEW_BOTTOM_H);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&compact_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&compact_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_column));

    egui_view_two_pane_view_init(EGUI_VIEW_OF(&panel_compact));
    egui_view_set_size(EGUI_VIEW_OF(&panel_compact), TWO_PANE_VIEW_PREVIEW_W, TWO_PANE_VIEW_PREVIEW_H);
    egui_view_two_pane_view_set_font(EGUI_VIEW_OF(&panel_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_two_pane_view_set_meta_font(EGUI_VIEW_OF(&panel_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_two_pane_view_set_panes(EGUI_VIEW_OF(&panel_compact), &compact_first_pane, &compact_second_pane);
    egui_view_two_pane_view_set_compact_mode(EGUI_VIEW_OF(&panel_compact), 1);
    egui_view_two_pane_view_set_palette(EGUI_VIEW_OF(&panel_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0xEAF0F7),
                                        EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45),
                                        EGUI_COLOR_HEX(0x9D5D00), EGUI_COLOR_HEX(0x7A8796));
    egui_view_two_pane_view_override_static_preview_api(EGUI_VIEW_OF(&panel_compact), &panel_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&panel_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_column), EGUI_VIEW_OF(&panel_compact));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&read_only_column), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&read_only_column), TWO_PANE_VIEW_PREVIEW_W, TWO_PANE_VIEW_BOTTOM_H);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_column), 8, 0, 0, 0);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&read_only_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&read_only_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_column));

    egui_view_two_pane_view_init(EGUI_VIEW_OF(&panel_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&panel_read_only), TWO_PANE_VIEW_PREVIEW_W, TWO_PANE_VIEW_PREVIEW_H);
    egui_view_two_pane_view_set_font(EGUI_VIEW_OF(&panel_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_two_pane_view_set_meta_font(EGUI_VIEW_OF(&panel_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_two_pane_view_set_panes(EGUI_VIEW_OF(&panel_read_only), &read_only_first_pane, &read_only_second_pane);
    egui_view_two_pane_view_set_compact_mode(EGUI_VIEW_OF(&panel_read_only), 1);
    egui_view_two_pane_view_set_read_only_mode(EGUI_VIEW_OF(&panel_read_only), 1);
    egui_view_two_pane_view_set_palette(EGUI_VIEW_OF(&panel_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xDBE2E8), EGUI_COLOR_HEX(0xF3F6F9),
                                        EGUI_COLOR_HEX(0x536474), EGUI_COLOR_HEX(0x8896A4), EGUI_COLOR_HEX(0xB3BFCA), EGUI_COLOR_HEX(0xA7BDB6),
                                        EGUI_COLOR_HEX(0xC3AE88), EGUI_COLOR_HEX(0x9AA7B3));
    egui_view_two_pane_view_override_static_preview_api(EGUI_VIEW_OF(&panel_read_only), &panel_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&panel_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_column), EGUI_VIEW_OF(&panel_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
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
            apply_primary_default_state();
            apply_preview_states();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&panel_primary));
#endif
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TWO_PANE_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_state(1);
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TWO_PANE_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_state(2);
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TWO_PANE_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_state(3);
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TWO_PANE_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&panel_primary));
#endif
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TWO_PANE_VIEW_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
