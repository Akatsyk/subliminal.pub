#include "fakewalk.h"
#include "..\ragebot\antiaim.h"
#include "..\sdk\animation_state.h"

void fakewalk::create_move() { 
	vec3_t velocity{ g_ctx.m_local->m_vecVelocity() };
	int    ticks{ }, max{ 16 };

	if (!g_cfg.misc.fakewalk || !g_csgo.m_inputsys()->IsButtonDown(g_cfg.misc.fakewalk_key))
		return;

	if (!(g_ctx.m_local->m_fFlags() & FL_ONGROUND))
		return;

	// user was running previously and abrubtly held the fakewalk key
	// we should quick-stop under this circumstance to hit the 0.22 flick
	// perfectly, and speed up our fakewalk after running even more.
	//if( g_cl.m_initial_flick ) {
	//	Movement::QuickStop( );
	//	return;
	//}

	// reference:
	// https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/game/shared/gamemovement.cpp#L1612

	// calculate friction.
	float friction = g_csgo.m_cvar()->FindVar("sv_friction")->GetFloat() * g_ctx.m_local->m_surfaceFriction();

	for (; ticks < 14; ++ticks) {
		// calculate speed.
		float speed = velocity.Length();

		// if too slow return.
		if (speed <= 0.1f)
			break;

		// bleed off some speed, but if we have less than the bleed, threshold, bleed the threshold amount.
		float control = max(speed, g_csgo.m_cvar()->FindVar("sv_stopspeed")->GetFloat());

		// calculate the drop amount.
		float drop = control * friction * g_csgo.m_globals()->m_interval_per_tick;

		// scale the velocity.
		float newspeed = max(0.f, speed - drop);

		if (newspeed != speed) {
			// determine proportion of old speed we are using.
			newspeed /= speed;

			// adjust velocity according to proportion.
			velocity *= newspeed;
		}
	}

	// zero forwardmove and sidemove.
	if (ticks > ((max - 1) - g_csgo.m_clientstate()->m_choked_commands) || !g_csgo.m_clientstate()->m_choked_commands) {
		g_ctx.get_command()->m_forwardmove = g_ctx.get_command()->m_sidemove = 0.f;
	}
}









































