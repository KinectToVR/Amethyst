using System;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Microsoft.AppCenter.Crashes;

namespace Amethyst.Utils;

/// <summary>
///     A logging class which mimics GLOG's trace style. While not highly configurable, this is
///     designed to be a simple drag and drop logger which relies solely on built-in C# APIs
/// </summary>
/// @https://github.com/KinectToVR/Amethyst-Installer @Hekky
public static partial class Logger
{
    public static string LogFilePath { get; set; }

    // This is used instead of Thread.CurrentThread.ManagedThreadId since it returns the OS thread rather than the managed thread
    // Consider using ManagedThreadId instead of this if you have to run this on non-Windows platforms
    // https://stackoverflow.com/a/1679270
    [LibraryImport("Kernel32", EntryPoint = "GetCurrentThreadId")]
    private static partial int GetCurrentWin32ThreadId();

    #region Log Functions

    public static void Info(string text, [CallerLineNumber] int lineNumber = 0, [CallerFilePath] string filePath = "",
        [CallerMemberName] string memberName = "")
    {
        LogInternal(FormatToLogMessage(text, "I", lineNumber, filePath, memberName));
    }

    public static void Info(object obj, [CallerLineNumber] int lineNumber = 0, [CallerFilePath] string filePath = "",
        [CallerMemberName] string memberName = "")
    {
        LogInternal(FormatToLogMessage(obj.ToString(), "I", lineNumber, filePath, memberName));
    }

    public static void Warn(string text, [CallerLineNumber] int lineNumber = 0, [CallerFilePath] string filePath = "",
        [CallerMemberName] string memberName = "")
    {
        LogInternal(FormatToLogMessage(text, "W", lineNumber, filePath, memberName), ConsoleColor.Yellow);
    }

    public static void Warn(object obj, [CallerLineNumber] int lineNumber = 0, [CallerFilePath] string filePath = "",
        [CallerMemberName] string memberName = "")
    {
        LogInternal(FormatToLogMessage(obj.ToString(), "W", lineNumber, filePath, memberName), ConsoleColor.Yellow);
    }

    public static void Error(string text, [CallerLineNumber] int lineNumber = 0, [CallerFilePath] string filePath = "",
        [CallerMemberName] string memberName = "")
    {
        LogInternal(FormatToLogMessage(text, "E", lineNumber, filePath, memberName), ConsoleColor.Red);
    }

    public static void Error(Exception exception, [CallerLineNumber] int lineNumber = 0,
        [CallerFilePath] string filePath = "", [CallerMemberName] string memberName = "")
    {
        LogInternal(FormatToLogMessage(
            $"Exception occurred! {exception.GetType().Name} " +
            $"in {exception.Source}: {exception.Message}\n{exception.StackTrace}",
            "E", lineNumber, filePath, memberName), ConsoleColor.Red);

        Crashes.TrackError(exception);
    }

    public static void Error(object obj, [CallerLineNumber] int lineNumber = 0, [CallerFilePath] string filePath = "",
        [CallerMemberName] string memberName = "")
    {
        LogInternal(FormatToLogMessage(obj.ToString(), "E", lineNumber, filePath, memberName), ConsoleColor.Red);
    }

    public static void Fatal(string text, [CallerLineNumber] int lineNumber = 0, [CallerFilePath] string filePath = "",
        [CallerMemberName] string memberName = "")
    {
        LogInternal(FormatToLogMessage(text, "F", lineNumber, filePath, memberName), ConsoleColor.DarkRed);
    }

    public static void Fatal(Exception exception, [CallerLineNumber] int lineNumber = 0,
        [CallerFilePath] string filePath = "", [CallerMemberName] string memberName = "")
    {
        LogInternal(FormatToLogMessage(
            $"Exception occurred! {exception.GetType().Name} " +
            $"in {exception.Source}: {exception.Message}\n{exception.StackTrace}",
            "F", lineNumber, filePath, memberName), ConsoleColor.DarkRed);

        Crashes.TrackError(exception);
    }

    public static void Fatal(object obj, [CallerLineNumber] int lineNumber = 0, [CallerFilePath] string filePath = "",
        [CallerMemberName] string memberName = "")
    {
        LogInternal(FormatToLogMessage(obj.ToString(), "F", lineNumber, filePath, memberName), ConsoleColor.DarkRed);
    }

    internal static void PrivateDoNotUseLogExecption(string message, string displayString,
        [CallerLineNumber] int lineNumber = 0, [CallerFilePath] string filePath = "",
        [CallerMemberName] string memberName = "")
    {
        LogInternalUniqueMessage(FormatToLogMessage(message, "F", lineNumber, filePath, memberName),
            FormatToLogMessage(displayString, "F", lineNumber, filePath, memberName), ConsoleColor.DarkRed);
    }

    #endregion

    #region Logger Internals

    /// <summary>
    ///     Initializes the logger
    /// </summary>
    public static void Init(string filePath = "")
    {
        LogFilePath = filePath == ""
            ? $"{Assembly.GetCallingAssembly().GetName()}" +
              $"_{DateTime.Now:yyyyMMdd-HHmmss.ffffff}.log"
            : filePath;

        LogFilePath = Path.GetFullPath(filePath);
        var dir = Path.GetFullPath(Path.GetDirectoryName(LogFilePath) ?? string.Empty);

        var loggingPathDidntExist = false;
        if (!Directory.Exists(dir))
        {
            Directory.CreateDirectory(dir);
            loggingPathDidntExist = true;
        }

        // Create the file, rest of the methods will append
        File.Create(LogFilePath).Close();

        if (loggingPathDidntExist)
            LogInternal($"Created logging directory at \"{dir}\"");

        LogInternal($"Log file created at: {DateTime.Now:yyyy/MM/dd HH:mm:ss}");
        LogInternal($"Running on machine: {Environment.MachineName}");
        LogInternal("Running duration (h:mm:ss): 0:00:00");
        LogInternal("Log line format: [IWEF]yyyyMMdd HH:mm:ss.ffffff threadid file::member:line] msg");
    }

    private static string FormatToLogMessage(string message, string level, int lineNumber, string filePath,
        string memberName)
    {
        return $"{level}{DateTime.Now:yyyyMMdd HH:mm:ss.ffffff} " +
               $"{GetCurrentWin32ThreadId(),5:#####} {Path.GetFileName(filePath)}::{memberName}:{lineNumber}] {message}";
    }

    private static void LogInternal(string message)
    {
        Console.ResetColor();
        Console.WriteLine(message);

        if (LogFilePath is null)
            throw new InvalidOperationException("Tried logging something without calling Logger.Init()! Aborting...");

        try
        {
            File.AppendAllLines(LogFilePath, new[] { message });
        }
        catch (Exception)
        {
            // ignored
        }

#if DEBUG
        Debug.WriteLine(message);
#endif
    }

    private static void LogInternal(string message, ConsoleColor color)
    {
        Console.ForegroundColor = color;
        Console.WriteLine(message);
        if (LogFilePath is null)
            throw new InvalidOperationException("Tried logging something without calling Logger.Init()! Aborting...");

        try
        {
            File.AppendAllLines(LogFilePath, new[] { message });
        }
        catch (Exception)
        {
            // ignored
        }

#if DEBUG
        Debug.WriteLine(message);
#endif
    }

    private static void LogInternalUniqueMessage(string message, string messageUi, ConsoleColor color)
    {
        Console.ForegroundColor = color;
        Console.WriteLine(message);
        if (LogFilePath is null)
            throw new InvalidOperationException("Tried logging something without calling Logger.Init()! Aborting...");

        try
        {
            File.AppendAllLines(LogFilePath, new[] { message });
        }
        catch (Exception)
        {
            // ignored
        }

#if DEBUG
        Debug.WriteLine(message);
#endif
    }

    #endregion
}