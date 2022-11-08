using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace Amethyst.Classes;

public static class Shared
{
    public static class Main
    {
        // Navigate the main view (w/ animations)
        public static void NavigateToPage(string navItemTag,
            Microsoft.UI.Xaml.Media.Animation.NavigationTransitionInfo transitionInfo)
        {
        }

        // Vector of std.pair holding the Navigation Tag and the relative Navigation Page.
        public static List<(string Tag, Type Page)> Pages;

        // Main Window
        public static Microsoft.UI.Xaml.Controls.NavigationViewItem
            GeneralItem,
            SettingsItem,
            DevicesItem,
            InfoItem,
            ConsoleItem,
            HelpButton;

        public static Mutex ApplicationMultiInstanceMutex;

        public static Microsoft.UI.Xaml.Window Window;
        public static Microsoft.UI.Windowing.AppWindow AppWindow;
        public static IntPtr AppWindowId;
        public static Microsoft.UI.Dispatching.DispatcherQueue DispatcherQueue;

        public static Microsoft.Windows.AppNotifications.AppNotificationManager NotificationManager;
        public static Microsoft.Windows.ApplicationModel.Resources.ResourceManager ResourceManager;
        public static Microsoft.Windows.ApplicationModel.Resources.ResourceContext ResourceContext;

        public static Microsoft.UI.Xaml.Controls.TextBlock
            AppTitleLabel,
            FlyoutHeader,
            FlyoutFooter,
            FlyoutContent;

        public static Microsoft.UI.Xaml.Controls.Grid
            InterfaceBlockerGrid, NavigationBlockerGrid;

        public static Microsoft.UI.Xaml.Controls.NavigationView MainNavigationView;
        public static Microsoft.UI.Xaml.Controls.Frame MainContentFrame;

        public static Microsoft.UI.Xaml.Controls.FontIcon UpdateIconDot;
        public static Microsoft.UI.Xaml.Controls.Flyout UpdateFlyout;

        public static Microsoft.UI.Xaml.Controls.Button
            InstallNowButton, InstallLaterButton;

        public static Microsoft.UI.Xaml.Media.SolidColorBrush
            AttentionBrush, NeutralBrush;

        public static class NavigationItems
        {
            public static Microsoft.UI.Xaml.Controls.FontIcon
                NavViewGeneralButtonIcon,
                NavViewSettingsButtonIcon,
                NavViewDevicesButtonIcon,
                NavViewInfoButtonIcon,
                NavViewOkashiButtonIcon;

            public static Microsoft.UI.Xaml.Controls.TextBlock
                NavViewGeneralButtonLabel,
                NavViewSettingsButtonLabel,
                NavViewDevicesButtonLabel,
                NavViewInfoButtonLabel,
                NavViewOkashiButtonLabel;
        }
    }

    public static class General
    {
        // General Page
        public static bool GeneralTabSetupFinished = false,
            PendingOffsetsUpdate = false;

        public static Microsoft.UI.Xaml.Controls.Primitives.ToggleButton ToggleTrackersButton;
        public static Microsoft.UI.Xaml.Controls.ToggleSplitButton SkeletonToggleButton;
        public static Microsoft.UI.Xaml.Controls.CheckBox ForceRenderCheckBox;
        public static Microsoft.UI.Xaml.Controls.MenuFlyoutItem OffsetsButton;

        public static Microsoft.UI.Xaml.Controls.Button
            CalibrationButton, ReRegisterButton, ServerOpenDiscordButton;

        public static Microsoft.UI.Xaml.Controls.TextBlock
            VersionLabel,
            DeviceNameLabel,
            DeviceStatusLabel,
            ErrorWhatText,
            TrackingDeviceErrorLabel,
            ServerStatusLabel,
            ServerErrorLabel,
            ServerErrorWhatText,
            ForceRenderText;

        public static Microsoft.UI.Xaml.Controls.Grid
            OffsetsControlHostGrid,
            ErrorButtonsGrid,
            ErrorWhatGrid,
            ServerErrorWhatGrid,
            ServerErrorButtonsGrid;

        public static Microsoft.UI.Xaml.Controls.Primitives.ToggleButton
            ToggleFreezeButton;

        public static Microsoft.UI.Xaml.Controls.ToggleMenuFlyoutItem
            FreezeOnlyLowerToggle;

        public static Microsoft.UI.Xaml.Controls.TextBlock
            AdditionalDeviceErrorsHyperlink;

        public static Task AdditionalDeviceErrorsHyperlinkTappedEvent =
            new(async () =>
            {
                // Placeholder event
            });
    }

    public static class Settings
    {
        // Settings Page
        public static bool SettingsLocalInitFinished = false;

        public static Microsoft.UI.Xaml.Controls.Button RestartButton;
        public static Microsoft.UI.Xaml.Controls.Grid FlipDropDownGrid;
        public static Microsoft.UI.Xaml.Controls.ScrollViewer PageMainScrollViewer;
        public static Microsoft.UI.Xaml.Controls.ToggleSwitch FlipToggle;
        public static Microsoft.UI.Xaml.Controls.Slider SoundsVolumeSlider;
        public static Microsoft.UI.Xaml.Controls.Expander FlipDropDown;

        public static Microsoft.UI.Xaml.Controls.CheckBox
            ExternalFlipCheckBox,
            AutoSpawnCheckbox,
            EnableSoundsCheckbox,
            AutoStartCheckBox,
            CheckOverlapsCheckBox;

        public static Microsoft.UI.Xaml.Controls.TextBlock
            ExternalFlipCheckBoxLabel,
            SetErrorFlyoutText,
            ExternalFlipStatusLabel;

        public static Microsoft.UI.Xaml.Controls.StackPanel
            ExternalFlipStackPanel,
            JointExpanderHostStackPanel,
            ExternalFlipStatusStackPanel,
            FlipDropDownContainer;
    }

    public static class Devices
    {
        // Devices Page
        public static bool DevicesTabSetupFinished = false, // On-load setup
            DevicesTabReSetupFinished = false, // Other setup
            DevicesJointsSetupPending = false, // Overrides
            DevicesSignalJoints = true, // Optionally no signal
            DevicesModelSetupFinished = false; // MVVM setup done?

        public static Microsoft.UI.Xaml.Controls.TextBlock
            DeviceNameLabel,
            DeviceStatusLabel,
            ErrorWhatText,
            TrackingDeviceErrorLabel,
            OverridesLabel,
            JointBasisLabel;

        public static Microsoft.UI.Xaml.Controls.Grid
            DeviceErrorGrid,
            TrackingDeviceChangePanel,
            DevicesMainContentGridOuter,
            DevicesMainContentGridInner,
            SelectedDeviceSettingsHostContainer;

        public static Microsoft.UI.Xaml.Controls.TreeView DevicesTreeView;

        public static Microsoft.UI.Xaml.Controls.ItemsRepeater PluginsItemsRepeater;

        public static Microsoft.UI.Xaml.Controls.Flyout
            NoJointsFlyout,
            SetDeviceTypeFlyout;

        public static Microsoft.UI.Xaml.Controls.Button
            SetAsOverrideButton,
            SetAsBaseButton,
            DeselectDeviceButton;

        public static Semaphore 
            SmphSignalCurrentUpdate = new(0, 1),
            SmphSignalStartMain = new(0, 1);

        public static string PreviousSelectedTrackingDeviceGuid;
        public static string SelectedTrackingDeviceGuid;

        public static Microsoft.UI.Xaml.Controls.ScrollViewer DevicesMainContentScrollViewer;

        public static Microsoft.UI.Xaml.Controls.StackPanel
            DevicesOverridesSelectorStackPanelOuter,
            DevicesOverridesSelectorStackPanelInner,
            DevicesJointSelectorStackPanelOuter,
            DevicesJointSelectorStackPanelInner,
            JointExpanderHostStackPanel,
            SelectedDeviceSettingsRootLayoutPanel,
            OverridesExpanderHostStackPanel;
        
        public static Task ReloadSelectedDevice(bool manual, bool reconnect = false)
        {
            throw new NotImplementedException();
        }
    }
}