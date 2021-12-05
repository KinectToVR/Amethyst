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
    }

    int32_t DevicesPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void DevicesPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void DevicesPage::myButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        myButton().Content(box_value(L"Clicked"));
    }
}
