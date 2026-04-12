#include "egui.h"
#include "egui_view_title_bar.h"
#include "uicode.h"
#include "demo_scaffold.h"
#include "resource/egui_icon_material_symbols.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define TITLE_BAR_ROOT_WIDTH        224
#define TITLE_BAR_ROOT_HEIGHT       214
#define TITLE_BAR_PRIMARY_WIDTH     196
#define TITLE_BAR_PRIMARY_HEIGHT    96
#define TITLE_BAR_PREVIEW_WIDTH     104
#define TITLE_BAR_PREVIEW_HEIGHT    72
#define TITLE_BAR_BOTTOM_ROW_WIDTH  216
#define TITLE_BAR_BOTTOM_ROW_HEIGHT 72
#define TITLE_BAR_RECORD_WAIT       90
#define TITLE_BAR_RECORD_FRAME_WAIT 170

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)(sizeof(primary_snapshots) / sizeof(primary_snapshots[0])))
#define COMPACT_SNAPSHOT_COUNT ((uint8_t)(sizeof(compact_snapshots) / sizeof(compact_snapshots[0])))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_title_bar_t title_bar_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_title_bar_t title_bar_compact;
static egui_view_title_bar_t title_bar_read_only;
static egui_view_api_t title_bar_compact_api;
static egui_view_api_t title_bar_read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "TitleBar";

static const egui_view_title_bar_snapshot_t primary_snapshots[] = {
        {EGUI_ICON_MS_HOME, "Workspace", "Atlas", "Weekly review", "Live", "Share", "More", 1, 1, 1},
        {EGUI_ICON_MS_SETTINGS, "Navigation", "Files", "Pane toggled", "Docked", "Review", "More", 0, 1, 0},
        {EGUI_ICON_MS_INFO, "Project", "Release", "Go-live check", "Ready", "Send", "", 1, 0, 1},
        {EGUI_ICON_MS_LOCK, "Audit", "Read back", "Frozen shell", "Read", "Done", "Reset", 1, 1, 0},
};

static const egui_view_title_bar_snapshot_t compact_snapshots[] = {
        {EGUI_ICON_MS_HOME, NULL, "Atlas", NULL, NULL, "Sync", NULL, 1, 0, 1},
        {EGUI_ICON_MS_SETTINGS, NULL, "Files", NULL, NULL, "Open", NULL, 0, 1, 0},
};

static const egui_view_title_bar_snapshot_t read_only_snapshots[] = {
        {EGUI_ICON_MS_LOCK, NULL, "Preview", NULL, NULL, "Read", NULL, 1, 0, 0},
};

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_title_bar_set_current_snapshot(EGUI_VIEW_OF(&title_bar_primary), (uint8_t)(index % PRIMARY_SNAPSHOT_COUNT));
}

static void apply_compact_snapshot(uint8_t index)
{
    egui_view_title_bar_set_current_snapshot(EGUI_VIEW_OF(&title_bar_compact), (uint8_t)(index % COMPACT_SNAPSHOT_COUNT));
}

static void apply_read_only_state(void)
{
    egui_view_title_bar_set_current_snapshot(EGUI_VIEW_OF(&title_bar_read_only), 0);
    egui_view_title_bar_set_read_only_mode(EGUI_VIEW_OF(&title_bar_read_only), 1);
}

static void on_primary_action(egui_view_t *self, uint8_t part, uint8_t snapshot_index)
{
    EGUI_UNUSED(self);
    EGUI_UNUSED(snapshot_index);

    switch (part)
    {
    case EGUI_VIEW_TITLE_BAR_PART_BACK:
        apply_primary_snapshot(1);
        break;
    case EGUI_VIEW_TITLE_BAR_PART_PANE_TOGGLE:
        apply_primary_snapshot(2);
        break;
    case EGUI_VIEW_TITLE_BAR_PART_PRIMARY_ACTION:
        apply_primary_snapshot(3);
        break;
    case EGUI_VIEW_TITLE_BAR_PART_SECONDARY_ACTION:
    default:
        apply_primary_snapshot(0);
        break;
    }
}

static void dismiss_primary_title_bar_focus(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_clear_focus(EGUI_VIEW_OF(&title_bar_primary));
#endif
}

static int dismiss_primary_focus_on_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

    if (event->type == EGUI_MOTION_EVENT_ACTION_DOWN)
    {
        dismiss_primary_title_bar_focus();
    }
    return 1;
}

#if EGUI_CONFIG_RECORDING_TEST
static void set_click_view_center(egui_sim_action_t *p_action, egui_view_t *view, int interval_ms)
{
    p_action->type = EGUI_SIM_ACTION_CLICK;
    p_action->x1 = view->region_screen.location.x + view->region_screen.size.width / 2;
    p_action->y1 = view->region_screen.location.y + view->region_screen.size.height / 2;
    p_action->x2 = 0;
    p_action->y2 = 0;
    p_action->steps = 0;
    p_action->interval_ms = interval_ms;
}

static void set_click_title_bar_part(egui_sim_action_t *p_action, egui_view_t *view, uint8_t part, int interval_ms)
{
    egui_region_t region;

    if (!egui_view_title_bar_get_part_region(view, part, &region))
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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), TITLE_BAR_ROOT_WIDTH, TITLE_BAR_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), TITLE_BAR_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_title_bar_init(EGUI_VIEW_OF(&title_bar_primary));
    egui_view_set_size(EGUI_VIEW_OF(&title_bar_primary), TITLE_BAR_PRIMARY_WIDTH, TITLE_BAR_PRIMARY_HEIGHT);
    egui_view_title_bar_set_snapshots(EGUI_VIEW_OF(&title_bar_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    egui_view_title_bar_set_font(EGUI_VIEW_OF(&title_bar_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_title_bar_set_meta_font(EGUI_VIEW_OF(&title_bar_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_title_bar_set_icon_font(EGUI_VIEW_OF(&title_bar_primary), EGUI_FONT_ICON_MS_20);
    egui_view_title_bar_set_on_action_listener(EGUI_VIEW_OF(&title_bar_primary), on_primary_action);
    egui_view_title_bar_set_palette(EGUI_VIEW_OF(&title_bar_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD4DCE4), EGUI_COLOR_HEX(0x1C2935),
                                    EGUI_COLOR_HEX(0x6D7B89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xF4F7FA), EGUI_COLOR_HEX(0xD7DFE7),
                                    EGUI_COLOR_HEX(0xDCE4EB));
    egui_view_set_margin(EGUI_VIEW_OF(&title_bar_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_bar_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), TITLE_BAR_BOTTOM_ROW_WIDTH, TITLE_BAR_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_title_bar_init(EGUI_VIEW_OF(&title_bar_compact));
    egui_view_set_size(EGUI_VIEW_OF(&title_bar_compact), TITLE_BAR_PREVIEW_WIDTH, TITLE_BAR_PREVIEW_HEIGHT);
    egui_view_title_bar_set_snapshots(EGUI_VIEW_OF(&title_bar_compact), compact_snapshots, COMPACT_SNAPSHOT_COUNT);
    egui_view_title_bar_set_font(EGUI_VIEW_OF(&title_bar_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_title_bar_set_meta_font(EGUI_VIEW_OF(&title_bar_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_title_bar_set_icon_font(EGUI_VIEW_OF(&title_bar_compact), EGUI_FONT_ICON_MS_16);
    egui_view_title_bar_set_compact_mode(EGUI_VIEW_OF(&title_bar_compact), 1);
    egui_view_title_bar_set_palette(EGUI_VIEW_OF(&title_bar_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD7DFE7), EGUI_COLOR_HEX(0x21303D),
                                    EGUI_COLOR_HEX(0x6D7B89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xF4F7FA), EGUI_COLOR_HEX(0xD9E1E8),
                                    EGUI_COLOR_HEX(0xE1E8EE));
    egui_view_title_bar_override_static_preview_api(EGUI_VIEW_OF(&title_bar_compact), &title_bar_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    title_bar_compact_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&title_bar_compact), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&title_bar_compact));

    egui_view_title_bar_init(EGUI_VIEW_OF(&title_bar_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&title_bar_read_only), TITLE_BAR_PREVIEW_WIDTH, TITLE_BAR_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&title_bar_read_only), 8, 0, 0, 0);
    egui_view_title_bar_set_snapshots(EGUI_VIEW_OF(&title_bar_read_only), read_only_snapshots, 1);
    egui_view_title_bar_set_font(EGUI_VIEW_OF(&title_bar_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_title_bar_set_meta_font(EGUI_VIEW_OF(&title_bar_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_title_bar_set_icon_font(EGUI_VIEW_OF(&title_bar_read_only), EGUI_FONT_ICON_MS_16);
    egui_view_title_bar_set_compact_mode(EGUI_VIEW_OF(&title_bar_read_only), 1);
    egui_view_title_bar_set_read_only_mode(EGUI_VIEW_OF(&title_bar_read_only), 1);
    egui_view_title_bar_set_palette(EGUI_VIEW_OF(&title_bar_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xD9E1E8), EGUI_COLOR_HEX(0x556676),
                                    EGUI_COLOR_HEX(0x8896A4), EGUI_COLOR_HEX(0xA9B7C4), EGUI_COLOR_HEX(0xF7F9FB), EGUI_COLOR_HEX(0xDCE3EA),
                                    EGUI_COLOR_HEX(0xE8EDF2));
    egui_view_title_bar_override_static_preview_api(EGUI_VIEW_OF(&title_bar_read_only), &title_bar_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    title_bar_read_only_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&title_bar_read_only), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&title_bar_read_only));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);
    apply_read_only_state();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
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
            apply_read_only_state();
        }
        EGUI_SIM_SET_WAIT(p_action, TITLE_BAR_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TITLE_BAR_RECORD_FRAME_WAIT);
        return true;
    case 2:
        set_click_title_bar_part(p_action, EGUI_VIEW_OF(&title_bar_primary), EGUI_VIEW_TITLE_BAR_PART_BACK, TITLE_BAR_RECORD_WAIT);
        return true;
    case 3:
        EGUI_SIM_SET_WAIT(p_action, TITLE_BAR_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TITLE_BAR_RECORD_FRAME_WAIT);
        return true;
    case 5:
        set_click_title_bar_part(p_action, EGUI_VIEW_OF(&title_bar_primary), EGUI_VIEW_TITLE_BAR_PART_PANE_TOGGLE, TITLE_BAR_RECORD_WAIT);
        return true;
    case 6:
        EGUI_SIM_SET_WAIT(p_action, TITLE_BAR_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TITLE_BAR_RECORD_FRAME_WAIT);
        return true;
    case 8:
        set_click_title_bar_part(p_action, EGUI_VIEW_OF(&title_bar_primary), EGUI_VIEW_TITLE_BAR_PART_PRIMARY_ACTION, TITLE_BAR_RECORD_WAIT);
        return true;
    case 9:
        EGUI_SIM_SET_WAIT(p_action, TITLE_BAR_RECORD_WAIT);
        return true;
    case 10:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TITLE_BAR_RECORD_FRAME_WAIT);
        return true;
    case 11:
        if (first_call)
        {
            apply_compact_snapshot(1);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&title_bar_primary));
#endif
            egui_view_title_bar_set_current_part(EGUI_VIEW_OF(&title_bar_primary), EGUI_VIEW_TITLE_BAR_PART_BACK);
        }
        EGUI_SIM_SET_WAIT(p_action, TITLE_BAR_RECORD_WAIT);
        return true;
    case 12:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TITLE_BAR_RECORD_FRAME_WAIT);
        return true;
    case 13:
        set_click_view_center(p_action, EGUI_VIEW_OF(&title_bar_compact), TITLE_BAR_RECORD_WAIT);
        return true;
    case 14:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TITLE_BAR_RECORD_FRAME_WAIT);
        return true;
    case 15:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 520);
        return true;
    default:
        return false;
    }
}
#endif
