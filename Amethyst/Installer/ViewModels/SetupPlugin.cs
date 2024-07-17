using System;
using System.Collections.Generic;
using System.ComponentModel;
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
using Microsoft.UI.Xaml.Controls;

namespace Amethyst.Installer.ViewModels;

public class SetupPluginGroup
{
    public string Name { get; set; }
    public bool NameValid => !string.IsNullOrEmpty(Name);
    public bool NameInvalid => string.IsNullOrEmpty(Name);

    public ListViewSelectionMode GroupSelectionMode => NameValid
        ? ListViewSelectionMode.Single // One in a group
        : ListViewSelectionMode.Multiple; // Many

    public List<SetupPlugin> Plugins { get; set; }
}

public class SetupPlugin : INotifyPropertyChanged
{
    public Type PluginType => CoreSetupData.PluginType;
    public string Name { get; set; } = "[UNKNOWN]";
    public string Guid { get; set; } = "[INVALID]";
    public string Publisher { get; set; }
    public string Website { get; set; }

    public string DependencyLink { get; set; }
    public string DependencySource { get; set; }
    public bool DependencySetupPending { get; set; }

    public IDependencyInstaller DependencyInstaller { get; set; }
    public ICoreSetupData CoreSetupData { get; set; }
    public UIElement Icon => CoreSetupData?.PluginIcon as UIElement;

    public LoadAttemptedPlugin.DependencyInstallHandler InstallHandler { get; } = new();

    public (LocalisationFileJson Root, string Directory) LocalizationResourcesRoot { get; set; }

    public Uri DependencyLinkUri => Uri.TryCreate(DependencyLink, UriKind.RelativeOrAbsolute, out var uri) ? uri : null;

    public Uri DependencySourceUri =>
        Uri.TryCreate(DependencySource, UriKind.RelativeOrAbsolute, out var uri) ? uri : null;

    public Uri WebsiteUri => Uri.TryCreate(Website, UriKind.RelativeOrAbsolute, out var uri) ? uri : null;

    public bool PublisherValid => !string.IsNullOrEmpty(Publisher);
    public bool WebsiteValid => !string.IsNullOrEmpty(Website);
    public bool GuidValid => !string.IsNullOrEmpty(Guid) && Guid is not "[INVALID]" and not "INVALID";
    public bool IsEnabled => true;

    public bool IsLimited => SetupData.LimitedSetup && (DependencyInstaller?
        .ListDependencies().Any(x => !x.IsInstalled && x.IsMandatory) ?? false);

    public bool IsLimitedDisplay => IsLimited && !SetupData.LimitedHide;

    public bool DependencyLinkValid => !string.IsNullOrEmpty(DependencyLink);
    public bool DependencySourceValid => !string.IsNullOrEmpty(DependencySource);
    public bool DependencyLinksValid => DependencyLinkValid && DependencySourceValid;

    public bool ShowDependencyInstaller => DependencyInstaller is not null;

    public bool ShowDependencyLinks =>
        !ShowDependencyInstaller && (DependencyLinkValid || DependencySourceValid);

    public event PropertyChangedEventHandler PropertyChanged;

    public async Task PerformDependencyInstallation(IDependency dependency)
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

        // Theoretically not possible, but check anyway
        if (dependency is null) return;
        InstallHandler.TokenSource = new CancellationTokenSource();
        InstallHandler.DependencyName = dependency.Name;

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
        InstallHandler.NoProgress = false;
        InstallHandler.OnPropertyChanged();

        // Unblock user input now
        await Task.Delay(500, InstallHandler.TokenSource.Token);
        InstallHandler.AllowUserInput = true;
        InstallHandler.OnPropertyChanged();

        // Loop over all dependencies and install them, give up on failures
        try
        {
            /* Setup and start the installation */

            // Prepare the progress update handler
            var progress = new Progress<InstallationProgress>();
            InstallHandler.DependencyName = dependency.Name;
            InstallHandler.OnPropertyChanged(); // The name

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
            InstallHandler.NoProgress = true;

            if (result)
                InstallHandler.StageName =
                    Interfacing.LocalizedJsonString("/Installer/Dep/Contents/Success")
                        .Format(InstallHandler.DependencyName);

            InstallHandler.OnPropertyChanged();
            await Task.Delay(6000, InstallHandler.TokenSource.Token);

            if (!result)
            {
                InstallHandler.StageName =
                    Interfacing.LocalizedJsonString("/SharedStrings/Plugins/Dep/Contents/Failure")
                        .Format(InstallHandler.DependencyName);

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

            InstallHandler.TokenSource.Dispose();
            return; // Exit the whole handler
        }
        catch (OperationCanceledException e)
        {
            // Show the fail information
            InstallHandler.StageName = Interfacing.LocalizedJsonString(
                "/SharedStrings/Plugins/Dep/Contents/Cancelled").Format(e.Message);

            InstallHandler.ProgressError = true;
            InstallHandler.HideProgress = true;
            InstallHandler.NoProgress = true;
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
            InstallHandler.NoProgress = true;
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

	public async Task PerformFixApplication(IFix fix, object param = null)
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

		// Theoretically not possible, but check anyway
		if (fix is null) return;
		InstallHandler.TokenSource = new CancellationTokenSource();
		InstallHandler.DependencyName = fix.Name;

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
		InstallHandler.NoProgress = false;
		InstallHandler.OnPropertyChanged();

		// Unblock user input now
		await Task.Delay(500, InstallHandler.TokenSource.Token);
		InstallHandler.AllowUserInput = true;
		InstallHandler.OnPropertyChanged();

		// Loop over all dependencies and install them, give up on failures
		try
		{
			/* Setup and start the installation */

			// Prepare the progress update handler
			var progress = new Progress<InstallationProgress>();
			InstallHandler.DependencyName = fix.Name;
			InstallHandler.OnPropertyChanged(); // The name

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
			InstallHandler.InstallationWorker = fix.Apply(progress, InstallHandler.TokenSource.Token, param);

			// Actually start the installation now
			var result = await InstallHandler.InstallationWorker;

			/* Parse the result and present it */

			// Show the progress indicator [and the last message if failed]
			InstallHandler.ProgressError = !result;
			InstallHandler.ProgressIndeterminate = false;
			InstallHandler.ProgressValue = 100;
			InstallHandler.HideProgress = true;
			InstallHandler.NoProgress = true;

			if (result)
				InstallHandler.StageName =
					Interfacing.LocalizedJsonString("/Installer/Fix/Contents/Success")
						.Format(InstallHandler.DependencyName);

			InstallHandler.OnPropertyChanged();
			await Task.Delay(6000, InstallHandler.TokenSource.Token);

			if (!result)
			{
				InstallHandler.StageName =
					Interfacing.LocalizedJsonString("/SharedStrings/Plugins/Fix/Contents/Failure")
						.Format(InstallHandler.DependencyName);

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

			InstallHandler.TokenSource.Dispose();
			return; // Exit the whole handler
		}
		catch (OperationCanceledException e)
		{
			// Show the fail information
			InstallHandler.StageName = Interfacing.LocalizedJsonString(
				"/SharedStrings/Plugins/Dep/Contents/Cancelled").Format(e.Message);

			InstallHandler.ProgressError = true;
			InstallHandler.HideProgress = true;
			InstallHandler.NoProgress = true;
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
				"/SharedStrings/Plugins/Fix/Contents/InternalException").Format(ex.Message);

			InstallHandler.ProgressError = true;
			InstallHandler.HideProgress = true;
			InstallHandler.NoProgress = true;
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

    public double BoolToOpacity(bool v)
    {
        return v ? 1.0 : 0.5;
    }

    public Visibility InvertVisibility(bool v)
    {
        return v ? Visibility.Collapsed : Visibility.Visible;
    }

    public void OnPropertyChanged(string propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}