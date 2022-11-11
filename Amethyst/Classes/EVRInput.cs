using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Amethyst.Utils;
using Valve.VR;

namespace Amethyst.Classes;

public static class EVRInput
{
    // Action strings set in action_manifest.json

    // Required
    private const string k_actionSetDefault = "/actions/default"; // Default

    private const string k_actionLeftJoystick = "/actions/default/in/LeftJoystick"; // Left-hand Move/Rotate Controls
    private const string k_actionRightJoystick = "/actions/default/in/RightJoystick"; // Right-hand Move/Rotate Controls

    private const string k_actionConfirmAndSave = "/actions/default/in/ConfirmAndSave"; // Confirm and Save
    private const string k_actionModeSwap = "/actions/default/in/ModeSwap"; // Swap Move/Rotate Modes
    private const string k_actionFineTune = "/actions/default/in/FineTune"; // Fine-tuning

    // Optional
    private const string k_actionTrackerFreeze = "/actions/default/in/TrackerFreeze"; // Freeze Trackers
    private const string k_actionFlipToggle = "/actions/default/in/FlipToggle"; // Toggle Flip

    // Main SteamEVRInput class
    public class SteamEVRInput
    {
        // Action manifest path
        private const string m_actionManifestPath = "action_manifest.json";

        private ulong m_ConfirmAndSaveHandler = 0;
        private ulong m_FineTuneHandler = 0;
        private ulong m_FlipToggleHandler = 0;

        // Calibration actions
        private ulong m_LeftJoystickHandler = 0;
        private ulong m_ModeSwapHandler = 0;
        private ulong m_RightJoystickHandler = 0;

        // Tracking freeze actions
        private ulong m_TrackerFreezeHandler = 0;

        // Buttons data
        private InputDigitalActionData_t
            m_ConfirmAndSaveData,
            m_ModeSwapData,
            m_FineTuneData,
            m_TrackerFreezeData,
            m_FlipToggleData;

        // The action sets
        private VRActiveActionSet_t
            m_defaultActionSet = new();

        // Tracking Default set
        private ulong m_DefaultSetHandler = 0;

        // Analogs data
        private InputAnalogActionData_t
            m_LeftJoystickHandlerData,
            m_RightJoystickHandlerData;

        // Note: SteamVR must be initialized beforehand.
        // Preferred type is (vr::VRApplication_Scene)
        public bool InitInputActions()
        {
            // Find the absolute path of manifest
            var absoluteManifestPath =
                Path.Join(Interfacing.GetProgramLocation().DirectoryName, m_actionManifestPath);

            if (!File.Exists(absoluteManifestPath))
            {
                Logger.Error("Action manifest was not found in the program " +
                             $"({Interfacing.GetProgramLocation().Directory}) directory.");
                return false; // Return failure status
            }

            // Set the action manifest. This should be in the executable directory.
            // Defined by m_actionManifestPath.
            var error = OpenVR.Input.SetActionManifestPath(absoluteManifestPath);
            if (error != EVRInputError.None)
            {
                Logger.Error($"Action manifest error: {error}");
                return false;
            }

            /**********************************************/
            // Here, setup every action with its handler
            /**********************************************/

            // Get action handle for Left Joystick
            error = OpenVR.Input.GetActionHandle(k_actionLeftJoystick, ref m_LeftJoystickHandler);
            if (error != EVRInputError.None)
            {
                Logger.Error("Action handle error: {error}");
                return false;
            }

            // Get action handle for Right Joystick
            error = OpenVR.Input.GetActionHandle(k_actionRightJoystick, ref m_RightJoystickHandler);
            if (error != EVRInputError.None)
            {
                Logger.Error("Action handle error: {error}");
                return false;
            }

            // Get action handle for Confirm And Save
            error = OpenVR.Input.GetActionHandle(k_actionConfirmAndSave, ref m_ConfirmAndSaveHandler);
            if (error != EVRInputError.None)
            {
                Logger.Error("Action handle error: {error}");
                return false;
            }

            // Get action handle for Mode Swap
            error = OpenVR.Input.GetActionHandle(k_actionModeSwap, ref m_ModeSwapHandler);
            if (error != EVRInputError.None)
            {
                Logger.Error("Action handle error: {error}");
                return false;
            }

            // Get action handle for Fine-tuning
            error = OpenVR.Input.GetActionHandle(k_actionFineTune, ref m_FineTuneHandler);
            if (error != EVRInputError.None)
            {
                Logger.Error("Action handle error: {error}");
                return false;
            }

            // Get action handle for Tracker Freeze
            error = OpenVR.Input.GetActionHandle(k_actionTrackerFreeze, ref m_TrackerFreezeHandler);
            if (error != EVRInputError.None)
            {
                Logger.Error("Action handle error: {error}");
                return false;
            }

            // Get action handle for Flip Toggle
            error = OpenVR.Input.GetActionHandle(k_actionFlipToggle, ref m_FlipToggleHandler);
            if (error != EVRInputError.None)
            {
                Logger.Error("Action handle error: {error}");
                return false;
            }

            /**********************************************/
            // Here, setup every action set handle
            /**********************************************/

            // Get set handle Default Set
            error = OpenVR.Input.GetActionSetHandle(k_actionSetDefault, ref m_DefaultSetHandler);
            if (error != EVRInputError.None)
            {
                Logger.Error("ActionSet handle error: {error}");
                return false;
            }

            /**********************************************/
            // Here, setup action-set handler
            /**********************************************/

            // Default Set
            m_defaultActionSet.ulActionSet = m_DefaultSetHandler;
            m_defaultActionSet.ulRestrictedToDevice = OpenVR.k_ulInvalidInputValueHandle;
            m_defaultActionSet.nPriority = 0;

            // Return OK
            Logger.Info("EVR Input Actions initialized OK");
            return true;
        }

        // Update Left Joystick Action
        private bool GetLeftJoystickState()
        {
            // Update the action and grab data
            var error = OpenVR.Input.GetAnalogActionData(
                m_LeftJoystickHandler,
                ref m_LeftJoystickHandlerData,
                (uint)Unsafe.SizeOf<InputAnalogActionData_t>(),
                OpenVR.k_ulInvalidInputValueHandle);

            // Return OK
            if (error == EVRInputError.None) return true;

            Logger.Error($"GetAnalogActionData call error: {error}");
            return false;
        }

        // Update Right Joystick Action
        private bool GetRightJoystickState()
        {
            // Update the action and grab data
            var error = OpenVR.Input.GetAnalogActionData(
                m_RightJoystickHandler,
                ref m_RightJoystickHandlerData,
                (uint)Unsafe.SizeOf<InputAnalogActionData_t>(),
                OpenVR.k_ulInvalidInputValueHandle);

            // Return OK
            if (error == EVRInputError.None) return true;

            Logger.Error($"GetAnalogActionData call error: {error}");
            return false;
        }

        // Update Confirm And Save Action
        private bool GetConfirmAndSaveState()
        {
            // Update the action and grab data
            var error = OpenVR.Input.GetDigitalActionData(
                m_ConfirmAndSaveHandler,
                ref m_ConfirmAndSaveData,
                (uint)Unsafe.SizeOf<InputDigitalActionData_t>(),
                OpenVR.k_ulInvalidInputValueHandle);

            // Return OK
            if (error == EVRInputError.None) return true;

            Logger.Error($"GetDigitalActionData call error: {error}");
            return false;
        }

        // Update Mode Swap Action
        private bool GetModeSwapState()
        {
            // Update the action and grab data
            var error = OpenVR.Input.GetDigitalActionData(
                m_ModeSwapHandler,
                ref m_ModeSwapData,
                (uint)Unsafe.SizeOf<InputDigitalActionData_t>(),
                OpenVR.k_ulInvalidInputValueHandle);

            // Return OK
            if (error == EVRInputError.None) return true;

            Logger.Error($"GetDigitalActionData call error: {error}");
            return false;
        }

        // Update Fine Tune Action
        private bool GetFineTuneState()
        {
            // Update the action and grab data
            var error = OpenVR.Input.GetDigitalActionData(
                m_FineTuneHandler,
                ref m_FineTuneData,
                (uint)Marshal.SizeOf(typeof(InputDigitalActionData_t)),
                OpenVR.k_ulInvalidInputValueHandle);

            // Return OK
            if (error == EVRInputError.None) return true;

            Logger.Error($"GetDigitalActionData call error: {error}");
            return false;
        }

        // Update Tracker Freeze Action
        private bool GetTrackerFreezeState()
        {
            // Update the action and grab data
            var error = OpenVR.Input.GetDigitalActionData(
                m_TrackerFreezeHandler,
                ref m_TrackerFreezeData,
                (uint)Unsafe.SizeOf<InputDigitalActionData_t>(),
                OpenVR.k_ulInvalidInputValueHandle);

            // Return OK
            if (error == EVRInputError.None) return true;

            Logger.Error($"GetDigitalActionData call error: {error}");
            return false;
        }

        // Update Tracker Freeze Action
        private bool GetFlipToggleState()
        {
            // Update the action and grab data
            var error = OpenVR.Input.GetDigitalActionData(
                m_FlipToggleHandler,
                ref m_FlipToggleData,
                (uint)Unsafe.SizeOf<InputDigitalActionData_t>(),
                OpenVR.k_ulInvalidInputValueHandle);

            // Return OK
            if (error == EVRInputError.None) return true;

            Logger.Error($"GetDigitalActionData call error: {error}");
            return false;
        }


        public bool UpdateActionStates()
        {
            /**********************************************/
            // Here, update main action sets' handles
            /**********************************************/

            // Update Default ActionSet states
            var error = OpenVR.Input.UpdateActionState(
                new[] { m_defaultActionSet },
                (uint)Unsafe.SizeOf<VRActiveActionSet_t>());

            if (error != EVRInputError.None)
            {
                Logger.Error($"ActionSet (Default) state update error: {error}");
                return false;
            }

            /**********************************************/
            // Here, update the actions and grab data-s
            /**********************************************/

            // Update the left joystick
            if (!GetLeftJoystickState())
            {
                Logger.Error("Left Joystick Action is not active, can't update!");
                return false;
            }

            // Update the right joystick
            if (!GetRightJoystickState())
            {
                Logger.Error("Right Joystick Action is not active, can't update!");
                return false;
            }

            // Update the confirm and save
            if (!GetConfirmAndSaveState())
            {
                Logger.Error("Confirm And Save Action is not active, can't update!");
                return false;
            }

            // Update the mode swap
            if (!GetModeSwapState())
            {
                Logger.Error("Mode Swap Action is not active, can't update!");
                return false;
            }

            // Update the fine tune
            if (!GetFineTuneState())
            {
                Logger.Error("Fine-tuning Action is not active, can't update!");
                return false;
            }

            // Update the freeze
            // This time without checks, since this one is optional
            GetTrackerFreezeState();

            // Return OK
            return true;
        }

        // Analog data poll
        public InputAnalogActionData_t LeftJoystickActionData()
        {
            return m_LeftJoystickHandlerData;
        }

        public InputAnalogActionData_t RightJoystickActionData()
        {
            return m_RightJoystickHandlerData;
        }

        // Digital data poll
        public InputDigitalActionData_t ConfirmAndSaveActionData()
        {
            return m_ConfirmAndSaveData;
        }

        public InputDigitalActionData_t ModeSwapActionData()
        {
            return m_ModeSwapData;
        }

        public InputDigitalActionData_t FineTuneActionData()
        {
            return m_FineTuneData;
        }

        public InputDigitalActionData_t TrackerFreezeActionData()
        {
            return m_TrackerFreezeData;
        }

        public InputDigitalActionData_t TrackerFlipToggleData()
        {
            return m_FlipToggleData;
        }
    }
}