#pragma once
#include <iostream>
#include <Windows.h>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>
#include <string>
#include <chrono>
#include <cmath>
#include <optional>

#include "Amethyst_API.pb.h"

#include "Amethyst_API_Devices.h"
#include "Amethyst_API_Paths.h"

#define AME_API_GET_TIMESTAMP_NOW \
	std::chrono::time_point_cast<std::chrono::microseconds>	\
	(std::chrono::system_clock::now()).time_since_epoch().count()

namespace ktvr
{
	// Interface Version
	static const char* IAME_API_Version = "IAME_API_Version_015";

	// Check Eigen quaternions
	template <typename _Scalar>
	Eigen::Quaternion<_Scalar> quaternion_normal(const K2Quaternion& q)
	{
		return Eigen::Quaternion<_Scalar>(
			std::isnormal(q.w()) ? q.w() : 1.f,
			std::isnormal(q.x()) ? q.x() : 0.f,
			std::isnormal(q.y()) ? q.y() : 0.f,
			std::isnormal(q.z()) ? q.z() : 0.f
		);
	}

	// Check Eigen Vectors
	template <typename _Scalar>
	Eigen::Vector3<_Scalar> vector3_normal(const K2Vector3& v)
	{
		return Eigen::Vector3<_Scalar>(
			std::isnormal(v.x()) ? v.x() : 0.f,
			std::isnormal(v.y()) ? v.y() : 0.f,
			std::isnormal(v.z()) ? v.z() : 0.f
		);
	}

	// Check Eigen quaternions
	template <typename _Scalar>
	Eigen::Quaternion<_Scalar> quaternion_normal(const Eigen::Quaternion<_Scalar>& q)
	{
		return Eigen::Quaternion<_Scalar>(
			std::isnormal(q.w()) ? q.w() : 1.f,
			std::isnormal(q.x()) ? q.x() : 0.f,
			std::isnormal(q.y()) ? q.y() : 0.f,
			std::isnormal(q.z()) ? q.z() : 0.f
			);
	}

	// Check Eigen Vectors
	template <typename _Scalar>
	Eigen::Vector3<_Scalar> vector3_normal(const Eigen::Vector3<_Scalar>& v)
	{
		return Eigen::Vector3<_Scalar>(
			std::isnormal(v.x()) ? v.x() : 0.f,
			std::isnormal(v.y()) ? v.y() : 0.f,
			std::isnormal(v.z()) ? v.z() : 0.f
			);
	}

	/// <summary>
	/// AME_API Semaphore handles for WINAPI calls
	/// </summary>
	inline HANDLE ame_api_to_semaphore,
	              ame_api_from_semaphore,
	              ame_api_start_semaphore;

	/// <summary>
	/// AME_API's last error string, check for empty
	/// </summary>
	inline std::string ame_api_last_error;

	/// <summary>
	/// Get AME_API's last error string
	/// </summary>
	inline std::string GetLastError() { return ame_api_last_error; }

	/// <summary>
	/// AME_API Pipe handle addresses for WINAPI calls
	/// </summary>
	inline std::wstring
		ame_api_to_pipe_address = L"\\\\.\\pipe\\ame_api_amethyst_to_pipe" + StringToWString(IAME_API_Version),
		ame_api_from_pipe_address = L"\\\\.\\pipe\\ame_api_amethyst_from_pipe" + StringToWString(IAME_API_Version),
		ame_api_to_semaphore_address = L"Global\\ame_api_amethyst_to_semaphore" + StringToWString(IAME_API_Version),
		ame_api_from_semaphore_address = L"Global\\ame_api_amethyst_from_semaphore" + StringToWString(IAME_API_Version),
		ame_api_start_semaphore_address = L"Global\\ame_api_amethyst_start_semaphore" + StringToWString(
			IAME_API_Version);

	/**
	 * \brief Connects socket object to selected port, K2 uses 7135
	 * \return Returns 0 for success and -1 for failure
	 */
	KTVR_API int init_ame_api(
		const std::wstring& k2_to_pipe = ame_api_to_pipe_address,
		const std::wstring& k2_from_pipe = ame_api_from_pipe_address,
		const std::wstring& k2_to_sem = ame_api_to_semaphore_address,
		const std::wstring& k2_from_sem = ame_api_from_semaphore_address,
		const std::wstring& k2_start_sem = ame_api_start_semaphore_address) noexcept;

	/**
	 * \brief Disconnects socket object from port
	 * \return Returns 0 for success and -1 for failure
	 */
	KTVR_API int close_ame_api() noexcept;

	/**
	 * \brief Send message and get a server reply, there is no need to decode return
	 * \param data String which is to be sent
	 * \param want_reply Check if the client wants a reply
	 * \return Returns server's reply to the message
	 */
	KTVR_API std::string send_message(const std::string& data, bool want_reply = true) noexcept(false);

	// External functions for the template below
	KTVR_API std::monostate send_message_no_reply(K2Message message);
	KTVR_API K2ResponseMessage send_message_want_reply(K2Message message);

	/**
	 * \brief Send message and get a server reply
	 * \param message Message which is to be sent
	 * \argument want_reply Check if the client wants a reply
	 * \return Returns server's reply to the message
	 */
	template <bool want_reply = true>
	std::conditional_t<want_reply, K2ResponseMessage, std::monostate>
	send_message(K2Message message) noexcept(false)
	{
		if constexpr (want_reply)
			return send_message_want_reply(std::move(message));
		else
			return send_message_no_reply(std::move(message));
	}

	/**
	 * \brief Connect (activate/spawn) tracker in SteamVR
	 * \param tracker Tracker's role which is to connect
	 * \param state Tracker's state to be set
	 * \argument want_reply Check if the client wants a reply
	 * \return Returns tracker id / success?
	 */
	template <bool want_reply = true>
	std::conditional_t<want_reply, K2ResponseMessage, std::monostate>
	set_tracker_state(const ITrackerType tracker, const bool state) noexcept
	{
		try
		{
			auto message = K2Message();

			message.set_messagetype(K2Message_SetTrackerState);
			message.set_tracker(tracker);
			message.set_state(state);

			// Send and grab the response
			return send_message<want_reply>(message);
		}
		catch (const std::exception& e)
		{
			if constexpr (want_reply) return K2ResponseMessage(); // Success is set to false by default
			else return std::monostate();
		}
	}

	/**
	 * \brief Connect (activate/spawn) all trackers in SteamVR
	 * \param state Tracker's state to be set
	 * \argument want_reply Check if the client wants a reply
	 * \return Returns success?
	 */
	template <bool want_reply = true>
	std::conditional_t<want_reply, K2ResponseMessage, std::monostate>
	set_state_all(bool state) noexcept
	{
		try
		{
			auto message = K2Message();

			message.set_messagetype(K2Message_SetStateAll);
			message.set_state(state);

			// Send and grab the response
			return send_message<want_reply>(message);
		}
		catch (const std::exception& e)
		{
			if constexpr (want_reply) return K2ResponseMessage(); // Success is set to false by default
			else return std::monostate();
		}
	}

	/**
	 * \brief Update tracker's pose and data in SteamVR driver
	 * \param tracker_bases New bases for trackers
	 * \return Returns tracker role / success?
	 */
	template <bool want_reply = false>
	std::conditional_t<want_reply, K2ResponseMessage, std::monostate>
	update_tracker_vector(std::vector<K2TrackerBase> tracker_bases) noexcept
	{
		try
		{
			auto message = K2Message();

			message.set_messagetype(K2Message_UpdateTrackerPoseVector);
			message.mutable_trackerbasevector()->Add(tracker_bases.begin(), tracker_bases.end());

			// Send and grab the response
			return send_message<want_reply>(message);
		}
		catch (const std::exception& e)
		{
			if constexpr (want_reply) return K2ResponseMessage(); // Success is set to false by default
			else return std::monostate();
		}
	}


	/**
	 * \brief Update trackers' state in SteamVR driver
	 * \param status_pairs New statuses for trackers
	 * \return Returns tracker role / success?
	 */
	template <bool want_reply = false>
	std::conditional_t<want_reply, K2ResponseMessage, std::monostate>
	update_tracker_state_vector(const std::vector<std::pair<ITrackerType, bool>>& status_pairs) noexcept
	{
		try
		{
			auto message = K2Message();

			message.set_messagetype(K2Message_SetTrackerStateVector);

			// Compose a vector of pairs
			std::vector<K2StatusPair> pairs;
			for (const auto& [tracker, status] : status_pairs)
			{
				auto pair = K2StatusPair();
				pair.set_tracker(tracker);
				pair.set_status(status);

				pairs.push_back(pair);
			}

			// Send the vector to the server driver
			message.mutable_trackerstatusesvector()->Add(pairs.begin(), pairs.end());

			// Send and grab the response
			return send_message<want_reply>(message);
		}
		catch (const std::exception& e)
		{
			if constexpr (want_reply) return K2ResponseMessage(); // Success is set to false by default
			else return std::monostate();
		}
	}

	/**
	 * \brief Update tracker's pose in SteamVR driver with already existing values
	 * \param tracker Tracker for updating data
	 * \argument want_reply Check if the client wants a reply
	 * \return Returns tracker id / success?
	 */
	template <bool want_reply = true>
	std::conditional_t<want_reply, K2ResponseMessage, std::monostate>
	refresh_tracker_pose(const ITrackerType tracker) noexcept
	{
		try
		{
			auto message = K2Message();

			message.set_messagetype(K2Message_RefreshTracker);
			message.set_tracker(tracker);

			// Send and grab the response
			return send_message<want_reply>(message);
		}
		catch (const std::exception& e)
		{
			if constexpr (want_reply) return K2ResponseMessage(); // Success is set to false by default
			else return std::monostate();
		}
	}

	/**
	 * \brief Request OpenVR to restart with a message
	 * \param reason Reason for the restart
	 * \argument want_reply Check if the client wants a reply
	 * \return Returns tracker id / success?
	 */
	template <bool want_reply = true>
	std::conditional_t<want_reply, K2ResponseMessage, std::monostate>
	request_vr_restart(const std::string& reason) noexcept
	{
		try
		{
			auto message = K2Message();

			message.set_messagetype(K2Message_RequestRestart);
			message.set_message_string(reason);

			// Send and grab the response
			return send_message<want_reply>(message);
		}
		catch (const std::exception& e)
		{
			if constexpr (want_reply) return K2ResponseMessage(); // Success is set to false by default
			else return std::monostate();
		}
	}

	/**
	 * \brief Test connection with the server
	 * \return Returns send_time / total_time / success?
	 */
	KTVR_API std::tuple<K2ResponseMessage, long long, long long> test_connection() noexcept;
}
