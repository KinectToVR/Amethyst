using System;
using System.IO;
using Microsoft.Win32;

namespace K2CrashHandler.Helpers;

public class VRHelper
{
    public string CopiedDriverPath = "";
    public string SteamPath = "";
    public string SteamVrPath = "";
    public string SteamVrSettingsPath = "";
    public string VRPathReg = "";

    public string GetFullInstallationPath()
    {
        return "C:/K2EX"; // TODO Registry maybe?
    }

    public void UpdateSteamPaths()
    {
        SteamPath = "";
        SteamPath = Registry.GetValue(@"HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Valve\Steam", "InstallPath", null)
            ?.ToString();

        if (string.IsNullOrEmpty(SteamPath))
        {
            // steam not found TODO
        }

        SteamVrPath = "";
        VRPathReg = "";
        try
        {
            var openVrPaths = OpenVRPaths.Read();
            foreach (var runtimePath in openVrPaths.runtime)
            {
                var tempVrPathReg = Path.Combine(runtimePath, "bin", "win64", "vrpathreg.exe");
                if (File.Exists(tempVrPathReg))
                {
                    SteamVrPath = runtimePath;
                    VRPathReg = tempVrPathReg;
                    break;
                }
            }
        }
        catch (Exception)
        {
        }

        if (VRPathReg == "")
        {
            // VRPathReg not found TODO
        }

        SteamVrSettingsPath = Path.Combine(SteamPath, "config", "steamvr.vrsettings");
        CopiedDriverPath = Path.Combine(SteamVrPath, "drivers", "Amethyst");

        if (!File.Exists(SteamVrSettingsPath))
        {
            // vrsettings not found TODO
        }
    }
}