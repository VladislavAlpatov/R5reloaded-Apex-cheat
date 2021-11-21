#pragma once
#include <CasualLibrary.hpp>
#include "imgui/imgui.h"
#include "Vec3.h"
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


class EspDrawer
{
private:
	Memory::External* m_pProgram;
	ImDrawList*       m_pDrawList;
    uintptr_t         m_iApexModule;
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
public:
	EspDrawer(Memory::External* program, ImDrawList* drawList)
	{
		this->m_pProgram    =  program;
		this->m_pDrawList   = drawList;
        this->m_iApexModule = program->getModule("r5apex.exe").get();

	}
	void DrawBoxESP(uintptr_t entity, ImColor& color)
	{
        auto top    = this->WorldToScreen(this->m_pProgram->read<ImVec3>(entity + offsets::m_vecOrigin), this->m_pProgram->read<viewmatrix>(this->m_iApexModule + offsets::view_matrix));
        auto bottom = this->WorldToScreen(this->m_pProgram->read<ImVec3>(entity + offsets::m_vecCameraPos) + ImVec3(0, 0, 10), this->m_pProgram->read<viewmatrix>(this->m_iApexModule + offsets::view_matrix));

        float height = fabsf(top.y - bottom.y);

        ImVec3 top_left     = bottom  - ImVec3(height / 4.f, 0, 0);
        ImVec3 top_right    = bottom  + ImVec3(height / 4.f, 0, 0);
        ImVec3 bottom_right = top     + ImVec3(height / 4.f, 0, 0);
        ImVec3 bottom_left  = top     - ImVec3(height / 4.f, 0, 0);

        bottom_left.x = top_left.x;
        bottom_right.x = top_right.x;

        this->m_pDrawList->AddLine(top_left,    top_right,    color, 2);
        this->m_pDrawList->AddLine(top_left,    bottom_left,  color, 2);
        this->m_pDrawList->AddLine(top_right,   bottom_right, color, 2);
        this->m_pDrawList->AddLine(bottom_left, bottom_right, color, 2);
	}
    void DrawLineESP(uintptr_t entity, ImColor color)
    {
        auto origin = this->WorldToScreen(this->m_pProgram->read<ImVec3>(entity + offsets::m_vecOrigin), this->m_pProgram->read<viewmatrix>(this->m_iApexModule + offsets::view_matrix));

        this->m_pDrawList->AddLine(ImVec2(1920 / 2, 1080), origin, color, 2);
    }
    bool IsEntityOnScreen(uintptr_t entity)
    {
        return this->WorldToScreen(this->m_pProgram->read<ImVec3>(entity + offsets::m_vecOrigin), this->m_pProgram->read<viewmatrix>(this->m_iApexModule + offsets::view_matrix)).z > 0.f;
    }
    void UpdateDrawList(ImDrawList* drawList)
    {
        this->m_pDrawList = drawList;
    }
};

