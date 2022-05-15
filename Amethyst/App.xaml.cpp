#include "pch.h"
#include <MddBootstrap.h>

#include "App.xaml.h"
#include "MainWindow.xaml.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml::Navigation;
using namespace KinectToVR;
using namespace KinectToVR::implementation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

#ifdef _DEBUG
#define DBGPRINT(kwszDebugFormatString, ...) _DBGPRINT(__FUNCTIONW__, __LINE__, kwszDebugFormatString, __VA_ARGS__)

VOID _DBGPRINT(LPCWSTR kwszFunction, INT iLineNumber, LPCWSTR kwszDebugFormatString, ...) \
{
    INT cbFormatString = 0;
    va_list args;
    PWCHAR wszDebugString = NULL;
    size_t st_Offset = 0;

    va_start(args, kwszDebugFormatString);

    cbFormatString = _scwprintf(L"[%s:%d] ", kwszFunction, iLineNumber) * sizeof(WCHAR);
    cbFormatString += _vscwprintf(kwszDebugFormatString, args) * sizeof(WCHAR) + 2;

    /* Depending on the size of the format string, allocate space on the stack or the heap. */
    wszDebugString = (PWCHAR)_malloca(cbFormatString);

    /* Populate the buffer with the contents of the format string. */
    StringCbPrintfW(wszDebugString, cbFormatString, L"[%s:%d] ", kwszFunction, iLineNumber);
    StringCbLengthW(wszDebugString, cbFormatString, &st_Offset);
    StringCbVPrintfW(&wszDebugString[st_Offset / sizeof(WCHAR)], cbFormatString - st_Offset, kwszDebugFormatString, args);

    OutputDebugStringW(wszDebugString);

    _freea(wszDebugString);
    va_end(args);
}
#else
#define DBGPRINT( kwszDebugFormatString, ... ) ;;
#endif

winrt::Microsoft::Windows::AppNotifications::AppNotification CreateToastNotification()
{
    winrt::hstring payload =
        LR"(<toast launch="action = viewDownload &amp; downloadId = 9438108">
        <visual>
            <binding template = "ToastGeneric">
                <text>Downloading this week's new music...</text>
            </binding>
        </visual>
    </toast>)";
    return winrt::Microsoft::Windows::AppNotifications::AppNotification(payload);
}

/// <summary>
/// Initializes the singleton application object.  This is the first line of authored code
/// executed, and as such is the logical equivalent of main() or WinMain().
/// </summary>
App::App()
{
    /* Set up everything before the launch */

    // Request the dark theme to be set
    this->RequestedTheme(ApplicationTheme::Dark);

    /* Bootstrap here (still waiting for stable 1.1) */
    
    /* Initialize the main app and launch it */

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
    window = make<MainWindow>();
    window.Activate();
}
