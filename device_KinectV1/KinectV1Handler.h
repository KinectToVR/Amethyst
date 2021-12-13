#pragma once
#include <Windows.h>
#include <Ole2.h>
#include <NuiApi.h>
#include <NuiImageCamera.h>
#include <NuiSensor.h>
#include <NuiSkeleton.h>

#include "KinectToVR_API.h"
/* Not exported */

class KinectV1Handler : public ktvr::TrackingDeviceBase
{
	// A representation of the Kinect elements for the v1 api
public:
	KinectV1Handler()
	{
		KinectV1Handler::initialize();
		TrackingDeviceBase::deviceType = ktvr::K2_Kinect;
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

extern "C" __declspec(dllexport) void* TrackingDeviceBaseFactory(const char* pVersionName, int* pReturnCode)
{
	static KinectV1Handler TrackingHandler; // Create a new device handler -> KinectV1
	static ktvr::TrackingDeviceBase BaseTrackingHandler = // Slice the handler to the general handler
		static_cast<ktvr::TrackingDeviceBase&>(TrackingHandler);

	// Return the device handler for tracking
	// but only if interfaces are the same / up-to-date
	if (0 == strcmp(ktvr::IK2API_Version, pVersionName))
	{
		return &BaseTrackingHandler;
	}

	// Return code for initialization
	(*pReturnCode) = ktvr::K2InitError_None;
	if (pReturnCode)
		*pReturnCode = ktvr::K2InitError_BadInterface;
}
