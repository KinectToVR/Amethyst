#pragma once
#include "K2Interfacing.h"
#include "TrackingDevices.h"

namespace k2app::main
{
	inline void K2UpdateVRPositions()
	{
		// TODO GRAB HEAD POS HERE
	}

	inline void K2UpdateInputBindings()
	{
		using namespace interfacing;

		/* Here, update EVR Input actions */

		// Backup the current ( OLD ) data
		const bool bak_mode_swap_state = evr_input.modeSwapActionData().bState,
		           bak_freeze_state = evr_input.trackerFreezeActionData().bState,
		           bak_flip_toggle_state = evr_input.trackerFlipToggleData().bState;

		// Update all input actions
		if (!evr_input.UpdateActionStates())
			LOG(ERROR) << "Could not update EVR Input Actions. Please check logs for further information.";

		// Update the Tracking Freeze : toggle
		// Only if the state has changed from 1 to 0: button was clicked
		if (!evr_input.trackerFreezeActionData().bState && bak_freeze_state)
		{
			LOG(INFO) << "[Input Actions] Input: Tracking freeze toggled.";
			isTrackingFrozen = !isTrackingFrozen;

			// Play a Sound and Update UI
			winrt::Microsoft::UI::Xaml::ElementSoundPlayer::Play(
				winrt::Microsoft::UI::Xaml::ElementSoundKind::Invoke);
		}

		// Update the Flip Toggle : toggle
		// Only if the state has changed from 1 to 0: button was clicked
		if (!evr_input.trackerFlipToggleData().bState && bak_flip_toggle_state)
		{
			LOG(INFO) << "[Input Actions] Input: Flip toggled.";

			// Also validate the result
			if (auto const& trackingDevice = TrackingDevices::getCurrentDevice(); trackingDevice.index() == 0)
			{
				// Kinect Basis
				K2Settings.isFlipEnabled =
					std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isFlipSupported()
						? !K2Settings.isFlipEnabled // If supported
						: false; // If not supported
			}
			else if (trackingDevice.index() == 1)
			{
				// Joints Basis
				K2Settings.isFlipEnabled = !false;
			}

			// Save settings
			K2Settings.saveSettings();

			// Play a Sound and Update UI
			winrt::Microsoft::UI::Xaml::ElementSoundPlayer::Play(
				winrt::Microsoft::UI::Xaml::ElementSoundKind::Invoke);

			if (shared::settings::flipCheckBox.get())
				shared::settings::flipCheckBox.get()->IsChecked(K2Settings.isFlipEnabled);
		}

		// Update the Calibration:Confirm : one-time switch
		// Only one-way switch this time
		if (evr_input.confirmAndSaveActionData().bState)
			calibration_confirm = true;

		/*LOG_IF(evr_input.fineTuneActionData().bState, INFO) << "FineTune ON!";
		LOG_IF(evr_input.fineTuneActionData().bActive, INFO) << "FineTune Action ACTIVE!";*/

		// Update the Calibration:ModeSwap : one-time switch
		// Only if the state has changed from 1 to 0: chord was done
		calibration_modeSwap =
			(!evr_input.modeSwapActionData().bState && bak_mode_swap_state);

		// Update the Calibration:FineTune : held switch
		calibration_fineTune = evr_input.fineTuneActionData().bState;

		// Update the Calibration:Joystick : vector2 x2
		calibration_joystick_positions[0][0] = evr_input.leftJoystickActionData().x;
		calibration_joystick_positions[0][1] = evr_input.leftJoystickActionData().y;

		calibration_joystick_positions[1][0] = evr_input.rightJoystickActionData().x;
		calibration_joystick_positions[1][1] = evr_input.rightJoystickActionData().y;
	}

	inline void K2UpdateTrackingDevices()
	{
		/* Update the base device here */
		switch (auto const& device = TrackingDevices::
			getCurrentDevice(); device.index())
		{
		case 0:
			std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(device)->update();
			break;
		case 1:
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(device)->update();
			break;
		}

		/* Update the override device here (optionally) */
		if (auto const& device_pair = TrackingDevices::
			getCurrentOverrideDevice_Safe(); device_pair.first)
			switch (device_pair.second.index())
			{
			case 0:
				std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(device_pair.second)->update();
				break;
			case 1:
				std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(device_pair.second)->update();
				break;
			}
	}

	inline int p_loops = 0; // Loops passed since last status update
	inline bool initialized_bak = false; // Backup initialized? value
	inline void K2UpdateServerTrackers()
	{
		// Update only if we're connected
		if (interfacing::K2AppTrackersSpawned &&
			!interfacing::serverDriverFailure)
		{
			// Update status 1/1000 loops / ~8s
			// or right after any change
			for (int i = 0; i < 3; i++)
			{
				// try 3 times
				if (p_loops >= 1000 ||
					(initialized_bak != interfacing::K2AppTrackersInitialized))
				{
					// Update status in server

					// Would be in a loop but somehow bugs everything if it is
					if (K2Settings.isJointEnabled[0])
						ktvr::set_tracker_state<false>(interfacing::K2TrackersVector.at(0).id,
						                               K2Settings.isJointTurnedOn[0] &&
						                               interfacing::K2AppTrackersInitialized);
					if (K2Settings.isJointEnabled[1])
						ktvr::set_tracker_state<false>(interfacing::K2TrackersVector.at(1).id,
						                               K2Settings.isJointTurnedOn[1] &&
						                               interfacing::K2AppTrackersInitialized);
					if (K2Settings.isJointEnabled[2])
						ktvr::set_tracker_state<false>(interfacing::K2TrackersVector.at(2).id,
						                               K2Settings.isJointTurnedOn[2] &&
						                               interfacing::K2AppTrackersInitialized);

					// Update internal status
					initialized_bak = interfacing::K2AppTrackersInitialized;

					// Reset
					p_loops = 0;
				}
				else p_loops++;
			}
		}
	}

	// Update trackers inside the app here
	inline void K2UpdateAppTrackers()
	{
		// TODO PARSE AND COMPOSE POSES & ROTS
		// this is gonna take the most time I guess...
	}

	// The main program loop
	inline void K2MainLoop()
	{
		// Warning: this is meant to work as fire-and-forget
		LOG(INFO) << "[K2Main] Waiting for the start sem to open..";
		shared::devices::smphSignalStartMain.acquire();

		LOG(INFO) << "[K2Main] Starting the main app loop now...";
		while (true)
		{
			auto loop_start_time = std::chrono::high_resolution_clock::now();

			/* Update things here */

			K2UpdateVRPositions(); // Update HMD poses
			K2UpdateInputBindings(); // Update input

			K2UpdateTrackingDevices(); // Update actual tracking
			K2UpdateAppTrackers(); // Track joints from raw data

			K2UpdateServerTrackers(); // Send it to the server

			// Wait until certain loop time has passed
			if (auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
					std::chrono::high_resolution_clock::now() - loop_start_time).count();
				duration <= 12222222.f) // If we were too fast, sleep peacefully @80hz
				std::this_thread::sleep_for(std::chrono::nanoseconds(12222222 - duration));
		}
	}
}
