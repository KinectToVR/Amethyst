#include "pch.h"
#include "ConsolePage.xaml.h"
#if __has_include("ConsolePage.g.cpp")
#include "ConsolePage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::KinectToVR::implementation
{
    ConsolePage::ConsolePage()
    {
        InitializeComponent();
    }
}
