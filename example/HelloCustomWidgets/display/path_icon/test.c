#include "egui.h"
#include "egui_view_path_icon.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define PATH_ICON_ROOT_WIDTH        224
#define PATH_ICON_ROOT_HEIGHT       148
#define PATH_ICON_PRIMARY_SIZE      56
#define PATH_ICON_PREVIEW_SIZE      30
#define PATH_ICON_BOTTOM_ROW_WIDTH  68
#define PATH_ICON_BOTTOM_ROW_HEIGHT 30
#define PATH_ICON_RECORD_WAIT       90
#define PATH_ICON_RECORD_FRAME_WAIT 170
#define PATH_ICON_RECORD_FINAL_WAIT 280
#define PATH_ICON_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct path_icon_snapshot path_icon_snapshot_t;
struct path_icon_snapshot
{
    const egui_view_path_icon_data_t *data;
    const char *icon_name;
    egui_color_t icon_name_color;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_path_icon_t primary_icon;
static egui_view_label_t primary_name_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_path_icon_t subtle_icon;
static egui_view_path_icon_t accent_icon;
static egui_view_api_t subtle_icon_api;
static egui_view_api_t accent_icon_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "PathIcon";

static path_icon_snapshot_t primary_snapshots[] = {
        {
                NULL,
                "Bookmark",
                EGUI_COLOR_HEX(0x0F6CBD),
        },
        {
                NULL,
                "Heart",
                EGUI_COLOR_HEX(0xC23960),
        },
        {
                NULL,
                "Send",
                EGUI_COLOR_HEX(0x0F7B45),
        },
};

static void layout_page(void);

static void init_primary_snapshots(void)
{
    primary_snapshots[0].data = egui_view_path_icon_get_bookmark_data();
    primary_snapshots[1].data = egui_view_path_icon_get_heart_data();
    primary_snapshots[2].data = egui_view_path_icon_get_send_data();
}

static void init_text_label(egui_view_label_t *label, egui_dim_t width, egui_dim_t height, const char *text, const egui_font_t *font,
                            egui_color_t color, uint8_t align)
{
    egui_view_label_init(EGUI_VIEW_OF(label));
    egui_view_set_size(EGUI_VIEW_OF(label), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(label), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(label), align);
    egui_view_label_set_font(EGUI_VIEW_OF(label), font);
    egui_view_label_set_font_color(EGUI_VIEW_OF(label), color, EGUI_ALPHA_100);
}

static void apply_primary_snapshot(uint8_t index)
{
    const path_icon_snapshot_t *snapshot = &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT];

    egui_view_path_icon_set_data(EGUI_VIEW_OF(&primary_icon), snapshot->data);
    egui_view_path_icon_set_palette(EGUI_VIEW_OF(&primary_icon), snapshot->icon_name_color);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_name_label), snapshot->icon_name);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_name_label), snapshot->icon_name_color, EGUI_ALPHA_100);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(PATH_ICON_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    egui_view_path_icon_set_data(EGUI_VIEW_OF(&subtle_icon), egui_view_path_icon_get_heart_data());
    egui_view_path_icon_apply_subtle_style(EGUI_VIEW_OF(&subtle_icon));

    egui_view_path_icon_set_data(EGUI_VIEW_OF(&accent_icon), egui_view_path_icon_get_send_data());
    egui_view_path_icon_apply_accent_style(EGUI_VIEW_OF(&accent_icon));

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
    init_primary_snapshots();

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), PATH_ICON_ROOT_WIDTH, PATH_ICON_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, PATH_ICON_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_path_icon_init(EGUI_VIEW_OF(&primary_icon));
    egui_view_set_size(EGUI_VIEW_OF(&primary_icon), PATH_ICON_PRIMARY_SIZE, PATH_ICON_PRIMARY_SIZE);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_icon), 0, 0, 0, 8);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&primary_icon), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_icon));

    init_text_label(&primary_name_label, PATH_ICON_ROOT_WIDTH, 12, "Bookmark", (const egui_font_t *)&egui_res_font_montserrat_10_4,
                    EGUI_COLOR_HEX(0x0F6CBD), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_name_label), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_name_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), PATH_ICON_BOTTOM_ROW_WIDTH, PATH_ICON_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_path_icon_init(EGUI_VIEW_OF(&subtle_icon));
    egui_view_set_size(EGUI_VIEW_OF(&subtle_icon), PATH_ICON_PREVIEW_SIZE, PATH_ICON_PREVIEW_SIZE);
    egui_view_path_icon_override_static_preview_api(EGUI_VIEW_OF(&subtle_icon), &subtle_icon_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&subtle_icon), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&subtle_icon));

    egui_view_path_icon_init(EGUI_VIEW_OF(&accent_icon));
    egui_view_set_size(EGUI_VIEW_OF(&accent_icon), PATH_ICON_PREVIEW_SIZE, PATH_ICON_PREVIEW_SIZE);
    egui_view_set_margin(EGUI_VIEW_OF(&accent_icon), 8, 0, 0, 0);
    egui_view_path_icon_override_static_preview_api(EGUI_VIEW_OF(&accent_icon), &accent_icon_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&accent_icon), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&accent_icon));

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
        EGUI_SIM_SET_WAIT(p_action, PATH_ICON_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, PATH_ICON_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PATH_ICON_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, PATH_ICON_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PATH_ICON_RECORD_FRAME_WAIT);
        return true;
    case 5:
        EGUI_SIM_SET_WAIT(p_action, PATH_ICON_RECORD_FINAL_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PATH_ICON_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
