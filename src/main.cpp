#include "Simulation.hpp"
#include "Samples.hpp"

#include "UI/LayerStackWidget.hpp"
#include "UI/ContentEditor.hpp"

#include "Renderer/Shader.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Renderer.hpp"

#include "Base/Camera.h"
#include "Base/Window.h"
#include "Base/Event.h"
#include "Base/Resolver.h"

#include <glad/glad.h>
#include <glm/gtx/string_cast.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include <c3ga/Mvec.hpp>
#include <c3gaTools.hpp>


int main(int argc, char* argv[])
{
    auto& resolver = Resolver::Init(fs::weakly_canonical(argv[0])
                                    .parent_path()
                                    .parent_path());

    auto window = Window({1280, 720, "Particle System"});
    
    Camera camera(50.0f, 1280.0f / 720.0f, 0.1f, 10000.0f);
    bool anyWindowHovered = false;

    auto eventCallback = [&](Event* event) {
        switch (event->GetType()) 
        {
            case EventType::WindowResized: {
                auto resizeEvent = dynamic_cast<WindowResizedEvent*>(event);
                glViewport(0, 0, resizeEvent->GetWidth(), resizeEvent->GetHeight());
                camera.SetViewportSize(resizeEvent->GetWidth(), resizeEvent->GetHeight());
                break;
            }
        }

        if (!anyWindowHovered)
            camera.OnEvent(event);
    };
    window.SetEventCallback(eventCallback);

    // Initialize the global "managers" 
    SimulationEngine& simEngine = SimulationEngine::Init();

    LayerStackPtr layerStack = std::make_shared<LayerStack>();
    Samples::LoadCompleteExample(layerStack);

    Renderer renderer;
    RenderSettings& renderSettings = renderer.GetRenderSettings();

    // Initialize the UI widgets
    LayerStackWidget layerStackWid(layerStack);

    std::vector<ContentEditor> contentEditors = {ContentEditor(layerStack)};

    double prevTime = window.GetTime();
    double currTime, deltaTime;
    bool somethingChanged = false;
    while (!window.ShouldClose()) {
        currTime = window.GetTime();
        deltaTime = currTime - prevTime;

        camera.Update();
        simEngine.Update(deltaTime);

        // Set dirty all the animated layers
        for (const auto& layer : layerStack->GetLayers())
            if (auto provider = std::dynamic_pointer_cast<Explicit>(layer->GetProvider()))
                if (provider->IsAnimated())
                    layer->SetDirty(DirtyBits_Provider);

        // Update all the visible layers
        for (const auto& layer : layerStack->GetLayers())
            if (layer->IsVisible()) 
                somethingChanged |= layer->Update();

        glClearColor(0.2f, 0.25f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (somethingChanged) {
            renderer.Invalidate();
        }
        renderer.Render(layerStack->GetLayers(), camera.GetViewProjectionMatrix());

        // Reset the invalidation variable 
        somethingChanged = false;

        // // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        anyWindowHovered = false;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        windowFlags |= ImGuiWindowFlags_NoTitleBar |
                       ImGuiWindowFlags_NoCollapse |
                       ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoMove |
                       ImGuiWindowFlags_NoBringToFrontOnFocus |
                       ImGuiWindowFlags_NoNavFocus | 
                       ImGuiWindowFlags_NoBackground;

        ImGui::Begin("MainDockSpace", nullptr, windowFlags);

        // Pop the rounding, border & padding style vars set previously
        ImGui::PopStyleVar(3);

        // Submit the DockSpace
        ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
        static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspaceFlags);

        if (ImGui::BeginMainMenuBar())
        {
            ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 6));
            if (ImGui::BeginMenu("File"))
            {
                if(ImGui::MenuItem("  New##FileMenuNew")) 
                {
                    layerStackWid.Clear();
                    somethingChanged = true;
                }

                ImGui::MenuItem("  Open##FileMenuOpen", "", nullptr, false);

                if(ImGui::BeginMenu("  Open sample##FileMenuOpenSamples")) 
                {
                    if(ImGui::MenuItem("  Spheres intersection##FileMenuLoadSpheresIntersect")) 
                        Samples::LoadSpheresIntersect(layerStack);
                    if(ImGui::MenuItem("  Planes intersection##FileMenuLoadPlanesIntersect")) 
                        Samples::LoadPlanesIntersect(layerStack);
                    if(ImGui::MenuItem("  Complete example##FileMenuLoadCompleteExample")) 
                        Samples::LoadCompleteExample(layerStack);

                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Display"))
            {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 8));
                ImGui::SeparatorText("Dual Mode");
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, -2));
                ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(8, 8));
                if (ImGui::RadioButton("Default##DisplayMenuDefault", 
                                       renderSettings.dualMode == DualMode_Default)){
                    renderSettings.dualMode = DualMode_Default;
                    somethingChanged = true;
                }
                if (ImGui::RadioButton("Dual##DisplayMenuDual",       
                                       renderSettings.dualMode == DualMode_Dual)){
                    renderSettings.dualMode = DualMode_Dual;
                    somethingChanged = true;
                }
                if (ImGui::RadioButton("Both##DisplayMenuBoth",       
                                       renderSettings.dualMode == DualMode_Both)){
                    renderSettings.dualMode = DualMode_Both;
                    somethingChanged = true;
                }
                ImGui::PopStyleVar(3);

                ImGui::Spacing();

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Editors"))
            {
                if (ImGui::MenuItem("New Content Editor"))
                    contentEditors.emplace_back(layerStack);

                ImGui::EndMenu();
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleColor();

            ImGui::EndMainMenuBar();
        }

        ImGui::End(); // Main Dockspace
        // ImGui::ShowDemoWindow();

        // Layer stack widget
        somethingChanged |= layerStackWid.Draw();
        anyWindowHovered |= layerStackWid.IsHovered();

        // Content editors
        for (auto it = contentEditors.begin() ; it != contentEditors.end() ;)
        {
            auto& editor = *it;
            if (editor.ShouldClose())
            {
                contentEditors.erase(it);
                continue;
            }

            editor.SetDualMode(renderSettings.dualMode);
            if (!editor.IsLocked())
                editor.SetCurrentLayer(layerStackWid.GetLastSelectedLayer());

            somethingChanged |= editor.Draw();
            anyWindowHovered |= editor.IsHovered();

            ++it;
        }
        
        anyWindowHovered |= ImGui::IsAnyItemHovered();

        // // Render ImGui items
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    	
        window.Update();
        prevTime = currTime;
    }

    return 0;
}
