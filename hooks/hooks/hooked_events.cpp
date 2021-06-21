#include "..\hooks.hpp"
#include "..\..\cheats\misc\logs.h"
#include "..\..\cheats\visuals\other_esp.h"
#include "..\..\cheats\visuals\bullet_tracers.h"
#include "..\..\cheats\ragebot\antiaim.h"
#include "../../buybot.h"

#pragma comment(lib, "Winmm.lib")
static bool m_setup = false;
void setup()
{
	if (m_setup)
		return;

	static auto developer = g_csgo.m_cvar()->FindVar("developer");
	developer->SetValue(1);
	static auto con_filter_text = g_csgo.m_cvar()->FindVar("con_filter_text");
	static auto con_filter_text_out = g_csgo.m_cvar()->FindVar("con_filter_text_out");
	static auto con_filter_enable = g_csgo.m_cvar()->FindVar("con_filter_enable");
	static auto contimes = g_csgo.m_cvar()->FindVar("contimes");

	contimes->SetValue(15);
	con_filter_text->SetValue("L ");
	con_filter_text_out->SetValue(" ");
	con_filter_enable->SetValue(2);

	g_csgo.m_engine()->ExecuteClientCmd("clear");

	m_setup = true;
}

void C_HookedEvents::FireGameEvent( IGameEvent* event ) {

	setup();

	static auto hit_sound = [ &event ] ( ) -> void {
		if ( !g_ctx.available( ) )
			return;

		auto attacker = event->GetInt( "attacker" );
		auto user = event->GetInt( "userid" );

		if (g_cfg.esp.hitsound) {
			if ( g_csgo.m_engine( )->GetPlayerForUserID( user ) != g_csgo.m_engine( )->GetLocalPlayer( ) && g_csgo.m_engine( )->GetPlayerForUserID( attacker ) == g_csgo.m_engine( )->GetLocalPlayer( ) ) {
				g_csgo.m_surface( )->PlaySound_( "buttons\\arena_switch_press_02.wav" );
			}
		}
	};

	auto event_name = event->GetName( );

	if (!strcmp(event_name, "round_prestart"))
	{
		buybot::get().on_event();
		antiaim::get().freeze_check = true;
	}
	else if ( !strcmp( event_name, "round_freeze_end" ) )
		antiaim::get( ).freeze_check = false;

	if ( g_cfg.esp.hitsound ) {
		if (!strcmp(event_name, "player_hurt"))
			hit_sound( );
	}

	if ( g_cfg.esp.hitmarker[ HITMARKER_STATIC ].enabled || g_cfg.esp.hitmarker[ HITMARKER_DYNAMIC ].enabled )
		otheresp::get( ).hitmarker_event( event );

	if ( g_cfg.esp.bullet_tracer )
		bullettracers::get( ).events( event );

	eventlogs::get( ).events( event );

	g_ctx.m_globals.remove_shot( event );
}

int C_HookedEvents::GetEventDebugID( void ) {
	return EVENT_DEBUG_ID_INIT;
}

void C_HookedEvents::RegisterSelf( ) {
	m_iDebugId = EVENT_DEBUG_ID_INIT;
	g_csgo.m_eventmanager( )->AddListener( this, "player_hurt", false );
	g_csgo.m_eventmanager( )->AddListener( this, "item_purchase", false );
	g_csgo.m_eventmanager( )->AddListener( this, "bomb_beginplant", false );
	g_csgo.m_eventmanager( )->AddListener( this, "bomb_begindefuse", false );
	g_csgo.m_eventmanager( )->AddListener( this, "bullet_impact", false );
	g_csgo.m_eventmanager( )->AddListener( this, "round_prestart", false );
	g_csgo.m_eventmanager( )->AddListener( this, "round_freeze_end", false );
}

void C_HookedEvents::RemoveSelf() {
	g_csgo.m_eventmanager()->RemoveListener(this);
}






































