#pragma once
#include <map>
#include <string>

/* The locale maps */

// Map each status to all available locales
inline std::map<std::wstring, std::wstring> elevation_angle_label_map
{
	{
		L"en-US",
		L"Kinect elevation angle:"
	},
	{
		L"lc-LC",
		L"K'nect ewevatiown angww:"
	}
};
