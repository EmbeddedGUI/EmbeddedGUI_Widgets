#include "egui.h"
#include "egui_view_bitmap_icon.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define BITMAP_ICON_ROOT_WIDTH        224
#define BITMAP_ICON_ROOT_HEIGHT       224
#define BITMAP_ICON_PANEL_WIDTH       196
#define BITMAP_ICON_PANEL_HEIGHT      112
#define BITMAP_ICON_PRIMARY_SIZE      52
#define BITMAP_ICON_PREVIEW_WIDTH     104
#define BITMAP_ICON_PREVIEW_HEIGHT    72
#define BITMAP_ICON_PREVIEW_SIZE      28
#define BITMAP_ICON_BOTTOM_ROW_WIDTH  216
#define BITMAP_ICON_BOTTOM_ROW_HEIGHT 72
#define BITMAP_ICON_RECORD_WAIT       90
#define BITMAP_ICON_RECORD_FRAME_WAIT 170

typedef struct
{
    const char *heading;
    const egui_image_t *image;
    const char *image_name;
    egui_color_t image_name_color;
    const char *note;
} bitmap_icon_snapshot_t;

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_linearlayout_t primary_panel;
static egui_view_label_t primary_heading_label;
static egui_view_bitmap_icon_t primary_icon;
static egui_view_label_t primary_name_label;
static egui_view_label_t primary_note_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t subtle_panel;
static egui_view_label_t subtle_heading_label;
static egui_view_bitmap_icon_t subtle_icon;
static egui_view_label_t subtle_body_label;
static egui_view_linearlayout_t accent_panel;
static egui_view_label_t accent_heading_label;
static egui_view_bitmap_icon_t accent_icon;
static egui_view_label_t accent_body_label;
static egui_view_api_t subtle_icon_api;
static egui_view_api_t accent_icon_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_surface_panel_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_surface_panel_params, &bg_surface_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_surface_panel, &bg_surface_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_muted_panel_param, EGUI_COLOR_HEX(0xFBFCFD), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_muted_panel_params, &bg_muted_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_muted_panel, &bg_muted_panel_params);

static const char *title_text = "BitmapIcon";

static bitmap_icon_snapshot_t primary_snapshots[] = {
        {
                "Document asset",
                NULL,
                "Document",
                EGUI_COLOR_HEX(0x0F6CBD),
                "Foreground tint carries the meaning.",
        },
        {
                "Message cue",
                NULL,
                "Mail",
                EGUI_COLOR_HEX(0x0F7B45),
                "Mask sources swap without new layout.",
        },
        {
                "Attention callout",
                NULL,
                "Alert",
                EGUI_COLOR_HEX(0xC42B1C),
                "Alpha masks stay crisp at icon size.",
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
    egui_view_set_padding(EGUI_VIEW_OF(panel), 6, 6, 6, 6);
}

static void init_primary_snapshots(void)
{
    primary_snapshots[0].image = egui_view_bitmap_icon_get_document_image();
    primary_snapshots[1].image = egui_view_bitmap_icon_get_mail_image();
    primary_snapshots[2].image = egui_view_bitmap_icon_get_alert_image();
}

static void apply_primary_snapshot(uint8_t index)
{
    const bitmap_icon_snapshot_t *snapshot = &primary_snapshots[index % EGUI_ARRAY_SIZE(primary_snapshots)];

    egui_view_label_set_text(EGUI_VIEW_OF(&primary_heading_label), snapshot->heading);
    egui_view_bitmap_icon_set_image(EGUI_VIEW_OF(&primary_icon), snapshot->image);
    egui_view_bitmap_icon_set_palette(EGUI_VIEW_OF(&primary_icon), snapshot->image_name_color);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_name_label), snapshot->image_name);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&primary_name_label), snapshot->image_name_color, EGUI_ALPHA_100);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_note_label), snapshot->note);
}

static void apply_preview_states(void)
{
    egui_view_bitmap_icon_set_image(EGUI_VIEW_OF(&subtle_icon), egui_view_bitmap_icon_get_mail_image());
    egui_view_bitmap_icon_apply_subtle_style(EGUI_VIEW_OF(&subtle_icon));

    egui_view_bitmap_icon_set_image(EGUI_VIEW_OF(&accent_icon), egui_view_bitmap_icon_get_alert_image());
    egui_view_bitmap_icon_apply_accent_style(EGUI_VIEW_OF(&accent_icon));
}

void test_init_ui(void)
{
    init_primary_snapshots();

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), BITMAP_ICON_ROOT_WIDTH, BITMAP_ICON_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, BITMAP_ICON_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    init_panel(&primary_panel, BITMAP_ICON_PANEL_WIDTH, BITMAP_ICON_PANEL_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_panel), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_panel));

    init_text_label(&primary_heading_label, 176, 12, "Document asset", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x51606F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_heading_label), 0, 0, 0, 2);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_heading_label));

    egui_view_bitmap_icon_init(EGUI_VIEW_OF(&primary_icon));
    egui_view_set_size(EGUI_VIEW_OF(&primary_icon), BITMAP_ICON_PRIMARY_SIZE, BITMAP_ICON_PRIMARY_SIZE);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_icon), 0, 0, 0, 3);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_icon));

    init_text_label(&primary_name_label, 176, 12, "Document", (const egui_font_t *)&egui_res_font_montserrat_10_4, EGUI_COLOR_HEX(0x0F6CBD),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_name_label), 0, 0, 0, 2);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_name_label));

    init_text_label(&primary_note_label, 176, 18, "Foreground tint carries the meaning.",
                    (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_note_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), BITMAP_ICON_BOTTOM_ROW_WIDTH, BITMAP_ICON_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    init_panel(&subtle_panel, BITMAP_ICON_PREVIEW_WIDTH, BITMAP_ICON_PREVIEW_HEIGHT, EGUI_BG_OF(&bg_muted_panel), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&subtle_panel));

    init_text_label(&subtle_heading_label, 84, 12, "Subtle", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&subtle_heading_label), 0, 0, 0, 2);
    egui_view_group_add_child(EGUI_VIEW_OF(&subtle_panel), EGUI_VIEW_OF(&subtle_heading_label));

    egui_view_bitmap_icon_init(EGUI_VIEW_OF(&subtle_icon));
    egui_view_set_size(EGUI_VIEW_OF(&subtle_icon), BITMAP_ICON_PREVIEW_SIZE, BITMAP_ICON_PREVIEW_SIZE);
    egui_view_set_margin(EGUI_VIEW_OF(&subtle_icon), 0, 0, 0, 4);
    egui_view_bitmap_icon_override_static_preview_api(EGUI_VIEW_OF(&subtle_icon), &subtle_icon_api);
    egui_view_group_add_child(EGUI_VIEW_OF(&subtle_panel), EGUI_VIEW_OF(&subtle_icon));

    init_text_label(&subtle_body_label, 84, 12, "Mail", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&subtle_panel), EGUI_VIEW_OF(&subtle_body_label));

    init_panel(&accent_panel, BITMAP_ICON_PREVIEW_WIDTH, BITMAP_ICON_PREVIEW_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&accent_panel), 8, 0, 0, 0);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&accent_panel));

    init_text_label(&accent_heading_label, 84, 12, "Accent", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&accent_heading_label), 0, 0, 0, 2);
    egui_view_group_add_child(EGUI_VIEW_OF(&accent_panel), EGUI_VIEW_OF(&accent_heading_label));

    egui_view_bitmap_icon_init(EGUI_VIEW_OF(&accent_icon));
    egui_view_set_size(EGUI_VIEW_OF(&accent_icon), BITMAP_ICON_PREVIEW_SIZE, BITMAP_ICON_PREVIEW_SIZE);
    egui_view_set_margin(EGUI_VIEW_OF(&accent_icon), 0, 0, 0, 4);
    egui_view_bitmap_icon_override_static_preview_api(EGUI_VIEW_OF(&accent_icon), &accent_icon_api);
    egui_view_group_add_child(EGUI_VIEW_OF(&accent_panel), EGUI_VIEW_OF(&accent_icon));

    init_text_label(&accent_body_label, 84, 12, "Alert", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&accent_panel), EGUI_VIEW_OF(&accent_body_label));

    apply_primary_snapshot(0);
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&primary_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&subtle_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&accent_panel));
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
        EGUI_SIM_SET_WAIT(p_action, BITMAP_ICON_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, BITMAP_ICON_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, BITMAP_ICON_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, BITMAP_ICON_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, BITMAP_ICON_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, BITMAP_ICON_RECORD_FRAME_WAIT);
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
