// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using Windows.Foundation;
using Windows.UI.ViewManagement;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.VisualBasic;
using System;
using Amethyst.Classes;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst;

/// <summary>
///     Provides application-specific behavior to supplement the default Application class.
/// </summary>
public partial class App : Application
{
    private Window _mWindow;

    /// <summary>
    ///     Initializes the singleton application object.  This is the first line of authored code
    ///     executed, and as such is the logical equivalent of main() or WinMain().
    /// </summary>
    public App()
    {
        InitializeComponent();

        ApplicationView.PreferredLaunchViewSize =
            new Size(1000, 700); // Set default window launch size (WinUI)
        ApplicationView.PreferredLaunchWindowingMode =
            ApplicationViewWindowingMode.PreferredLaunchViewSize;

        // Initialize logger
        Logger.Init(Interfacing.GetK2AppDataLogFileDir("Amethyst",
            $"Amethyst_{DateTime.Now.ToString("yyyyMMdd-HHmmss.ffffff")}.log"));
    }

    /// <summary>
    ///     Invoked when the application is launched normally by the end user.  Other entry points
    ///     will be used such as when the application is launched to open a specific file.
    /// </summary>
    /// <param name="args">Details about the launch request and process.</param>
    protected override void OnLaunched(LaunchActivatedEventArgs args)
    {
        _mWindow = new MainWindow();
        _mWindow.Activate();
    }
}