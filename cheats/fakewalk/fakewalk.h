#pragma once

#include "..\..\includes.hpp"

class fakewalk : public singleton< fakewalk > {
public:
	void create_move( );
private:
	float fake_walk_called = 0.f;
};