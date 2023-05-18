using System.Collections.Generic;
using Amethyst.MVVM;
using Amethyst.Utils;
using Microsoft.UI.Xaml.Controls;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Controls;

public sealed partial class LoadAttemptedPluginsView : UserControl
{
    public IEnumerable<LoadAttemptedPlugin> DisplayedPlugins { get; set; }

    public LoadAttemptedPluginsView()
    {
        InitializeComponent();
    }

    private void Expander_Expanding(Expander sender, ExpanderExpandingEventArgs args)
    {
        if (!(sender?.IsLoaded ?? false)) return;
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }
}