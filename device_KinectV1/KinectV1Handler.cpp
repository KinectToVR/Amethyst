#include "pch.h"
#include "KinectV1Handler.h"

HRESULT KinectV1Handler::getStatusResult()
{
	if (kinectSensor)
		return kinectSensor->NuiStatus();
	return E_NUI_NOTCONNECTED;
}

std::string KinectV1Handler::statusResultString(HRESULT stat)
{
	// Wrap status to string for readability
	switch (stat)
	{
	case S_OK: return "Success!\nS_OK\nEverything's good!";
	case S_NUI_INITIALIZING: return "INITIALIZING\nS_NUI_INITIALIZING\nThe device is connected, but still initializing.";
	case E_NUI_NOTCONNECTED: return "NOTCONNECTED\nE_NUI_NOTCONNECTED\nThe device is not connected.";
	case E_NUI_NOTGENUINE: return "NOTGENUINE\nE_NUI_NOTGENUINE\nThe device is not a valid Kinect.";
	case E_NUI_NOTSUPPORTED: return "NOTSUPPORTED\nE_NUI_NOTSUPPORTED\nThe device is an unsupported model.";
	case E_NUI_INSUFFICIENTBANDWIDTH: return
			"INSUFFICIENTBANDWIDTH\nE_NUI_INSUFFICIENTBANDWIDTH\nThe device is connected to a hub without the necessary bandwidth requirements.";
	case E_NUI_NOTPOWERED: return "NOTPOWERED\nE_NUI_NOTPOWERED\nThere is either a problem with your adapter/cables or with the Kinect device driver registration in Windows.";
	case E_NUI_NOTREADY: return "NOTREADY\nE_NUI_NOTREADY\nThere was some other unspecified error.";
	default: return "Undefined: " + std::to_string(stat) + "\nE_UNDEFINED\nSomething weird has happened, though we can't tell what.";
	}
}

void KinectV1Handler::initialize()
{
	try
	{
		initialized = initKinect();
		if (!initialized) throw FailedKinectInitialization;
	}
	catch (std::exception& e)
	{
	}
}

void KinectV1Handler::shutdown()
{
	try
	{
		// Shut down the sensor (Only NUI API)
		if (kinectSensor) // Protect from null call
			kinectSensor->NuiShutdown();
	}
	catch (std::exception& e)
	{
	}
}

void KinectV1Handler::update()
{
	if (isInitialized())
	{
		HRESULT kinectStatus = kinectSensor->NuiStatus();
		if (kinectStatus == S_OK) // Update only if sensor works
		{
			updateSkeletalData();
		}
	}
}

bool KinectV1Handler::initKinect()
{
	//Get a working Kinect Sensor
	int numSensors = 0;
	if (NuiGetSensorCount(&numSensors) < 0 || numSensors < 1)
	{
		return false;
	}
	if (NuiCreateSensorByIndex(0, &kinectSensor) < 0)
	{
		return false;
	}
	//Initialize Sensor
	HRESULT hr = kinectSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_SKELETON);
	kinectSensor->NuiSkeletonTrackingEnable(nullptr, 0); //NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE

	return kinectSensor;
}

bool KinectV1Handler::acquireKinectFrame(NUI_IMAGE_FRAME& imageFrame, HANDLE& rgbStream, INuiSensor*& sensor)
{
	return (sensor->NuiImageStreamGetNextFrame(rgbStream, 1, &imageFrame) < 0);
}

void KinectV1Handler::releaseKinectFrame(NUI_IMAGE_FRAME& imageFrame, HANDLE& rgbStream, INuiSensor*& sensor)
{
	sensor->NuiImageStreamReleaseFrame(rgbStream, &imageFrame);
}

void KinectV1Handler::updateSkeletalData()
{
	if (kinectSensor->NuiSkeletonGetNextFrame(0, &skeletonFrame) >= 0)
	{
		// Parameters for ms' filter.
		// There may be a need to experiment with it,
		// Since it's the first filter happening

		/* We have our own filters */
		//NUI_TRANSFORM_SMOOTH_PARAMETERS params;
		///*
		//params.fCorrection = .25f;
		//params.fJitterRadius = .4f;
		//params.fMaxDeviationRadius = .25f;
		//params.fPrediction = .25f;
		//params.fSmoothing = .25f;
		////*/
		/////*
		//params.fSmoothing = .15f;
		//params.fCorrection = .25f;
		//params.fMaxDeviationRadius = .17f;
		//params.fJitterRadius = .11f;
		//params.fPrediction = .17f;
		////*/

		//kinectSensor->NuiTransformSmooth(&skeletonFrame, &params); //Smooths jittery tracking
		/* We have our own filters */

		for (int i = 0; i < NUI_SKELETON_COUNT; ++i)
		{
			NUI_SKELETON_TRACKING_STATE trackingState = skeletonFrame.SkeletonData[i].eTrackingState;

			if (NUI_SKELETON_TRACKED == trackingState)
			{
				skeletonTracked = true; // We've got it!
				/* Copy joint positions */
				for (int j = 0; j < ktvr::Joint_Total; ++j)
				{
					//TrackingDeviceBase::jointPositions[j].w() = skeletonFrame.SkeletonData[i].SkeletonPositions[globalIndex[j]].w;
					jointPositions[j].x() = skeletonFrame.SkeletonData[i].
						SkeletonPositions[
							globalIndex[j]].x;
					jointPositions[j].y() = skeletonFrame.SkeletonData[i].
						SkeletonPositions[
							globalIndex[j]].y;
					jointPositions[j].z() = skeletonFrame.SkeletonData[i].
						SkeletonPositions[
							globalIndex[j]].z;

					trackingStates[j] = skeletonFrame.SkeletonData[i].
						eSkeletonPositionTrackingState
						[globalIndex[j]];
				}

				// Calculate bone orientations (deprecated, may be replaced later)
				NuiSkeletonCalculateBoneOrientations(&skeletonFrame.SkeletonData[i], boneOrientations);

				/* Copy joint orientations */
				for (int k = 0; k < ktvr::Joint_Total; ++k)
				{
					jointOrientations[k].w() = boneOrientations[globalIndex[k]].
					                           absoluteRotation.
					                           rotationQuaternion.w;
					jointOrientations[k].x() = boneOrientations[globalIndex[k]].
					                           absoluteRotation.
					                           rotationQuaternion.x;
					jointOrientations[k].y() = boneOrientations[globalIndex[k]].
					                           absoluteRotation.
					                           rotationQuaternion.y;
					jointOrientations[k].z() = boneOrientations[globalIndex[k]].
					                           absoluteRotation.
					                           rotationQuaternion.z;
				}
				break; // Only first skeleton
			}
			skeletonTracked = false;
		}
	}
}
