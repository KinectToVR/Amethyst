using System;
using System.Runtime.InteropServices;
using Microsoft.ApplicationInsights;
using Microsoft.ApplicationInsights.Extensibility;
using Microsoft.Win32;

// Everything should be noexcept(true)
namespace K2InsightsHandler
{
    [ComVisible(true)]
    [Guid("2a4a21d3-8701-45f0-bb24-e1b9bd4d76ed")]
    [ClassInterface(ClassInterfaceType.None)]
    public class InsightsHandler
    {
        private readonly TelemetryConfiguration configuration = TelemetryConfiguration.CreateDefault();
        private TelemetryConfiguration _ = TelemetryConfiguration.Active;

        private TelemetryClient tc = new TelemetryClient(); // Hollow

        public void Initialize()
        {
            try
            {
                configuration.InstrumentationKey = "b6e7d4dc-c14b-4c3d-9342-31db3d3350fb";
                tc = new TelemetryClient(configuration);

                tc.Context.Component.Version = "1.0.1.9"; // Amethyst version
                tc.Context.User.Id = GetMachineGuid(); // User id (random)
                tc.Context.Device.OperatingSystem = Environment.OSVersion.ToString();
            }
            catch (Exception)
            {
            }
        }

        public void LogEvent(string name)
        {
            try
            {
                tc.TrackEvent(name);
                tc.Flush();
            }
            catch (Exception)
            {
            }
        }

        public void LogTrace(string message)
        {
            try
            {
                tc.TrackTrace(message);
                tc.Flush();
            }
            catch (Exception)
            {
            }
        }

        public void LogMetric(string name, double value)
        {
            try
            {
                tc.TrackMetric(name, value);
                tc.Flush();
            }
            catch (Exception)
            {
            }
        }

        public void LogPageView(string name)
        {
            try
            {
                tc.TrackPageView(name);
                tc.Flush();
            }
            catch (Exception)
            {
            }
        }

        public void LogException(Exception exception)
        {
            try
            {
                tc.TrackException(exception);
                tc.Flush();
            }
            catch (Exception)
            {
            }
        }

        private string GetMachineGuid()
        {
            using (var rk = RegistryKey.OpenBaseKey(
                           RegistryHive.LocalMachine, RegistryView.Registry64)
                       .OpenSubKey(@"SOFTWARE\Microsoft\Cryptography"))
            {
                if (rk == null)
                    return Guid.NewGuid().ToString();

                return rk.GetValue("MachineGuid") == null
                    ? Guid.NewGuid().ToString()
                    : rk.GetValue("MachineGuid").ToString();
            }
        }
    }
}