#pragma once
#include <Windows.h>
#include <string>

/*
 * AME_API Paths
 *
 * This is a separate header because we won't need linking
 * & doing much more stuff for nothing, just gonna include
 * this single header + WinAPI as you can see in #includes
 *
 */

namespace ktvr
{
	// Get file location in AppData
	inline std::wstring GetK2AppDataFileDir(const std::wstring& relativeFilePath)
	{
		CreateDirectory(std::wstring(std::wstring(_wgetenv(L"APPDATA")) +
			                L"\\Amethyst\\").c_str(), nullptr);
		return std::wstring(_wgetenv(L"APPDATA")) + L"\\Amethyst\\" + relativeFilePath;
	}

	// Get file location in AppData
	inline std::wstring GetK2AppDataLogFileDir(const std::wstring& relativeFilePath)
	{
		CreateDirectory(std::wstring(std::wstring(_wgetenv(L"APPDATA")) +
			                L"\\Amethyst\\logs\\").c_str(), nullptr);
		return std::wstring(_wgetenv(L"APPDATA")) + L"\\Amethyst\\logs\\" + relativeFilePath;
	}
}
