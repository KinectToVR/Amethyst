using WinUI.Fluent.Icons;

namespace Amethyst.Installer.ViewModels;

public class CustomError
{
    public string Title { get; set; }
    public string Message { get; set; }
    public bool CanContinue { get; set; }

    public FluentSymbolIcon Icon { get; set; }

    public bool HasIcon => Icon is not null;
}