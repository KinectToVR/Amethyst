using System.Runtime.InteropServices;
using Amethyst.Utils;
using Amethyst.Vendor;

namespace Amethyst.Classes;

public static class K2EVRInput
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
        private const string
            m_actionManifestPath = "action_manifest.json";

        private readonly ulong
            m_ConfirmAndSaveHandler = 0;

        private readonly ulong
            m_FineTuneHandler = 0;

        private readonly ulong
            m_FlipToggleHandler = 0;

        // Calibration actions
        private readonly ulong
            m_LeftJoystickHandler = 0;

        private readonly ulong
            m_ModeSwapHandler = 0;

        private readonly ulong
            m_RightJoystickHandler = 0;

        // Tracking freeze actions
        private readonly ulong
            m_TrackerFreezeHandler = 0;

        // Buttons data
        private InputDigitalActionData_t
            m_ConfirmAndSaveData,
            m_ModeSwapData,
            m_FineTuneData,
            m_TrackerFreezeData,
            m_FlipToggleData;

        // The action sets
        private readonly VRActiveActionSet_t
            m_defaultActionSet = new();

        // Tracking Default set
        private ulong m_DefaultSetHandler = 0;

        // Analogs data
        private InputAnalogActionData_t
            m_LeftJoystickHandlerData,
            m_RightJoystickHandlerData;

        // Update Left Joystick Action
        private bool GetLeftJoystickState()
        {
            // Update the action and grab data
            var error = new IVRInput().GetAnalogActionData(
                m_LeftJoystickHandler,
                ref m_LeftJoystickHandlerData,
                (uint)Marshal.SizeOf(typeof(InputAnalogActionData_t)),
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
            var error = new IVRInput().GetAnalogActionData(
                m_RightJoystickHandler,
                ref m_RightJoystickHandlerData,
                (uint)Marshal.SizeOf(typeof(InputAnalogActionData_t)),
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
            var error = new IVRInput().GetDigitalActionData(
                m_ConfirmAndSaveHandler,
                ref m_ConfirmAndSaveData,
                (uint)Marshal.SizeOf(typeof(InputDigitalActionData_t)),
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
            var error = new IVRInput().GetDigitalActionData(
                m_ModeSwapHandler,
                ref m_ModeSwapData,
                (uint)Marshal.SizeOf(typeof(InputDigitalActionData_t)),
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
            var error = new IVRInput().GetDigitalActionData(
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
            var error = new IVRInput().GetDigitalActionData(
                m_TrackerFreezeHandler,
                ref m_TrackerFreezeData,
                (uint)Marshal.SizeOf(typeof(InputDigitalActionData_t)),
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
            var error = new IVRInput().GetDigitalActionData(
                m_FlipToggleHandler,
                ref m_FlipToggleData,
                (uint)Marshal.SizeOf(typeof(InputDigitalActionData_t)),
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
            var error = new IVRInput().UpdateActionState(
                new[] { m_defaultActionSet },
                (uint)Marshal.SizeOf(typeof(VRActiveActionSet_t)), 1);

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