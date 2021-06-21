#include "buybot.h"

void buybot::on_event()
{
	if (g_csgo.m_inputsys()->IsButtonDown(g_cfg.buy.additional))
	{
		switch (g_cfg.buy.add_main_weapon)
		{
		case 0: g_csgo.m_engine()->ClientCmd("buy scar20; buy g3sg1;"); break;
		case 1: g_csgo.m_engine()->ClientCmd("buy ssg08;"); break;
		case 2: g_csgo.m_engine()->ClientCmd("buy awp;"); break;
		default:
			break;
		}

		switch (g_cfg.buy.add_sec_weapon)
		{
		case 1: g_csgo.m_engine()->ClientCmd("buy deagle; buy revolver;"); break;
		case 2: g_csgo.m_engine()->ClientCmd("buy elite;"); break;
		default:
			break;
		}

		if (g_cfg.buy.add_equipment[0].enabled)
			g_csgo.m_engine()->ClientCmd("buy hegrenade;");

		if (g_cfg.buy.add_equipment[1].enabled)
			g_csgo.m_engine()->ClientCmd("buy smokegrenade;");

		if (g_cfg.buy.add_equipment[2].enabled)
			g_csgo.m_engine()->ClientCmd("buy incgrenade; buy molotov;");

		if (g_cfg.buy.add_equipment[3].enabled)
			g_csgo.m_engine()->ClientCmd("buy defuser;");

		if (g_cfg.buy.add_equipment[4].enabled)
			g_csgo.m_engine()->ClientCmd("buy vest;");

		if (g_cfg.buy.add_equipment[5].enabled)
			g_csgo.m_engine()->ClientCmd("buy vesthelm;");
		
		if (g_cfg.buy.add_equipment[6].enabled)
			g_csgo.m_engine()->ClientCmd("buy taser;");
	}
	else
	{
		switch (g_cfg.buy.main_weapon)
		{
		case 0: g_csgo.m_engine()->ClientCmd("buy scar20; buy g3sg1;"); break;
		case 1: g_csgo.m_engine()->ClientCmd("buy ssg08;"); break;
		case 2: g_csgo.m_engine()->ClientCmd("buy awp;"); break;
		default:
			break;
		}

		switch (g_cfg.buy.sec_weapon)
		{
		case 1: g_csgo.m_engine()->ClientCmd("buy deagle; buy revolver;"); break;
		case 2: g_csgo.m_engine()->ClientCmd("buy elite;"); break;
		default:
			break;
		}

		if (g_cfg.buy.equipment[0].enabled)
			g_csgo.m_engine()->ClientCmd("buy hegrenade;");

		if (g_cfg.buy.equipment[1].enabled)
			g_csgo.m_engine()->ClientCmd("buy smokegrenade;");

		if (g_cfg.buy.equipment[2].enabled)
			g_csgo.m_engine()->ClientCmd("buy incgrenade; buy molotov;");

		if (g_cfg.buy.equipment[3].enabled)
			g_csgo.m_engine()->ClientCmd("buy defuser;");

		if (g_cfg.buy.equipment[4].enabled)
			g_csgo.m_engine()->ClientCmd("buy vest;");

		if (g_cfg.buy.equipment[5].enabled)
			g_csgo.m_engine()->ClientCmd("buy vesthelm;");

		if (g_cfg.buy.equipment[6].enabled)
			g_csgo.m_engine()->ClientCmd("buy taser;");
	}
}