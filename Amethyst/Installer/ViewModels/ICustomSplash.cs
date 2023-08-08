using System;
using System.ComponentModel;
using System.Threading.Tasks;
using Windows.System;
using Amethyst.Classes;
using Amethyst.Installer.Controls;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;

namespace Amethyst.Installer.ViewModels;

public interface ICustomSplash : INotifyPropertyChanged
{
    public bool ShowVideo { get; }
    public string Header { get; }
    public string Message { get; }
    public UserControl Control { get; }

    [DefaultValue(null)] public object ActionMessage { get; }
    public Func<Task> Action { get; }

    [DefaultValue(null)] public string BottomText { get; }
    public Func<Task> BottomTextAction { get; }

    public bool HasMessageText => !string.IsNullOrEmpty(Message);
    public bool HasMessageObject => Control is not null;
    public bool HasAction => ActionMessage is not null && Action is not null;
    public bool HasBottomTextAction => BottomText is not null && BottomTextAction is not null;
}

public class WelcomeSplash : ICustomSplash
{
    public WelcomeSplash()
    {
        Translator.Get.PropertyChanged +=
            (_, _) => Shared.Main.DispatcherQueue.TryEnqueue(() => { OnPropertyChanged(); });
    }

    public bool ShowVideo => false;
    public string Header => Interfacing.LocalizedJsonString("/Installer/Splashes/Welcome/Header");
    public string Message => null;
    public UserControl Control => new TitleControl();

    public object ActionMessage => Interfacing.LocalizedJsonString("/Installer/Splashes/Welcome/ActionMessage");
    public Func<Task> Action { get; set; }

    public string BottomText => Interfacing.LocalizedJsonString("/SharedStrings/Buttons/Help/Licenses");

    public Func<Task> BottomTextAction => async () =>
    {
        await Launcher.LaunchUriAsync(
            "https://github.com/KinectToVR/Amethyst/blob/main/Amethyst/Assets/Licenses.txt".ToUri());
    };

    public event PropertyChangedEventHandler PropertyChanged;

    private void OnPropertyChanged(string propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}

public class EndingSplash : ICustomSplash
{
    public EndingSplash()
    {
        Translator.Get.PropertyChanged +=
            (_, _) => Shared.Main.DispatcherQueue.TryEnqueue(() => { OnPropertyChanged(); });
    }

    public bool ShowVideo => true;
    public string Header => Interfacing.LocalizedJsonString("/Installer/Splashes/Ending/Header");
    public string Message => Interfacing.LocalizedJsonString("/Installer/Splashes/Ending/Message");
    public UserControl Control => null;

    public object ActionMessage => Interfacing.LocalizedJsonString("/Installer/Splashes/Ending/ActionMessage");
    public Func<Task> Action { get; set; }

    public string BottomText =>
        Interfacing.LocalizedJsonString("/SharedStrings/Buttons/Help/Docs/InfoPage/OpenCollective");

    public Func<Task> BottomTextAction => async () =>
    {
        await Launcher.LaunchUriAsync("https://opencollective.com/k2vr".ToUri());
    };

    public event PropertyChangedEventHandler PropertyChanged;

    private void OnPropertyChanged(string propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}