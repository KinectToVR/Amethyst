using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading;
using Microsoft.Win32;

namespace K2CrashHandler.Helpers;

public class VRHelper
{
    public string CopiedDriverPath = "";
    public string SteamPath = "";
    public string SteamVRPath = "";
    public string SteamVRSettingsPath = "";
    public string VRPathReg = "";
    
    // Returns: <Exists>, <Path> of SteamVR, VRSettings, CopiedDriver
    public
        Tuple<Tuple<bool, bool, bool>,
            Tuple<string, string, string>
        > UpdateSteamPaths()
    {
        SteamPath = Registry.GetValue(@"HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Valve\Steam",
            "InstallPath", null)?.ToString();

        SteamVRPath = "";
        VRPathReg = "";
        try
        {
            var openVrPaths = OpenVRPaths.Read();
            foreach (var runtimePath in openVrPaths.runtime)
            {
                var tempVrPathReg = Path.Combine(runtimePath, "bin", "win64", "vrpathreg.exe");
                if (File.Exists(tempVrPathReg))
                {
                    SteamVRPath = runtimePath;
                    VRPathReg = tempVrPathReg;
                    break;
                }
            }
        }
        catch (Exception)
        {
        }

        SteamVRSettingsPath = Path.Combine(SteamPath, "config", "steamvr.vrsettings");
        CopiedDriverPath = Path.Combine(SteamVRPath, "drivers", "Amethyst");

        // Return the found-outs
        return new
            Tuple<Tuple<bool, bool, bool>,
                Tuple<string, string, string>>
            (
                new Tuple<bool, bool, bool>
                (
                    !string.IsNullOrEmpty(SteamPath),
                    File.Exists(SteamVRSettingsPath),
                    Directory.Exists(CopiedDriverPath)
                ),
                new Tuple<string, string, string>
                (
                    SteamVRPath,
                    SteamVRSettingsPath,
                    CopiedDriverPath
                )
            );
    }

    public bool CloseSteamVR()
    {
        // Check if SteamVR is running
        if (Process.GetProcesses()
                .FirstOrDefault(proc => proc.ProcessName == "vrserver" || proc.ProcessName == "vrmonitor") == null)
            return true;

        // Close VrMonitor
        foreach (var process in Process.GetProcesses().Where(proc => proc.ProcessName == "vrmonitor"))
        {
            process.CloseMainWindow();
            Thread.Sleep(5000);
            if (!process.HasExited)
            {
                /* When SteamVR is open with no headset detected,
                    CloseMainWindow will only close the "headset not found" popup
                    so we kill it, if it's still open */
                process.Kill();
                Thread.Sleep(3000);
            }
        }

        // Close VrServer
        /* Apparently, SteamVR server can run without the monitor,
           so we close that, if it's open as well (monitor will complain if you close server first) */
        foreach (var process in Process.GetProcesses().Where(proc => proc.ProcessName == "vrserver"))
        {
            // CloseMainWindow won't work here because it doesn't have a window
            process.Kill();
            Thread.Sleep(5000);
            if (!process.HasExited)
                return false;
        }

        return true;
    }
}