#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"

class bullettracers : public singleton< bullettracers > {
private:
	class trace_info {
	public:
		trace_info( Vector starts, Vector positions, float times, Color color ) {
			this->start = starts;
			this->position = positions;
			this->time = times;
			this->color = color;
		}

		Vector position;
		Vector start;
		float time;
		Color color;
	};

	std::vector<trace_info> impacts;

public:
	void draw_beam();
	void events( IGameEvent* event );
	bool beam_exists(const float curtime);
};