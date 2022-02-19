#include "pch.h"
#include "KinectV2Handler.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <Ole2.h>

#include <ppl.h>
#include <thread>
#include <chrono>

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

std::string KinectV2Handler::statusResultString(HRESULT stat)
{
	switch (stat)
	{
	case S_OK: return "Success!\nS_OK\nEverything's good!";
	case S_FALSE: return
			"Sensor Unavailable!\nE_NOTAVAILABLE\nCheck if the Kinect plugged in to your PC's USB and power plugs.";
	default: return "Undefined: " + std::to_string(stat) +
			"\nE_UNDEFINED\nSomething weird has happened, though we can't tell what.";
	}
}

void KinectV2Handler::initialize()
{
	try
	{
		initialized = initKinect();
		// initializeColor();
		// Commented both image frames out, as most people use the kinect for skeletal data
		// Updating all of the arrays uses a shit ton of CPU, but then again, it's still WIP
		// initializeDepth();
		initializeSkeleton();
		if (!initialized) throw FailedKinectInitialization;
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
		__except (EXCEPTION_INVALID_HANDLE)
		{
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
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) // Unneccesary?
			{
				DispatchMessage(&msg);
			}

			if (h_bodyFrameEvent)
			{
				//printf("Kinect Event ID: %d\n" ,(int)h_bodyFrameEvent);

				//now check for IR Events
				HANDLE handles[] = {reinterpret_cast<HANDLE>(h_bodyFrameEvent)};
				// , reinterpret_cast<HANDLE>(ke.hMSEvent)		};

				switch (MsgWaitForMultipleObjects(_countof(handles), handles, false, 0, QS_ALLINPUT))
				{
				case WAIT_OBJECT_0:
					{
						IBodyFrameArrivedEventArgs* pArgs = nullptr;
						//printf("Body Frame Event Signaled.\n");

						if (bodyFrameReader)
						{
							HRESULT hr = bodyFrameReader->GetFrameArrivedEventData(h_bodyFrameEvent, &pArgs);
							//printf("Retreive Frame Arrive Event Data -HR: %d\n", hr);

							if (SUCCEEDED(hr))
							{
								//printf("Retreived Frame Arrived Event Data\n");
								onBodyFrameArrived(*bodyFrameReader, *pArgs);
								pArgs->Release();
								//printf("Frame Arrived Event Data Released\n");
							}
						}
					}
					break;
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
		HRESULT frameReceived = bodyFrameReader->AcquireLatestFrame(&bodyFrame);
		if (FAILED(frameReceived))
		{
			if (frameReceived == E_PENDING)
			{
				// LOG(INFO) << "Could not retrieve skeleton frame, stuck pending...";
			}
		}
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
	for (int i = 0; i < BODY_COUNT; i++)
	{
		if (kinectBodies[i])
			kinectBodies[i]->get_IsTracked(&isTracking);
		if (isTracking)
		{
			skeletonTracked = true;
			kinectBodies[i]->GetJoints(JointType_Count, joints);
			kinectBodies[i]->GetJointOrientations(JointType_Count, boneOrientations);

			/* Copy joint positions */
			for (int j = 0; j < ktvr::Joint_Total; ++j)
			{
				//TrackingDeviceBase::jointPositions[j].w = 1.0f;
				jointPositions[j].x() = joints[globalIndex[j]].Position.X;
				jointPositions[j].y() = joints[globalIndex[j]].Position.Y;
				jointPositions[j].z() = joints[globalIndex[j]].Position.Z;

				trackingStates[j] = joints[globalIndex[j]].TrackingState;
			}

			/* Copy joint orientations */
			for (int k = 0; k < ktvr::Joint_Total; ++k)
			{
				// Don't copy ankle rotations - we're SLERP'ing them
				if (k == ktvr::Joint_AnkleLeft || k == JointType_AnkleRight ||
					k == ktvr::Joint_ElbowLeft || k == ktvr::Joint_ElbowRight ||
					k == ktvr::Joint_KneeLeft || k == ktvr::Joint_KneeRight)continue;

				jointOrientations[k].w() = boneOrientations[globalIndex[k]].Orientation.
				                                                            w;
				jointOrientations[k].x() = boneOrientations[globalIndex[k]].Orientation.
				                                                            x;
				jointOrientations[k].y() = boneOrientations[globalIndex[k]].Orientation.
				                                                            y;
				jointOrientations[k].z() = boneOrientations[globalIndex[k]].Orientation.
				                                                            z;
			}

			/* Fix feet orientation */

			// Yes, dear mathematician...
			// I'm applying the main rotation to the actual offset, quite the reverse innit?
			// So, MS has decided that we're all are crabs. No jokes here. We're damn crabs.
			// And so, I've decided to ditch this trend and still stay vampi.. I mean human.

			// Anyway, slerp is slowing down the afterparty kinect v2 which wants everyone
			// to shake and express self greatness 
			// (or just tracks every damn molecule which shakes due to the internal heat)

			jointOrientations[ktvr::Joint_AnkleLeft] =
				jointOrientations[ktvr::Joint_AnkleLeft].slerp(
					0.7f, Eigen::Quaternionf(
						boneOrientations[globalIndex[ktvr::Joint_AnkleLeft]].
						Orientation.w,
						boneOrientations[globalIndex[ktvr::Joint_AnkleLeft]].
						Orientation.x,
						boneOrientations[globalIndex[ktvr::Joint_AnkleLeft]].
						Orientation.y,
						boneOrientations[globalIndex[ktvr::Joint_AnkleLeft]].
						Orientation.z
					)) *
				Eigen::Quaternionf(Eigen::AngleAxisf(0.f, Eigen::Vector3f::UnitX())
					* Eigen::AngleAxisf(3.14159265358979323846 / 2.0, Eigen::Vector3f::UnitY())
					* Eigen::AngleAxisf(0.f, Eigen::Vector3f::UnitZ()));

			jointOrientations[ktvr::Joint_AnkleRight] =
				jointOrientations[ktvr::Joint_AnkleRight].slerp(
					0.7f, Eigen::Quaternionf(
						boneOrientations[globalIndex[
							ktvr::Joint_AnkleRight]].Orientation.w,
						boneOrientations[globalIndex[
							ktvr::Joint_AnkleRight]].Orientation.x,
						boneOrientations[globalIndex[
							ktvr::Joint_AnkleRight]].Orientation.y,
						boneOrientations[globalIndex[
							ktvr::Joint_AnkleRight]].Orientation.z
					)) *
				Eigen::Quaternionf(Eigen::AngleAxisf(0.f, Eigen::Vector3f::UnitX())
					* Eigen::AngleAxisf(-3.14159265358979323846 / 2.0, Eigen::Vector3f::UnitY())
					* Eigen::AngleAxisf(0.f, Eigen::Vector3f::UnitZ()));

			jointOrientations[ktvr::Joint_KneeLeft] =
				jointOrientations[ktvr::Joint_KneeLeft].slerp(
					0.7f, Eigen::Quaternionf(
						boneOrientations[globalIndex[ktvr::Joint_KneeLeft]].
						Orientation.w,
						boneOrientations[globalIndex[ktvr::Joint_KneeLeft]].
						Orientation.x,
						boneOrientations[globalIndex[ktvr::Joint_KneeLeft]].
						Orientation.y,
						boneOrientations[globalIndex[ktvr::Joint_KneeLeft]].
						Orientation.z
					)) *
				Eigen::Quaternionf(Eigen::AngleAxisf(0.f, Eigen::Vector3f::UnitX())
					* Eigen::AngleAxisf(3.14159265358979323846 / 2.0, Eigen::Vector3f::UnitY())
					* Eigen::AngleAxisf(0.f, Eigen::Vector3f::UnitZ()));

			jointOrientations[ktvr::Joint_KneeRight] =
				jointOrientations[ktvr::Joint_KneeRight].slerp(
					0.7f, Eigen::Quaternionf(
						boneOrientations[globalIndex[
							ktvr::Joint_KneeRight]].Orientation.w,
						boneOrientations[globalIndex[
							ktvr::Joint_KneeRight]].Orientation.x,
						boneOrientations[globalIndex[
							ktvr::Joint_KneeRight]].Orientation.y,
						boneOrientations[globalIndex[
							ktvr::Joint_KneeRight]].Orientation.z
					)) *
				Eigen::Quaternionf(Eigen::AngleAxisf(0.f, Eigen::Vector3f::UnitX())
					* Eigen::AngleAxisf(-3.14159265358979323846 / 2.0, Eigen::Vector3f::UnitY())
					* Eigen::AngleAxisf(0.f, Eigen::Vector3f::UnitZ()));

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
		std::this_thread::sleep_for(std::chrono::seconds(2));
		// Necessary to allow kinect to become available behind the scenes

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
		// Release the Kinect sensor, called form k2vr
		// OR from the crash handler
		if (kinectSensor)
		{
			// Protect from null call
			terminateSkeleton();

			kinectSensor->Close();
			kinectSensor->Release();
		}
	}
	catch (std::exception& e)
	{
	}
}
