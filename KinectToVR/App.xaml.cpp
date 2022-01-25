#include "pch.h"
#include <MddBootstrap.h>

#include "App.xaml.h"
#include "MainWindow.xaml.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Navigation;
using namespace KinectToVR;
using namespace KinectToVR::implementation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

/// <summary>
/// Initializes the singleton application object.  This is the first line of authored code
/// executed, and as such is the logical equivalent of main() or WinMain().
/// </summary>
App::App()
{
    // Request the dark theme to be set
    this->RequestedTheme(ApplicationTheme::Dark);

    InitializeComponent();
    
    Windows::UI::ViewManagement::ApplicationView::PreferredLaunchViewSize(Windows::Foundation::Size(1000, 700));
    Windows::UI::ViewManagement::ApplicationView::PreferredLaunchWindowingMode(Windows::UI::ViewManagement::ApplicationViewWindowingMode::PreferredLaunchViewSize);
    
#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
    UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e)
    {
        if (IsDebuggerPresent())
        {
            auto errorMessage = e.Message(); // LOG it?
            __debugbreak();
        }
    });
#endif
}

/// <summary>
/// Invoked when the application is launched normally by the end user.  Other entry points
/// will be used such as when the application is launched to open a specific file.
/// </summary>
/// <param name="e">Details about the launch request and process.</param>
void App::OnLaunched(LaunchActivatedEventArgs const&)
{
    // Unsupported in AppSDK 1.0

    //// Take a dependency on Windows App SDK Stable.
    //const UINT32 majorMinorVersion{ 0x00010000 };
    //PCWSTR versionTag{ L"" };
    //const PACKAGE_VERSION minVersion{};

    //const HRESULT hr{ MddBootstrapInitialize(majorMinorVersion, versionTag, minVersion) };

    //// Check the return code. If there is a failure, display the result.
    //if (FAILED(hr))
    //{
    //    wprintf(L"Error 0x%X in MddBootstrapInitialize(0x%08X, %s, %hu.%hu.%hu.%hu)\n",
    //        hr, majorMinorVersion, versionTag, minVersion.Major, minVersion.Minor, minVersion.Build, minVersion.Revision);
    //    return; //hr;
    //}

    window = make<MainWindow>();
    window.Activate();

    //// Release the DDLM and clean up.
    //MddBootstrapShutdown();

}
