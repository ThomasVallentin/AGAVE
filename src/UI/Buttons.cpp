#include "Buttons.hpp"


namespace ImGui {


bool TextButton(const char* label,
                const char* iconCode, 
                const ImVec4& color, 
                const ImVec4& hoveredColor, 
                const ImVec4& pressedColor, 
                ImGuiButtonFlags flags)
{

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(iconCode, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(ImVec2(0, 0), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_Repeat;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render
    const ImU32 col = GetColorU32(0, 0.0f);  // Render transparent frame
    RenderNavHighlight(bb, id);
    RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

    if (g.LogEnabled)
        LogSetNextTextDecoration("[", "]");

    PushStyleColor(ImGuiCol_Text, (held && hovered) ? pressedColor : hovered ? hoveredColor : color);
    RenderTextClipped(bb.Min + style.FramePadding + ImVec2(-1, -1), bb.Max - style.FramePadding, iconCode, NULL, &label_size, style.ButtonTextAlign, &bb);
    PopStyleColor();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, iconCode, g.LastItemData.StatusFlags);
    return pressed;
}   

bool IconButton(const char* label,
                const char* iconCode, 
                const IconSize& iconSize,
                const ImVec4& color, 
                const ImVec4& hoveredColor, 
                const ImVec4& pressedColor, 
                ImGuiButtonFlags flags) 
{
    PushFont(IconicFont(iconSize));
    PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));

    bool pressed = TextButton(label, iconCode, color, hoveredColor, pressedColor, flags);

    PopStyleVar();
    PopFont();

    return pressed;
}

bool CheckedTextButton(const char* label, const char* onText, const char* offText, const bool& onOff, 
                       const ImVec4& onColor, const ImVec4& hoveredColor, const ImVec4& offColor, 
                       ImGuiButtonFlags flags)
{
    if (onOff)
        return TextButton(label, onText, onColor, hoveredColor, offColor, flags);
    
    return TextButton(label, offText, offColor, offColor, offColor, flags);
}

bool CheckedIconButton(const char* label, const char* onIconCode, const char* offIconCode, const bool& onOff, 
                       const IconSize& iconSize, const ImVec4& onColor, const ImVec4& hoveredColor, const ImVec4& offColor, 
                       ImGuiButtonFlags flags)
{
    if (onOff)
        return IconButton(label, onIconCode, iconSize, onColor, hoveredColor, offColor, flags);
    
    return IconButton(label, offIconCode, iconSize, offColor, offColor, offColor, flags);
}        
                    



}  // namespace ImGui
