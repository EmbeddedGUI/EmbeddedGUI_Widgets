#include <string.h>

#include "egui.h"
#include "egui_view_rich_edit_box.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define RICH_EDIT_BOX_ROOT_WIDTH        224
#define RICH_EDIT_BOX_ROOT_HEIGHT       270
#define RICH_EDIT_BOX_PRIMARY_WIDTH     196
#define RICH_EDIT_BOX_PRIMARY_HEIGHT    146
#define RICH_EDIT_BOX_PREVIEW_WIDTH     104
#define RICH_EDIT_BOX_PREVIEW_HEIGHT    82
#define RICH_EDIT_BOX_BOTTOM_ROW_WIDTH  216
#define RICH_EDIT_BOX_BOTTOM_ROW_HEIGHT 82
#define RICH_EDIT_BOX_RECORD_WAIT       90
#define RICH_EDIT_BOX_RECORD_FRAME_WAIT 170
#define RICH_EDIT_BOX_RECORD_FINAL_WAIT 320

#define PRIMARY_DOCUMENT_COUNT ((uint8_t)(sizeof(primary_documents) / sizeof(primary_documents[0])))
#define COMPACT_DOCUMENT_COUNT ((uint8_t)(sizeof(compact_documents) / sizeof(compact_documents[0])))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_rich_edit_box_t rich_edit_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_rich_edit_box_t rich_edit_compact;
static egui_view_rich_edit_box_t rich_edit_read_only;
static egui_view_api_t rich_edit_compact_api;
static egui_view_api_t rich_edit_read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Rich Edit Box";

static const egui_view_rich_edit_box_preset_t primary_presets[] = {
        {"Body", "Draft paragraph", EGUI_VIEW_RICH_EDIT_BOX_STYLE_BODY},
        {"Callout", "Highlight note", EGUI_VIEW_RICH_EDIT_BOX_STYLE_CALLOUT},
        {"Checklist", "Task bullets", EGUI_VIEW_RICH_EDIT_BOX_STYLE_CHECKLIST},
};

static const egui_view_rich_edit_box_document_t primary_documents[] = {
        {"OPS", "Shift briefing", "Summarize the current handoff in one surface.", "Live handoff", "Line check complete.", primary_presets, 3, 0},
        {"QA", "Bug scrub", "Use preset pills to change the current paragraph tone.", "Review notes", "Investigate focus ring drift.", primary_presets, 3, 1},
        {"REL", "Launch checklist", "Compact preview mirrors the same document family.", "Release steps", "Ship build\nVerify smoke", primary_presets, 3, 2},
};

static const egui_view_rich_edit_box_preset_t compact_presets[] = {
        {"Body", "", EGUI_VIEW_RICH_EDIT_BOX_STYLE_BODY},
        {"Callout", "", EGUI_VIEW_RICH_EDIT_BOX_STYLE_CALLOUT},
        {"List", "", EGUI_VIEW_RICH_EDIT_BOX_STYLE_CHECKLIST},
};

static const egui_view_rich_edit_box_document_t compact_documents[] = {
        {"UI", "Compact note", "", "", "Hold draft.", compact_presets, 3, 0},
        {"QA", "Compact list", "", "", "Tap preset.", compact_presets, 3, 2},
};

static const egui_view_rich_edit_box_document_t read_only_document = {
        "LOCK", "Read only draft", "", "Preview only", "Callout stays muted.", compact_presets, 3, 1};

static void apply_primary_document(uint8_t index)
{
    egui_view_rich_edit_box_set_current_document(EGUI_VIEW_OF(&rich_edit_primary), index % PRIMARY_DOCUMENT_COUNT);
}

static void apply_compact_document(uint8_t index)
{
    egui_view_rich_edit_box_set_current_document(EGUI_VIEW_OF(&rich_edit_compact), index % COMPACT_DOCUMENT_COUNT);
}

static void dismiss_primary_rich_edit_focus(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_clear_focus(EGUI_VIEW_OF(&rich_edit_primary));
#endif
}

static int dismiss_primary_focus_on_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

    if (event->type == EGUI_MOTION_EVENT_ACTION_DOWN)
    {
        dismiss_primary_rich_edit_focus();
    }
    return 1;
}

#if EGUI_CONFIG_RECORDING_TEST
static void request_page_snapshot(void)
{
    egui_view_invalidate(EGUI_VIEW_OF(&root_layout));
    recording_request_snapshot();
}

static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event;

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&rich_edit_primary));
#endif
    memset(&event, 0, sizeof(event));
    event.key_code = key_code;
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    egui_view_dispatch_key_event(EGUI_VIEW_OF(&rich_edit_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    egui_view_dispatch_key_event(EGUI_VIEW_OF(&rich_edit_primary), &event);
}

static void set_click_part(egui_sim_action_t *p_action, egui_view_t *view, uint8_t part, int interval_ms)
{
    egui_region_t region;

    if (!egui_view_rich_edit_box_get_part_region(view, part, &region))
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
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), RICH_EDIT_BOX_ROOT_WIDTH, RICH_EDIT_BOX_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), RICH_EDIT_BOX_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_rich_edit_box_init(EGUI_VIEW_OF(&rich_edit_primary));
    egui_view_set_size(EGUI_VIEW_OF(&rich_edit_primary), RICH_EDIT_BOX_PRIMARY_WIDTH, RICH_EDIT_BOX_PRIMARY_HEIGHT);
    egui_view_rich_edit_box_set_documents(EGUI_VIEW_OF(&rich_edit_primary), primary_documents, PRIMARY_DOCUMENT_COUNT);
    egui_view_rich_edit_box_set_font(EGUI_VIEW_OF(&rich_edit_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_rich_edit_box_set_meta_font(EGUI_VIEW_OF(&rich_edit_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_rich_edit_box_set_palette(EGUI_VIEW_OF(&rich_edit_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DDE5), EGUI_COLOR_HEX(0xFBFCFE),
                                        EGUI_COLOR_HEX(0x1B2834), EGUI_COLOR_HEX(0x6C7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xDCE5EE));
    egui_view_set_margin(EGUI_VIEW_OF(&rich_edit_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&rich_edit_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), RICH_EDIT_BOX_BOTTOM_ROW_WIDTH, RICH_EDIT_BOX_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_rich_edit_box_init(EGUI_VIEW_OF(&rich_edit_compact));
    egui_view_set_size(EGUI_VIEW_OF(&rich_edit_compact), RICH_EDIT_BOX_PREVIEW_WIDTH, RICH_EDIT_BOX_PREVIEW_HEIGHT);
    egui_view_rich_edit_box_set_documents(EGUI_VIEW_OF(&rich_edit_compact), compact_documents, COMPACT_DOCUMENT_COUNT);
    egui_view_rich_edit_box_set_font(EGUI_VIEW_OF(&rich_edit_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_rich_edit_box_set_meta_font(EGUI_VIEW_OF(&rich_edit_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_rich_edit_box_set_compact_mode(EGUI_VIEW_OF(&rich_edit_compact), 1);
    egui_view_rich_edit_box_set_palette(EGUI_VIEW_OF(&rich_edit_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xCFE0DC), EGUI_COLOR_HEX(0xF7FBFA),
                                        EGUI_COLOR_HEX(0x183332), EGUI_COLOR_HEX(0x5E7B76), EGUI_COLOR_HEX(0x0D9488), EGUI_COLOR_HEX(0xD9ECE7));
    egui_view_rich_edit_box_override_static_preview_api(EGUI_VIEW_OF(&rich_edit_compact), &rich_edit_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    rich_edit_compact_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&rich_edit_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&rich_edit_compact));

    egui_view_rich_edit_box_init(EGUI_VIEW_OF(&rich_edit_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&rich_edit_read_only), RICH_EDIT_BOX_PREVIEW_WIDTH, RICH_EDIT_BOX_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&rich_edit_read_only), 8, 0, 0, 0);
    egui_view_rich_edit_box_set_documents(EGUI_VIEW_OF(&rich_edit_read_only), &read_only_document, 1);
    egui_view_rich_edit_box_set_font(EGUI_VIEW_OF(&rich_edit_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_rich_edit_box_set_meta_font(EGUI_VIEW_OF(&rich_edit_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_rich_edit_box_set_compact_mode(EGUI_VIEW_OF(&rich_edit_read_only), 1);
    egui_view_rich_edit_box_set_read_only_mode(EGUI_VIEW_OF(&rich_edit_read_only), 1);
    egui_view_rich_edit_box_set_palette(EGUI_VIEW_OF(&rich_edit_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xD9E1E8), EGUI_COLOR_HEX(0xF7F9FB),
                                        EGUI_COLOR_HEX(0x556575), EGUI_COLOR_HEX(0x8997A4), EGUI_COLOR_HEX(0xA3B2BE), EGUI_COLOR_HEX(0xE4EBF1));
    egui_view_rich_edit_box_override_static_preview_api(EGUI_VIEW_OF(&rich_edit_read_only), &rich_edit_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    rich_edit_read_only_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&rich_edit_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&rich_edit_read_only));

    apply_primary_document(0);
    apply_compact_document(0);

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&rich_edit_primary));
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
            apply_primary_document(0);
            apply_compact_document(0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&rich_edit_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, RICH_EDIT_BOX_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RICH_EDIT_BOX_RECORD_FRAME_WAIT);
        return true;
    case 2:
        set_click_part(p_action, EGUI_VIEW_OF(&rich_edit_primary), EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR, RICH_EDIT_BOX_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RICH_EDIT_BOX_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_R);
            apply_primary_key(EGUI_KEY_CODE_E);
            apply_primary_key(EGUI_KEY_CODE_V);
            apply_primary_key(EGUI_KEY_CODE_I);
            apply_primary_key(EGUI_KEY_CODE_E);
            apply_primary_key(EGUI_KEY_CODE_W);
        }
        EGUI_SIM_SET_WAIT(p_action, RICH_EDIT_BOX_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RICH_EDIT_BOX_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_TAB);
            apply_primary_key(EGUI_KEY_CODE_RIGHT);
            apply_primary_key(EGUI_KEY_CODE_ENTER);
        }
        EGUI_SIM_SET_WAIT(p_action, RICH_EDIT_BOX_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RICH_EDIT_BOX_RECORD_FRAME_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            apply_primary_document(2);
            apply_compact_document(1);
        }
        EGUI_SIM_SET_WAIT(p_action, RICH_EDIT_BOX_RECORD_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RICH_EDIT_BOX_RECORD_FRAME_WAIT);
        return true;
    case 10:
        set_click_view_center(p_action, EGUI_VIEW_OF(&rich_edit_compact), RICH_EDIT_BOX_RECORD_WAIT);
        return true;
    case 11:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RICH_EDIT_BOX_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
