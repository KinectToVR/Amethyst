using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using Amethyst.Classes;
using Amethyst.Utils;
using Newtonsoft.Json;

namespace Amethyst.Schedulers;

public class StartupTask
{
    public string Name { get; set; }
    public string Data { get; set; }
    public bool Priority { get; set; }
}

public class StartupUpdateTask : StartupTask
{
    public string PluginFolder { get; set; }
    public string UpdatePackage { get; set; }
}

public class StartupDeleteTask : StartupTask
{
    public string PluginFolder { get; set; }
}

public static class StartupController
{
    public static TaskController Controller { get; set; } = new();

    public class TaskController
    {
        // Vector of actions Amethyst should do upon a graceful shutdown
        // This will not be run when quitting due to runtime failures
        public ObservableCollection<StartupTask> StartupTasks { get; private set; } = new();

        // For updating the plugins, i.e. clean and unpack a downloaded plugin zip
        public IEnumerable<StartupUpdateTask> UpdateTasks => StartupTasks.OfType<StartupUpdateTask>();

        // For deleting plugins, i.e. delete a plugin folder while it's not loaded yet
        public IEnumerable<StartupDeleteTask> DeleteTasks => StartupTasks.OfType<StartupDeleteTask>();

        // Save scheduled tasks
        private void SaveTasks()
        {
            try
            {
                // Save scheduled startup tasks
                File.WriteAllText(Path.Join(Interfacing.ProgramLocation.DirectoryName, "Startup.json"),
                    JsonConvert.SerializeObject(StartupTasks, Formatting.Indented,
                        new JsonSerializerSettings { TypeNameHandling = TypeNameHandling.All }));
            }
            catch (Exception e)
            {
                Logger.Error($"Error saving startup scheduled tasks! Message: {e.Message}");
            }
        }

        // Re/Load scheduled tasks
        public void ReadTasks()
        {
            try
            {
                // Read scheduled startup tasks
                StartupTasks = JsonConvert.DeserializeObject<ObservableCollection<StartupTask>>
                               (File.ReadAllText(Path.Join(Interfacing.ProgramLocation.DirectoryName, "Startup.json")),
                                   new JsonSerializerSettings { TypeNameHandling = TypeNameHandling.All }) ??
                               new ObservableCollection<StartupTask>();
            }
            catch (Exception e)
            {
                Logger.Error($"Error reading scheduled startup tasks! Message: {e.Message}");
                StartupTasks = new ObservableCollection<StartupTask>(); // Reset if null
            }

            StartupTasks.CollectionChanged += (_, _) =>
            {
                // Save our changes
                SaveTasks();
            };
        }
    }
}