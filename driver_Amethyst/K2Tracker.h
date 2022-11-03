#pragma once
#include "pch.h"

#include <openvr_driver.h>
#include <Amethyst_API.h>

// Mapping enum to string for eliminating if-else loop
const std::map<ktvr::ITrackerType, const char*>
	ITrackerType_String{
		{ktvr::ITrackerType::Tracker_Handed, "vive_tracker_handed"},
		{ktvr::ITrackerType::Tracker_LeftFoot, "vive_tracker_left_foot"},
		{ktvr::ITrackerType::Tracker_RightFoot, "vive_tracker_right_foot"},
		{ktvr::ITrackerType::Tracker_LeftShoulder, "vive_tracker_left_Shoulder"},
		{ktvr::ITrackerType::Tracker_RightShoulder, "vive_tracker_right_shoulder"},
		{ktvr::ITrackerType::Tracker_LeftElbow, "vive_tracker_left_elbow"},
		{ktvr::ITrackerType::Tracker_RightElbow, "vive_tracker_right_elbow"},
		{ktvr::ITrackerType::Tracker_LeftKnee, "vive_tracker_left_knee"},
		{ktvr::ITrackerType::Tracker_RightKnee, "vive_tracker_right_knee"},
		{ktvr::ITrackerType::Tracker_Waist, "vive_tracker_waist"},
		{ktvr::ITrackerType::Tracker_Chest, "vive_tracker_chest"},
		{ktvr::ITrackerType::Tracker_Camera, "vive_tracker_camera"},
		{ktvr::ITrackerType::Tracker_Keyboard, "vive_tracker_keyboard"}
	},

	ITrackerType_Role_String{
		{ktvr::ITrackerType::Tracker_Handed, "TrackerRole_Handed"},
		{ktvr::ITrackerType::Tracker_LeftFoot, "TrackerRole_LeftFoot"},
		{ktvr::ITrackerType::Tracker_RightFoot, "TrackerRole_RightFoot"},
		{ktvr::ITrackerType::Tracker_LeftShoulder, "TrackerRole_LeftShoulder"},
		{ktvr::ITrackerType::Tracker_RightShoulder, "TrackerRole_RightShoulder"},
		{ktvr::ITrackerType::Tracker_LeftElbow, "TrackerRole_LeftElbow"},
		{ktvr::ITrackerType::Tracker_RightElbow, "TrackerRole_RightElbow"},
		{ktvr::ITrackerType::Tracker_LeftKnee, "TrackerRole_LeftKnee"},
		{ktvr::ITrackerType::Tracker_RightKnee, "TrackerRole_RightKnee"},
		{ktvr::ITrackerType::Tracker_Waist, "TrackerRole_Waist"},
		{ktvr::ITrackerType::Tracker_Chest, "TrackerRole_Chest"},
		{ktvr::ITrackerType::Tracker_Camera, "TrackerRole_Camera"},
		{ktvr::ITrackerType::Tracker_Keyboard, "TrackerRole_Keyboard"}
	},

	ITrackerType_Role_Serial{
		{ktvr::ITrackerType::Tracker_Handed, "AME-HANDED"},
		{ktvr::ITrackerType::Tracker_LeftFoot, "AME-LFOOT"},
		{ktvr::ITrackerType::Tracker_RightFoot, "AME-RFOOT"},
		{ktvr::ITrackerType::Tracker_LeftShoulder, "AME-LSHOULDER"},
		{ktvr::ITrackerType::Tracker_RightShoulder, "AME-RSHOULDER"},
		{ktvr::ITrackerType::Tracker_LeftElbow, "AME-LELBOW"},
		{ktvr::ITrackerType::Tracker_RightElbow, "AME-RELBOW"},
		{ktvr::ITrackerType::Tracker_LeftKnee, "AME-LKNEE"},
		{ktvr::ITrackerType::Tracker_RightKnee, "AME-RKNEE"},
		{ktvr::ITrackerType::Tracker_Waist, "AME-WAIST"},
		{ktvr::ITrackerType::Tracker_Chest, "AME-CHEST"},
		{ktvr::ITrackerType::Tracker_Camera, "AME-CAMERA"},
		{ktvr::ITrackerType::Tracker_Keyboard, "AME-KEYBOARD"}
	};

class K2Tracker : public vr::ITrackedDeviceServerDriver
{
public:
	explicit K2Tracker(const ktvr::K2TrackerBase& tracker_base);
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
	vr::EVRInitError Activate(vr::TrackedDeviceIndex_t index) override;

	/**
	 * \brief Deactivate tracker (remove)
	 */
	void Deactivate() override
	{
		// Clear device id
		_index = vr::k_unTrackedDeviceIndexInvalid;
	}

	/**
	 * \brief Handle debug request (not needed/implemented)
	 */
	void DebugRequest(const char* request, char* response_buffer, uint32_t response_buffer_size) override
	{
		// No custom debug requests defined
		if (response_buffer_size >= 1)
			response_buffer[0] = 0;
	}

	void EnterStandby() override
	{
	}

	virtual void LeaveStandby()
	{
	}

	virtual bool ShouldBlockStandbyMode() { return false; }

	/**
	 * \brief Get component handle (for OpenVR)
	 */
	void* GetComponent(const char* component) override
	{
		// No extra components on this device so always return nullptr
		return nullptr;
	}

	/**
	 * \brief Return device's actual pose
	 */
	vr::DriverPose_t GetPose() override;

	// Update pose
	void set_pose(const ktvr::K2TrackerPose& pose);

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
