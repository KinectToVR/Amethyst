using System;
using System.Collections.Generic;
using System.IO;

namespace K2CrashHandler.Helpers
{
    class OpenVRPaths
    {
        static string path = Environment.ExpandEnvironmentVariables(Path.Combine("%LocalAppData%", "openvr", "openvrpaths.vrpath"));

        // Prevent Warning CS0649: Field '...' is never assigned to, and will always have its default value null:
#pragma warning disable 0649
        public List<string> config;
        public List<string> external_drivers = new List<string>();
        public string jsonid;
        public List<string> log;
        public List<string> runtime;
        public int version;
#pragma warning restore 0649

        public static OpenVRPaths Read()
        {
            OpenVRPaths temp = JsonFile.Read<OpenVRPaths>(path);

            if (temp.external_drivers == null)
            {
                temp.external_drivers = new List<string>();
            }

            return temp;
        }

        public void Write()
        {
            JsonFile.Write(path, this, 1, '\t');
        }
    }
}