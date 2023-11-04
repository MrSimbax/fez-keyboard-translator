#include <algorithm>
#define RAYLIB_IMPLEMENTATION
#define RAYGUI_IMPLEMENTATION

#include <raygui.h>
#include <raylib.h>

#include <cmath>
#include <cstring>
#include <format>
#include <iostream>
#include <vector>

namespace
{
constexpr int LETTER_ICON_ID_START = 224;
constexpr int TRANSLATED_TEXT_MAX_SIZE = 128;

constexpr bool DO_NOT_LOAD_ICON_NAMES = false;
constexpr bool TEXTBOX_IS_NOT_EDITABLE = false;

constexpr float MARGIN_SIZE = 2.0f;
constexpr float VERTICAL_SPACING = 1.0f;
constexpr float HORIZONTAL_SPACING = VERTICAL_SPACING;

constexpr float LETTER_BUTTON_SIZE = 32.0f;
constexpr float KEYBOARD_INDENT_SIZE = LETTER_BUTTON_SIZE / 2.0f;

constexpr float TEXT_BOX_HEIGHT = 2 * LETTER_BUTTON_SIZE + VERTICAL_SPACING;
constexpr float TEXT_BOX_BUTTON_SIZE = LETTER_BUTTON_SIZE;

constexpr int SCREEN_WIDTH = 333;
constexpr int SCREEN_HEIGHT =
    MARGIN_SIZE + TEXT_BOX_HEIGHT + VERTICAL_SPACING + 4 * LETTER_BUTTON_SIZE + 3 * VERTICAL_SPACING + MARGIN_SIZE;

constexpr Vector2 WINDOW_CONTENT_ANCHOR{.x = MARGIN_SIZE, .y = MARGIN_SIZE};
constexpr Vector2 WINDOW_CONTENT_SIZE{.x = SCREEN_WIDTH - 2.0f * MARGIN_SIZE, .y = SCREEN_HEIGHT - 2.0f * MARGIN_SIZE};

constexpr float TEXT_BOX_WIDTH = WINDOW_CONTENT_SIZE.x - 2.0f * (TEXT_BOX_BUTTON_SIZE + HORIZONTAL_SPACING);

constexpr Vector2 KEYBOARD_ANCHOR = {
    .x = WINDOW_CONTENT_ANCHOR.x, .y = WINDOW_CONTENT_ANCHOR.y + TEXT_BOX_HEIGHT + VERTICAL_SPACING};

const char* const COPY_TO_CLIPBOARD_TEXT = "#16#";
const char* const CLEAR_ICON_TEXT = "#143#";

constexpr auto getIconIdFromLetter(char letter) -> int
{
    return LETTER_ICON_ID_START + letter - 'a';
}

auto getIconText(int iconId) -> std::string
{
    return std::format("#{}#", iconId);
}

struct UiState
{
    std::string translatedText{};
};

struct KeyboardButton
{
    char letter;
    float x;
    float y;
};

auto makeKeyboardLetterButtons()
{
    static const std::vector<std::vector<char>> letterButtons{
        {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'},
        {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l'},
        {'z', 'x', 'c', 'v', 'b', 'n', 'm'}};
    static const float maxRowSize =
        (LETTER_BUTTON_SIZE + HORIZONTAL_SPACING) * static_cast<float>(letterButtons[0].size());

    std::vector<KeyboardButton> buttons;

    float x = KEYBOARD_ANCHOR.x;
    float y = KEYBOARD_ANCHOR.y;
    float indent = 0;

    for (const auto& row : letterButtons)
    {
        for (char letter : row)
        {
            buttons.push_back({.letter = letter, .x = x + indent, .y = y});

            x += LETTER_BUTTON_SIZE + HORIZONTAL_SPACING;
        }

        x = KEYBOARD_ANCHOR.x;
        indent += KEYBOARD_INDENT_SIZE;
        y += LETTER_BUTTON_SIZE + VERTICAL_SPACING;
    }

    return buttons;
}

auto drawSpaceBar(UiState& state)
{
    constexpr float spaceButtonWidth = 4 * LETTER_BUTTON_SIZE + 3 * HORIZONTAL_SPACING;
    if (GuiButton(
            Rectangle{
                .x = KEYBOARD_ANCHOR.x + 3 * LETTER_BUTTON_SIZE + 2 * HORIZONTAL_SPACING,
                .y = KEYBOARD_ANCHOR.y + 3 * (LETTER_BUTTON_SIZE + VERTICAL_SPACING),
                .width = spaceButtonWidth,
                .height = LETTER_BUTTON_SIZE},
            "Space") != 0)
    {
        state.translatedText.push_back(' ');
    }
}

auto drawLetters(UiState& state)
{
    std::vector<KeyboardButton> keyboardButtons{makeKeyboardLetterButtons()};
    std::reverse(keyboardButtons.begin(), keyboardButtons.end());
    for (const auto& keyboardButton : keyboardButtons)
    {
        auto letterIconId = getIconIdFromLetter(keyboardButton.letter);
        auto uppercaseLetter = static_cast<char>(std::toupper(keyboardButton.letter));

        GuiSetTooltip(std::string{uppercaseLetter}.c_str());

        if (GuiButton(
                Rectangle{
                    .x = keyboardButton.x,
                    .y = keyboardButton.y,
                    .width = LETTER_BUTTON_SIZE,
                    .height = LETTER_BUTTON_SIZE},
                getIconText(letterIconId).c_str()) != 0)
        {
            state.translatedText.push_back(uppercaseLetter);
        }
    }
    GuiSetTooltip(nullptr);
}

auto drawBackspace(UiState& state)
{
    constexpr float backspaceButtonWidth = 2 * LETTER_BUTTON_SIZE + HORIZONTAL_SPACING;
    if (GuiButton(
            Rectangle{
                .x = WINDOW_CONTENT_ANCHOR.x + (WINDOW_CONTENT_SIZE.x - backspaceButtonWidth),
                .y = WINDOW_CONTENT_ANCHOR.y + LETTER_BUTTON_SIZE + VERTICAL_SPACING,
                .width = backspaceButtonWidth,
                .height = LETTER_BUTTON_SIZE},
            "Backspace") != 0)
    {
        if (not state.translatedText.empty())
        {
            state.translatedText.pop_back();
        }
    }
}

auto drawKeyboard(UiState& state) -> void
{
    drawSpaceBar(state);
    drawLetters(state);
    drawBackspace(state);
}

auto drawTextBox(UiState& state)
{
    constexpr Rectangle textBoxBounds{
        .x = WINDOW_CONTENT_ANCHOR.x, .y = WINDOW_CONTENT_ANCHOR.y, .width = TEXT_BOX_WIDTH, .height = TEXT_BOX_HEIGHT};
    GuiPanel(textBoxBounds, nullptr);

    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_TOP);
    GuiSetStyle(DEFAULT, TEXT_WRAP_MODE, TEXT_WRAP_CHAR);
    constexpr Rectangle labelBounds{
        .x = textBoxBounds.x + 1,
        .y = textBoxBounds.y + 1,
        .width = textBoxBounds.width - 2,
        .height = textBoxBounds.height - 2,
    };
    GuiLabel(labelBounds, state.translatedText.c_str());
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_MIDDLE);
    GuiSetStyle(DEFAULT, TEXT_WRAP_MODE, TEXT_WRAP_NONE);

    GuiSetTooltip("Copy to clipboard");
    constexpr Rectangle copyToClipboardButtonBounds{
        .x = WINDOW_CONTENT_ANCHOR.x + TEXT_BOX_WIDTH + HORIZONTAL_SPACING,
        .y = WINDOW_CONTENT_ANCHOR.y,
        .width = TEXT_BOX_BUTTON_SIZE,
        .height = TEXT_BOX_BUTTON_SIZE};
    if (GuiButton(copyToClipboardButtonBounds, COPY_TO_CLIPBOARD_TEXT) != 0)
    {
        SetClipboardText(state.translatedText.c_str());
    }

    GuiSetTooltip("Clear text");
    constexpr Rectangle clearTextButtonBounds{
        .x = WINDOW_CONTENT_ANCHOR.x + TEXT_BOX_WIDTH + HORIZONTAL_SPACING + TEXT_BOX_BUTTON_SIZE + HORIZONTAL_SPACING,
        .y = WINDOW_CONTENT_ANCHOR.y,
        .width = TEXT_BOX_BUTTON_SIZE,
        .height = TEXT_BOX_BUTTON_SIZE};
    if (GuiButton(clearTextButtonBounds, CLEAR_ICON_TEXT) != 0)
    {
        state.translatedText.clear();
    }
    GuiSetTooltip(nullptr);
}

auto drawUi(UiState& state) -> void
{
    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    drawKeyboard(state);
    drawTextBox(state);
}
} // namespace

auto main() -> int
{
    GuiLoadIcons("assets/iconset.rgi", DO_NOT_LOAD_ICON_NAMES);
    GuiEnableTooltip();

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "FEZ Keyboard Translator");

    SetTargetFPS(60);

    UiState uiState{};

    RenderTexture2D target = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

    while (not WindowShouldClose())
    {
        auto dpiScale = GetWindowScaleDPI();
        SetMouseScale(1.0f / dpiScale.x, 1.0f / dpiScale.y);
        SetWindowSize(std::floor(SCREEN_WIDTH * dpiScale.x), std::floor(SCREEN_HEIGHT * dpiScale.y));

        BeginTextureMode(target);
        drawUi(uiState);
        EndTextureMode();

        BeginDrawing();
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

        constexpr Rectangle source{.x = 0.0f, .y = 0.0f, .width = SCREEN_WIDTH, .height = -SCREEN_HEIGHT};
        Rectangle destination{
            .x = 0.0f, .y = 0.0f, .width = SCREEN_WIDTH * dpiScale.x, .height = SCREEN_HEIGHT * dpiScale.y};
        constexpr Vector2 origin{.x = 0.0f, .y = 0.0f};
        constexpr float rotation{0.0f};
        constexpr Color tint{WHITE};
        DrawTexturePro(target.texture, source, destination, origin, rotation, tint);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
