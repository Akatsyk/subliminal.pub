#include "..\hooks.hpp"

#include "..\..\cheats\ragebot\antiaim.h"
#include "..\..\cheats\sdk\animation_state.h"
#include "..\..\cheats\visuals\other_esp.h"
#include "..\..\cheats\misc\fakelag.h"
#include "..\..\cheats\misc\circlestrafer.h"
#include "..\..\cheats\misc\prediction_system.h"
#include "..\..\cheats\ragebot\aimbot.h"
#include "..\..\cheats\legit\legit_backtrack.h"
#include "..\..\cheats\misc\bunnyhop.h"
#include "..\..\cheats\misc\airstrafe.h"
#include "..\..\cheats\lagcompensation\lagcompensation.h"
#include "..\..\cheats\misc\spammers.h"
#include "..\..\cheats\fakewalk\fakewalk.h"

using CreateMove_t = bool( __thiscall* )( IClientMode *, float, CUserCmd * );

bool datagram_setup = false;

bool __stdcall hooks::hooked_createmove( float smt, CUserCmd * m_pcmd ) {
	static auto original_fn = clientmode_hook->get_func_address< CreateMove_t >( 24 );
	original_fn(g_csgo.m_clientmode(), smt, m_pcmd);

	if ( !m_pcmd->m_command_number || !g_ctx.available( ) || !g_ctx.m_local )
		return original_fn( g_csgo.m_clientmode( ), smt, m_pcmd );

	spammers::get( ).clan_tag( );

	if ( !g_ctx.m_local->is_alive( ) )
		return original_fn( g_csgo.m_clientmode( ), smt, m_pcmd );

	g_ctx.set_command( m_pcmd );

	// update fixed server time
	util::server_time( m_pcmd );

	// ping spike
	if ( g_cfg.misc.ping_spike && !netchannel_hook ) {
		auto netchannel = g_csgo.m_clientstate( )->m_net_channel;

		if ( netchannel ) {
			netchannel_hook = new vmthook( reinterpret_cast< DWORD** >( netchannel ) );
			netchannel_hook->hook_function( reinterpret_cast< uintptr_t >( hooked_senddatagram ), 48 );
		}
	}
	// todo: remove this
	if (g_cfg.misc.sniper_crosshair) {
		static bool o = false;
		if (!o) {o = true;	g_csgo.m_cvar()->FindVar("weapon_debug_spread_show")->SetValue(3); }
	}
	else {
		static bool o = false;
		if (!o) { o = true;	g_csgo.m_cvar()->FindVar("weapon_debug_spread_show")->SetValue(0); }
	}

	// update animations
	animation_state::get().create_move();

	g_csgo.m_cvar()->FindVar("r_jiggle_bones")->SetValue(0);

	uintptr_t *frame_ptr;
	__asm mov frame_ptr, ebp;

	g_ctx.send_packet = true;
	g_ctx.m_globals.fakelagging = false;
	vec3_t original_angle = m_pcmd->m_viewangles;

	// upredicted velocity for lby breaker
	g_ctx.unpred_velocity = g_ctx.m_local->m_vecVelocity( );

	bunnyhop::get().create_move();

	airstrafe::get().create_move();

	fakelag::get().create_move();

	// engine prediction
	engineprediction::get( ).start( ); {

		fakewalk::get().create_move();

		vec3_t wish_angle = m_pcmd->m_viewangles;
		m_pcmd->m_viewangles = original_angle;

		antiaim::get().update_lowerbody_breaker();

		antiaim::get( ).create_move( );

		aimbot::get( ).create_move( );

		// add shots to counter
		g_ctx.m_globals.add_shot( );

		// fix movement
		util::movement_fix( wish_angle, m_pcmd );
	}
	engineprediction::get().finish();


	lagcompensation::get( ).create_move( );

	// clamp choked commands
	if ( g_csgo.m_clientstate( )->m_choked_commands > 14 )
		g_ctx.send_packet = true;

	// set send_packet
	*( bool* )( *frame_ptr - 0x1C ) = g_ctx.send_packet;

	// get angles for thirdperson
	if (!g_ctx.send_packet)
		g_ctx.m_globals.real_angles = m_pcmd->m_viewangles;
	else
		g_ctx.m_globals.fake_angles = m_pcmd->m_viewangles;

	// clamp angles for anti-ut
	if ( g_cfg.misc.anti_untrusted )
		math::normalize_angles( m_pcmd->m_viewangles );

	// movement clamp
	if ( m_pcmd->m_forwardmove > 450.0f )
		m_pcmd->m_forwardmove = 450.0f;
	else if ( m_pcmd->m_forwardmove < -450.0f )
		m_pcmd->m_forwardmove = -450.0f;

	if ( m_pcmd->m_sidemove > 450.0f )
		m_pcmd->m_sidemove = 450.0f;
	else if ( m_pcmd->m_sidemove < -450.0f )
		m_pcmd->m_sidemove = -450.0f;

	return false;
}







































