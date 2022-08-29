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
	inline std::string GetK2AppDataFileDir(const std::string& relativeFilePath)
	{
		CreateDirectoryA(std::string(std::string(std::getenv("APPDATA")) + "\\Amethyst\\").c_str(),
		                 nullptr);
		return std::string(std::getenv("APPDATA")) + "\\Amethyst\\" + relativeFilePath;
	}

	// Get file location in AppData
	inline std::string GetK2AppDataLogFileDir(const std::string& relativeFilePath)
	{
		CreateDirectoryA(std::string(std::string(std::getenv("APPDATA")) + "\\Amethyst\\logs\\").c_str(),
		                 nullptr);
		return std::string(std::getenv("APPDATA")) + "\\Amethyst\\logs\\" + relativeFilePath;
	}
}
