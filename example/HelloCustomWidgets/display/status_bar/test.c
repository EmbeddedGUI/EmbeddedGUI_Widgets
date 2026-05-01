#include "egui.h"
#include "egui_view_status_bar.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define STATUS_BAR_ROOT_WIDTH        238
#define STATUS_BAR_ROOT_HEIGHT       156
#define STATUS_BAR_PRIMARY_WIDTH     204
#define STATUS_BAR_PRIMARY_HEIGHT    36
#define STATUS_BAR_SECONDARY_WIDTH   94
#define STATUS_BAR_SECONDARY_HEIGHT  28
#define STATUS_BAR_PREVIEW_ROW_WIDTH 200
#define STATUS_BAR_RECORD_WAIT       90
#define STATUS_BAR_RECORD_FRAME_WAIT 170
#define STATUS_BAR_RECORD_FINAL_WAIT 280
#define STATUS_BAR_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct status_bar_snapshot status_bar_snapshot_t;
struct status_bar_snapshot
{
    const egui_view_status_bar_item_t *items;
    uint8_t item_count;
    const char *caption;
    egui_color_t caption_color;
    uint8_t palette;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_status_bar_t primary_control;
static egui_view_label_t caption_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_status_bar_t secondary_preview;
static egui_view_status_bar_t muted_preview;
static egui_view_api_t secondary_preview_api;
static egui_view_api_t muted_preview_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "StatusBar";

static const egui_view_status_bar_item_t standard_items[] = {
        {"Sync", "Ready", 2, EGUI_VIEW_STATUS_BAR_STATE_OK, 1},
        {"Line", "124", 1, EGUI_VIEW_STATUS_BAR_STATE_INFO, 0},
        {"Mode", "Edit", 1, EGUI_VIEW_STATUS_BAR_STATE_NORMAL, 0},
};

static const egui_view_status_bar_item_t accent_items[] = {
        {"Build", "Running", 2, EGUI_VIEW_STATUS_BAR_STATE_INFO, 1},
        {"Warnings", "2", 1, EGUI_VIEW_STATUS_BAR_STATE_WARN, 1},
        {"Branch", "main", 1, EGUI_VIEW_STATUS_BAR_STATE_NORMAL, 0},
};

static const egui_view_status_bar_item_t telemetry_items[] = {
        {"CPU", "42%", 1, EGUI_VIEW_STATUS_BAR_STATE_INFO, 0},
        {"Mem", "61%", 1, EGUI_VIEW_STATUS_BAR_STATE_OK, 0},
        {"Net", "Idle", 1, EGUI_VIEW_STATUS_BAR_STATE_NORMAL, 0},
};

static const egui_view_status_bar_item_t locked_items[] = {
        {"State", "Locked", 2, EGUI_VIEW_STATUS_BAR_STATE_NORMAL, 0},
        {"Owner", "System", 1, EGUI_VIEW_STATUS_BAR_STATE_INFO, 0},
        {"Write", "Off", 1, EGUI_VIEW_STATUS_BAR_STATE_WARN, 0},
};

static const egui_view_status_bar_item_t secondary_preview_items[] = {
        {"CPU", "42%", 1, EGUI_VIEW_STATUS_BAR_STATE_INFO, 0},
        {"Net", "Idle", 1, EGUI_VIEW_STATUS_BAR_STATE_NORMAL, 0},
};

static const egui_view_status_bar_item_t muted_preview_items[] = {
        {"Lock", "On", 1, EGUI_VIEW_STATUS_BAR_STATE_NORMAL, 0},
        {"Edit", "Off", 1, EGUI_VIEW_STATUS_BAR_STATE_WARN, 0},
};

static const status_bar_snapshot_t primary_snapshots[] = {
        {standard_items, (uint8_t)EGUI_ARRAY_SIZE(standard_items), "Ready / standard", EGUI_COLOR_HEX(0x0F6CBD), 0},
        {accent_items, (uint8_t)EGUI_ARRAY_SIZE(accent_items), "Running / accent", EGUI_COLOR_HEX(0x0F6CBD), 1},
        {telemetry_items, (uint8_t)EGUI_ARRAY_SIZE(telemetry_items), "Telemetry / app palette", EGUI_COLOR_HEX(0x0C7C73), 2},
        {locked_items, (uint8_t)EGUI_ARRAY_SIZE(locked_items), "Locked / app muted", EGUI_COLOR_HEX(0x65717E), 3},
};

static void layout_page(void);

static void init_text_label(egui_view_label_t *label, egui_dim_t width, egui_dim_t height, const char *text, const egui_font_t *font,
                            egui_color_t color, uint8_t align_type)
{
    egui_view_label_init(EGUI_VIEW_OF(label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(label), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(label), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(label), align_type);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(label), font);
    egui_view_label_set_font_color(EGUI_VIEW_OF(label), color, EGUI_ALPHA_100);
}

static void apply_status_bar_palette(egui_view_t *view, uint8_t palette)
{
    switch (palette)
    {
    case 1:
        egui_view_status_bar_set_palette(view, EGUI_COLOR_HEX(0xF7FBFF), EGUI_COLOR_HEX(0xB9D6F0), EGUI_COLOR_HEX(0xCDE0F2),
                                         EGUI_COLOR_HEX(0x173247), EGUI_COLOR_HEX(0x5D7183), EGUI_COLOR_HEX(0x0F6CBD),
                                         EGUI_COLOR_HEX(0x0F7B45), EGUI_COLOR_HEX(0xA15C00));
        break;
    case 2:
        egui_view_status_bar_set_palette(view, EGUI_COLOR_HEX(0xF8FBFD), EGUI_COLOR_HEX(0xD2DCE6), EGUI_COLOR_HEX(0xDFE7EF),
                                         EGUI_COLOR_HEX(0x21313E), EGUI_COLOR_HEX(0x6E7E8E), EGUI_COLOR_HEX(0x0C7C73),
                                         EGUI_COLOR_HEX(0x107C41), EGUI_COLOR_HEX(0xA15C00));
        break;
    case 3:
        egui_view_status_bar_set_palette(view, EGUI_COLOR_HEX(0xF5F7FA), EGUI_COLOR_HEX(0xD7DEE6), EGUI_COLOR_HEX(0xE1E7ED),
                                         EGUI_COLOR_HEX(0x687684), EGUI_COLOR_HEX(0x8B98A5), EGUI_COLOR_HEX(0x788593),
                                         EGUI_COLOR_HEX(0x768777), EGUI_COLOR_HEX(0x92765F));
        break;
    default:
        egui_view_status_bar_set_palette(view, EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xCCD6E0), EGUI_COLOR_HEX(0xDCE4EC),
                                         EGUI_COLOR_HEX(0x1D2A36), EGUI_COLOR_HEX(0x637283), EGUI_COLOR_HEX(0x0F6CBD),
                                         EGUI_COLOR_HEX(0x107C41), EGUI_COLOR_HEX(0xB26A00));
        break;
    }
}

static void apply_primary_snapshot(uint8_t index)
{
    const status_bar_snapshot_t *snapshot = &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT];

    egui_view_set_enable(EGUI_VIEW_OF(&primary_control), 1);
    apply_status_bar_palette(EGUI_VIEW_OF(&primary_control), snapshot->palette);
    egui_view_status_bar_set_items(EGUI_VIEW_OF(&primary_control), snapshot->items, snapshot->item_count);
    egui_view_label_set_text(EGUI_VIEW_OF(&caption_label), snapshot->caption);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&caption_label), snapshot->caption_color, EGUI_ALPHA_100);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(STATUS_BAR_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    egui_view_set_enable(EGUI_VIEW_OF(&secondary_preview), 1);
    apply_status_bar_palette(EGUI_VIEW_OF(&secondary_preview), 2);
    egui_view_status_bar_set_items(EGUI_VIEW_OF(&secondary_preview), secondary_preview_items, (uint8_t)EGUI_ARRAY_SIZE(secondary_preview_items));

    egui_view_set_enable(EGUI_VIEW_OF(&muted_preview), 0);
    apply_status_bar_palette(EGUI_VIEW_OF(&muted_preview), 3);
    egui_view_status_bar_set_items(EGUI_VIEW_OF(&muted_preview), muted_preview_items, (uint8_t)EGUI_ARRAY_SIZE(muted_preview_items));

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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), STATUS_BAR_ROOT_WIDTH, STATUS_BAR_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, STATUS_BAR_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4,
                    EGUI_COLOR_HEX(0x21303F), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_status_bar_init(EGUI_VIEW_OF(&primary_control));
    egui_view_set_size(EGUI_VIEW_OF(&primary_control), STATUS_BAR_PRIMARY_WIDTH, STATUS_BAR_PRIMARY_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_control), 0, 0, 0, 10);
    egui_view_status_bar_set_fonts(EGUI_VIEW_OF(&primary_control), (const egui_font_t *)&egui_res_font_montserrat_8_4,
                                   (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_control));

    init_text_label(&caption_label, STATUS_BAR_ROOT_WIDTH, 12, "Ready / standard", (const egui_font_t *)&egui_res_font_montserrat_8_4,
                    EGUI_COLOR_HEX(0x0F6CBD), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&caption_label), 0, 0, 0, 14);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&caption_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), STATUS_BAR_PREVIEW_ROW_WIDTH, STATUS_BAR_SECONDARY_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_status_bar_init(EGUI_VIEW_OF(&secondary_preview));
    egui_view_set_size(EGUI_VIEW_OF(&secondary_preview), STATUS_BAR_SECONDARY_WIDTH, STATUS_BAR_SECONDARY_HEIGHT);
    egui_view_status_bar_set_fonts(EGUI_VIEW_OF(&secondary_preview), (const egui_font_t *)&egui_res_font_montserrat_8_4,
                                   (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_status_bar_override_static_preview_api(EGUI_VIEW_OF(&secondary_preview), &secondary_preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&secondary_preview), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&secondary_preview));

    egui_view_status_bar_init(EGUI_VIEW_OF(&muted_preview));
    egui_view_set_size(EGUI_VIEW_OF(&muted_preview), STATUS_BAR_SECONDARY_WIDTH, STATUS_BAR_SECONDARY_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&muted_preview), 12, 0, 0, 0);
    egui_view_status_bar_set_fonts(EGUI_VIEW_OF(&muted_preview), (const egui_font_t *)&egui_res_font_montserrat_8_4,
                                   (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_status_bar_override_static_preview_api(EGUI_VIEW_OF(&muted_preview), &muted_preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&muted_preview), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&muted_preview));

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
        EGUI_SIM_SET_WAIT(p_action, STATUS_BAR_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, STATUS_BAR_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, STATUS_BAR_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, STATUS_BAR_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, STATUS_BAR_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_snapshot(3);
        }
        EGUI_SIM_SET_WAIT(p_action, STATUS_BAR_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, STATUS_BAR_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_default_state();
        }
        EGUI_SIM_SET_WAIT(p_action, STATUS_BAR_RECORD_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, STATUS_BAR_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
