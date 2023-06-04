using System;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using Windows.System;
using Amethyst.Classes;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Documents;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Popups;

/// <summary>
///     An empty page that can be used on its own or navigated to within a Frame.
/// </summary>
public sealed partial class CrashDialog
{
    private readonly string _logFileLocation = "0";

    private readonly string _primaryButtonText = "[NOT SET]";
    private readonly string _secondaryButtonText = "[NOT SET]";
    private readonly SemaphoreSlim _semaphoreObject = new(0);

    private bool _confirmationFlyoutResult,
        _confirmationFlyoutRunning;

    public CrashDialog()
    {
        InitializeComponent();
    }

    public CrashDialog
    (
        string title = "[NO TITLE]",
        string content = "[CONTENT NOT SET]",
        string primaryButtonText = "[NOT SET]",
        string secondaryButtonText = "[NOT SET]",
        RoutedEventHandler primaryButtonHandler = null,
        RoutedEventHandler secondaryButtonHandler = null,
        bool accentPrimaryButton = true,
        string logFileLocation = "0"
    )
    {
        InitializeComponent();

        DialogTitle.Content = title;
        DialogContent.Text = content;

        // Back up
        _primaryButtonText = primaryButtonText;
        _secondaryButtonText = secondaryButtonText;
        _logFileLocation = logFileLocation;

        DialogPrimaryButton.Content = _primaryButtonText;
        DialogSecondaryButton.Content = _secondaryButtonText;

        DialogPrimaryButton.Click += primaryButtonHandler;
        DialogSecondaryButton.Click += secondaryButtonHandler;

        if (accentPrimaryButton) DialogPrimaryButton.Style = (Style)Resources["AccentButtonStyle"];

        LogFilesRun.Text = File.Exists(_logFileLocation)
            ? "If you're looking the log file, it's"
            : "If you're looking log files, they're";
    }

    private void LogsHyperlink_OnClick(Hyperlink sender, HyperlinkClickEventArgs args)
    {
        SystemShell.OpenFolderAndSelectItem(File.Exists(_logFileLocation)
            ? _logFileLocation
            : Path.Combine(Interfacing.TemporaryFolder.Path, "logs\\"));
    }

    private async void DiscordHyperlink_OnClick(Hyperlink sender, HyperlinkClickEventArgs args)
    {
        await Launcher.LaunchUriAsync(new Uri("https://discord.gg/YBQCRDG"));
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
        _confirmationFlyoutRunning = true;
        ConfirmationFlyout.Placement = FlyoutPlacementMode.TopEdgeAlignedLeft;
        ConfirmationFlyout.ShowAt(DialogPrimaryButton);

        // Wait for the result
        await _semaphoreObject.WaitAsync();

        // Wait a bit
        await Task.Delay(1200);

        // Return the result
        return _confirmationFlyoutResult;
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
        _confirmationFlyoutRunning = true;
        ConfirmationFlyout.Placement = FlyoutPlacementMode.TopEdgeAlignedRight;
        ConfirmationFlyout.ShowAt(DialogSecondaryButton);

        // Wait for the result
        await _semaphoreObject.WaitAsync();

        // Return the result
        return _confirmationFlyoutResult;
    }

    private void ConfirmationFlyoutConfirmActionButton_Click(object sender, RoutedEventArgs e)
    {
        if (!_confirmationFlyoutRunning) return;

        // Set the result and release the semaphore
        _confirmationFlyoutResult = true;
        _semaphoreObject.Release();

        _confirmationFlyoutRunning = false;
        ConfirmationFlyout.Hide();
    }

    private void ConfirmationFlyoutCancelActionButton_Click(object sender, RoutedEventArgs e)
    {
        if (!_confirmationFlyoutRunning) return;

        // Set the result and release the semaphore
        _confirmationFlyoutResult = false;
        _semaphoreObject.Release();

        _confirmationFlyoutRunning = false;
        ConfirmationFlyout.Hide();
    }

    private void ConfirmationFlyout_Closing(FlyoutBase sender, FlyoutBaseClosingEventArgs args)
    {
        if (!_confirmationFlyoutRunning) return;

        // Set the result and release the semaphore
        _confirmationFlyoutResult = false;
        _semaphoreObject.Release();

        _confirmationFlyoutRunning = false;
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

        DialogPrimaryButton.Content = actionPending ? viewbox : _primaryButtonText;
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

        DialogSecondaryButton.Content = actionPending ? viewbox : _secondaryButtonText;
    }
}