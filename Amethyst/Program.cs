using System;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.UI.Dispatching;
using Microsoft.UI.Xaml;
using Microsoft.Windows.AppLifecycle;
using WinRT;

namespace Amethyst;

public class Program
{
    private static App _app;

    [STAThread]
    private static async Task<int> Main(string[] args)
    {
        ComWrappersSupport.InitializeComWrappers();
        var isRedirect = await DecideRedirection();
        if (!isRedirect)
            Application.Start(p =>
            {
                var context = new DispatcherQueueSynchronizationContext(
                    DispatcherQueue.GetForCurrentThread());
                SynchronizationContext.SetSynchronizationContext(context);
                _app = new App();
            });

        return 0;
    }

    private static async Task<bool> DecideRedirection()
    {
        var isRedirect = false;
        var args = AppInstance.GetCurrent().GetActivatedEventArgs();
        var keyInstance = AppInstance.FindOrRegisterForKey("K2VRTeam.Amethyst.App.Reg");

        if (keyInstance.IsCurrent)
        {
            keyInstance.Activated += OnActivated;
        }
        else
        {
            isRedirect = true;
            await keyInstance.RedirectActivationToAsync(args);
        }

        return isRedirect;
    }

    private static void OnActivated(object sender, AppActivationArguments args)
    {
        if (_app is null) throw new Exception("App must not be null!");
        // Check if args.Data is ProtocolActivatedEventArgs for uri invocation
    }
}