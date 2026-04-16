#include <string.h>

#include "egui.h"
#include "egui_view_color_picker.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define COLOR_PICKER_ROOT_WIDTH         224
#define COLOR_PICKER_ROOT_HEIGHT        204
#define COLOR_PICKER_PRIMARY_WIDTH      196
#define COLOR_PICKER_PRIMARY_HEIGHT     112
#define COLOR_PICKER_PREVIEW_WIDTH      104
#define COLOR_PICKER_PREVIEW_HEIGHT     52
#define COLOR_PICKER_BOTTOM_ROW_WIDTH   216
#define COLOR_PICKER_BOTTOM_ROW_HEIGHT  52
#define COLOR_PICKER_RECORD_WAIT        100
#define COLOR_PICKER_RECORD_FRAME_WAIT  170
#define COLOR_PICKER_RECORD_FINAL_WAIT  280
#define COLOR_PICKER_DEFAULT_SNAPSHOT   0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct color_picker_snapshot color_picker_snapshot_t;
struct color_picker_snapshot
{
    const char *label;
    const char *helper;
    uint8_t hue_index;
    uint8_t saturation_index;
    uint8_t value_index;
    uint8_t current_part;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_color_picker_t picker_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_column;
static egui_view_color_picker_t picker_compact;
static egui_view_linearlayout_t read_only_column;
static egui_view_color_picker_t picker_read_only;
static egui_view_api_t picker_compact_api;
static egui_view_api_t picker_read_only_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Color Picker";

static const color_picker_snapshot_t primary_snapshots[] = {
        {"Accent color", "Use arrows or tap the hue rail", 7, 4, 1, EGUI_VIEW_COLOR_PICKER_PART_PALETTE},
        {"Signal color", "Warm families surface alerts", 1, 5, 1, EGUI_VIEW_COLOR_PICKER_PART_PALETTE},
        {"Surface tint", "Muted tones keep cards calm", 4, 3, 2, EGUI_VIEW_COLOR_PICKER_PART_PALETTE},
};

static const color_picker_snapshot_t compact_snapshot = {NULL, NULL, 5, 4, 1, EGUI_VIEW_COLOR_PICKER_PART_PALETTE};
static const color_picker_snapshot_t read_only_snapshot = {NULL, NULL, 8, 2, 2, EGUI_VIEW_COLOR_PICKER_PART_PALETTE};

static void layout_page(void);

static void apply_snapshot(egui_view_t *view, const color_picker_snapshot_t *snapshot)
{
    egui_view_color_picker_set_label(view, snapshot->label);
    egui_view_color_picker_set_helper(view, snapshot->helper);
    egui_view_color_picker_set_selection(view, snapshot->hue_index, snapshot->saturation_index, snapshot->value_index);
    egui_view_color_picker_set_current_part(view, snapshot->current_part);
}

static void apply_primary_snapshot(uint8_t index)
{
    apply_snapshot(EGUI_VIEW_OF(&picker_primary), &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT]);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(COLOR_PICKER_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    apply_snapshot(EGUI_VIEW_OF(&picker_compact), &compact_snapshot);
    apply_snapshot(EGUI_VIEW_OF(&picker_read_only), &read_only_snapshot);
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
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}

static void focus_primary_picker(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&picker_primary));
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
    ui_ready = 0;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), COLOR_PICKER_ROOT_WIDTH, COLOR_PICKER_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), COLOR_PICKER_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_color_picker_init(EGUI_VIEW_OF(&picker_primary));
    egui_view_set_size(EGUI_VIEW_OF(&picker_primary), COLOR_PICKER_PRIMARY_WIDTH, COLOR_PICKER_PRIMARY_HEIGHT);
    egui_view_color_picker_set_font(EGUI_VIEW_OF(&picker_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_color_picker_set_meta_font(EGUI_VIEW_OF(&picker_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_color_picker_set_palette(EGUI_VIEW_OF(&picker_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                       EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_set_margin(EGUI_VIEW_OF(&picker_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&picker_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), COLOR_PICKER_BOTTOM_ROW_WIDTH, COLOR_PICKER_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&compact_column));
    egui_view_set_size(EGUI_VIEW_OF(&compact_column), COLOR_PICKER_PREVIEW_WIDTH, COLOR_PICKER_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&compact_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&compact_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_column));

    egui_view_color_picker_init(EGUI_VIEW_OF(&picker_compact));
    egui_view_set_size(EGUI_VIEW_OF(&picker_compact), COLOR_PICKER_PREVIEW_WIDTH, COLOR_PICKER_PREVIEW_HEIGHT);
    egui_view_color_picker_set_font(EGUI_VIEW_OF(&picker_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_color_picker_set_meta_font(EGUI_VIEW_OF(&picker_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_color_picker_set_compact_mode(EGUI_VIEW_OF(&picker_compact), 1);
    egui_view_color_picker_set_palette(EGUI_VIEW_OF(&picker_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                       EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_color_picker_override_static_preview_api(EGUI_VIEW_OF(&picker_compact), &picker_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&picker_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_column), EGUI_VIEW_OF(&picker_compact));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&read_only_column));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_column), COLOR_PICKER_PREVIEW_WIDTH, COLOR_PICKER_BOTTOM_ROW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_column), 8, 0, 0, 0);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&read_only_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&read_only_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_column));

    egui_view_color_picker_init(EGUI_VIEW_OF(&picker_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&picker_read_only), COLOR_PICKER_PREVIEW_WIDTH, COLOR_PICKER_PREVIEW_HEIGHT);
    egui_view_color_picker_set_font(EGUI_VIEW_OF(&picker_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_color_picker_set_meta_font(EGUI_VIEW_OF(&picker_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_color_picker_set_compact_mode(EGUI_VIEW_OF(&picker_read_only), 1);
    egui_view_color_picker_set_read_only_mode(EGUI_VIEW_OF(&picker_read_only), 1);
    egui_view_color_picker_set_palette(EGUI_VIEW_OF(&picker_read_only), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                       EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x7A8796));
    egui_view_color_picker_override_static_preview_api(EGUI_VIEW_OF(&picker_read_only), &picker_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&picker_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_column), EGUI_VIEW_OF(&picker_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    layout_page();
    focus_primary_picker();
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event = {0};

    focus_primary_picker();
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    egui_view_dispatch_key_event(EGUI_VIEW_OF(&picker_primary), &event);

    event.type = EGUI_KEY_EVENT_ACTION_UP;
    egui_view_dispatch_key_event(EGUI_VIEW_OF(&picker_primary), &event);
    if (ui_ready)
    {
        layout_page();
    }
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
            apply_primary_default_state();
            apply_preview_states();
            focus_primary_picker();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COLOR_PICKER_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_RIGHT);
        }
        EGUI_SIM_SET_WAIT(p_action, COLOR_PICKER_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COLOR_PICKER_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_TAB);
            apply_primary_key(EGUI_KEY_CODE_DOWN);
        }
        EGUI_SIM_SET_WAIT(p_action, COLOR_PICKER_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COLOR_PICKER_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_snapshot(1);
            focus_primary_picker();
        }
        EGUI_SIM_SET_WAIT(p_action, COLOR_PICKER_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COLOR_PICKER_RECORD_FRAME_WAIT);
        return true;
    case 7:
        EGUI_SIM_SET_WAIT(p_action, COLOR_PICKER_RECORD_FINAL_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COLOR_PICKER_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
