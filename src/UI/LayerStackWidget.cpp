#include "LayerStackWidget.hpp"

#include "Resources.hpp"
#include "Buttons.hpp"
#include "Icons.hpp"

#include "Base/Inputs.h"
#include "Base/Logging.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>





LayerStackWidget::LayerStackWidget()
{
}

LayerStackWidget::LayerStackWidget(const LayerStackPtr& layerStack) :
        m_layerStack(layerStack)
{
}


// == Drawing ==

bool LayerStackWidget::DrawLayer(const LayerPtr& layer, const int& index)
{
    bool somethingChanged = false;

    std::string layerName = layer->GetName();
    uint32_t uuid = layer->GetUUID();
    std::string identifier = std::to_string(uuid);
    ImGuiTreeNodeFlags nodeFlags = (ImGuiTreeNodeFlags_FramePadding |
                                    ImGuiTreeNodeFlags_Framed |
                                    ImGuiTreeNodeFlags_AllowItemOverlap |
                                    ImGuiTreeNodeFlags_SpanAvailWidth |
                                    ImGuiTreeNodeFlags_NoTreePushOnOpen |
                                    ImGuiTreeNodeFlags_Leaf);

    int popColors = 0;
    bool isSource = false;
    bool isSelected = IsSelected(layer);
    if (isSelected) {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.30f, 0.30f, 0.30f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.40f, 0.40f, 0.40f, 1.00f));
        popColors += 2;
    }
    else if (isSource = IsSource(layer))
    {
        ImGui::PushStyleColor(ImGuiCol_Header,        ImVec4(0.224f, 0.488f, 0.591f, 0.300));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.225f, 0.683f, 0.862f, 0.300));
        popColors += 2;
    }

    // Create the tree node
    ImGui::Separator();
    std::string treeNodeLabel = std::string(ICON_LAYER_GROUP) + "###LayerStackNode" + identifier;
    ImGui::PushFont(IconicFont());
    ImGui::TreeNodeEx(treeNodeLabel.c_str(), nodeFlags);
    ImGui::PopFont();
    ImGui::PopStyleColor(popColors);

    bool focused = ImGui::IsItemFocused();
    bool clicked = ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen();

    static char renamedName[256];
    static bool renameIgnoreActive = false;
    ImGui::SameLine(32.0f);
    if (m_renamedUUID == uuid)
    {
        ImGui::SetKeyboardFocusHere();
        ImGui::InputText("##RenameLayer", renamedName, 256, ImGuiInputTextFlags_AutoSelectAll);
        // The active status has a 1 frame delay so we only care about it on the next frame
        if (!renameIgnoreActive && !ImGui::IsItemActive())
        {
            layer->SetName(std::string(renamedName));
            strcpy(renamedName, "");
            m_renamedUUID = 0;
        }
        renameIgnoreActive = false;
    } 
    else
    {
        ImGui::Text(layerName.c_str());
    } 

    bool toggled = false;

    // Dual checkbox
    bool dual = layer->IsDual();
    ImGui::SameLine(ImGui::GetWindowWidth() - 69.0f - (float)dual);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 1.0f);
    if (ImGui::CheckedTextButton((std::string("##DualToggle") + layerName).c_str(), "D*", "D", dual))
    {
        layer->SetDual(!dual);
        somethingChanged = true;
        toggled = true;
    }

    // Animated checkbox
    auto provider = std::dynamic_pointer_cast<Explicit>(layer->GetProvider());
    bool animated = provider && provider->IsAnimated();
    ImGui::SameLine(ImGui::GetWindowWidth() - 46.0f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
    ImGui::BeginDisabled(!(bool)provider);
    if (ImGui::CheckedIconButton((std::string("##AnimatedToggle") + layerName).c_str(), 
                                 ICON_PERSON_RUNNING, 
                                 ICON_PERSON, 
                                 animated, 
                                 IconSize::Medium,
                                 ImVec4(0.8f, 0.8f, 0.8f, 1.0f),
                                 ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
                                 provider ? ImVec4(0.09f, 0.09f, 0.09f, 1.0f) : ImVec4(0.5f, 0.05f, 0.05f, 1.0f)))
    {
        provider->SetAnimated(!animated);
        somethingChanged = true;
        toggled = true;
    }
    ImGui::EndDisabled();

    // Visibility checkbox
    bool visible = layer->IsVisible();
    ImGui::SameLine(ImGui::GetWindowWidth() - 26.0f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
    if (ImGui::CheckedIconButton((std::string("##VisibleToggle") + layerName).c_str(), 
                                 ICON_EYE, 
                                 ICON_EYE_SLASH, 
                                 visible, 
                                 IconSize::Small))
    {
        layer->SetVisible(!visible);
        somethingChanged = true;
        toggled = true;
    }

    // Handle mouse clicks
    if (!toggled && clicked) 
    {
        // Double click -> rename the current layer
        if (clicked && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            strcpy(renamedName, layerName.c_str());
            m_renamedUUID = uuid;
            ImGui::SetFocusID(ImGui::GetID("##RenameLayer"), ImGui::GetCurrentWindow());
            renameIgnoreActive = true;
        }

        // Shift modifier + click -> Range selection
        else if (Inputs::IsKeyPressed(KeyCode::LeftShift)) 
        {
            if (m_lastIndex >= 0)
            {
                int minIndex = index;
                if (minIndex > m_lastIndex)
                    std::swap(minIndex, m_lastIndex);
                
                const auto& layers = m_layerStack->GetLayers();
                for (int i=minIndex ; i < m_lastIndex + 1 ; ++i)
                {
                    SelectLayer(layers[i]);
                }
            } else {
                SelectLayer(layer);
            }

            m_lastIndex = index;
        }

        // Ctrl modifier + click -> Toggle selection
        else if (Inputs::IsKeyPressed(KeyCode::LeftCtrl))
        {
            if (isSelected){
                DeselectLayer(layer);
            } else {
                SelectLayer(layer);
                m_lastIndex = index;
            }
        }

        // Simple click -> Replace selection
        else {
            ClearSelection();
            SelectLayer(layer);
            m_lastIndex = index;
        }
    }

    if (focused && Inputs::IsKeyPressed(KeyCode::Delete)) 
    {
        DeleteSelectedLayers();
        somethingChanged = true;
    }

    return somethingChanged;
}

bool LayerStackWidget::Draw() 
{
    ImGui::Begin("Layer Stack", nullptr, ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {

        if (ImGui::BeginMenu("Create"))
        {
            if (ImGui::MenuItem("Layer"))
                m_layerStack->NewLayer("Layer", {});

            if (ImGui::MenuItem("Random generator"))
                m_layerStack->NewRandomGenerator("Layer");

            if (ImGui::MenuItem("Subset", "", nullptr, m_selection.size() >= 1))
                m_layerStack->NewSubset("Layer", m_selection.back());

            if (ImGui::MenuItem("SelfCombination", "", nullptr, m_selection.size() >= 1))
                m_layerStack->NewSelfCombination("Layer", m_selection.back());

            if (ImGui::MenuItem("Combination", "", nullptr, m_selection.size() >= 2))
                m_layerStack->NewCombination("Layer", m_selection[m_selection.size() - 2], m_selection.back());

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::PushStyleColor(ImGuiCol_Header,        ImVec4(0.17f, 0.17f, 0.17f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.27f, 0.27f, 0.27f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,  ImVec4(0.09f, 0.09f, 0.09f, 1.00f));

    bool somethingChanged = false;

    if (m_layerStack)
    {
        const auto& layers = m_layerStack->GetLayers();
        for (int i=0 ; i < layers.size() ; ++i)
        {
            somethingChanged |= DrawLayer(layers[i], i);
        }
    }
    else 
    {
        ImGui::Text("No layer stack attached to the widget.");
    }

    m_hovered = ImGui::IsWindowHovered();

    ImGui::PopStyleColor(3);
    ImGui::End();

    return somethingChanged;
}


// == Layer management ==

void LayerStackWidget::SetLayerStack(const LayerStackPtr& layerStack)
{
    if (layerStack != m_layerStack)
    {
        m_layerStack = layerStack;
        ClearSelection();
    }
}

LayerPtrArray& LayerStackWidget::GetSelection() 
{
    return m_selection;
}

bool LayerStackWidget::IsSelected(const LayerPtr &layer) const 
{
    return std::find(m_selection.begin(), m_selection.end(), layer) != m_selection.end();
}

void LayerStackWidget::SelectLayer(const LayerPtr &layer) 
{
    if (!IsSelected(layer))
    {
        m_selection.push_back(layer);
        UpdateSources();
    }
}

void LayerStackWidget::DeselectLayer(const LayerPtr &layer) 
{
    auto it = std::find(m_selection.begin(), m_selection.end(), layer);
    if (it != m_selection.end())
    {
        m_selection.erase(it);
        UpdateSources();
    }

    m_lastIndex = -1; 
}

void LayerStackWidget::ClearSelection() 
{
    m_selection.clear();
    m_sources.clear();
    m_lastIndex = -1; 
}

LayerPtr LayerStackWidget::GetLastSelectedLayer() const
{
     if (!m_selection.empty() && m_lastIndex >= 0) 
        return m_layerStack->GetLayer(m_lastIndex);

    return {};
}

void LayerStackWidget::DeleteSelectedLayers()
{
    // Make sure we can delete the layers without causing too much trouble
    for (const auto& layer : m_selection)
    {
        for (const auto& dest : layer->GetDestinations())
        {
            auto destination = dest.lock();
            if (destination && !IsSelected(destination))
            {
                LOG_ERROR("Cannot remove the selected layers as other layers depend on them");
                return;    
            }            
        }
    }

    // Delete the layers
    for (const auto& layer : m_selection)
    {
        m_layerStack->RemoveLayer(layer);
    }

    ClearSelection();
}

void LayerStackWidget::Clear()
{
    m_layerStack->Clear();
    ClearSelection();
}


// == Sources ==

bool LayerStackWidget::IsSource(const LayerPtr &layer) 
{
    return std::find(m_sources.begin(), m_sources.end(), layer) != m_sources.end();
}

void LayerStackWidget::UpdateSources() 
{
    m_sources.clear();
    for (const auto& layer : m_selection)
    {
        auto provider = layer->GetProvider();
        if (!provider)
            continue;

        const auto& sources = layer->GetSources();
        uint32_t sourceCount = std::min(provider->GetSourceCount(), 
                                        (uint32_t)sources.size());

        for (uint32_t i=0 ; i < sourceCount ; ++i)
            m_sources.push_back(sources[i].lock());
    }
}
