#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"
#include <deque>

#define NET_FRAMES_BACKUP 64 // must be power of 2. 
#define NET_FRAMES_MASK ( NET_FRAMES_BACKUP - 1 )

enum resolver_modes : int {
	none,
	brute,
	override,
	lby,
	lby_update,
	anti_freestand
};

class c_player_resolver {
public:
	int m_mode;

	struct resolver_info_t {
		// data
		vec3_t m_velocity;
		vec3_t m_origin;
		float m_lowerbody;
		int m_flags;
		float m_pitch;
		float m_at_target;

		// last moving lby
		float m_last_move_lby;
		bool m_last_move_lby_valid;
		float m_last_move_time;

		// ghetto pitch fix
		float last_pitchdown;

		// stuff for fakewalk
		AnimationLayer m_anim_layers[ 15 ];

		bool m_balance_adjust_triggered, m_balance_adjust_playing;
		float m_last_balance_adjust_time;

		bool m_fakewalking;
		bool m_predicted_flick;
		bool m_flick;

		// angle data
		vec3_t back;
		vec3_t left;
		vec3_t right;

		// brute data
		float m_predicted_yaw_brute;
	};

	struct anti_freestand_info_t {
		int right_damage = 0, left_damage = 0;
		float right_fraction = 0.f, left_fraction = 0.f;
	};

	player_t * m_e;

	resolver_info_t m_current;
	resolver_info_t m_previous;
	anti_freestand_info_t m_antifreestand;

	bool m_has_previous;

	void create_move( vec3_t latency_based_eye_pos );
	void lby_prediction();
	void run( );
	void update( );
	void update_fakewalk_data( resolver_info_t & current, player_t * e );
	void resolve( );
	void pitch_resolve( );
};

struct tickrecord_t {
	vec3_t m_vec_origin;
	vec3_t m_eye_angles;
	vec3_t m_abs_angles;
	vec3_t m_bbmin;
	vec3_t m_bbmax;
	vec3_t m_velocity;
	vec3_t m_abs_origin;

	vec3_t m_previous_origin;
	float m_previous_simulation_time;
	float m_previous_curtime;

	float m_simulation_time;
	float m_duck_amount;
	float m_duck_speed;
	float m_lowerbody;
	float m_curtime;

	int m_flags;
	int m_cached_count;
	int m_writable_bones;

	matrix3x4_t m_bone_matrix[ 128 ];
	std::array<float, 24u> m_pose_params;
	std::array<AnimationLayer, 15u> m_anim_layers;

	float m_anim_time;
	c_baseplayeranimationstate * m_anim_state;

	c_player_resolver* m_resolver;

	void save( player_t * e, c_player_resolver* resolver = nullptr ) {
		m_resolver = resolver;

		m_vec_origin = e->m_vecOrigin();
		m_abs_angles = e->abs_angles();
		m_eye_angles = e->m_angEyeAngles();
		m_velocity = e->m_vecVelocity();
		m_simulation_time = e->m_flSimulationTime();
		m_duck_amount = e->m_flDuckAmount();
		m_duck_speed = e->m_flDuckSpeed();
		m_lowerbody = e->m_flLowerBodyYawTarget();
		m_flags = e->m_fFlags();
		m_curtime = g_csgo.m_globals()->m_curtime;

		m_anim_layers = e->get_anim_layers_();
		m_pose_params = e->get_pose_params();

		auto collideable = e->GetCollideable();
		m_bbmin = collideable->OBBMins();
		m_bbmax = collideable->OBBMaxs();

		m_cached_count = e->get_bone_cache()->m_CachedBoneCount;
		if (m_cached_count)
		{
			memcpy(m_bone_matrix, e->get_bone_cache()->m_pCachedBones, 48 * m_cached_count);
			m_writable_bones = *(&e->m_BoneAccessor()->m_WritableBones + 8);
		}
		else m_writable_bones = 0;
	}

	// function: writes current record to bone cache.
	void apply(player_t* player, bool backup = false)
	{
		player->m_fFlags() = m_flags;
		player->m_flSimulationTime() = m_simulation_time;
		player->m_angEyeAngles() = m_eye_angles;
		player->set_abs_angles(m_abs_angles);
		player->m_vecVelocity() = m_velocity;
		player->set_abs_origin(backup ? m_abs_origin : m_vec_origin);
		player->m_vecOrigin() = m_vec_origin;
		player->m_flLowerBodyYawTarget() = m_lowerbody;
		player->m_flDuckAmount() = m_duck_amount;

		const auto collideable = player->GetCollideable();
		collideable->OBBMaxs() = m_bbmax;
		collideable->OBBMins() = m_bbmin;

		if (m_cached_count && m_cached_count > 0)
		{
			memcpy(player->get_bone_cache()->m_pCachedBones, m_bone_matrix, 48 * m_cached_count);
			*(&player->m_BoneAccessor()->m_WritableBones + 8) = m_writable_bones;
		}
	}
};

struct dormant_record_t
{
	vec3_t m_abs_ang;
	float m_flEyePitch; //0x74
	float m_flEyeYaw; //0x78
	float m_flPitch; //0x7C
	float m_flGoalFeetYaw; //0x80
	float m_flCurrentFeetYaw; //0x84
	float m_flCurrentTorsoYaw; //0x88
	float m_flLeanAmount; //0x90
	float m_flFeetCycle; //0x98 0 to 1
	float m_flFeetYawRate; //0x9C 0 to 1
};

struct lag_record_t
{
	lag_record_t() = default;
	lag_record_t(player_t* player);
	void apply(player_t* player, bool backup = false) const;

	int index;
	bool valid;
	matrix3x4_t matrix[128];

	bool m_fake_walk;
	int m_mode;
	bool m_dormant;
	vec3_t m_velocity;
	vec3_t m_origin;
	vec3_t m_abs_origin;
	vec3_t m_anim_velocity;
	vec3_t m_obb_mins;
	vec3_t m_obb_maxs;
	std::array<AnimationLayer, 15> m_layers;
	std::array<float, 24> m_poses;
	float m_anim_time;
	float m_sim_time;
	float m_duck;
	float m_body;
	vec3_t m_eye_angles;
	vec3_t m_abs_ang;
	int m_flags;
	int m_lag;
	vec3_t m_rotation;

	int m_cached_count;
	int m_tick;
	int m_writable_bones;

	bool m_lby_flick;
	bool m_shot;
	bool m_override;

	// lagfix stuff.
	bool   m_broke_lc;
	vec3_t m_pred_origin;
	vec3_t m_pred_velocity;
	float  m_pred_time;
	int    m_pred_flags;

	void predict() {
		m_broke_lc = false;
		m_pred_origin = m_origin;
		m_pred_velocity = m_velocity;
		m_pred_time = m_sim_time;
		m_pred_flags = m_flags;
	}
};


struct player_log_t
{
	player_log_t()
	{
		player = nullptr;
		m_bCustomCorrection = false;
		m_bFilled = false;
		m_iPlayerListYawCorrection = 0;
		onground_nospread = false;
		m_bOverride = false;
		m_nShotsLby = 0;
		last_hit_bute = 0;
		m_iMode = 0;
		lastvalid = 0;
		lastvalid_vis = 0;
		spindelta = 0;
		spinbody = 0;
		step = 0;
	}
	player_t* player;
	bool m_bFilled;
	int m_iMode;

	float spindelta;
	float spinbody;
	int step;

	int lastvalid;
	int lastvalid_vis;

	float m_flSpawntime;

	float m_flLowerBodyYawTarget;
	float m_flOldLowerBodyYawTarget;
	float m_flLastLowerBodyYawTargetUpdateTime;
	float m_flLastMovingLowerBodyYawTarget;
	float m_flLastMovingLowerBodyYawTargetTime;
	float nextBodyUpdate;
	float m_flSavedLbyDelta;
	float m_flBruteStartLby;

	vec3_t m_vecLastShotEyeAngles;
	float m_flLastShotSimulationTime;

	int m_iPlayerListYawCorrection;
	bool m_bCustomCorrection;
	float m_flPitch;

	float m_flProxyPitch;
	float m_flProxyYaw;

	bool onground_nospread;
	bool fakewalking;

	int m_nShots;
	int oldshots;
	int m_nShotsLby;

	bool m_bLbyFlick;
	bool m_bShot;
	bool m_bOverride;

	int last_hit_bute;

	bool m_bRunningTimer;
	vec3_t m_vecLastNonDormantOrig;

	bool m_moved;
	float m_body_update;
	int m_stand_index;
	int m_stand_index2;
	int m_body_index;
	lag_record_t m_walk_record;

	std::deque<lag_record_t> record;
	dormant_record_t dormant_record;
	float last_dormant_time;
	vec3_t last_nondormant_origin;
};

class player_log : public singleton<player_log>
{
public:
	bool handle_bone_setup(lag_record_t& record, const int bone_mask, matrix3x4_t* bone_out);
	void log(const ClientFrameStage_t stage);
	player_log_t& get_log(const int index);
	void filter_records(bool cm = false);
private:
	player_log_t logs[65];
};

struct player_record_t {
	player_t * m_e;

	c_player_resolver * m_resolver = new c_player_resolver( );

	void log( );

	std::deque< tickrecord_t > get_valid_track( );

	std::deque< tickrecord_t > m_track;
};

struct incoming_sequence_t {
	incoming_sequence_t::incoming_sequence_t( int instate, int outstate, int seqnr, float time ) {
		m_in_reliable_state = instate;
		m_out_reliable_state = outstate;
		m_sequence_nr = seqnr;
		m_current_time = time;
	}

	int m_in_reliable_state;
	int m_out_reliable_state;
	int m_sequence_nr;
	float m_current_time;
};

enum {
	disable_interpolation,
	enable_interpolation
};

class lagcompensation : public singleton< lagcompensation > {
public:
	void fsn( ClientFrameStage_t stage );
	void create_move( );
	bool continue_loop( int id, player_t * e, ClientFrameStage_t stage );

	void apply_interpolation_flags( player_t * e, int flag );

	void animation_fix(player_t* e);


	player_record_t players[ 65 ];
	std::deque< vec3_t > last_eye_positions;
public:
	std::deque< incoming_sequence_t > sequences;
	int m_last_incoming_sequence_number;

	void update_sequence( );
	void clear_sequence( );
	void add_latency( INetChannel * net_channel );

	bool valid_simtime(const float& simtime);

	// animation variables.
	vec3_t  m_angle;
	vec3_t  m_rotation;
	vec3_t  m_radar;
	float  m_body;
	float  m_body_pred;
	float  m_speed;
	float  m_anim_time;
	float  m_anim_frame;
	bool   m_ground;
	bool   m_lagcomp;
};

static bool complete_restore = false;