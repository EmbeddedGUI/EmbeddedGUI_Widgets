#include <string.h>

#include "egui.h"
#include "egui_view_text_box.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define TEXT_BOX_ROOT_WIDTH        224
#define TEXT_BOX_ROOT_HEIGHT       128
#define TEXT_BOX_PRIMARY_WIDTH     196
#define TEXT_BOX_PRIMARY_HEIGHT    40
#define TEXT_BOX_PREVIEW_WIDTH     104
#define TEXT_BOX_PREVIEW_HEIGHT    32
#define TEXT_BOX_BOTTOM_ROW_WIDTH  216
#define TEXT_BOX_BOTTOM_ROW_HEIGHT 32
#define TEXT_BOX_RECORD_WAIT       90
#define TEXT_BOX_RECORD_FRAME_WAIT 170

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_textinput_t box_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_textinput_t box_compact;
static egui_view_textinput_t box_read_only;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Text Box";

static void apply_primary_state(void)
{
    hcw_text_box_set_placeholder(EGUI_VIEW_OF(&box_primary), "Display name");
    hcw_text_box_set_text(EGUI_VIEW_OF(&box_primary), "Node 01");
    hcw_text_box_set_max_length(EGUI_VIEW_OF(&box_primary), 16);
}

static void apply_compact_state(const char *text)
{
    hcw_text_box_set_placeholder(EGUI_VIEW_OF(&box_compact), "Compact");
    hcw_text_box_set_text(EGUI_VIEW_OF(&box_compact), text);
    hcw_text_box_set_max_length(EGUI_VIEW_OF(&box_compact), 16);
}

static void apply_read_only_state(void)
{
    hcw_text_box_set_placeholder(EGUI_VIEW_OF(&box_read_only), "Read only");
    hcw_text_box_set_text(EGUI_VIEW_OF(&box_read_only), "Managed");
    hcw_text_box_set_max_length(EGUI_VIEW_OF(&box_read_only), 16);
}

static void on_primary_submit(egui_view_t *self, const char *text)
{
    EGUI_UNUSED(self);
    apply_compact_state(text);
}

static void dismiss_primary_text_box(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_clear_focus(EGUI_VIEW_OF(&box_primary));
#endif
}

static int dismiss_primary_focus_on_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

    if (event->type == EGUI_MOTION_EVENT_ACTION_DOWN)
    {
        dismiss_primary_text_box();
    }
    return 0;
}

void test_init_ui(void)
{
    static egui_view_api_t primary_api;
    static egui_view_api_t compact_api;
    static egui_view_api_t read_only_api;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), TEXT_BOX_ROOT_WIDTH, TEXT_BOX_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), TEXT_BOX_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_textinput_init(EGUI_VIEW_OF(&box_primary));
    egui_view_set_size(EGUI_VIEW_OF(&box_primary), TEXT_BOX_PRIMARY_WIDTH, TEXT_BOX_PRIMARY_HEIGHT);
    hcw_text_box_set_font(EGUI_VIEW_OF(&box_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_text_box_apply_standard_style(EGUI_VIEW_OF(&box_primary));
    hcw_text_box_override_interaction_api(EGUI_VIEW_OF(&box_primary), &primary_api);
    egui_view_textinput_set_on_submit(EGUI_VIEW_OF(&box_primary), on_primary_submit);
    egui_view_set_margin(EGUI_VIEW_OF(&box_primary), 0, 0, 0, 8);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&box_primary), true);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&box_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), TEXT_BOX_BOTTOM_ROW_WIDTH, TEXT_BOX_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_textinput_init(EGUI_VIEW_OF(&box_compact));
    egui_view_set_size(EGUI_VIEW_OF(&box_compact), TEXT_BOX_PREVIEW_WIDTH, TEXT_BOX_PREVIEW_HEIGHT);
    hcw_text_box_set_font(EGUI_VIEW_OF(&box_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_text_box_apply_compact_style(EGUI_VIEW_OF(&box_compact));
    hcw_text_box_override_static_preview_api(EGUI_VIEW_OF(&box_compact), &compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    compact_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&box_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&box_compact));

    egui_view_textinput_init(EGUI_VIEW_OF(&box_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&box_read_only), TEXT_BOX_PREVIEW_WIDTH, TEXT_BOX_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&box_read_only), 8, 0, 0, 0);
    hcw_text_box_set_font(EGUI_VIEW_OF(&box_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_text_box_apply_read_only_style(EGUI_VIEW_OF(&box_read_only));
    hcw_text_box_override_static_preview_api(EGUI_VIEW_OF(&box_read_only), &read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    read_only_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&box_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&box_read_only));

    apply_primary_state();
    apply_compact_state("Queued");
    apply_read_only_state();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_primary_key(uint8_t key_code, uint8_t is_shift)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    event.is_shift = is_shift ? 1 : 0;
    EGUI_VIEW_OF(&box_primary)->api->on_key_event(EGUI_VIEW_OF(&box_primary), &event);

    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&box_primary)->api->on_key_event(EGUI_VIEW_OF(&box_primary), &event);
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
            apply_primary_state();
            apply_compact_state("Queued");
            apply_read_only_state();
            egui_view_request_focus(EGUI_VIEW_OF(&box_primary));
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TEXT_BOX_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_BACKSPACE, 0);
        }
        EGUI_SIM_SET_WAIT(p_action, TEXT_BOX_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_2, 0);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TEXT_BOX_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_ENTER, 0);
        }
        EGUI_SIM_SET_WAIT(p_action, TEXT_BOX_RECORD_WAIT);
        return true;
    case 4:
        EGUI_SIM_SET_CLICK_VIEW(p_action, &box_compact, TEXT_BOX_RECORD_WAIT);
        return true;
    case 5:
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
