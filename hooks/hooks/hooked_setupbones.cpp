#include "..\hooks.hpp"

bool __fastcall hooks::hooked_setupbones( void * ecx, void * edx, matrix3x4_t *bone_world_out, int max_bones, int bone_mask, float current_time ) 
{
	return original_setupbones( ecx, bone_world_out, max_bones, bone_mask, current_time );
}

using IsHLTV_t = bool(__thiscall*)(void*);

bool __fastcall hooks::hooked_isHLTV()
{
	static auto SetupVelocity = reinterpret_cast<uintptr_t*>(util::pattern_scan("client.dll", "84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80"));

	// AccumulateLayers
	if (g_ctx.m_globals.m_ishltv_fix)
		return true;

	// fix for animstate velocity.
	if (SetupVelocity && _ReturnAddress() == SetupVelocity)
		return true;

	return engine_hook->get_func_address< IsHLTV_t >(93);
}


//void _fastcall hooks::do_extra_bone_processing(void* ecx, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, byte* boneComputed, CIKContext* context)
//{
//	if (g_pLocalPlayer && ecx == g_pLocalPlayer)
//		return;
//
//	const auto player = reinterpret_cast<C_CSPlayer*>(ecx);
//	if (player)
//		return;
//
//	orig_do_extra_bone_processing(ecx, hdr, pos, q, matrix, boneComputed, context);
//}





































