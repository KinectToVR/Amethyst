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
	}
};

inline std::map<std::wstring, std::wstring> status_initializing_map
{
	{
		L"en-US",
		L"INITIALIZING\nS_NUI_INITIALIZING\nThe device is connected, but still initializing."
	},
	{
		L"lc-LC",
		L"INITIALIZING\nS_NUI_INITIALIZING\nDe wevice ish connectd, but shtil intiawiziwg."
	},
	{
		L"fr-FR",
		L"INITIALIZING\nS_NUI_INITIALIZING\nL'appareil est connecté, mais encore en initialisation."
	}
};

inline std::map<std::wstring, std::wstring> status_not_connected_map
{
	{
		L"en-US",
		L"NOTCONNECTED\nE_NUI_NOTCONNECTED\nThe device is not connected."
	},
	{
		L"lc-LC",
		L"NOTCONNECTED\nE_NUI_NOTCONNECTED\nDe wevice ish nat cowwecshtd."
	},
	{
		L"fr-FR",
		L"NOTCONNECTED\nE_NUI_NOTCONNECTED\nL'appareil n'est pas connecté."
	}
};

inline std::map<std::wstring, std::wstring> status_not_genuine_map
{
	{
		L"en-US",
		L"NOTGENUINE\nE_NUI_NOTGENUINE\nThe SDK is reporting the Kinect as invalid."
	},
	{
		L"lc-LC",
		L"NOTGUINEA\nE_NUI_NOTGUINEA\nDe ESSDEKAY sayz K'nect ish nat veeld."
	},
	{
		L"fr-FR",
		L"NOTGENUINE\nE_NUI_NOTGENUINE\nLe SDK reporte la Kinect comme étant invalide."
	}
};

inline std::map<std::wstring, std::wstring> status_not_supported_map
{
	{
		L"en-US",
		L"NOTSUPPORTED\nE_NUI_NOTSUPPORTED\nThe device is unsupported. (Xbox Kinect requires the SDK)"
	},
	{
		L"lc-LC",
		L"NOTSHUPPOWTD\nE_NUI_NOTSHUPPOWTD\nDe w'vice ish unshuppowted. (Ecksbawk K'nect wequiez de ESSDEKAY)"
	},
	{
		L"fr-FR",
		L"NOTSUPPORTED\nE_NUI_NOTSUPPORTED\nL'appareil n'est pas supporté. (Kinect pour Xbox requiert le SDK)"
	}
};

inline std::map<std::wstring, std::wstring> status_insufficient_bandwidth_map
{
	{
		L"en-US",
		L"INSUFFICIENTBANDWIDTH\nE_NUI_INSUFFICIENTBANDWIDTH\nThe device is connected to a hub without the necessary bandwidth requirements."
	},
	{
		L"lc-LC",
		L"INSUFFICIENTBANDWIDTH\nE_NUI_INSUFFICIENTBANDWIDTH\nDe wev'ce s wonwectd t hab w/ot e necshesshawy bandwidtsch wequiwmentsh."
	},
	{
		L"fr-FR",
		L"INSUFFICIENTBANDWIDTH\nE_NUI_INSUFFICIENTBANDWIDTH\nL'appareil est connecté a un hub qui n'offre pas pas la bande passante nécessaire."
	}
};

inline std::map<std::wstring, std::wstring> status_not_powered_map
{
	{
		L"en-US",
		L"NOTPOWERED\nE_NUI_NOTPOWERED\nThere is either a problem with your adapter/cables or with the Kinect device driver registration in Windows."
	},
	{
		L"lc-LC",
		L"NOTPOWERED\nE_NUI_NOTPOWERED\nWre s withew a pwobwem wif y'r awaptew/wabwes o witw e K'nect d'v'ce dwivew wegistwatiown in Windowows."
	},
	{
		L"fr-FR",
		L"NOTPOWERED\nE_NUI_NOTPOWERED\nIl y a un problème soit avec votre adaptateur/cables, ou avec les pilotes de la Kinect dans Windows."
	}
};

inline std::map<std::wstring, std::wstring> status_not_ready_map
{
	{
		L"en-US",
		L"NOTREADY\nE_NUI_NOTREADY\nThere was some other unspecified error."
	},
	{
		L"lc-LC",
		L"NOTREADY\nE_NUI_NOTREADY\nWeadwy.. Shteadwy.. Oh nayyyy!."
	},
	{
		L"fr-FR",
		L"NOTREADY\nE_NUI_NOTREADY\nUne erreur non spécifiée est survenue."
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
