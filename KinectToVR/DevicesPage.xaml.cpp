#include "pch.h"
#include "DevicesPage.xaml.h"
#if __has_include("DevicesPage.g.cpp")
#include "DevicesPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::KinectToVR::implementation
{
	DevicesPage::DevicesPage()
	{
		InitializeComponent();

		// Create tracking devices' list
		Windows::Foundation::Collections::IObservableVector<KinectToVR::TrackingDevicesView> m_TrackingDevicesViewModels
		{
			single_threaded_observable_vector<KinectToVR::TrackingDevicesView>()
		};
		
		// Watch for insertions
		m_TrackingDevicesViewModels.VectorChanged(
			[&](Windows::Foundation::Collections::IObservableVector<KinectToVR::TrackingDevicesView> const& sender,
			   Windows::Foundation::Collections::IVectorChangedEventArgs const& args)
			{
				// Report a registration and parse (eventually remove the last item)

				// Set the current device
				sender.GetAt(0).Current(true);

				// Re-set all indexes
				for (uint32_t i = 0; i < sender.Size(); i++)
				{
					auto item = sender.GetAt(i);
					item.DeviceID(i + 1);
				}
			});

		// Add tracking devices here
		m_TrackingDevicesViewModels.Append(
			make<TrackingDevicesView>(L"Kinect V1 (X360)", L"🟢 Connected", L"🟢 Online"));

		m_TrackingDevicesViewModels.Append(
			make<TrackingDevicesView>(L"Kinect V2 (XBONE)", L"🟢 Connected", L"🔴 Offline"));

		m_TrackingDevicesViewModels.Append(
			make<TrackingDevicesView>(L"PSMoveService", L"🔴 Disconnected", L"🔴 Offline"));
		
		// Set currently tracking device & selected device
		// RadioButton is set on ItemChanged
		TrackingDeviceListView().SelectedIndex(1);
		
		// Register tracking devices' list
		TrackingDeviceListView().ItemsSource(m_TrackingDevicesViewModels);
	}
}


void winrt::KinectToVR::implementation::DevicesPage::TrackingDeviceListView_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{

}

void winrt::KinectToVR::implementation::DevicesPage::RefreshDeviceButton_Tapped(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Input::TappedRoutedEventArgs const& e)
{
	const auto ui_element = sender.as<Controls::NavigationViewItem>();
	auto tracking_device = ui_element.DataContext().as<TrackingDevicesView>();

	/*Media::RotateTransform rotateTransform;
	rotateTransform.CenterX(0.5);
	rotateTransform.CenterY(0.5);
	rotateTransform.Angle(0);
	
	refresh_icon.RenderTransform(rotateTransform);

	Duration duration{ std::chrono::seconds(1) };

	Media::Animation::DoubleAnimation animation;
	animation.Duration(duration);

	Media::Animation::Storyboard storyboard;
	storyboard.Duration(duration);

	storyboard.Children().Append(animation);

	storyboard.SetTarget(animation, rotateTransform);

	storyboard.SetTargetProperty(animation, L"(UIElement.RenderTransform).(CompositeTransform.Rotation)");
	animation.From(0.0);
	animation.To(360.0);

	storyboard.SpeedRatio(1.0);
	storyboard.Begin();*/
}
