#pragma once
#include <iostream>
#include <Windows.h>
#include <optional>
#include <chrono>
#include "K2Tracker.h"
#include <Amethyst_API.h>
#include <semaphore>

inline std::binary_semaphore smphFrameUpdate{0};

class K2ServerDriver
{
public:
	// Parse a message from K2API
	void parse_message(const ktvr::K2Message& message);
	bool _isActive = false; // Server status

	// IPC things to work properly
	//Global Handle for Semaphore
	HANDLE k2api_to_Semaphore,
	       k2api_from_Semaphore,
	       k2api_start_Semaphore;

	[[nodiscard]] int init_ServerDriver(
		const std::wstring& ame_api_to_pipe = ktvr::ame_api_to_pipe_address,
		const std::wstring& ame_api_from_pipe = ktvr::ame_api_from_pipe_address,
		const std::wstring& ame_api_to_sem = ktvr::ame_api_to_semaphore_address,
		const std::wstring& ame_api_from_sem = ktvr::ame_api_from_semaphore_address,
		const std::wstring& ame_api_start_sem = ktvr::ame_api_start_semaphore_address);
	void setActive(bool m_isActive) { _isActive = m_isActive; }

	// Value should not be discarded, it'd be useless
	[[nodiscard]] bool isActive() const { return _isActive; }

	// Tracker vector
	std::vector<K2Tracker> trackerVector;
};
