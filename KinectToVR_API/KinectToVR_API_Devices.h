#pragma once
#include <Windows.h>

#include <string>
#include <vector>

#include <Eigen/Dense>

/*
 * K2API Devices
 *
 * This is a separate header because we won't need linking
 * & doing much more stuff for nothing, just gonna include
 * this single header +Eigen as you can see up in includes
 *
 */

namespace ktvr {

	// Interace Version
	static const char* IK2API_Devices_Version = "IK2API_Version_003";

	// Return messaging types
	enum K2InitErrorType
	{
		K2InitError_Invalid,
		// Default
		K2InitError_None,
		// Just the ID
		K2InitError_BadInterface
	};

	// Global Joint Types,
	// see enumeration in external/Kinect
	enum ITrackedJointType
	{
		Joint_Head,
		Joint_Neck,
		Joint_SpineShoulder,
		Joint_ShoulderLeft,
		Joint_ElbowLeft,
		Joint_WristLeft,
		Joint_HandLeft,
		Joint_HandTipLeft,
		Joint_ThumbLeft,
		Joint_ShoulderRight,
		Joint_ElbowRight,
		Joint_WristRight,
		Joint_HandRight,
		Joint_HandTipRight,
		Joint_ThumbRight,
		Joint_SpineMiddle,
		Joint_SpineWaist,
		Joint_HipLeft,
		Joint_KneeLeft,
		Joint_AnkleLeft,
		Joint_FootLeft,
		Joint_HipRight,
		Joint_KneeRight,
		Joint_AnkleRight,
		Joint_FootRight,
		Joint_Total
	};

	// Global joint states
	enum ITrackedJointState
	{
		State_NotTracked,
		State_Inferred,
		State_Tracked
	};

	// Device types for tracking
	enum ITrackingDeviceType
	{
		K2_Unknown,
		K2_Kinect,
		K2_Joints,
		K2_Override
	};

	// Device types for joints [KINECT]
	enum ITrackingDeviceCharacteristics
	{
		// Not set???
		K2_Character_Unknown,
		// NO mathbased, only [ head, waist, ankles ]
		K2_Character_Basic,
		// SUP mathbased, only [ head, waist, knees, ankles, foot_tips ]
		K2_Character_Simple,
		// SUP mathbased, [ everything ]
		K2_Character_Full 
	};

	// Alias for code readability
	typedef int JointTrackingState, K2DeviceType, K2DeviceCharacteristics, MessageType, MessageCode;

	// Tracking Device class for client plugins to base on [KINECT]
	class K2TrackingDeviceBase_KinectBasis
	{
	public:
		virtual ~K2TrackingDeviceBase_KinectBasis()
		{
		}

		// These 3 functions are critical.
		// All 3 are called by K2App,
		// - in init you should set the device up (and run the first frame)
		// - in update you should update the array of joints with data
		// - in shutdown you should gracefully turn your device off
		virtual void initialize()
		{
		}

		virtual void shutdown()
		{
		}

		virtual void update()
		{
		}

		// Should be set up at construction
		// Kinect type must provide joints: [ head, waist, knees, ankles, foot_tips ] or [ head, waist, ankles ]
		// Other type must provide joints: [ waist, ankles ] and will persuade manual calibration

		// Basic character will provide the same as JointsBasis but with head to support autocalibration
		// Simple character will provide the same as Basic but with ankles and knees to support mathbased
		// Full character will provide every kinect joint
		K2DeviceCharacteristics getDeviceCharacteristics() { return deviceCharacteristics; }

		K2DeviceType getDeviceType() { return deviceType; }
		std::string getDeviceName() { return deviceName; } // Custom name

		std::array<Eigen::Vector3f, 25> getJointPositions() { return jointPositions; }
		std::array<Eigen::Quaternionf, 25> getJointOrientations() { return jointOrientations; }
		std::array<JointTrackingState, 25> getTrackingStates() { return trackingStates; }

		// After init, this should always return true
		[[nodiscard]] bool isInitialized() const { return initialized; }

		// These will indicate the device's status.
		// Both should be updated either on call or as frequent as possible
		virtual HRESULT getStatusResult() { return E_NOTIMPL; }
		virtual std::string statusResultString(HRESULT stat) { return "statusResultString behaviour not defined"; }

		// This should be updated on every frame,
		// along with joint devices
		// -> will lead to global tracking loss notification
		//    if set to false at runtime somewhen
		[[nodiscard]] bool isSkeletonTracked() const { return skeletonTracked; }

		// Should be set up at construction
		[[nodiscard]] bool isFlipSupported() const { return flipSupported; } // Flip block
		[[nodiscard]] bool isAppOrientationSupported() const { return appOrientationSupported; } // Math-based

	protected:
		K2DeviceCharacteristics deviceCharacteristics = K2_Character_Unknown;

		K2DeviceType deviceType = K2_Unknown;
		std::string deviceName = "Name not set";

		bool initialized = false;
		bool skeletonTracked = false;

		bool flipSupported = true;
		bool appOrientationSupported = true;

		std::array<Eigen::Vector3f, 25> jointPositions = { Eigen::Vector3f(0.f, 0.f, 0.f) };
		std::array<Eigen::Quaternionf, 25> jointOrientations = { Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f) };
		std::array<JointTrackingState, 25> trackingStates = { State_NotTracked };

		class FailedKinectInitialization : public std::exception
		{
			[[nodiscard]] virtual const char* what() const throw()
			{
				return "Failure to initialize the Tracking Device. Is it set up properly?";
			}
		} FailedKinectInitialization;
	};

	// Tracking Device Joint class for client plugins to base on [PSMS]
	class K2TrackedJoint
	{
		// Named joint, provides pos, rot, state and ofc name
	public:
		K2TrackedJoint()
		{
		}
		K2TrackedJoint(std::string name) : jointName{ std::move(name) }
		{
		}

		std::string getJointName() { return jointName; } // Custom name

		Eigen::Vector3f getJointPosition() { return jointPosition; }
		Eigen::Quaternionf getJointOrientation() { return jointOrientation; }
		JointTrackingState getTrackingState() { return trackingState; }

		// For servers!
		void update(Eigen::Vector3f position,
			Eigen::Quaternionf orientation,
			JointTrackingState state)
		{
			jointPosition = position;
			jointOrientation = orientation;
			trackingState = state;
		}

		// For servers!
		void update(JointTrackingState state)
		{
			trackingState = state;
		}

	protected:
		// Tracker should be centered automatically
		Eigen::Quaternionf jointOrientation = Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f);
		Eigen::Vector3f jointPosition = Eigen::Vector3f(0.f, 0.f, 0.f);
		JointTrackingState trackingState = State_NotTracked;

		std::string jointName = "Name not set";
	};

	// Tracking Device class for client plugins to base on [PSMS]
	class K2TrackingDeviceBase_JointsBasis
	{
	public:
		virtual ~K2TrackingDeviceBase_JointsBasis()
		{
		}

		// These 3 functions are critical.
		// All 3 are called by K2App,
		// - in init you should set the device up (and run the first frame)
		// - in update you should update the array of joints with data
		// - in shutdown you should gracefully turn your device off
		virtual void initialize()
		{
		}

		virtual void shutdown()
		{
		}

		virtual void update()
		{
		}

		// Should be set up at construction
		// Kinect type must provide joints: [ head, waist, knees, ankles, foot_tips ]
		// Other type must provide joints: [ waist, ankles ] and will persuade manual calibration
		K2DeviceType getDeviceType() { return deviceType; }
		std::string getDeviceName() { return deviceName; } // Custom name

		// Joints' vector. You need to update appended joints in every update() call
		std::vector<K2TrackedJoint> getTrackedJoints() { return trackedJoints; }

		// After init, this should always return true
		[[nodiscard]] bool isInitialized() const { return initialized; }

		// These will indicate the device's status.
		// Both should be updated either on call or as frequent as possible
		virtual HRESULT getStatusResult() { return E_NOTIMPL; }
		virtual std::string statusResultString(HRESULT stat) { return "statusResultString behaviour not defined"; }

		// Signal the joint eg psm_id0 that it's being selected
		virtual void signalJoint(uint32_t at)
		{
		} // Just empty, do not throw cause not everyone will override it

		// This should be updated on every frame,
		// along with joint devices
		// -> will lead to global tracking loss notification
		//    if set to false at runtime somewhen
		[[nodiscard]] bool isSkeletonTracked() const { return skeletonTracked; }

	protected:
		K2DeviceType deviceType = K2_Unknown;
		std::string deviceName = "Name not set";

		bool initialized = false;
		bool skeletonTracked = false;

		std::vector<K2TrackedJoint> trackedJoints = { K2TrackedJoint() };

		class FailedJointsInitialization : public std::exception
		{
			[[nodiscard]] virtual const char* what() const throw()
			{
				return "Failure to initialize the Tracking Device. Is it set up properly?";
			}
		} FailedJointsInitialization;
	};

}