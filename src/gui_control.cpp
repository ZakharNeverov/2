#include "gui_control.h"
#include "camera_control.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glut.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <chrono>


static int fps = 0;
static int frameCount = 0;
static std::chrono::time_point<std::chrono::steady_clock> lastTime;

void initGUI()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGLUT_Init();
    ImGui_ImplOpenGL3_Init("#version 330");  // Укажите версию GLSL
}

void shutdownGUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();
}


void renderGUI()
{
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLUT_NewFrame();
    ImGui::NewFrame();  

    
    ImGui::SetNextWindowSize(ImVec2(300, 200)); 
    ImGui::SetNextWindowPos(ImVec2(10, 10));    

    
    ImGui::Begin("Light Parameters");

    extern float lightPosition[4];
    extern float lightIntensity;
    extern float lightBaseColor[3];
    glm::vec3 CameraPosition = getCameraPosition();

    ImGui::Text("Light Position");
    ImGui::SliderFloat3("Position", lightPosition, -10.0f, 10.0f);

    ImGui::Text("Light Intensity");
    ImGui::SliderFloat("Intensity", &lightIntensity, 0.0f, 1.0f);

    ImGui::Text("Light Color");
    ImGui::ColorEdit3("Base Color", lightBaseColor);
    
    ImGui::Separator();
    ImGui::Text("Camera Position:\n %.2fx %.2fy %.2fz",CameraPosition.x,CameraPosition.y,CameraPosition.z);


    ImGui::Separator();
    ImGui::Text("FPS: %d", fps);

    ImGui::End();

    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    updateFPS();
}

void updateFPS()
{
    frameCount++;
    auto currentTime = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();

    if (deltaTime >= 16.0f)
    {
        fps = frameCount;
        frameCount = 0;
        lastTime = currentTime;
    }
}
