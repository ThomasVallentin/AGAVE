#include "ContentEditor.hpp"

#include "Resources.hpp"
#include "Buttons.hpp"

#include "Base/Inputs.h"
#include "Base/Logging.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

static const char* kEmptySelectionMessage = "Select a layer to view and edit its content.";

static const c3ga::MvecType mvecTypes[] = {
    c3ga::MvecType::Point,
    c3ga::MvecType::Sphere,
    c3ga::MvecType::DualSphere,
    c3ga::MvecType::Plane,
    c3ga::MvecType::DualPlane,
    c3ga::MvecType::Line,
    c3ga::MvecType::DualLine,
    c3ga::MvecType::Circle,     // = DualPairPoint
    c3ga::MvecType::PairPoint   // = DualCircle
    };


static uint32_t contentEditorIdCounter = 0;

ContentEditor::ContentEditor(const LayerStackPtr& layerStack)
{
    m_layerStack = layerStack;
    m_id = contentEditorIdCounter++;
}

ContentEditor::~ContentEditor()
{

}


// == Provider choice ==

bool DrawProviderComboBox(const LayerPtr& layer)
{
    bool somethingChanged = false;

    const char* providerNames[] = {"Explicit",
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
    uint32_t currentIndex = provider ? provider->GetType() : ProviderType_Explicit;
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


// == RandomGenerator ==

bool DrawRandomGenerator(const LayerPtr& layer, const DualMode& dualMode)
{
    bool somethingChanged = false;
    auto provider = std::dynamic_pointer_cast<RandomGenerator>(layer->GetProvider());
    std::string identifier = std::to_string(layer->GetUUID());

    // ObjectType combo box
    c3ga::MvecType types[] = {c3ga::MvecType::Point,
                              c3ga::MvecType::Sphere,
                              c3ga::MvecType::DualSphere,
                              c3ga::MvecType::Plane,
                              c3ga::MvecType::DualPlane,
                              c3ga::MvecType::PairPoint,
                              c3ga::MvecType::DualPairPoint,
                              };
    const char* typeNames[] = {"Point",
                               "Sphere",
                               "DualSphere",
                               "Plane",
                               "DualPlane",
                               "PairPoint",
                               "DualPairPoint",
                               };

    c3ga::MvecType objType = provider->GetObjectType();
    std::string objTypeName = c3ga::typeToName(objType, true, !(dualMode & DualMode_Default));

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Type :");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::BeginCombo((std::string("##RandomGeneratorObjTypeCombo") + identifier).c_str(), objTypeName.c_str()))
    {
        for (size_t i=0 ; i < IM_ARRAYSIZE(typeNames) ; i++)
        {
            bool selected = objTypeName.c_str() == typeNames[i];
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
    bool somethingChanged = false;
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
        somethingChanged = true;
    }

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Dimension :");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(75);
    if (ImGui::DragInt((std::string("##SelfCombinationDimensionDrag") + std::to_string(layer->GetUUID())).c_str(), &dimension, 0.05f, 0, 4))
    {
        provider->SetDimension(dimension);
        layer->SetDirty(DirtyBits_Provider);
        somethingChanged = true;
    }

    auto prodWithEi = provider->GetProductWithEi();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Last product with Ei :");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(75);
    if (ImGui::Checkbox((std::string("##SelfCombinationEi") + std::to_string(layer->GetUUID())).c_str(), &prodWithEi))
    {
        provider->SetProductWithEi(prodWithEi);
        layer->SetDirty(DirtyBits_Provider);
        somethingChanged = true;
    }

    return somethingChanged;
}
 

// == Sources ==

bool DrawSource(const LayerPtrArray& layers, const LayerPtr& currentLayer, LayerWeakPtr& source, int index)
{
    LayerPtr src = source.lock();
    std::string currentName = src ? src->GetName().c_str() : ""; 
    const char* identifier = (std::to_string(currentLayer->GetUUID()) + std::to_string(index)).c_str();

    bool somethingChanged = false;
    ImGui::SetNextItemWidth(150.0f);
    if (ImGui::BeginCombo((std::string("##SourceCombo") + identifier).c_str(), currentName.c_str()))
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


// == Controls ==

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

bool DrawVectorControl(const std::string& identifier, c3ga::Mvec<double>& vector, const float& sensitivity=0.05f, const float& width=100.0f) 
{
    bool somethingChanged = false;
    float values[3] = {(float)vector[c3ga::E1], (float)vector[c3ga::E2], (float)vector[c3ga::E3]};
    float unitWidth = width / 3.0f;

    // X
    somethingChanged |= DrawDragControl("X", (std::string("##VectorControlZX") + identifier).c_str(), 
                                        values, 
                                        ImVec4(0.8, 0.1, 0.1, 1.0),
                                        sensitivity,
                                        unitWidth);

    // Y
    ImGui::SameLine();
    somethingChanged |= DrawDragControl("Y", (std::string("##VectorControlY") + identifier).c_str(), 
                                        &values[1], 
                                        ImVec4(0.1, 0.6, 0.1, 1.0),
                                        sensitivity,
                                        unitWidth);

    // Z
    ImGui::SameLine();
    somethingChanged |= DrawDragControl("Z", (std::string("##VectorControlZ") + identifier).c_str(), 
                                        &values[2], 
                                        ImVec4(0.2, 0.2, 0.8, 1.0),
                                        sensitivity,
                                        unitWidth);

    if (somethingChanged)
        vector = c3ga::vector<double>((double)values[0], (double)values[1], (double)values[2]);

    return somethingChanged;
}

bool DrawPointControl(const std::string& identifier, c3ga::Mvec<double>& point, const float& sensitivity=0.05f, const float& width=100.0f) 
{
    bool somethingChanged = DrawVectorControl(identifier, point, sensitivity, width);
    if (somethingChanged)
        point = c3ga::point(point);

    return somethingChanged;
}

bool DrawDualLineToLineControl(const std::string& identifier, c3ga::Mvec<double>& dualLine, const float& sensitivity=0.05f, const float& width=100.0f) 
{
    bool somethingChanged = false;
    c3ga::Mvec<double> origin, direction;
    c3ga::originAndDirectionFromDualLine(dualLine, origin, direction);
    direction /= direction.norm();

    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float textWidth = ImGui::CalcTextSize("Orig:").x;
    float indent = ImGui::GetCursorPosX() + textWidth;
    float remainingWidth = width - textWidth - spacing;

    ImGui::Text("Orig:");
    ImGui::SameLine();
    somethingChanged |= DrawPointControl(identifier, origin, sensitivity, remainingWidth);

    ImGui::SetCursorPosX(indent - ImGui::CalcTextSize("Dir:").x);
    ImGui::Text("Dir:");
    ImGui::SameLine();
    somethingChanged |= DrawVectorControl(identifier, direction, sensitivity, remainingWidth);

    if (somethingChanged)
        dualLine = (origin ^ direction ^ c3ga::ei<double>());

    return somethingChanged;
}

bool DrawDualCircleControl(const std::string& identifier, c3ga::Mvec<double>& dualCircle, const float& sensitivity=0.05f, const float& width=100.0f) 
{
    bool somethingChanged = false;
    double r;
    c3ga::Mvec<double> center, direction;
    c3ga::extractDualCircle(dualCircle, r, center, direction);

    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float textWidth = ImGui::CalcTextSize("Center:").x;
    float indent = ImGui::GetCursorPosX() + textWidth;
    float remainingWidth = width - textWidth - spacing;

    // Center
    ImGui::Text("Center:");
    ImGui::SameLine();
    somethingChanged |= DrawPointControl(identifier + "Pos", center, sensitivity, remainingWidth * 0.75f);

    // Radius
    ImGui::SameLine();
    float radius = (float)r;
    somethingChanged |= DrawDragControl("R", (std::string("##DualCircleControlRadius") + identifier).c_str(), 
                                        &radius, 
                                        ImVec4(0.3, 0.3, 0.3, 1.0),
                                        sensitivity,
                                        remainingWidth * 0.25f);
    
    // Direction
    ImGui::SetCursorPosX(indent - ImGui::CalcTextSize("Dir:").x);
    ImGui::Text("Dir:");
    ImGui::SameLine();
    somethingChanged |= DrawVectorControl(identifier + "Dir", direction, sensitivity, remainingWidth);

    if (somethingChanged)
    {
        r = radius == 0 ? 1e-6 : (double)radius;  // Avoid degenerate mvecs when radius is zero.
        direction = direction.norm() == 0 ? c3ga::vector<double>(0, 1, 0) : direction;
        dualCircle = c3ga::dualCircle(center, direction / direction.norm(), r);
    }

    return somethingChanged;
}

bool DrawPairPointControl(const std::string& identifier, c3ga::Mvec<double>& pairPoint, const float& sensitivity=0.05f, const float& width=100.0f) 
{
    bool somethingChanged = false;
    c3ga::Mvec<double> p1, p2;
    c3ga::extractPairPoint(pairPoint, p1, p2);

    float indent = ImGui::GetCursorPosX();

    // Point 1
    somethingChanged |= DrawPointControl(identifier + "Point1", p1, sensitivity, width);

    // Point 2
    ImGui::SetCursorPosX(indent);
    somethingChanged |= DrawPointControl(identifier + "Point2", p2, sensitivity, width);

    if (somethingChanged)
        pairPoint = p1 ^ p2;

    return somethingChanged;
}

bool DrawDualPlaneControl(const std::string& identifier, c3ga::Mvec<double>& dualPlane, const float& sensitivity=0.05f, const float& width=100.0f) 
{
    bool somethingChanged = false;
    float unitWidth = width / 4.0f;
    float values[4] = {(float)dualPlane[c3ga::E1], 
                       (float)dualPlane[c3ga::E2],
                       (float)dualPlane[c3ga::E3],
                       (float)dualPlane[c3ga::Ei]
                      };
    // A
    somethingChanged |= DrawDragControl("A", (std::string("##DualPlaneControlA") + identifier).c_str(), 
                                        values, 
                                        ImVec4(0.2, 0.2, 0.8, 1.0),
                                        sensitivity,
                                        unitWidth);

    // B
    ImGui::SameLine();
    somethingChanged |= DrawDragControl("B", (std::string("##DualPlaneControlB") + identifier).c_str(), 
                                        &values[1], 
                                        ImVec4(0.2, 0.2, 0.8, 1.0),
                                        sensitivity,
                                        unitWidth);

    // C
    ImGui::SameLine();
    somethingChanged |= DrawDragControl("C", (std::string("##DualPlaneControlC") + identifier).c_str(), 
                                        &values[2], 
                                        ImVec4(0.2, 0.2, 0.8, 1.0),
                                        sensitivity,
                                        unitWidth);

    // Z
    ImGui::SameLine();
    somethingChanged |= DrawDragControl("D", (std::string("##DualPlaneControlD") + identifier).c_str(), 
                                        &values[3], 
                                        ImVec4(0.2, 0.2, 0.8, 1.0),
                                        sensitivity,
                                        unitWidth);

    if (somethingChanged)
    {
        dualPlane = c3ga::Mvec<double>();
        dualPlane[c3ga::E1] = (double)values[0];
        dualPlane[c3ga::E2] = (double)values[1];
        dualPlane[c3ga::E3] = (double)values[2];
        dualPlane[c3ga::Ei] = (double)values[3];
    }

    return somethingChanged;
}

bool DrawDualSphereControl(const std::string& identifier, c3ga::Mvec<double>& dualSphere, const float& sensitivity=0.05f, const float& width=100.0f) 
{
    bool somethingChanged = false;
    float unitWidth = width / 4.0f;

    c3ga::Mvec<double> c;
    double r;
    c3ga::radiusAndCenterFromDualSphere(dualSphere, r, c);
    float center[3] = {(float)c[c3ga::E1], (float)c[c3ga::E2], (float)c[c3ga::E3]};
    float radius = (float)r;

    // X
    somethingChanged |= DrawDragControl("X", (std::string("##DualSphereControlX") + identifier).c_str(), 
                                        center, 
                                        ImVec4(0.8, 0.1, 0.1, 1.0),
                                        sensitivity,
                                        unitWidth);

    // Y
    ImGui::SameLine();
    somethingChanged |= DrawDragControl("Y", (std::string("##DualSphereControlY") + identifier).c_str(), 
                                        &center[1], 
                                        ImVec4(0.1, 0.6, 0.1, 1.0),
                                        sensitivity,
                                        unitWidth);

    // Z
    ImGui::SameLine();
    somethingChanged |= DrawDragControl("Z", (std::string("##DualSphereControlZ") + identifier).c_str(), 
                                        &center[2], 
                                        ImVec4(0.2, 0.2, 0.8, 1.0),
                                        sensitivity,
                                        unitWidth);

    // Radius
    ImGui::SameLine();
    somethingChanged |= DrawDragControl("R", (std::string("##DualSphereControlRadius") + identifier).c_str(), 
                                        &radius, 
                                        ImVec4(0.3, 0.3, 0.3, 1.0),
                                        sensitivity,
                                        unitWidth);
    
    // Avoiding degeneracy since it causes the mvec type to change
    if (radius == 0.0)
        radius = 1e-6;

    if (somethingChanged)
        dualSphere = c3ga::dualSphere((double)center[0], 
                                      (double)center[1], 
                                      (double)center[2], 
                                      (double)radius);

    return somethingChanged;
}


// == Content ==

bool DrawLayerContent(const LayerPtr& layer, const DualMode& dualMode) 
{
    bool somethingChanged = false;
    bool preferDual = !(dualMode & DualMode_Default);

    auto& objects = layer->GetObjects();
    auto provider = layer->GetProvider();
    bool isExplicit = provider->GetType() == ProviderType_Explicit;
    bool enabled = isExplicit && !std::dynamic_pointer_cast<Explicit>(provider)->IsAnimated();
    ImGui::BeginDisabled(!enabled);
    for (size_t index=0 ; index < objects.size() ; )
    {
        auto& obj = objects[index];
        c3ga::MvecType objType = c3ga::getTypeOf(obj);
        std::string objTypeName = c3ga::typeToName(objType, true, preferDual);

        // Type combo box
        std::string identifier = std::to_string(layer->GetUUID()) + std::to_string(index);
        ImGui::SetNextItemWidth(100);
        if (ImGui::BeginCombo((std::string("##ObjTypeCombo") + identifier).c_str(), objTypeName.c_str()))
        {
            for (size_t i=0 ; i < IM_ARRAYSIZE(mvecTypes) ; i++)
            {
                bool selected = objType == mvecTypes[i];
                if (ImGui::Selectable(c3ga::typeToName(mvecTypes[i], true, preferDual).c_str(), selected) && !selected) {
                    obj = convert(obj, objType, mvecTypes[i]);
                    objType = mvecTypes[i];
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
                ImGui::SameLine();
                somethingChanged |= DrawPointControl(identifier, obj, sensitivity, availableWidth);
                break;
            }

            case c3ga::MvecType::Sphere:
            case c3ga::MvecType::ImaginarySphere: {
                auto dualSphere = -obj.dual();
                ImGui::SameLine();
                if (DrawDualSphereControl(identifier, dualSphere, sensitivity, availableWidth)) 
                {
                    somethingChanged = true;
                    obj = dualSphere.dual();
                }
                
                break;
            }
            
            case c3ga::MvecType::DualSphere:
            case c3ga::MvecType::ImaginaryDualSphere: {
                ImGui::SameLine();
                somethingChanged |= DrawDualSphereControl(identifier, obj, sensitivity, availableWidth);
                break;
            }
        
            case c3ga::MvecType::Plane: {
                auto dualPlane = obj.dual();
                ImGui::SameLine();
                if (DrawDualPlaneControl(identifier, dualPlane, sensitivity, availableWidth)) {
                    somethingChanged = true;
                    obj = -dualPlane.dual();
                }

                break;
            }

            case c3ga::MvecType::DualPlane: {
                ImGui::SameLine();
                somethingChanged |= DrawDualPlaneControl(identifier, obj, sensitivity, availableWidth);
                break;
            }
        
            case c3ga::MvecType::Line: {
                auto dualLine = obj.dual();
                ImGui::SameLine();
                if (DrawDualLineToLineControl(identifier, dualLine, sensitivity, availableWidth)) {
                    somethingChanged = true;
                    obj = dualLine;
                }

                removeButtonOffset = ImVec2(0.0f, -9.0f);

                break;
            }

            case c3ga::MvecType::DualLine: {
                ImGui::SameLine();
                if (DrawDualLineToLineControl(identifier, obj, sensitivity, availableWidth)) {
                    somethingChanged = true;
                    obj = obj.dual();
                }

                removeButtonOffset = ImVec2(0.0f, -9.0f);

                break;
            }

            case c3ga::MvecType::Circle:
            case c3ga::MvecType::ImaginaryCircle: {
                if (preferDual)
                {
                    // Dual pair point control
                    auto pairPoint = obj.dual();
                    ImGui::SameLine();
                    if(DrawPairPointControl(identifier, pairPoint, sensitivity, availableWidth))
                    {
                        somethingChanged = true;
                        obj = pairPoint.dual();
                    }
                }
                else 
                {
                    // Display circle control
                    auto dualCircle = -obj.dual();
                    ImGui::SameLine();
                    if (DrawDualCircleControl(identifier, dualCircle, sensitivity, availableWidth)) 
                    {
                        somethingChanged = true;
                        obj = dualCircle.dual();
                    }
                }
                removeButtonOffset = ImVec2(0.0f, -9.0f);
                break;
            }

            case c3ga::MvecType::PairPoint:
            case c3ga::MvecType::ImaginaryPairPoint: {
                if (preferDual)
                {
                    // Display dual circle control
                    ImGui::SameLine();
                    somethingChanged |= DrawDualCircleControl(identifier, obj, sensitivity, availableWidth);
                }
                else
                {
                    // Display pair point control
                    ImGui::SameLine();
                    somethingChanged |= DrawPairPointControl(identifier, obj, sensitivity, availableWidth);
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
            objects.erase(objects.begin() + index);
            somethingChanged = true;
        } else {
            ++index;
        }
        ImGui::PopStyleVar();
    }
    ImGui::EndDisabled();

    if (isExplicit)
    {
        ImGui::Spacing();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 1));
        if (ImGui::Button("+"))
        {
            objects.push_back(c3ga::point(0.0, 0.0, 0.0));
            somethingChanged = true;
        }
        ImGui::PopStyleVar();
    }

    if (somethingChanged) {
        layer->SetDirty(DirtyBits_Provider);
    }

    return somethingChanged;
}

// == Provider ==

bool DrawProvider(const LayerPtr& layer, const LayerStackPtr& layerStack, const DualMode& dualMode) 
{
    const auto& layers = layerStack->GetLayers();
    auto sources = layer->GetSources();

    bool somethingChanged = false;

    // Provider
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Provider :");
    ImGui::SameLine();
    somethingChanged |= DrawProviderComboBox(layer);

    // Sources
    bool sourcesChanged = false;
    auto provider = layer->GetProvider();
    if (provider)
    {
        switch (provider->GetType())
        {
            case ProviderType_Explicit: {
                break;
            }

            case ProviderType_RandomGenerator: {
                somethingChanged |= DrawRandomGenerator(layer, dualMode);
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
                layerStack->DisconnectLayers(src.lock(), layer);

            for (const auto& src : sources) {
                layerStack->ConnectLayers(src.lock(), layer);
            }
        }
    }

    somethingChanged |= sourcesChanged;

    return somethingChanged;
}

    
void DrawLockButton(const char* identifier, bool& locked)
{
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
    if (ImGui::CheckedIconButton(identifier, ICON_LOCK, ICON_UNLOCK, locked, IconSize::Small))
        locked = !locked;
}

bool ContentEditor::Draw()
{
    bool somethingChanged = false;
    std::string id = std::to_string(m_id);
    ImGui::Begin((std::string("Content Editor##") + id).c_str(), &m_opened);
    
    if (!m_layer)
    {
        ImVec2 textSize = ImVec2(ImGui::CalcTextSize(kEmptySelectionMessage));
        ImGui::SetCursorPos((ImGui::GetContentRegionAvail() - textSize) / 2.0f);
        ImGui::Text(kEmptySelectionMessage);
        m_locked = false;
    }
    else
    {
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Layer : ");
        ImGui::SameLine();
        std::string layerName = m_layer->GetName();
        if (ImGui::InputText((std::string("##ContentEditorRenameLayer") + id).c_str(), &layerName))
            m_layer->SetName(layerName);

        ImGui::SameLine();
        DrawLockButton((std::string("##ContentEditorLock") + id).c_str(), m_locked);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        somethingChanged |= DrawProvider(m_layer, m_layerStack, m_dualMode);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        somethingChanged |= DrawLayerContent(m_layer, m_dualMode);
    }

    m_hovered = ImGui::IsWindowHovered();

    ImGui::End();

    return somethingChanged;
}
