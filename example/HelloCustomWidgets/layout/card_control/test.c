#include "egui.h"
#include "egui_view_card_control.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define CARD_CONTROL_ROOT_WIDTH        224
#define CARD_CONTROL_ROOT_HEIGHT       236
#define CARD_CONTROL_PRIMARY_WIDTH     196
#define CARD_CONTROL_PRIMARY_HEIGHT    102
#define CARD_CONTROL_PREVIEW_WIDTH     104
#define CARD_CONTROL_PREVIEW_HEIGHT    74
#define CARD_CONTROL_BOTTOM_ROW_WIDTH  216
#define CARD_CONTROL_BOTTOM_ROW_HEIGHT 74
#define CARD_CONTROL_RECORD_WAIT       90
#define CARD_CONTROL_RECORD_FRAME_WAIT 180

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)(sizeof(primary_snapshots) / sizeof(primary_snapshots[0])))
#define COMPACT_SNAPSHOT_COUNT ((uint8_t)(sizeof(compact_snapshots) / sizeof(compact_snapshots[0])))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_card_control_t card_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_card_control_t card_compact;
static egui_view_card_control_t card_read_only;
static egui_view_api_t card_compact_api;
static egui_view_api_t card_read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Card Control";

static const egui_view_card_control_snapshot_t primary_snapshots[] = {
        {"SYNC", "WF", "Workspace flow", "Three review stages stay pinned inside one card.", "Live", "Quick actions stay visible on the right.",
         EGUI_VIEW_CARD_CONTROL_TONE_ACCENT, 1, EGUI_VIEW_CARD_CONTROL_CONTROL_VALUE},
        {"ACCESS", "ID", "Identity checks", "Approval routing stays close to the main content.", NULL, "Switch affordance remains lightweight.",
         EGUI_VIEW_CARD_CONTROL_TONE_SUCCESS, 0, EGUI_VIEW_CARD_CONTROL_CONTROL_SWITCH_ON},
        {"DEPLOY", "UP", "Release approval", "Staged publish exposes a chevron affordance for deeper actions.", NULL,
         "Chevron entry routes to the rollout detail page.", EGUI_VIEW_CARD_CONTROL_TONE_WARNING, 1,
         EGUI_VIEW_CARD_CONTROL_CONTROL_CHEVRON},
};

static const egui_view_card_control_snapshot_t compact_snapshots[] = {
        {"SYNC", "WF", "Compact flow", "Short body.", "Live", "", EGUI_VIEW_CARD_CONTROL_TONE_ACCENT, 1, EGUI_VIEW_CARD_CONTROL_CONTROL_VALUE},
        {"WARN", "UP", "Compact gate", "Short body.", NULL, "", EGUI_VIEW_CARD_CONTROL_TONE_WARNING, 1, EGUI_VIEW_CARD_CONTROL_CONTROL_SWITCH_OFF},
};

static const egui_view_card_control_snapshot_t read_only_snapshot = {
        "REVIEW", "CK", "Read only card", "Editing stays locked while review is active.", "Locked", "Preview only.",
        EGUI_VIEW_CARD_CONTROL_TONE_NEUTRAL, 0, EGUI_VIEW_CARD_CONTROL_CONTROL_VALUE};

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_card_control_set_current_snapshot(EGUI_VIEW_OF(&card_primary), index % PRIMARY_SNAPSHOT_COUNT);
}

static void apply_compact_snapshot(uint8_t index)
{
    egui_view_card_control_set_current_snapshot(EGUI_VIEW_OF(&card_compact), index % COMPACT_SNAPSHOT_COUNT);
}

static int dismiss_primary_focus_on_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (event->type == EGUI_MOTION_EVENT_ACTION_DOWN)
    {
        egui_view_clear_focus(EGUI_VIEW_OF(&card_primary));
    }
#else
    EGUI_UNUSED(event);
#endif
    return 1;
}

#if EGUI_CONFIG_RECORDING_TEST
static void set_click_card_part(egui_sim_action_t *p_action, egui_view_t *view, int interval_ms)
{
    egui_region_t region;

    if (!egui_view_card_control_get_part_region(view, EGUI_VIEW_CARD_CONTROL_PART_CARD, &region))
    {
        EGUI_SIM_SET_WAIT(p_action, interval_ms);
        return;
    }

    p_action->type = EGUI_SIM_ACTION_CLICK;
    p_action->x1 = region.location.x + region.size.width / 2;
    p_action->y1 = region.location.y + region.size.height / 2;
    p_action->x2 = 0;
    p_action->y2 = 0;
    p_action->steps = 0;
    p_action->interval_ms = interval_ms;
}

static void request_page_snapshot(void)
{
    egui_view_invalidate(EGUI_VIEW_OF(&root_layout));
    recording_request_snapshot();
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), CARD_CONTROL_ROOT_WIDTH, CARD_CONTROL_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), CARD_CONTROL_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_card_control_init(EGUI_VIEW_OF(&card_primary));
    egui_view_set_size(EGUI_VIEW_OF(&card_primary), CARD_CONTROL_PRIMARY_WIDTH, CARD_CONTROL_PRIMARY_HEIGHT);
    egui_view_card_control_set_snapshots(EGUI_VIEW_OF(&card_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    egui_view_card_control_set_font(EGUI_VIEW_OF(&card_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_card_control_set_meta_font(EGUI_VIEW_OF(&card_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_card_control_set_palette(EGUI_VIEW_OF(&card_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xF7F9FC), EGUI_COLOR_HEX(0xD2DBE3),
                                       EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45),
                                       EGUI_COLOR_HEX(0x9D5D00), EGUI_COLOR_HEX(0x7A8796));
    egui_view_set_margin(EGUI_VIEW_OF(&card_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&card_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), CARD_CONTROL_BOTTOM_ROW_WIDTH, CARD_CONTROL_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_card_control_init(EGUI_VIEW_OF(&card_compact));
    egui_view_set_size(EGUI_VIEW_OF(&card_compact), CARD_CONTROL_PREVIEW_WIDTH, CARD_CONTROL_PREVIEW_HEIGHT);
    egui_view_card_control_set_snapshots(EGUI_VIEW_OF(&card_compact), compact_snapshots, COMPACT_SNAPSHOT_COUNT);
    egui_view_card_control_set_font(EGUI_VIEW_OF(&card_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_card_control_set_meta_font(EGUI_VIEW_OF(&card_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_card_control_set_compact_mode(EGUI_VIEW_OF(&card_compact), 1);
    egui_view_card_control_override_static_preview_api(EGUI_VIEW_OF(&card_compact), &card_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    card_compact_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&card_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&card_compact));

    egui_view_card_control_init(EGUI_VIEW_OF(&card_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&card_read_only), CARD_CONTROL_PREVIEW_WIDTH, CARD_CONTROL_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&card_read_only), 8, 0, 0, 0);
    egui_view_card_control_set_snapshots(EGUI_VIEW_OF(&card_read_only), &read_only_snapshot, 1);
    egui_view_card_control_set_font(EGUI_VIEW_OF(&card_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_card_control_set_meta_font(EGUI_VIEW_OF(&card_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_card_control_set_compact_mode(EGUI_VIEW_OF(&card_read_only), 1);
    egui_view_card_control_set_read_only_mode(EGUI_VIEW_OF(&card_read_only), 1);
    egui_view_card_control_set_palette(EGUI_VIEW_OF(&card_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xF8FAFB), EGUI_COLOR_HEX(0xD8DFE6),
                                       EGUI_COLOR_HEX(0x233241), EGUI_COLOR_HEX(0x708091), EGUI_COLOR_HEX(0x98A5B2), EGUI_COLOR_HEX(0xA7B4BF),
                                       EGUI_COLOR_HEX(0xB8B0A2), EGUI_COLOR_HEX(0xB4BDC8));
    egui_view_card_control_override_static_preview_api(EGUI_VIEW_OF(&card_read_only), &card_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    card_read_only_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&card_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&card_read_only));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&card_primary));
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
            apply_primary_snapshot(0);
            apply_compact_snapshot(0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&card_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_CONTROL_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_CONTROL_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_CONTROL_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_compact_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_CONTROL_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 8:
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (first_call)
        {
            egui_view_request_focus(EGUI_VIEW_OF(&card_primary));
        }
#endif
        EGUI_SIM_SET_WAIT(p_action, CARD_CONTROL_RECORD_WAIT);
        return true;
    case 9:
        set_click_card_part(p_action, EGUI_VIEW_OF(&card_compact), 220);
        return true;
    case 10:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 520);
        return true;
    default:
        break;
    }

    return false;
}
#endif
