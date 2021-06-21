#pragma once

#include "../misc/Studio.hpp"
#include "IMaterialSystem.hpp"
#include "IEngineTrace.hpp" //Has some structs we need here

class CPhysCollide;
class CUtlBuffer;
class IClientRenderable;
class CStudioHdr;
struct virtualmodel_t;

class CBoneCache
{
public:
    matrix3x4_t* m_pCachedBones;
    char pad[0x8];
    unsigned int m_CachedBoneCount;
};

enum RenderableTranslucencyType_t
{
    RENDERABLE_IS_OPAQUE = 0,
    RENDERABLE_IS_TRANSLUCENT,
    RENDERABLE_IS_TWO_PASS,    // has both translucent and opaque sub-partsa
};

class IVModelInfo {
public:
    // indexes for virtuals and hooks.
    enum indices : size_t {
        GETMODEL = 1,
        GETMODELINDEX = 2,
        GETMODELFRAMECOUNT = 8,
        GETSTUDIOMODEL = 30,
        FINDORLOADMODEL = 43
    };

public:
    __forceinline const model_t* GetModel(int modelindex) {
        return call_virtual< const model_t* (__thiscall*)(void*, int) >(this, GETMODEL)(this, modelindex);
    }

    __forceinline int GetModelIndex(const char* model) {
        return call_virtual< int(__thiscall*)(void*, const char*) >(this, GETMODELINDEX)(this, model);
    }

    __forceinline int GetModelFrameCount(const model_t* model) {
        return call_virtual< int(__thiscall*)(void*, const model_t*) >(this, GETMODELFRAMECOUNT)(this, model);
    }

    __forceinline studiohdr_t* GetStudiomodel(const model_t* model) {
        return call_virtual< studiohdr_t* (__thiscall*)(void*, const model_t*) >(this, GETSTUDIOMODEL)(this, model);
    }

    __forceinline const model_t* FindOrLoadModel(const char* name) {
        return call_virtual< const model_t* (__thiscall*)(void*, const char*) >(this, FINDORLOADMODEL)(this, name);
    }
};