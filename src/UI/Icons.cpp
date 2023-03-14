#include "Icons.hpp"

#include "Base/Resolver.h"

#include <imgui.h>

#include <iostream>



static ImFont* iconicFontTiny   = nullptr;
static ImFont* iconicFontSmall  = nullptr;
static ImFont* iconicFontMedium = nullptr;
static ImFont* iconicFontLarge  = nullptr;
static ImFont* iconicFontHuge   = nullptr;


ImFont* IconicFont(const IconSize& size)
{
    switch (size)
    {
        case IconSize::Tiny:   return iconicFontTiny;
        case IconSize::Small:  return iconicFontSmall;
        case IconSize::Medium: return iconicFontMedium;
        case IconSize::Large:  return iconicFontLarge;
        case IconSize::Huge:   return iconicFontHuge;
    }

    return iconicFontMedium;
}

void InitIconicFont()
{
    if (!iconicFontTiny)
    {
        const char* fontPath = Resolver::Get().Resolve(FONT_ICON_FILE_NAME_SOLID).c_str();
        ImGuiIO& io = ImGui::GetIO();

        // merge in icons from Font Awesome
        static const ImWchar icons_ranges[] = { ICON_MIN, ICON_MAX_16, 0 };
        ImFontConfig icons_config; 
        // icons_config.MergeMode = true; 
        icons_config.PixelSnapH = true; 

        float iconFontSize = 14.0f * 2.0f / 3.0f;
        icons_config.GlyphMinAdvanceX = iconFontSize;  
        iconicFontTiny = io.Fonts->AddFontFromFileTTF( "/media/tvallentin/BA0EC01B0EBFCF1F/M2/GA/Projet/resources/fonts/fa-solid-900.ttf", iconFontSize, &icons_config, icons_ranges );

        iconFontSize = 18.0f * 2.0f / 3.0f;
        icons_config.GlyphMinAdvanceX = iconFontSize;  
        iconicFontSmall = io.Fonts->AddFontFromFileTTF( "/media/tvallentin/BA0EC01B0EBFCF1F/M2/GA/Projet/resources/fonts/fa-solid-900.ttf", iconFontSize, &icons_config, icons_ranges );

        iconFontSize = 22.0f * 2.0f / 3.0f;
        icons_config.GlyphMinAdvanceX = iconFontSize;  
        iconicFontMedium = io.Fonts->AddFontFromFileTTF( "/media/tvallentin/BA0EC01B0EBFCF1F/M2/GA/Projet/resources/fonts/fa-solid-900.ttf", iconFontSize, &icons_config, icons_ranges );

        iconFontSize = 26.0f * 2.0f / 3.0f;
        icons_config.GlyphMinAdvanceX = iconFontSize;  
        iconicFontLarge = io.Fonts->AddFontFromFileTTF( "/media/tvallentin/BA0EC01B0EBFCF1F/M2/GA/Projet/resources/fonts/fa-solid-900.ttf", iconFontSize, &icons_config, icons_ranges );

        iconFontSize = 30.0f * 2.0f / 3.0f;
        icons_config.GlyphMinAdvanceX = iconFontSize;  
        iconicFontHuge = io.Fonts->AddFontFromFileTTF( "/media/tvallentin/BA0EC01B0EBFCF1F/M2/GA/Projet/resources/fonts/fa-solid-900.ttf", iconFontSize, &icons_config, icons_ranges );
    }
}
