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

// LMAO Eat Dirt Micro&Soft
// Imma just cache object before the fancy UWP delegation reownership
std::shared_ptr<Controls::FontIcon> updateIconDot;

// Helper local variables
bool updateFound = false,
     main_localInitFinished = false;

// Updates checking function
Windows::Foundation::IAsyncAction winrt::KinectToVR::implementation::MainWindow::checkUpdates(
	winrt::Microsoft::UI::Xaml::UIElement const& show_el, const bool show, const DWORD delay_ms)
{
	// Attempt only after init
	if (main_localInitFinished) {

		{
			// Sleep on UI (Non-blocking)
			winrt::apartment_context ui_thread;
			co_await winrt::resume_background();
			Sleep(delay_ms);
			co_await ui_thread;
		}

		// Don't check if found
		if (!updateFound)
		{
			// Here check for updates (via external bool)
			IconRotation().Begin();
			// Capture the calling context.
			winrt::apartment_context ui_thread;
			co_await winrt::resume_background();

			// Check for updates
			auto start_time = std::chrono::high_resolution_clock::now();

			// Check now
			srand(time(NULL)); // Generate somewhat random up-to-date or not
			updateFound = rand() & 1;

			// Limit time to (min) 1s
			if (auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::high_resolution_clock::now() - start_time).count(); duration <= 1000.f)
				std::this_thread::sleep_for(std::chrono::milliseconds(1000 - duration));

			// Resume to UI and stop the animation
			co_await ui_thread;
			IconRotation().Stop();

			if (updateFound)
			{
				FlyoutHeader().Text(L"New Update Available");
				FlyoutFooter().Text(L"KinectToVR v1.0.1");
				FlyoutContent().Text(L"- OpenSSL Library update\n- Several Kinect V2 fixes");

				auto thickness = Thickness();
				thickness.Left = 0;
				thickness.Top = 0;
				thickness.Right = 0;
				thickness.Bottom = 12;
				FlyoutContent().Margin(thickness);

				InstallLaterButton().Visibility(Visibility::Visible);
				InstallNowButton().Visibility(Visibility::Visible);

				updateIconDot.get()->Visibility(Visibility::Visible);
			}
			else
			{
				FlyoutHeader().Text(L"You're Up To Date");
				FlyoutFooter().Text(L"KinectToVR v1.0.0");
				FlyoutContent().Text(L"Please tell us if you have any ideas\nfor the next KinectToVR update.");

				auto thickness = Thickness();
				thickness.Left = 0;
				thickness.Top = 0;
				thickness.Right = 0;
				thickness.Bottom = 0;
				FlyoutContent().Margin(thickness);

				InstallLaterButton().Visibility(Visibility::Collapsed);
				InstallNowButton().Visibility(Visibility::Collapsed);

				updateIconDot.get()->Visibility(Visibility::Collapsed);
			}
		}

		Controls::Primitives::FlyoutShowOptions options;
		options.Placement(Controls::Primitives::FlyoutPlacementMode::Bottom);
		options.ShowMode(Controls::Primitives::FlyoutShowMode::Transient);

		if (updateFound || show)
			UpdateFlyout().ShowAt(show_el, options);

	}
}

namespace winrt::KinectToVR::implementation
{
	MainWindow::MainWindow()
	{
		InitializeComponent();

		// Cache needed UI elements
		updateIconDot = std::make_shared<Controls::FontIcon>(UpdateIconDot());

		// Set up
		this->Title(L"KinectToVR");

		this->ExtendsContentIntoTitleBar(true);
		this->SetTitleBar(DragElement());

		m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
			(L"general", winrt::xaml_typename<GeneralPage>()));
		m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
			(L"settings", winrt::xaml_typename<SettingsPage>()));
		m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
			(L"devices", winrt::xaml_typename<DevicesPage>()));
		m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
			(L"info", winrt::xaml_typename<InfoPage>()));

		// Notify of the setup end
		main_localInitFinished = true;
	}
}

void winrt::KinectToVR::implementation::MainWindow::NavView_Loaded(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// NavView doesn't load any page by default, so load home page.
	NavView().SelectedItem(NavView().MenuItems().GetAt(0));

	// If navigation occurs on SelectionChanged, then this isn't needed.
	// Because we use ItemInvoked to navigate, we need to call Navigate
	// here to load the home page.
	NavView_Navigate(L"general",
	                 Microsoft::UI::Xaml::Media::Animation::EntranceNavigationTransitionInfo());
}

void winrt::KinectToVR::implementation::MainWindow::NavView_ItemInvoked(
	winrt::Microsoft::UI::Xaml::Controls::NavigationView const& sender,
	winrt::Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs const& args)
{
	NavView_Navigate(
		winrt::unbox_value_or<winrt::hstring>(
			args.InvokedItemContainer().Tag(), L"").c_str(),
		args.RecommendedNavigationTransitionInfo());
}

void winrt::KinectToVR::implementation::MainWindow::NavView_Navigate(
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

void winrt::KinectToVR::implementation::MainWindow::ContentFrame_NavigationFailed(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Navigation::NavigationFailedEventArgs const& e)
{
	throw winrt::hresult_error(
		E_FAIL, winrt::hstring(L"Failed to load Page ") + e.SourcePageType().Name);
}

void winrt::KinectToVR::implementation::MainWindow::NavView_BackRequested(
	winrt::Microsoft::UI::Xaml::Controls::NavigationView const& sender,
	winrt::Microsoft::UI::Xaml::Controls::NavigationViewBackRequestedEventArgs const& args)
{
	TryGoBack();
}

void winrt::KinectToVR::implementation::MainWindow::CoreDispatcher_AcceleratorKeyActivated(
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

void winrt::KinectToVR::implementation::MainWindow::CoreWindow_PointerPressed(
	Windows::UI::Core::CoreWindow const& /* sender */,
	Windows::UI::Core::PointerEventArgs const& args)
{
	// Handle mouse back button.
	if (args.CurrentPoint().Properties().IsXButton1Pressed())
	{
		args.Handled(TryGoBack());
	}
}

void winrt::KinectToVR::implementation::MainWindow::System_BackRequested(
	Windows::Foundation::IInspectable const& /* sender */,
	Windows::UI::Core::BackRequestedEventArgs const& args)
{
	if (!args.Handled())
	{
		args.Handled(TryGoBack());
	}
}

bool winrt::KinectToVR::implementation::MainWindow::TryGoBack()
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

void winrt::KinectToVR::implementation::MainWindow::On_Navigated(
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

Windows::UI::Xaml::Controls::Primitives::Popup GetPopup()
{
	auto popups =
		Windows::UI::Xaml::Media::VisualTreeHelper::GetOpenPopups(Windows::UI::Xaml::Window::Current());
	if (popups.Size() > 0)
		return popups.GetAt(0);
	return nullptr;
}

void winrt::KinectToVR::implementation::MainWindow::InstallLaterButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	UpdateFlyout().Hide();
}

void winrt::KinectToVR::implementation::MainWindow::InstallNowButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	UpdateFlyout().Hide();
}


void winrt::KinectToVR::implementation::MainWindow::ExitButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Save and Exit with 0
	exit(0);
}


void winrt::KinectToVR::implementation::MainWindow::MinimizeButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
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


Windows::Foundation::IAsyncAction winrt::KinectToVR::implementation::MainWindow::UpdateButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Check for updates (and show)
	co_await checkUpdates(sender.as<UIElement>(), true);
}


Windows::Foundation::IAsyncAction winrt::KinectToVR::implementation::MainWindow::UpdateButton_Loaded(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Check for updates (and show)
	co_await checkUpdates(sender.as<UIElement>(), true, 2000);
}
