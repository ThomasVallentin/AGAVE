#include "Simulation.hpp"
#include "Samples.hpp"

#include "UI/LayerStackWidget.hpp"
#include "UI/Icons.hpp"

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

        camera.OnEvent(event);
    };
    window.SetEventCallback(eventCallback);

    // Initialize the global "managers" 
    SimulationEngine& simEngine = SimulationEngine::Init();

    LayerStackPtr layerStack = std::make_shared<LayerStack>();
    Samples::LoadCompleteExample(layerStack);

    Renderer renderer;
    RenderSettings& renderSettings = renderer.GetRenderSettings();

    // Generate the content of the scene
    LayerStackWidget layerStackWid(layerStack);

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
        renderer.Render(layerStack, camera.GetViewProjectionMatrix());

        // Reset the invalidation variable 
        somethingChanged = false;

        // // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

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
                ImGui::SeparatorText("Display Mode");
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, -2));
                ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(8, 8));
                if (ImGui::RadioButton("Default##DisplayMenuDefault", 
                                       renderSettings.displayMode == RenderSettings::DisplayMode::Default)){
                    renderSettings.displayMode = RenderSettings::DisplayMode::Default;
                    somethingChanged = true;
                }
                if (ImGui::RadioButton("Dual##DisplayMenuDual",       
                                       renderSettings.displayMode == RenderSettings::DisplayMode::Dual)){
                    renderSettings.displayMode = RenderSettings::DisplayMode::Dual;
                    somethingChanged = true;
                }
                if (ImGui::RadioButton("Both##DisplayMenuBoth",       
                                       renderSettings.displayMode == RenderSettings::DisplayMode::Both)){
                    renderSettings.displayMode = RenderSettings::DisplayMode::Both;
                    somethingChanged = true;
                }
                ImGui::PopStyleVar(3);

                ImGui::Spacing();

                ImGui::EndMenu();
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleColor();

            ImGui::EndMainMenuBar();
        }

        ImGui::End(); // Main Dockspace
        ImGui::ShowDemoWindow();
        somethingChanged |= layerStackWid.Draw();

        // // Render ImGui items
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    	
        window.Update();
        prevTime = currTime;
    }

    return 0;
}
