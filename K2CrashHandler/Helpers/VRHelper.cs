using System;
using System.Collections.Generic;
using System.IO;
using System.Xml.Serialization;
using Microsoft.UI.Xaml;
using Microsoft.Win32;

namespace K2CrashHandler.Helpers
{
    public class VRHelper
    {
        public string SteamPath = "";
        public string SteamVrPath = "";
        public string SteamVrSettingsPath = "";
        public string VRPathReg = "";
        public string CopiedDriverPath = "";

        public string GetFullInstallationPath()
        {
            return "C:/K2EX"; // TODO Registry maybe?
        }

        public void UpdateSteamPaths()
        {
            SteamPath = "";
            SteamPath = Registry.GetValue(@"HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Valve\Steam", "InstallPath", null)
                ?.ToString();

            if (String.IsNullOrEmpty(SteamPath))
            {
                // steam not found TODO
            }

            SteamVrPath = "";
            VRPathReg = "";
            try
            {
                var openVrPaths = OpenVRPaths.Read();
                foreach (string runtimePath in openVrPaths.runtime)
                {
                    string tempVrPathReg = Path.Combine(runtimePath, "bin", "win64", "vrpathreg.exe");
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
}