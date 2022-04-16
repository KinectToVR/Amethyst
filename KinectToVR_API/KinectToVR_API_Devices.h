#pragma once
#include <Windows.h>

#include <string>
#include <vector>
#include <variant>
#include <array>

#include <Eigen/Dense>

/*
 * K2API Devices
 *
 * This is a separate header because we won't need linking
 * & doing much more stuff for nothing, just gonna include
 * this single header +Eigen as you can see up in includes
 *
 */

namespace ktvr
{
	// Interace Version
	static const char* IK2API_Devices_Version = "IK2API_Version_007";

	// Return messaging types
	enum K2InitErrorType
	{
		K2InitError_Invalid,
		// Default
		K2InitError_None,
		// Just the ID
		K2InitError_BadInterface
	};

	// Global Joint Types,
	// see enumeration in external/Kinect
	enum ITrackedJointType
	{
		Joint_Head,
		Joint_Neck,
		Joint_SpineShoulder,
		Joint_ShoulderLeft,
		Joint_ElbowLeft,
		Joint_WristLeft,
		Joint_HandLeft,
		Joint_HandTipLeft,
		Joint_ThumbLeft,
		Joint_ShoulderRight,
		Joint_ElbowRight,
		Joint_WristRight,
		Joint_HandRight,
		Joint_HandTipRight,
		Joint_ThumbRight,
		Joint_SpineMiddle,
		Joint_SpineWaist,
		Joint_HipLeft,
		Joint_KneeLeft,
		Joint_AnkleLeft,
		Joint_FootLeft,
		Joint_HipRight,
		Joint_KneeRight,
		Joint_AnkleRight,
		Joint_FootRight,
		Joint_Total
	};

	// Global joint states
	enum ITrackedJointState
	{
		State_NotTracked,
		State_Inferred,
		State_Tracked
	};

	// Device types for tracking
	enum ITrackingDeviceType
	{
		K2_Unknown,
		K2_Kinect,
		K2_Joints,
		K2_Override
	};

	// Device types for joints [KINECT]
	enum ITrackingDeviceCharacteristics
	{
		// Not set???
		K2_Character_Unknown,
		// NO mathbased, only [ head, waist, ankles ]
		K2_Character_Basic,
		// SUP mathbased, only [ head, elbows, waist, knees, ankles, foot_tips ]
		K2_Character_Simple,
		// SUP mathbased, [ everything ]
		K2_Character_Full
	};

	// Alias for code readability
	typedef int JointTrackingState, K2DeviceType, K2DeviceCharacteristics, MessageType, MessageCode;

	// Tracking Device Joint class for client plugins
	class K2TrackedJoint
	{
		// Named joint, provides pos, rot, state and ofc name
	public:
		K2TrackedJoint()
		{
		}

		K2TrackedJoint(std::string name) : jointName{std::move(name)}
		{
		}

		K2TrackedJoint(Eigen::Vector3f const& pos, Eigen::Quaternionf const& rot,
		               JointTrackingState const& state, std::string const& name) :
			jointOrientation{rot}, jointPosition{pos},
			trackingState{state}, jointName{name}
		{
		}

		std::string getJointName() { return jointName; } // Custom name

		Eigen::Vector3f getJointPosition() { return jointPosition; }
		Eigen::Quaternionf getJointOrientation() { return jointOrientation; }
		JointTrackingState getTrackingState() { return trackingState; }

		// For servers!
		void update(Eigen::Vector3f position,
		            Eigen::Quaternionf orientation,
		            JointTrackingState state)
		{
			jointPosition = position;
			jointOrientation = orientation;
			trackingState = state;
		}

		// For servers!
		void update(JointTrackingState state)
		{
			trackingState = state;
		}

	protected:
		// Tracker should be centered automatically
		Eigen::Quaternionf jointOrientation = Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f);
		Eigen::Vector3f jointPosition = Eigen::Vector3f(0.f, 0.f, 0.f);
		JointTrackingState trackingState = State_NotTracked;

		std::string jointName = "Name not set";
	};

	// Namespace with settings daemon elements / Interfacing
	namespace Interface
	{
		// TextBlock Class : (Label)
		class TextBlock
		{
		public:
			TextBlock() = default;
			virtual ~TextBlock() = default;

			/* XAML-Derived functions & handlers */

			// Visibility Get and Set
			virtual bool Visibility() { return true; }

			virtual void Visibility(bool const& visibility)
			{
			}

			// Width Get and Set
			virtual uint32_t Width() { return 0; }

			virtual void Width(uint32_t const& width)
			{
			}

			// Height Get and Set
			virtual uint32_t Height() { return 0; }

			virtual void Height(uint32_t const& height)
			{
			}

			// Text Get and Set
			virtual std::string Text() { return ""; }

			virtual void Text(std::string const& text)
			{
			}
		};

		// Button Class : (Same as XAMLs)
		class Button
		{
		public:
			Button() = default;
			virtual ~Button() = default;

			/* XAML-Derived functions & handlers */

			// Visibility Get and Set
			virtual bool Visibility() { return true; }

			virtual void Visibility(bool const& visibility)
			{
			}

			// Width Get and Set
			virtual uint32_t Width() { return 0; }

			virtual void Width(uint32_t const& width)
			{
			}

			// Height Get and Set
			virtual uint32_t Height() { return 0; }

			virtual void Height(uint32_t const& height)
			{
			}

			// IsEnabled Get and Set
			virtual bool IsEnabled() { return true; }

			virtual void IsEnabled(bool const& enabled)
			{
			}

			// Label Set (No Get here, sadly)
			virtual void Content(std::string const& content)
			{
			}

			// Function handlers for plugin to use
			std::function<void(Button*)> OnClick;
			//void (*OnClick)(Button* this_button);
		};

		// NumberBox Class : (SpinBox, Same as XAMLs)
		class NumberBox
		{
		public:
			NumberBox() = default;
			virtual ~NumberBox() = default;

			/* XAML-Derived functions & handlers */

			// Visibility Get and Set
			virtual bool Visibility() { return true; }

			virtual void Visibility(bool const& visibility)
			{
			}

			// Width Get and Set
			virtual uint32_t Width() { return 0; }

			virtual void Width(uint32_t const& width)
			{
			}

			// Height Get and Set
			virtual uint32_t Height() { return 0; }

			virtual void Height(uint32_t const& height)
			{
			}

			// IsEnabled Get and Set
			virtual bool IsEnabled() { return true; }

			virtual void IsEnabled(bool const& enabled)
			{
			}

			// Value Get and Set
			virtual int Value() { return 0; }

			virtual void Value(int const& value)
			{
			}

			// Function handlers for plugin to use
			std::function<void(NumberBox*, int const&)> OnValueChanged;
			//void (*OnValueChanged)(NumberBox* this_number_box, int const& new_value);
		};

		// ComboBox Class : (ComboBox / DropDownBox, Same as XAMLs)
		class ComboBox
		{
		public:
			ComboBox() = default;
			virtual ~ComboBox() = default;

			/* XAML-Derived functions & handlers */

			// Visibility Get and Set
			virtual bool Visibility() { return true; }

			virtual void Visibility(bool const& visibility)
			{
			}

			// Width Get and Set
			virtual uint32_t Width() { return 0; }

			virtual void Width(uint32_t const& width)
			{
			}

			// Height Get and Set
			virtual uint32_t Height() { return 0; }

			virtual void Height(uint32_t const& height)
			{
			}

			// IsEnabled Get and Set
			virtual bool IsEnabled() { return true; }

			virtual void IsEnabled(bool const& enabled)
			{
			}

			// Selected Index Get and Set
			virtual uint32_t SelectedIndex() { return 0; }

			virtual void SelectedIndex(uint32_t const& value)
			{
			}

			// Items Vector Get and Set
			virtual std::vector<std::string> Items() { return {}; }

			// WARNING: DON'T CALL THIS DURING ANY OTHER MODIFICATION LIKE SELECTIONCHANGED
			virtual void Items(std::vector<std::string> const& entries)
			{
			}

			// Function handlers for plugin to use
			std::function<void(ComboBox*, uint32_t const&)> OnSelectionChanged;
			//void (*OnSelectionChanged)(ComboBox* this_combo_box, uint32_t const& new_value);
		};

		// CheckBox Class : (Same as XAMLs)
		class CheckBox
		{
		public:
			CheckBox() = default;
			virtual ~CheckBox() = default;

			/* XAML-Derived functions & handlers */

			// Visibility Get and Set
			virtual bool Visibility() { return true; }

			virtual void Visibility(bool const& visibility)
			{
			}

			// Width Get and Set
			virtual uint32_t Width() { return 0; }

			virtual void Width(uint32_t const& width)
			{
			}

			// Height Get and Set
			virtual uint32_t Height() { return 0; }

			virtual void Height(uint32_t const& height)
			{
			}

			// IsEnabled Get and Set
			virtual bool IsEnabled() { return true; }

			virtual void IsEnabled(bool const& enabled)
			{
			}

			// IsChecked Get and Set
			virtual bool IsChecked() { return false; }

			virtual void IsChecked(bool const& is_checked)
			{
			}

			// Function handlers for plugin to use
			std::function<void(CheckBox*)> OnChecked;
			std::function<void(CheckBox*)> OnUnchecked;
			//void (*OnChecked)(CheckBox* this_check_box);
			//void (*OnUnchecked)(CheckBox* this_check_box);
		};

		// ToggleSwitch Class : (A bit altered XAMLs)
		class ToggleSwitch
		{
		public:
			ToggleSwitch() = default;
			virtual ~ToggleSwitch() = default;

			/* XAML-Derived functions & handlers */

			// Visibility Get and Set
			virtual bool Visibility() { return true; }

			virtual void Visibility(bool const& visibility)
			{
			}

			// Width Get and Set
			virtual uint32_t Width() { return 0; }

			virtual void Width(uint32_t const& width)
			{
			}

			// Height Get and Set
			virtual uint32_t Height() { return 0; }

			virtual void Height(uint32_t const& height)
			{
			}

			// IsEnabled Get and Set
			virtual bool IsEnabled() { return true; }

			virtual void IsEnabled(bool const& enabled)
			{
			}

			// IsChecked Get and Set
			virtual bool IsChecked() { return false; }

			virtual void IsChecked(bool const& is_checked)
			{
			}

			// Function handlers for plugin to use
			std::function<void(ToggleSwitch*)> OnChecked;
			std::function<void(ToggleSwitch*)> OnUnchecked;
			//void (*OnChecked)(ToggleSwitch* this_toggle_switch);
			//void (*OnUnchecked)(ToggleSwitch* this_toggle_switch);
		};

		// TextBox Class : (Same as XAMLs, Text Input)
		class TextBox
		{
		public:
			TextBox() = default;
			virtual ~TextBox() = default;

			/* XAML-Derived functions & handlers */

			// Visibility Get and Set
			virtual bool Visibility() { return true; }

			virtual void Visibility(bool const& visibility)
			{
			}

			// Width Get and Set
			virtual uint32_t Width() { return 0; }

			virtual void Width(uint32_t const& width)
			{
			}

			// Height Get and Set
			virtual uint32_t Height() { return 0; }

			virtual void Height(uint32_t const& height)
			{
			}

			// Text Get and Set
			virtual std::string Text() { return ""; }

			virtual void Text(std::string const& text)
			{
			}

			// Function handlers for plugin to use
			std::function<void(TextBox*)> OnEnterKeyDown;
			//void (*OnEnterKeyDown)(TextBox* this_text_box);
		};

		// ProgressRing Class : (Same as XAMLs)
		class ProgressRing
		{
		public:
			ProgressRing() = default;
			virtual ~ProgressRing() = default;

			/* XAML-Derived functions & handlers */

			// Visibility Get and Set
			virtual bool Visibility() { return true; }

			virtual void Visibility(bool const& visibility)
			{
			}

			// Width Get and Set
			virtual uint32_t Width() { return 0; }

			virtual void Width(uint32_t const& width)
			{
			}

			// Height Get and Set
			virtual uint32_t Height() { return 0; }

			virtual void Height(uint32_t const& height)
			{
			}

			// Progress Get and Set (Set <0 to mark as indeterminate)
			virtual int32_t Progress() { return -1; }

			virtual void Progress(int32_t const& progress)
			{
			}
		};

		// ProgressBar Class : (Same as XAMLs)
		class ProgressBar
		{
		public:
			ProgressBar() = default;
			virtual ~ProgressBar() = default;

			/* XAML-Derived functions & handlers */

			// Visibility Get and Set
			virtual bool Visibility() { return true; }

			virtual void Visibility(bool const& visibility)
			{
			}

			// Width Get and Set
			virtual uint32_t Width() { return 0; }

			virtual void Width(uint32_t const& width)
			{
			}

			// Height Get and Set
			virtual uint32_t Height() { return 0; }

			virtual void Height(uint32_t const& height)
			{
			}

			// Progress Get and Set (Set <0 to mark as indeterminate)
			virtual int32_t Progress() { return -1; }

			virtual void Progress(int32_t const& progress)
			{
			}

			// Paused Get and Set
			virtual bool ShowPaused() { return false; }

			virtual void ShowPaused(bool const& show_paused)
			{
			}

			// Error Get and Set
			virtual bool ShowError() { return false; }

			virtual void ShowError(bool const& show_error)
			{
			}
		};

		// LayoutRoot appending enum:
		// when appending a single element,
		// you can choose which side it should
		// snap to (HorizontalAlignment)
		enum class SingleLayoutHorizontalAlignment
		{
			// Snap to the left
			Left,
			// Snap to the right
			Right,
			// Try to be centered
			Center
		};

		// A typedef to save time and space:
		// an std variant around all currently possible ui elements
		// Note: To achieve an empty element (spacer),
		//       you should use a TextBlock with empty text
		typedef std::variant<
			TextBlock*,
			Button*,
			NumberBox*,
			ComboBox*,
			CheckBox*,
			ToggleSwitch*,
			TextBox*,
			ProgressRing*,
			ProgressBar*> Element;

		// PluginSettings layout's root pane
		// This is a xaml vertical stack panel,
		// with additional methods involving grids,
		// allowing us to push different config layouts
		class LayoutRoot
		{
		public:
			// WARNING: It's useless to call this plugin-wise
			LayoutRoot() = default;
			virtual ~LayoutRoot() = default;

			// Append a One-Row single element
			virtual void AppendSingleElement(
				Element const& element,
				SingleLayoutHorizontalAlignment const& alignment =
					SingleLayoutHorizontalAlignment::Left)
			{
			}

			// Append a One-Row element pair : */* column space
			virtual void AppendElementPair(
				Element const& first_element,
				Element const& second_element)
			{
			}

			// Append a One-Row element vector : */* column space
			virtual void AppendElementVector(
				std::vector<Element> const& element_vector)
			{
			}
		};
	}

	// Tracking Device class for client plugins to base on [KINECT]
	class K2TrackingDeviceBase_KinectBasis
	{
	public:
		virtual ~K2TrackingDeviceBase_KinectBasis()
		{
		}

		// These 4 functions are critical
		// All 4 are called by K2App

		// This is called after the app loads the plugin
		virtual void onLoad()
		{
		}

		// This initializes/connects the device
		virtual void initialize()
		{
		}

		// This is called when the device is closed
		virtual void shutdown()
		{
		}

		// This is called to update the device (each loop)
		virtual void update()
		{
		}

		// Should be set up at construction
		// Kinect type must provide joints: [ head, waist, knees, ankles, foot_tips ] or [ head, waist, ankles ]
		// Other type must provide joints: [ waist, ankles ] and will persuade manual calibration

		// Basic character will provide the same as JointsBasis but with head to support autocalibration
		// Simple character will provide the same as Basic but with ankles and knees to support mathbased
		// Full character will provide every kinect joint
		K2DeviceCharacteristics getDeviceCharacteristics() { return deviceCharacteristics; }

		K2DeviceType getDeviceType() { return deviceType; }
		std::string getDeviceName() { return deviceName; } // Custom name

		std::array<Eigen::Vector3f, 25> getJointPositions() { return jointPositions; }
		std::array<Eigen::Quaternionf, 25> getJointOrientations() { return jointOrientations; }
		std::array<JointTrackingState, 25> getTrackingStates() { return trackingStates; }

		// After init, this should always return true
		[[nodiscard]] bool isInitialized() const { return initialized; }

		// These will indicate the device's status.
		// Both should be updated either on call or as frequent as possible
		virtual HRESULT getStatusResult() { return E_NOTIMPL; }
		virtual std::string statusResultString(HRESULT stat) { return "statusResultString behaviour not defined"; }

		// This should be updated on every frame,
		// along with joint devices
		// -> will lead to global tracking loss notification
		//    if set to false at runtime somewhen
		[[nodiscard]] bool isSkeletonTracked() const { return skeletonTracked; }

		// Should be set up at construction
		// Mark this as false ALSO if your device supports 360 tracking by itself
		[[nodiscard]] bool isFlipSupported() const { return flipSupported; } // Flip block

		// Should be set up at construction
		// This will allow Amethyst to calculate rotations by itself, additionally
		[[nodiscard]] bool isAppOrientationSupported() const { return appOrientationSupported; } // Math-based

		/* Helper functions which may be internally called by the device plugin */
		Eigen::Vector3f (*getHMDPosition)(); // Get the HMD Position
		Eigen::Quaternionf (*getHMDOrientation)(); // Get the HMD Rotation
		float (*getHMDOrientationYaw)(); // Get the HMD Yaw, exclusively

		/*
		 * Helper to get all joints' positions from the app,
		 * which are sent to the openvr server driver.
		 * Note: if joint's unused, its trackingState will be 0
		 * Note: Waist,LFoot,RFoot,LElbow,RElbow,LKnee,RKnee
		 */
		std::array<K2TrackedJoint, 7> (*getAppJointPoses)();

		// To support settings daemon and register the layout root,
		// the device must properly report it first
		// -> will lead to showing an additional 'settings' button
		// Note: each device has to save its settings independently
		//       and may use the K2AppData from the Paths' header
		[[nodiscard]] bool isSettingsDaemonSupported() const { return settingsSupported; }

		/*
		 * A pointer to the default layout root registered for the device.
		 * Note: Each device / Plugin gets its own layout root
		 * Note: To show 'settings', the device must report that it supports them
		 */
		Interface::LayoutRoot* layoutRoot;

		// Create a text block
		Interface::TextBlock* (*CreateTextBlock)(std::string const& text);

		// Create a labeled button
		Interface::Button* (*CreateButton)(std::string const& content);

		// Create a number box
		Interface::NumberBox* (*CreateNumberBox)(int const& value);

		// Create a combo box
		Interface::ComboBox* (*CreateComboBox)(std::vector<std::string> const& entries);

		// Create a check box
		Interface::CheckBox* (*CreateCheckBox)();

		// Create a toggle switch
		Interface::ToggleSwitch* (*CreateToggleSwitch)();

		// Create a text box
		Interface::TextBox* (*CreateTextBox)();

		// Create a progress ring
		Interface::ProgressRing* (*CreateProgressRing)();

		// Create a progress bar
		Interface::ProgressBar* (*CreateProgressBar)();

	protected:
		K2DeviceCharacteristics deviceCharacteristics = K2_Character_Unknown;

		K2DeviceType deviceType = K2_Unknown;
		std::string deviceName = "Name not set";

		bool initialized = false;
		bool skeletonTracked = false;

		bool settingsSupported = false;

		bool flipSupported = true;
		bool appOrientationSupported = true;

		std::array<Eigen::Vector3f, 25> jointPositions = {
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f),
			Eigen::Vector3f(0.f, 0.f, 0.f)
		};

		std::array<Eigen::Quaternionf, 25> jointOrientations = {
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f),
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f)
		};

		std::array<JointTrackingState, 25> trackingStates = {}; // State_NotTracked

		class FailedKinectInitialization : public std::exception
		{
			[[nodiscard]] virtual const char* what() const throw()
			{
				return "Failure to initialize the Tracking Device. Is it set up properly?";
			}
		} FailedKinectInitialization;
	};

	// Tracking Device class for client plugins to base on [PSMS]
	class K2TrackingDeviceBase_JointsBasis
	{
	public:
		virtual ~K2TrackingDeviceBase_JointsBasis()
		{
		}

		// These 4 functions are critical
		// All 4 are called by K2App

		// This is called after the app loads the plugin
		virtual void onLoad()
		{
		}

		// This initializes/connects the device
		virtual void initialize()
		{
		}

		// This is called when the device is closed
		virtual void shutdown()
		{
		}

		// This is called to update the device (each loop)
		virtual void update()
		{
		}

		// Should be set up at construction
		// Kinect type must provide joints: [ head, waist, knees, ankles, foot_tips ]
		// Other type must provide joints: [ waist, ankles ] and will persuade manual calibration
		K2DeviceType getDeviceType() { return deviceType; }
		std::string getDeviceName() { return deviceName; } // Custom name

		// Joints' vector. You need to update appended joints in every update() call
		std::vector<K2TrackedJoint> getTrackedJoints() { return trackedJoints; }

		// After init, this should always return true
		[[nodiscard]] bool isInitialized() const { return initialized; }

		// These will indicate the device's status.
		// Both should be updated either on call or as frequent as possible
		virtual HRESULT getStatusResult() { return E_NOTIMPL; }
		virtual std::string statusResultString(HRESULT stat) { return "statusResultString behaviour not defined"; }

		// Signal the joint eg psm_id0 that it's being selected
		virtual void signalJoint(uint32_t at)
		{
		} // Just empty, do not throw cause not everyone will override it

		// This should be updated on every frame,
		// along with joint devices
		// -> will lead to global tracking loss notification
		//    if set to false at runtime somewhen
		[[nodiscard]] bool isSkeletonTracked() const { return skeletonTracked; }

		/* Helper functions which may be internally called by the device plugin */
		Eigen::Vector3f (*getHMDPosition)(); // Get the HMD Position
		Eigen::Quaternionf (*getHMDOrientation)(); // Get the HMD Rotation
		float (*getHMDOrientationYaw)(); // Get the HMD Yaw, exclusively

		/*
		 * Helper to get all joints' positions from the app,
		 * which are sent to the openvr server driver.
		 * Note: if joint's unused, its trackingState will be 0
		 * Note: Waist,LFoot,RFoot,LElbow,RElbow,LKnee,RKnee
		 */
		std::array<K2TrackedJoint, 7> (*getAppJointPoses)();

		// To support settings daemon and register the layout root,
		// the device must properly report it first
		// -> will lead to showing an additional 'settings' button
		// Note: each device has to save its settings independently
		//       and may use the K2AppData from the Paths' header
		[[nodiscard]] bool isSettingsDaemonSupported() const { return settingsSupported; }

		/*
		 * A pointer to the default layout root registered for the device.
		 * Note: Each device / Plugin gets its own layout root
		 * Note: To show 'settings', the device must report that it supports them
		 */
		Interface::LayoutRoot* layoutRoot;

		// Create a text block
		Interface::TextBlock* (*CreateTextBlock)(std::string const& text);

		// Create a labeled button
		Interface::Button* (*CreateButton)(std::string const& content);

		// Create a number box
		Interface::NumberBox* (*CreateNumberBox)(int const& value);

		// Create a combo box
		Interface::ComboBox* (*CreateComboBox)(std::vector<std::string> const& entries);

		// Create a check box
		Interface::CheckBox* (*CreateCheckBox)();

		// Create a toggle switch
		Interface::ToggleSwitch* (*CreateToggleSwitch)();

		// Create a text box
		Interface::TextBox* (*CreateTextBox)();

		// Create a progress ring
		Interface::ProgressRing* (*CreateProgressRing)();

		// Create a progress bar
		Interface::ProgressBar* (*CreateProgressBar)();

	protected:
		K2DeviceType deviceType = K2_Unknown;
		std::string deviceName = "Name not set";

		bool initialized = false;
		bool skeletonTracked = false;

		bool settingsSupported = false;

		std::vector<K2TrackedJoint> trackedJoints = {K2TrackedJoint()};

		class FailedJointsInitialization : public std::exception
		{
			[[nodiscard]] virtual const char* what() const throw()
			{
				return "Failure to initialize the Tracking Device. Is it set up properly?";
			}
		} FailedJointsInitialization;
	};

	// Tracking Device class for client plugins to base on [Pull-Only]
	class K2TrackingDeviceBase_Spectator
	{
	public:
		virtual ~K2TrackingDeviceBase_Spectator()
		{
		}

		// This is called after the app loads the plugin
		virtual void onLoad()
		{
		}

		/* Helper functions which may be internally called by the device plugin */
		Eigen::Vector3f (*getHMDPosition)(); // Get the HMD Position
		Eigen::Quaternionf (*getHMDOrientation)(); // Get the HMD Rotation
		float (*getHMDOrientationYaw)(); // Get the HMD Yaw, exclusively

		/*
		 * Helper to get all joints' positions from the app,
		 * which are sent to the openvr server driver.
		 * Note: if joint's unused, its trackingState will be 0
		 * Note: Waist,LFoot,RFoot,LElbow,RElbow,LKnee,RKnee
		 */
		std::array<K2TrackedJoint, 7> (*getAppJointPoses)();
	};
}
