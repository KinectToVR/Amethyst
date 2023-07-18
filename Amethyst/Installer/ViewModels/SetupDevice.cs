using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using System.Threading;
using System.Threading.Tasks;
using Amethyst.Classes;
using Amethyst.MVVM;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;
using Microsoft.UI.Xaml;

namespace Amethyst.Installer.ViewModels;

public class SetupPlugin
{
    public Type PluginType => CoreSetupData.PluginType;
    public string Name { get; init; } = "[UNKNOWN]";
    public string Guid { get; init; } = "[INVALID]";
    public string Publisher { get; init; }
    public string Website { get; init; }

    public string DependencyLink { get; init; }
    public string DependencySource { get; init; }

    public IDependencyInstaller DependencyInstaller { get; init; }
    public ICoreSetupData CoreSetupData { get; init; }

    public LoadAttemptedPlugin.DependencyInstallHandler InstallHandler { get; } = new();

    public (LocalisationFileJson Root, string Directory) LocalizationResourcesRoot { get; set; }

    public Uri DependencyLinkUri => Uri.TryCreate(DependencyLink, UriKind.RelativeOrAbsolute, out var uri) ? uri : null;

    public Uri DependencySourceUri =>
        Uri.TryCreate(DependencySource, UriKind.RelativeOrAbsolute, out var uri) ? uri : null;

    public Uri WebsiteUri => Uri.TryCreate(Website, UriKind.RelativeOrAbsolute, out var uri) ? uri : null;

    public bool PublisherValid => !string.IsNullOrEmpty(Publisher);
    public bool WebsiteValid => !string.IsNullOrEmpty(Website);
    public bool GuidValid => !string.IsNullOrEmpty(Guid) && Guid is not "[INVALID]" and not "INVALID";

    public bool DependencyLinkValid => !string.IsNullOrEmpty(DependencyLink);
    public bool DependencySourceValid => !string.IsNullOrEmpty(DependencySource);
    public bool DependencyLinksValid => DependencyLinkValid && DependencySourceValid;

    public bool ShowDependencyInstaller => DependencyInstaller is not null;

    public bool ShowDependencyLinks =>
        !ShowDependencyInstaller && (DependencyLinkValid || DependencySourceValid);

    public async void InstallPluginDependencies(object sender, RoutedEventArgs e)
    {
        // Show the EULA, if provided by the installer
        foreach (var dependency in DependencyInstaller?.ListDependencies()?
                     .Where(x => !x.IsInstalled && x.IsMandatory &&
                                 !string.IsNullOrEmpty(x.InstallerEula)))
        {
            Shared.Main.EulaHeader.Text = $"{dependency.Name} EULA";
            Shared.Main.EulaText.Text = dependency.InstallerEula;

            Shared.Main.EulaFlyout.ShowAt(Shared.Main.MainGrid);
            await Shared.Main.EulaFlyoutClosed.WaitAsync();

            // Validate the result and continue/exit
            if (!Shared.Main.EulaFlyoutResult) return;
        }

        try
        {
            // Install plugin dep using the installer
            await PerformDependencyInstallation();
        }
        catch (Exception ex)
        {
            Logger.Error(ex);
        }
    }

    public void CancelDependencyInstallation()
    {
        try
        {
            InstallHandler?.TokenSource?.Cancel();
        }
        catch (Exception e)
        {
            Logger.Error(e);
        }
    }

    private async Task PerformDependencyInstallation()
    {
        /*
         * Logic
         * - Validate the DependencyInstaller
         * - Show the installation grid
         * - Disable all user input
         *
         * Success
         * - Show the last message, progress 100%S
         * - Change to 'success', wait 6s
         *
         * Failure
         * - Show the last message, progress 100%F
         * - Wait 6s, change to 'failure', wait 5s
         */

        /* Show the installer part */

        // Prepare a list of our dependencies
        var dependenciesToInstall = DependencyInstaller?.ListDependencies()
            .Where(x => !x.IsInstalled && x.IsMandatory).ToList();

        // Theoretically not possible, but check anyway
        if (dependenciesToInstall is null || !dependenciesToInstall.Any()) return;
        InstallHandler.TokenSource = new CancellationTokenSource();

        // Block temporarily
        InstallHandler.AllowUserInput = false;
        InstallHandler.OnPropertyChanged();
        await Task.Delay(500, InstallHandler.TokenSource.Token);

        // Show the installer grid
        InstallHandler.InstallationWorker = null;
        InstallHandler.InstallingDependencies = true;
        InstallHandler.ProgressError = false;
        InstallHandler.ProgressIndeterminate = true;
        InstallHandler.ProgressValue = 0.0;
        InstallHandler.StageName = string.Empty;
        InstallHandler.HideProgress = false;
        InstallHandler.OnPropertyChanged();

        // Unblock user input now
        await Task.Delay(500, InstallHandler.TokenSource.Token);
        InstallHandler.AllowUserInput = true;
        InstallHandler.OnPropertyChanged();

        // Loop over all dependencies and install them, give up on failures
        foreach (var dependency in dependenciesToInstall)
            try
            {
                /* Setup and start the installation */

                // Prepare the progress update handler
                var progress = new Progress<InstallationProgress>();
                progress.ProgressChanged += (_, installationProgress) =>
                    Shared.Main.DispatcherQueue.TryEnqueue(() =>
                    {
                        // Update our progress here
                        InstallHandler.StageName = installationProgress.StageTitle;
                        InstallHandler.ProgressIndeterminate = installationProgress.IsIndeterminate;
                        InstallHandler.ProgressValue = installationProgress.OverallProgress * 100 ?? 0;

                        // Trigger a partial interface reload
                        InstallHandler.OnPropertyChanged();
                    });

                // Capture the installation thread
                InstallHandler.InstallationWorker = dependency.Install(progress, InstallHandler.TokenSource.Token);

                // Actually start the installation now
                var result = await InstallHandler.InstallationWorker;

                /* Parse the result and present it */

                // Show the progress indicator [and the last message if failed]
                InstallHandler.ProgressError = !result;
                InstallHandler.ProgressIndeterminate = false;
                InstallHandler.ProgressValue = 100;
                InstallHandler.HideProgress = true;

                if (result)
                    InstallHandler.StageName =
                        Interfacing.LocalizedJsonString("/SharedStrings/Plugins/Dep/Contents/Success");

                InstallHandler.OnPropertyChanged();
                await Task.Delay(6000, InstallHandler.TokenSource.Token);

                if (!result)
                {
                    InstallHandler.StageName =
                        Interfacing.LocalizedJsonString("/SharedStrings/Plugins/Dep/Contents/Failure");

                    InstallHandler.OnPropertyChanged();
                    await Task.Delay(5000, InstallHandler.TokenSource.Token);
                }

                // Block temporarily
                InstallHandler.AllowUserInput = false;
                InstallHandler.OnPropertyChanged();
                await Task.Delay(500, InstallHandler.TokenSource.Token);

                // Hide the installer grid
                InstallHandler.InstallingDependencies = false;
                InstallHandler.OnPropertyChanged();
                await Task.Delay(500, InstallHandler.TokenSource.Token);

                // Unblock user input now
                InstallHandler.AllowUserInput = true;
                InstallHandler.OnPropertyChanged();

                /* Show the restart notice */
                if (!result)
                {
                    InstallHandler.TokenSource.Dispose();
                    return; // Exit the whole handler
                }

                if (dependenciesToInstall.Last() == dependency)
                    Shared.Main.DispatcherQueue.TryEnqueue(() =>
                    {
                        Shared.TeachingTips.MainPage.ReloadInfoBar.IsOpen = true;
                        Shared.TeachingTips.MainPage.ReloadInfoBar.Opacity = 1.0;
                    });
            }
            catch (OperationCanceledException e)
            {
                // Show the fail information
                InstallHandler.StageName = Interfacing.LocalizedJsonString(
                    "/SharedStrings/Plugins/Dep/Contents/Cancelled").Format(e.Message);

                InstallHandler.ProgressError = true;
                InstallHandler.HideProgress = true;
                InstallHandler.ProgressValue = 100;

                InstallHandler.OnPropertyChanged();
                await Task.Delay(5000);

                // Hide the installer grid and 'install'
                InstallHandler.InstallingDependencies = false;
                InstallHandler.OnPropertyChanged();
                await Task.Delay(500);

                // Unblock user input now
                InstallHandler.AllowUserInput = true;
                InstallHandler.OnPropertyChanged();
            }
            catch (Exception ex)
            {
                // Show the fail information
                InstallHandler.StageName = Interfacing.LocalizedJsonString(
                    "/SharedStrings/Plugins/Dep/Contents/InternalException").Format(ex.Message);

                InstallHandler.ProgressError = true;
                InstallHandler.HideProgress = true;
                InstallHandler.ProgressValue = 100;

                InstallHandler.OnPropertyChanged();
                await Task.Delay(5000);

                // Hide the installer grid and 'install'
                InstallHandler.InstallingDependencies = false;
                InstallHandler.OnPropertyChanged();
                await Task.Delay(500);

                // Unblock user input now
                InstallHandler.AllowUserInput = true;
                InstallHandler.OnPropertyChanged();
            }

        // Clean up after installation
        InstallHandler.TokenSource.Dispose();
    }

    public string TrimString(string s, int l)
    {
        return s?[..Math.Min(s.Length, l)] +
               (s?.Length > l ? "..." : "");
    }

    public static SetupPlugin CreateFrom(string pluginAssembly)
    {
        try
        {
            // Prepare assembly resources
            var coreAssemblies =
                Directory.GetFiles(RuntimeEnvironment.GetRuntimeDirectory(), "*.dll").ToList();
            coreAssemblies.Add(Path.Join(Interfacing.ProgramLocation.DirectoryName, "Amethyst.Plugins.Contract.dll"));

            // Load the failed assembly for metadata retrieval
            var metadataContext = new MetadataLoadContext(new PathAssemblyResolver(coreAssemblies))
                .LoadFromAssemblyPath(pluginAssembly);

            // Prepare a null context for instantiation
            ICoreSetupData coreSetupDataContext = null;
            IDependencyInstaller installerContext = null;

            // Find the plugin export, if exists
            var placeholderGuid = System.Guid.NewGuid().ToString().ToUpper();
            var result = metadataContext.ExportedTypes.FirstOrDefault(x => x.CustomAttributes
                .Any(export => export.ConstructorArguments.FirstOrDefault().Value?.ToString() is "Guid"));

            // Check whether the plugin defines a setup data element
            if (result?.GetMetadata<Type>("CoreSetupData") is not null)
                try
                {
                    var contextResult = new AssemblyLoadContext(placeholderGuid)
                        .LoadFromAssemblyPath(pluginAssembly)
                        .GetType(result.GetMetadata<Type>(
                            "CoreSetupData")?.FullName ?? string.Empty, true);

                    // Instantiate the installer and capture it for the outer scope
                    coreSetupDataContext = contextResult.Instantiate<ICoreSetupData>();
                }
                catch (Exception ex)
                {
                    Logger.Error(ex);
                }
            else throw new MissingFieldException("Setup plugins must have CoreSetupData defined.");

            // Check whether the plugin defines a dependency installer
            if (result.GetMetadata<Type>("DependencyInstaller") is not null)
                try
                {
                    var contextResult = new AssemblyLoadContext(placeholderGuid)
                        .LoadFromAssemblyPath(pluginAssembly)
                        .GetType(result.GetMetadata<Type>(
                            "DependencyInstaller")?.FullName ?? string.Empty, true);

                    // Instantiate the installer and capture it for the outer scope
                    installerContext = contextResult.Instantiate<IDependencyInstaller>();
                }
                catch (Exception ex)
                {
                    Logger.Error(ex);
                }

            // Create the plugin object
            var plugin = new SetupPlugin
            {
                Name = result.GetMetadata("Name", Path.GetFileName(pluginAssembly)),
                Guid = $"{result.GetMetadata("Guid", $"{placeholderGuid}")}:SETUP",

                DependencyLink = result.GetMetadata("DependencyLink", string.Empty)
                    ?.Format(Interfacing.DocsLanguageCode),
                DependencySource = result.GetMetadata("DependencySource", string.Empty),

                DependencyInstaller = installerContext,
                CoreSetupData = coreSetupDataContext
            };

            // Add the plugin to the 'attempted' list
            AppPlugins.InstallerPluginsList.Add(plugin);

            // Check whether the plugin defines a dependency installer
            // ReSharper disable once InvertIf | Metadata already checked
            if (installerContext is not null)
                try
                {
                    // Set the device's string resources root to its provided folder
                    // (If it wants to change it, it's gonna need to do that after OnLoad anyway)
                    Logger.Info($"Registering (" +
                                $"{result.GetMetadata("Name", $"{pluginAssembly}")}, " +
                                $"{result.GetMetadata("Guid", $"{pluginAssembly}")}:SETUP) " +
                                "default root language resource context (AppPlugins)...");

                    Interfacing.Plugins.SetLocalizationResourcesRoot(
                        Path.Join(Directory.GetParent(pluginAssembly)!.FullName, "Assets", "Strings"),
                        $"{result.GetMetadata("Guid", $"{placeholderGuid}")}:SETUP");

                    Logger.Info($"Overwriting (" +
                                $"{result.GetMetadata("Name", $"{pluginAssembly}")}, " +
                                $"{result.GetMetadata("Guid", $"{pluginAssembly}")}:SETUP) " +
                                "'s localization host (IAmethystHost)...");

                    // Allow the installer to use Amethyst APIs
                    installerContext.Host = new CoreHost(
                        $"{result.GetMetadata("Guid", $"{placeholderGuid}")}:SETUP");
                }
                catch (Exception ex)
                {
                    Logger.Error(ex);
                }

            return plugin;
        }
        catch (Exception ex)
        {
            Logger.Error(ex);
            return null;
        }
    }
}