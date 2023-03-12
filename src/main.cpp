#include "Simulation.hpp"
#include "LayerStackWidget.hpp"

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

    LayerStackPtr stack = std::make_shared<LayerStack>();

    // Generate the content of the scene
    MvecArray objects = {c3ga::dualSphere<double>(0, 0, 0, 1).dual()};
    auto lyr1 = stack->NewLayer("Layer1", objects);
    objects = {c3ga::dualSphere<double>(0, 1, 0, 1).dual()};
    auto lyr2 = stack->NewLayer("Layer2", objects);
    auto lyr3 = stack->NewCombination("Layer3", lyr1, lyr2, Operators::OuterProduct);

    Renderer renderer;

    LayerStackWidget layerStackWid(stack);

    double prevTime = window.GetTime();
    double currTime;
    bool somethingChanged = false;
    while (!window.ShouldClose()) {
        currTime = window.GetTime();

        // Update all the visible layers
        for (const auto& layer : stack->GetLayers())
        {
            if (layer->IsVisible()) {
                somethingChanged |= layer->Update();
            }
        }

        camera.Update();

        glClearColor(0.2f, 0.25f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (somethingChanged) {
            renderer.Invalidate();
        }
        renderer.Render(stack, camera.GetViewProjectionMatrix());

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
            if (ImGui::BeginMenu("File"))
            {
                if(ImGui::MenuItem("Clear context##MainMenuClearAll")) 
                {
                    layerStackWid.Clear();
                    somethingChanged = true;
                }

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        ImGui::End(); // Main Dockspace
        // ImGui::ShowDemoWindow();
        somethingChanged |= layerStackWid.Draw();

        // // Render ImGui items
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    	
        window.Update();
        prevTime = currTime;
    }

    return 0;
}
