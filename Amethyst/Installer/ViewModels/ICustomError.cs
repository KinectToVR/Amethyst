using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Threading.Tasks;
using Amethyst.Classes;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using WinUI.Fluent.Icons;

namespace Amethyst.Installer.ViewModels;

public interface ICustomError : INotifyPropertyChanged
{
    public string Title { get; }
    public string Message { get; }
    public bool CanContinue { get; }
    public string TipTitle { get; }
    public string TipMessage { get; }

    public FluentSymbolIcon Icon { get; }

    [DefaultValue(null)] public object ActionMessage { get; }
    public Func<Task> Action { get; }

    public bool HasIcon => Icon is not null;
    public bool HasAction => ActionMessage is not null && Action is not null;
}

public class PermissionsError : ICustomError
{
    public PermissionsError()
    {
        Translator.Get.PropertyChanged +=
            (_, _) => Shared.Main.DispatcherQueue.TryEnqueue(() => { OnPropertyChanged(); });
    }

    public string Title => Interfacing.LocalizedJsonString("/Installer/Views/SetupError/Admin/Title");
    public string Message => Interfacing.LocalizedJsonString("/Installer/Views/SetupError/Admin/Message");

    public string TipTitle => Interfacing.LocalizedJsonString("/Installer/Views/SetupError/Admin/Tip/Title");
    public string TipMessage => Interfacing.LocalizedJsonString("/Installer/Views/SetupError/Admin/Tip/Message");

    public bool CanContinue => true;
    public FluentSymbolIcon Icon => new(FluentSymbol.Shield48);
    public Func<Task> Action { get; set; }

    public object ActionMessage => new StackPanel
    {
        Orientation = Orientation.Horizontal,
        Children =
        {
            new FluentSymbolIcon(FluentSymbol.Shield20),
            new TextBlock
            {
                Margin = new Thickness { Left = 10 },
                FontSize = 15,
                Text = Interfacing.LocalizedJsonString("/Installer/Views/SetupError/Admin/Action")
            }
        }
    };

    public event PropertyChangedEventHandler PropertyChanged;

    private void OnPropertyChanged(string propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}

public class InternetError : ICustomError
{
    public InternetError()
    {
        Translator.Get.PropertyChanged +=
            (_, _) => Shared.Main.DispatcherQueue.TryEnqueue(() => { OnPropertyChanged(); });
    }

    public string Title => Interfacing.LocalizedJsonString("/Installer/Views/SetupError/Internet/Title");
    public string Message => Interfacing.LocalizedJsonString("/Installer/Views/SetupError/Internet/Message");

    public string TipTitle => Interfacing.LocalizedJsonString("/Installer/Views/SetupError/Internet/Tip/Title");
    public string TipMessage => Interfacing.LocalizedJsonString("/Installer/Views/SetupError/Internet/Tip/Message");

    public bool CanContinue => true;
    public FluentSymbolIcon Icon => new(FluentSymbol.WiFiOff24);
    public Func<Task> Action => null;
    public object ActionMessage => null;

    public event PropertyChangedEventHandler PropertyChanged;

    private void OnPropertyChanged(string propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}

public class PluginsError : ICustomError
{
    public PluginsError()
    {
        Translator.Get.PropertyChanged +=
            (_, _) => Shared.Main.DispatcherQueue.TryEnqueue(() => { OnPropertyChanged(); });
    }

    public string Title => Interfacing.LocalizedJsonString("/Installer/Views/SetupError/Plugins/Title");
    public string Message => Interfacing.LocalizedJsonString("/Installer/Views/SetupError/Plugins/Message");

    public string TipTitle => null;
    public string TipMessage => null;
    public bool CanContinue => false;

    public FluentSymbolIcon Icon => new(FluentSymbol.PlugDisconnected28);
    public Func<Task> Action => null;
    public object ActionMessage => null;

    public event PropertyChangedEventHandler PropertyChanged;

    private void OnPropertyChanged(string propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}