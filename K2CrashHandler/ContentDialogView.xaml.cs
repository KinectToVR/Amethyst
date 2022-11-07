using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Documents;

namespace K2CrashHandler;

public sealed partial class ContentDialogView
{
    private readonly string _logFileLocation = "0";

    private readonly string _primaryButtonText = "[NOT SET]";
    private readonly string _secondaryButtonText = "[NOT SET]";
    private readonly SemaphoreSlim _semaphoreObject = new(0);

    private bool _confirmationFlyoutResult,
        _confirmationFlyoutRunning;

    public ContentDialogView()
    {
        InitializeComponent();
    }

    public ContentDialogView
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
        DialogContent.Content = content;

        // Back up
        this._primaryButtonText = primaryButtonText;
        this._secondaryButtonText = secondaryButtonText;
        this._logFileLocation = logFileLocation;

        DialogPrimaryButton.Content = this._primaryButtonText;
        DialogSecondaryButton.Content = this._secondaryButtonText;

        DialogPrimaryButton.Click += primaryButtonHandler;
        DialogSecondaryButton.Click += secondaryButtonHandler;

        if (accentPrimaryButton) DialogPrimaryButton.Style = (Style)Resources["AccentButtonStyle"];

        LogFilesRun.Text = File.Exists(this._logFileLocation)
            ? "If you're looking the log file, it's"
            : "If you're looking log files, they're";
    }

    private void LogsHyperlink_OnClick(Hyperlink sender, HyperlinkClickEventArgs args)
    {
        OpenFolderAndSelectItem(File.Exists(_logFileLocation)
            ? _logFileLocation
            : Path.Combine(Environment.GetFolderPath(
                Environment.SpecialFolder.ApplicationData), "Amethyst\\logs\\"));
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

    // An alternative to:
    //      Process.Start("explorer.exe", $"/select,{filePath}");
    // P/Invoke version allows us to do the same task without spawning a new instance of explorer.exe
    // [Ripped straight from https://github.com/KinectToVR/Amethyst-Installer/blob/main/Amethyst-Installer]
    public static void OpenFolderAndSelectItem(string filePath)
    {
        filePath = Path.GetFullPath(filePath); // Resolve absolute path
        var folderPath = Path.GetDirectoryName(filePath);
        var file = Path.GetFileName(filePath);

        IntPtr nativeFolder;
        SHParseDisplayName(folderPath, IntPtr.Zero, out nativeFolder, 0, out _);

        if (nativeFolder == IntPtr.Zero)
            return;

        IntPtr nativeFile;
        SHParseDisplayName(Path.Combine(folderPath, file), IntPtr.Zero, out nativeFile, 0, out _);

        // Open the folder without the file selected if we can't find the file
        IntPtr[] fileArray = { nativeFile != IntPtr.Zero ? nativeFile : nativeFolder };

        SHOpenFolderAndSelectItems(nativeFolder, (uint)fileArray.Length, fileArray, 0);

        Marshal.FreeCoTaskMem(nativeFolder);
        if (nativeFile != IntPtr.Zero) Marshal.FreeCoTaskMem(nativeFile);
    }

    [DllImport("shell32.dll", SetLastError = true)]
    private static extern int SHOpenFolderAndSelectItems(IntPtr pidlFolder, uint cidl,
        [In] [MarshalAs(UnmanagedType.LPArray)]
        IntPtr[] apidl, uint dwFlags);

    [DllImport("shell32.dll", SetLastError = true)]
    private static extern uint SHParseDisplayName([MarshalAs(UnmanagedType.LPWStr)] string name, IntPtr bindingContext,
        [Out] out IntPtr pidl, uint sfgaoIn, [Out] out uint psfgaoOut);

    [DllImport("shell32.dll", CharSet = CharSet.Unicode)]
    private static extern uint SHGetNameFromIDList(IntPtr pidl, Sigdn sigdnName, [Out] out IntPtr ppszName);

    private enum Sigdn : uint
    {
        Normaldisplay = 0x00000000,
        Parentrelativeparsing = 0x80018001,
        Desktopabsoluteparsing = 0x80028000,
        Parentrelativeediting = 0x80031001,
        Desktopabsoluteediting = 0x8004c000,
        Filesyspath = 0x80058000,
        Url = 0x80068000,
        Parentrelativeforaddressbar = 0x8007c001,
        Parentrelative = 0x80080001
    }
}