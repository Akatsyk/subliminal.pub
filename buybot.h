#pragma once

#include "includes.hpp"

class buybot : public singleton< buybot > {
public:
	void on_event();
};