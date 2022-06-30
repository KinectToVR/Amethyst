#pragma once
#include <map>
#include <string>

/* The locale maps */

// Map each status to all available locales
inline std::map<std::wstring, std::wstring> dim_lights_label_map
{
	{
		L"en-US",
		L"If you're using PSMS for rotation tracking only, you can dim controller LED lights to save power."
	},
	{
		L"lc-LC",
		L"Iw uww ushin PSMS fow wotatiown twackwing onwy, u cn dmm contwolew WED wighwts 2 sav powaa."
	}
};
