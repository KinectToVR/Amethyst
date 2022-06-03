﻿#pragma once
#include <pch.h>
#include <map>
#include <string>

/* The locale maps */

// Map each status to all available locales
inline std::map<std::wstring, std::wstring> status_ok_map
{
	{
		L"en-US",
		L"Success! (Code 1)\nI_OK\nEverything's good!"
	},
	{
		L"lc-LC",
		L"Wohoo! (Code 1)\nI_OK\nAll gud!"
	}
};

inline std::map<std::wstring, std::wstring> status_wtf_map
{
	{
		L"en-US",
		L"COULD NOT CHECK STATUS (Code -12)\nE_WTF\nSomething's fucked a really big time."
	},
	{
		L"lc-LC",
		L"WHERE SHTATUS (Code -12)\nE_WTF\nSomethings wucksed big big."
	}
};

inline std::map<std::wstring, std::wstring> status_exception_map
{
	{
		L"en-US",
		L"EXCEPTION WHILE CHECKING (Code -10)\nE_EXCEPTION_WHILE_CHECKING\nCheck SteamVR add-ons (NOT overlays) and enable Amethyst."
	},
	{
		L"lc-LC",
		L"CULD NO CHECK WUUU (Code -10)\nE_EXCEPTION_WHILE_CHECKING\nChecsk iw Amethysht wishibwe in ShteamVR n wabwwe it if nay."
	}
};

inline std::map<std::wstring, std::wstring> status_connection_error_map
{
	{
		L"en-US",
		L"SERVER CONNECTION ERROR (Code -1)\nE_CONNECTION_ERROR\nYour Amethyst SteamVR driver may be broken or outdated."
	},
	{
		L"lc-LC",
		L"NO KINNECSHTION (Code -1)\nE_CONNECTION_ERROR\nY'r Amethysht ShteamVR diver jump'd into a fwozen pool, oww."
	}
};

inline std::map<std::wstring, std::wstring> status_server_failure_map
{
	{
		L"en-US",
		L"FATAL SERVER FAILURE (Code 10)\nE_FATAL_SERVER_FAILURE\nPlease restart, check logs and write to us on Discord."
	},
	{
		L"lc-LC",
		L"SOMETHINS KINDA NOT WIGHWT (Code 10)\nE_FATAL_SERVER_FAILURE\nReshtart n den cry, u do nothinnn."
	}
};

inline std::map<std::wstring, std::wstring> status_api_failure_map
{
	{
		L"en-US",
		L"COULD NOT CONNECT TO K2API (Code -11)\nE_K2API_FAILURE\nThis error shouldn't occur, actually. Something's wrong a big part."
	},
	{
		L"lc-LC",
		L"WHERE K2API (Code -11)\nE_K2API_FAILURE\nHoww d'd y even caus this ewwow, YUU CHEETAHHH!"
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
