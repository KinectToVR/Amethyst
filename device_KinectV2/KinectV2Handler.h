#pragma once
#include <Kinect.h>
#include <Windows.h>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include "Amethyst_API_Devices.h"
#include "Amethyst_API_Paths.h"

/* Not exported */

class KinectV2Handler : public ktvr::K2TrackingDeviceBase_KinectBasis
{
public:
	KinectV2Handler()
	{
		//KinectV2Handler::initialize();
		LOG(INFO) << "Constructing the Kinect V2 (XBONE) Handler for KinectBasis K2TrackingDevice...";

		K2TrackingDeviceBase_KinectBasis::deviceType = ktvr::K2_Kinect;
		K2TrackingDeviceBase_KinectBasis::deviceName = "Xbox One Kinect";

		K2TrackingDeviceBase_KinectBasis::deviceCharacteristics = ktvr::K2_Character_Full;
		K2TrackingDeviceBase_KinectBasis::flipSupported = true;
		K2TrackingDeviceBase_KinectBasis::appOrientationSupported = true;
	}

	virtual ~KinectV2Handler()
	{
	}

	IKinectSensor* kinectSensor = nullptr;
	IBodyFrameReader* bodyFrameReader = nullptr;
	IColorFrameReader* colorFrameReader = nullptr;
	IDepthFrameReader* depthFrameReader = nullptr;
	IMultiSourceFrame* multiFrame = nullptr;
	ICoordinateMapper* coordMapper = nullptr;
	BOOLEAN isTracking = false;

	Joint joints[JointType_Count];
	JointOrientation boneOrientations[JointType_Count];
	IBody* kinectBodies[BODY_COUNT] = {nullptr};

	HRESULT getStatusResult() override;
	std::string statusResultString(HRESULT stat) override;

	void initialize() override;
	void update() override;
	void shutdown() override;

	bool convertColorToDepthResolution = false;
	void onBodyFrameArrived(IBodyFrameReader& sender, IBodyFrameArrivedEventArgs& eventArgs);
	virtual void updateSkeletalData();

private:
	bool initKinect();
	void updateParseFrame();
	void initializeSkeleton();
	void terminateSkeleton();

	WAITABLE_HANDLE h_bodyFrameEvent;
	bool newBodyFrameArrived = false;

	/* For translating Kinect joint enumeration to K2 space */
	int globalIndex[25] = {3, 2, 20, 4, 5, 6, 7, 21, 22, 8, 9, 10, 11, 23, 24, 1, 0, 12, 13, 14, 15, 16, 17, 18, 19};
};

/* Exported for dynamic linking */
extern "C" __declspec(dllexport) void* TrackingDeviceBaseFactory(
	const char* pVersionName, int* pReturnCode)
{
	// Return the device handler for tracking
	// but only if interfaces are the same / up-to-date
	if (0 == strcmp(ktvr::IK2API_Devices_Version, pVersionName))
	{
		static KinectV2Handler TrackingHandler; // Create a new device handler -> KinectV2

		*pReturnCode = ktvr::K2InitError_None;
		return &TrackingHandler;
	}

	// Return code for initialization
	*pReturnCode = ktvr::K2InitError_BadInterface;
}
