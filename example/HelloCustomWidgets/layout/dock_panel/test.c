#include "egui.h"
#include "egui_view_dock_panel.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define DOCK_PANEL_ROOT_WIDTH            224
#define DOCK_PANEL_ROOT_HEIGHT           236
#define DOCK_PANEL_PRIMARY_PANEL_WIDTH   196
#define DOCK_PANEL_PRIMARY_PANEL_HEIGHT  118
#define DOCK_PANEL_PRIMARY_DOCK_WIDTH    176
#define DOCK_PANEL_PRIMARY_DOCK_HEIGHT   64
#define DOCK_PANEL_PREVIEW_PANEL_WIDTH   104
#define DOCK_PANEL_PREVIEW_PANEL_HEIGHT  76
#define DOCK_PANEL_PREVIEW_DOCK_WIDTH    84
#define DOCK_PANEL_PREVIEW_DOCK_HEIGHT   32
#define DOCK_PANEL_BOTTOM_ROW_WIDTH      216
#define DOCK_PANEL_BOTTOM_ROW_HEIGHT     76
#define DOCK_PANEL_ITEM_CAPACITY         4
#define DOCK_PANEL_RAIL_ITEM_COUNT       3
#define DOCK_PANEL_FOOTER_ITEM_COUNT     2
#define DOCK_PANEL_RECORD_WAIT           90
#define DOCK_PANEL_RECORD_FRAME_WAIT     170
#define DOCK_PANEL_RECORD_FINAL_WAIT     520
#define DOCK_PANEL_DEFAULT_SNAPSHOT      1

typedef enum
{
    DOCK_ITEM_TONE_NEUTRAL = 0,
    DOCK_ITEM_TONE_ACCENT,
    DOCK_ITEM_TONE_WARM,
} dock_item_tone_t;

typedef struct
{
    const char *title;
    dock_item_tone_t tone;
    uint8_t dock_type;
    egui_dim_t width;
    egui_dim_t height;
} dock_item_t;

typedef struct
{
    const char *heading;
    const char *note;
    uint8_t use_compact_style;
    uint8_t last_child_fill;
    uint8_t item_count;
    dock_item_t items[DOCK_PANEL_ITEM_CAPACITY];
} dock_snapshot_t;

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_linearlayout_t primary_panel;
static egui_view_label_t primary_heading_label;
static hcw_dock_panel_t primary_dock_panel;
static egui_view_linearlayout_t primary_cards[DOCK_PANEL_ITEM_CAPACITY];
static egui_view_label_t primary_card_titles[DOCK_PANEL_ITEM_CAPACITY];
static egui_view_label_t primary_note_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t rail_panel;
static egui_view_label_t rail_heading_label;
static hcw_dock_panel_t rail_preview_panel;
static egui_view_linearlayout_t rail_cards[DOCK_PANEL_RAIL_ITEM_COUNT];
static egui_view_label_t rail_card_titles[DOCK_PANEL_RAIL_ITEM_COUNT];
static egui_view_label_t rail_note_label;
static egui_view_linearlayout_t footer_panel;
static egui_view_label_t footer_heading_label;
static hcw_dock_panel_t footer_preview_panel;
static egui_view_linearlayout_t footer_cards[DOCK_PANEL_FOOTER_ITEM_COUNT];
static egui_view_label_t footer_card_titles[DOCK_PANEL_FOOTER_ITEM_COUNT];
static egui_view_label_t footer_note_label;
static egui_view_api_t rail_preview_api;
static egui_view_api_t footer_preview_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_surface_panel_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_surface_panel_params, &bg_surface_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_surface_panel, &bg_surface_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_muted_panel_param, EGUI_COLOR_HEX(0xFBFCFD), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_muted_panel_params, &bg_muted_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_muted_panel, &bg_muted_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_dock_standard_param, EGUI_COLOR_HEX(0xF9FBFD), EGUI_ALPHA_100, 10);
EGUI_BACKGROUND_PARAM_INIT(bg_dock_standard_params, &bg_dock_standard_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_dock_standard, &bg_dock_standard_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_dock_compact_param, EGUI_COLOR_HEX(0xF4F7FA), EGUI_ALPHA_100, 10);
EGUI_BACKGROUND_PARAM_INIT(bg_dock_compact_params, &bg_dock_compact_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_dock_compact, &bg_dock_compact_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_card_neutral_param, EGUI_COLOR_HEX(0xF4F7FA), EGUI_ALPHA_100, 10);
EGUI_BACKGROUND_PARAM_INIT(bg_card_neutral_params, &bg_card_neutral_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_card_neutral, &bg_card_neutral_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_card_accent_param, EGUI_COLOR_HEX(0xE8F1FB), EGUI_ALPHA_100, 10);
EGUI_BACKGROUND_PARAM_INIT(bg_card_accent_params, &bg_card_accent_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_card_accent, &bg_card_accent_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_card_warm_param, EGUI_COLOR_HEX(0xF9EEE2), EGUI_ALPHA_100, 10);
EGUI_BACKGROUND_PARAM_INIT(bg_card_warm_params, &bg_card_warm_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_card_warm, &bg_card_warm_params);

static const char *title_text = "DockPanel";

static const dock_snapshot_t primary_snapshots[] = {
        {
                "Inspector shell",
                "Docked bars reserve edges while the remaining pane stays calm.",
                0,
                1,
                4,
                {
                        {"Top", DOCK_ITEM_TONE_NEUTRAL, HCW_DOCK_PANEL_DOCK_TOP, 120, 14},
                        {"Nav", DOCK_ITEM_TONE_ACCENT, HCW_DOCK_PANEL_DOCK_LEFT, 36, 28},
                        {"Tools", DOCK_ITEM_TONE_WARM, HCW_DOCK_PANEL_DOCK_RIGHT, 42, 28},
                        {"Pane", DOCK_ITEM_TONE_NEUTRAL, HCW_DOCK_PANEL_DOCK_FILL, 60, 28},
                },
        },
        {
                "Reading pane",
                "Top, side and footer rails can frame a fill region without overlap.",
                0,
                0,
                4,
                {
                        {"Header", DOCK_ITEM_TONE_NEUTRAL, HCW_DOCK_PANEL_DOCK_TOP, 120, 14},
                        {"Outline", DOCK_ITEM_TONE_ACCENT, HCW_DOCK_PANEL_DOCK_LEFT, 40, 28},
                        {"Article", DOCK_ITEM_TONE_NEUTRAL, HCW_DOCK_PANEL_DOCK_FILL, 56, 28},
                        {"Actions", DOCK_ITEM_TONE_WARM, HCW_DOCK_PANEL_DOCK_BOTTOM, 120, 14},
                },
        },
        {
                "Compact tools",
                "Compact docking keeps a narrow shell readable without extra chrome.",
                1,
                0,
                4,
                {
                        {"Top", DOCK_ITEM_TONE_NEUTRAL, HCW_DOCK_PANEL_DOCK_TOP, 120, 10},
                        {"Rail", DOCK_ITEM_TONE_ACCENT, HCW_DOCK_PANEL_DOCK_LEFT, 34, 20},
                        {"Board", DOCK_ITEM_TONE_NEUTRAL, HCW_DOCK_PANEL_DOCK_FILL, 48, 20},
                        {"Foot", DOCK_ITEM_TONE_WARM, HCW_DOCK_PANEL_DOCK_BOTTOM, 120, 10},
                },
        },
};

static const dock_snapshot_t rail_preview_snapshot = {
        "Rail",
        "Static preview.",
        0,
        1,
        3,
        {
                {"T", DOCK_ITEM_TONE_NEUTRAL, HCW_DOCK_PANEL_DOCK_TOP, 64, 8},
                {"L", DOCK_ITEM_TONE_ACCENT, HCW_DOCK_PANEL_DOCK_LEFT, 18, 12},
                {"F", DOCK_ITEM_TONE_NEUTRAL, HCW_DOCK_PANEL_DOCK_FILL, 24, 12},
        },
};

static const dock_snapshot_t footer_preview_snapshot = {
        "Footer",
        "Quiet shell.",
        1,
        0,
        2,
        {
                {"F", DOCK_ITEM_TONE_NEUTRAL, HCW_DOCK_PANEL_DOCK_FILL, 40, 14},
                {"B", DOCK_ITEM_TONE_WARM, HCW_DOCK_PANEL_DOCK_BOTTOM, 64, 8},
        },
};

static egui_background_t *dock_card_get_background(dock_item_tone_t tone)
{
    switch (tone)
    {
    case DOCK_ITEM_TONE_ACCENT:
        return EGUI_BG_OF(&bg_card_accent);
    case DOCK_ITEM_TONE_WARM:
        return EGUI_BG_OF(&bg_card_warm);
    default:
        return EGUI_BG_OF(&bg_card_neutral);
    }
}

static egui_color_t dock_card_get_title_color(dock_item_tone_t tone)
{
    switch (tone)
    {
    case DOCK_ITEM_TONE_ACCENT:
        return EGUI_COLOR_HEX(0x0F5EA6);
    case DOCK_ITEM_TONE_WARM:
        return EGUI_COLOR_HEX(0x8C4A00);
    default:
        return EGUI_COLOR_HEX(0x21303F);
    }
}

static void init_text_label(egui_view_label_t *label, egui_dim_t width, egui_dim_t height, const char *text, const egui_font_t *font, egui_color_t color,
                            uint8_t align_type)
{
    egui_view_label_init(EGUI_VIEW_OF(label));
    egui_view_set_size(EGUI_VIEW_OF(label), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(label), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(label), align_type);
    egui_view_label_set_font(EGUI_VIEW_OF(label), font);
    egui_view_label_set_font_color(EGUI_VIEW_OF(label), color, EGUI_ALPHA_100);
}

static void init_panel(egui_view_linearlayout_t *panel, egui_dim_t width, egui_dim_t height, egui_background_t *background, uint8_t align_type)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(panel));
    egui_view_set_size(EGUI_VIEW_OF(panel), width, height);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(panel), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(panel), align_type);
    egui_view_set_background(EGUI_VIEW_OF(panel), background);
    egui_view_set_padding(EGUI_VIEW_OF(panel), 10, 10, 10, 10);
}

static void init_card(egui_view_linearlayout_t *card, egui_view_label_t *title_value, egui_dim_t width, egui_dim_t height)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(card));
    egui_view_set_size(EGUI_VIEW_OF(card), width, height);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(card), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(card), EGUI_ALIGN_CENTER);
    egui_view_set_padding(EGUI_VIEW_OF(card), 4, 4, 2, 2);
    egui_view_set_background(EGUI_VIEW_OF(card), EGUI_BG_OF(&bg_card_neutral));

    init_text_label(title_value, width - 8, height - 4, "", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x21303F), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(card), EGUI_VIEW_OF(title_value));
}

static void set_card_state(egui_view_linearlayout_t *card, egui_view_label_t *title_value, const dock_item_t *item, uint8_t visible)
{
    if (!visible)
    {
        egui_view_set_gone(EGUI_VIEW_OF(card), 1);
        return;
    }

    egui_view_set_gone(EGUI_VIEW_OF(card), 0);
    egui_view_set_size(EGUI_VIEW_OF(card), item->width, item->height);
    egui_view_set_background(EGUI_VIEW_OF(card), dock_card_get_background(item->tone));
    egui_view_set_size(EGUI_VIEW_OF(title_value), item->width - 8, item->height - 4);
    egui_view_label_set_text(EGUI_VIEW_OF(title_value), item->title);
    egui_view_label_set_font_color(EGUI_VIEW_OF(title_value), dock_card_get_title_color(item->tone), EGUI_ALPHA_100);
    hcw_dock_panel_set_child_dock(EGUI_VIEW_OF(card), item->dock_type);
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(card));
}

static void apply_dock_style_for_snapshot(egui_view_t *dock_panel, const dock_snapshot_t *snapshot)
{
    if (snapshot->use_compact_style)
    {
        hcw_dock_panel_apply_compact_style(dock_panel);
        egui_view_set_background(dock_panel, EGUI_BG_OF(&bg_dock_compact));
    }
    else
    {
        hcw_dock_panel_apply_standard_style(dock_panel);
        egui_view_set_background(dock_panel, EGUI_BG_OF(&bg_dock_standard));
    }
    hcw_dock_panel_set_last_child_fill(dock_panel, snapshot->last_child_fill);
}

static void apply_snapshot_to_dock_panel(egui_view_t *dock_panel, egui_view_linearlayout_t *cards, egui_view_label_t *titles, uint8_t capacity,
                                         const dock_snapshot_t *snapshot)
{
    uint8_t index;

    apply_dock_style_for_snapshot(dock_panel, snapshot);
    for (index = 0; index < capacity; index++)
    {
        uint8_t visible = index < snapshot->item_count;
        set_card_state(&cards[index], &titles[index], &snapshot->items[index], visible);
    }
    hcw_dock_panel_layout_childs(dock_panel);
}

static void layout_card_set(egui_view_linearlayout_t *cards, uint8_t capacity)
{
    uint8_t index;

    for (index = 0; index < capacity; index++)
    {
        if (!EGUI_VIEW_OF(&cards[index])->is_gone)
        {
            egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&cards[index]));
        }
    }
}

static void layout_local_views(void)
{
    hcw_dock_panel_layout_childs(EGUI_VIEW_OF(&primary_dock_panel));
    hcw_dock_panel_layout_childs(EGUI_VIEW_OF(&rail_preview_panel));
    hcw_dock_panel_layout_childs(EGUI_VIEW_OF(&footer_preview_panel));
    layout_card_set(primary_cards, DOCK_PANEL_ITEM_CAPACITY);
    layout_card_set(rail_cards, DOCK_PANEL_RAIL_ITEM_COUNT);
    layout_card_set(footer_cards, DOCK_PANEL_FOOTER_ITEM_COUNT);
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&primary_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&rail_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&footer_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));
}

#if EGUI_CONFIG_RECORDING_TEST
static void refresh_root_layout(void)
{
    layout_local_views();
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}
#endif

static void apply_primary_snapshot(uint8_t index)
{
    const dock_snapshot_t *snapshot = &primary_snapshots[index % EGUI_ARRAY_SIZE(primary_snapshots)];

    egui_view_label_set_text(EGUI_VIEW_OF(&primary_heading_label), snapshot->heading);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_note_label), snapshot->note);
    apply_snapshot_to_dock_panel(EGUI_VIEW_OF(&primary_dock_panel), primary_cards, primary_card_titles, DOCK_PANEL_ITEM_CAPACITY, snapshot);
}

static void apply_preview_states(void)
{
    apply_snapshot_to_dock_panel(EGUI_VIEW_OF(&rail_preview_panel), rail_cards, rail_card_titles, DOCK_PANEL_RAIL_ITEM_COUNT, &rail_preview_snapshot);
    apply_snapshot_to_dock_panel(EGUI_VIEW_OF(&footer_preview_panel), footer_cards, footer_card_titles, DOCK_PANEL_FOOTER_ITEM_COUNT, &footer_preview_snapshot);
}

void test_init_ui(void)
{
    uint8_t index;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), DOCK_PANEL_ROOT_WIDTH, DOCK_PANEL_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, DOCK_PANEL_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 0, 8, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    init_panel(&primary_panel, DOCK_PANEL_PRIMARY_PANEL_WIDTH, DOCK_PANEL_PRIMARY_PANEL_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_panel), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_panel));

    init_text_label(&primary_heading_label, 176, 12, primary_snapshots[DOCK_PANEL_DEFAULT_SNAPSHOT].heading, (const egui_font_t *)&egui_res_font_montserrat_8_4,
                    EGUI_COLOR_HEX(0x5E6D7C), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_heading_label));

    hcw_dock_panel_init(EGUI_VIEW_OF(&primary_dock_panel));
    egui_view_set_size(EGUI_VIEW_OF(&primary_dock_panel), DOCK_PANEL_PRIMARY_DOCK_WIDTH, DOCK_PANEL_PRIMARY_DOCK_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_dock_panel), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_dock_panel));

    for (index = 0; index < DOCK_PANEL_ITEM_CAPACITY; index++)
    {
        init_card(&primary_cards[index], &primary_card_titles[index], 60, 18);
        egui_view_group_add_child(EGUI_VIEW_OF(&primary_dock_panel), EGUI_VIEW_OF(&primary_cards[index]));
    }

    init_text_label(&primary_note_label, 176, 10, primary_snapshots[DOCK_PANEL_DEFAULT_SNAPSHOT].note,
                    (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_note_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), DOCK_PANEL_BOTTOM_ROW_WIDTH, DOCK_PANEL_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    init_panel(&rail_panel, DOCK_PANEL_PREVIEW_PANEL_WIDTH, DOCK_PANEL_PREVIEW_PANEL_HEIGHT, EGUI_BG_OF(&bg_muted_panel), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&rail_panel));

    init_text_label(&rail_heading_label, 84, 12, "Rail", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&rail_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&rail_panel), EGUI_VIEW_OF(&rail_heading_label));

    hcw_dock_panel_init(EGUI_VIEW_OF(&rail_preview_panel));
    egui_view_set_size(EGUI_VIEW_OF(&rail_preview_panel), DOCK_PANEL_PREVIEW_DOCK_WIDTH, DOCK_PANEL_PREVIEW_DOCK_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&rail_preview_panel), 0, 0, 0, 4);
    hcw_dock_panel_override_static_preview_api(EGUI_VIEW_OF(&rail_preview_panel), &rail_preview_api);
    egui_view_group_add_child(EGUI_VIEW_OF(&rail_panel), EGUI_VIEW_OF(&rail_preview_panel));

    for (index = 0; index < DOCK_PANEL_RAIL_ITEM_COUNT; index++)
    {
        init_card(&rail_cards[index], &rail_card_titles[index], 26, 10);
        egui_view_group_add_child(EGUI_VIEW_OF(&rail_preview_panel), EGUI_VIEW_OF(&rail_cards[index]));
    }

    init_text_label(&rail_note_label, 84, 10, "Static preview.", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89),
                    EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&rail_panel), EGUI_VIEW_OF(&rail_note_label));

    init_panel(&footer_panel, DOCK_PANEL_PREVIEW_PANEL_WIDTH, DOCK_PANEL_PREVIEW_PANEL_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&footer_panel), 8, 0, 0, 0);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&footer_panel));

    init_text_label(&footer_heading_label, 84, 12, "Footer", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&footer_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&footer_panel), EGUI_VIEW_OF(&footer_heading_label));

    hcw_dock_panel_init(EGUI_VIEW_OF(&footer_preview_panel));
    egui_view_set_size(EGUI_VIEW_OF(&footer_preview_panel), DOCK_PANEL_PREVIEW_DOCK_WIDTH, DOCK_PANEL_PREVIEW_DOCK_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&footer_preview_panel), 0, 0, 0, 4);
    hcw_dock_panel_override_static_preview_api(EGUI_VIEW_OF(&footer_preview_panel), &footer_preview_api);
    egui_view_group_add_child(EGUI_VIEW_OF(&footer_panel), EGUI_VIEW_OF(&footer_preview_panel));

    for (index = 0; index < DOCK_PANEL_FOOTER_ITEM_COUNT; index++)
    {
        init_card(&footer_cards[index], &footer_card_titles[index], 32, 10);
        egui_view_group_add_child(EGUI_VIEW_OF(&footer_preview_panel), EGUI_VIEW_OF(&footer_cards[index]));
    }

    init_text_label(&footer_note_label, 84, 10, "Quiet shell.", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89),
                    EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&footer_panel), EGUI_VIEW_OF(&footer_note_label));

    apply_preview_states();
    apply_primary_snapshot(DOCK_PANEL_DEFAULT_SNAPSHOT);

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
    apply_preview_states();
    apply_primary_snapshot(DOCK_PANEL_DEFAULT_SNAPSHOT);
    layout_local_views();
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
            apply_preview_states();
            apply_primary_snapshot(DOCK_PANEL_DEFAULT_SNAPSHOT);
            refresh_root_layout();
        }
        EGUI_SIM_SET_WAIT(p_action, DOCK_PANEL_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_preview_states();
            apply_primary_snapshot(DOCK_PANEL_DEFAULT_SNAPSHOT);
            refresh_root_layout();
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DOCK_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_snapshot(0);
            refresh_root_layout();
        }
        EGUI_SIM_SET_WAIT(p_action, DOCK_PANEL_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DOCK_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_snapshot(2);
            refresh_root_layout();
        }
        EGUI_SIM_SET_WAIT(p_action, DOCK_PANEL_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DOCK_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_snapshot(DOCK_PANEL_DEFAULT_SNAPSHOT);
            refresh_root_layout();
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DOCK_PANEL_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
