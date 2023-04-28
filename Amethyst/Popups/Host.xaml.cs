using System;
using System.IO;
using Windows.Graphics;
using Amethyst.Classes;
using Amethyst.Utils;
using Microsoft.UI;
using Microsoft.UI.Composition;
using Microsoft.UI.Composition.SystemBackdrops;
using Microsoft.UI.Windowing;
using Microsoft.UI.Xaml;
using WinRT;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Popups;

/// <summary>
///     An empty window that can be used on its own or navigated to within a Frame.
/// </summary>
public sealed partial class Host : Window
{
    private DesktopAcrylicController _acrylicController;
    private SystemBackdropConfiguration _configurationSource;
    private MicaController _micaController;

    private WindowsSystemDispatcherQueueHelper _wsdqHelper; // See separate sample below for implementation

    public Host()
    {
        Logger.Info($"Constructing a new {GetType()}...");
        Logger.Info("Initializing shared XAML components...");
        InitializeComponent();

        Logger.Info("Applying available window backdrops...");
        TrySetMicaBackdrop();

        // Set up
        Logger.Info("Setting up the window decoration data...");
        Title = "Amethyst";

        // Set titlebar/taskview icon
        Logger.Info("Setting the App Window icon...");
        AppWindow.SetIcon(Path.Combine(
            Interfacing.ProgramLocation.DirectoryName, "Assets", "ktvr.ico"));

        Logger.Info("Extending the window titlebar...");
        if (AppWindowTitleBar.IsCustomizationSupported())
        {
            // Chad Windows 11
            AppWindow.TitleBar.ExtendsContentIntoTitleBar = true;
            AppWindow.TitleBar.SetDragRectangles(new RectInt32[]
            {
                new(0, 0, 10000000, 30)
            });

            AppWindow.TitleBar.ButtonBackgroundColor = Colors.Transparent;
            AppWindow.TitleBar.ButtonInactiveBackgroundColor = Colors.Transparent;
            AppWindow.TitleBar.ButtonHoverBackgroundColor =
                AppWindow.TitleBar.ButtonPressedBackgroundColor;
        }
        else
            // Poor ass Windows 10 <1809
        {
            Logger.Warn("Time to get some updates for your archaic Windows install, man!");
        }

        // Resize the window
        Logger.Info("Resizing the application window...");
        AppWindow.Resize(new SizeInt32(500, 500));
        AppWindow.Show(true); // Activate
    }

    public bool Result { get; set; } = false;

    private void TrySetMicaBackdrop()
    {
        Logger.Info("Searching for supported backdrop systems...");
        if (!MicaController.IsSupported() && !DesktopAcrylicController.IsSupported())
        {
            Logger.Info("Mica and acrylic are not supported! Time to update Windows, man!");
            return; // Mica/acrylic is not supported on this system
        }

        Logger.Info("Creating a new system dispatcher helper...");
        _wsdqHelper = new WindowsSystemDispatcherQueueHelper();

        Logger.Info("Setting up the system dispatcher helper...");
        _wsdqHelper.EnsureWindowsSystemDispatcherQueueController();

        // Hooking up the policy object
        Logger.Info("Hooking up system backdrop policies...");
        _configurationSource = new SystemBackdropConfiguration();

        Logger.Info("Setting up activation and theme handlers...");
        Activated += Window_Activated;

        // Initial configuration state.
        Logger.Info("Initializing the configuration source...");
        _configurationSource.IsInputActive = true;
        SetConfigurationSourceTheme();

        if (MicaController.IsSupported())
        {
            Logger.Info("Creating a new shared mica backdrop controller...");
            _micaController = new MicaController();

            // Enable the system backdrop.
            // Note: Be sure to have "using WinRT;" to support the Window.As<...>() call.
            Logger.Info("Registering the generated backdrop within the controller...");
            _micaController.AddSystemBackdropTarget(this.As<ICompositionSupportsSystemBackdrop>());

            Logger.Info("Configuring the backdrop within the controller...");
            _micaController.SetSystemBackdropConfiguration(_configurationSource);
        }
        else if (DesktopAcrylicController.IsSupported())
        {
            Logger.Info("Creating a new shared acrylic backdrop controller...");
            _acrylicController = new DesktopAcrylicController();

            // Enable the system backdrop.
            // Note: Be sure to have "using WinRT;" to support the Window.As<...>() call.
            Logger.Info("Registering the generated backdrop within the controller...");
            _acrylicController.AddSystemBackdropTarget(this.As<ICompositionSupportsSystemBackdrop>());

            Logger.Info("Configuring the backdrop within the controller...");
            _acrylicController.SetSystemBackdropConfiguration(_configurationSource);
        }
    }

    private void Window_Activated(object sender, WindowActivatedEventArgs args)
    {
        _configurationSource.IsInputActive = args.WindowActivationState != WindowActivationState.Deactivated;
    }
    
    private void SetConfigurationSourceTheme()
    {
        _configurationSource.Theme = Application.Current.RequestedTheme switch
        {
            ApplicationTheme.Dark => SystemBackdropTheme.Dark,
            ApplicationTheme.Light => SystemBackdropTheme.Light,
            _ => _configurationSource.Theme
        };
    }
}