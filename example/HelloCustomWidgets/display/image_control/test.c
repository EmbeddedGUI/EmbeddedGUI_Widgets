#include "egui.h"
#include "egui_view_image_control.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define IMAGE_CONTROL_ROOT_WIDTH        236
#define IMAGE_CONTROL_ROOT_HEIGHT       228
#define IMAGE_CONTROL_PRIMARY_WIDTH     166
#define IMAGE_CONTROL_PRIMARY_HEIGHT    104
#define IMAGE_CONTROL_PREVIEW_ROW_WIDTH 196
#define IMAGE_CONTROL_PREVIEW_WIDTH     92
#define IMAGE_CONTROL_PREVIEW_HEIGHT    50
#define IMAGE_CONTROL_RECORD_WAIT       90
#define IMAGE_CONTROL_RECORD_FRAME_WAIT 170
#define IMAGE_CONTROL_RECORD_FINAL_WAIT 280
#define IMAGE_CONTROL_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct image_control_snapshot image_control_snapshot_t;
struct image_control_snapshot
{
    const egui_image_t *image;
    const char *source_name;
    uint8_t stretch;
    const char *caption;
    egui_color_t caption_color;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_image_control_t primary_image;
static egui_view_label_t caption_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_image_control_t secondary_preview;
static egui_view_image_control_t muted_preview;
static egui_view_api_t secondary_preview_api;
static egui_view_api_t muted_preview_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Image";

static image_control_snapshot_t primary_snapshots[] = {
        {
                NULL,
                "Landscape",
                EGUI_VIEW_IMAGE_CONTROL_STRETCH_UNIFORM,
                "Landscape / Uniform",
                EGUI_COLOR_HEX(0x0F6CBD),
        },
        {
                NULL,
                "Portrait",
                EGUI_VIEW_IMAGE_CONTROL_STRETCH_UNIFORM,
                "Portrait / Uniform",
                EGUI_COLOR_HEX(0x8A5A00),
        },
        {
                NULL,
                "Square",
                EGUI_VIEW_IMAGE_CONTROL_STRETCH_FILL,
                "Square / Fill",
                EGUI_COLOR_HEX(0x0F7B45),
        },
        {
                NULL,
                "Landscape",
                EGUI_VIEW_IMAGE_CONTROL_STRETCH_NONE,
                "Landscape / None",
                EGUI_COLOR_HEX(0x4F5F70),
        },
};

static void layout_page(void);

static void init_primary_snapshots(void)
{
    primary_snapshots[0].image = egui_view_image_control_get_landscape_image();
    primary_snapshots[1].image = egui_view_image_control_get_portrait_image();
    primary_snapshots[2].image = egui_view_image_control_get_square_image();
    primary_snapshots[3].image = egui_view_image_control_get_landscape_image();
}

static void apply_primary_snapshot(uint8_t index)
{
    const image_control_snapshot_t *snapshot = &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT];

    egui_view_image_control_set_palette(EGUI_VIEW_OF(&primary_image), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xB8C7D7),
                                        EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x798694));
    egui_view_image_control_set_source(EGUI_VIEW_OF(&primary_image), snapshot->image, snapshot->source_name);
    egui_view_image_control_set_stretch(EGUI_VIEW_OF(&primary_image), snapshot->stretch);
    egui_view_label_set_text(EGUI_VIEW_OF(&caption_label), snapshot->caption);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&caption_label), snapshot->caption_color, EGUI_ALPHA_100);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(IMAGE_CONTROL_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    egui_view_image_control_set_palette(EGUI_VIEW_OF(&secondary_preview), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD0D9E2),
                                        EGUI_COLOR_HEX(0x0F7B45), EGUI_COLOR_HEX(0x7E8A97));
    egui_view_image_control_set_source(EGUI_VIEW_OF(&secondary_preview), egui_view_image_control_get_square_image(), "Square");
    egui_view_image_control_set_stretch(EGUI_VIEW_OF(&secondary_preview), EGUI_VIEW_IMAGE_CONTROL_STRETCH_FILL);

    egui_view_image_control_set_palette(EGUI_VIEW_OF(&muted_preview), EGUI_COLOR_HEX(0xF7F9FB), EGUI_COLOR_HEX(0xD3DCE5),
                                        EGUI_COLOR_HEX(0x6B7785), EGUI_COLOR_HEX(0x7E8A97));
    egui_view_image_control_set_source(EGUI_VIEW_OF(&muted_preview), egui_view_image_control_get_portrait_image(), "Portrait");
    egui_view_image_control_set_stretch(EGUI_VIEW_OF(&muted_preview), EGUI_VIEW_IMAGE_CONTROL_STRETCH_UNIFORM);

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
    init_primary_snapshots();

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), IMAGE_CONTROL_ROOT_WIDTH, IMAGE_CONTROL_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&title_label), IMAGE_CONTROL_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_image_control_init(EGUI_VIEW_OF(&primary_image));
    egui_view_set_size(EGUI_VIEW_OF(&primary_image), IMAGE_CONTROL_PRIMARY_WIDTH, IMAGE_CONTROL_PRIMARY_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_image), 0, 0, 0, 8);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&primary_image), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_image));

    egui_view_label_init(EGUI_VIEW_OF(&caption_label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&caption_label), IMAGE_CONTROL_ROOT_WIDTH, 12);
    egui_view_label_set_text(EGUI_VIEW_OF(&caption_label), "Landscape / Uniform");
    egui_view_label_set_align_type(EGUI_VIEW_OF(&caption_label), EGUI_ALIGN_CENTER);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(&caption_label), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&caption_label), EGUI_COLOR_HEX(0x0F6CBD), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&caption_label), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&caption_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), IMAGE_CONTROL_PREVIEW_ROW_WIDTH, IMAGE_CONTROL_PREVIEW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_image_control_init(EGUI_VIEW_OF(&secondary_preview));
    egui_view_set_size(EGUI_VIEW_OF(&secondary_preview), IMAGE_CONTROL_PREVIEW_WIDTH, IMAGE_CONTROL_PREVIEW_HEIGHT);
    egui_view_image_control_override_static_preview_api(EGUI_VIEW_OF(&secondary_preview), &secondary_preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&secondary_preview), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&secondary_preview));

    egui_view_image_control_init(EGUI_VIEW_OF(&muted_preview));
    egui_view_set_size(EGUI_VIEW_OF(&muted_preview), IMAGE_CONTROL_PREVIEW_WIDTH, IMAGE_CONTROL_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&muted_preview), 12, 0, 0, 0);
    egui_view_image_control_override_static_preview_api(EGUI_VIEW_OF(&muted_preview), &muted_preview_api);
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
        EGUI_SIM_SET_WAIT(p_action, IMAGE_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, IMAGE_CONTROL_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, IMAGE_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, IMAGE_CONTROL_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, IMAGE_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_snapshot(3);
        }
        EGUI_SIM_SET_WAIT(p_action, IMAGE_CONTROL_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, IMAGE_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_default_state();
        }
        EGUI_SIM_SET_WAIT(p_action, IMAGE_CONTROL_RECORD_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, IMAGE_CONTROL_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
