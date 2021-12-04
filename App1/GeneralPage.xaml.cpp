#include "pch.h"
#include "GeneralPage.xaml.h"
#if __has_include("GeneralPage.g.cpp")
#include "GeneralPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::App1::implementation
{
    GeneralPage::GeneralPage()
    {
        InitializeComponent();
    }

    int32_t GeneralPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void GeneralPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void GeneralPage::myButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        myButton().Content(box_value(L"Clicked"));
    }
}
