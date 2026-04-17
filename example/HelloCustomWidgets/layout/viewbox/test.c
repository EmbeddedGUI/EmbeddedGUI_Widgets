#include "egui.h"
#include "egui_view_viewbox.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define VIEWBOX_ROOT_WIDTH        224
#define VIEWBOX_ROOT_HEIGHT       240
#define VIEWBOX_PRIMARY_WIDTH     196
#define VIEWBOX_PRIMARY_HEIGHT    120
#define VIEWBOX_PREVIEW_WIDTH     104
#define VIEWBOX_PREVIEW_HEIGHT    78
#define VIEWBOX_BOTTOM_ROW_WIDTH  216
#define VIEWBOX_BOTTOM_ROW_HEIGHT 78
#define VIEWBOX_RECORD_WAIT       90
#define VIEWBOX_RECORD_FRAME_WAIT 180
#define VIEWBOX_RECORD_FINAL_WAIT 280

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)(sizeof(primary_snapshots) / sizeof(primary_snapshots[0])))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_viewbox_t viewbox_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_viewbox_t viewbox_compact;
static egui_view_viewbox_t viewbox_read_only;
static egui_view_api_t viewbox_compact_api;
static egui_view_api_t viewbox_read_only_api;
static uint8_t ui_ready;

static void layout_page(void);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Viewbox";

static const egui_view_viewbox_preset_t primary_presets[] = {
        {"Uniform", "Keep ratio", EGUI_VIEW_VIEWBOX_STRETCH_UNIFORM},
        {"Fill", "Stretch shell", EGUI_VIEW_VIEWBOX_STRETCH_FILL},
        {"Down only", "No upscale", EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY},
};

static const egui_view_viewbox_snapshot_t primary_snapshots[] = {
        {"OPS", "Device preview", "Large content fits a compact viewport without leaving the reference shell.", "Telemetry deck", "420 x 240 baseline",
         "Footer stays attached when the surface scales.", "Uniform preset", primary_presets, 420, 240, 3, 0},
        {"REL", "Cover preview", "Fill mode shows how one surface absorbs the whole viewport when the shell wins.", "Launch poster", "520 x 180 artwork",
         "Wide artwork stretches to each edge for cover-like composition.", "Fill preset", primary_presets, 520, 180, 3, 1},
        {"QA", "Inspector thumb", "Downscale only keeps small content at 1:1 until the viewport becomes tighter.", "Audit chip", "96 x 64 surface",
         "Compact sources do not upscale in the lightweight reference view.", "Downscale only", primary_presets, 96, 64, 3, 2},
};

static const egui_view_viewbox_preset_t compact_presets[] = {
        {"Uniform", "", EGUI_VIEW_VIEWBOX_STRETCH_UNIFORM},
        {"Fill", "", EGUI_VIEW_VIEWBOX_STRETCH_FILL},
        {"Down", "", EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY},
};

static const egui_view_viewbox_snapshot_t compact_snapshot = {"UI", "Compact fit", "", "Tile", "240 x 160", "", "", compact_presets, 240, 160, 3, 0};

static const egui_view_viewbox_snapshot_t read_only_snapshot = {
        "LOCK", "Read only viewbox", "", "Poster", "520 x 180", "", "Muted static preview", compact_presets, 520, 180, 3, 1};

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_viewbox_set_current_snapshot(EGUI_VIEW_OF(&viewbox_primary), index % PRIMARY_SNAPSHOT_COUNT);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(0);
}

static void apply_preview_states(void)
{
    egui_view_viewbox_set_current_snapshot(EGUI_VIEW_OF(&viewbox_compact), 0);
    egui_view_viewbox_set_current_snapshot(EGUI_VIEW_OF(&viewbox_read_only), 0);
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
static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event = {0};

    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    EGUI_VIEW_OF(&viewbox_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&viewbox_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&viewbox_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&viewbox_primary), &event);
}

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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), VIEWBOX_ROOT_WIDTH, VIEWBOX_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), VIEWBOX_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_viewbox_init(EGUI_VIEW_OF(&viewbox_primary));
    egui_view_set_size(EGUI_VIEW_OF(&viewbox_primary), VIEWBOX_PRIMARY_WIDTH, VIEWBOX_PRIMARY_HEIGHT);
    egui_view_viewbox_set_snapshots(EGUI_VIEW_OF(&viewbox_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    egui_view_viewbox_set_font(EGUI_VIEW_OF(&viewbox_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_viewbox_set_meta_font(EGUI_VIEW_OF(&viewbox_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_viewbox_set_palette(EGUI_VIEW_OF(&viewbox_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xF5F8FA), EGUI_COLOR_HEX(0xD4DDE6),
                                  EGUI_COLOR_HEX(0x1B2834), EGUI_COLOR_HEX(0x6D7C8A), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_set_margin(EGUI_VIEW_OF(&viewbox_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&viewbox_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), VIEWBOX_BOTTOM_ROW_WIDTH, VIEWBOX_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_viewbox_init(EGUI_VIEW_OF(&viewbox_compact));
    egui_view_set_size(EGUI_VIEW_OF(&viewbox_compact), VIEWBOX_PREVIEW_WIDTH, VIEWBOX_PREVIEW_HEIGHT);
    egui_view_viewbox_set_snapshots(EGUI_VIEW_OF(&viewbox_compact), &compact_snapshot, 1);
    egui_view_viewbox_set_font(EGUI_VIEW_OF(&viewbox_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_viewbox_set_meta_font(EGUI_VIEW_OF(&viewbox_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_viewbox_set_compact_mode(EGUI_VIEW_OF(&viewbox_compact), 1);
    egui_view_viewbox_set_palette(EGUI_VIEW_OF(&viewbox_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xF3FBF9), EGUI_COLOR_HEX(0xCEE3DD),
                                  EGUI_COLOR_HEX(0x19322F), EGUI_COLOR_HEX(0x5F7B74), EGUI_COLOR_HEX(0x0D9488));
    egui_view_viewbox_override_static_preview_api(EGUI_VIEW_OF(&viewbox_compact), &viewbox_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&viewbox_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&viewbox_compact));

    egui_view_viewbox_init(EGUI_VIEW_OF(&viewbox_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&viewbox_read_only), VIEWBOX_PREVIEW_WIDTH, VIEWBOX_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&viewbox_read_only), 8, 0, 0, 0);
    egui_view_viewbox_set_snapshots(EGUI_VIEW_OF(&viewbox_read_only), &read_only_snapshot, 1);
    egui_view_viewbox_set_font(EGUI_VIEW_OF(&viewbox_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_viewbox_set_meta_font(EGUI_VIEW_OF(&viewbox_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_viewbox_set_compact_mode(EGUI_VIEW_OF(&viewbox_read_only), 1);
    egui_view_viewbox_set_read_only_mode(EGUI_VIEW_OF(&viewbox_read_only), 1);
    egui_view_viewbox_set_palette(EGUI_VIEW_OF(&viewbox_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xF5F8FA), EGUI_COLOR_HEX(0xD8E1E8),
                                  EGUI_COLOR_HEX(0x556574), EGUI_COLOR_HEX(0x8A98A5), EGUI_COLOR_HEX(0xA5B2BE));
    egui_view_viewbox_override_static_preview_api(EGUI_VIEW_OF(&viewbox_read_only), &viewbox_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&viewbox_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&viewbox_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&viewbox_primary));
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
            egui_view_request_focus(EGUI_VIEW_OF(&viewbox_primary));
#endif
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, VIEWBOX_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_RIGHT);
        }
        EGUI_SIM_SET_WAIT(p_action, VIEWBOX_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, VIEWBOX_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, VIEWBOX_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, VIEWBOX_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, VIEWBOX_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, VIEWBOX_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&viewbox_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, VIEWBOX_RECORD_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, VIEWBOX_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
