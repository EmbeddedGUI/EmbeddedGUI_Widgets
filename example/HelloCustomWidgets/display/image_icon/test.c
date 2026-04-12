#include "egui.h"
#include "egui_view_image_icon.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define IMAGE_ICON_ROOT_WIDTH        224
#define IMAGE_ICON_ROOT_HEIGHT       266
#define IMAGE_ICON_PANEL_WIDTH       196
#define IMAGE_ICON_PANEL_HEIGHT      136
#define IMAGE_ICON_PRIMARY_SIZE      72
#define IMAGE_ICON_PREVIEW_WIDTH     104
#define IMAGE_ICON_PREVIEW_HEIGHT    88
#define IMAGE_ICON_PREVIEW_SIZE      40
#define IMAGE_ICON_BOTTOM_ROW_WIDTH  216
#define IMAGE_ICON_BOTTOM_ROW_HEIGHT 88
#define IMAGE_ICON_RECORD_WAIT       90
#define IMAGE_ICON_RECORD_FRAME_WAIT 170

typedef struct
{
    const char *heading;
    const egui_image_t *image;
    const char *image_name;
    egui_color_t image_name_color;
    const char *note;
} image_icon_snapshot_t;

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_linearlayout_t primary_panel;
static egui_view_label_t primary_heading_label;
static egui_view_image_icon_t primary_icon;
static egui_view_label_t primary_name_label;
static egui_view_label_t primary_note_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t warm_panel;
static egui_view_label_t warm_heading_label;
static egui_view_image_icon_t warm_icon;
static egui_view_label_t warm_body_label;
static egui_view_linearlayout_t fresh_panel;
static egui_view_label_t fresh_heading_label;
static egui_view_image_icon_t fresh_icon;
static egui_view_label_t fresh_body_label;
static egui_view_api_t warm_icon_api;
static egui_view_api_t fresh_icon_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_surface_panel_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_surface_panel_params, &bg_surface_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_surface_panel, &bg_surface_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_muted_panel_param, EGUI_COLOR_HEX(0xFBFCFD), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_muted_panel_params, &bg_muted_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_muted_panel, &bg_muted_panel_params);

static const char *title_text = "ImageIcon";

static image_icon_snapshot_t primary_snapshots[] = {
        {
                "Project cover",
                NULL,
                "Thumbnail",
                EGUI_COLOR_HEX(0x0F6CBD),
                "Readable picture detail at icon size.",
        },
        {
                "Editorial pick",
                NULL,
                "Warm tone",
                EGUI_COLOR_HEX(0xA15C00),
                "Source swaps keep the same frame.",
        },
        {
                "Shared memory",
                NULL,
                "Fresh tone",
                EGUI_COLOR_HEX(0x0F7B45),
                "Meaning stays in the picture.",
        },
};

static void init_text_label(egui_view_label_t *label, egui_dim_t width, egui_dim_t height, const char *text, const egui_font_t *font, egui_color_t color,
                            uint8_t align)
{
    egui_view_label_init(EGUI_VIEW_OF(label));
    egui_view_set_size(EGUI_VIEW_OF(label), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(label), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(label), align);
    egui_view_label_set_font(EGUI_VIEW_OF(label), font);
    egui_view_label_set_font_color(EGUI_VIEW_OF(label), color, EGUI_ALPHA_100);
}

static void init_panel(egui_view_linearlayout_t *panel, egui_dim_t width, egui_dim_t height, egui_background_t *background, uint8_t align)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(panel));
    egui_view_set_size(EGUI_VIEW_OF(panel), width, height);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(panel), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(panel), align);
    egui_view_set_background(EGUI_VIEW_OF(panel), background);
    egui_view_set_padding(EGUI_VIEW_OF(panel), 8, 8, 8, 8);
}

static void init_primary_snapshots(void)
{
    primary_snapshots[0].image = egui_view_image_icon_get_default_image();
    primary_snapshots[1].image = egui_view_image_icon_get_warm_image();
    primary_snapshots[2].image = egui_view_image_icon_get_fresh_image();
}

static void apply_primary_snapshot(uint8_t index)
{
    const image_icon_snapshot_t *snapshot = &primary_snapshots[index % EGUI_ARRAY_SIZE(primary_snapshots)];

    egui_view_label_set_text(EGUI_VIEW_OF(&primary_heading_label), snapshot->heading);
    egui_view_image_icon_set_image(EGUI_VIEW_OF(&primary_icon), snapshot->image);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_name_label), snapshot->image_name);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_name_label), snapshot->image_name_color, EGUI_ALPHA_100);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_note_label), snapshot->note);
}

static void apply_preview_states(void)
{
    egui_view_image_icon_set_image(EGUI_VIEW_OF(&warm_icon), egui_view_image_icon_get_warm_image());
    egui_view_image_icon_set_image(EGUI_VIEW_OF(&fresh_icon), egui_view_image_icon_get_fresh_image());
}

void test_init_ui(void)
{
    init_primary_snapshots();

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), IMAGE_ICON_ROOT_WIDTH, IMAGE_ICON_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, IMAGE_ICON_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    init_panel(&primary_panel, IMAGE_ICON_PANEL_WIDTH, IMAGE_ICON_PANEL_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_panel), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_panel));

    init_text_label(&primary_heading_label, 176, 12, "Project cover", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x51606F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_heading_label));

    egui_view_image_icon_init(EGUI_VIEW_OF(&primary_icon));
    egui_view_set_size(EGUI_VIEW_OF(&primary_icon), IMAGE_ICON_PRIMARY_SIZE, IMAGE_ICON_PRIMARY_SIZE);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_icon), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_icon));

    init_text_label(&primary_name_label, 176, 12, "Thumbnail", (const egui_font_t *)&egui_res_font_montserrat_10_4, EGUI_COLOR_HEX(0x0F6CBD),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_name_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_name_label));

    init_text_label(&primary_note_label, 176, 12, "Readable picture detail at icon size.",
                    (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_note_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), IMAGE_ICON_BOTTOM_ROW_WIDTH, IMAGE_ICON_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    init_panel(&warm_panel, IMAGE_ICON_PREVIEW_WIDTH, IMAGE_ICON_PREVIEW_HEIGHT, EGUI_BG_OF(&bg_muted_panel), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&warm_panel));

    init_text_label(&warm_heading_label, 84, 12, "Warm", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&warm_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&warm_panel), EGUI_VIEW_OF(&warm_heading_label));

    egui_view_image_icon_init(EGUI_VIEW_OF(&warm_icon));
    egui_view_set_size(EGUI_VIEW_OF(&warm_icon), IMAGE_ICON_PREVIEW_SIZE, IMAGE_ICON_PREVIEW_SIZE);
    egui_view_set_margin(EGUI_VIEW_OF(&warm_icon), 0, 0, 0, 4);
    egui_view_image_icon_override_static_preview_api(EGUI_VIEW_OF(&warm_icon), &warm_icon_api);
    egui_view_group_add_child(EGUI_VIEW_OF(&warm_panel), EGUI_VIEW_OF(&warm_icon));

    init_text_label(&warm_body_label, 84, 10, "Editorial", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&warm_panel), EGUI_VIEW_OF(&warm_body_label));

    init_panel(&fresh_panel, IMAGE_ICON_PREVIEW_WIDTH, IMAGE_ICON_PREVIEW_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&fresh_panel), 8, 0, 0, 0);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&fresh_panel));

    init_text_label(&fresh_heading_label, 84, 12, "Fresh", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&fresh_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&fresh_panel), EGUI_VIEW_OF(&fresh_heading_label));

    egui_view_image_icon_init(EGUI_VIEW_OF(&fresh_icon));
    egui_view_set_size(EGUI_VIEW_OF(&fresh_icon), IMAGE_ICON_PREVIEW_SIZE, IMAGE_ICON_PREVIEW_SIZE);
    egui_view_set_margin(EGUI_VIEW_OF(&fresh_icon), 0, 0, 0, 4);
    egui_view_image_icon_override_static_preview_api(EGUI_VIEW_OF(&fresh_icon), &fresh_icon_api);
    egui_view_group_add_child(EGUI_VIEW_OF(&fresh_panel), EGUI_VIEW_OF(&fresh_icon));

    init_text_label(&fresh_body_label, 84, 10, "Shared", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&fresh_panel), EGUI_VIEW_OF(&fresh_body_label));

    apply_primary_snapshot(0);
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&primary_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&warm_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&fresh_panel));
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
            apply_preview_states();
        }
        EGUI_SIM_SET_WAIT(p_action, IMAGE_ICON_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, IMAGE_ICON_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, IMAGE_ICON_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, IMAGE_ICON_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, IMAGE_ICON_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, IMAGE_ICON_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_snapshot(0);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 520);
        return true;
    default:
        return false;
    }
}
#endif
