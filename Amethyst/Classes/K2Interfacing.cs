using System;
using System.IO;
using System.Reflection;
using AmethystPluginContract;

namespace Amethyst.Classes;

public static class Interfacing
{
    public static FileInfo GetProgramLocation()
    {
        return new FileInfo(Assembly.GetExecutingAssembly().Location);
    }

    public static DirectoryInfo GetK2AppDataTempDir()
    {
        return Directory.CreateDirectory(Path.GetTempPath() + "Amethyst");
    }

    public static string GetK2AppDataFileDir(string relativeFilePath)
    {
        Directory.CreateDirectory(Path.Join(
            Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "Amethyst"));

        return Path.Join(Environment.GetFolderPath(
            Environment.SpecialFolder.ApplicationData), "Amethyst", relativeFilePath);
    }

    public static string GetK2AppDataLogFileDir(string relativeFolderName, string relativeFilePath)
    {
        Directory.CreateDirectory(Path.Join(
            Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
            "Amethyst", "logs", relativeFolderName));

        return Path.Join(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
            "Amethyst", "logs", relativeFolderName, relativeFilePath);
    }

    // Application settings
    public static K2AppSettings AppSettings = new K2AppSettings();


}