#include <string.h>

#include "egui.h"
#include "egui_view_password_box.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define PASSWORD_BOX_ROOT_WIDTH        224
#define PASSWORD_BOX_ROOT_HEIGHT       154
#define PASSWORD_BOX_PRIMARY_WIDTH     196
#define PASSWORD_BOX_PRIMARY_HEIGHT    70
#define PASSWORD_BOX_PREVIEW_WIDTH     106
#define PASSWORD_BOX_PREVIEW_HEIGHT    44
#define PASSWORD_BOX_BOTTOM_ROW_WIDTH  216
#define PASSWORD_BOX_BOTTOM_ROW_HEIGHT 44
#define PASSWORD_BOX_RECORD_WAIT       90
#define PASSWORD_BOX_RECORD_FRAME_WAIT 170

typedef struct password_box_snapshot password_box_snapshot_t;
struct password_box_snapshot
{
    const char *label;
    const char *helper;
    const char *placeholder;
    const char *text;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_password_box_t box_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_column;
static egui_view_password_box_t box_compact;
static egui_view_linearlayout_t read_only_column;
static egui_view_password_box_t box_read_only;

static uint8_t primary_snapshot_index = 0;
static uint8_t compact_snapshot_index = 0;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Password Box";

static const password_box_snapshot_t primary_snapshots[] = {
        {"Wi-Fi passphrase", "Use 8 to 32 chars", "Enter password", "studio-24"},
        {"Deploy secret", "Keep this hidden on shared screens", "Secret", "release-key-7"},
};

static const password_box_snapshot_t compact_snapshots[] = {
        {NULL, NULL, "Quick PIN", "7429"},
        {NULL, NULL, "Quick PIN", "A-1709"},
};

static int consume_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);
    EGUI_UNUSED(event);
    return 1;
}

static void apply_snapshot(egui_view_t *view, const password_box_snapshot_t *snapshot)
{
    egui_view_password_box_set_label(view, snapshot->label);
    egui_view_password_box_set_helper(view, snapshot->helper);
    egui_view_password_box_set_placeholder(view, snapshot->placeholder);
    egui_view_password_box_set_text(view, snapshot->text);
    egui_view_password_box_set_revealed(view, 0);
    egui_view_password_box_set_current_part(view, EGUI_VIEW_PASSWORD_BOX_PART_FIELD);
}

static void apply_primary_snapshot(uint8_t index)
{
    primary_snapshot_index = index % EGUI_ARRAY_SIZE(primary_snapshots);
    apply_snapshot(EGUI_VIEW_OF(&box_primary), &primary_snapshots[primary_snapshot_index]);
}

static void apply_compact_snapshot(uint8_t index)
{
    compact_snapshot_index = index % EGUI_ARRAY_SIZE(compact_snapshots);
    apply_snapshot(EGUI_VIEW_OF(&box_compact), &compact_snapshots[compact_snapshot_index]);
}

static void apply_read_only_state(void)
{
    egui_view_password_box_set_label(EGUI_VIEW_OF(&box_read_only), NULL);
    egui_view_password_box_set_helper(EGUI_VIEW_OF(&box_read_only), NULL);
    egui_view_password_box_set_placeholder(EGUI_VIEW_OF(&box_read_only), "Read only");
    egui_view_password_box_set_text(EGUI_VIEW_OF(&box_read_only), "fleet-admin");
    egui_view_password_box_set_revealed(EGUI_VIEW_OF(&box_read_only), 0);
    egui_view_password_box_set_current_part(EGUI_VIEW_OF(&box_read_only), EGUI_VIEW_PASSWORD_BOX_PART_FIELD);
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), PASSWORD_BOX_ROOT_WIDTH, PASSWORD_BOX_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), PASSWORD_BOX_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_password_box_init(EGUI_VIEW_OF(&box_primary));
    egui_view_set_size(EGUI_VIEW_OF(&box_primary), PASSWORD_BOX_PRIMARY_WIDTH, PASSWORD_BOX_PRIMARY_HEIGHT);
    egui_view_password_box_set_font(EGUI_VIEW_OF(&box_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_password_box_set_meta_font(EGUI_VIEW_OF(&box_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_password_box_set_palette(EGUI_VIEW_OF(&box_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                       EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_set_margin(EGUI_VIEW_OF(&box_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&box_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), PASSWORD_BOX_BOTTOM_ROW_WIDTH, PASSWORD_BOX_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&compact_column));
    egui_view_set_size(EGUI_VIEW_OF(&compact_column), PASSWORD_BOX_PREVIEW_WIDTH, PASSWORD_BOX_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&compact_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&compact_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_column));

    egui_view_password_box_init(EGUI_VIEW_OF(&box_compact));
    egui_view_set_size(EGUI_VIEW_OF(&box_compact), PASSWORD_BOX_PREVIEW_WIDTH, PASSWORD_BOX_PREVIEW_HEIGHT);
    egui_view_password_box_set_font(EGUI_VIEW_OF(&box_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_password_box_set_meta_font(EGUI_VIEW_OF(&box_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_password_box_set_compact_mode(EGUI_VIEW_OF(&box_compact), 1);
    egui_view_password_box_set_palette(EGUI_VIEW_OF(&box_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                       EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD));
    static egui_view_api_t box_compact_touch_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&box_compact), &box_compact_touch_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&box_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_column), EGUI_VIEW_OF(&box_compact));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&read_only_column));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_column), PASSWORD_BOX_PREVIEW_WIDTH, PASSWORD_BOX_BOTTOM_ROW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_column), 4, 0, 0, 0);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&read_only_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&read_only_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_column));

    egui_view_password_box_init(EGUI_VIEW_OF(&box_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&box_read_only), PASSWORD_BOX_PREVIEW_WIDTH, PASSWORD_BOX_PREVIEW_HEIGHT);
    egui_view_password_box_set_font(EGUI_VIEW_OF(&box_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_password_box_set_meta_font(EGUI_VIEW_OF(&box_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_password_box_set_compact_mode(EGUI_VIEW_OF(&box_read_only), 1);
    egui_view_password_box_set_read_only_mode(EGUI_VIEW_OF(&box_read_only), 1);
    egui_view_password_box_set_palette(EGUI_VIEW_OF(&box_read_only), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                       EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x7A8796));
    static egui_view_api_t box_read_only_touch_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&box_read_only), &box_read_only_touch_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&box_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_column), EGUI_VIEW_OF(&box_read_only));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);
    apply_read_only_state();

    {
        hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);
    }

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&compact_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&read_only_column));
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
            apply_primary_snapshot(0);
            apply_compact_snapshot(0);
            apply_read_only_state();
        }
        EGUI_SIM_SET_WAIT(p_action, PASSWORD_BOX_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PASSWORD_BOX_RECORD_FRAME_WAIT);
        return true;
    case 2:
        EGUI_SIM_SET_CLICK_VIEW(p_action, &box_primary, PASSWORD_BOX_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_BACKSPACE, 0);
        }
        EGUI_SIM_SET_WAIT(p_action, PASSWORD_BOX_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_2, 0);
        }
        EGUI_SIM_SET_WAIT(p_action, PASSWORD_BOX_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PASSWORD_BOX_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            egui_view_password_box_set_revealed(EGUI_VIEW_OF(&box_primary), 1);
        }
        EGUI_SIM_SET_WAIT(p_action, PASSWORD_BOX_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PASSWORD_BOX_RECORD_FRAME_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_TAB, 0);
        }
        EGUI_SIM_SET_WAIT(p_action, PASSWORD_BOX_RECORD_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_SPACE, 0);
        }
        EGUI_SIM_SET_WAIT(p_action, PASSWORD_BOX_RECORD_WAIT);
        return true;
    case 10:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PASSWORD_BOX_RECORD_FRAME_WAIT);
        return true;
    case 11:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, PASSWORD_BOX_RECORD_WAIT);
        return true;
    case 12:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PASSWORD_BOX_RECORD_FRAME_WAIT);
        return true;
    case 13:
        if (first_call)
        {
            apply_compact_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, PASSWORD_BOX_RECORD_WAIT);
        return true;
    case 14:
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
