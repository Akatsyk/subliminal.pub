#include "animation_state.h"

float spawn_time = 0.0f;
CBaseHandle * entity_handle = nullptr;
c_baseplayeranimationstate * server_animation_state = nullptr;

void animation_state::create_move( ) {
	bool
		allocate = ( server_animation_state == nullptr ),
		change = ( !allocate ) && ( &g_ctx.m_local->GetRefEHandle( ) != entity_handle ),
		reset = ( !allocate && !change ) && ( g_ctx.m_local->m_flSpawnTime( ) != spawn_time );

	if ( change )
		g_csgo.m_memalloc( )->Free( server_animation_state );

	if ( reset ) {
		reset_state( server_animation_state );
		spawn_time = g_ctx.m_local->m_flSpawnTime( );
	}

	if ( allocate || change ) {
		auto state = ( c_baseplayeranimationstate * )g_csgo.m_memalloc( )->Alloc( sizeof( c_baseplayeranimationstate ) );

		if ( state != nullptr )
			create_state( state, g_ctx.m_local );

		entity_handle = const_cast< CBaseHandle * >( &g_ctx.m_local->GetRefEHandle( ) );
		spawn_time = g_ctx.m_local->m_flSpawnTime( );

		server_animation_state = state;
	}

	float server_time = util::server_time( );
	if ( !g_csgo.m_clientstate( )->m_choked_commands)
	{

		float pose_parameters[24];
		AnimationLayer anim_layers[13];

		const auto curtime = g_csgo.m_globals()->m_curtime;
		const auto frametime = g_csgo.m_globals()->m_frametime;

		// set curtime to animtime.
		// set frametime to ipt just like on the server during simulation.
		g_csgo.m_globals()->m_curtime = g_ctx.m_local->m_flOldSimulationTime() + g_csgo.m_globals()->m_interval_per_tick;
		g_csgo.m_globals()->m_frametime = g_csgo.m_globals()->m_interval_per_tick;

		// backup stuff that we do not want to fuck with.
		const auto backup_origin = g_ctx.m_local->m_vecOrigin();
		const auto backup_velocity = g_ctx.m_local->m_vecVelocity();
		const auto backup_flags = g_ctx.m_local->m_fFlags();
		const auto backup_duck_amt = g_ctx.m_local->m_flDuckAmount();
		const auto backup_lby = g_ctx.m_local->m_flLowerBodyYawTarget();

		memcpy(pose_parameters, g_ctx.m_local->m_flPoseParameter(), sizeof(float) * 24);
		memcpy(anim_layers, g_ctx.m_local->get_animlayers(), sizeof(AnimationLayer) * 13);

		if (server_animation_state->m_frame == g_csgo.m_globals()->m_framecount)
			server_animation_state->m_frame -= 1;

		update_state(server_animation_state, g_ctx.get_command()->m_viewangles);

		g_ctx.m_local->m_vecOrigin() = backup_origin;
		g_ctx.m_local->m_vecVelocity() = backup_velocity;
		g_ctx.m_local->m_fFlags() = backup_flags;
		g_ctx.m_local->m_flDuckAmount() = backup_duck_amt;
		g_ctx.m_local->m_flLowerBodyYawTarget() = backup_lby;

		memcpy(g_ctx.m_local->m_flPoseParameter(), pose_parameters, sizeof(float) * 24);
		memcpy(g_ctx.m_local->get_animlayers(), anim_layers, sizeof(AnimationLayer) * 13);

		g_csgo.m_globals()->m_curtime = curtime;
		g_csgo.m_globals()->m_frametime = frametime;
	}
}

void animation_state::create_state( c_baseplayeranimationstate * state, player_t * player ) {
	using Fn = void( __thiscall* )( c_baseplayeranimationstate*, player_t* );
	static auto fn = reinterpret_cast< Fn >( util::pattern_scan( "client.dll", "55 8B EC 56 8B F1 B9 ? ? ? ? C7 46" ) );
	fn( state, player );
}

void animation_state::update_state( c_baseplayeranimationstate * state, vec3_t angles ) {
	using Fn = void( __vectorcall* )( void *, void *, float, float, float, void * );
	static auto fn = reinterpret_cast< Fn >( util::pattern_scan( "client.dll", "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24" ) );
	fn( state, nullptr, 0.0f, angles[ 1 ], angles[ 0 ], nullptr );
}

void animation_state::reset_state( c_baseplayeranimationstate * state ) {
	using Fn = void( __thiscall* )( c_baseplayeranimationstate * );
	static auto fn = reinterpret_cast< Fn >( util::pattern_scan( "client.dll", "56 6A 01 68 ? ? ? ? 8B F1" ) );
	fn( state );
}

c_baseplayeranimationstate * animation_state::animstate( ) {
	return server_animation_state;
}









































