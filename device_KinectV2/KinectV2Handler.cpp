#include "pch.h"
#include "KinectV2Handler.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <Ole2.h>

#include <ppl.h>
#include <thread>
#include <chrono>

#define _PI 3.14159265358979323846

HRESULT KinectV2Handler::getStatusResult()
{
	BOOLEAN avail;
	if (kinectSensor) // Protect from null call
	{
		kinectSensor->get_IsAvailable(&avail);
		if (avail)
			return S_OK;
	}
	return S_FALSE;
}

std::wstring KinectV2Handler::statusResultWString(HRESULT stat)
{
	// Wrap status to string for readability
	if (_loaded)
		switch (stat)
		{
		case S_OK: return requestLocalizedString(L"/Plugins/KinectV2/Statuses/Success");
		case S_FALSE: return requestLocalizedString(L"/Plugins/KinectV2/Statuses/NotAvailable");
		default: return L"Undefined: " + std::to_wstring(stat) +
				L"\nE_UNDEFINED\nSomething weird has happened, though we can't tell what.";
		}

	return L"Undefined: " + std::to_wstring(stat) +
		L"\nE_UNDEFINED\nSomething weird has happened, though we can't tell what.";
}

void KinectV2Handler::initialize()
{
	try
	{
		initialized = initKinect();
		LOG(INFO) << "Initializing: updated Kinect V2 status with: " <<
			WStringToString(statusResultWString(getStatusResult()));
		
		initializeSkeleton();
		if (!initialized) throw FailedKinectInitialization;

		// Recreate the updater thread
		if (!m_updater_thread)
			m_updater_thread.reset(new std::thread(
				&KinectV2Handler::updater, this));
	}
	catch (std::exception& e)
	{
	}
}

void KinectV2Handler::initializeSkeleton()
{
	if (bodyFrameReader)
		bodyFrameReader->Release();
	IBodyFrameSource* bodyFrameSource;
	kinectSensor->get_BodyFrameSource(&bodyFrameSource);
	bodyFrameSource->OpenReader(&bodyFrameReader);

	// Newfangled event based frame capture
	// https://github.com/StevenHickson/PCL_Kinect2SDK/blob/master/src/Microsoft_grabber2.cpp
	h_bodyFrameEvent = (WAITABLE_HANDLE)CreateEvent(nullptr, FALSE, FALSE, nullptr);
	HRESULT hr = bodyFrameReader->SubscribeFrameArrived(&h_bodyFrameEvent);
	if (bodyFrameSource) bodyFrameSource->Release();
}

void KinectV2Handler::terminateSkeleton()
{
	if (bodyFrameReader)
	{
		HRESULT hr = bodyFrameReader->UnsubscribeFrameArrived(h_bodyFrameEvent);
		if (FAILED(hr))
		{
			throw std::exception("Couldn't unsubscribe frame!");
		}
		__try
		{
			CloseHandle((HANDLE)h_bodyFrameEvent);
			bodyFrameReader->Release();
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			[&, this]
			{
				LOG(INFO) <<
					"Shutting down: V2 bodyFrameReader's termination failed! The device may misbehave until the next reconnection.";
			}();
		}
		h_bodyFrameEvent = NULL;
		bodyFrameReader = nullptr;
	}
}

void KinectV2Handler::update()
{
	if (isInitialized())
	{
		BOOLEAN isAvailable = false;
		HRESULT kinectStatus = kinectSensor->get_IsAvailable(&isAvailable);
		if (kinectStatus == S_OK)
		{
			// NEW ARRIVED FRAMES ------------------------
			MSG msg;
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) // Unnecessary?
				DispatchMessage(&msg);

			if (h_bodyFrameEvent)
				if (HANDLE handles[] = {reinterpret_cast<HANDLE>(h_bodyFrameEvent)};
					// Wait for a frame to arrive, give up after >3s of nothing
					MsgWaitForMultipleObjects(_countof(handles), handles,
					                          false, 3000, QS_ALLINPUT) == WAIT_OBJECT_0)
				{
					IBodyFrameArrivedEventArgs* pArgs = nullptr;
					if (bodyFrameReader &&
						SUCCEEDED(bodyFrameReader->GetFrameArrivedEventData(h_bodyFrameEvent, &pArgs)))
					{
						onBodyFrameArrived(*bodyFrameReader, *pArgs);
						pArgs->Release(); // Release the frame
					}
				}
		}
	}
}

void KinectV2Handler::onBodyFrameArrived(IBodyFrameReader& sender, IBodyFrameArrivedEventArgs& eventArgs)
{
	updateSkeletalData();
}

void KinectV2Handler::updateSkeletalData()
{
	if (bodyFrameReader)
	{
		IBodyFrame* bodyFrame = nullptr;
		const HRESULT frameReceived = bodyFrameReader->AcquireLatestFrame(&bodyFrame);
		if (FAILED(frameReceived) && frameReceived == E_PENDING)
			LOG(WARNING) << "[KinectV2 Device] Could not retrieve skeleton frame, stuck pending...";

		//IBodyFrameReference* frameRef = nullptr;
		//multiFrame->get_BodyFrameReference(&frameRef);
		//frameRef->AcquireFrame(&bodyFrame);
		//if (frameRef) frameRef->Release();
		//if (bodyFrameReader) bodyFrameReader->Release();
		if (!bodyFrame) return;

		bodyFrame->GetAndRefreshBodyData(BODY_COUNT, kinectBodies);
		newBodyFrameArrived = true;
		if (bodyFrame) bodyFrame->Release();

		// We have the frame, now parse it
		updateParseFrame();
	}
}

void KinectV2Handler::updateParseFrame()
{
	for (const auto& kinect_bodies : kinectBodies)
	{
		if (kinect_bodies)
			kinect_bodies->get_IsTracked(&isTracking);

		if (isTracking)
		{
			skeletonTracked = true;
			kinect_bodies->GetJoints(JointType_Count, joints);
			kinect_bodies->GetJointOrientations(JointType_Count, boneOrientations);

			/* Copy joint positions */
			for (int j = 0; j < ktvr::Joint_Total; ++j)
			{
				trackedJoints[j].update_position(
					{
						joints[globalIndex[j]].Position.X,
						joints[globalIndex[j]].Position.Y,
						joints[globalIndex[j]].Position.Z
					});

				trackedJoints[j].update_state(
					static_cast<ktvr::ITrackedJointState>(joints[globalIndex[j]].TrackingState));
			}

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
						boneOrientations[globalIndex[k]].Orientation.w,
						boneOrientations[globalIndex[k]].Orientation.x,
						boneOrientations[globalIndex[k]].Orientation.y,
						boneOrientations[globalIndex[k]].Orientation.z
					});
			}

			/* Fix feet orientation */

			// Yes, dear mathematician...
			// I'm applying the main rotation to the actual offset, quite the reverse innit?
			// So, MS has decided that we're all are crabs. No jokes here. We're damn crabs.
			// And so, I've decided to ditch this trend and still stay vampi.. I mean human.

			// Anyway, slerp is slowing down the afterparty kinect v2 which wants everyone
			// to shake and express self greatness 
			// (or just tracks every damn molecule which shakes due to the internal heat)

			trackedJoints[ktvr::Joint_AnkleLeft].update_orientation(
				trackedJoints[ktvr::Joint_AnkleLeft].getJointOrientation().slerp(
					0.3, Eigen::Quaterniond(
						boneOrientations[globalIndex[ktvr::Joint_AnkleLeft]].
						Orientation.w,
						boneOrientations[globalIndex[ktvr::Joint_AnkleLeft]].
						Orientation.x,
						boneOrientations[globalIndex[ktvr::Joint_AnkleLeft]].
						Orientation.y,
						boneOrientations[globalIndex[ktvr::Joint_AnkleLeft]].
						Orientation.z
					) *
					Eigen::Quaterniond(Eigen::AngleAxisd(0., Eigen::Vector3d::UnitX())
						* Eigen::AngleAxisd(_PI / 2., Eigen::Vector3d::UnitY())
						* Eigen::AngleAxisd(0., Eigen::Vector3d::UnitZ()))));

			trackedJoints[ktvr::Joint_AnkleRight].update_orientation(
				trackedJoints[ktvr::Joint_AnkleRight].getJointOrientation().slerp(
					0.3, Eigen::Quaterniond(
						boneOrientations[globalIndex[
							ktvr::Joint_AnkleRight]].Orientation.w,
						boneOrientations[globalIndex[
							ktvr::Joint_AnkleRight]].Orientation.x,
						boneOrientations[globalIndex[
							ktvr::Joint_AnkleRight]].Orientation.y,
						boneOrientations[globalIndex[
							ktvr::Joint_AnkleRight]].Orientation.z
					) *
					Eigen::Quaterniond(Eigen::AngleAxisd(0., Eigen::Vector3d::UnitX())
						* Eigen::AngleAxisd(-_PI / 2., Eigen::Vector3d::UnitY())
						* Eigen::AngleAxisd(0., Eigen::Vector3d::UnitZ()))));

			trackedJoints[ktvr::Joint_KneeLeft].update_orientation(
				trackedJoints[ktvr::Joint_KneeLeft].getJointOrientation().slerp(
					0.35, Eigen::Quaterniond(
						boneOrientations[globalIndex[ktvr::Joint_KneeLeft]].
						Orientation.w,
						boneOrientations[globalIndex[ktvr::Joint_KneeLeft]].
						Orientation.x,
						boneOrientations[globalIndex[ktvr::Joint_KneeLeft]].
						Orientation.y,
						boneOrientations[globalIndex[ktvr::Joint_KneeLeft]].
						Orientation.z
					) *
					Eigen::Quaterniond(Eigen::AngleAxisd(0., Eigen::Vector3d::UnitX())
						* Eigen::AngleAxisd(_PI / 3., Eigen::Vector3d::UnitY())
						* Eigen::AngleAxisd(0., Eigen::Vector3d::UnitZ()))));

			trackedJoints[ktvr::Joint_KneeRight].update_orientation(
				trackedJoints[ktvr::Joint_KneeRight].getJointOrientation().slerp(
					0.35, Eigen::Quaterniond(
						boneOrientations[globalIndex[
							ktvr::Joint_KneeRight]].Orientation.w,
						boneOrientations[globalIndex[
							ktvr::Joint_KneeRight]].Orientation.x,
						boneOrientations[globalIndex[
							ktvr::Joint_KneeRight]].Orientation.y,
						boneOrientations[globalIndex[
							ktvr::Joint_KneeRight]].Orientation.z
					) *
					Eigen::Quaterniond(Eigen::AngleAxisd(0., Eigen::Vector3d::UnitX())
						* Eigen::AngleAxisd(-_PI / 3., Eigen::Vector3d::UnitY())
						* Eigen::AngleAxisd(0., Eigen::Vector3d::UnitZ()))));

			trackedJoints[ktvr::Joint_ElbowLeft].update_orientation(
				trackedJoints[ktvr::Joint_ElbowLeft].getJointOrientation().slerp(
					0.35, Eigen::Quaterniond(
						boneOrientations[globalIndex[ktvr::Joint_ElbowLeft]].
						Orientation.w,
						boneOrientations[globalIndex[ktvr::Joint_ElbowLeft]].
						Orientation.x,
						boneOrientations[globalIndex[ktvr::Joint_ElbowLeft]].
						Orientation.y,
						boneOrientations[globalIndex[ktvr::Joint_ElbowLeft]].
						Orientation.z
					)));

			trackedJoints[ktvr::Joint_ElbowRight].update_orientation(
				trackedJoints[ktvr::Joint_ElbowRight].getJointOrientation().slerp(
					0.35, Eigen::Quaterniond(
						boneOrientations[globalIndex[
							ktvr::Joint_ElbowRight]].Orientation.w,
						boneOrientations[globalIndex[
							ktvr::Joint_ElbowRight]].Orientation.x,
						boneOrientations[globalIndex[
							ktvr::Joint_ElbowRight]].Orientation.y,
						boneOrientations[globalIndex[
							ktvr::Joint_ElbowRight]].Orientation.z
					)));

			newBodyFrameArrived = false;
			break; // Only first skeleton
		}
		skeletonTracked = false;
	}
}

bool KinectV2Handler::initKinect()
{
	if (FAILED(GetDefaultKinectSensor(&kinectSensor)))
	{
		return false;
	}
	if (kinectSensor)
	{
		kinectSensor->get_CoordinateMapper(&coordMapper);

		HRESULT hr_open = kinectSensor->Open();
		//kinectSensor->OpenMultiSourceFrameReader( FrameSourceTypes::FrameSourceTypes_Body| FrameSourceTypes::FrameSourceTypes_Depth
		//    | FrameSourceTypes::FrameSourceTypes_Color,
		//   &frameReader);
		//return frameReader;

		// Necessary to allow kinect to become available behind the scenes
		std::this_thread::sleep_for(std::chrono::seconds(2));

		BOOLEAN available = false;
		kinectSensor->get_IsAvailable(&available);

		if (FAILED(hr_open) || !available)
		{
			return false;
		}
		return true;
	}
	return false;
}

void KinectV2Handler::shutdown()
{
	try
	{
		LOG(INFO) << "Shutting down: Kinect V2 streams' termination pending...";

		// Release the Kinect sensor, called form k2vr
		// OR from the crash handler
		if (kinectSensor)
		{
			// Protect from null call
			terminateSkeleton();

			[&, this]
			{
				__try
				{
					initialized = false;

					kinectSensor->Close();
					kinectSensor->Release();

					kinectSensor = nullptr;
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					[&,this]
					{
						LOG(INFO) <<
							"Shutting down: V2 kinectSensor's termination failed! The device may misbehave until the next reconnection.";
					}();
				}
			}();
		}
	}
	catch (const std::exception& e)
	{
		LOG(INFO) <<
			"Shutting down: Kinect V2 streams' termination failed! The device may misbehave until the next reconnection.";
	}
}
