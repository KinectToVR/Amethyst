#include "pch.h"
#include "MainWindow.xaml.h"

#include <winrt/impl/Windows.UI.Xaml.Media.Animation.2.h>

#include "App.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::App1::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();
        
        this->ExtendsContentIntoTitleBar(true);
        this->SetTitleBar(DragElement());
        
        m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
            (L"general", winrt::xaml_typename<GeneralPage>()));
        m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
            (L"controllers", winrt::xaml_typename<ControllersPage>()));
        m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
            (L"devices", winrt::xaml_typename<DevicesPage>()));
        m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
            (L"configuration", winrt::xaml_typename<ConfigurationPage>()));
    }
}

void winrt::App1::implementation::MainWindow::NavView_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    // NavView doesn't load any page by default, so load home page.
    NavView().SelectedItem(NavView().MenuItems().GetAt(0));

    // If navigation occurs on SelectionChanged, then this isn't needed.
    // Because we use ItemInvoked to navigate, we need to call Navigate
    // here to load the home page.
    NavView_Navigate(L"general",
        Microsoft::UI::Xaml::Media::Animation::EntranceNavigationTransitionInfo());
}

void winrt::App1::implementation::MainWindow::NavView_ItemInvoked(winrt::Microsoft::UI::Xaml::Controls::NavigationView const& sender, winrt::Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs const& args)
{
    NavView_Navigate(
        winrt::unbox_value_or<winrt::hstring>(
            args.InvokedItemContainer().Tag(), L"").c_str(),
        args.RecommendedNavigationTransitionInfo());
}

void winrt::App1::implementation::MainWindow::NavView_Navigate(
    std::wstring navItemTag,
    Microsoft::UI::Xaml::Media::Animation::NavigationTransitionInfo const& transitionInfo)
{
    Windows::UI::Xaml::Interop::TypeName pageTypeName;

    for (auto&& eachPage : m_pages)
    {
        if (eachPage.first == navItemTag)
        {
            pageTypeName = eachPage.second;
            break;
        }
    }

    // Get the page type before navigation so you can prevent duplicate
    // entries in the backstack.
    Windows::UI::Xaml::Interop::TypeName preNavPageType =
        ContentFrame().CurrentSourcePageType();

    // Navigate only if the selected page isn't currently loaded.
    if (pageTypeName.Name != L"" && preNavPageType.Name != pageTypeName.Name)
    {
        ContentFrame().Navigate(pageTypeName, nullptr, transitionInfo);
    }
}

void winrt::App1::implementation::MainWindow::ContentFrame_NavigationFailed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Navigation::NavigationFailedEventArgs const& e)
{
    throw winrt::hresult_error(
        E_FAIL, winrt::hstring(L"Failed to load Page ") + e.SourcePageType().Name);
}

void winrt::App1::implementation::MainWindow::NavView_BackRequested(winrt::Microsoft::UI::Xaml::Controls::NavigationView const& sender, winrt::Microsoft::UI::Xaml::Controls::NavigationViewBackRequestedEventArgs const& args)
{
    TryGoBack();
}

void winrt::App1::implementation::MainWindow::CoreDispatcher_AcceleratorKeyActivated(
    Windows::UI::Core::CoreDispatcher const& /* sender */,
    Windows::UI::Core::AcceleratorKeyEventArgs const& args)
{
    // When Alt+Left are pressed navigate back
    if (args.EventType() == Windows::UI::Core::CoreAcceleratorKeyEventType::SystemKeyDown
        && args.VirtualKey() == Windows::System::VirtualKey::Left
        && args.KeyStatus().IsMenuKeyDown
        && !args.Handled())
    {
        args.Handled(TryGoBack());
    }
}

void winrt::App1::implementation::MainWindow::CoreWindow_PointerPressed(
    Windows::UI::Core::CoreWindow const& /* sender */,
    Windows::UI::Core::PointerEventArgs const& args)
{
    // Handle mouse back button.
    if (args.CurrentPoint().Properties().IsXButton1Pressed())
    {
        args.Handled(TryGoBack());
    }
}

void winrt::App1::implementation::MainWindow::System_BackRequested(
    Windows::Foundation::IInspectable const& /* sender */,
    Windows::UI::Core::BackRequestedEventArgs const& args)
{
    if (!args.Handled())
    {
        args.Handled(TryGoBack());
    }
}

bool winrt::App1::implementation::MainWindow::TryGoBack()
{
    if (!ContentFrame().CanGoBack())
        return false;
    // Don't go back if the nav pane is overlayed.
    if (NavView().IsPaneOpen() &&
        (NavView().DisplayMode() == muxc::NavigationViewDisplayMode::Compact ||
            NavView().DisplayMode() == muxc::NavigationViewDisplayMode::Minimal))
        return false;
    ContentFrame().GoBack();
    return true;
}

void winrt::App1::implementation::MainWindow::On_Navigated(
    Windows::Foundation::IInspectable const& /* sender */,
    Windows::UI::Xaml::Navigation::NavigationEventArgs const& args)
{
    NavView().IsBackEnabled(ContentFrame().CanGoBack());

    if (ContentFrame().SourcePageType().Name != L"")
    {
        for (auto&& eachPage : m_pages)
        {
            if (eachPage.second.Name == args.SourcePageType().Name)
            {
                for (auto&& eachMenuItem : NavView().MenuItems())
                {
                    auto navigationViewItem =
                        eachMenuItem.try_as<muxc::NavigationViewItem>();
                    {
                        if (navigationViewItem)
                        {
                            winrt::hstring hstringValue =
                                winrt::unbox_value_or<winrt::hstring>(
                                    navigationViewItem.Tag(), L"");
                            if (hstringValue == eachPage.first)
                            {
                                NavView().SelectedItem(navigationViewItem);
                                NavView().Header(navigationViewItem.Content());
                            }
                        }
                    }
                }
                break;
            }
        }
    }
}

void winrt::App1::implementation::MainWindow::Exit_Tapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::TappedRoutedEventArgs const& e)
{
    // Save and Exit with 0
    exit(0);
}

void winrt::App1::implementation::MainWindow::Minimize_Tapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::TappedRoutedEventArgs const& e)
{
    // Minimize with win+down
    INPUT inputs[4] = {};
    ZeroMemory(inputs, sizeof(inputs));

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_LWIN;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_DOWN;

    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = VK_DOWN;
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_LWIN;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

void winrt::App1::implementation::MainWindow::Update_Tapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::TappedRoutedEventArgs const& e)
{
    
}
