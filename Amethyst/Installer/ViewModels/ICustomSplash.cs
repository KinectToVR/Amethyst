using System;
using System.ComponentModel;
using System.Threading.Tasks;
using Amethyst.Classes;
using Amethyst.Installer.Controls;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;

namespace Amethyst.Installer.ViewModels;

public interface ICustomSplash
{
    public bool ShowVideo { get; }
    public string Header { get; }
    public string Message { get; }
    public UserControl Control { get; }

    [DefaultValue(null)] public object ActionMessage { get; }
    public Func<Task> Action { get; }

    public bool HasMessageText => !string.IsNullOrEmpty(Message);
    public bool HasMessageObject => Control is not null;
    public bool HasAction => ActionMessage is not null && Action is not null;
}

public class WelcomeSplash : ICustomSplash
{
    public bool ShowVideo => false;
    public string Header => Interfacing.LocalizedJsonString("/Installer/Splashes/Welcome/Header");
    public string Message => null;
    public UserControl Control => new TitleControl();
    public object ActionMessage => Interfacing.LocalizedJsonString("/Installer/Splashes/Welcome/ActionMessage");
    public Func<Task> Action { get; set; }
}

public class EndingSplash : ICustomSplash
{
    public bool ShowVideo => true;
    public string Header => Interfacing.LocalizedJsonString("/Installer/Splashes/Ending/Header");
    public string Message => Interfacing.LocalizedJsonString("/Installer/Splashes/Ending/Message");
    public UserControl Control => null;
    public object ActionMessage => Interfacing.LocalizedJsonString("/Installer/Splashes/Ending/ActionMessage");
    public Func<Task> Action { get; set; }
}