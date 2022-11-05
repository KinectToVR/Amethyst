using System;
using System.Collections.Generic;
using System.IO;

namespace K2CrashHandler.Helpers;

internal class OpenVRPaths
{
    private static readonly string path =
        Environment.ExpandEnvironmentVariables(Path.Combine("%LocalAppData%", "openvr", "openvrpaths.vrpath"));

    public static OpenVRPaths Read()
    {
        var temp = JsonFile.Read<OpenVRPaths>(path);

        if (temp.external_drivers == null) temp.external_drivers = new List<string>();

        return temp;
    }

    public void Write()
    {
        JsonFile.Write(path, this, 1, '\t');
    }

    // Prevent Warning CS0649: Field '...' is never assigned to, and will always have its default value null:
#pragma warning disable 0649
    public List<string> config;
    public List<string> external_drivers = new();
    public string jsonid;
    public List<string> log;
    public List<string> runtime;
    public int version;
#pragma warning restore 0649
}