#pragma once
#include <iostream>
#include <Windows.h>
#include <optional>
#include <chrono>
#include "K2Tracker.h"
#include <KinectToVR_API.h>

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
		const std::string& k2_to_pipe = "\\\\.\\pipe\\k2api_to_pipe",
		const std::string& k2_from_pipe = "\\\\.\\pipe\\k2api_from_pipe",
		const std::string& k2_to_sem = "Global\\k2api_to_sem",
		const std::string& k2_from_sem = "Global\\k2api_from_sem",
		const std::string& k2_start_sem = "Global\\k2api_start_sem");
	void setActive(bool m_isActive) { _isActive = m_isActive; }

	// Value should not be discarded, it'd be useless
	[[nodiscard]] bool isActive() const { return _isActive; }

	// Tracker vector with pre-appended 3 default lower body trackers
	std::vector<K2Tracker> trackerVector
	{
		K2Tracker( // WAIST TRACKER
			ktvr::K2TrackerBase(
			ktvr::K2TrackerPose(), // Default pose 
			ktvr::K2TrackerData(
				"LHR-CB9AD1T0", // Serial
				ktvr::ITrackerType::Tracker_Waist, // Role
				false // AutoAdd
			)
		)),

		K2Tracker( // LEFT FOOT TRACKER
			ktvr::K2TrackerBase(
			ktvr::K2TrackerPose(), // Default pose 
			ktvr::K2TrackerData(
				"LHR-CB9AD1T1", // Serial
				ktvr::ITrackerType::Tracker_LeftFoot, // Role
				false // AutoAdd
			)
		)),

		K2Tracker( // RIGHT FOOT TRACKER
			ktvr::K2TrackerBase(
			ktvr::K2TrackerPose(), // Default pose 
			ktvr::K2TrackerData(
				"LHR-CB9AD1T2", // Serial
				ktvr::ITrackerType::Tracker_RightFoot, // Role
				false // AutoAdd
			)
		)),

		K2Tracker( // LEFT ELBOW TRACKER
			ktvr::K2TrackerBase(
			ktvr::K2TrackerPose(), // Default pose 
			ktvr::K2TrackerData(
				"LHR-CB9AD1T3", // Serial
				ktvr::ITrackerType::Tracker_LeftElbow, // Role
				false // AutoAdd
			)
		)),

		K2Tracker( // RIGHT ELBOW TRACKER
			ktvr::K2TrackerBase(
			ktvr::K2TrackerPose(), // Default pose 
			ktvr::K2TrackerData(
				"LHR-CB9AD1T4", // Serial
				ktvr::ITrackerType::Tracker_RightElbow, // Role
				false // AutoAdd
			)
		)),

		K2Tracker( // LEFT KNEE TRACKER
			ktvr::K2TrackerBase(
			ktvr::K2TrackerPose(), // Default pose 
			ktvr::K2TrackerData(
				"LHR-CB9AD1T5", // Serial
				ktvr::ITrackerType::Tracker_LeftKnee, // Role
				false // AutoAdd
			)
		)),

		K2Tracker( // RIGHT KNEE TRACKER
			ktvr::K2TrackerBase(
			ktvr::K2TrackerPose(), // Default pose 
			ktvr::K2TrackerData(
				"LHR-CB9AD1T6", // Serial
				ktvr::ITrackerType::Tracker_RightKnee, // Role
				false // AutoAdd
			)
		))
	};
};
