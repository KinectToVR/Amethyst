#pragma once
#include <map>
#include <string>

/* The locale maps */

// Map each status to all available locales
inline std::map<std::wstring, std::wstring> status_ok_map
{
	{
		L"en-US",
		L"Success!\nS_OK\nEverything's good!"
	},
	{
		L"lc-LC",
		L"Shucshesh!\nS_OK\nEvewytshin's gud!"
	}
};

inline std::map<std::wstring, std::wstring> status_unavailable_map
{
	{
		L"en-US",
		L"Sensor Unavailable!\nE_NOTAVAILABLE\nCheck if the Kinect is plugged in to your PC's USB and power plugs."
	},
	{
		L"lc-LC",
		L"Shenshor Unvable!\nE_NOTAVAILABLE\nCshecsk if de K'nect's pwuggwed into y'r PC's UShB and powew pwugwsh."
	}
};

/* Helper functions */

// Get the current use language, e.g. en-US
inline std::wstring GetUserLocale()
{
	wchar_t localeName[LOCALE_NAME_MAX_LENGTH] = { 0 };
	return GetUserDefaultLocaleName(
		localeName, sizeof(localeName) / sizeof(*(localeName))) == 0
		? std::wstring()
		: localeName;
}

// Get the current status string (but localized)
inline std::wstring GetLocalizedStatusWString(
	std::wstring locale, std::map<std::wstring, std::wstring> status_map)
{
	return status_map.contains(locale)
		       ? status_map[locale]
		       : status_map[L"en-US"];
}

// Get the current status string (but localized)
inline std::wstring GetLocalizedStatusWStringAutomatic(
	std::map<std::wstring, std::wstring> status_map)
{
	return status_map.contains(GetUserLocale())
		       ? status_map[GetUserLocale()]
		       : status_map[L"en-US"];
}
