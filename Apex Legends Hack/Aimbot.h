#pragma once
#include <CasualLibrary.hpp>
#include "Vec3.h"
#include "offsets.h"
#include <vector>

class Aimbot
{
private:
    Memory::External*       m_pProgram;
    std::vector<uintptr_t>* m_entiyList;
    uintptr_t               m_iApexModule;

public:
    Aimbot(Memory::External* program, std::vector<uintptr_t>* entiyList)
    {
        this->m_pProgram    = program;
        this->m_entiyList   = entiyList;
        this->m_iApexModule = program->getModule("r5apex.exe").get();
    
    }
    void Work()
    {

        for (unsigned char i = 0; i < this->m_entiyList->size(); ++i)
        {
            auto ent = this->m_entiyList->at(i);
            ImVec3 entityPosition = this->m_pProgram->read<ImVec3>(ent + offsets::m_vecCameraPos);

            if (this->IfPositionInFov(this->ClacAimAngles(entityPosition), 30.f) and this->m_pProgram->read<int>(ent + offsets::m_iHealth) > 0)
                this->AimAt(entityPosition, 30);
        }
    }
private:

    void AimAt(const ImVec3& targetPosition, float fov)
    {
        ImVec3 outAngles = this->ClacAimAngles(targetPosition);

        if ( (fabsf(outAngles.x) < 89.f and outAngles.y < 180.f) and this->IfPositionInFov(outAngles, fov))
            this->m_pProgram->write<ImVec3>(0x1D3332198, outAngles);

    }
    __forceinline bool IfPositionInFov(const ImVec3& angles, float fov)
    {

        ImVec2  fov_target = this->m_pProgram->read<ImVec3>(0x1D3332198) - angles;

        return (fov_target.x <= fov and fov_target.y <= fov and fov_target.x >= -fov and fov_target.y >= -fov);
    
    }
    __forceinline ImVec3 ClacAimAngles(const ImVec3& targetPosition)
    {
        ImVec3 localPlayerCameraPosition = this->m_pProgram->read<ImVec3>(this->m_iApexModule + offsets::vecLocalpPlayerCameraPos);
        ImVec3 outAngles;

        outAngles.x = -asinf((targetPosition.z - localPlayerCameraPosition.z) / localPlayerCameraPosition.DistTo(targetPosition)) * (180 / 3.1415926f);
        outAngles.y = atan2f(targetPosition.y - localPlayerCameraPosition.y, targetPosition.x - localPlayerCameraPosition.x) * (180 / 3.1415926f);

        return outAngles;
    }
   
};

