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

class viewmatrix
{
public:
    float* operator[](int index)
    {
        return this->data[index];
    }

private:
    float data[4][4];
};

ImVec3 WorldToScreen(ImVec3 pos, viewmatrix dwViewmatrix)
{
    float _x = dwViewmatrix[0][0] * pos.x + dwViewmatrix[0][1] * pos.y + dwViewmatrix[0][2] * pos.z + dwViewmatrix[0][3];
    float _y = dwViewmatrix[1][0] * pos.x + dwViewmatrix[1][1] * pos.y + dwViewmatrix[1][2] * pos.z + dwViewmatrix[1][3];
    float _z = dwViewmatrix[2][0] * pos.x + dwViewmatrix[2][1] * pos.y + dwViewmatrix[2][2] * pos.z + dwViewmatrix[2][3];
    float  w = dwViewmatrix[3][0] * pos.x + dwViewmatrix[3][1] * pos.y + dwViewmatrix[3][2] * pos.z + dwViewmatrix[3][3];

    const ImVec2 window_size = ImVec2(1920, 1080);

    float x = (window_size.x / 2 * (_x / w)) + ((_x / w) + window_size.x / 2);

    float y = -(window_size.y / 2 * (_y / w)) + ((_y / w) + window_size.y / 2);

    return ImVec3(x, y, w);
}

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

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR    lpCmdLine, _In_ int       nCmdShow)
{
    std::vector<uintptr_t> entity_list;
    bool draw = false;

    glfwInit();

    glfwWindowHint(GLFW_FLOATING,                true);
    glfwWindowHint(GLFW_RESIZABLE,               false);
    glfwWindowHint(GLFW_MAXIMIZED,               true);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, true);
    glfwWindowHint(GLFW_MOUSE_PASSTHROUGH,       true);

    Memory::External program        = Memory::External("r5apex.exe");
    uintptr_t        apex_module    = program.getModule("r5apex.exe").get();
    
    std::thread(EntityListUpdateThread, std::ref(entity_list), std::ref(program)).detach();


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

        for (auto ent : entity_list)
        {
            auto top    = WorldToScreen(program.read<ImVec3>(ent + offsets::m_vecOrigin), program.read<viewmatrix>(apex_module + offsets::view_matrix));
            auto bottom = WorldToScreen(program.read<ImVec3>(ent + offsets::m_vecCameraPos) + ImVec3(0,0,10), program.read<viewmatrix>(apex_module + offsets::view_matrix));

            if (top.z < 0.f or program.read<int>(ent + offsets::m_iHealth) <= 0)
                continue;
            float height = fabsf(top.y - bottom.y);

            ImVec3 top_left     = bottom -  ImVec3(height / 4.f, 0, 0);
            ImVec3 top_right    = bottom + ImVec3(height / 4.f, 0, 0);
            ImVec3 bottom_right = top    + ImVec3(height / 4.f, 0, 0);
            ImVec3 bottom_left  = top    - ImVec3(height / 4.f, 0, 0);
            bottom_left.x = top_left.x;
            bottom_right.x = top_right.x;
            draw_list->AddLine(top_left,    top_right,    ImColor(255, 255, 255), 2);
            draw_list->AddLine(top_left,    bottom_left,  ImColor(255, 255, 255), 2);
            draw_list->AddLine(top_right,   bottom_right, ImColor(255, 255, 255), 2);
            draw_list->AddLine(bottom_left, bottom_right, ImColor(255, 255, 255), 2);

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
                    ImGui::Text("General options");
                    break;
                case 2:
                    ImGui::Text("Automatic target acquisition system");
                    break;
                case 3:
                    ImGui::Text("Automatic target shooting");
                    break;
                case 4:
                    ImGui::Text("Exstra sensory perception");
                    break;
                case 5:
                    ImGui::Text("Miscelenious settings");
                    break;
                case 6:
                    ImGui::Text("About program");
                    break;
                }
                ImGui::EndChild();
            }
            bool x;
            switch (tab)
            {
            case 1:
                ImGui::Checkbox("AimBot", &x);
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
