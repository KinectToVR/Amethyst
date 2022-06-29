#include "pch.h"
#include "KinectV1Handler.h"
#include "LocalizedStatuses.h"

#define _PI 3.14159265358979323846

HRESULT KinectV1Handler::getStatusResult()
{
	if (kinectSensor)
	{
		const auto res = kinectSensor->NuiStatus();

		if (_loaded)
		{
			settingsSupported = (res == S_OK);

			m_elevation_label->Visibility(res == S_OK);
			m_elevation_spinner->Visibility(res == S_OK);
		}

		if (res == S_OK)
		{
			NuiCameraElevationGetAngle(reinterpret_cast<long*>(&sensorAngle));
			save_settings();
		}

		return res;
	}
	return E_NUI_NOTCONNECTED;
}

std::wstring KinectV1Handler::statusResultWString(HRESULT stat)
{
	// Wrap status to string for readability
	switch (stat)
	{
	case S_OK: return GetLocalizedStatusWStringAutomatic(status_ok_map);
	case S_NUI_INITIALIZING: return GetLocalizedStatusWStringAutomatic(status_initializing_map);
	case E_NUI_NOTCONNECTED: return GetLocalizedStatusWStringAutomatic(status_not_connected_map);
	case E_NUI_NOTGENUINE: return GetLocalizedStatusWStringAutomatic(status_not_genuine_map);
	case E_NUI_NOTSUPPORTED: return GetLocalizedStatusWStringAutomatic(status_not_supported_map);
	case E_NUI_INSUFFICIENTBANDWIDTH: return GetLocalizedStatusWStringAutomatic(status_insufficient_bandwidth_map);
	case E_NUI_NOTPOWERED: return GetLocalizedStatusWStringAutomatic(status_not_powered_map);
	case E_NUI_NOTREADY: return GetLocalizedStatusWStringAutomatic(status_not_ready_map);
	default: return L"Undefined: " + std::to_wstring(stat) +
			L"\nE_UNDEFINED\nSomething weird has happened, though we can't tell what.";
	}
}

void KinectV1Handler::initialize()
{
	try
	{
		initialized = initKinect();

		LOG(INFO) << "Initializing: updated Kinect V1 status with: " <<
			WStringToString(statusResultWString(getStatusResult()));

		if (_loaded)
		{
			settingsSupported = (getStatusResult() == S_OK);

			m_elevation_label->Visibility(getStatusResult() == S_OK);
			m_elevation_spinner->Visibility(getStatusResult() == S_OK);
		}

		if (getStatusResult() == S_OK)
		{
			NuiCameraElevationGetAngle(reinterpret_cast<long*>(&sensorAngle));
			save_settings();
		}

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
		LOG(INFO) << "Shutting down: Kinect V1 streams' termination pending...";
		settingsSupported = false; // Hide

		// Shut down the sensor (Only NUI API)
		if (kinectSensor) // Protect from null call
			[&, this]
			{
				__try
				{
					kinectSensor->NuiShutdown();

					initialized = false;
					kinectSensor = nullptr;
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					[&, this]
					{
						LOG(INFO) <<
							"Shutting down: V1 kinectSensor's termination failed! The device may misbehave until the next reconnection.";
					}();
				}
			}();
	}
	catch (std::exception& e)
	{
		LOG(INFO) <<
			"Shutting down: Kinect V1 streams' termination failed! The device may misbehave until the next reconnection.";
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
					// Don't copy ankle rotations - we're SLERP'ing them
					if (k == ktvr::Joint_AnkleLeft || k == ktvr::Joint_AnkleRight ||
						k == ktvr::Joint_ElbowLeft || k == ktvr::Joint_ElbowRight ||
						k == ktvr::Joint_KneeLeft || k == ktvr::Joint_KneeRight)
						continue;

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

				// Anyway, slerp is slowing down the afterparty kinect v1 which wants everyone
				// to shake and express self greatness 
				// (or just tracks every damn molecule which shakes due to the internal heat)

				jointOrientations[ktvr::Joint_AnkleLeft] =
					jointOrientations[ktvr::Joint_AnkleLeft].slerp(
						0.3f, Eigen::Quaternionf(
							boneOrientations[globalIndex[ktvr::Joint_AnkleLeft]].
							absoluteRotation.rotationQuaternion.w,
							boneOrientations[globalIndex[ktvr::Joint_AnkleLeft]].
							absoluteRotation.rotationQuaternion.x,
							boneOrientations[globalIndex[ktvr::Joint_AnkleLeft]].
							absoluteRotation.rotationQuaternion.y,
							boneOrientations[globalIndex[ktvr::Joint_AnkleLeft]].
							absoluteRotation.rotationQuaternion.z
						));

				jointOrientations[ktvr::Joint_AnkleRight] =
					jointOrientations[ktvr::Joint_AnkleRight].slerp(
						0.3f, Eigen::Quaternionf(
							boneOrientations[globalIndex[
								ktvr::Joint_AnkleRight]].absoluteRotation.rotationQuaternion.w,
							boneOrientations[globalIndex[
								ktvr::Joint_AnkleRight]].absoluteRotation.rotationQuaternion.x,
							boneOrientations[globalIndex[
								ktvr::Joint_AnkleRight]].absoluteRotation.rotationQuaternion.y,
							boneOrientations[globalIndex[
								ktvr::Joint_AnkleRight]].absoluteRotation.rotationQuaternion.z
						));

				jointOrientations[ktvr::Joint_KneeLeft] =
					jointOrientations[ktvr::Joint_KneeLeft].slerp(
						0.35f, Eigen::Quaternionf(
							boneOrientations[globalIndex[ktvr::Joint_KneeLeft]].
							absoluteRotation.rotationQuaternion.w,
							boneOrientations[globalIndex[ktvr::Joint_KneeLeft]].
							absoluteRotation.rotationQuaternion.x,
							boneOrientations[globalIndex[ktvr::Joint_KneeLeft]].
							absoluteRotation.rotationQuaternion.y,
							boneOrientations[globalIndex[ktvr::Joint_KneeLeft]].
							absoluteRotation.rotationQuaternion.z
						));

				jointOrientations[ktvr::Joint_KneeRight] =
					jointOrientations[ktvr::Joint_KneeRight].slerp(
						0.35f, Eigen::Quaternionf(
							boneOrientations[globalIndex[
								ktvr::Joint_KneeRight]].absoluteRotation.rotationQuaternion.w,
							boneOrientations[globalIndex[
								ktvr::Joint_KneeRight]].absoluteRotation.rotationQuaternion.x,
							boneOrientations[globalIndex[
								ktvr::Joint_KneeRight]].absoluteRotation.rotationQuaternion.y,
							boneOrientations[globalIndex[
								ktvr::Joint_KneeRight]].absoluteRotation.rotationQuaternion.z
						));

				jointOrientations[ktvr::Joint_ElbowLeft] =
					jointOrientations[ktvr::Joint_ElbowLeft].slerp(
						0.35f, Eigen::Quaternionf(
							boneOrientations[globalIndex[ktvr::Joint_ElbowLeft]].
							absoluteRotation.rotationQuaternion.w,
							boneOrientations[globalIndex[ktvr::Joint_ElbowLeft]].
							absoluteRotation.rotationQuaternion.x,
							boneOrientations[globalIndex[ktvr::Joint_ElbowLeft]].
							absoluteRotation.rotationQuaternion.y,
							boneOrientations[globalIndex[ktvr::Joint_ElbowLeft]].
							absoluteRotation.rotationQuaternion.z
						) *
						Eigen::Quaternionf(Eigen::AngleAxisf(0.f, Eigen::Vector3f::UnitX())
							* Eigen::AngleAxisf(-_PI / 2.0, Eigen::Vector3f::UnitY())
							* Eigen::AngleAxisf(0.f, Eigen::Vector3f::UnitZ())));

				jointOrientations[ktvr::Joint_ElbowRight] =
					jointOrientations[ktvr::Joint_ElbowRight].slerp(
						0.35f, Eigen::Quaternionf(
							boneOrientations[globalIndex[
								ktvr::Joint_ElbowRight]].absoluteRotation.rotationQuaternion.w,
							boneOrientations[globalIndex[
								ktvr::Joint_ElbowRight]].absoluteRotation.rotationQuaternion.x,
							boneOrientations[globalIndex[
								ktvr::Joint_ElbowRight]].absoluteRotation.rotationQuaternion.y,
							boneOrientations[globalIndex[
								ktvr::Joint_ElbowRight]].absoluteRotation.rotationQuaternion.z
						) *
						Eigen::Quaternionf(Eigen::AngleAxisf(0.f, Eigen::Vector3f::UnitX())
							* Eigen::AngleAxisf(_PI / 2.0, Eigen::Vector3f::UnitY())
							* Eigen::AngleAxisf(0.f, Eigen::Vector3f::UnitZ())));

				break; // Only first skeleton
			}
			skeletonTracked = false;
		}
	}
}
