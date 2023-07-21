using System;
using System.ComponentModel;
using System.Threading.Tasks;
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
    public string Header => "Welcome to";
    public string Message => null;
    public UserControl Control => new TitleControl();
    public object ActionMessage => "Continue";
    public Func<Task> Action { get; set; }
}

public class EndingSplash : ICustomSplash
{
    public bool ShowVideo => true;
    public string Header => "Amethyst is now set up.";
    public string Message => "Have fun!";
    public UserControl Control => null;
    public object ActionMessage => "Start Amethyst";
    public Func<Task> Action { get; set; }
}