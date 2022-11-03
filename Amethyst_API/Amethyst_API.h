#pragma once
#include <pch.h>

#include "Amethyst_API_Devices.h"
#include "Amethyst_API_Paths.h"

namespace ktvr
{
	// Interface Version
	static const char* IAME_API_Version = "IAME_API_Version_020";

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
	/// AME_API Server handles for RPC calls
	/// </summary>
	static inline std::unique_ptr<IK2DriverService::Stub> stub;
	static inline std::shared_ptr<grpc::Channel> channel;
	
	/**
	 * \brief Connects socket object to selected port, AME uses 7135
	 * \return 0: OK, -1: Exception, -2: Channel fail, -3: Stub failure
	 */
	KTVR_API int init_ame_api(const int& port = 7135) noexcept;

	KTVR_API std::vector<std::pair<ITrackerType, bool>>
	update_tracker_state_vector_r(const std::vector<std::pair<ITrackerType, bool>>&) noexcept;
	KTVR_API std::monostate
	update_tracker_state_vector_n(const std::vector<std::pair<ITrackerType, bool>>&) noexcept;

	KTVR_API std::vector<std::pair<ITrackerType, bool>>
	update_tracker_vector_r(const std::vector<K2TrackerBase>&) noexcept;
	KTVR_API std::monostate
	update_tracker_vector_n(const std::vector<K2TrackerBase>&) noexcept;

	KTVR_API std::vector<std::pair<ITrackerType, bool>>
	refresh_tracker_pose_vector_r(const std::vector<ITrackerType>&) noexcept;
	KTVR_API std::monostate
	refresh_tracker_pose_vector_n(const std::vector<ITrackerType>&) noexcept;

	KTVR_API std::pair<ITrackerType, bool> request_vr_restart_r(const std::string&) noexcept;
	KTVR_API std::monostate request_vr_restart_n(const std::string&) noexcept;

	/**
	 * \brief Update trackers' state in SteamVR driver
	 * \param status_pairs New statuses for trackers
	 * \return Returns tracker role / success?
	 */
	template <bool WantReply = false>
	std::conditional_t<WantReply, std::vector<std::pair<ITrackerType, bool>>, std::monostate>
	update_tracker_state_vector(const std::vector<std::pair<ITrackerType, bool>>& status_pairs) noexcept
	{
		if constexpr (WantReply) return update_tracker_state_vector_r(status_pairs);
		else return update_tracker_state_vector_n(status_pairs); // std::monostate
	}

	/**
	 * \brief Update tracker's pose and data in SteamVR driver
	 * \param tracker_bases New bases for trackers
	 * \return Returns tracker role / success?
	 */
	template <bool WantReply = false>
	std::conditional_t<WantReply, std::vector<std::pair<ITrackerType, bool>>, std::monostate>
	update_tracker_vector(const std::vector<K2TrackerBase>& tracker_bases) noexcept
	{
		if constexpr (WantReply) return update_tracker_vector_r(tracker_bases);
		else return update_tracker_vector_n(tracker_bases); // std::monostate
	}

	/**
	 * \brief Update tracker's pose in SteamVR driver with already existing values
	 * \param trackers Tracker for updating data
	 * \argument WantReply Check if the client wants a reply
	 * \return Returns tracker id / success?
	 */
	template <bool WantReply = true>
	std::conditional_t<WantReply, std::vector<std::pair<ITrackerType, bool>>, std::monostate>
	refresh_tracker_pose_vector(const std::vector<ITrackerType>& trackers) noexcept
	{
		if constexpr (WantReply) return refresh_tracker_pose_vector_r(trackers);
		else return refresh_tracker_pose_vector_n(trackers); // std::monostate
	}

	/**
	 * \brief Request OpenVR to restart with a message
	 * \param reason Reason for the restart
	 * \argument WantReply Check if the client wants a reply
	 * \return Returns tracker id / success?
	 */
	template <bool WantReply = true>
	std::conditional_t<WantReply, std::pair<ITrackerType, bool>, std::monostate>
	request_vr_restart(const std::string& reason) noexcept
	{
		if constexpr (WantReply) return request_vr_restart_r(reason);
		else return request_vr_restart_n(reason); // std::monostate
	}

	/**
	 * \brief Test connection with the server
	 * \return Returns ok? / send_time / receive_time / elapsed_time (now-send)
	 */
	KTVR_API std::tuple<bool, long long, long long, long long> test_connection() noexcept;
}
