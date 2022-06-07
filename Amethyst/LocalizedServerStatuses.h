#pragma once
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
	},
	{
		L"fr-FR",
		L"Succès! (Code 1)\nI_OK\nTout fonctionne!"
	},
	{
		L"ru-RU",
		L"Успешно! (Код 1)\nI_OK\nВсе работает!"
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
		L"CULD NO CHECK AAAAA (Code -12)\nE_WTF\nShomethinwsh fucksed wucksewd a big big pawwt"
	},
	{
		L"fr-FR",
		L"STATUT INACCESSIBLE (Code -12)\nE_WTF\nQuelque chose de grave s'est produit."
	},
	{
		L"ru-RU",
		L"НЕВОЗМОЖНО ПРОВЕРИТЬ СТАТУС (Код -12)\nE_WTF\nЧто-то пошло очень не так на этот раз."
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
	},
	{
		L"fr-FR",
		L"UNE EXCEPTION EST SURVENUE (Code -10)\nE_EXCEPTION_WHILE_CHECKING\nVérifiez qu'Amethyst est activé dans les extensions (PAS les overlays) de SteamVR."
	},
	{
		L"ru-RU",
		L"ИСКЛЮЧЕНИЕ ПРИ ПРОВЕРКЕ (Код -10)\nE_EXCEPTION_WHILE_CHECKING\nПроверьте модули (не оверлеи) SteamVR и включите Amethyst."
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
	},
	{
		L"fr-FR",
		L"ERREUR DE CONNEXION (Code -1)\nE_CONNECTION_ERROR\nLa version de L'extension SteamVR n'est probablement pas à jour."
	},
	{
		L"ru-RU",
		L"ОШИБКА ПОДКЛЮЧЕНИЯ К СЕРВЕРУ (Код -1)\nE_CONNECTION_ERROR\nВозможно ваша версия драйвера Amethyst для SteamVR устарела или повреждена."
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
	},
	{
		L"fr-FR",
		L"ERREUR FATALE (Code 10)\nE_FATAL_SERVER_FAILURE\nRelancez Amethyst. Si le problème persiste contactez-nous sur Discord."
	},
	{
		L"ru-RU",
		L"КРИТИЧЕСКАЯ ОШИБКА СЕРВЕРА (Код 10)\nE_FATAL_SERVER_FAILURE\nПожалуйста перезагрузите Amethyst, проверьте журнал событий и напишите нам в Discord."
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
	},
	{
		L"fr-FR",
		L"CONNEXION A K4API IMPOSSIBLE (Code -11)\nE_K2API_FAILURE\nÇa devrait vraiment pas arriver. En fait, quelque chose est foutu grave."
	},
	{
		L"ru-RU",
		L"НЕВОЗМОЖНО ПОДКЛЮЧИТЬСЯ К K2API (Код -11)\nE_K2API_FAILURE\nЭтой ошибки вообще не должно быть. Что-то совсем все плохо."
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
