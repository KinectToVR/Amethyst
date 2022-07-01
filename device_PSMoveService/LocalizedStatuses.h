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
	},
	{
		L"fr-FR",
		L"Succès!\nS_OK\nTout fonctionne!"
	},
	{
		L"ru-RU",
		L"Успешно!\nS_OK\nВсе работает!"
	},
	{
		L"de-DE",
		L"Erfolg!\nS_OK\nAlles ist gut!"
	}
};

inline std::map<std::wstring, std::wstring> status_not_running_map
{
	{
		L"en-US",
		L"Connection error!\nE_PSMS_NOT_RUNNING\nCheck if PSMoveService is running, working and accessible by clients."
	},
	{
		L"lc-LC",
		L"Connecschtion ewow!\nE_PSMS_NOT_RUNNING\nChsechk f PShMoveShervish ish wunniwng, wowkinwg n acshcshesshsshibwe b clientsh."
	},
	{
		L"fr-FR",
		L"Erreur de connexion!\nE_PSMS_NOT_RUNNING\nVérifiez si PSMoveService est bien lancé, fonctionne et est accessible aux clients."
	},
	{
		L"ru-RU",
		L"Ошибка подключения!\nE_PSMS_NOT_RUNNING\nУбедитесь, что PSMoveService запущен и доступен для подключения клиентами."
	},
	{
		L"de-DE",
		L"Verbindungsfehler!\nE_PSMS_NOT_RUNNING\nÜberprüfe, ob PSMoveService ausgeführt wird, funktioniert und für Clients zugänglich ist."
	}
};

/* Helper functions */

// Get the current use language, e.g. en-US
inline std::wstring GetUserLocale()
{
	wchar_t localeName[LOCALE_NAME_MAX_LENGTH] = {0};
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
