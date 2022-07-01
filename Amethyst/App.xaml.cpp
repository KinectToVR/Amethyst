#include "pch.h"
#include <MddBootstrap.h>

#include "App.xaml.h"
#include "MainWindow.xaml.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace winrt::Microsoft::UI::Xaml;
using namespace Controls;
using namespace Navigation;
using namespace Amethyst;
using namespace implementation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

/// <summary>
/// Initializes the singleton application object.  This is the first line of authored code
/// executed, and as such is the logical equivalent of main() or WinMain().
/// </summary>
App::App()
{
	/* Set up everything before the launch */

	/* Initialize the main app and launch it */

	InitializeComponent();

	Windows::UI::ViewManagement::ApplicationView::PreferredLaunchViewSize(Size(1000, 700));
	Windows::UI::ViewManagement::ApplicationView::PreferredLaunchWindowingMode(
		Windows::UI::ViewManagement::ApplicationViewWindowingMode::PreferredLaunchViewSize);

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
void App::OnLaunched(const LaunchActivatedEventArgs&)
{
	window = make<MainWindow>();
	window.Activate();
}
