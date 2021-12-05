#include "pch.h"
#include "ControllersPage.xaml.h"
#if __has_include("ControllersPage.g.cpp")
#include "ControllersPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::KinectToVR::implementation
{
    ControllersPage::ControllersPage()
    {
        InitializeComponent();
    }

    int32_t ControllersPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void ControllersPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void ControllersPage::myButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        myButton().Content(box_value(L"Clicked"));
    }
}
