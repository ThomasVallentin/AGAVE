#include "Simulation.hpp"
#include "Layer.hpp"

#include "Renderer/Shader.h"
#include "Renderer/VertexArray.h"

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
    
    MvecArray objects;
    for (int i = 0 ; i < 10 ; ++i)
    {
        objects.push_back(c3ga::randomPoint<float>());
    }
    c3ga::Mvec<float> sphere = c3ga::randomPoint<float>() ^ c3ga::randomPoint<float>() ^ c3ga::randomPoint<float>() ^ c3ga::randomPoint<float>();
    LOG_INFO("%s", c3ga::whoAmI(sphere).c_str());

    LayerPtr lyr1 = std::make_shared<Layer>(objects);
    LinkPtr sub1 = std::make_shared<Subset>(lyr1, 4, 4, Link::OuterOp);
    LayerPtr lyr2 = std::make_shared<Layer>();
    sub1->Update(lyr2);

    LOG_INFO("Layer 1 : %d", lyr1->Get().size());
    for (const auto& obj : *lyr1) {
        LOG_INFO("%s", c3ga::whoAmI(obj).c_str());
        obj.display();
    }

    LOG_INFO("Layer 2 %d", lyr2->Get().size());
    for (const auto& obj : *lyr2) {
        LOG_INFO("%s", c3ga::whoAmI(obj).c_str());
        obj.display();
    }

    // Combination comb1;













    return 0;

    // ShaderPtr shader = Shader::Open(resolver.Resolve("resources/shaders/particles.vert"),
    //                                 resolver.Resolve("resources/shaders/particles.frag"));
    // shader->Bind();
    
    // Camera camera(50.0f, 1280.0f / 720.0f, 0.1f, 10000.0f);

    // auto eventCallback = [&](Event* event) {
    //     switch (event->GetType()) 
    //     {
    //         case EventType::WindowResized: {
    //             auto resizeEvent = dynamic_cast<WindowResizedEvent*>(event);
    //             glViewport(0, 0, resizeEvent->GetWidth(), resizeEvent->GetHeight());
    //             camera.SetViewportSize(resizeEvent->GetWidth(), resizeEvent->GetHeight());
    //             break;
    //         }
    //     }

    //     camera.OnEvent(event);
    // };
    // window.SetEventCallback(eventCallback);

    // // Fill the scene with points
    // SimulationEngine engine;
    // auto& objects = engine.GetObjects();
    // for (int i = 0 ; i < 100 ; ++i)
    // {
    //     objects.push_back({c3ga::randomPoint<float>(), c3ga::randomPoint<float>()});
    // }

    // std::vector<glm::vec3> points(100);
    // for (int i = 0 ; i < points.size() ; ++i) {
    //     const auto& object = objects[i];
    //     points[i] = {object.position[c3ga::E1], object.position[c3ga::E2], object.position[c3ga::E3]};
    // }
    // VertexBufferPtr vbo = VertexBuffer::Create(points.data(), 
    //                                            points.size() * sizeof(glm::vec3));
    // VertexBufferLayout layout = {{"Position", 3, GL_FLOAT, false}};
    // vbo->SetLayout(layout);

    // VertexArrayPtr vao = VertexArray::Create();
    // vao->Bind();
    // vao->AddVertexBuffer(vbo);
    // // vao->SetIndexBuffer(ebo);
    // vao->Unbind();

    // double prevTime = window.GetTime();
    // double currTime;
    // while (!window.ShouldClose()) {
    //     currTime = window.GetTime();

    //     engine.Update(currTime - prevTime);

 	//     vbo->Bind();
    //     for (int i = 0 ; i < points.size() ; ++i) {
    //         const auto& object = objects[i];
    //         points[i] = {object.position[c3ga::E1], object.position[c3ga::E2], object.position[c3ga::E3]};
    //     }
    //     vbo->SetData(points.data(), points.size() * sizeof(glm::vec3));

    //     shader->Bind();
    //     shader->SetMat4("uViewProjMatrix", camera.GetViewProjectionMatrix());
    //     vao->Bind();

    //     glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    //     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //     glDrawArrays(GL_POINTS, 0, points.size());

    //     // // Start the Dear ImGui frame
    //     ImGui_ImplOpenGL3_NewFrame();
    //     ImGui_ImplGlfw_NewFrame();
    //     ImGui::NewFrame();

    //     if (ImGui::IsWindowFocused() || ImGui::IsWindowHovered()) {
    //         camera.Update();
    //     }

    //     ImGui::Begin("Control panel", nullptr);
    //     {
    //         if (ImGui::CollapsingHeader("Environment parameters", ImGuiTreeNodeFlags_DefaultOpen))
    //         {
    //             // Gravity
    //             float gravity = 0.0;
    //             indentedLabel("Gravity");
    //             ImGui::SameLine();
    //             ImGui::SliderFloat("##GravitySlider", &gravity, 0.0f, 50.0);
    //         }
    //     }

    //     ImGui::End();

    //     // // Render ImGui items
    //     ImGui::Render();
    //     ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    	
    //     window.Update();
    //     prevTime = currTime;
    // }

    // return 0;
}
