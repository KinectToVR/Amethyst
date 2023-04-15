using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Linq;
using System.Threading.Tasks;
using Amethyst.Classes;
using Amethyst.Utils;

namespace Amethyst.Schedulers;

public class ShutdownTask
{
    public string Name { get; set; }
    public string Data { get; set; }
    public bool Priority { get; set; } = false;
    public Func<Task<bool>> Action { get; init; }
}

public static class ShutdownController
{
    // Vector of actions Amethyst should do upon a graceful shutdown
    // This will not be run when quitting due to runtime failures
    public static List<ShutdownTask> ShutdownTasks { get; } = new();

    // Execute all shutdown tasks with proper logging
    public static async Task ExecuteAllTasks()
    {
        // First run all queued tasks waiting for us
        Logger.Info("Running all actions queued for shutdown...");

        Logger.Info("Preparing an immutable task list...");
        var queuedShutdownTasks = ShutdownTasks.ToImmutableList();

        Logger.Info("Executing tasks marked as priority...");
        foreach (var task in queuedShutdownTasks.Where(x => x.Priority))
        {
            Logger.Info($"Starting task with name \"{task.Name}\"...");
            try
            {
                // Try running the queued task
                AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
                Logger.Info($"Task result: {await task.Action()}");
            }
            catch (Exception e)
            {
                // Don't care further
                Logger.Warn(e);
            }
        }

        Logger.Info("Executing all other tasks now...");
        foreach (var task in queuedShutdownTasks.Where(x => !x.Priority))
        {
            Logger.Info($"Starting task with name \"{task.Name}\"...");
            try
            {
                // Try running the queued task, no sounds here though
                Logger.Info($"Task result: {await task.Action()}");
            }
            catch (Exception e)
            {
                // Don't care further
                Logger.Warn(e);
            }
        }

        // Finally proceed to the typical shutdown scenario
        Logger.Info("Executing a standard shutdown...");

        // Handle the exit actions
        await Interfacing.HandleAppExit(1000);
    }
}