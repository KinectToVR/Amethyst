using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Threading;
using System.Threading.Tasks;
using Amethyst.Utils;
using Microsoft.UI.Xaml.Controls;
using static System.Net.Mime.MediaTypeNames;

namespace Amethyst.Classes;

public static class Shared
{
    public static class Semaphores
    {
        public static Semaphore
            ReloadMainWindowSemaphore = new(0, 1),
            ReloadGeneralPageSemaphore = new(0, 1),
            ReloadSettingsPageSemaphore = new(0, 1),
            ReloadDevicesPageSemaphore = new(0, 1),
            ReloadInfoPageSemaphore = new(0, 1);
    }

    public static class Main
    {
        // Vector of std.pair holding the Navigation Tag and the relative Navigation Page.
        public static List<(string Tag, Type Page)> Pages;

        // Main Window
        public static NavigationViewItem
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

        public static TextBlock
            AppTitleLabel,
            FlyoutHeader,
            FlyoutFooter,
            FlyoutContent;

        public static Grid
            InterfaceBlockerGrid, NavigationBlockerGrid;

        public static NavigationView MainNavigationView;
        public static Frame MainContentFrame;

        public static FontIcon UpdateIconDot;
        public static Flyout UpdateFlyout;

        public static Button
            InstallNowButton, InstallLaterButton;

        public static Microsoft.UI.Xaml.Media.SolidColorBrush
            AttentionBrush, NeutralBrush;

        public static class NavigationItems
        {
            public static FontIcon
                NavViewGeneralButtonIcon,
                NavViewSettingsButtonIcon,
                NavViewDevicesButtonIcon,
                NavViewInfoButtonIcon,
                NavViewOkashiButtonIcon;

            public static TextBlock
                NavViewGeneralButtonLabel,
                NavViewSettingsButtonLabel,
                NavViewDevicesButtonLabel,
                NavViewInfoButtonLabel,
                NavViewOkashiButtonLabel;
        }

        // Navigate the main view (w/ animations)
        public static void NavigateToPage(string navItemTag,
            Microsoft.UI.Xaml.Media.Animation.NavigationTransitionInfo transitionInfo)
        {
            Logger.Info($"Navigation requested! Page tag: {navItemTag}");

            var page = Pages.FirstOrDefault(p => p.Tag.Equals(navItemTag)).Page;
            var preNavPageType = MainContentFrame.CurrentSourcePageType;

            // Navigate only if the selected page isn't currently loaded
            if (page is null || preNavPageType == page) return;
            AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

            // Switch bring back the current item to the base state
            if (!string.IsNullOrEmpty(preNavPageType?.Name))
            {
                switch (preNavPageType.Name)
                {
                    case "Amethyst.Pages.General":
                        NavigationItems.NavViewGeneralButtonIcon.Translation = new Vector3(0,-8,0);
                        NavigationItems.NavViewGeneralButtonLabel.Opacity = 1.0;

                        NavigationItems.NavViewGeneralButtonIcon.Foreground = NeutralBrush;
                        NavigationItems.NavViewGeneralButtonIcon.Glyph = "\uE80F";
                        break;
                    case "Amethyst.Pages.Settings":
                        NavigationItems.NavViewSettingsButtonIcon.Translation = new Vector3(0,-8,0);
                        NavigationItems.NavViewSettingsButtonLabel.Opacity = 1.0;

                        NavigationItems.NavViewSettingsButtonIcon.Foreground = NeutralBrush;
                        NavigationItems.NavViewSettingsButtonIcon.Glyph = "\uE713";
                        break;
                    case "Amethyst.Pages.Devices":
                        NavigationItems.NavViewDevicesButtonIcon.Translation = new Vector3(0,-8,0);
                        NavigationItems.NavViewDevicesButtonLabel.Opacity = 1.0;

                        NavigationItems.NavViewDevicesButtonIcon.Foreground = NeutralBrush;

                        NavigationItems.NavViewDevicesButtonIcon.Glyph = "\uF158";
                        NavigationItems.NavViewDevicesButtonIcon.FontSize = 20;
                        break;
                    case "Amethyst.Pages.Info":
                        NavigationItems.NavViewInfoButtonIcon.Translation = new Vector3(0,-8,0);
                        NavigationItems.NavViewInfoButtonLabel.Opacity = 1.0;

                        NavigationItems.NavViewInfoButtonIcon.Foreground = NeutralBrush;
                        NavigationItems.NavViewInfoButtonIcon.Glyph = "\uE946";
                        break;
                }
            }

            // Switch the next navview item to the active state
            if (!string.IsNullOrEmpty(page.Name))
            {
                switch (page.Name)
                {
                    case "Amethyst.GeneralPage":
                        NavigationItems.NavViewGeneralButtonIcon.Glyph = "\uEA8A";
                        NavigationItems.NavViewGeneralButtonIcon.Foreground = AttentionBrush;

                        NavigationItems.NavViewGeneralButtonLabel.Opacity = 0.0;
                        NavigationItems.NavViewGeneralButtonIcon.Translation = Vector3.Zero;
                        break;
                    case "Amethyst.SettingsPage":
                        NavigationItems.NavViewSettingsButtonIcon.Glyph = "\uF8B0";
                        NavigationItems.NavViewSettingsButtonIcon.Foreground = AttentionBrush;

                        NavigationItems.NavViewSettingsButtonLabel.Opacity = 0.0;
                        NavigationItems.NavViewSettingsButtonIcon.Translation = Vector3.Zero;
                        break;
                    case "Amethyst.DevicesPage":
                        NavigationItems.NavViewDevicesButtonLabel.Opacity = 0.0;
                        NavigationItems.NavViewDevicesButtonIcon.Translation = Vector3.Zero;

                        NavigationItems.NavViewDevicesButtonIcon.Foreground = AttentionBrush;

                        NavigationItems.NavViewDevicesButtonIcon.Glyph = "\uEBD2";
                        NavigationItems.NavViewDevicesButtonIcon.FontSize = 23;
                        break;
                    case "Amethyst.InfoPage":
                        NavigationItems.NavViewInfoButtonIcon.Glyph = "\uF167";
                        NavigationItems.NavViewInfoButtonIcon.Foreground = AttentionBrush;

                        NavigationItems.NavViewInfoButtonLabel.Opacity = 0.0;
                        NavigationItems.NavViewInfoButtonIcon.Translation = Vector3.Zero;
                        break;
                }
            }

            Interfacing.CurrentPageTag = navItemTag; // Cache the current page tag
            Interfacing.CurrentPageClass = page.Name; // Cache the current page tag

            MainContentFrame.Navigate(page, null, transitionInfo);
        }
    }

    public static class General
    {
        // General Page
        public static bool GeneralTabSetupFinished = false;
        public static Microsoft.UI.Xaml.Controls.Primitives.ToggleButton ToggleTrackersButton;
        public static ToggleSplitButton SkeletonToggleButton;
        public static CheckBox ForceRenderCheckBox;
        public static MenuFlyoutItem OffsetsButton;

        public static Button
            CalibrationButton, ReRegisterButton, ServerOpenDiscordButton;

        public static TextBlock
            VersionLabel,
            DeviceNameLabel,
            DeviceStatusLabel,
            ErrorWhatText,
            TrackingDeviceErrorLabel,
            ServerStatusLabel,
            ServerErrorLabel,
            ServerErrorWhatText,
            ForceRenderText;

        public static Grid
            OffsetsControlHostGrid,
            ErrorButtonsGrid,
            ErrorWhatGrid,
            ServerErrorWhatGrid,
            ServerErrorButtonsGrid;

        public static Microsoft.UI.Xaml.Controls.Primitives.ToggleButton
            ToggleFreezeButton;

        public static ToggleMenuFlyoutItem
            FreezeOnlyLowerToggle;

        public static TextBlock
            AdditionalDeviceErrorsHyperlink;

        public static Task AdditionalDeviceErrorsHyperlinkTappedEvent =
            new(() =>
            {
                // Placeholder event // async
            });
    }

    public static class Settings
    {
        // Settings Page
        public static bool SettingsLocalInitFinished = false;

        public static Button RestartButton;
        public static Grid FlipDropDownGrid;
        public static ScrollViewer PageMainScrollViewer;
        public static ToggleSwitch FlipToggle;
        public static Slider SoundsVolumeSlider;
        public static Expander FlipDropDown;

        public static CheckBox
            ExternalFlipCheckBox,
            AutoSpawnCheckbox,
            EnableSoundsCheckbox,
            AutoStartCheckBox,
            CheckOverlapsCheckBox;

        public static TextBlock
            ExternalFlipCheckBoxLabel,
            SetErrorFlyoutText,
            ExternalFlipStatusLabel;

        public static StackPanel
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

        public static TextBlock
            DeviceNameLabel,
            DeviceStatusLabel,
            ErrorWhatText,
            TrackingDeviceErrorLabel,
            OverridesLabel,
            JointBasisLabel;

        public static Grid
            DeviceErrorGrid,
            TrackingDeviceChangePanel,
            DevicesMainContentGridOuter,
            DevicesMainContentGridInner,
            SelectedDeviceSettingsHostContainer;

        public static TreeView DevicesTreeView;

        public static ItemsRepeater PluginsItemsRepeater;

        public static Flyout
            NoJointsFlyout,
            SetDeviceTypeFlyout;

        public static Button
            SetAsOverrideButton,
            SetAsBaseButton,
            DeselectDeviceButton;

        public static Semaphore
            SmphSignalCurrentUpdate = new(0, 1),
            SmphSignalStartMain = new(0, 1);

        public static string PreviousSelectedTrackingDeviceGuid;
        public static string SelectedTrackingDeviceGuid;

        public static ScrollViewer DevicesMainContentScrollViewer;

        public static StackPanel
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

    public static class TeachingTips
    {
        public static class Main
        {
            public static TeachingTip
                InitializerTeachingTip, ReloadTeachingTip;
        }

        public static class General
        {
            public static TeachingTip
                ToggleTrackersTeachingTip, StatusTeachingTip;
        }

        public static class Settings
        {
            public static TeachingTip
                ManageTrackersTeachingTip, AutoStartTeachingTip;
        }

        public static class Devices
        {
            public static TeachingTip
                DevicesListTeachingTip, DeviceControlsTeachingTip;
        }

        public static class Info
        {
            public static TeachingTip
                HelpTeachingTip;
        }
    }
}