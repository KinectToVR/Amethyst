using System;
using System.IO;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Activation;
using Windows.Web.Http;
using K2CrashHandler.Helpers;
using Microsoft.UI.Xaml;
using Newtonsoft.Json.Linq;
using LaunchActivatedEventArgs = Microsoft.UI.Xaml.LaunchActivatedEventArgs;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace K2CrashHandler;

/// <summary>
///     Provides application-specific behavior to supplement the default Application class.
/// </summary>
public partial class App : Application
{
    private Window _mWindow;

    /// <summary>
    ///     Initializes the singleton application object.  This is the first line of authored code
    ///     executed, and as such is the logical equivalent of main() or WinMain().
    /// </summary>
    public App()
    {
        InitializeComponent();

        try
        {
            var amethystConfigText = File.ReadAllText(Path.Combine(Environment.GetFolderPath(
                Environment.SpecialFolder.ApplicationData), "Amethyst", "AmethystSettings.json"));

            Shared.LanguageCode = amethystConfigText.Contains("\"AppLanguage\": \"")
                ? amethystConfigText.Substring(
                    amethystConfigText.IndexOf("\"AppLanguage\": \"", StringComparison.Ordinal) +
                    "\"AppLanguage\": \"".Length, 2)
                : "en";

            if (!int.TryParse(amethystConfigText.AsSpan(
                        amethystConfigText.IndexOf("\"AppTheme\": ", StringComparison.Ordinal) +
                        "\"AppTheme\": ".Length, 1),
                    out var themeConfig)) return;

            Shared.DocsLanguageCode = Shared.LanguageCode;
            Current.RequestedTheme = themeConfig switch
            {
                2 => ApplicationTheme.Light,
                1 => ApplicationTheme.Dark,
                _ => Current.RequestedTheme
            };

            // Do this in the meantime
            Task.Factory.StartNew(async () =>
            {
                try
                {
                    var client = new HttpClient();

                    using var response = await client.GetAsync(new Uri("https://docs.k2vr.tech/shared/locales.json"));
                    using var content = response.Content;
                    var json = await content.ReadAsStringAsync();

                    // Optionally fall back to English
                    if (JObject.Parse(json)[Shared.DocsLanguageCode] == null)
                        Shared.DocsLanguageCode = "en";
                }
                catch (Exception)
                {
                    Shared.DocsLanguageCode = "en";
                }
            });

            // Update in the meantime
            Task.Factory.StartNew(UpdateHelper.CheckForUpdates);
        }
        catch (Exception)
        {
            // Ignored
        }
    }

    /// <summary>
    ///     Invoked when the application is launched normally by the end user.  Other entry points
    ///     will be used such as when the application is launched to open a specific file.
    /// </summary>
    /// <param name="args">Details about the launch request and process.</param>
    protected override void OnLaunched(LaunchActivatedEventArgs args)
    {
        _mWindow = new MainWindow();
        _mWindow.Activate();
    }
}