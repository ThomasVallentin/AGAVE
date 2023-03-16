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

void LayerStackWidget::SetLayerStack(const LayerStackPtr& layerStack)
{
    if (layerStack != m_layerStack)
    {
        m_layerStack = layerStack;
        ClearSelection();
    }
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

    if (Inputs::IsKeyPressed(KeyCode::Delete)) 
    {
        DeleteSelectedLayers();
        somethingChanged = true;
    }

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
    bool opened = false;
    bool somethingChanged = DrawLayerTreeNode(layer, index, opened);

    // Draw the extended data if the item is expanded
    if (opened) {
        somethingChanged |= DrawLayerContent(layer);
        ImGui::Spacing();
        ImGui::TreePop();
    }

    return somethingChanged;
}


// == Tree node ==

bool LayerStackWidget::DrawLayerTreeNode(const LayerPtr& layer, const int& index, bool& opened)
{
    bool somethingChanged = false;

    std::string layerName = layer->GetName();
    uint32_t uuid = layer->GetUUID();
    std::string identifier = std::to_string(uuid);
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

    // Create the tree node
    ImGui::Separator();
    std::string treeNodeLabel = (m_renamedUUID == uuid ? 
                                 std::string("###LayerStackNode") + identifier : 
                                 layerName + "###LayerStackNode" + identifier);
    opened = ImGui::TreeNodeEx(treeNodeLabel.c_str(), nodeFlags);
    ImGui::PopStyleColor(popColors);
    
    // Processing clicked before creating the visibility checkbox it messes the clicked up 
    bool clicked = ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen();

    static char renamedName[256];
    static bool renameIgnoreActive = false;
    if (m_renamedUUID == uuid)
    {
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 13.0f);
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

    return somethingChanged;
}


// == Provider choice ==

bool DrawProviderComboBox(const LayerPtr& layer)
{
    bool somethingChanged = false;

    const char* providerNames[] = {"None (empty layer)",
                                   "Explicit",
                                   "Random generator",
                                   "Subset",
                                   "Combination",
                                   "Self combination"};
    auto createProvider = [](const uint32_t& index) -> ProviderPtr
    {
        switch (index)
        {
            case ProviderType_Explicit:
                return std::make_shared<Explicit>();
            case ProviderType_RandomGenerator:
                return std::make_shared<RandomGenerator>();
            case ProviderType_Subset:
                return std::make_shared<Subset>();
            case ProviderType_SelfCombination:
                return std::make_shared<SelfCombination>();
            case ProviderType_Combination:
                return std::make_shared<Combination>();
        }

        return {};
    };

    auto provider = layer->GetProvider();
    std::string identifier = std::to_string(layer->GetUUID());
    uint32_t currentIndex = provider ? provider->GetType() : ProviderType_None;
    ImGui::SetNextItemWidth(150);
    if (ImGui::BeginCombo((std::string("##ProviderCombo") + identifier).c_str(), providerNames[currentIndex]))
    {
        for (size_t i=0 ; i < IM_ARRAYSIZE(providerNames) ; ++i)
        {
            bool selected = i == currentIndex;
            if (ImGui::Selectable(providerNames[i], selected)) {
                layer->SetProvider(createProvider(i));
                somethingChanged = true;
            }

            if (selected)
                ImGui::SetItemDefaultFocus();  
        }

        ImGui::EndCombo();
    }

    return somethingChanged;
}


// == Operator choice ==

bool DrawOperatorComboBox(const LayerPtr& layer, const std::shared_ptr<OperatorBasedProvider>& provider)
{
    bool somethingChanged = false;

    auto indexFromOperator = [](const Operator& op)->uint32_t {
        if (!op) {
            return 0;
        } else if (op == Operators::OuterProduct) {
            return 1;
        } else if (op == Operators::InnerProduct) {
            return 2;
        } else if (op == Operators::GeomProduct) {
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
                            Operators::OuterProduct,
                            Operators::InnerProduct,
                            Operators::GeomProduct};

    uint32_t currentIndex = indexFromOperator(provider->GetOperator());
    if (ImGui::BeginCombo((std::string("##OperatorCombo") + std::to_string(layer->GetUUID())).c_str(), 
                          opNames[currentIndex]))
    {
        for (size_t i=0 ; i < IM_ARRAYSIZE(opNames) ; ++i)
        {
            bool selected = i == currentIndex;
            int flags = (i == 4) ? ImGuiSelectableFlags_Disabled : 0;

            if (ImGui::Selectable(opNames[i], selected, flags)) {
                provider->SetOperator(operators[i]);
                layer->SetDirty(DirtyBits_Provider);
                somethingChanged = true;
            }

            if (selected)
                ImGui::SetItemDefaultFocus();  
        }

        ImGui::EndCombo();
    }

    return somethingChanged;
}


// == Explicit provider content ==

bool DrawDragControl(const char* label, 
                     const char* name, 
                     float* value, 
                     const ImVec4& color, 
                     const float& sensitivity=0.05f, 
                     const float& width=100.0f)
{
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 3));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 3));
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);

    ImGui::SetNextItemWidth(20);
    ImGui::Button(label);

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    ImGui::SameLine();
    ImGui::SetNextItemWidth(width - 20);
    bool result =  ImGui::DragFloat(name, value, sensitivity, 0.0f, 0.0f, "%.2f");
    ImGui::PopStyleVar();
    
    return result;
}

bool DrawPositionControl(const std::string& name, float* position, const float& sensitivity=0.05f, const float& width=100.0f) 
{
    bool somethingChanged = false;

    // X
    somethingChanged |= DrawDragControl("X", (std::string("##PositionDragFloatX") + name).c_str(), 
                                        position, 
                                        ImVec4(0.8, 0.1, 0.1, 1.0),
                                        sensitivity,
                                        width);

    // Y
    ImGui::SameLine();
    somethingChanged |= DrawDragControl("Y", (std::string("##PositionDragFloatY") + name).c_str(), 
                                        &position[1], 
                                        ImVec4(0.1, 0.6, 0.1, 1.0),
                                        sensitivity,
                                        width);

    // Z
    ImGui::SameLine();
    somethingChanged |= DrawDragControl("Z", (std::string("##PositionDragFloatZ") + name).c_str(), 
                                        &position[2], 
                                        ImVec4(0.2, 0.2, 0.8, 1.0),
                                        sensitivity,
                                        width);

    return somethingChanged;
}

bool DrawPlaneControl(const std::string& name, float* plane, const float& sensitivity=0.05f, const float& width=100.0f) 
{
    bool somethingChanged = false;

    // A
    somethingChanged |= DrawDragControl("A", (std::string("##PlaneDragFloatX") + name).c_str(), 
                                        plane, 
                                        ImVec4(0.2, 0.2, 0.8, 1.0),
                                        sensitivity,
                                        width);

    // B
    ImGui::SameLine();
    somethingChanged |= DrawDragControl("B", (std::string("##PlaneDragFloatY") + name).c_str(), 
                                        &plane[1], 
                                        ImVec4(0.2, 0.2, 0.8, 1.0),
                                        sensitivity,
                                        width);

    // C
    ImGui::SameLine();
    somethingChanged |= DrawDragControl("C", (std::string("##PlaneDragFloatC") + name).c_str(), 
                                        &plane[2], 
                                        ImVec4(0.2, 0.2, 0.8, 1.0),
                                        sensitivity,
                                        width);

    // Z
    ImGui::SameLine();
    somethingChanged |= DrawDragControl("D", (std::string("##PlaneDragFloatZ") + name).c_str(), 
                                        &plane[3], 
                                        ImVec4(0.2, 0.2, 0.8, 1.0),
                                        sensitivity,
                                        width);

    return somethingChanged;
}

bool DrawRadiusControl(const std::string& name, float* radius, const float& sensitivity=0.05f, const float& width=100.0f)
{
    return DrawDragControl("R", (std::string("##RadiusDragFloatY") + name).c_str(), 
                           radius, 
                           ImVec4(0.3, 0.3, 0.3, 1.0),
                           sensitivity,
                           width);
}

bool DrawExplicitProvider(const LayerPtr& layer) 
{
    bool somethingChanged = false;
    c3ga::MvecType types[] = {c3ga::MvecType::Point,
                              c3ga::MvecType::Sphere,
                              c3ga::MvecType::DualSphere,
                              c3ga::MvecType::Plane,
                              c3ga::MvecType::DualPlane};
    const char* typeNames[] = {"Point",
                               "Sphere",
                               "DualSphere",
                               "Plane",
                               "DualPlane"};
    auto& objects = layer->GetObjects();
    auto provider = std::dynamic_pointer_cast<Explicit>(layer->GetProvider());

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Content :");

    ImGui::BeginDisabled(provider->IsAnimated());
    for (size_t i=0 ; i < objects.size() ; )
    {
        auto& obj = objects[i];
        c3ga::MvecType objType = c3ga::getTypeOf(obj);
        std::string objTypeName = c3ga::typeToName(objType);

        // Type combo box
        std::string identifier = std::to_string(layer->GetUUID()) + std::to_string(i);
        ImGui::SetNextItemWidth(100);
        if (ImGui::BeginCombo((std::string("##ObjTypeCombo") + identifier).c_str(), objTypeName.c_str()))
        {
            for (size_t i=0 ; i < IM_ARRAYSIZE(typeNames) ; i++)
            {
                bool selected = objTypeName == typeNames[i];
                if (ImGui::Selectable(typeNames[i], selected) && !selected) {
                    obj = convert(obj, objType, types[i]);
                    objType = types[i];
                    somethingChanged = true;
                }

                if (selected)
                    ImGui::SetItemDefaultFocus();  
            }
            ImGui::EndCombo();
        }

        // Details
        float availableWidth = ImGui::GetContentRegionAvail().x - 100.0f - 14.0f;
        float sensitivity = 0.05f;
        ImVec2 removeButtonOffset(0.0f, 1.0f);
        switch (objType)
        {
            case c3ga::MvecType::Point: {
                float position[3] = {(float)obj[c3ga::E1], (float)obj[c3ga::E2], (float)obj[c3ga::E3]};
                ImGui::SameLine();
                if (DrawPositionControl(identifier, position, sensitivity, availableWidth / 3.0f)) {
                    obj = c3ga::point((double)position[0], (double)position[1], (double)position[2]);
                    somethingChanged = true;
                }
                break;
            }

            case c3ga::MvecType::Sphere:
            case c3ga::MvecType::ImaginarySphere: {
                c3ga::Mvec<double> c;
                double r;
                c3ga::radiusAndCenterFromDualSphere(-obj.dual(), r, c);

                ImGui::SameLine();
                float center[3] = {(float)c[c3ga::E1], (float)c[c3ga::E2], (float)c[c3ga::E3]};
                bool sphereChanged = DrawPositionControl(identifier, center, sensitivity, availableWidth / 4.0f);

                ImGui::SameLine();
                float radius = (float)r;
                sphereChanged |= DrawRadiusControl(identifier, &radius, sensitivity, availableWidth / 4.0f);
                if (radius == 0.0)
                    radius = 1e-6;

                if (sphereChanged) {
                    obj = c3ga::dualSphere((double)center[0], 
                                           (double)center[1], 
                                           (double)center[2], 
                                           (double)radius).dual();
                    somethingChanged = true;
                }
                break;
            }
            
            case c3ga::MvecType::DualSphere:
            case c3ga::MvecType::ImaginaryDualSphere: {
                c3ga::Mvec<double> c;
                double r;
                c3ga::radiusAndCenterFromDualSphere(obj, r, c);
                
                ImGui::SameLine();
                float center[3] = {(float)c[c3ga::E1], (float)c[c3ga::E2], (float)c[c3ga::E3]};
                bool sphereChanged = DrawPositionControl(identifier, center, sensitivity, availableWidth / 4.0f);

                ImGui::SameLine();
                float radius = (float)r;
                sphereChanged |= DrawRadiusControl(identifier, &radius, sensitivity, availableWidth / 4.0f);
                if (radius == 0.0)
                    radius = 1e-6;

                if (sphereChanged) {
                    obj = c3ga::dualSphere((double)center[0], 
                                           (double)center[1], 
                                           (double)center[2], 
                                           (double)radius);
                    somethingChanged = true;
                }
                break;
            }
        
            case c3ga::MvecType::Plane: {
                auto plane = obj.dual();
                float values[4] = {(float)plane[c3ga::E1], 
                                   (float)plane[c3ga::E2],
                                   (float)plane[c3ga::E3],
                                   (float)plane[c3ga::Ei]
                                  };

                ImGui::SameLine();
                if (DrawPlaneControl(identifier, values, sensitivity, availableWidth / 4.0f)) {
                    plane = c3ga::Mvec<double>();
                    plane[c3ga::E1] = (double)values[0];
                    plane[c3ga::E2] = (double)values[1];
                    plane[c3ga::E3] = (double)values[2];
                    plane[c3ga::Ei] = (double)values[3];
                    obj = -plane.dual();
                    somethingChanged = true;
                }
                break;
            }

            case c3ga::MvecType::DualPlane: {
                float values[4] = {(float)obj[c3ga::E1], 
                                   (float)obj[c3ga::E2],
                                   (float)obj[c3ga::E3],
                                   (float)obj[c3ga::Ei]
                                  };

                LOG_INFO("pre %f, %f, %f, %f", values[0], values[1], values[2], values[3]);
                if (DrawPlaneControl(identifier, values, sensitivity, availableWidth / 4.0f)) {
                    obj[c3ga::E1] = (double)values[0];
                    obj[c3ga::E2] = (double)values[1];
                    obj[c3ga::E3] = (double)values[2];
                    obj[c3ga::Ei] = (double)values[3];
                    somethingChanged = true;
                }
                break;
            }
        
            case c3ga::MvecType::Line: {
                bool lineChanged = false;
                auto line = obj.dual();

                c3ga::Mvec<double> origin, direction;
                c3ga::originAndDirectionFromDualLine(line, origin, direction);
                direction /= direction.norm();
                float org[3] = {(float)origin[c3ga::E1], (float)origin[c3ga::E2], (float)origin[c3ga::E3]};
                ImGui::SameLine();
                ImGui::Text("Origin");
                ImGui::SameLine();
                availableWidth -= 39.0f;
                if (DrawPositionControl(identifier, org, sensitivity, availableWidth / 3.0f)) {
                    origin = c3ga::point((double)org[0], (double)org[1], (double)org[2]);
                    lineChanged = true;
                }

                float dir[3] = {(float)direction[c3ga::E1], (float)direction[c3ga::E2], (float)direction[c3ga::E3]};
                ImGui::SetCursorPosX(142);
                ImGui::Text("Dir");
                ImGui::SameLine();
                if (DrawPositionControl(identifier + "Direction", dir, sensitivity, availableWidth / 3.0f)) {
                    direction = c3ga::vector((double)dir[0], (double)dir[1], (double)dir[2]);
                    lineChanged = true;
                }

                if (lineChanged)
                {
                    obj = (origin ^ direction ^ c3ga::ei<double>());
                    somethingChanged |= true;
                }

                removeButtonOffset = ImVec2(0.0f, -9.0f);

                break;
            }

            case c3ga::MvecType::DualLine: {
                bool lineChanged = false;

                c3ga::Mvec<double> origin, direction;
                c3ga::originAndDirectionFromDualLine(obj, origin, direction);
                direction /= direction.norm();
                float org[3] = {(float)origin[c3ga::E1], (float)origin[c3ga::E2], (float)origin[c3ga::E3]};
                ImGui::SameLine();
                ImGui::Text("Origin");
                ImGui::SameLine();
                availableWidth -= 39.0f;
                if (DrawPositionControl(identifier, org, sensitivity, availableWidth / 3.0f)) {
                    origin = c3ga::point((double)org[0], (double)org[1], (double)org[2]);
                    lineChanged = true;
                }

                float dir[3] = {(float)direction[c3ga::E1], (float)direction[c3ga::E2], (float)direction[c3ga::E3]};
                ImGui::SetCursorPosX(142);
                ImGui::Text("Dir");
                ImGui::SameLine();
                if (DrawPositionControl(identifier + "Direction", dir, sensitivity, availableWidth / 3.0f)) {
                    direction = c3ga::vector((double)dir[0], (double)dir[1], (double)dir[2]);
                    lineChanged = true;
                }

                if (lineChanged)
                {
                    obj = (origin ^ direction ^ c3ga::ei<double>());
                    somethingChanged |= true;
                }

                removeButtonOffset = ImVec2(0.0f, -9.0f);

                break;
            }
        }

        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 16.0f);
        ImGui::SetCursorPos(ImGui::GetCursorPos() + removeButtonOffset);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 2));
        if (ImGui::Button((std::string("X##RemoveButton") + identifier).c_str()))
        {
            objects.erase(objects.begin() + i);
            somethingChanged = true;
        } else {
            ++i;
        }
        ImGui::PopStyleVar();
    }
    ImGui::EndDisabled();

    ImGui::Spacing();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 1));
    if (ImGui::Button("+"))
    {
        objects.push_back(c3ga::point(0.0, 0.0, 0.0));
        somethingChanged = true;
    }
    ImGui::PopStyleVar();

    if (somethingChanged) {
        layer->SetDirty(DirtyBits_Provider);
    }

    return somethingChanged;
}


// == RandomGenerator ==

bool DrawRandomGenerator(const LayerPtr& layer)
{
    bool somethingChanged = false;
    auto provider = std::dynamic_pointer_cast<RandomGenerator>(layer->GetProvider());
    std::string identifier = std::to_string(layer->GetUUID());

    // ObjectType combo box
    c3ga::MvecType types[] = {c3ga::MvecType::Point,
                              c3ga::MvecType::Sphere,
                              c3ga::MvecType::DualSphere};
    const char* typeNames[] = {"Point",
                               "Sphere",
                               "DualSphere"};

    c3ga::MvecType objType = provider->GetObjectType();
    const char* objTypeName = c3ga::typeToName(objType).c_str();

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Type :");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::BeginCombo((std::string("##RandomGeneratorObjTypeCombo") + identifier).c_str(), objTypeName))
    {
        for (size_t i=0 ; i < IM_ARRAYSIZE(typeNames) ; i++)
        {
            bool selected = objTypeName == typeNames[i];
            if (ImGui::Selectable(typeNames[i], selected) && !selected) {
                provider->SetObjectType(types[i]);
                layer->SetDirty(DirtyBits_Provider);
                somethingChanged = true;
            }

            if (selected)
                ImGui::SetItemDefaultFocus();  
        }
        ImGui::EndCombo();
    }

    int count = provider->GetCount();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Count :");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(75);
    if (ImGui::DragInt((std::string("##RandomGeneratorCountDrag") + identifier).c_str(), &count, 0.05f, 0, 10000))
    {
        provider->SetCount(count);
        layer->SetDirty(DirtyBits_Provider);
        somethingChanged = true;
    }

    float extents = provider->GetExtents();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Extents :");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(75);
    if (ImGui::DragFloat((std::string("##RandomGeneratorExtentsDrag") + identifier).c_str(), &extents, 0.05f))
    {
        provider->SetExtents(extents);
        layer->SetDirty(DirtyBits_Provider);
        somethingChanged = true;
    }

    return somethingChanged;
}


// == Subset ==

bool DrawSubsetProvider(const LayerPtr& layer)
{
    auto provider = std::dynamic_pointer_cast<Subset>(layer->GetProvider());
    int count = provider->GetCount();

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Count :");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(75);
    if (ImGui::DragInt((std::string("##SubsetCountDrag") + std::to_string(layer->GetUUID())).c_str(), &count, 0.05f, -1, 1000))
    {
        provider->SetCount(count);
        layer->SetDirty(DirtyBits_Provider);
        return true;
    }

    return false;
}
 

// == Self combination ==

bool DrawSelfCombinationProvider(const LayerPtr& layer)
{
    auto provider = std::dynamic_pointer_cast<SelfCombination>(layer->GetProvider());
    int count = provider->GetCount();
    int dimension = provider->GetDimension();

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Count :");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(75);
    if (ImGui::DragInt((std::string("##SelfCombinationCountDrag") + std::to_string(layer->GetUUID())).c_str(), &count, 0.05f, -1, 1000))
    {
        provider->SetCount(count);
        layer->SetDirty(DirtyBits_Provider);
        return true;
    }

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Dimension :");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(75);
    if (ImGui::DragInt((std::string("##SelfCombinationDimensionDrag") + std::to_string(layer->GetUUID())).c_str(), &dimension, 0.05f, 0, 4))
    {
        provider->SetDimension(dimension);
        layer->SetDirty(DirtyBits_Provider);
        return true;
    }

    return false;
}
 

// == Sources ==

bool DrawSource(const LayerPtrArray& layers, const LayerPtr& currentLayer, LayerWeakPtr& source, int index)
{
    LayerPtr src = source.lock();
    const char* currentName = src ? src->GetName().c_str() : ""; 
    const char* identifier = (std::to_string(currentLayer->GetUUID()) + std::to_string(index)).c_str();

    bool somethingChanged = false;
    ImGui::SetNextItemWidth(150.0f);
    if (ImGui::BeginCombo((std::string("##SourceCombo") + identifier).c_str(), currentName))
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

    bool dual = currentLayer->SourceIsDual(index);
    ImGui::SameLine();
    if (ImGui::CheckedTextButton((std::string("SourceDualCBox") + identifier).c_str(), "D*", "D", dual))
    {
        currentLayer->SetSourceDual(index, !dual);
    }

    return somethingChanged;
}


// == Layer content ==

bool LayerStackWidget::DrawLayerContent(const LayerPtr& layer)
{
    const auto& layers = m_layerStack->GetLayers();
    auto sources = layer->GetSources();

    bool somethingChanged = false;

    // Provider
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Provider :");
    ImGui::SameLine();
    if (somethingChanged |= DrawProviderComboBox(layer)) {
        UpdateSources();
    }

    // Sources
    bool sourcesChanged = false;
    auto provider = layer->GetProvider();
    if (provider)
    {
        switch (provider->GetType())
        {
            case ProviderType_Explicit: {
                somethingChanged |= DrawExplicitProvider(layer);
                break;
            }

            case ProviderType_RandomGenerator: {
                somethingChanged |= DrawRandomGenerator(layer);
                break;
            }

            case ProviderType_Subset: {
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Source :");
                ImGui::SameLine();

                if (sources.empty())
                    sources.resize(1);

                sourcesChanged |= DrawSource(layers, layer, sources[0], 0);

                sourcesChanged |= DrawSubsetProvider(layer);

                break;
            }

            case ProviderType_Combination: {
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Source 1 :");
                ImGui::SameLine();

                if (sources.size() < 2)
                    sources.resize(2);

                sourcesChanged |= DrawSource(layers, layer, sources[0], 0);

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Source 2 :");
                ImGui::SameLine();
                sourcesChanged |= DrawSource(layers, layer, sources[1], 1);

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Operator :");
                ImGui::SameLine();
                somethingChanged |= DrawOperatorComboBox(layer, std::dynamic_pointer_cast<OperatorBasedProvider>(provider));

                break;
            }

            case ProviderType_SelfCombination: {
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Source :");
                ImGui::SameLine();

                if (sources.empty())
                    sources.resize(1);

                sourcesChanged |= DrawSource(layers, layer, sources[0], 0);

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Operator :");
                ImGui::SameLine();
                somethingChanged |= DrawOperatorComboBox(layer, std::dynamic_pointer_cast<OperatorBasedProvider>(provider));
                somethingChanged |= DrawSelfCombinationProvider(layer);

                break;
            }

        }

        if (sourcesChanged) {
            for (const auto& src : layer->GetSources())
                m_layerStack->DisconnectLayers(src.lock(), layer);

            for (const auto& src : sources) {
                m_layerStack->ConnectLayers(src.lock(), layer);

            }

            UpdateSources();
        }
    }

    somethingChanged |= sourcesChanged;

    return somethingChanged;
}


// == Selection management ==

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
