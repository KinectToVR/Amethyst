using System;
using System.Threading.Tasks;
using WinUI.Fluent.Icons;

namespace Amethyst.Installer.ViewModels;

public class CustomError
{
    public string Title { get; set; }
    public string Message { get; set; }
    public bool CanContinue { get; set; }

    public FluentSymbolIcon Icon { get; set; }

    public string ActionMessage { get; set; } = null;
    public Func<Task> Action { get; set; }

    public bool HasIcon => Icon is not null;
    public bool HasAction => !string.IsNullOrEmpty(ActionMessage) && Action is not null;
}