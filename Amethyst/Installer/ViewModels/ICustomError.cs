using System;
using System.ComponentModel;
using System.Threading.Tasks;
using Microsoft.UI.Xaml;
using WinUI.Fluent.Icons;

namespace Amethyst.Installer.ViewModels;

public interface ICustomError
{
    public string Title { get; }
    public string Message { get; }
    public bool CanContinue { get; }

    public FluentSymbolIcon Icon { get; }

    [DefaultValue(null)]
    public object ActionMessage { get; }
    public Func<Task> Action { get; }

    public bool HasIcon => Icon is not null;
    public bool HasAction => ActionMessage is not null && Action is not null;
}