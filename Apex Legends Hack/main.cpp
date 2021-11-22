#include <Windows.h>
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <string>
#include "Vec3.h"
#include <CasualLibrary.hpp>
#include <vector>
#include <thread>
#include "offsets.h"
#include "EspDrawer.h"
#include "Aimbot.h"

class Settings
{
public:
    bool  m_bBoxEsp     = false;
    bool  m_bAimBot     = false;
    bool  m_bLineEsp    = false;
    float m_fAimBotFov  = 10;
    char  custom_name[64] = {0};
    ImColor m_cLineEspColor = ImColor(255, 255, 255);
    ImColor m_cBoxEspColor = ImColor(255, 255, 255);
};

void inline TextCentered(std::string text) {
    auto windowWidth = ImGui::GetWindowSize().x;
    auto textWidth = ImGui::CalcTextSize(text.c_str()).x;
    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::Text(text.c_str());
}

void EntityListUpdateThread(std::vector<uintptr_t>& list, Memory::External& program)
{
    uintptr_t apex_module = program.getModule("r5apex.exe").get();
    while (true)
    {
        std::vector<uintptr_t> temp_vector;

        for (unsigned int i = 0; i < 15000; ++i)
        {
            auto ent = program.read<uintptr_t>(apex_module + offsets::enity_list + i * 0x30);
            if (!ent or program.read<uintptr_t>(ent) != apex_module + 0x15AE338)
                continue;
            temp_vector.push_back(ent);
        }
        list = temp_vector;
        Sleep(500);
    }
}
void AimbotThread(std::vector<uintptr_t>* entityList, Memory::External* program)
{

    while (true)
    {
        Aimbot aimBot = Aimbot(program, entityList);

        if (!GetAsyncKeyState(VK_LBUTTON))
        {
            Sleep(50);
            continue;
        }
        aimBot.Work();
        

    }
}
#ifdef _DEBUG
int main()
#else
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR    lpCmdLine, _In_ int       nCmdShow)
#endif // _DEBUG
{
    std::vector<uintptr_t> entity_list;
    bool draw = false;
    Settings settings;
    glfwInit();

    glfwWindowHint(GLFW_FLOATING,                true);
    glfwWindowHint(GLFW_RESIZABLE,               false);
    glfwWindowHint(GLFW_MAXIMIZED,               true);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, true);
    glfwWindowHint(GLFW_MOUSE_PASSTHROUGH,       true);

    Memory::External program        = Memory::External("r5apex.exe");
    uintptr_t        apex_module    = program.getModule("r5apex.exe").get();
    std::thread(EntityListUpdateThread, std::ref(entity_list), std::ref(program)).detach();
    std::thread(AimbotThread, &entity_list, &program).detach();

    auto video_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    GLFWwindow* window = glfwCreateWindow(video_mode->width, video_mode->height, "Overlay", NULL, NULL);

    glfwSetWindowAttrib(window, GLFW_DECORATED, false);

    // позовляет нормально растянуть окно на экран
    glfwSetWindowMonitor(window, NULL, 0, 0, video_mode->width, video_mode->height + 1, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    auto theme = ImGui::GetStyle().Colors;

    theme[ImGuiCol_WindowBg] = ImColor(8, 15, 32, 255);
    theme[ImGuiCol_Border]   = ImColor(27, 117, 255, 255);
    theme[ImGuiCol_Button]   = ImColor(2, 91, 255, 255);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    int tab = 0;
    EspDrawer espDrawer = EspDrawer(&program, nullptr);
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        if (GetAsyncKeyState(VK_DELETE) & 1)
        {
            draw = !draw;
            glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, !glfwGetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH));
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        auto draw_list = ImGui::GetBackgroundDrawList();
        espDrawer.UpdateDrawList(draw_list);

        for (auto ent : entity_list)
        {
            if (program.read<int>(ent + offsets::m_iHealth) <= 0 or !espDrawer.IsEntityOnScreen(ent))
                continue;

            if (settings.m_bLineEsp)
            {
                espDrawer.DrawLineESP(ent, settings.m_cLineEspColor);
            }
            if (settings.m_bBoxEsp)
            {
                espDrawer.DrawBoxESP(ent, settings.m_cBoxEspColor);
            }

        }
        if (!draw)
        {
            ImGui::Render();
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
            continue;
        }

        draw_list->AddRectFilled(ImVec2(), ImVec2(video_mode->width, video_mode->height), ImColor(0, 0, 0, 90));
       

        ImGui::Begin("X", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration);
        {
            ImGui::SetWindowSize(ImVec2(500, 500));
            TextCentered("Settings");

            ImGui::BeginChild("Test", ImVec2(0, 50), true, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            {
                if (ImGui::Button("General"))
                    tab = 1;

                ImGui::SameLine();
                if (ImGui::Button("Aim Bot"))
                    tab = 2;
                ImGui::SameLine();
                if (ImGui::Button("Trigger Bot"))
                    tab = 3;
                ImGui::SameLine();
                if (ImGui::Button("Visuals"))
                    tab = 4;
                ImGui::SameLine();
                if (ImGui::Button("Misc"))
                    tab = 5;
                ImGui::SameLine();

                if (ImGui::Button("About"))
                    tab = 6;
                switch (tab)
                {
                case 1:
                    ImGui::Text("General Options");
                    break;
                case 2:
                    ImGui::Text("Automatic Target Acquisition System");
                    break;
                case 3:
                    ImGui::Text("Automatic Target Shooting");
                    break;
                case 4:
                    ImGui::Text("Exstra Sensory Perception");
                    break;
                case 5:
                    ImGui::Text("Miscelenious Settings");
                    break;
                case 6:
                    ImGui::Text("About");
                    break;
                }
                ImGui::EndChild();
            }
            switch (tab)
            {
            case 1:
                break;
            case 4:
                ImGui::ColorEdit3("ColorL", (float*)&settings.m_cBoxEspColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                ImGui::SameLine();
                ImGui::Checkbox("Box  ESP", &settings.m_bBoxEsp);
                ImGui::ColorEdit3("ColorB", (float*)&settings.m_cLineEspColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                ImGui::SameLine();
                ImGui::Checkbox("Line ESP", &settings.m_bLineEsp);
                break;
            case 5:
                ImGui::Text("Name spoofer");
                ImGui::SameLine();
                ImGui::InputText("\0", settings.custom_name, 64);
                ImGui::SameLine();
                if (ImGui::Button("Change"))
                {
                    uintptr_t nameAddr = apex_module + offsets::sLocalPlayerName;

                    for (auto chr : settings.custom_name)
                    {

                        program.write<char>(nameAddr, chr);
                        nameAddr++;
                   }
                }
                break;
            case 6:
                ImGui::Text("Registered to: \"%s\"", "G935");
                ImGui::Text("Build Date: %s, %s", __DATE__, __TIME__);
                break;
            }
            ImGui::End();
        }

        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
