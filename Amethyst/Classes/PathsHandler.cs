using System;
using System.IO;
using System.Reflection;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.Storage;
using Amethyst.Plugins.Contract;

namespace Amethyst.Classes;

public static class PathsHandler
{
    public static async Task Setup()
    {
        IsAmethystPackaged = GetIsAmethystPackaged;
        if (IsAmethystPackaged) return;

        var root = await StorageFolder.GetFolderFromPathAsync(
            Path.Join(ProgramLocation.DirectoryName!));

        TemporaryFolderUnpackaged = await (await root
                .CreateFolderAsync("AppData", CreationCollisionOption.OpenIfExists))
            .CreateFolderAsync("TempState", CreationCollisionOption.OpenIfExists);

        LocalFolderUnpackaged = await (await root
                .CreateFolderAsync("AppData", CreationCollisionOption.OpenIfExists))
            .CreateFolderAsync("LocalState", CreationCollisionOption.OpenIfExists);

        await LocalSettings.ReadSettingsAsync(silent: true);
    }

    public static bool IsAmethystPackaged { get; set; }

    public static bool GetIsAmethystPackaged
    {
        get
        {
            try
            {
                return Package.Current is not null;
            }
            catch (Exception)
            {
                return false;
            }
        }
    }

    public static AppDataContainer LocalSettings => new();

    public static FileInfo ProgramLocation => new(Assembly.GetExecutingAssembly().Location);

    public static StorageFolder TemporaryFolder => IsAmethystPackaged ? ApplicationData.Current.TemporaryFolder : TemporaryFolderUnpackaged;

    public static StorageFolder LocalFolder => IsAmethystPackaged ? ApplicationData.Current.LocalFolder : LocalFolderUnpackaged;

    public static StorageFolder TemporaryFolderUnpackaged { get; set; } // Assigned on Setup()

    public static StorageFolder LocalFolderUnpackaged { get; set; } // Assigned on Setup()

    public static async Task<StorageFolder> GetPluginsFolder()
    {
        return await LocalFolder
            .CreateFolderAsync("Plugins", CreationCollisionOption.OpenIfExists);
    }

    public static async Task<StorageFolder> GetPluginsTempFolder()
    {
        return await LocalFolder
            .CreateFolderAsync("Plugins", CreationCollisionOption.OpenIfExists);
    }

    public static async Task<StorageFile> GetAppDataFile(string relativeFilePath)
    {
        return await LocalFolder.CreateFileAsync(relativeFilePath, CreationCollisionOption.OpenIfExists);
    }

    public static async Task<StorageFolder> GetAppDataPluginFolder(string relativeFilePath)
    {
        return string.IsNullOrEmpty(relativeFilePath)
            ? await GetPluginsFolder()
            : await (await GetPluginsFolder())
                .CreateFolderAsync(relativeFilePath, CreationCollisionOption.OpenIfExists);
    }

    public static async Task<StorageFolder> GetTempPluginFolder(string relativeFilePath)
    {
        return string.IsNullOrEmpty(relativeFilePath)
            ? await GetPluginsTempFolder()
            : await (await GetPluginsTempFolder())
                .CreateFolderAsync(relativeFilePath, CreationCollisionOption.OpenIfExists);
    }

    public static string GetAppDataFilePath(string relativeFilePath)
    {
        return Path.Join(LocalFolder.Path, relativeFilePath);
    }

    public static string GetAppDataLogFilePath(string relativeFilePath)
    {
        Directory.CreateDirectory(Path.Join(TemporaryFolder.Path, "Logs", "Amethyst"));
        return Path.Join(TemporaryFolder.Path, "Logs", "Amethyst", relativeFilePath);
    }
}

public class PluginsPathsHelper : IPathHelper
{
    // Main Amethyst.exe location
    public FileInfo ProgramLocation => PathsHandler.ProgramLocation;

    // AppData/TempState
    public DirectoryInfo TemporaryFolder => new(PathsHandler.TemporaryFolder.Path);

    // AppData/LocalState
    public DirectoryInfo LocalFolder => new(PathsHandler.LocalFolder.Path);

    // The location of ALL plugins folder
    public async Task<DirectoryInfo> GetPluginsFolder()
    {
        return new DirectoryInfo((await PathsHandler.GetPluginsFolder()).Path);
    }

    // The location of plugin working copies
    public async Task<DirectoryInfo> GetPluginsTempFolder()
    {
        return new DirectoryInfo((await PathsHandler.GetPluginsTempFolder()).Path);
    }

    // Get file from AppData/LocalState
    public async Task<FileInfo> GetAppDataFile(string relativeFilePath)
    {
        return new FileInfo((await PathsHandler.GetAppDataFile(relativeFilePath)).Path);
    }

    // Get folder from shared (not packed) plugin folder
    public async Task<DirectoryInfo> GetAppDataPluginFolder(string relativeFilePath)
    {
        return new DirectoryInfo((await PathsHandler.GetAppDataPluginFolder(relativeFilePath)).Path);
    }

    // Get folder from working copy plugin folder
    public async Task<DirectoryInfo> GetTempPluginFolder(string relativeFilePath)
    {
        return new DirectoryInfo((await PathsHandler.GetTempPluginFolder(relativeFilePath)).Path);
    }

    // Get file path from AppData/LocalState
    public FileInfo GetAppDataFilePath(string relativeFilePath)
    {
        return new FileInfo(PathsHandler.GetAppDataFilePath(relativeFilePath));
    }

    // Get log file path from AppData/TempState
    public FileInfo GetAppDataLogFilePath(string relativeFilePath)
    {
        return new FileInfo(PathsHandler.GetAppDataLogFilePath(relativeFilePath));
    }
}