#pragma once
#include <string>
#include <openvr_driver.h>
#include <Amethyst_API.h>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include <boost/assign/list_of.hpp>
#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_map.hpp>

// Mapping enum to string for eliminating if-else loop
const boost::unordered_map<ktvr::ITrackerType, const char*>
ITrackerType_String = boost::assign::map_list_of
(ktvr::ITrackerType::Tracker_Handed, "vive_tracker_handed")
(ktvr::ITrackerType::Tracker_LeftFoot, "vive_tracker_left_foot")
(ktvr::ITrackerType::Tracker_RightFoot, "vive_tracker_right_foot")
(ktvr::ITrackerType::Tracker_LeftShoulder, "vive_tracker_left_Shoulder")
(ktvr::ITrackerType::Tracker_RightShoulder, "vive_tracker_right_shoulder")
(ktvr::ITrackerType::Tracker_LeftElbow, "vive_tracker_left_elbow")
(ktvr::ITrackerType::Tracker_RightElbow, "vive_tracker_right_elbow")
(ktvr::ITrackerType::Tracker_LeftKnee, "vive_tracker_left_knee")
(ktvr::ITrackerType::Tracker_RightKnee, "vive_tracker_right_knee")
(ktvr::ITrackerType::Tracker_Waist, "vive_tracker_waist")
(ktvr::ITrackerType::Tracker_Chest, "vive_tracker_chest")
(ktvr::ITrackerType::Tracker_Camera, "vive_tracker_camera")
(ktvr::ITrackerType::Tracker_Keyboard, "vive_tracker_keyboard"),

ITrackerType_Role_String = boost::assign::map_list_of
(ktvr::ITrackerType::Tracker_Handed, "TrackerRole_Handed")
(ktvr::ITrackerType::Tracker_LeftFoot, "TrackerRole_LeftFoot")
(ktvr::ITrackerType::Tracker_RightFoot, "TrackerRole_RightFoot")
(ktvr::ITrackerType::Tracker_LeftShoulder, "TrackerRole_LeftShoulder")
(ktvr::ITrackerType::Tracker_RightShoulder, "TrackerRole_RightShoulder")
(ktvr::ITrackerType::Tracker_LeftElbow, "TrackerRole_LeftElbow")
(ktvr::ITrackerType::Tracker_RightElbow, "TrackerRole_RightElbow")
(ktvr::ITrackerType::Tracker_LeftKnee, "TrackerRole_LeftKnee")
(ktvr::ITrackerType::Tracker_RightKnee, "TrackerRole_RightKnee")
(ktvr::ITrackerType::Tracker_Waist, "TrackerRole_Waist")
(ktvr::ITrackerType::Tracker_Chest, "TrackerRole_Chest")
(ktvr::ITrackerType::Tracker_Camera, "TrackerRole_Camera")
(ktvr::ITrackerType::Tracker_Keyboard, "TrackerRole_Keyboard");

class K2Tracker : public vr::ITrackedDeviceServerDriver
{
public:

	explicit K2Tracker(ktvr::K2TrackerBase const& tracker_base);
	virtual ~K2Tracker() = default;

	/**
	 * \brief Get tracker serial number
	 * \return Returns tracker's serial in std::string
	 */
	[[nodiscard]] std::string get_serial() const;

	/**
	 * \brief Get device index in OpenVR
	 * \return OpenVR device index in uint32_t
	 */
	[[nodiscard]] vr::TrackedDeviceIndex_t get_index() const;

	/**
	 * \brief Get device role in K2 / OVR
	 * \return K2 tracker type / role
	 */
	[[nodiscard]] ktvr::ITrackerType get_role() const;

	/**
	 * \brief Update void for server driver
	 */
	void update();

	/**
	 * \brief Function processing OpenVR events
	 */
	void process_event(const vr::VREvent_t& event);

	/**
	 * \brief Activate device (called from OpenVR)
	 * \return InitError for OpenVR if we're set up correctly
	 */
	virtual vr::EVRInitError Activate(vr::TrackedDeviceIndex_t index);

	/**
	 * \brief Deactivate tracker (remove)
	 */
	virtual void Deactivate() {
		// Clear device id
		_index = vr::k_unTrackedDeviceIndexInvalid;
	}

	/**
	 * \brief Handle debug request (not needed/implemented)
	 */
	virtual void DebugRequest(const char* request, char* response_buffer, uint32_t response_buffer_size) {
		// No custom debug requests defined
		if (response_buffer_size >= 1)
			response_buffer[0] = 0;
	}

	virtual void EnterStandby() { }
	virtual void LeaveStandby() { }
	virtual bool ShouldBlockStandbyMode() { return false; }

	/**
	 * \brief Get component handle (for OpenVR)
	 */
	void* GetComponent(const char* component) {
		// No extra components on this device so always return nullptr
		return NULL;
	}

	/**
	 * \brief Return device's actual pose
	 */
	vr::DriverPose_t GetPose() override;

	// Update pose
	void set_pose(ktvr::K2PosePacket const& pose);

	// Update data (only if uninitialized)
	void set_data(ktvr::K2DataPacket const& data);

	void set_state(bool state);
	bool spawn(); // TrackedDeviceAdded

	// Get to know if tracker is activated (added)
	[[nodiscard]] bool is_added() const { return _added; }
	// Get to know if tracker is active (connected)
	[[nodiscard]] bool is_active() const { return _active; }

	// Get the base object
	[[nodiscard]] ktvr::K2TrackerBase getTrackerBase();

private:

	// Tracker base to be returned
	ktvr::K2TrackerBase _trackerBase;

	// Is tracker added/active
	bool _added = false, _active = false;
	bool _activated = false;

	// Stores the openvr supplied device index.
	vr::TrackedDeviceIndex_t _index;

	// Stores the devices current pose.
	vr::DriverPose_t _pose;

	// An identifier for OpenVR for when we want to make property changes to this device.
	vr::PropertyContainerHandle_t _props;

	// A struct for concise storage of all of the component handles for this device.
	struct TrackerComponents
	{
		vr::VRInputComponentHandle_t
			_system_click,
			_haptic;
	};

	TrackerComponents _components;
	std::string _serial;
	int _role;
};
