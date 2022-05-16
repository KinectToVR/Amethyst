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

#include <Eigen/Dense>

#include <boost/optional.hpp>

#include "Amethyst_API_Devices.h"
#include "Amethyst_API_Paths.h"

/*
 * Default IPC defines are:
 *
 * \\\\.\\pipe\\k2api_ame_to_pipe
 * \\\\.\\pipe\\k2api_ame_from_pipe
 *
 * Global\\k2api_ame_to_sem
 * Global\\k2api_ame_from_sem
 * Global\\k2api_ame_start_sem
 */

#define K2API_GET_TIMESTAMP_NOW \
	std::chrono::time_point_cast<std::chrono::microseconds>	\
	(std::chrono::system_clock::now()).time_since_epoch().count()

#ifdef Amethyst_API_EXPORTS
#define KTVR_API __declspec(dllexport)
#else
#define KTVR_API __declspec(dllimport)
#endif

namespace ktvr
{
	// Interace Version
	static const char* IK2API_Version = "IK2API_Version_007";

	// OpenVR Tracker types
	enum class ITrackerType
	{
		Tracker_Handed,
		Tracker_LeftFoot,
		Tracker_RightFoot,
		Tracker_LeftShoulder,
		Tracker_RightShoulder,
		Tracker_LeftElbow,
		Tracker_RightElbow,
		Tracker_LeftKnee,
		Tracker_RightKnee,
		Tracker_Waist,
		Tracker_Chest,
		Tracker_Camera,
		Tracker_Keyboard
	};

	// K2API messaging types
	enum class K2MessageType
	{
		// Default
		K2Message_Invalid,
		// Add
		K2Message_AddTracker,
		// State
		K2Message_SetTrackerState,
		K2Message_SetStateAll,
		// Update
		K2Message_UpdateTrackerPose,
		K2Message_UpdateTrackerData,
		// Update but multiple at once
		K2Message_UpdateTrackerVector,
		// Get tracker
		K2Message_DownloadTracker,
		// Refresh tracker pose
		K2Message_RefreshTracker,
		// Request a restart
		K2Message_RequestRestart,
		// Test message
		K2Message_Ping
	};

	// Return messaging types
	enum class K2ResponseMessageType
	{
		K2ResponseMessage_Invalid,
		// Default
		K2ResponseMessage_ID,
		// Just the ID
		K2ResponseMessage_Success,
		// State success, type only for ping
		K2ResponseMessage_Tracker // Tracker object, includes ID too!
	};

	// Return messaging types
	enum class K2ResponseMessageCode
	{
		K2ResponseMessageCode_Exception = -10,
		// Exception occurred
		K2ResponseMessageCode_UnknownError = -1,
		// IDK
		K2ResponseMessageCode_Invalid = 0,
		// Default Invalid
		K2ResponseMessageCode_OK = 1,
		// Default OK
		K2ResponseMessageCode_SpawnFailed = 2,
		// Spawn failed, exception
		K2ResponseMessageCode_AlreadyPresent = 3,
		// Serial already present
		K2ResponseMessageCode_BadRequest = 4,
		// Unknown message type, wrong ID
		K2ResponseMessageCode_ParsingError = 5,
		// Global parsing exception
		K2ResponseMessageCode_BadSerial = 6 // Serial was empty
	};

	// Alias for code readability
	typedef int JointTrackingState, MessageType, MessageCode;

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

	// Pose (pos & rot) class for a spanned tracker
	class K2TrackerPose
	{
	public:
		// Tracker should be centered automatically
		Eigen::Quaternionf orientation = Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f);
		Eigen::Vector3f position = Eigen::Vector3f(0.f, 0.f, 0.f);

		template <class Archive>
		KTVR_API void serialize(Archive& ar, unsigned int version);

		// Default constructors
		K2TrackerPose() = default;
		~K2TrackerPose() = default;

		// Copy constructors
		K2TrackerPose(const K2TrackerPose&) = default;
		K2TrackerPose& operator=(const K2TrackerPose&) = default;

		// Move operators
		K2TrackerPose(K2TrackerPose&&) = default;
		K2TrackerPose& operator=(K2TrackerPose&&) = default;

		// Quick constructor
		K2TrackerPose(Eigen::Quaternionf m_orientation, Eigen::Vector3f m_position) :
			orientation(std::move(m_orientation)), position(std::move(m_position))
		{
		}
	};

	// Assuming not autoadded by default
	// To autoadd set isActive to true
	// Data packet, as known in k2api
	class K2TrackerData
	{
	public:
		std::string serial; // Must be set manually
		uint32_t role = 0; // Handed Tracker
		bool isActive = false;

		template <class Archive>
		KTVR_API void serialize(Archive& ar, unsigned int version);

		// Default constructors
		K2TrackerData() = default;
		~K2TrackerData() = default;

		// Copy constructors
		K2TrackerData(const K2TrackerData&) = default;
		K2TrackerData& operator=(const K2TrackerData&) = default;

		// Move operators
		K2TrackerData(K2TrackerData&&) = default;
		K2TrackerData& operator=(K2TrackerData&&) = default;

		// Quick constructor
		K2TrackerData(std::string m_serial, ITrackerType m_role, bool m_isActive = false) :
			serial(std::move(m_serial)), role(static_cast<uint32_t>(m_role)), isActive(m_isActive)
		{
		}
	};

	class K2PosePacket : public K2TrackerPose
	{
	public:
		double millisFromNow = 0; // Time offset after sending

		K2PosePacket() = default;
		~K2PosePacket() = default;

		// Copy constructors
		K2PosePacket(const K2PosePacket&) = default;
		K2PosePacket& operator=(const K2PosePacket&) = default;

		// Move operators
		K2PosePacket(K2PosePacket&& packet) noexcept : K2TrackerPose(packet)
		{
		}

		K2PosePacket& operator=(K2PosePacket&& packet) noexcept
		{
			K2TrackerPose::operator=(packet);
			return *this;
		}

		// Default constructor 2
		K2PosePacket(const K2TrackerPose& m_pose, const int& millis) :
			K2TrackerPose(m_pose), millisFromNow(millis)
		{
		}

		// Default constructor
		K2PosePacket(const K2TrackerPose& m_pose) :
			K2TrackerPose(m_pose)
		{
		}

		template <class Archive>
		KTVR_API void serialize(Archive& ar, unsigned int version);
	};

	class K2DataPacket : public K2TrackerData
	{
	public:
		double millisFromNow = 0; // Time offset after sending

		// Default constructors
		K2DataPacket() = default;
		~K2DataPacket() = default;

		// Copy constructors
		K2DataPacket(const K2DataPacket&) = default;
		K2DataPacket& operator=(const K2DataPacket&) = default;

		// Move operators
		K2DataPacket(K2DataPacket&& packet) noexcept : K2TrackerData(packet)
		{
		}

		K2DataPacket& operator=(K2DataPacket&& packet) noexcept
		{
			K2TrackerData::operator=(packet);
			return *this;
		}

		// Default constructor 2
		K2DataPacket(const K2TrackerData& m_data, const int& millis) :
			K2TrackerData(m_data), millisFromNow(millis)
		{
		}

		// Default constructor
		K2DataPacket(const K2TrackerData& m_data) :
			K2TrackerData(m_data)
		{
		}

		template <class Archive>
		KTVR_API void serialize(Archive& ar, unsigned int version);
	};

	class K2TrackerBase
	{
	public:
		K2TrackerPose pose = K2TrackerPose();
		K2TrackerData data = K2TrackerData();
		int id = -1; // For error case

		template <class Archive>
		KTVR_API void serialize(Archive& ar, unsigned int version);

		// Default constructors
		K2TrackerBase() = default;
		~K2TrackerBase() = default;

		// Copy constructors
		K2TrackerBase(const K2TrackerBase&) = default;
		K2TrackerBase& operator=(const K2TrackerBase&) = default;

		// Move operators
		K2TrackerBase(K2TrackerBase&&) = default;
		K2TrackerBase& operator=(K2TrackerBase&&) = default;

		K2TrackerBase(const K2TrackerPose& m_pose, K2TrackerData m_data) :
			pose(m_pose), data(std::move(m_data))
		{
		}
	};

	// Tracker: for new tracker creation
	// ID: while updating a tracker for state
	// Pose / Data: while updating a tracker
	//    :update possible via base object too
	class K2Message
	{
	public:
		// Message type, assume fail
		MessageType messageType = static_cast<int>(K2MessageType::K2Message_Invalid);

		// Message timestamp when sent
		long long messageTimestamp = 0,
		          messageManualTimestamp = 0; // This one's for mid-events

		// Since we're updating the main timestamp on creation,
		// we'd like to check other ones somehow

		// Object, parsing depends on type
		boost::optional<K2TrackerBase> tracker_base;
		boost::optional<std::vector<K2TrackerBase>> tracker_bases_vector;
		boost::optional<K2PosePacket> tracker_pose;
		K2DataPacket tracker_data;

		// Rest object, depends on type too
		int id = -1;
		bool state = false, want_reply = true;
		std::string message_string; // Placeholder for anything

		template <class Archive>
		KTVR_API void serialize(Archive& ar, unsigned int version);

		// Serialize as string
		KTVR_API std::string serializeToString();

		// Serialize from string, do not call as an overwrite
		[[nodiscard]] KTVR_API static K2Message parseFromString(const std::string& str) noexcept;

		// Default constructors
		K2Message() = default;
		~K2Message() = default;

		// Copy constructors
		K2Message(const K2Message&) = default;
		K2Message& operator=(const K2Message&) = default;

		// Move operators
		K2Message(K2Message&&) = default;
		K2Message& operator=(K2Message&&) = default;

		/*
		 * Lower constructors are made just
		 * to have a faster way to construct a message,
		 * without caring about its type.
		 * I mean, just why not?
		 */

		// Update the tracker's pose
		K2Message(int m_id, K2PosePacket m_pose) :
			messageType{static_cast<int>(K2MessageType::K2Message_UpdateTrackerPose)},
			tracker_pose{std::move(m_pose)},
			id{m_id}
		{
		}

		// Update multiple trackers' pose & data
		K2Message(std::vector<K2TrackerBase> m_tracker_bases) :
			messageType{static_cast<int>(K2MessageType::K2Message_UpdateTrackerVector)},
			tracker_bases_vector{std::move(m_tracker_bases)}
		{
		}

		// Update the tracker's data
		K2Message(int m_id, K2DataPacket m_data) :
			messageType{static_cast<int>(K2MessageType::K2Message_UpdateTrackerData)},
			tracker_data{std::move(m_data)},
			id{m_id}
		{
		}

		// Add a tracker, to automatically spawn,
		// set it's state to true
		K2Message(K2TrackerBase m_tracker) :
			messageType{static_cast<int>(K2MessageType::K2Message_AddTracker)},
			tracker_base{std::move(m_tracker)}
		{
		}

		// Basically the upper command,
		// although written a bit different
		// It uhmmm... will let us autospawn, but at the call
		K2Message(K2TrackerBase m_tracker, bool m_state) :
			messageType{static_cast<int>(K2MessageType::K2Message_AddTracker)},
			tracker_base{std::move(m_tracker)},
			state{m_state}
		{
		}

		// Set all trackers' state
		K2Message(const bool m_state) :
			messageType{static_cast<int>(K2MessageType::K2Message_SetStateAll)},
			state{m_state}
		{
		}

		// Set one tracker's state
		K2Message(const int m_id, const bool m_state) :
			messageType{static_cast<int>(K2MessageType::K2Message_SetTrackerState)},
			id{m_id}, state{m_state}
		{
		}

		// Download a tracker
		K2Message(const int m_id) :
			messageType{static_cast<int>(K2MessageType::K2Message_DownloadTracker)},
			id{m_id}
		{
		}
	};

	// Tracker: for new tracker creation
	// ID: while updating a tracker for state
	// Pose / Data: while updating a tracker
	//    :update possible via base object too
	class K2ResponseMessage
	{
	public:
		// Message type, assume fail
		MessageType messageType = static_cast<int>(K2ResponseMessageType::K2ResponseMessage_Invalid);

		// Message timestamp when sent
		long long messageTimestamp = 0,
		          messageManualTimestamp = 0; // This one's for mid-events

		// Since we're updating the main timestamp on creation,
		// we'd like to check other ones somehow

		// For downloading a tracker
		K2TrackerBase tracker_base = K2TrackerBase();

		// Rest object, depends on type too
		int id = -1; // Tracker's id, assume fail
		MessageCode result = -1; // For error case
		bool success = false;

		template <class Archive>
		KTVR_API void serialize(Archive& ar, unsigned int version);

		// Serialize as string
		KTVR_API std::string serializeToString();

		// Serialize from string, do not call as an overwriter
		[[nodiscard]] KTVR_API static K2ResponseMessage parseFromString(const std::string& str) noexcept;

		// Default constructors
		K2ResponseMessage() = default;
		~K2ResponseMessage() = default;

		// Copy constructors
		K2ResponseMessage(const K2ResponseMessage&) = default;
		K2ResponseMessage& operator=(const K2ResponseMessage&) = default;

		// Move operators
		K2ResponseMessage(K2ResponseMessage&&) = default;
		K2ResponseMessage& operator=(K2ResponseMessage&&) = default;

		/*
		 * Lower constructors are made just because
		 * to have a faster way to construct a message,
		 * without caring about its type.
		 * I mean, just why not?
		 */

		// ID as the response
		K2ResponseMessage(const int m_id) :
			messageType{static_cast<int>(K2ResponseMessageType::K2ResponseMessage_ID)},
			id{m_id}
		{
		}

		// Success return, eg for ping or somewhat
		// Set all trackers' state
		K2ResponseMessage(const bool m_success) :
			messageType{static_cast<int>(K2ResponseMessageType::K2ResponseMessage_Success)},
			success{m_success}
		{
		}

		// Return whole tracker object: creation / download
		K2ResponseMessage(K2TrackerBase m_tracker) :
			messageType{static_cast<int>(K2ResponseMessageType::K2ResponseMessage_Tracker)},
			tracker_base{std::move(m_tracker)}
		{
		}
	};

	/**
	 * \brief Dump std::string to hex character string
	 * \param s String to be converted
	 * \return Returns hex string
	 */
	KTVR_API std::string hexString(const std::string& s);

	/**
	 * \brief Dump hex std::string to ascii character string
	 * \param s String to be converted
	 * \return Returns ascii string
	 */
	KTVR_API std::string asciiString(const std::string& s);

	/// <summary>
	/// K2API Semaphore handles for WINAPI calls
	/// </summary>
	inline HANDLE k2api_to_Semaphore,
	              k2api_from_Semaphore,
	              k2api_start_Semaphore;

	/// <summary>
	/// K2API's last error string, check for empty
	/// </summary>
	inline std::string k2api_last_error;

	/// <summary>
	/// Get K2API's last error string
	/// </summary>
	inline std::string GetLastError() { return k2api_last_error; }

	/// <summary>
	/// K2API Pipe handle addresses for WINAPI calls
	/// </summary>
	inline std::string
		k2api_to_pipe_address,
		k2api_from_pipe_address;

	/**
	 * \brief Connects socket object to selected port, K2 uses 7135
	 * \return Returns 0 for success and -1 for failure
	 */
	KTVR_API int init_k2api(
		const std::string& k2_to_pipe = "\\\\.\\pipe\\k2api_ame_to_pipe",
		const std::string& k2_from_pipe = "\\\\.\\pipe\\k2api_ame_from_pipe",
		const std::string& k2_to_sem = "Global\\k2api_ame_to_sem",
		const std::string& k2_from_sem = "Global\\k2api_ame_from_sem",
		const std::string& k2_start_sem = "Global\\k2api_ame_start_sem") noexcept;

	/**
	 * \brief Disconnects socket object from port
	 * \return Returns 0 for success and -1 for failure
	 */
	KTVR_API int close_k2api() noexcept;

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
	 * \brief Add tracker to driver's trackers list
	 * \param tracker Tracker base that should be used for device creation
	 * \return Returns new tracker object / id / success, overwrites object's id
	 */
	KTVR_API [[nodiscard]] K2ResponseMessage add_tracker(K2TrackerBase& tracker) noexcept;

	/**
	 * \brief Connect (activate/spawn) tracker in SteamVR
	 * \param id Tracker's id which is to connect
	 * \param state Tracker's state to be set
	 * \argument want_reply Check if the client wants a reply
	 * \return Returns tracker id / success?
	 */
	template <bool want_reply = true>
	std::conditional_t<want_reply, K2ResponseMessage, std::monostate>
	set_tracker_state(int id, bool state) noexcept
	{
		try
		{
			// Send and grab the response
			// Thanks to our constructors,
			// message will set all
			// Send the message and return
			return send_message<want_reply>(
				K2Message(id, state));
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
			// Send and grab the response
			// Thanks to our constructors,
			// message will set all
			// Send the message
			return send_message<want_reply>(
				K2Message(state));
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
	 * \return Returns tracker id / success?
	 */
	template <bool want_reply = false>
	std::conditional_t<want_reply, K2ResponseMessage, std::monostate>
	update_tracker_vector(const std::vector<K2TrackerBase>& tracker_bases) noexcept
	{
		try
		{
			// Send and grab the response
			// Thanks to our constructors,
			// message will set all
			// Send the message and return
			return send_message<want_reply>(
				K2Message(tracker_bases));
		}
		catch (const std::exception& e)
		{
			if constexpr (want_reply) return K2ResponseMessage(); // Success is set to false by default
			else return std::monostate();
		}
	}

	/**
	 * \brief Update tracker's pose in SteamVR driver
	 * \param id Tracker's id which is to update
	 * \param tracker_pose New pose for tracker
	 * \argument want_reply Check if the client wants a reply
	 * \return Returns tracker id / success?
	 */
	template <bool want_reply = true>
	std::conditional_t<want_reply, K2ResponseMessage, std::monostate>
	update_tracker_pose(int id, const K2PosePacket& tracker_pose) noexcept
	{
		try
		{
			// Send and grab the response
			// Thanks to our constructors,
			// message will set all
			// Send the message and return
			return send_message<want_reply>(
				K2Message(id, tracker_pose));
		}
		catch (const std::exception& e)
		{
			if constexpr (want_reply) return K2ResponseMessage(); // Success is set to false by default
			else return std::monostate();
		}
	}


	/**
	 * \brief Update tracker's pose in SteamVR driver
	 * \param tracker_handle Tracker for updating data
	 * \argument want_reply Check if the client wants a reply
	 * \return Returns tracker id / success?
	 */
	template <bool want_reply = true>
	std::conditional_t<want_reply, K2ResponseMessage, std::monostate>
	update_tracker_pose(const K2TrackerBase& tracker_handle) noexcept
	{
		try
		{
			// Send the message and return
			return update_tracker_pose<want_reply>(tracker_handle.id, tracker_handle.pose);
		}
		catch (const std::exception& e)
		{
			if constexpr (want_reply) return K2ResponseMessage(); // Success is set to false by default
			else return std::monostate();
		}
	}

	/**
	 * \brief Update tracker's data in SteamVR driver (ONLY for yet not spawned trackers)
	 * \param id Tracker's id which is to update
	 * \param tracker_data New pose for tracker
	 * \argument want_reply Check if the client wants a reply
	 * \return Returns tracker id / success?
	 */
	template <bool want_reply = true>
	std::conditional_t<want_reply, K2ResponseMessage, std::monostate>
	update_tracker_data(int id, const K2DataPacket& tracker_data) noexcept
	{
		try
		{
			// Send and grab the response
			// Thanks to our constructors,
			// message will set all
			// Send the message and return
			return send_message<want_reply>(
				K2Message(id, tracker_data));
		}
		catch (const std::exception& e)
		{
			if constexpr (want_reply) return K2ResponseMessage(); // Success is set to false by default
			else return std::monostate();
		}
	}

	/**
	 * \brief Update tracker's data in SteamVR driver
	 * \param tracker_handle Tracker for updating data
	 * \argument want_reply Check if the client wants a reply
	 * \return Returns tracker id / success?
	 */
	template <bool want_reply = true>
	std::conditional_t<want_reply, K2ResponseMessage, std::monostate>
	update_tracker_data(const K2TrackerBase& tracker_handle) noexcept
	{
		try
		{
			// Send the message and return
			return update_tracker_data<want_reply>(tracker_handle.id, tracker_handle.data);
		}
		catch (const std::exception& e)
		{
			if constexpr (want_reply) return K2ResponseMessage(); // Success is set to false by default
			else return std::monostate();
		}
	}

	/**
	 * \brief Update tracker in SteamVR driver
	 * \param tracker Tracker base to be updated from
	 * \argument want_reply Check if the client wants a reply
	 * \return Returns tracker id / success?
	 */
	template <bool want_reply = true>
	std::conditional_t<want_reply, K2ResponseMessage, std::monostate>
	update_tracker(const K2TrackerBase& tracker) noexcept
	{
		try
		{
			// Send the message and return
			update_tracker_pose<want_reply>(tracker.id, tracker.pose);

			// Data is more important then return data
			return update_tracker_data<want_reply>(tracker.id, tracker.data);
		}
		catch (const std::exception& e)
		{
			if constexpr (want_reply) return K2ResponseMessage(); // Success is set to false by default
			else return std::monostate();
		}
	}

	/**
	 * \brief Update tracker's pose in SteamVR driver with already existing values
	 * \param tracker_id Tracker for updating data
	 * \argument want_reply Check if the client wants a reply
	 * \return Returns tracker id / success?
	 */
	template <bool want_reply = true>
	std::conditional_t<want_reply, K2ResponseMessage, std::monostate>
	refresh_tracker_pose(const int& tracker_id) noexcept
	{
		try
		{
			// Send and grab the response
			auto message = K2Message();
			message.id = tracker_id;
			message.messageType = static_cast<int>(K2MessageType::K2Message_RefreshTracker);

			// Send the message and return
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
			// Send and grab the response
			auto message = K2Message();
			message.message_string = std::move(reason);
			message.messageType = static_cast<int>(K2MessageType::K2Message_RequestRestart);

			// Send the message and return
			return send_message<want_reply>(message);
		}
		catch (const std::exception& e)
		{
			if constexpr (want_reply) return K2ResponseMessage(); // Success is set to false by default
			else return std::monostate();
		}
	}

	/**
	 * \brief Grab all possible data from existing tracker
	 * \param tracker_id Tracker id for download
	 * \return Returns tracker object / id / success?
	 */
	KTVR_API K2ResponseMessage download_tracker(const int& tracker_id) noexcept;

	/**
	 * \brief Grab all possible data from existing tracker
	 * \param tracker_serial Tracker id for download
	 * \return Returns tracker object / id / success?
	 */
	KTVR_API K2ResponseMessage download_tracker(const std::string& tracker_serial) noexcept;

	/**
	 * \brief Grab all possible data from existing tracker
	 * \param tracker Tracker base id is to be grabbed from
	 * \return Returns tracker object / id / success?
	 */
	KTVR_API K2ResponseMessage download_tracker(const K2TrackerBase& tracker) noexcept;

	/**
	 * \brief Grab all possible data from existing tracker
	 * \param tracker_role Tracker role for download
	 * \return Returns tracker object / id / success?
	 */
	KTVR_API K2ResponseMessage download_tracker(const ITrackerType& tracker_role) noexcept;

	/**
	 * \brief Test connection with the server
	 * \return Returns send_time / total_time / success?
	 */
	KTVR_API std::tuple<K2ResponseMessage, long long, long long> test_connection() noexcept;
}