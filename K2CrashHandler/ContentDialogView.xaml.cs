using System;
using System.Diagnostics;
using System.IO;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Documents;

namespace K2CrashHandler
{
    public sealed partial class ContentDialogView
    {
        public ContentDialogView()
        {
            InitializeComponent();
        }

        public ContentDialogView
        (
            String title = "[NO TITLE]",
            String content = "[CONTENT NOT SET]",
            String primaryButtonText = "[NOT SET]",
            String secondaryButtonText = "[NOT SET]",
            RoutedEventHandler primaryButtonHandler = null,
            RoutedEventHandler secondaryButtonHandler = null,
            bool accentPrimaryButton = true
        )
        {
            InitializeComponent();

            DialogTitle.Content = title;
            DialogContent.Content = content;

            DialogPrimaryButton.Content = primaryButtonText;
            DialogSecondaryButton.Content = secondaryButtonText;

            DialogPrimaryButton.Click += primaryButtonHandler;
            DialogSecondaryButton.Click += secondaryButtonHandler;

            if (accentPrimaryButton) DialogPrimaryButton.Style = (Style)Resources["AccentButtonStyle"];

            // Disabled until we can somehow find ame inside registry or just decide to blind shoot with c:/k2ex TODO
            if (!accentPrimaryButton) DialogPrimaryButton.IsEnabled = false;
        }

        private void LogsHyperlink_OnClick(Hyperlink sender, HyperlinkClickEventArgs args)
        {
            var appData = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "Amethyst\\logs");

            Process.Start("explorer.exe", appData);
        }

        private void DiscordHyperlink_OnClick(Hyperlink sender, HyperlinkClickEventArgs args)
        {
            Process.Start("explorer.exe", "https://discord.gg/YBQCRDG");
        }
    }
}