using System;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Documents;

namespace K2CrashHandler;

public sealed partial class ContentDialogView
{
    private SemaphoreSlim semaphoreObject = new(0);

    private bool confirmationFlyoutResult = false,
        confirmationFlyoutRunning = false;

    private string primaryButtonText = "[NOT SET]";
    private string secondaryButtonText = "[NOT SET]";
    private string logFileLocation = "0";

    public ContentDialogView()
    {
        InitializeComponent();
    }

    public ContentDialogView
    (
        string title = "[NO TITLE]",
        string content = "[CONTENT NOT SET]",
        string _primaryButtonText = "[NOT SET]",
        string _secondaryButtonText = "[NOT SET]",
        RoutedEventHandler primaryButtonHandler = null,
        RoutedEventHandler secondaryButtonHandler = null,
        bool accentPrimaryButton = true,
        string _logFileLocation = "0"
    )
    {
        InitializeComponent();

        DialogTitle.Content = title;
        DialogContent.Content = content;

        // Back up
        primaryButtonText = _primaryButtonText;
        secondaryButtonText = _secondaryButtonText;
        logFileLocation = _logFileLocation;

        DialogPrimaryButton.Content = primaryButtonText;
        DialogSecondaryButton.Content = secondaryButtonText;

        DialogPrimaryButton.Click += primaryButtonHandler;
        DialogSecondaryButton.Click += secondaryButtonHandler;

        if (accentPrimaryButton) DialogPrimaryButton.Style = (Style)Resources["AccentButtonStyle"];

        LogFilesRun.Text = File.Exists(logFileLocation)
            ? "If you're looking the log file, it's"
            : "If you're looking log files, they're";
    }

    private void LogsHyperlink_OnClick(Hyperlink sender, HyperlinkClickEventArgs args)
    {
        var appData = Path.Combine(Environment.GetFolderPath(
            Environment.SpecialFolder.ApplicationData), "Amethyst\\logs");

        Process.Start("explorer.exe", File.Exists(logFileLocation)
            ? $"/select,\"{logFileLocation}\""
            : appData);
    }

    private void DiscordHyperlink_OnClick(Hyperlink sender, HyperlinkClickEventArgs args)
    {
        Process.Start("explorer.exe", "https://discord.gg/YBQCRDG");
    }

    public async Task<bool> HandlePrimaryButtonConfirmationFlyout(string content, string confirmButtonText,
        string cancelButtonText)
    {
        // Set up flyout contents
        ConfirmationFlyoutContent.Text = content;
        ConfirmationFlyoutConfirmActionButton.Content = confirmButtonText;
        ConfirmationFlyoutCancelActionButton.Content = cancelButtonText;

        ConfirmationFlyoutConfirmActionButton.Visibility =
            (Visibility)Convert.ToInt32(string.IsNullOrEmpty(confirmButtonText));
        ConfirmationFlyoutCancelActionButton.Visibility =
            (Visibility)Convert.ToInt32(string.IsNullOrEmpty(cancelButtonText));

        // Show the flyout
        confirmationFlyoutRunning = true;
        ConfirmationFlyout.Placement = FlyoutPlacementMode.TopEdgeAlignedLeft;
        ConfirmationFlyout.ShowAt(DialogPrimaryButton);

        // Wait for the result
        await semaphoreObject.WaitAsync();

        // Wait a bit
        await Task.Delay(1200);

        // Return the result
        return confirmationFlyoutResult;
    }

    public async Task<bool> HandleSecondaryButtonConfirmationFlyout(string content, string confirmButtonText,
        string cancelButtonText)
    {
        // Set up flyout contents
        ConfirmationFlyoutContent.Text = content;
        ConfirmationFlyoutConfirmActionButton.Content = confirmButtonText;
        ConfirmationFlyoutCancelActionButton.Content = cancelButtonText;

        ConfirmationFlyoutConfirmActionButton.Visibility =
            (Visibility)Convert.ToInt32(string.IsNullOrEmpty(confirmButtonText));
        ConfirmationFlyoutCancelActionButton.Visibility =
            (Visibility)Convert.ToInt32(string.IsNullOrEmpty(cancelButtonText));

        // Show the flyout
        confirmationFlyoutRunning = true;
        ConfirmationFlyout.Placement = FlyoutPlacementMode.TopEdgeAlignedRight;
        ConfirmationFlyout.ShowAt(DialogSecondaryButton);

        // Wait for the result
        await semaphoreObject.WaitAsync();

        // Return the result
        return confirmationFlyoutResult;
    }

    private void ConfirmationFlyoutConfirmActionButton_Click(object sender, RoutedEventArgs e)
    {
        if (!confirmationFlyoutRunning) return;

        // Set the result and release the semaphore
        confirmationFlyoutResult = true;
        semaphoreObject.Release();

        confirmationFlyoutRunning = false;
        ConfirmationFlyout.Hide();
    }

    private void ConfirmationFlyoutCancelActionButton_Click(object sender, RoutedEventArgs e)
    {
        if (!confirmationFlyoutRunning) return;

        // Set the result and release the semaphore
        confirmationFlyoutResult = false;
        semaphoreObject.Release();

        confirmationFlyoutRunning = false;
        ConfirmationFlyout.Hide();
    }

    private void ConfirmationFlyout_Closing(FlyoutBase sender, FlyoutBaseClosingEventArgs args)
    {
        if (!confirmationFlyoutRunning) return;

        // Set the result and release the semaphore
        confirmationFlyoutResult = false;
        semaphoreObject.Release();

        confirmationFlyoutRunning = false;
    }

    public void PrimaryButtonActionPending(bool actionPending)
    {
        ProgressRing ring = new()
        {
            IsActive = true,
            IsIndeterminate = true
        };
        Viewbox viewbox = new()
        {
            Child = ring,
            Height = 20
        };

        DialogPrimaryButton.Content = actionPending ? viewbox : primaryButtonText;
    }

    public void SecondaryButtonActionPending(bool actionPending)
    {
        ProgressRing ring = new()
        {
            IsActive = true,
            IsIndeterminate = true
        };
        Viewbox viewbox = new()
        {
            Child = ring,
            Height = 22
        };

        DialogSecondaryButton.Content = actionPending ? viewbox : secondaryButtonText;
    }
}