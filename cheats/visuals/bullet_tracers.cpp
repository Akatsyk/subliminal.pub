#include "bullet_tracers.h"

#include "..\..\sdk\misc\BeamInfo_t.hpp"

void bullettracers::draw_beam( ) {
	if (!g_ctx.available() || !g_ctx.m_local || !g_cfg.esp.bullet_tracer)
	{
		impacts.clear();
		return;
	}

	if (impacts.size() > 30)
		impacts.pop_back();

	for (size_t i = 0; i < impacts.size(); i++)
	{
		auto impact = impacts[i];

		BeamInfo_t beam_info;
		beam_info.m_vecStart = impact.start;
	
		beam_info.m_vecStart.z -= 2.f;
		beam_info.m_vecEnd = impact.position;
		beam_info.m_nType = TE_BEAMPOINTS;
		beam_info.m_pszModelName = "sprites/purplelaser1.vmt";
		beam_info.m_nModelIndex = -1;
		beam_info.m_flHaloScale = 0.0f;
		beam_info.m_flLife = 4.f;
		beam_info.m_flWidth = 2.0f;
		beam_info.m_flEndWidth = 2.0f;
		beam_info.m_flFadeLength = 0.0f;
		beam_info.m_flAmplitude = 2.0f;
		beam_info.m_flBrightness = 255.f;
		beam_info.m_flSpeed = 0.2f;
		beam_info.m_nStartFrame = 0;
		beam_info.m_flFrameRate = 0.f;
		beam_info.m_flRed = float(impact.color.r());
		beam_info.m_flGreen = float(impact.color.g());
		beam_info.m_flBlue = float(impact.color.b());
		beam_info.m_nSegments = 2;
		beam_info.m_bRenderable = true;
		beam_info.m_nFlags = FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;

		Beam_t* beam = g_csgo.m_viewrenderbeams()->CreateBeamPoints(beam_info);
		if (beam)
			g_csgo.m_viewrenderbeams()->DrawBeam(beam);
	}

	impacts.clear();
}

void bullettracers::events(IGameEvent* game_event)
{
	const auto userid = g_csgo.m_engine()->GetPlayerForUserID(game_event->GetInt("userid"));

	if (userid != g_ctx.m_local->EntIndex())
		return;

	static float last_curtime;

	
	const Vector pos(game_event->GetFloat("x"), game_event->GetFloat("y"), game_event->GetFloat("z"));
	Color col;

	trace_info impact(g_ctx.m_local->get_eye_pos(), pos, g_csgo.m_globals()->m_curtime, g_cfg.esp.bullet_tracer_color);

	if (last_curtime == g_csgo.m_globals()->m_curtime)
	{
		if (!impacts.empty() && impact.time < (impacts.end() - 1)->time || impacts.empty())
			return;

		impacts.pop_back();
	}

	last_curtime = g_csgo.m_globals()->m_curtime;

	impacts.push_back(impact);
}

bool bullettracers::beam_exists(const float curtime)
{
	for (auto& impact : impacts)
	{
		if ( impact.time == curtime )
			return true;
	}

	return false;
}








































