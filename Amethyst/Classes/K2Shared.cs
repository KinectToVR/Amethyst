using System;
using System.Collections.Generic;
using System.Threading;

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
}