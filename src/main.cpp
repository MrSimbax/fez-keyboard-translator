#include <string>
#define RAYLIB_IMPLEMENTATION
#define RAYGUI_IMPLEMENTATION

#define RAYGUI_CUSTOM_ICONS
#include "iconset.rgi.h"

#include <raygui.h>
#include <raylib.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <format>
#include <iostream>
#include <span>
#include <string_view>
#include <vector>

namespace
{
/*
I have decided to overcomplicate things and calculate all the UI parameters at compile-time.
Why? Well... I was so preoccupied with whether or not I could, that I didn't stop to think whether I should.
*/

template <typename Key, typename Value, std::size_t Size>
struct ConstexprMap
{
    consteval auto operator()(const Key& key) const -> Value
    {
        const auto iter =
            std::find_if(std::begin(data), std::end(data), [&key](const auto& value) { return value.first == key; });
        if (iter != std::end(data))
        {
            return iter->second;
        }
        throw std::range_error("Not Found");
    }

    std::array<std::pair<Key, Value>, Size> data;
};

// No clue how to format a string at compile-time, so map it is
constexpr ConstexprMap<int, std::string_view, 49> getIconTextFromId{{{
    // Letters
    {224, "#224#"},
    {225, "#225#"},
    {226, "#226#"},
    {227, "#227#"},
    {228, "#228#"},
    {229, "#229#"},
    {230, "#230#"},
    {231, "#231#"},
    {232, "#232#"},
    {233, "#233#"},
    {234, "#234#"},
    {235, "#235#"},
    {236, "#236#"},
    {237, "#237#"},
    {238, "#238#"},
    {239, "#239#"},
    {240, "#240#"},
    {241, "#241#"},
    {242, "#242#"},
    {243, "#243#"},
    {244, "#244#"},
    {245, "#245#"},
    {246, "#246#"},
    {247, "#247#"},
    {248, "#248#"},
    {249, "#249#"},
    // Numbers
    {80, "#80#"},
    {81, "#81#"},
    {82, "#82#"},
    {83, "#83#"},
    {84, "#84#"},
    {85, "#85#"},
    {86, "#86#"},
    {87, "#87#"},
    {88, "#88#"},
    {89, "#89#"},
    {90, "#90#"},
    {91, "#91#"},
    {92, "#92#"},
    {93, "#93#"},
    {94, "#94#"},
    {95, "#95#"},
    // input
    {96, "#96#"},
    {97, "#97#"},
    {98, "#98#"},
    {99, "#99#"},
    {100, "#100#"},
    {101, "#101#"},
    {102, "#102#"},
}}};

constexpr ConstexprMap<int, std::string_view, 42> getStringFromNumber{{{
    {0, "0"},
    {1, "1"},
    {2, "2"},
    {3, "3"},
    {4, "4"},
    {5, "5"},
    {6, "6"},
    {7, "7"},
    {8, "8"},
    {9, "9"},
    {10, "10"},
}}};

constexpr ConstexprMap<char, std::string_view, 26> getStringFromLetter{{{
    {'A', "A"}, {'B', "B"}, {'C', "C"}, {'D', "D"}, {'E', "E"}, {'F', "F"}, {'G', "G"}, {'H', "H"}, {'I', "I"},
    {'J', "J"}, {'K', "K"}, {'L', "L"}, {'M', "M"}, {'N', "N"}, {'O', "O"}, {'Q', "Q"}, {'P', "P"}, {'R', "R"},
    {'S', "S"}, {'T', "T"}, {'U', "U"}, {'V', "V"}, {'W', "W"}, {'X', "X"}, {'Y', "Y"}, {'Z', "Z"},
}}};

constexpr int LETTER_ICON_ID_START = 224;

constexpr float MARGIN_SIZE = 24.0f;
constexpr float VERTICAL_SPACING = 1.0f;
constexpr float HORIZONTAL_SPACING = VERTICAL_SPACING;

constexpr float LETTER_BUTTON_SIZE = 32.0f;
constexpr float KEYBOARD_INDENT_SIZE = LETTER_BUTTON_SIZE / 2.0f;

constexpr float TEXT_BOX_HEIGHT = 2 * LETTER_BUTTON_SIZE + VERTICAL_SPACING;
constexpr float TEXT_BOX_BUTTON_SIZE = LETTER_BUTTON_SIZE;

constexpr int SCREEN_WIDTH = 541;
constexpr int SCREEN_HEIGHT =
    MARGIN_SIZE + TEXT_BOX_HEIGHT + VERTICAL_SPACING + 4 * LETTER_BUTTON_SIZE + 3 * VERTICAL_SPACING + MARGIN_SIZE;

constexpr Vector2 WINDOW_CONTENT_ANCHOR{.x = MARGIN_SIZE, .y = MARGIN_SIZE};
constexpr Vector2 WINDOW_CONTENT_SIZE{.x = SCREEN_WIDTH - 2.0f * MARGIN_SIZE, .y = SCREEN_HEIGHT - 2.0f * MARGIN_SIZE};

constexpr float TEXT_BOX_WIDTH = WINDOW_CONTENT_SIZE.x - 2.0f * (TEXT_BOX_BUTTON_SIZE + HORIZONTAL_SPACING);

constexpr Vector2 KEYBOARD_ANCHOR{
    .x = WINDOW_CONTENT_ANCHOR.x, .y = WINDOW_CONTENT_ANCHOR.y + TEXT_BOX_HEIGHT + VERTICAL_SPACING};

constexpr std::array<char, 10> KEYBOARD_LETTERS_FIRST_ROW{'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'};
constexpr std::array<char, 9> KEYBOARD_LETTERS_SECOND_ROW{'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L'};
constexpr std::array<char, 7> KEYBOARD_LETTERS_THIRD_ROW{'Z', 'X', 'C', 'V', 'B', 'N', 'M'};
constexpr std::array<std::span<const char, std::dynamic_extent>, 3> KEYBOARD_LETTERS{{
    {KEYBOARD_LETTERS_FIRST_ROW.begin(), KEYBOARD_LETTERS_FIRST_ROW.end()},
    {KEYBOARD_LETTERS_SECOND_ROW.begin(), KEYBOARD_LETTERS_SECOND_ROW.end()},
    {KEYBOARD_LETTERS_THIRD_ROW.begin(), KEYBOARD_LETTERS_THIRD_ROW.end()},
}};
constexpr float LETTER_KEYBOARD_WIDTH =
    (LETTER_BUTTON_SIZE + HORIZONTAL_SPACING) * static_cast<float>(KEYBOARD_LETTERS_FIRST_ROW.size());

enum class SymbolVariant
{
    A,
    B
};

struct Number
{
    int value;
    SymbolVariant variant;
};

constexpr auto operator==(const auto& a, const auto& b) -> bool
{
    return a.value == b.value and a.variant == b.variant;
}

constexpr Vector2 NUMERICAL_KEYBOARD_ANCHOR{
    .x = KEYBOARD_ANCHOR.x + LETTER_KEYBOARD_WIDTH + LETTER_BUTTON_SIZE, .y = KEYBOARD_ANCHOR.y};
constexpr std::array<Number, 4> NUMERICAL_KEYBOARD_FIRST_ROW{
    {
        {1, SymbolVariant::A},
        {2, SymbolVariant::A},
        {3, SymbolVariant::A},
        {4, SymbolVariant::A},
    },
};
constexpr std::array<Number, 4> NUMERICAL_KEYBOARD_SECOND_ROW{
    {
        {3, SymbolVariant::B},
        {5, SymbolVariant::B},
        {7, SymbolVariant::A},
        {5, SymbolVariant::A},
    },
};
constexpr std::array<Number, 4> NUMERICAL_KEYBOARD_THIRD_ROW{
    {
        {7, SymbolVariant::B},
        {6, SymbolVariant::B},
        {9, SymbolVariant::A},
        {8, SymbolVariant::A},
    },
};
constexpr std::array<Number, 4> NUMERICAL_KEYBOARD_FOURTH_ROW{
    {
        {0, SymbolVariant::A},
        {4, SymbolVariant::B},
        {6, SymbolVariant::A},
        {10, SymbolVariant::A},
    },
};
constexpr std::array<std::array<Number, 4>, 4> KEYBOARD_NUMBERS{
    {NUMERICAL_KEYBOARD_FIRST_ROW,
     NUMERICAL_KEYBOARD_SECOND_ROW,
     NUMERICAL_KEYBOARD_THIRD_ROW,
     NUMERICAL_KEYBOARD_FOURTH_ROW}};

constexpr Vector2 INPUT_KEYBOARD_ANCHOR = {
    .x = KEYBOARD_ANCHOR.x + 7 * LETTER_BUTTON_SIZE + 6 * HORIZONTAL_SPACING,
    .y = KEYBOARD_ANCHOR.y + 2 * (LETTER_BUTTON_SIZE + VERTICAL_SPACING)};
constexpr std::array<std::string_view, 4> INPUT_KEYBOARD_FIRST_ROW{{"", "LT", "UP", "RT"}};
constexpr std::array<std::string_view, 4> INPUT_KEYBOARD_SECOND_ROW{{"JUMP", "LEFT", "DOWN", "RIGHT"}};
constexpr std::array<std::array<std::string_view, 4>, 2> KEYBOARD_INPUTS{
    {INPUT_KEYBOARD_FIRST_ROW, INPUT_KEYBOARD_SECOND_ROW}};

constexpr std::string_view COPY_TO_CLIPBOARD_TEXT{"#16#"};
constexpr std::string_view CLEAR_ICON_TEXT{"#143#"};

constexpr auto getIconIdFromLetter(char letter) -> int
{
    return LETTER_ICON_ID_START + letter - 'A';
}

using IconID = int;
constexpr ConstexprMap<Number, IconID, 16> getIconIdFromNumber{
    {
        {
            {Number{1, SymbolVariant::A}, IconID{80}},
            {Number{2, SymbolVariant::A}, IconID{81}},
            {Number{3, SymbolVariant::A}, IconID{82}},
            {Number{4, SymbolVariant::A}, IconID{83}},
            {Number{3, SymbolVariant::B}, IconID{84}},
            {Number{5, SymbolVariant::B}, IconID{85}},
            {Number{7, SymbolVariant::A}, IconID{86}},
            {Number{5, SymbolVariant::A}, IconID{87}},
            {Number{7, SymbolVariant::B}, IconID{88}},
            {Number{6, SymbolVariant::B}, IconID{89}},
            {Number{9, SymbolVariant::A}, IconID{90}},
            {Number{8, SymbolVariant::A}, IconID{91}},
            {Number{0, SymbolVariant::A}, IconID{92}},
            {Number{4, SymbolVariant::B}, IconID{93}},
            {Number{6, SymbolVariant::A}, IconID{94}},
            {Number{10, SymbolVariant::A}, IconID{95}},
        },
    },
};

constexpr ConstexprMap<std::string_view, int, 7> getIconIdFromInput{{{
    {"UP", 96},
    {"RIGHT", 97},
    {"DOWN", 98},
    {"LEFT", 99},
    {"JUMP", 100},
    {"RT", 101},
    {"LT", 102},
}}};

struct UiState
{
    std::string translatedText{};
};

struct KeyboardButton
{
    std::string_view text;
    std::string_view iconText;
    float x;
    float y;
};

consteval auto makeKeyboardLetterButtons() -> std::array<KeyboardButton, 26>
{
    std::array<KeyboardButton, 26> buttons{};

    size_t i = 0;
    float x = KEYBOARD_ANCHOR.x;
    float y = KEYBOARD_ANCHOR.y;
    float indent = 0;

    for (const auto& row : KEYBOARD_LETTERS)
    {
        for (char letter : row)
        {
            buttons.at(i++) = {
                .text = getStringFromLetter(letter),
                .iconText = getIconTextFromId(getIconIdFromLetter(letter)),
                .x = x + indent,
                .y = y,
            };

            x += LETTER_BUTTON_SIZE + HORIZONTAL_SPACING;
        }

        x = KEYBOARD_ANCHOR.x;
        indent += KEYBOARD_INDENT_SIZE;
        y += LETTER_BUTTON_SIZE + VERTICAL_SPACING;
    }

    std::reverse(buttons.begin(), buttons.end());

    return buttons;
}

constexpr auto KEYBOARD_LETTER_BUTTONS = makeKeyboardLetterButtons();

consteval auto makeNumericalKeyboardButtons() -> std::array<KeyboardButton, 16>
{
    std::array<KeyboardButton, 16> buttons{};

    size_t i = 0;
    float x = NUMERICAL_KEYBOARD_ANCHOR.x;
    float y = NUMERICAL_KEYBOARD_ANCHOR.y;

    for (const auto& row : KEYBOARD_NUMBERS)
    {
        for (Number number : row)
        {
            buttons.at(i++) = {
                .text = getStringFromNumber(number.value),
                .iconText = getIconTextFromId(getIconIdFromNumber(number)),
                .x = x,
                .y = y,
            };

            x += LETTER_BUTTON_SIZE + HORIZONTAL_SPACING;
        }

        x = NUMERICAL_KEYBOARD_ANCHOR.x;
        y += LETTER_BUTTON_SIZE + VERTICAL_SPACING;
    }

    std::reverse(buttons.begin(), buttons.end());

    return buttons;
}

constexpr auto NUMERICAL_KEYBOARD_BUTTONS = makeNumericalKeyboardButtons();

consteval auto makeInputKeyboardButtons() -> std::array<KeyboardButton, 7>
{
    std::array<KeyboardButton, 7> buttons{};

    size_t i = 0;
    float x = INPUT_KEYBOARD_ANCHOR.x;
    float y = INPUT_KEYBOARD_ANCHOR.y;

    for (const auto& row : KEYBOARD_INPUTS)
    {
        for (std::string_view input : row)
        {
            if (not input.empty())
            {
                buttons.at(i++) = {
                    .text = input,
                    .iconText = getIconTextFromId(getIconIdFromInput(input)),
                    .x = x,
                    .y = y,
                };
            }

            x += LETTER_BUTTON_SIZE + HORIZONTAL_SPACING;
        }

        x = INPUT_KEYBOARD_ANCHOR.x;
        y += LETTER_BUTTON_SIZE + VERTICAL_SPACING;
    }

    std::reverse(buttons.begin(), buttons.end());

    return buttons;
}

constexpr auto INPUT_KEYBOARD_BUTTONS = makeInputKeyboardButtons();

auto drawSpaceBar(UiState& state)
{
    constexpr float spaceButtonWidth = 4 * LETTER_BUTTON_SIZE + 3 * HORIZONTAL_SPACING;
    if (GuiButton(
            Rectangle{
                .x = KEYBOARD_ANCHOR.x + 3 * LETTER_BUTTON_SIZE + 2 * HORIZONTAL_SPACING,
                .y = KEYBOARD_ANCHOR.y + 3 * (LETTER_BUTTON_SIZE + VERTICAL_SPACING),
                .width = spaceButtonWidth,
                .height = LETTER_BUTTON_SIZE,
            },
            "Space") != 0)
    {
        state.translatedText.push_back(' ');
    }
}

auto drawKeyboardButton(UiState& state, KeyboardButton button)
{
    GuiSetTooltip(button.text.data());

    if (GuiButton(
            Rectangle{.x = button.x, .y = button.y, .width = LETTER_BUTTON_SIZE, .height = LETTER_BUTTON_SIZE},
            button.iconText.data()) != 0)
    {
        state.translatedText += button.text;
    }
}

auto drawLetters(UiState& state)
{
    for (const auto& letterButton : KEYBOARD_LETTER_BUTTONS)
    {
        drawKeyboardButton(state, letterButton);
    }
    GuiSetTooltip(nullptr);
}

auto drawNumbers(UiState& state)
{
    for (const auto& numberButton : NUMERICAL_KEYBOARD_BUTTONS)
    {
        drawKeyboardButton(state, numberButton);
    }
    GuiSetTooltip(nullptr);
}

auto drawInputs(UiState& state)
{
    for (const auto& inputButton : INPUT_KEYBOARD_BUTTONS)
    {
        drawKeyboardButton(state, inputButton);
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
    drawInputs(state);
    drawLetters(state);
    drawNumbers(state);
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
    GuiLabel(labelBounds, (state.translatedText + "|").c_str());
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_MIDDLE);
    GuiSetStyle(DEFAULT, TEXT_WRAP_MODE, TEXT_WRAP_NONE);

    GuiSetTooltip("Copy to clipboard");
    constexpr Rectangle copyToClipboardButtonBounds{
        .x = WINDOW_CONTENT_ANCHOR.x + TEXT_BOX_WIDTH + HORIZONTAL_SPACING,
        .y = WINDOW_CONTENT_ANCHOR.y,
        .width = TEXT_BOX_BUTTON_SIZE,
        .height = TEXT_BOX_BUTTON_SIZE};
    if (GuiButton(copyToClipboardButtonBounds, COPY_TO_CLIPBOARD_TEXT.data()) != 0)
    {
        SetClipboardText(state.translatedText.c_str());
    }

    GuiSetTooltip("Clear text");
    constexpr Rectangle clearTextButtonBounds{
        .x = WINDOW_CONTENT_ANCHOR.x + TEXT_BOX_WIDTH + HORIZONTAL_SPACING + TEXT_BOX_BUTTON_SIZE + HORIZONTAL_SPACING,
        .y = WINDOW_CONTENT_ANCHOR.y,
        .width = TEXT_BOX_BUTTON_SIZE,
        .height = TEXT_BOX_BUTTON_SIZE};
    if (GuiButton(clearTextButtonBounds, CLEAR_ICON_TEXT.data()) != 0)
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

auto getRenderTexture() -> RenderTexture2D
{
    RenderTexture2D target = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);
    return target;
}

auto getDpiScale() -> Vector2
{
#if defined(PLATFORM_WEB)
    return {.x = 1.0f, .y = 1.0f};
#else
    return GetWindowScaleDPI();
#endif
}

auto updateAndDrawFrame() -> void
{
    static UiState uiState{};
    static RenderTexture2D target = getRenderTexture();

    auto dpiScale = getDpiScale();
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
} // namespace

auto main() -> int
{
    GuiEnableTooltip();
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "FEZ Keyboard Translator");

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(updateAndDrawFrame, 0, 1);
#else
    SetTargetFPS(60);

    while (not WindowShouldClose())
    {
        updateAndDrawFrame();
    }
#endif

    CloseWindow();

    return 0;
}
