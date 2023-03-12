#include "LayerStackWidget.hpp"

#include "Base/Inputs.h"
#include "Base/Logging.h"

#include <imgui.h>
#include <imgui_internal.h>


void LayerStackWidget::SetLayerStack(const LayerStackPtr& layerStack)
{
    if (layerStack != m_layerStack)
    {
        m_layerStack = layerStack;
        ClearSelection();
    }
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

bool LayerStackWidget::Draw() 
{
    ImGui::Begin("Layer Stack", nullptr, ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {

        if (ImGui::BeginMenu("Create"))
        {
            if (ImGui::MenuItem("Layer"))
            {
                m_layerStack->NewLayer("Layer", {});
            }

            if (ImGui::MenuItem("Subset", "", nullptr, m_selection.size() == 1))
            {
                m_layerStack->NewSubset("Layer", m_selection[0], 0);
            }

            if (ImGui::MenuItem("Combination", "", nullptr, m_selection.size() == 2))
            {
                m_layerStack->NewCombination("Layer", m_selection[0], m_selection[1]);
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::PushStyleColor(ImGuiCol_Header,        ImVec4(0.17f, 0.17f, 0.17f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.27f, 0.27f, 0.27f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,  ImVec4(0.09f, 0.09f, 0.09f, 1.00f));

    if (Inputs::IsKeyPressed(KeyCode::Delete)) 
    {
        Clear();
    }

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

    ImGui::PopStyleColor(3);
    ImGui::End();

    return somethingChanged;
}

bool LayerStackWidget::DrawLayer(const LayerPtr& layer, const int& index)
{
    bool somethingChanged = false;

    const std::string layerName = layer->GetName();
    ImGuiTreeNodeFlags nodeFlags = (ImGuiTreeNodeFlags_FramePadding |
                                    ImGuiTreeNodeFlags_Framed |
                                    ImGuiTreeNodeFlags_OpenOnArrow |
                                    ImGuiTreeNodeFlags_AllowItemOverlap |
                                    ImGuiTreeNodeFlags_SpanAvailWidth);

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

    ImGui::Separator();
    bool opened = ImGui::TreeNodeEx((layerName + "##LayerStackNode").c_str(), nodeFlags);
    ImGui::PopStyleColor(popColors);

    bool clicked = ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen();

    // Context menu
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Delete selected layer", "Suppr"))
        {
            DeleteSelectedLayers();
            somethingChanged = true;
        }

        ImGui::EndPopup();
    }

    // Visibility Checkbox
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

    ImGui::SameLine(ImGui::GetWindowWidth() - 23);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
    bool toggled = ImGui::Checkbox((std::string("##VisibilityCBox") + layerName).c_str(), 
                                   &layer->GetVisiblity(),
                                   ImGuiButtonFlags_PressedOnClick);
    somethingChanged |= toggled;
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();

    // Update selection if visibility was not toggled nor item was expanded
    if (!toggled && clicked) 
    {
        // Shift modifier -> Range selection
        if (Inputs::IsKeyPressed(KeyCode::LeftShift)) {
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

        // Ctrl modifier -> Toggle selection
        else if (Inputs::IsKeyPressed(KeyCode::LeftCtrl))
        {
            if (isSelected){
                DeselectLayer(layer);
            } else {
                SelectLayer(layer);
                m_lastIndex = index;
            }
        }

        // No modifier -> Replace selection
        else {
            ClearSelection();
            SelectLayer(layer);
            m_lastIndex = index;
        }
    }

    // Draw the extended data if the item is expanded
    if (opened) {
        somethingChanged |= DrawMapping(layer, isSelected, isSource);
        ImGui::TreePop();
    }

    return somethingChanged;
}


bool DrawMappingComboBox(const LayerPtr& layer)
{
    bool somethingChanged = false;

    const char* mappingNames[] = {"None (empty result)",
                                  "Copy",
                                  "Subset",
                                  "Combination"};
    auto createMapping = [](const uint32_t& index) -> MappingPtr
    {
        switch (index)
        {
            case MappingType::CopyMapping:
                return std::make_shared<Copy>();
            case MappingType::SubsetMapping:
                return std::make_shared<Subset>();
            case MappingType::CombinationMapping:
                return std::make_shared<Combination>();
        }

        return nullptr;
    };

    auto mapping = layer->GetMapping();
    uint32_t currentIndex = mapping ? mapping->GetType() : MappingType::NoMapping;
    if (ImGui::BeginCombo((std::string("##MappingCombo") + layer->GetName()).c_str(), mappingNames[currentIndex]))
    {
        for (size_t i=0 ; i < IM_ARRAYSIZE(mappingNames) ; ++i)
        {
            bool selected = i == currentIndex;
            if (ImGui::Selectable(mappingNames[i], selected)) {
                layer->SetMapping(createMapping(i));
                somethingChanged = true;
            }

            if (selected)
                ImGui::SetItemDefaultFocus();  
        }

        ImGui::EndCombo();
    }

    return somethingChanged;
}


bool DrawOperatorComboBox(const LayerPtr& layer)
{
    bool somethingChanged = false;

    auto indexFromOperator = [](const Operator& op)->uint32_t {
        if (!op) {
            return 0;
        } else if (op == Layer::OuterOp) {
            return 1;
        } else if (op == Layer::InnerOp) {
            return 2;
        } else if (op == Layer::GeomOp) {
            return 3;
        } else {
            return 4;
        }
    };
    const char* opNames[] = {"No operator",
                             "Outer product",
                             "Inner product",
                             "Geometric product",
                             "Custom operator"};
    Operator operators[] = {nullptr,
                            Layer::OuterOp,
                            Layer::InnerOp,
                            Layer::GeomOp};

    uint32_t currentIndex = indexFromOperator(layer->GetOperator());
    if (ImGui::BeginCombo((std::string("##OperatorCombo") + layer->GetName()).c_str(), opNames[currentIndex]))
    {
        for (size_t i=0 ; i < IM_ARRAYSIZE(opNames) ; ++i)
        {
            bool selected = i == currentIndex;
            int flags = (i == 4) ? ImGuiSelectableFlags_Disabled : 0;

            if (ImGui::Selectable(opNames[i], selected, flags)) {
                layer->SetOperator(operators[i]);
                somethingChanged = true;
            }

            if (selected)
                ImGui::SetItemDefaultFocus();  
        }

        ImGui::EndCombo();
    }

    return somethingChanged;
}

bool DrawSourceComboBox(const LayerPtrArray& layers, const LayerPtr& currentLayer, LayerWeakPtr& source, int index)
{
    LayerPtr src = source.lock();
    const char* currentName = src ? src->GetName().c_str() : ""; 

    bool somethingChanged = false;
    if (ImGui::BeginCombo((std::string("##SourceCombo") + currentLayer->GetName() + std::to_string(index)).c_str(), currentName))
    {
        for (size_t i = 0; i < layers.size(); i++)
        {
            const auto& layer = layers[i];
            bool selected = layer == src;
            int flags = layer == currentLayer ? ImGuiSelectableFlags_Disabled : 0;

            if (ImGui::Selectable(layer->GetName().c_str(), selected, flags)) {
                source = layer;
                somethingChanged = true;
            }

            if (selected)
                ImGui::SetItemDefaultFocus();  
        }

        ImGui::EndCombo();
    }

    return somethingChanged;
}


bool LayerStackWidget::DrawMapping(const LayerPtr& layer, 
                                   const bool& isSelected, 
                                   const bool& isSource)
{
    const auto& layers = m_layerStack->GetLayers();
    auto sources = layer->GetSources();

    bool somethingChanged;

    // Mapping
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Mapping :");
    ImGui::SameLine();
    if (somethingChanged = DrawMappingComboBox(layer)) {
        UpdateSources();
    }

    // Sources
    bool sourcesChanged = false;
    auto mapping = layer->GetMapping();
    if (mapping)
    {
        switch (mapping->GetType())
        {
            case MappingType::CopyMapping: {
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Source :");
                ImGui::SameLine();

                if (sources.empty())
                    sources.resize(1);

                sourcesChanged |= DrawSourceComboBox(layers, layer, sources[0], 0);

                break;
            }

            case MappingType::SubsetMapping: {
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Source :");
                ImGui::SameLine();

                if (sources.empty())
                    sources.resize(1);

                sourcesChanged |= DrawSourceComboBox(layers, layer, sources[0], 0);

                break;
            }

            case MappingType::CombinationMapping: {
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Source 1 :");
                ImGui::SameLine();

                if (sources.size() < 2)
                    sources.resize(2);

                sourcesChanged |= DrawSourceComboBox(layers, layer, sources[0], 0);

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Source 2 :");
                ImGui::SameLine();
                sourcesChanged |= DrawSourceComboBox(layers, layer, sources[1], 1);

                break;
            }
        }

        if (sourcesChanged) {
            for (const auto& src : layer->GetSources())
                m_layerStack->DisconnectLayers(src.lock(), layer);

            for (const auto& src : sources)
                m_layerStack->ConnectLayers(src.lock(), layer);

            UpdateSources();
        }
    }

    somethingChanged || sourcesChanged;

    // Operator
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Operator :");            
    ImGui::SameLine();
    somethingChanged |= DrawOperatorComboBox(layer);

    return somethingChanged;
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

bool LayerStackWidget::IsSource(const LayerPtr &layer) 
{
    return std::find(m_sources.begin(), m_sources.end(), layer) != m_sources.end();
}

void LayerStackWidget::UpdateSources() 
{
    m_sources.clear();
    for (const auto& layer : m_selection)
    {
        auto mapping = layer->GetMapping();
        if (!mapping)
            continue;

        const auto& sources = layer->GetSources();
        uint32_t sourceCount = std::min(mapping->GetSourceCount(), 
                                        (uint32_t)sources.size());

        for (uint32_t i=0 ; i < sourceCount ; ++i)
            m_sources.push_back(sources[i].lock());
    }
}
