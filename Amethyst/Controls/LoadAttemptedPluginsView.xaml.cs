using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using Amethyst.Classes;
using Amethyst.MVVM;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Input;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Controls;

public sealed partial class LoadAttemptedPluginsView : UserControl
{
    private MenuFlyout FixesFlyout { get; set; }

    public LoadAttemptedPluginsView()
    {
        InitializeComponent();

        FixesFlyout = new MenuFlyout();
    }

    public IEnumerable<LoadAttemptedPlugin> DisplayedPlugins { get; set; }

    private void Expander_Expanding(Expander sender, ExpanderExpandingEventArgs args)
    {
        if (!(sender?.IsLoaded ?? false)) return;
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void ExpanderTitle_OnRightTapped(object sender, RightTappedRoutedEventArgs e)
    {
        if (sender is not Grid { DataContext: LoadAttemptedPlugin { DependencyInstaller: not null } plugin } grid) return;
        if (plugin.InstallHandler?.InstallingDependencies ?? false) return;

        FixesFlyout.Items.Clear();

        plugin.DependencyInstaller.ListDependencies().Select(
            x =>
            {
                var item = new MenuFlyoutItem
                {
                    Text = $"{Interfacing.LocalizedJsonString("/PluginManager/Install")}{x.Name}"
                };

                item.Click += async (_, _) =>
                {
                    await plugin.InstallPluginDependency(x);
                    FixesFlyout.Hide();
                };

                return item;
            }).ToList().ForEach(FixesFlyout.Items.Add);

        var fixes = plugin.DependencyInstaller.ListFixes();

        if (FixesFlyout.Items.Count > 0 && fixes.Count > 0)
            FixesFlyout.Items.Add(new MenuFlyoutSeparator());

        fixes.Select(x =>
        {
            var item = new MenuFlyoutItem
            {
                Text = $"{Interfacing.LocalizedJsonString("/PluginManager/Fix")}{x.Name}"
            };

            item.Click += async (_, _) =>
            {
                await plugin.ApplyPluginFix(x);
                FixesFlyout.Hide();
            };

            return item;
        }).ToList().ForEach(FixesFlyout.Items.Add);

        if (FixesFlyout.Items.Any())
            FixesFlyout.ShowAt(grid, new FlyoutShowOptions { Placement = FlyoutPlacementMode.BottomEdgeAlignedLeft });
    }
}