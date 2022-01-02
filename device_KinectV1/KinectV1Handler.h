#pragma once
#include <Windows.h>
#include <Ole2.h>
#include <NuiApi.h>
#include <NuiImageCamera.h>
#include <NuiSensor.h>
#include <NuiSkeleton.h>

#include "KinectToVR_API_Devices.h"

/* Not exported */

class KinectV1Handler : public ktvr::K2TrackingDeviceBase_KinectBasis
{
	// A representation of the Kinect elements for the v1 api
public:
	KinectV1Handler()
	{
		//KinectV1Handler::initialize();
		K2TrackingDeviceBase_KinectBasis::deviceType = ktvr::K2_Kinect;
		K2TrackingDeviceBase_KinectBasis::deviceName = "Xbox 360 Kinect";

		K2TrackingDeviceBase_KinectBasis::deviceCharacteristics = ktvr::K2_Character_Full;
		K2TrackingDeviceBase_KinectBasis::flipSupported = true;
		K2TrackingDeviceBase_KinectBasis::appOrientationSupported = true;
	}

	HANDLE kinectRGBStream = nullptr;
	HANDLE kinectDepthStream = nullptr;
	INuiSensor* kinectSensor = nullptr;
	NUI_SKELETON_FRAME skeletonFrame = {0};

	NUI_SKELETON_BONE_ORIENTATION boneOrientations[NUI_SKELETON_POSITION_COUNT];

	void initialize() override;
	void update() override;
	void shutdown() override;

	virtual ~KinectV1Handler()
	{
	}

	HRESULT getStatusResult() override;
	std::string statusResultString(HRESULT stat) override;

private:
	bool initKinect();
	bool acquireKinectFrame(NUI_IMAGE_FRAME& imageFrame, HANDLE& rgbStream, INuiSensor*& sensor);
	void releaseKinectFrame(NUI_IMAGE_FRAME& imageFrame, HANDLE& rgbStream, INuiSensor*& sensor);

	void updateSkeletalData();

	/* For translating Kinect joint enumeration to K2 space */
	int globalIndex[25] = {3, 2, 2, 4, 5, 6, 7, 7, 7, 8, 9, 10, 11, 11, 11, 1, 0, 12, 13, 14, 15, 16, 17, 18, 19};
};

/* Exported for dynamic linking */
extern "C" __declspec(dllexport) void* TrackingDeviceBaseFactory(
	const char* pVersionName, int* pReturnCode)
{
	// Return the device handler for tracking
	// but only if interfaces are the same / up-to-date
	if (0 == strcmp(ktvr::IK2API_Devices_Version, pVersionName))
	{
		static KinectV1Handler TrackingHandler; // Create a new device handler -> KinectV2

		*pReturnCode = ktvr::K2InitError_None;
		return &TrackingHandler;
	}

	// Return code for initialization
	*pReturnCode = ktvr::K2InitError_BadInterface;
}
