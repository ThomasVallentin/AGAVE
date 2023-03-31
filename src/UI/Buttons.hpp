
#ifndef BUTTONS_H
#define BUTTONS_H

#include "Icons.hpp"

#include <imgui_internal.h>


namespace ImGui {



bool TextButton(const char* label,
                const char* iconCode, 
                const ImVec4& color=ImVec4(0.8f, 0.8f, 0.8f, 1), 
                const ImVec4& hoveredColor=ImVec4(1, 1, 1, 1), 
                const ImVec4& pressedColor=ImVec4(0.09f, 0.09f, 0.09f, 1), 
                ImGuiButtonFlags flags=ImGuiButtonFlags_PressedOnClick);

bool IconButton(const char* label,
                const char* iconCode, 
                const IconSize& iconSize=IconSize::Medium,
                const ImVec4& color=ImVec4(0.8f, 0.8f, 0.8f, 1), 
                const ImVec4& hoveredColor=ImVec4(1, 1, 1, 1), 
                const ImVec4& pressedColor=ImVec4(0.09f, 0.09f, 0.09f, 1), 
                ImGuiButtonFlags flags=ImGuiButtonFlags_PressedOnClick);

bool CheckedTextButton(const char* label,
                       const char* onText, 
                       const char* offText, 
                       const bool& onOff, 
                       const ImVec4& color=ImVec4(0.8f, 0.8f, 0.8f, 1), 
                       const ImVec4& hoveredColor=ImVec4(1, 1, 1, 1), 
                       const ImVec4& offColor=ImVec4(0.09f, 0.09f, 0.09f, 1), 
                       ImGuiButtonFlags flags=ImGuiButtonFlags_PressedOnClick);

bool CheckedIconButton(const char* label,
                       const char* onIconCode, 
                       const char* offIconCode, 
                       const bool& onOff, 
                       const IconSize& iconSize=IconSize::Medium,
                       const ImVec4& onColor=ImVec4(0.8f, 0.8f, 0.8f, 1), 
                       const ImVec4& hoveredColor=ImVec4(1, 1, 1, 1), 
                       const ImVec4& offColor=ImVec4(0.09f, 0.09f, 0.09f, 1), 
                       ImGuiButtonFlags flags=ImGuiButtonFlags_PressedOnClick);

}  // namespace ImGui


#endif // BUTTONS_H