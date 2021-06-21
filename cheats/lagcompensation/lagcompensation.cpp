#include "lagcompensation.h"

void lagcompensation::fsn(ClientFrameStage_t stage) {
	for (int i = 1; i <= g_csgo.m_globals()->m_maxclients; i++) {
		auto e = static_cast<player_t*>(g_csgo.m_entitylist()->GetClientEntity(i));
		auto& player = players[i];

		player.m_e = e;

		if (!continue_loop(i, e, stage))
			continue;

		switch (stage) {
		case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
			e->set_abs_origin(e->m_vecOrigin());

			player.m_resolver->m_e = e;
			player.m_resolver->run();

			break;
		case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
			e->update_clientside_animation();

			player.log();

			break;
		case FRAME_NET_UPDATE_END:
			apply_interpolation_flags(e, disable_interpolation);

			break;
		case FRAME_RENDER_START:
			*(int*)((uintptr_t)e + 0xA30) = g_csgo.m_globals()->m_framecount;
			*(int*)((uintptr_t)e + 0xA28) = 0;

			animation_fix(e);

			break;
		}
	}
}

void lagcompensation::create_move() {
	last_eye_positions.push_front(g_ctx.m_local->get_eye_pos());
	if (last_eye_positions.size() > 128)
		last_eye_positions.pop_back();

	auto nci = g_csgo.m_engine()->GetNetChannelInfo();
	if (!nci)
		return;

	const int latency_ticks = TIME_TO_TICKS(nci->GetLatency(FLOW_OUTGOING));
	const auto latency_based_eye_pos = last_eye_positions.size() <= latency_ticks ? last_eye_positions.back() : last_eye_positions[latency_ticks];

	for (int i = 1; i <= g_csgo.m_globals()->m_maxclients; i++) {
		auto e = static_cast<player_t*>(g_csgo.m_entitylist()->GetClientEntity(i));
		auto& player = players[i];

		player.m_e = e;

		if (!e) { player.m_e = nullptr; continue; }

		if (!e->valid(true))
			continue;

		player.m_resolver->m_e = e;
		player.m_resolver->create_move(latency_based_eye_pos);
	}
}

void player_record_t::log() {
	if (!g_cfg.ragebot.lagcomp) {
		m_track.clear();

		return;
	}

	player_t* e = m_e;

	if (!m_track.size()) {
		tickrecord_t record;
		record.save(e, m_resolver);

		m_track.push_front(record);

		return;
	}

	if (m_track.front().m_simulation_time != e->m_flSimulationTime()) {
		tickrecord_t record;
		record.save(e, m_resolver);

		record.m_previous_origin = m_track.front().m_vec_origin;
		record.m_previous_simulation_time = m_track.front().m_simulation_time;
		record.m_previous_curtime = m_track.front().m_curtime;

		m_track.push_front(record);
	}

	if (g_csgo.m_globals()->m_curtime - m_track.back().m_curtime > 1.f)
		m_track.pop_back();
}

std::deque< tickrecord_t > player_record_t::get_valid_track() {
	auto delta_time = [&](float simulation_time) -> float {
		const auto nci = g_csgo.m_engine()->GetNetChannelInfo();
		if (!nci)
			return 0.f;

		float correct = 0;

		correct += nci->GetLatency(FLOW_OUTGOING);
		correct += nci->GetLatency(FLOW_INCOMING);
		correct += util::lerp_time();

		const auto delta_time = correct - (g_csgo.m_globals()->m_curtime - simulation_time);

		return fabsf(delta_time) <= 0.2f && correct < 1.f;
	};

	std::deque< tickrecord_t > track;

	for (const auto& record : m_track) {
		if (delta_time(record.m_simulation_time))
			track.push_back(record);
	}

	return track;
}

bool lagcompensation::continue_loop(int id, player_t* e, ClientFrameStage_t stage) {
	auto& player = players[id];
	if (!e) { player.m_e = nullptr; return false; }

	switch (stage) {
	case FRAME_RENDER_START:
		if (!e->valid(true))
			return false;

		break;
	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
		if (!e->valid(true)) {
			delete player.m_resolver;
			player.m_resolver = new c_player_resolver();

			return false;
		}

		break;
	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
		if (e->m_iHealth() <= 0 || e->m_iTeamNum() == g_ctx.m_local->m_iTeamNum()) {
			player.m_track.clear();

			return false;
		}

		if (e->IsDormant() || e->m_bGunGameImmunity())
			return false;

		break;
	case FRAME_NET_UPDATE_END:
		if (!e->valid(true))
			return false;

		break;
	}

	return true;
}

void lagcompensation::apply_interpolation_flags(player_t* e, int flag) {
	const auto var_map = reinterpret_cast<uintptr_t>(e) + 36;

	for (auto index = 0; index < *reinterpret_cast<int*>(var_map + 20); index++)
		*reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(var_map) + index * 12) = flag;
}

void lagcompensation::animation_fix(player_t* e) {
	c_baseplayeranimationstate* animation_state = e->get_animation_state();

	if (!animation_state)
		return;

	auto player = players[e->EntIndex()];

	if (player.m_track.size()) {
		tickrecord_t* record = &player.m_track.front();

		if (e->m_flSimulationTime() != record->m_anim_time) {
			e->m_bClientSideAnimation() = true;

			e->update_clientside_animation();

			record->m_anim_time = e->m_flSimulationTime();
			record->m_anim_state = e->get_animation_state();
		}
		else {
			e->m_bClientSideAnimation() = false;

			e->set_animation_state(record->m_anim_state);
		}

		e->set_abs_angles(vec3_t(0, animation_state->m_goal_feet_yaw, 0));
	}
}

void lagcompensation::update_sequence() {
	if (!g_csgo.m_clientstate())
		return;

	auto net_channel = g_csgo.m_clientstate()->m_net_channel;

	if (net_channel) {
		if (net_channel->m_nInSequenceNr > m_last_incoming_sequence_number) {
			m_last_incoming_sequence_number = net_channel->m_nInSequenceNr;
			sequences.push_front(incoming_sequence_t(net_channel->m_nInReliableState, net_channel->m_nOutReliableState, net_channel->m_nInSequenceNr, g_csgo.m_globals()->m_realtime));
		}

		if (sequences.size() > 2048)
			sequences.pop_back();
	}
}

void lagcompensation::clear_sequence() {
	m_last_incoming_sequence_number = 0;

	sequences.clear();
}

void lagcompensation::add_latency(INetChannel* net_channel) {
	for (auto& seq : sequences) {
		if (g_csgo.m_globals()->m_realtime - seq.m_current_time >= g_cfg.misc.ping_spike_value / 1000.f) {
			net_channel->m_nInReliableState = seq.m_in_reliable_state;
			net_channel->m_nInSequenceNr = seq.m_sequence_nr;

			break;
		}
	}
}


lag_record_t::lag_record_t(player_t* player)
{
	index = player->EntIndex();
	valid = true;
	m_fake_walk = false;
	//m_mode = RMODE_MOVING;
	m_dormant = player->IsDormant();
	m_velocity = player->m_vecVelocity();
	m_origin = player->m_vecOrigin();
	m_abs_origin = player->abs_origin();
	m_layers = player->get_anim_layers_();
	m_poses = player->get_pose_params();
	m_anim_time = player->m_flOldSimulationTime() + g_csgo.m_globals()->m_interval_per_tick;
	m_sim_time = player->m_flSimulationTime();
	m_duck = player->m_flDuckAmount();
	m_body = player->m_flLowerBodyYawTarget();
	m_eye_angles = player->m_angEyeAngles();
	m_abs_ang = player->abs_angles();
	m_flags = player->m_fFlags();
	m_rotation = player->m_angRotation();
	m_lag = TIME_TO_TICKS(player->m_flSimulationTime() - player->m_flOldSimulationTime());
	m_lby_flick = false;
	m_override = false;
	const auto collideable = player->GetCollideable();
	m_obb_maxs = collideable->OBBMaxs();
	m_obb_mins = collideable->OBBMins();
}

void lag_record_t::apply(player_t* player, bool backup) const
{
	player->m_fFlags() = m_flags;
	player->m_flSimulationTime() = m_sim_time;
	player->m_angEyeAngles() = m_eye_angles;
	player->set_abs_angles(m_abs_ang);
	player->m_vecVelocity() = m_velocity;
	player->set_abs_origin(backup ? m_abs_origin : m_origin);
	player->m_vecOrigin() = m_origin;
	player->m_flLowerBodyYawTarget() = m_body;
	player->m_flDuckAmount() = m_duck;
	const auto collideable = player->GetCollideable();
	collideable->OBBMaxs() = m_obb_maxs;
	collideable->OBBMins() = m_obb_mins;
}

bool player_log::handle_bone_setup(lag_record_t& record, const int bone_mask, matrix3x4_t* bone_out)
{
	auto player = static_cast<player_t*>(g_csgo.m_entitylist()->GetClientEntity(record.index));
	if (!player)
		return false;

	const auto hdr = player->get_model_ptr();
	if (!hdr)
		return false;

	auto bone_accessor = player->m_BoneAccessor();
	if (!bone_accessor)
		return false;

	const auto backup_matrix = bone_accessor->GetBoneArrayForWrite();
	if (!backup_matrix)
		return false;

	// back up
	const auto backup_absorigin = player->abs_origin();
	const auto backup_absangles = player->abs_angles();

	auto backup_poses = player->get_pose_params();
	auto backup_layers = player->get_anim_layers_();
	const auto backup_effects = player->m_fEffects();

	ALIGN16 matrix3x4_t parent_transform;
	math::angle_matrix(record.m_abs_ang, record.m_origin, parent_transform);

	Vector pos[128];
	QuaternionAligned q[128];
	ZeroMemory(pos, sizeof(Vector[128]));
	ZeroMemory(q, sizeof(QuaternionAligned[128]));

	player->m_fEffects() |= 8;
	player->set_abs_origin(record.m_origin);
	player->set_abs_angles(record.m_abs_ang);
	player->set_pose_params(record.m_poses);
	player->set_anim_layers(record.m_layers);

	// force game to call AccumulateLayers - pvs fix.
	g_ctx.m_globals.m_ishltv_fix = true;

	// set bone array for write.
	bone_accessor->SetBoneArrayForWrite(bone_out);

	// compute and build bones.
	player->standard_blending_rules(hdr, pos, q, record.m_sim_time, bone_mask);

	byte computed[32];
	ZeroMemory(computed, sizeof(byte[0x20]));
	player->build_transformations(hdr, pos, q, parent_transform, bone_mask, &computed[0]);

	// restore old matrix.
	bone_accessor->SetBoneArrayForWrite(backup_matrix);

	// restore original interpolated entity data.
	player->m_fEffects() = backup_effects;
	player->set_abs_origin(backup_absorigin);
	player->set_abs_angles(backup_absangles);
	player->set_pose_params(backup_poses);
	player->set_anim_layers(backup_layers);

	// revert to old game behavior.
	g_ctx.m_globals.m_ishltv_fix = false;

	return true;
}

void player_log::log(const ClientFrameStage_t stage)
{
	if (stage != FRAME_NET_UPDATE_END)
		return;

	if (!g_ctx.m_local)
		return;

	auto changed = false;

	for (auto i = 1; i < g_csgo.m_globals()->m_maxclients; i++)
	{
		auto& curlog = logs[i];
		auto player = static_cast<player_t*>(g_csgo.m_entitylist()->GetClientEntity(i));

		if (curlog.player != player)
		{
			curlog.record.clear();
		}
		curlog.player = player;

		if (!player || player == g_ctx.m_local)
		{
			curlog.record.clear();
			curlog.m_bRunningTimer = false;
			curlog.m_bShot = false;
			curlog.m_iMode = resolver_modes::lby;
			curlog.m_nShots = 0;
			curlog.m_nShotsLby = 0;
			continue;
		}

		if (player->m_iTeamNum() == g_ctx.m_local->m_iTeamNum())
		{
			if (curlog.record.size() > 1)
				curlog.record.pop_back();
		}

		bool reset = (!player->is_alive());

		// if this happens, delete all the lagrecords.
		if (reset)
		{
			player->m_bClientSideAnimation() = true;
			curlog.record.clear();
			curlog.m_bRunningTimer = false;
			curlog.m_bShot = false;
			curlog.m_iMode = resolver_modes::lby;
			curlog.m_nShots = 0;
			curlog.m_nShotsLby = 0;
			continue;
		}

		// indicate that this player has been out of pvs.
		// insert dummy record to separate records
		// to fix stuff like animation and prediction.
		if (player->IsDormant())
		{
			curlog.last_dormant_time = g_csgo.m_globals()->m_curtime;

			bool insert = true;

			// we have any records already?
			if (!curlog.record.empty())
			{
				const auto front = curlog.record.front();

				// we already have a dormancy separator.
				if (front.m_dormant)
					insert = false;
			}

			if (insert)
			{
				// add new record.
				curlog.record.push_front(lag_record_t(player));

				// get reference to newly added record.
				const auto current = &curlog.record.front();

				// mark as dormant.
				current->m_dormant = true;
			}
		}

		bool update = (curlog.record.empty() || player->m_flSimulationTime() > curlog.record[0].m_sim_time);
		// this is the first data update we are receving
		// OR we received data with a newer simulation context.
		if (update)
		{
			if (!curlog.record.empty() && curlog.record.front().m_dormant)
			{
				player->m_angRotation() = curlog.record.front().m_rotation;
			}
			// add new record.
			curlog.record.push_front(lag_record_t(player));

			// get reference to newly added record.
			auto current = &curlog.record.front();

			// mark as non dormant.
			current->m_dormant = false;

			// update animations on current record.
			// call resolver.
			//update_player_animations(current, player);

			auto setup_bones = [&](lag_record_t& record, int boneMask, matrix3x4_t* pBoneToWorldOut) -> bool
			{
				alignas(16) matrix3x4_t bone_out[128];
				const auto ret = handle_bone_setup(record, boneMask, bone_out);
				memcpy(pBoneToWorldOut, bone_out, sizeof(matrix3x4_t[128]));
				return ret;
			};

			//// create bone matrix for this record.
			setup_bones(*current, BONE_USED_BY_ANYTHING, current->matrix);

			changed = true;
		}

		// no need to store insane amt of data.
		while (curlog.record.size() > 256)
			curlog.record.pop_back();
	}

	if (changed)
		filter_records();
}

player_log_t& player_log::get_log(const int index)
{
	return logs[index];
}

void player_log::filter_records(bool cm)
{
	for (auto i = 1; i < g_csgo.m_globals()->m_maxclients; i++)
	{
		auto& curlog = logs[i];
		auto& record = curlog.record;
		if (record.empty())
		{
			curlog.m_bFilled = false;
			continue;
		}

		if (!curlog.player)
		{
			record.clear();
			curlog.m_bFilled = false;
			continue;
		}

		auto wasvalid = false;

		while (record.size() > 64)
			record.pop_back();

		for (auto j = 0u; j < record.size(); j++)
		{
			auto& currecord = record[j];
			if (currecord.m_dormant && j == 0)
				continue;

			if (currecord.m_dormant)
			{
				record.erase(record.begin() + j);
				j--;
				continue;
			}

			if ((currecord.valid = lagcompensation::get().valid_simtime(currecord.m_sim_time)))
			{
				wasvalid = true;
			}
			else if (wasvalid)
			{
				wasvalid = false;
				if (!cm)
					curlog.lastvalid_vis = j - 1;
				curlog.lastvalid = j - 1;
			}
		}
	}
}


bool lagcompensation::valid_simtime(const float& simtime)
{
	const auto nci = g_csgo.m_engine()->GetNetChannelInfo();
	if (!nci)
		return false;

	float correct = 0;

	correct += nci->GetLatency(FLOW_OUTGOING);
	correct += nci->GetLatency(FLOW_INCOMING);
	correct += util::lerp_time();

	const auto delta_time = correct - (g_csgo.m_globals()->m_curtime - simtime);

	return fabsf(delta_time) <= 0.2f && correct < 1.f;
}


//void player_log::update_player_animations(lag_record_t* record, player_t* m_player)
//{
//	auto log = &player_log::get().get_log(m_player->EntIndex());
//
//	auto state = m_player->get_animation_state();
//	if (!state)
//		return;
//
//	// player respawned.
//	if (m_player->m_flSpawnTime() != log->m_flSpawntime)
//	{
//		// reset animation state.
//		ResetAnimatreionState(state);
//
//		// note new spawn time.
//		log->m_flSpawntime = m_player->m_flSpawnTime();
//	}
//
//	// backup curtime.
//	float curtime = g_csgo.m_globals()->m_curtime;
//	float frametime = g_csgo.m_globals()->m_frametime;
//
//	// set curtime to animtime.
//	// set frametime to ipt just like on the server during simulation.
//	g_csgo.m_globals()->m_curtime = record->m_anim_time;
//	g_csgo.m_globals()->m_frametime = g_csgo.m_globals()->m_interval_per_tick;
//
//	// backup stuff that we do not want to fuck with.
//	AnimationBackup_t backup;
//
//	backup.m_origin = m_player->m_vecOrigin();
//	backup.m_abs_origin = m_player->abs_origin();
//	backup.m_velocity = m_player->m_vecVelocity();
//	backup.m_flags = m_player->m_fFlags();
//	backup.m_duck = m_player->m_flDuckAmount();
//	backup.m_body = m_player->m_flLowerBodyYawTarget();
//	backup.m_layers = m_player->get_anim_layers_();
//
//	// is player a bot?
//	bool bot = false;
//
//	// reset fakewalk state.
//	record->m_fake_walk = false;
//	//record->m_mode = Resolver::Modes::RESOLVE_NONE;
//
//	// fix velocity.
//	// https://github.com/VSES/SourceEngine2007/blob/master/se2007/game/client/c_baseplayer.cpp#L659
//	if (record->m_lag > 0 && record->m_lag < 16 && log->record.size() >= 2)
//	{
//		// get pointer to previous record.
//		auto previous = &log->record[1];
//
//		if (previous && !previous->m_dormant)
//			record->m_velocity = (record->m_origin - previous->m_origin) * (1.f / TICKS_TO_TIME(record->m_lag));
//	}
//
//	// set this fucker, it will get overriden.
//	record->m_anim_velocity = record->m_velocity;
//
//	// fix various issues with the game
//	// these issues can only occur when a player is choking data.
//	if (record->m_lag > 1 && !bot)
//	{
//		// detect fakewalk.
//		float speed = record->m_velocity.Length();
//
//		if (record->m_flags & FL_ONGROUND && record->m_layers[6].m_flWeight == 0.f && speed > 0.1f && speed < 100.f)
//			record->m_fake_walk = true;
//
//		if (record->m_fake_walk)
//			record->m_anim_velocity = record->m_velocity = { 0.f, 0.f, 0.f };
//
//		// we need atleast 2 updates/records
//		// to fix these issues.
//		if (log->record.size() >= 2)
//		{
//			// get pointer to previous record.
//			auto previous = &log->record[1];
//
//			if (previous && !previous->m_dormant)
//			{
//				// set previous flags.
//				m_player->m_fFlags() = previous->m_flags;
//
//				// strip the on ground flag.
//				m_player->m_fFlags() &= ~FL_ONGROUND;
//
//				// been onground for 2 consecutive ticks? fuck yeah.
//				if (record->m_flags & FL_ONGROUND && previous->m_flags & FL_ONGROUND)
//					m_player->m_fFlags() |= FL_ONGROUND;
//
//				// fix jump_fall.
//				else
//				{
//					if (record->m_layers[4].m_flWeight != 1.f && previous->m_layers[4].m_flWeight == 1.f && record->m_layers[5].m_flWeight != 0.f)
//						m_player->m_fFlags() |= FL_ONGROUND;
//
//					if (record->m_flags & FL_ONGROUND && !(previous->m_flags & FL_ONGROUND))
//						m_player->m_fFlags() &= ~FL_ONGROUND;
//				}
//
//				// calc fraction between both records.
//				float frac = (record->m_anim_time - record->m_sim_time) / (previous->m_sim_time - record->m_sim_time);
//
//				m_player->m_flDuckAmount() = math::lerp(record->m_duck, previous->m_duck, frac);
//
//				if (!record->m_fake_walk)
//					record->m_anim_velocity = math::lerp(record->m_velocity, previous->m_velocity, frac);
//			}
//		}
//	}
//
//
//	player.m_resolver->m_e = e;
//	c_player_resolver::get().run();
//
//	log->m_vecLastNonDormantOrig = record->m_origin;
//
//	// set stuff before animating.
//	m_player->get_origin() = record->m_origin;
//	m_player->set_abs_origin(record->m_origin);
//
//	m_player->get_velocity() = m_player->get_abs_velocity() = record->m_anim_velocity;
//	m_player->get_lby() = record->m_body;
//
//	// EFL_DIRTY_ABSVELOCITY
//	// skip call to C_BaseEntity::CalcAbsoluteVelocity
//	m_player->get_eflags() &= ~0x1000;
//
//	// write potentially resolved angles.
//	m_player->get_eye_angles() = record->m_eye_angles;
//
//	// fix animating in same frame.
//	if (state->m_iLastClientSideAnimationUpdateFramecount == g_pGlobals->framecount)
//		state->m_iLastClientSideAnimationUpdateFramecount -= 1;
//
//	//animations::get().set_interpolation_flags( m_player, 0 );
//
//	// 'm_animating' returns true if being called from SetupVelocity, passes raw velocity to animstate.
//	g_setup_vel_fix = true;
//	m_player->get_clientside_animation() = true;
//	m_player->update_clientside_anim();
//	m_player->get_clientside_animation() = false;
//	g_setup_vel_fix = false;
//
//	// store updated/animated poses and rotation in lagrecord.
//	record->m_poses = m_player->get_pose_params();
//	record->m_abs_ang = m_player->get_abs_angles();
//
//	// correct poses
//	resolver::resolve_poses(m_player, log);
//
//	// restore backup data.
//	m_player->get_origin() = backup.m_origin;
//	m_player->get_velocity() = backup.m_velocity;
//	m_player->get_abs_velocity() = backup.m_abs_velocity;
//	m_player->get_flags() = backup.m_flags;
//	m_player->get_eflags() = backup.m_eflags;
//	m_player->get_duck_amt() = backup.m_duck;
//	m_player->get_lby() = backup.m_body;
//	m_player->set_abs_origin(backup.m_abs_origin);
//	m_player->set_anim_layers(backup.m_layers);
//
//	// IMPORTANT: do not restore poses here, since we want to preserve them for rendering.
//	// also dont restore the render angles which indicate the model rotation.
//
//	// restore globals.
//	g_pGlobals->curtime = curtime;
//	g_pGlobals->frametime = frametime;
//}