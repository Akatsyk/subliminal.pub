#include "nightmode.h"

bool strstric(const std::string& strHaystack, const std::string& strNeedle)
{
	auto it = std::search(
		strHaystack.begin(), strHaystack.end(),
		strNeedle.begin(), strNeedle.end(),
		[](char ch1, char ch2) { return toupper(ch1) == toupper(ch2); }
	);
	return (it != strHaystack.end());
}

void nightmode::dark_mode() const
{
	const auto reset = [&]()
	{
		g_csgo.m_cvarspoofer()->get("r_drawspecificstaticprop")->SetInt(1);

		for (auto i = g_csgo.m_materialsystem()->FirstMaterial(); i != g_csgo.m_materialsystem()->InvalidMaterial(); i = g_csgo.m_materialsystem()->NextMaterial(i))
		{
			auto mat = g_csgo.m_materialsystem()->GetMaterial(i);
			if (!mat)
				continue;

			if (mat->IsErrorMaterial())
				continue;

			std::string name = mat->GetName();
			auto tex_name = mat->GetTextureGroupName();


			if (strstr(tex_name, "World") || strstr(tex_name, "StaticProp") || strstr(tex_name, "SkyBox"))
			{
				mat->ColorModulate(1.f, 1.f, 1.f);
				mat->AlphaModulate(1.f);
			}
		}
	};

	const auto set = [&]()
	{
		static auto load_named_sky = reinterpret_cast<void(__fastcall*)(const char*)>(util::pattern_scan("engine.dll", "55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45"));
		load_named_sky("sky_csgo_night02");

		g_csgo.m_cvarspoofer()->get("r_drawspecificstaticprop")->SetInt(0);

		for (auto i = g_csgo.m_materialsystem()->FirstMaterial(); i != g_csgo.m_materialsystem()->InvalidMaterial(); i = g_csgo.m_materialsystem()->NextMaterial(i))
		{
			auto mat = g_csgo.m_materialsystem()->GetMaterial(i);
			if (!mat)
				continue;

			if (mat->IsErrorMaterial())
				continue;

			std::string name = mat->GetName();
			auto tex_name = mat->GetTextureGroupName();

			auto dark_wall = (100 - g_cfg.esp.darkness) / 100.f;
			auto dark_prop = (130 - g_cfg.esp.darkness) / 100.f;

			math::clamp(dark_wall, 0.3f, 0.8f);
			math::clamp(dark_wall, 0.3f, 0.8f);

			if (g_cfg.esp.nightmode && strstr(tex_name, "World"))
			{
				mat->ColorModulate(dark_prop, dark_prop, dark_prop);
			}
			if (strstr(tex_name, "StaticProp"))
			{
				if (g_cfg.esp.nightmode)
					mat->ColorModulate(dark_wall, dark_wall, dark_wall);
				if (!strstric(name, "wall"))
					mat->AlphaModulate(1.f - g_cfg.esp.prop_transparency * 0.01f);
			}

			if (g_cfg.esp.nightmode && strstr(tex_name, "SkyBox"))
			{
				mat->ColorModulate(228.f / 255.f, 35.f / 255.f, 157.f / 255.f);
			}

		}


	};

	static auto done = true;
	static auto last_setting = false;
	static auto last_transparency = 0.f;
	static auto last_darkness = 0.f;
	static auto was_ingame = false;

	if (!done)
	{
		if (last_setting || last_transparency || last_darkness)
		{
			reset();
			set();
			done = true;
		}
		else
		{
			reset();
			done = true;
		}
	}


	if ( was_ingame != g_csgo.m_engine()->IsInGame() || last_setting != g_cfg.esp.nightmode || last_transparency != g_cfg.esp.prop_transparency || last_darkness != g_cfg.esp.darkness)
	{
		last_setting = g_cfg.esp.nightmode;
		last_transparency = g_cfg.esp.prop_transparency;
		last_darkness = g_cfg.esp.darkness;
		was_ingame = g_csgo.m_engine()->IsInGame();
		done = false;
	}
}

































