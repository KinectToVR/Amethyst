using System.Collections.Generic;
using Amethyst.Classes;
using Amethyst.MVVM;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Controls;

public sealed partial class LoadAttemptedPluginsView : UserControl
{
    public LoadAttemptedPluginsView()
    {
        InitializeComponent();
    }

    public IEnumerable<LoadAttemptedPlugin> DisplayedPlugins { get; set; }

    private void Expander_Expanding(Expander sender, ExpanderExpandingEventArgs args)
    {
        if (!(sender?.IsLoaded ?? false)) return;
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }
}