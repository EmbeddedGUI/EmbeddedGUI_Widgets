#include "egui.h"
#include "egui_view_grid.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define GRID_ROOT_WIDTH             224
#define GRID_ROOT_HEIGHT            214
#define GRID_PRIMARY_PANEL_WIDTH    196
#define GRID_PRIMARY_PANEL_HEIGHT   118
#define GRID_PRIMARY_GRID_WIDTH     176
#define GRID_PRIMARY_GRID_HEIGHT    68
#define GRID_PREVIEW_PANEL_WIDTH    104
#define GRID_PREVIEW_PANEL_HEIGHT   74
#define GRID_PREVIEW_GRID_WIDTH     84
#define GRID_PREVIEW_GRID_HEIGHT    38
#define GRID_BOTTOM_ROW_WIDTH       216
#define GRID_BOTTOM_ROW_HEIGHT      74
#define GRID_CARD_CAPACITY          6
#define GRID_STACK_PREVIEW_CARDS    2
#define GRID_DENSE_PREVIEW_CARDS    3
#define GRID_RECORD_WAIT            90
#define GRID_RECORD_FRAME_WAIT      170
#define GRID_RECORD_FINAL_WAIT      280
#define GRID_DEFAULT_SNAPSHOT       0

#define PRIMARY_SNAPSHOT_COUNT      ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef enum
{
    GRID_CARD_TONE_NEUTRAL = 0,
    GRID_CARD_TONE_ACCENT,
    GRID_CARD_TONE_WARM,
} grid_card_tone_t;

typedef struct
{
    const char *eyebrow;
    const char *title;
    grid_card_tone_t tone;
} grid_card_t;

typedef struct
{
    const char *heading;
    const char *note;
    uint8_t columns;
    uint8_t card_count;
    egui_dim_t card_width;
    egui_dim_t card_height;
    grid_card_t cards[GRID_CARD_CAPACITY];
} grid_snapshot_t;

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_linearlayout_t primary_panel;
static egui_view_label_t primary_heading_label;
static egui_view_gridlayout_t primary_grid;
static egui_view_linearlayout_t primary_cards[GRID_CARD_CAPACITY];
static egui_view_label_t primary_card_eyebrows[GRID_CARD_CAPACITY];
static egui_view_label_t primary_card_titles[GRID_CARD_CAPACITY];
static egui_view_label_t primary_note_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t stack_panel;
static egui_view_label_t stack_heading_label;
static egui_view_gridlayout_t stack_preview_grid;
static egui_view_linearlayout_t stack_cards[GRID_STACK_PREVIEW_CARDS];
static egui_view_label_t stack_card_eyebrows[GRID_STACK_PREVIEW_CARDS];
static egui_view_label_t stack_card_titles[GRID_STACK_PREVIEW_CARDS];
static egui_view_label_t stack_note_label;
static egui_view_linearlayout_t dense_panel;
static egui_view_label_t dense_heading_label;
static egui_view_gridlayout_t dense_preview_grid;
static egui_view_linearlayout_t dense_cards[GRID_DENSE_PREVIEW_CARDS];
static egui_view_label_t dense_card_eyebrows[GRID_DENSE_PREVIEW_CARDS];
static egui_view_label_t dense_card_titles[GRID_DENSE_PREVIEW_CARDS];
static egui_view_label_t dense_note_label;
static egui_view_api_t stack_preview_api;
static egui_view_api_t dense_preview_api;
static uint8_t ui_ready;
static char primary_heading_text[32];
static char primary_note_text[72];
static char primary_card_eyebrow_text[GRID_CARD_CAPACITY][16];
static char primary_card_title_text[GRID_CARD_CAPACITY][24];
static char stack_heading_text[16];
static char stack_note_text[24];
static char stack_card_eyebrow_text[GRID_STACK_PREVIEW_CARDS][8];
static char stack_card_title_text[GRID_STACK_PREVIEW_CARDS][16];
static char dense_heading_text[16];
static char dense_note_text[24];
static char dense_card_eyebrow_text[GRID_DENSE_PREVIEW_CARDS][8];
static char dense_card_title_text[GRID_DENSE_PREVIEW_CARDS][8];

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_surface_panel_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_surface_panel_params, &bg_surface_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_surface_panel, &bg_surface_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_muted_panel_param, EGUI_COLOR_HEX(0xFBFCFD), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_muted_panel_params, &bg_muted_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_muted_panel, &bg_muted_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_card_neutral_param, EGUI_COLOR_HEX(0xF4F7FA), EGUI_ALPHA_100, 10);
EGUI_BACKGROUND_PARAM_INIT(bg_card_neutral_params, &bg_card_neutral_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_card_neutral, &bg_card_neutral_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_card_accent_param, EGUI_COLOR_HEX(0xE8F1FB), EGUI_ALPHA_100, 10);
EGUI_BACKGROUND_PARAM_INIT(bg_card_accent_params, &bg_card_accent_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_card_accent, &bg_card_accent_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_card_warm_param, EGUI_COLOR_HEX(0xF9EEE2), EGUI_ALPHA_100, 10);
EGUI_BACKGROUND_PARAM_INIT(bg_card_warm_params, &bg_card_warm_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_card_warm, &bg_card_warm_params);

static const char *title_text = "Grid";

static uint8_t grid_has_text(const char *text)
{
    return (text != NULL && text[0] != '\0') ? 1 : 0;
}

static uint8_t grid_text_len(const char *text)
{
    uint8_t len = 0;

    if (text == NULL)
    {
        return 0;
    }

    while (text[len] != '\0')
    {
        len++;
    }
    return len;
}

static egui_dim_t grid_measure_text_width(const egui_font_t *font, const char *text)
{
    egui_dim_t width = 0;
    egui_dim_t height = 0;

    if (!grid_has_text(text) || font == NULL || font->api == NULL || font->api->get_str_size == NULL)
    {
        return 0;
    }

    font->api->get_str_size(font, text, 0, 0, &width, &height);
    return width;
}

static void grid_copy_elided(char *buffer, uint8_t buffer_size, const char *text, uint8_t max_chars)
{
    uint8_t copy_length;
    uint8_t index;
    uint8_t length;

    if (buffer == NULL || buffer_size == 0)
    {
        return;
    }

    buffer[0] = '\0';
    if (text == NULL || max_chars == 0)
    {
        return;
    }

    length = grid_text_len(text);
    if (length <= max_chars)
    {
        copy_length = length;
        if (copy_length >= buffer_size)
        {
            copy_length = buffer_size - 1;
        }
        for (index = 0; index < copy_length; ++index)
        {
            buffer[index] = text[index];
        }
        buffer[copy_length] = '\0';
        return;
    }

    if (max_chars <= 3)
    {
        copy_length = max_chars;
        if (copy_length >= buffer_size)
        {
            copy_length = buffer_size - 1;
        }
        for (index = 0; index < copy_length; ++index)
        {
            buffer[index] = '.';
        }
        buffer[copy_length] = '\0';
        return;
    }

    copy_length = max_chars - 3;
    if (copy_length > buffer_size - 4)
    {
        copy_length = buffer_size - 4;
    }
    for (index = 0; index < copy_length; ++index)
    {
        buffer[index] = text[index];
    }
    buffer[copy_length] = '.';
    buffer[copy_length + 1] = '.';
    buffer[copy_length + 2] = '.';
    buffer[copy_length + 3] = '\0';
}

static void grid_fit_text_to_width(const egui_font_t *font, const char *text, char *buffer, uint8_t buffer_size, egui_dim_t max_width,
                                   egui_dim_t fallback_char_width)
{
    uint8_t max_chars;

    if (buffer == NULL || buffer_size == 0)
    {
        return;
    }

    buffer[0] = '\0';
    if (!grid_has_text(text) || max_width <= 0)
    {
        return;
    }

    max_chars = grid_text_len(text);
    grid_copy_elided(buffer, buffer_size, text, max_chars);
    while (max_chars > 0)
    {
        egui_dim_t text_width = grid_measure_text_width(font, buffer);

        if (text_width <= 0)
        {
            text_width = (egui_dim_t)grid_text_len(buffer) * fallback_char_width;
        }
        if (text_width <= max_width)
        {
            break;
        }

        max_chars--;
        grid_copy_elided(buffer, buffer_size, text, max_chars);
    }
}

static void grid_set_fitted_label_text(egui_view_label_t *label, char *buffer, uint8_t buffer_size, const char *text, egui_dim_t fallback_char_width)
{
    egui_dim_t max_width;

    if (label == NULL || buffer == NULL || buffer_size == 0)
    {
        return;
    }

    max_width = EGUI_VIEW_OF(label)->region.size.width;
    grid_fit_text_to_width(label->font, text, buffer, buffer_size, max_width, fallback_char_width);
    egui_view_label_set_text(EGUI_VIEW_OF(label), buffer);
}

static const grid_snapshot_t primary_snapshots[] = {
        {
                "Two equal columns",
                "Related cards stay aligned with one quiet grid shell.",
                2,
                4,
                72,
                24,
                {
                        {"OPS", "Overview", GRID_CARD_TONE_ACCENT},
                        {"SYNC", "Queue", GRID_CARD_TONE_NEUTRAL},
                        {"DOCS", "Publish", GRID_CARD_TONE_WARM},
                        {"QA", "Review", GRID_CARD_TONE_NEUTRAL},
                },
        },
        {
                "Dense board",
                "Three columns compress secondary detail without losing order.",
                3,
                6,
                48,
                20,
                {
                        {"A", "Backlog", GRID_CARD_TONE_NEUTRAL},
                        {"B", "Signals", GRID_CARD_TONE_ACCENT},
                        {"C", "Risk", GRID_CARD_TONE_WARM},
                        {"D", "Owners", GRID_CARD_TONE_NEUTRAL},
                        {"E", "Builds", GRID_CARD_TONE_ACCENT},
                        {"F", "Ship", GRID_CARD_TONE_NEUTRAL},
                },
        },
        {
                "Review stack",
                "One column stretches summary rows for narrower surfaces.",
                1,
                3,
                156,
                20,
                {
                        {"OPS", "Rollout board", GRID_CARD_TONE_ACCENT},
                        {"QA", "Blocked items", GRID_CARD_TONE_WARM},
                        {"DOCS", "Publishing notes", GRID_CARD_TONE_NEUTRAL},
                },
        },
};

static const grid_snapshot_t stack_preview_snapshot = {
        "Stack",
        "Static preview.",
        1,
        2,
        70,
        14,
        {
                {"A", "Summary", GRID_CARD_TONE_NEUTRAL},
                {"B", "Notes", GRID_CARD_TONE_ACCENT},
        },
};

static const grid_snapshot_t dense_preview_snapshot = {
        "Dense",
        "Three columns.",
        3,
        3,
        20,
        14,
        {
                {"1", "A", GRID_CARD_TONE_NEUTRAL},
                {"2", "B", GRID_CARD_TONE_ACCENT},
                {"3", "C", GRID_CARD_TONE_WARM},
        },
};

static egui_background_t *grid_card_get_background(grid_card_tone_t tone)
{
    switch (tone)
    {
    case GRID_CARD_TONE_ACCENT:
        return EGUI_BG_OF(&bg_card_accent);
    case GRID_CARD_TONE_WARM:
        return EGUI_BG_OF(&bg_card_warm);
    default:
        return EGUI_BG_OF(&bg_card_neutral);
    }
}

static egui_color_t grid_card_get_eyebrow_color(grid_card_tone_t tone)
{
    switch (tone)
    {
    case GRID_CARD_TONE_ACCENT:
        return EGUI_COLOR_HEX(0x0F6CBD);
    case GRID_CARD_TONE_WARM:
        return EGUI_COLOR_HEX(0x8C4A00);
    default:
        return EGUI_COLOR_HEX(0x667687);
    }
}

static void init_text_label(egui_view_label_t *label, egui_dim_t width, egui_dim_t height, const char *text, const egui_font_t *font, egui_color_t color,
                            uint8_t align_type)
{
    egui_view_label_init(EGUI_VIEW_OF(label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(label), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(label), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(label), align_type);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(label), font);
    egui_view_label_set_font_color(EGUI_VIEW_OF(label), color, EGUI_ALPHA_100);
}

static void init_panel(egui_view_linearlayout_t *panel, egui_dim_t width, egui_dim_t height, egui_background_t *background, uint8_t align_type)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(panel), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(panel), width, height);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(panel), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(panel), align_type);
    egui_view_set_background(EGUI_VIEW_OF(panel), background);
    egui_view_set_padding(EGUI_VIEW_OF(panel), 10, 10, 10, 10);
}

static void init_card(egui_view_linearlayout_t *card, egui_view_label_t *eyebrow_label, egui_view_label_t *card_title_label, egui_dim_t width,
                      egui_dim_t height)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(card), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(card), width, height);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(card), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(card), EGUI_ALIGN_LEFT);
    egui_view_set_padding(EGUI_VIEW_OF(card), 6, 5, 6, 5);
    egui_view_set_background(EGUI_VIEW_OF(card), EGUI_BG_OF(&bg_card_neutral));

    init_text_label(eyebrow_label, width - 12, 8, "", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x667687),
                    EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(eyebrow_label), 0, 0, 0, 1);
    egui_view_group_add_child(EGUI_VIEW_OF(card), EGUI_VIEW_OF(eyebrow_label));

    init_text_label(card_title_label, width - 12, 10, "", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(card), EGUI_VIEW_OF(card_title_label));
}

static void set_card_state(egui_view_linearlayout_t *card, egui_view_label_t *eyebrow_label, char *eyebrow_buffer, uint8_t eyebrow_buffer_size,
                           egui_view_label_t *card_title_label, char *title_buffer, uint8_t title_buffer_size, const grid_card_t *item, egui_dim_t width,
                           egui_dim_t height, uint8_t visible)
{
    if (!visible)
    {
        egui_view_set_gone(EGUI_VIEW_OF(card), 1);
        return;
    }

    egui_view_set_gone(EGUI_VIEW_OF(card), 0);
    egui_view_set_size(EGUI_VIEW_OF(card), width, height);
    egui_view_set_background(EGUI_VIEW_OF(card), grid_card_get_background(item->tone));
    egui_view_set_padding(EGUI_VIEW_OF(card), 6, 5, 6, 5);

    egui_view_set_size(EGUI_VIEW_OF(eyebrow_label), width - 12, 8);
    grid_set_fitted_label_text(eyebrow_label, eyebrow_buffer, eyebrow_buffer_size, item->eyebrow, 4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(eyebrow_label), grid_card_get_eyebrow_color(item->tone), EGUI_ALPHA_100);

    egui_view_set_size(EGUI_VIEW_OF(card_title_label), width - 12, 10);
    grid_set_fitted_label_text(card_title_label, title_buffer, title_buffer_size, item->title, 4);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(card));
}

static void apply_snapshot_to_grid(egui_view_t *grid, egui_view_linearlayout_t *cards, egui_view_label_t *eyebrows, egui_view_label_t *titles,
                                   char *eyebrow_text, uint8_t eyebrow_text_size, char *card_title_text, uint8_t title_text_size, uint8_t capacity,
                                   const grid_snapshot_t *snapshot)
{
    uint8_t index;

    hcw_grid_set_columns(grid, snapshot->columns);
    for (index = 0; index < capacity; index++)
    {
        uint8_t visible = index < snapshot->card_count;
        set_card_state(&cards[index], &eyebrows[index], &eyebrow_text[index * eyebrow_text_size], eyebrow_text_size, &titles[index],
                       &card_title_text[index * title_text_size], title_text_size, &snapshot->cards[index], snapshot->card_width, snapshot->card_height,
                       visible);
    }
    hcw_grid_layout_childs(grid);
}

static void layout_local_views(void)
{
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&primary_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&stack_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&dense_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));
}

static void layout_page(void)
{
    layout_local_views();
    egui_core_layout_childs_user_root_view(uicode_get_core(), EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}

static void apply_primary_state(uint8_t index)
{
    const grid_snapshot_t *snapshot = &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT];

    grid_set_fitted_label_text(&primary_heading_label, primary_heading_text, sizeof(primary_heading_text), snapshot->heading, 4);
    grid_set_fitted_label_text(&primary_note_label, primary_note_text, sizeof(primary_note_text), snapshot->note, 4);
    apply_snapshot_to_grid(EGUI_VIEW_OF(&primary_grid), primary_cards, primary_card_eyebrows, primary_card_titles, &primary_card_eyebrow_text[0][0],
                           sizeof(primary_card_eyebrow_text[0]), &primary_card_title_text[0][0], sizeof(primary_card_title_text[0]), GRID_CARD_CAPACITY,
                           snapshot);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_state(GRID_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    grid_set_fitted_label_text(&stack_heading_label, stack_heading_text, sizeof(stack_heading_text), stack_preview_snapshot.heading, 4);
    grid_set_fitted_label_text(&stack_note_label, stack_note_text, sizeof(stack_note_text), stack_preview_snapshot.note, 4);
    grid_set_fitted_label_text(&dense_heading_label, dense_heading_text, sizeof(dense_heading_text), dense_preview_snapshot.heading, 4);
    grid_set_fitted_label_text(&dense_note_label, dense_note_text, sizeof(dense_note_text), dense_preview_snapshot.note, 4);

    apply_snapshot_to_grid(EGUI_VIEW_OF(&stack_preview_grid), stack_cards, stack_card_eyebrows, stack_card_titles, &stack_card_eyebrow_text[0][0],
                           sizeof(stack_card_eyebrow_text[0]), &stack_card_title_text[0][0], sizeof(stack_card_title_text[0]), GRID_STACK_PREVIEW_CARDS,
                           &stack_preview_snapshot);
    apply_snapshot_to_grid(EGUI_VIEW_OF(&dense_preview_grid), dense_cards, dense_card_eyebrows, dense_card_titles, &dense_card_eyebrow_text[0][0],
                           sizeof(dense_card_eyebrow_text[0]), &dense_card_title_text[0][0], sizeof(dense_card_title_text[0]), GRID_DENSE_PREVIEW_CARDS,
                           &dense_preview_snapshot);
    if (ui_ready)
    {
        layout_page();
    }
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
    uint8_t index;

    ui_ready = 0;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), GRID_ROOT_WIDTH, GRID_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, GRID_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    init_panel(&primary_panel, GRID_PRIMARY_PANEL_WIDTH, GRID_PRIMARY_PANEL_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_panel), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_panel));

    init_text_label(&primary_heading_label, 176, 12, primary_snapshots[GRID_DEFAULT_SNAPSHOT].heading, (const egui_font_t *)&egui_res_font_montserrat_8_4,
                    EGUI_COLOR_HEX(0x5E6D7C), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_heading_label), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_heading_label));

    egui_view_gridlayout_init(EGUI_VIEW_OF(&primary_grid), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&primary_grid), GRID_PRIMARY_GRID_WIDTH, GRID_PRIMARY_GRID_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_grid), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_grid));

    for (index = 0; index < GRID_CARD_CAPACITY; index++)
    {
        init_card(&primary_cards[index], &primary_card_eyebrows[index], &primary_card_titles[index], 72, 24);
        egui_view_group_add_child(EGUI_VIEW_OF(&primary_grid), EGUI_VIEW_OF(&primary_cards[index]));
    }

    init_text_label(&primary_note_label, 176, 10, primary_snapshots[GRID_DEFAULT_SNAPSHOT].note, (const egui_font_t *)&egui_res_font_montserrat_8_4,
                    EGUI_COLOR_HEX(0x6B7A89), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_note_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), GRID_BOTTOM_ROW_WIDTH, GRID_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    init_panel(&stack_panel, GRID_PREVIEW_PANEL_WIDTH, GRID_PREVIEW_PANEL_HEIGHT, EGUI_BG_OF(&bg_muted_panel), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&stack_panel));

    init_text_label(&stack_heading_label, 84, 12, "Stack", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&stack_heading_label), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&stack_panel), EGUI_VIEW_OF(&stack_heading_label));

    egui_view_gridlayout_init(EGUI_VIEW_OF(&stack_preview_grid), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&stack_preview_grid), GRID_PREVIEW_GRID_WIDTH, GRID_PREVIEW_GRID_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&stack_preview_grid), 0, 0, 0, 6);
    hcw_grid_override_static_preview_api(EGUI_VIEW_OF(&stack_preview_grid), &stack_preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&stack_preview_grid), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&stack_panel), EGUI_VIEW_OF(&stack_preview_grid));

    for (index = 0; index < GRID_STACK_PREVIEW_CARDS; index++)
    {
        init_card(&stack_cards[index], &stack_card_eyebrows[index], &stack_card_titles[index], 70, 14);
        egui_view_group_add_child(EGUI_VIEW_OF(&stack_preview_grid), EGUI_VIEW_OF(&stack_cards[index]));
    }

    init_text_label(&stack_note_label, 84, 10, "Static preview.", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89),
                    EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&stack_panel), EGUI_VIEW_OF(&stack_note_label));

    init_panel(&dense_panel, GRID_PREVIEW_PANEL_WIDTH, GRID_PREVIEW_PANEL_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&dense_panel), 8, 0, 0, 0);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&dense_panel));

    init_text_label(&dense_heading_label, 84, 12, "Dense", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&dense_heading_label), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&dense_panel), EGUI_VIEW_OF(&dense_heading_label));

    egui_view_gridlayout_init(EGUI_VIEW_OF(&dense_preview_grid), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&dense_preview_grid), GRID_PREVIEW_GRID_WIDTH, GRID_PREVIEW_GRID_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&dense_preview_grid), 0, 0, 0, 6);
    hcw_grid_override_static_preview_api(EGUI_VIEW_OF(&dense_preview_grid), &dense_preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&dense_preview_grid), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&dense_panel), EGUI_VIEW_OF(&dense_preview_grid));

    for (index = 0; index < GRID_DENSE_PREVIEW_CARDS; index++)
    {
        init_card(&dense_cards[index], &dense_card_eyebrows[index], &dense_card_titles[index], 20, 14);
        egui_view_group_add_child(EGUI_VIEW_OF(&dense_preview_grid), EGUI_VIEW_OF(&dense_cards[index]));
    }

    init_text_label(&dense_note_label, 84, 10, "Three columns.", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89),
                    EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&dense_panel), EGUI_VIEW_OF(&dense_note_label));

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
            apply_preview_states();
            apply_primary_default_state();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_state(1);
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_RECORD_FINAL_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_state(2);
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_RECORD_FINAL_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
