#include "pch.h"
#include "KinectV1Handler.h"

#define _PI 3.14159265358979323846

HRESULT KinectV1Handler::getStatusResult()
{
	if (kinectSensor)
	{
		const auto res = kinectSensor->NuiStatus();
		Flags_SettingsSupported = (res == S_OK);
		return res;
	}
	return E_NUI_NOTCONNECTED;
}

std::wstring KinectV1Handler::statusResultWString(HRESULT stat)
{
	// Wrap status to string for readability
	if (_loaded)
		switch (stat)
		{
		case S_OK: return requestLocalizedString(L"/Plugins/KinectV1/Statuses/Success");
		case S_NUI_INITIALIZING: return requestLocalizedString(L"/Plugins/KinectV1/Statuses/Initializing");
		case E_NUI_NOTCONNECTED: return requestLocalizedString(L"/Plugins/KinectV1/Statuses/NotConnected");
		case E_NUI_NOTGENUINE: return requestLocalizedString(L"/Plugins/KinectV1/Statuses/NotGenuine");
		case E_NUI_NOTSUPPORTED: return requestLocalizedString(L"/Plugins/KinectV1/Statuses/NotSupported");
		case E_NUI_INSUFFICIENTBANDWIDTH: return requestLocalizedString(
				L"/Plugins/KinectV1/Statuses/InsufficientBandwidth");
		case E_NUI_NOTPOWERED: return requestLocalizedString(L"/Plugins/KinectV1/Statuses/NotPowered");
		case E_NUI_NOTREADY: return requestLocalizedString(L"/Plugins/KinectV1/Statuses/NotReady");
		default: return L"Undefined: " + std::to_wstring(stat) +
				L"\nE_UNDEFINED\nSomething weird has happened, though we can't tell what.";
		}

	return L"Undefined: " + std::to_wstring(stat) +
		L"\nE_UNDEFINED\nSomething weird has happened, though we can't tell what.";
}

void KinectV1Handler::initialize()
{
	try
	{
		initialized = initKinect();

		LOG(INFO) << "Initializing: updated Kinect V1 status with: " <<
			WStringToString(statusResultWString(getStatusResult()));

		Flags_SettingsSupported = (getStatusResult() == S_OK);

		if (getStatusResult() == S_OK)
		{
			NuiCameraElevationGetAngle(reinterpret_cast<long*>(&sensorAngle));
			save_settings();
		}

		if (!initialized) throw FailedKinectInitialization;

		// Recreate the updater thread
		if (!m_updater_thread)
			m_updater_thread.reset(new std::thread(
				&KinectV1Handler::updater, this));
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
		Flags_SettingsSupported = false; // Hide

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
	// Update (only if the sensor is on and its status is ok)
	if (isInitialized() && kinectSensor->NuiStatus() == S_OK)
		updateSkeletalData();
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
	// Wait for a frame to arrive, give up after >3s of nothing
	if (kinectSensor->NuiSkeletonGetNextFrame(3000, &skeletonFrame) >= 0)
		for (auto& i : skeletonFrame.SkeletonData)
		{
			if (NUI_SKELETON_TRACKED == i.eTrackingState)
			{
				skeletonTracked = true; // We've got it!

				/* Copy joint positions */
				for (int j = 0; j < ktvr::Joint_Total; ++j)
				{
					trackedJoints[j].update_position(
						{
							i.SkeletonPositions[globalIndex[j]].x,
							i.SkeletonPositions[globalIndex[j]].y,
							i.SkeletonPositions[globalIndex[j]].z
						});

					trackedJoints[j].update_state(
						static_cast<ktvr::ITrackedJointState>(
							i.
							eSkeletonPositionTrackingState
							[globalIndex[j]]));
				}

				// Calculate bone orientations (deprecated, may be replaced later)
				NuiSkeletonCalculateBoneOrientations(&i, boneOrientations);

				/* Copy joint orientations */
				for (int k = 0; k < ktvr::Joint_Total; ++k)
				{
					// Don't copy ankle rotations - we're SLERP'ing them
					if (k == ktvr::Joint_AnkleLeft || k == ktvr::Joint_AnkleRight ||
						k == ktvr::Joint_ElbowLeft || k == ktvr::Joint_ElbowRight ||
						k == ktvr::Joint_KneeLeft || k == ktvr::Joint_KneeRight)
						continue;

					trackedJoints[k].update_orientation(
						{
							boneOrientations[globalIndex[k]].absoluteRotation.rotationQuaternion.w,
							boneOrientations[globalIndex[k]].absoluteRotation.rotationQuaternion.x,
							boneOrientations[globalIndex[k]].absoluteRotation.rotationQuaternion.y,
							boneOrientations[globalIndex[k]].absoluteRotation.rotationQuaternion.z
						});
				}

				// Anyway, slerp is slowing down the afterparty kinect v1 which wants everyone
				// to shake and express self greatness 
				// (or just tracks every damn molecule which shakes due to the internal heat)

				trackedJoints[ktvr::Joint_AnkleLeft].update_orientation(
					trackedJoints[ktvr::Joint_AnkleLeft].getJointOrientation().slerp(
						0.3, Eigen::Quaterniond(
							boneOrientations[globalIndex[ktvr::Joint_AnkleLeft]].
							absoluteRotation.rotationQuaternion.w,
							boneOrientations[globalIndex[ktvr::Joint_AnkleLeft]].
							absoluteRotation.rotationQuaternion.x,
							boneOrientations[globalIndex[ktvr::Joint_AnkleLeft]].
							absoluteRotation.rotationQuaternion.y,
							boneOrientations[globalIndex[ktvr::Joint_AnkleLeft]].
							absoluteRotation.rotationQuaternion.z
						)));

				trackedJoints[ktvr::Joint_AnkleRight].update_orientation(
					trackedJoints[ktvr::Joint_AnkleRight].getJointOrientation().slerp(
						0.3, Eigen::Quaterniond(
							boneOrientations[globalIndex[
								ktvr::Joint_AnkleRight]].absoluteRotation.rotationQuaternion.w,
							boneOrientations[globalIndex[
								ktvr::Joint_AnkleRight]].absoluteRotation.rotationQuaternion.x,
							boneOrientations[globalIndex[
								ktvr::Joint_AnkleRight]].absoluteRotation.rotationQuaternion.y,
							boneOrientations[globalIndex[
								ktvr::Joint_AnkleRight]].absoluteRotation.rotationQuaternion.z
						)));

				trackedJoints[ktvr::Joint_KneeLeft].update_orientation(
					trackedJoints[ktvr::Joint_KneeLeft].getJointOrientation().slerp(
						0.35, Eigen::Quaterniond(
							boneOrientations[globalIndex[ktvr::Joint_KneeLeft]].
							absoluteRotation.rotationQuaternion.w,
							boneOrientations[globalIndex[ktvr::Joint_KneeLeft]].
							absoluteRotation.rotationQuaternion.x,
							boneOrientations[globalIndex[ktvr::Joint_KneeLeft]].
							absoluteRotation.rotationQuaternion.y,
							boneOrientations[globalIndex[ktvr::Joint_KneeLeft]].
							absoluteRotation.rotationQuaternion.z
						)));

				trackedJoints[ktvr::Joint_KneeRight].update_orientation(
					trackedJoints[ktvr::Joint_KneeRight].getJointOrientation().slerp(
						0.35, Eigen::Quaterniond(
							boneOrientations[globalIndex[
								ktvr::Joint_KneeRight]].absoluteRotation.rotationQuaternion.w,
							boneOrientations[globalIndex[
								ktvr::Joint_KneeRight]].absoluteRotation.rotationQuaternion.x,
							boneOrientations[globalIndex[
								ktvr::Joint_KneeRight]].absoluteRotation.rotationQuaternion.y,
							boneOrientations[globalIndex[
								ktvr::Joint_KneeRight]].absoluteRotation.rotationQuaternion.z
						)));

				trackedJoints[ktvr::Joint_ElbowLeft].update_orientation(
					trackedJoints[ktvr::Joint_ElbowLeft].getJointOrientation().slerp(
						0.35, Eigen::Quaterniond(
							boneOrientations[globalIndex[ktvr::Joint_ElbowLeft]].
							absoluteRotation.rotationQuaternion.w,
							boneOrientations[globalIndex[ktvr::Joint_ElbowLeft]].
							absoluteRotation.rotationQuaternion.x,
							boneOrientations[globalIndex[ktvr::Joint_ElbowLeft]].
							absoluteRotation.rotationQuaternion.y,
							boneOrientations[globalIndex[ktvr::Joint_ElbowLeft]].
							absoluteRotation.rotationQuaternion.z
						) *
						Eigen::Quaterniond(Eigen::AngleAxisd(0., Eigen::Vector3d::UnitX())
							* Eigen::AngleAxisd(-_PI / 2., Eigen::Vector3d::UnitY())
							* Eigen::AngleAxisd(0., Eigen::Vector3d::UnitZ()))));

				trackedJoints[ktvr::Joint_ElbowRight].update_orientation(
					trackedJoints[ktvr::Joint_ElbowRight].getJointOrientation().slerp(
						0.35, Eigen::Quaterniond(
							boneOrientations[globalIndex[
								ktvr::Joint_ElbowRight]].absoluteRotation.rotationQuaternion.w,
							boneOrientations[globalIndex[
								ktvr::Joint_ElbowRight]].absoluteRotation.rotationQuaternion.x,
							boneOrientations[globalIndex[
								ktvr::Joint_ElbowRight]].absoluteRotation.rotationQuaternion.y,
							boneOrientations[globalIndex[
								ktvr::Joint_ElbowRight]].absoluteRotation.rotationQuaternion.z
						) *
						Eigen::Quaterniond(Eigen::AngleAxisd(0., Eigen::Vector3d::UnitX())
							* Eigen::AngleAxisd(_PI / 2., Eigen::Vector3d::UnitY())
							* Eigen::AngleAxisd(0., Eigen::Vector3d::UnitZ()))));

				break; // Only first skeleton
			}
			skeletonTracked = false;
		}
}
