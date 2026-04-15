#include <string.h>

#include "egui.h"
#include "egui_view_token_input.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define TOKEN_INPUT_ROOT_WIDTH        224
#define TOKEN_INPUT_ROOT_HEIGHT       180
#define TOKEN_INPUT_PRIMARY_WIDTH     196
#define TOKEN_INPUT_PRIMARY_HEIGHT    92
#define TOKEN_INPUT_PREVIEW_WIDTH     104
#define TOKEN_INPUT_PREVIEW_HEIGHT    48
#define TOKEN_INPUT_BOTTOM_ROW_WIDTH  216
#define TOKEN_INPUT_BOTTOM_ROW_HEIGHT 48
#define TOKEN_INPUT_RECORD_WAIT       90
#define TOKEN_INPUT_RECORD_FRAME_WAIT 170
#define TOKEN_INPUT_RECORD_FINAL_WAIT 280
#define TOKEN_INPUT_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

#define TOKEN_INPUT_STANDARD_BORDER 0xD5DCE4
#define TOKEN_INPUT_STANDARD_TEXT   0x1A2734
#define TOKEN_INPUT_STANDARD_MUTED  0x6B7A89
#define TOKEN_INPUT_STANDARD_ACCENT 0x0F6CBD
#define TOKEN_INPUT_STANDARD_SHADOW 0xEAF0F6

#define TOKEN_INPUT_READ_ONLY_TEXT   0x6B7A89
#define TOKEN_INPUT_READ_ONLY_MUTED  0x7A8796
#define TOKEN_INPUT_READ_ONLY_ACCENT 0x7A8796
#define TOKEN_INPUT_READ_ONLY_SHADOW 0xEEF3F8

typedef struct token_input_snapshot token_input_snapshot_t;
struct token_input_snapshot
{
    const char *placeholder;
    const char **tokens;
    uint8_t token_count;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_token_input_t editor_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_token_input_t editor_compact;
static egui_view_token_input_t editor_read_only;
static egui_view_api_t editor_compact_api;
static egui_view_api_t editor_read_only_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Token Input";

static const char *primary_tokens_0[] = {"Alice", "Ops", "QA"};
static const char *primary_tokens_1[] = {"Design", "Demo", "VIP", "Build"};
static const char *primary_tokens_2[] = {"Audit", "Mail", "Sync"};
static const char *compact_tokens[] = {"UI", "QA", "OPS", "SYS", "NET"};
static const char *read_only_tokens[] = {"Audit", "Ops", "QA", "Net", "Sys"};

static const token_input_snapshot_t primary_snapshots[] = {
        {"Add person", primary_tokens_0, 3},
        {"Add tag", primary_tokens_1, 4},
        {"Add source", primary_tokens_2, 3},
};

static const token_input_snapshot_t compact_snapshot = {"Add", compact_tokens, 5};
static const token_input_snapshot_t read_only_snapshot = {NULL, read_only_tokens, 5};

static void layout_page(void);
static void focus_primary_widget(void);

static void apply_snapshot_to_editor(egui_view_t *view, const token_input_snapshot_t *snapshot)
{
    egui_view_token_input_set_placeholder(view, snapshot->placeholder);
    egui_view_token_input_set_tokens(view, snapshot->tokens, snapshot->token_count);
}

static void apply_primary_snapshot(uint8_t index)
{
    apply_snapshot_to_editor(EGUI_VIEW_OF(&editor_primary), &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT]);
    egui_view_token_input_set_current_part(EGUI_VIEW_OF(&editor_primary), EGUI_VIEW_TOKEN_INPUT_PART_INPUT);
    if (ui_ready)
    {
        layout_page();
        focus_primary_widget();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(TOKEN_INPUT_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    apply_snapshot_to_editor(EGUI_VIEW_OF(&editor_compact), &compact_snapshot);
    egui_view_token_input_set_current_part(EGUI_VIEW_OF(&editor_compact), 0);

    apply_snapshot_to_editor(EGUI_VIEW_OF(&editor_read_only), &read_only_snapshot);
    egui_view_token_input_set_current_part(EGUI_VIEW_OF(&editor_read_only), 0);

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

static void focus_primary_widget(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&editor_primary));
#endif
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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), TOKEN_INPUT_ROOT_WIDTH, TOKEN_INPUT_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), TOKEN_INPUT_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_token_input_init(EGUI_VIEW_OF(&editor_primary));
    egui_view_set_size(EGUI_VIEW_OF(&editor_primary), TOKEN_INPUT_PRIMARY_WIDTH, TOKEN_INPUT_PRIMARY_HEIGHT);
    egui_view_token_input_set_font(EGUI_VIEW_OF(&editor_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_token_input_set_meta_font(EGUI_VIEW_OF(&editor_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_token_input_set_palette(EGUI_VIEW_OF(&editor_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(TOKEN_INPUT_STANDARD_BORDER),
                                      EGUI_COLOR_HEX(TOKEN_INPUT_STANDARD_TEXT), EGUI_COLOR_HEX(TOKEN_INPUT_STANDARD_MUTED),
                                      EGUI_COLOR_HEX(TOKEN_INPUT_STANDARD_ACCENT), EGUI_COLOR_HEX(TOKEN_INPUT_STANDARD_SHADOW));
    egui_view_set_margin(EGUI_VIEW_OF(&editor_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&editor_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), TOKEN_INPUT_BOTTOM_ROW_WIDTH, TOKEN_INPUT_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_token_input_init(EGUI_VIEW_OF(&editor_compact));
    egui_view_set_size(EGUI_VIEW_OF(&editor_compact), TOKEN_INPUT_PREVIEW_WIDTH, TOKEN_INPUT_PREVIEW_HEIGHT);
    egui_view_token_input_set_font(EGUI_VIEW_OF(&editor_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_token_input_set_meta_font(EGUI_VIEW_OF(&editor_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_token_input_set_compact_mode(EGUI_VIEW_OF(&editor_compact), 1);
    egui_view_token_input_set_palette(EGUI_VIEW_OF(&editor_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(TOKEN_INPUT_STANDARD_BORDER),
                                      EGUI_COLOR_HEX(TOKEN_INPUT_STANDARD_TEXT), EGUI_COLOR_HEX(TOKEN_INPUT_STANDARD_MUTED),
                                      EGUI_COLOR_HEX(TOKEN_INPUT_STANDARD_ACCENT), EGUI_COLOR_HEX(TOKEN_INPUT_STANDARD_SHADOW));
    egui_view_token_input_override_static_preview_api(EGUI_VIEW_OF(&editor_compact), &editor_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&editor_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&editor_compact));

    egui_view_token_input_init(EGUI_VIEW_OF(&editor_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&editor_read_only), TOKEN_INPUT_PREVIEW_WIDTH, TOKEN_INPUT_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&editor_read_only), 8, 0, 0, 0);
    egui_view_token_input_set_font(EGUI_VIEW_OF(&editor_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_token_input_set_meta_font(EGUI_VIEW_OF(&editor_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_token_input_set_compact_mode(EGUI_VIEW_OF(&editor_read_only), 1);
    egui_view_token_input_set_read_only_mode(EGUI_VIEW_OF(&editor_read_only), 1);
    egui_view_token_input_set_palette(EGUI_VIEW_OF(&editor_read_only), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(TOKEN_INPUT_STANDARD_BORDER),
                                      EGUI_COLOR_HEX(TOKEN_INPUT_READ_ONLY_TEXT), EGUI_COLOR_HEX(TOKEN_INPUT_READ_ONLY_MUTED),
                                      EGUI_COLOR_HEX(TOKEN_INPUT_READ_ONLY_ACCENT), EGUI_COLOR_HEX(TOKEN_INPUT_READ_ONLY_SHADOW));
    egui_view_token_input_override_static_preview_api(EGUI_VIEW_OF(&editor_read_only), &editor_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&editor_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&editor_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    layout_page();
    focus_primary_widget();
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
            focus_primary_widget();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TOKEN_INPUT_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, TOKEN_INPUT_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TOKEN_INPUT_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, TOKEN_INPUT_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TOKEN_INPUT_RECORD_FRAME_WAIT);
        return true;
    case 5:
        EGUI_SIM_SET_WAIT(p_action, TOKEN_INPUT_RECORD_FINAL_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TOKEN_INPUT_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
