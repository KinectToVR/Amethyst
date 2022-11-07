using System;
using System.Collections.Generic;
using System.IO;

namespace K2CrashHandler.Helpers;

internal class OpenVrPaths
{
    private static readonly string Path =
        Environment.ExpandEnvironmentVariables(System.IO.Path.Combine("%LocalAppData%", "openvr", "openvrpaths.vrpath"));

    public static OpenVrPaths Read()
    {
        var temp = JsonFile.Read<OpenVrPaths>(Path);

        if (temp.ExternalDrivers == null) temp.ExternalDrivers = new List<string>();

        return temp;
    }

    public void Write()
    {
        JsonFile.Write(Path, this, 1, '\t');
    }

    // Prevent Warning CS0649: Field '...' is never assigned to, and will always have its default value null:
#pragma warning disable 0649
    public List<string> Config;
    public List<string> ExternalDrivers = new();
    public string Jsonid;
    public List<string> Log;
    public List<string> Runtime;
    public int Version;
#pragma warning restore 0649
}