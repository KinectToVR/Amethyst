#pragma once
#include "pch.h"

inline std::wstring wstring_cast(std::string const& s)
{
	return std::wstring(s.begin(), s.end());
}

inline std::array<std::string, 3> split_status(std::string const& s)
{
	// If there are 3 strings separated by \n
	return std::array<std::string, 3>{
		s.substr(0, s.find("\n")),
		s.substr(s.find("\n") + 1, s.rfind("\n") - (s.find("\n") + 1)),
		s.substr(s.rfind("\n") + 1)
	};
}

namespace TrackingDevices
{
	// Vector of currently available tracking devices
	// std::variant cause there are 3 possible device types
	inline std::vector<
		std::variant<
			ktvr::TrackingDeviceBase_KinectBasis*,
			ktvr::TrackingDeviceBase_JointsBasis*,
			ktvr::TrackingDeviceBase_OnlyOverride*>>
	TrackingDevicesVector;

	// Pointer to the device's constructing function
	typedef void* (*TrackingDeviceBaseFactory)(const char* pVersionName, int* pReturnCode);
}
