#include "egui.h"
#include "egui_view_parallax_view.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define PARALLAX_ROOT_WIDTH      224
#define PARALLAX_ROOT_HEIGHT     304
#define PARALLAX_PRIMARY_WIDTH   194
#define PARALLAX_PRIMARY_HEIGHT  136
#define PARALLAX_PREVIEW_WIDTH   106
#define PARALLAX_PREVIEW_HEIGHT  82
#define PARALLAX_BOTTOM_WIDTH    218
#define PARALLAX_BOTTOM_HEIGHT   82
#define PARALLAX_RECORD_WAIT     90
#define PARALLAX_RECORD_FRAME_WAIT 170
#define PARALLAX_RECORD_FINAL_WAIT 280
#define PARALLAX_DEFAULT_SNAPSHOT 0
#define PARALLAX_COMPACT_PREVIEW_SNAPSHOT 1
#define PARALLAX_READ_ONLY_PREVIEW_SNAPSHOT 1

#define PRIMARY_SNAPSHOT_COUNT   ((uint8_t)EGUI_ARRAY_SIZE(primary_rows))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_parallax_view_t parallax_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_parallax_view_t parallax_compact;
static egui_view_parallax_view_t parallax_read_only;
static egui_view_api_t parallax_compact_api;
static egui_view_api_t parallax_read_only_api;
static uint8_t ui_ready;

static void layout_page(void);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Parallax View";

static const egui_view_parallax_view_row_t primary_rows[] = {
        {"Hero Banner", "Top", 0, EGUI_VIEW_PARALLAX_VIEW_TONE_ACCENT},
        {"Pinned Deck", "Mid", 180, EGUI_VIEW_PARALLAX_VIEW_TONE_SUCCESS},
        {"Quiet Layer", "Hold", 360, EGUI_VIEW_PARALLAX_VIEW_TONE_NEUTRAL},
        {"System Cards", "Tail", 560, EGUI_VIEW_PARALLAX_VIEW_TONE_WARNING},
};

static const egui_view_parallax_view_row_t compact_rows[] = {
        {"Front Rail", "Top", 0, EGUI_VIEW_PARALLAX_VIEW_TONE_ACCENT},
        {"Depth Strip", "Mid", 180, EGUI_VIEW_PARALLAX_VIEW_TONE_SUCCESS},
        {"Quiet Stack", "Tail", 360, EGUI_VIEW_PARALLAX_VIEW_TONE_NEUTRAL},
};

static const egui_view_parallax_view_row_t read_only_rows[] = {
        {"Audit Layer", "Fixed", 0, EGUI_VIEW_PARALLAX_VIEW_TONE_NEUTRAL},
        {"Review Shelf", "Lock", 180, EGUI_VIEW_PARALLAX_VIEW_TONE_WARNING},
        {"Pinned Notes", "Still", 320, EGUI_VIEW_PARALLAX_VIEW_TONE_SUCCESS},
};

static const egui_view_parallax_view_row_t *get_row(const egui_view_parallax_view_row_t *rows, uint8_t count, uint8_t index)
{
    if (rows == NULL || index >= count)
    {
        return NULL;
    }
    return &rows[index];
}

static void apply_primary_state(uint8_t index)
{
    const egui_view_parallax_view_row_t *row = get_row(primary_rows, PRIMARY_SNAPSHOT_COUNT, index);

    if (row == NULL)
    {
        return;
    }

    egui_view_parallax_view_set_offset(EGUI_VIEW_OF(&parallax_primary), row->anchor_offset);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_state(PARALLAX_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    const egui_view_parallax_view_row_t *compact_row = get_row(compact_rows, EGUI_ARRAY_SIZE(compact_rows), PARALLAX_COMPACT_PREVIEW_SNAPSHOT);
    const egui_view_parallax_view_row_t *read_only_row = get_row(read_only_rows, EGUI_ARRAY_SIZE(read_only_rows), PARALLAX_READ_ONLY_PREVIEW_SNAPSHOT);

    if (compact_row != NULL)
    {
        egui_view_parallax_view_set_offset(EGUI_VIEW_OF(&parallax_compact), compact_row->anchor_offset);
    }
    if (read_only_row != NULL)
    {
        egui_view_parallax_view_set_offset(EGUI_VIEW_OF(&parallax_read_only), read_only_row->anchor_offset);
    }
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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), PARALLAX_ROOT_WIDTH, PARALLAX_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), PARALLAX_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_parallax_view_init(EGUI_VIEW_OF(&parallax_primary));
    egui_view_set_size(EGUI_VIEW_OF(&parallax_primary), PARALLAX_PRIMARY_WIDTH, PARALLAX_PRIMARY_HEIGHT);
    egui_view_parallax_view_set_font(EGUI_VIEW_OF(&parallax_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_parallax_view_set_meta_font(EGUI_VIEW_OF(&parallax_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_parallax_view_set_header(EGUI_VIEW_OF(&parallax_primary), "Parallax surface", "Hero layers move slower", "Active");
    egui_view_parallax_view_set_rows(EGUI_VIEW_OF(&parallax_primary), primary_rows, 4);
    egui_view_parallax_view_set_content_metrics(EGUI_VIEW_OF(&parallax_primary), 720, 160);
    egui_view_parallax_view_set_vertical_shift(EGUI_VIEW_OF(&parallax_primary), 16);
    egui_view_parallax_view_set_step_size(EGUI_VIEW_OF(&parallax_primary), 60, 180);
    egui_view_set_margin(EGUI_VIEW_OF(&parallax_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&parallax_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), PARALLAX_BOTTOM_WIDTH, PARALLAX_BOTTOM_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_parallax_view_init(EGUI_VIEW_OF(&parallax_compact));
    egui_view_set_size(EGUI_VIEW_OF(&parallax_compact), PARALLAX_PREVIEW_WIDTH, PARALLAX_PREVIEW_HEIGHT);
    egui_view_parallax_view_set_font(EGUI_VIEW_OF(&parallax_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_parallax_view_set_meta_font(EGUI_VIEW_OF(&parallax_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_parallax_view_set_header(EGUI_VIEW_OF(&parallax_compact), "Compact", "Reduced depth", "Preview");
    egui_view_parallax_view_set_rows(EGUI_VIEW_OF(&parallax_compact), compact_rows, 3);
    egui_view_parallax_view_set_content_metrics(EGUI_VIEW_OF(&parallax_compact), 520, 160);
    egui_view_parallax_view_set_vertical_shift(EGUI_VIEW_OF(&parallax_compact), 8);
    egui_view_parallax_view_set_step_size(EGUI_VIEW_OF(&parallax_compact), 60, 180);
    egui_view_parallax_view_set_compact_mode(EGUI_VIEW_OF(&parallax_compact), 1);
    egui_view_parallax_view_override_static_preview_api(EGUI_VIEW_OF(&parallax_compact), &parallax_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&parallax_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&parallax_compact));

    egui_view_parallax_view_init(EGUI_VIEW_OF(&parallax_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&parallax_read_only), PARALLAX_PREVIEW_WIDTH, PARALLAX_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&parallax_read_only), 6, 0, 0, 0);
    egui_view_parallax_view_set_font(EGUI_VIEW_OF(&parallax_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_parallax_view_set_meta_font(EGUI_VIEW_OF(&parallax_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_parallax_view_set_header(EGUI_VIEW_OF(&parallax_read_only), "Read only", "Depth fixed", "Archive");
    egui_view_parallax_view_set_rows(EGUI_VIEW_OF(&parallax_read_only), read_only_rows, 3);
    egui_view_parallax_view_set_content_metrics(EGUI_VIEW_OF(&parallax_read_only), 460, 160);
    egui_view_parallax_view_set_vertical_shift(EGUI_VIEW_OF(&parallax_read_only), 6);
    egui_view_parallax_view_set_step_size(EGUI_VIEW_OF(&parallax_read_only), 60, 180);
    egui_view_parallax_view_set_compact_mode(EGUI_VIEW_OF(&parallax_read_only), 1);
    egui_view_parallax_view_set_read_only_mode(EGUI_VIEW_OF(&parallax_read_only), 1);
    egui_view_parallax_view_override_static_preview_api(EGUI_VIEW_OF(&parallax_read_only), &parallax_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&parallax_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&parallax_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&parallax_primary));
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
            egui_view_request_focus(EGUI_VIEW_OF(&parallax_primary));
#endif
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PARALLAX_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_state(1);
        }
        EGUI_SIM_SET_WAIT(p_action, PARALLAX_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PARALLAX_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_state(2);
        }
        EGUI_SIM_SET_WAIT(p_action, PARALLAX_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PARALLAX_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_state(3);
        }
        EGUI_SIM_SET_WAIT(p_action, PARALLAX_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PARALLAX_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
        }
        EGUI_SIM_SET_WAIT(p_action, PARALLAX_RECORD_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PARALLAX_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
