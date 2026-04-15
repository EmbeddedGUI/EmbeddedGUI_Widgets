#include "egui.h"
#include "egui_view_image_icon.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define IMAGE_ICON_ROOT_WIDTH        224
#define IMAGE_ICON_ROOT_HEIGHT       184
#define IMAGE_ICON_PRIMARY_SIZE      72
#define IMAGE_ICON_PREVIEW_SIZE      40
#define IMAGE_ICON_BOTTOM_ROW_WIDTH  88
#define IMAGE_ICON_BOTTOM_ROW_HEIGHT 40
#define IMAGE_ICON_RECORD_WAIT       90
#define IMAGE_ICON_RECORD_FRAME_WAIT 170
#define IMAGE_ICON_RECORD_FINAL_WAIT 280
#define IMAGE_ICON_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct image_icon_snapshot image_icon_snapshot_t;
struct image_icon_snapshot
{
    const egui_image_t *image;
    const char *image_name;
    egui_color_t image_name_color;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_image_icon_t primary_icon;
static egui_view_label_t primary_name_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_image_icon_t warm_icon;
static egui_view_image_icon_t fresh_icon;
static egui_view_api_t warm_icon_api;
static egui_view_api_t fresh_icon_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "ImageIcon";

static image_icon_snapshot_t primary_snapshots[] = {
        {
                NULL,
                "Thumbnail",
                EGUI_COLOR_HEX(0x0F6CBD),
        },
        {
                NULL,
                "Warm tone",
                EGUI_COLOR_HEX(0xA15C00),
        },
        {
                NULL,
                "Fresh tone",
                EGUI_COLOR_HEX(0x0F7B45),
        },
};

static void layout_page(void);

static void init_primary_snapshots(void)
{
    primary_snapshots[0].image = egui_view_image_icon_get_default_image();
    primary_snapshots[1].image = egui_view_image_icon_get_warm_image();
    primary_snapshots[2].image = egui_view_image_icon_get_fresh_image();
}

static void apply_primary_snapshot(uint8_t index)
{
    const image_icon_snapshot_t *snapshot = &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT];

    egui_view_image_icon_set_image(EGUI_VIEW_OF(&primary_icon), snapshot->image);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_name_label), snapshot->image_name);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_name_label), snapshot->image_name_color, EGUI_ALPHA_100);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(IMAGE_ICON_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    egui_view_image_icon_set_image(EGUI_VIEW_OF(&warm_icon), egui_view_image_icon_get_warm_image());
    egui_view_image_icon_set_image(EGUI_VIEW_OF(&fresh_icon), egui_view_image_icon_get_fresh_image());

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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), IMAGE_ICON_ROOT_WIDTH, IMAGE_ICON_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), IMAGE_ICON_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_image_icon_init(EGUI_VIEW_OF(&primary_icon));
    egui_view_set_size(EGUI_VIEW_OF(&primary_icon), IMAGE_ICON_PRIMARY_SIZE, IMAGE_ICON_PRIMARY_SIZE);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_icon), 0, 0, 0, 8);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&primary_icon), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_icon));

    egui_view_label_init(EGUI_VIEW_OF(&primary_name_label));
    egui_view_set_size(EGUI_VIEW_OF(&primary_name_label), IMAGE_ICON_ROOT_WIDTH, 12);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_name_label), "Thumbnail");
    egui_view_label_set_align_type(EGUI_VIEW_OF(&primary_name_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&primary_name_label), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_name_label), EGUI_COLOR_HEX(0x0F6CBD), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_name_label), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_name_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), IMAGE_ICON_BOTTOM_ROW_WIDTH, IMAGE_ICON_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_image_icon_init(EGUI_VIEW_OF(&warm_icon));
    egui_view_set_size(EGUI_VIEW_OF(&warm_icon), IMAGE_ICON_PREVIEW_SIZE, IMAGE_ICON_PREVIEW_SIZE);
    egui_view_image_icon_override_static_preview_api(EGUI_VIEW_OF(&warm_icon), &warm_icon_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&warm_icon), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&warm_icon));

    egui_view_image_icon_init(EGUI_VIEW_OF(&fresh_icon));
    egui_view_set_size(EGUI_VIEW_OF(&fresh_icon), IMAGE_ICON_PREVIEW_SIZE, IMAGE_ICON_PREVIEW_SIZE);
    egui_view_set_margin(EGUI_VIEW_OF(&fresh_icon), 8, 0, 0, 0);
    egui_view_image_icon_override_static_preview_api(EGUI_VIEW_OF(&fresh_icon), &fresh_icon_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&fresh_icon), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&fresh_icon));

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
        EGUI_SIM_SET_WAIT(p_action, IMAGE_ICON_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, IMAGE_ICON_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, IMAGE_ICON_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, IMAGE_ICON_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, IMAGE_ICON_RECORD_FRAME_WAIT);
        return true;
    case 5:
        EGUI_SIM_SET_WAIT(p_action, IMAGE_ICON_RECORD_FINAL_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, IMAGE_ICON_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
