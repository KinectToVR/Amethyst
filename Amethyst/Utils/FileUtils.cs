using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using System.Security.Principal;
using System.Text;
using Microsoft.Win32;

namespace Amethyst.Utils;

internal static class FileUtils
{
    private const int RmRebootReasonNone = 0;
    private const int CCH_RM_MAX_APP_NAME = 255;
    private const int CCH_RM_MAX_SVC_NAME = 63;

    private const uint TOKEN_QUERY = 0x0008;

    [DllImport("rstrtmgr.dll", CharSet = CharSet.Unicode)]
    private static extern int RmRegisterResources(uint pSessionHandle,
        uint nFiles,
        string[] rgsFilenames,
        uint nApplications,
        [In] RM_UNIQUE_PROCESS[] rgApplications,
        uint nServices,
        string[] rgsServiceNames);

    [DllImport("rstrtmgr.dll", CharSet = CharSet.Auto)]
    private static extern int RmStartSession(out uint pSessionHandle, int dwSessionFlags, string strSessionKey);

    [DllImport("rstrtmgr.dll")]
    private static extern int RmEndSession(uint pSessionHandle);

    [DllImport("rstrtmgr.dll")]
    private static extern int RmGetList(uint dwSessionHandle,
        out uint pnProcInfoNeeded,
        ref uint pnProcInfo,
        [In] [Out] RM_PROCESS_INFO[] rgAffectedApps,
        ref uint lpdwRebootReasons);

    /// <summary>
    ///     Find out what process(es) have a lock on the specified file.
    /// </summary>
    /// <param name="path">Path of the file.</param>
    /// <returns>Processes locking the file</returns>
    /// <remarks>
    ///     See also:
    ///     http://msdn.microsoft.com/en-us/library/windows/desktop/aa373661(v=vs.85).aspx
    ///     http://wyupdate.googlecode.com/svn-history/r401/trunk/frmFilesInUse.cs (no copyright in code at time of viewing)
    /// </remarks>
    public static List<Process> WhoIsLocking(string path)
    {
        if (!File.Exists(path))
        {
            Logger.Info($"The path provided ('{path}') is not a file!");
            if (!Directory.Exists(path))
            {
                Logger.Info($"The path provided ('{path}') is invalid!");
                return []; // Return an empty list, don't care
            }

            Logger.Info($"Searching for files in the provided path ('{path}') now...");
            return Directory.EnumerateFiles(path, "*", SearchOption.AllDirectories)
                .SelectMany(WhoIsLocking).DistinctBy(x => x.Id).ToList(); // Check all files
        }

        var key = Guid.NewGuid().ToString();
        List<Process> processes = [];

        var res = RmStartSession(out var handle, 0, key);
        if (res != 0) throw new Exception("Could not begin restart session.  Unable to determine file locker.");

        try
        {
            uint pnProcInfo = 0, lpdwRebootReasons = RmRebootReasonNone;
            var resources = new[] { path }; // Just checking on one resource.

            res = RmRegisterResources(handle, (uint)resources.Length, resources, 0, null, 0, null);
            if (res != 0) throw new Exception("Could not register resource.");

            //Note: there's a race condition here -- the first call to RmGetList() returns
            //      the total number of process. However, when we call RmGetList() again to get
            //      the actual processes this number may have increased.
            res = RmGetList(handle, out var pnProcInfoNeeded, ref pnProcInfo, null, ref lpdwRebootReasons);
            if (res == 234) // ERROR_MORE_DATA
            {
                // Create an array to store the process results
                var processInfo = new RM_PROCESS_INFO[pnProcInfoNeeded];
                pnProcInfo = pnProcInfoNeeded;

                // Get the list
                res = RmGetList(handle, out pnProcInfoNeeded, ref pnProcInfo, processInfo, ref lpdwRebootReasons);
                if (res == 0)
                {
                    processes = new List<Process>((int)pnProcInfo);

                    // Enumerate all of the results and add them to the 
                    // list to be returned
                    for (var i = 0; i < pnProcInfo; i++)
                        try
                        {
                            processes.Add(Process.GetProcessById(processInfo[i].Process.dwProcessId));
                        }
                        // catch the error -- in case the process is no longer running
                        catch (ArgumentException e)
                        {
                            Logger.Info(e);
                        }
                }
                else
                {
                    throw new Exception("Could not list processes locking resource.");
                }
            }
            else if (res != 0)
            {
                throw new Exception("Could not list processes locking resource. Failed to get size of result.");
            }
        }
        catch (Exception e)
        {
            Logger.Info(e);
        }
        finally
        {
            RmEndSession(handle);
        }

        return processes;
    }

    /// <summary>
    ///     Returns whether the current process is elevated or not
    /// </summary>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool IsCurrentProcessElevated()
    {
        var currentIdentity = WindowsIdentity.GetCurrent();
        var currentGroup = new WindowsPrincipal(currentIdentity);
        return currentGroup.IsInRole(WindowsBuiltInRole.Administrator);
    }

    public static bool IsProcessElevated(Process process)
    {
        try
        {
            var handle = OpenProcess(process, ProcessAccessFlags.QueryLimitedInformation);
            if (!OpenProcessToken(handle, TOKEN_QUERY, out var token)) return true;

            GetTokenInformation(token, TOKEN_INFORMATION_CLASS.TokenElevation,
                IntPtr.Zero, 0, out var length);

            var elevation = Marshal.AllocHGlobal((int)length);
            if (!GetTokenInformation(token, TOKEN_INFORMATION_CLASS.TokenElevation,
                    elevation, length, out _)) return true;

            return Marshal.PtrToStructure<TOKEN_ELEVATION>(elevation).TokenIsElevated != 0;
        }
        catch (Exception)
        {
            return true;
        }
    }

    [DllImport("advapi32.dll", SetLastError = true)]
    private static extern bool GetTokenInformation(
        IntPtr TokenHandle, TOKEN_INFORMATION_CLASS TokenInformationClass,
        IntPtr TokenInformation, uint TokenInformationLength, out uint ReturnLength);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern IntPtr OpenProcess(uint processAccess, bool bInheritHandle, uint processId);

    private static IntPtr OpenProcess(Process proc, ProcessAccessFlags flags)
    {
        return OpenProcess((uint)flags, false, (uint)proc.Id);
    }

    [DllImport("advapi32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool OpenProcessToken(IntPtr ProcessHandle, uint DesiredAccess, out IntPtr TokenHandle);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern bool QueryFullProcessImageName(
        [In] IntPtr hProcess,
        [In] int dwFlags,
        [Out] StringBuilder lpExeName,
        ref int lpdwSize);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern IntPtr OpenProcess(
        ProcessAccessFlags processAccess,
        bool bInheritHandle,
        int processId);

    public static string GetProcessFilename(Process p)
    {
        try
        {
            var capacity = 2000;
            var builder = new StringBuilder(capacity);
            var ptr = OpenProcess(ProcessAccessFlags.QueryLimitedInformation, false, p.Id);
            return !QueryFullProcessImageName(ptr, 0, builder, ref capacity) ? null : builder.ToString();
        }
        catch (Exception e)
        {
            Logger.Info(e);
            return null;
        }
    }

    public static string GetSteamInstallDirectory()
    {
        // Get Steam Directory from Registry, starting with 64-bit and falling back to 32-bit
        var steamInstallDirectory = (string)Registry.GetValue(
            @"HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Valve\Steam", "InstallPath", string.Empty);

        if (Directory.Exists(steamInstallDirectory)) return steamInstallDirectory;
        steamInstallDirectory = (string)Registry.GetValue(
            @"HKEY_LOCAL_MACHINE\SOFTWARE\Valve\Steam", "InstallPath", string.Empty);

        if (Directory.Exists(steamInstallDirectory)) return steamInstallDirectory;
        throw new FileNotFoundException("Steam installation directory not found!");
    }

    private struct TOKEN_ELEVATION
    {
#pragma warning disable CS0649
        public uint TokenIsElevated;
#pragma warning restore CS0649
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct RM_UNIQUE_PROCESS
    {
        public readonly int dwProcessId;
        public readonly FILETIME ProcessStartTime;
    }

    private enum RM_APP_TYPE
    {
        RmUnknownApp = 0,
        RmMainWindow = 1,
        RmOtherWindow = 2,
        RmService = 3,
        RmExplorer = 4,
        RmConsole = 5,
        RmCritical = 1000
    }

    private enum TOKEN_INFORMATION_CLASS
    {
        TokenUser = 1,
        TokenGroups,
        TokenPrivileges,
        TokenOwner,
        TokenPrimaryGroup,
        TokenDefaultDacl,
        TokenSource,
        TokenType,
        TokenImpersonationLevel,
        TokenStatistics,
        TokenRestrictedSids,
        TokenSessionId,
        TokenGroupsAndPrivileges,
        TokenSessionReference,
        TokenSandBoxInert,
        TokenAuditPolicy,
        TokenOrigin,
        TokenElevationType,
        TokenLinkedToken,
        TokenElevation,
        TokenHasRestrictions,
        TokenAccessInformation,
        TokenVirtualizationAllowed,
        TokenVirtualizationEnabled,
        TokenIntegrityLevel,
        TokenUIAccess,
        TokenMandatoryPolicy,
        TokenLogonSid,
        MaxTokenInfoClass
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    private struct RM_PROCESS_INFO
    {
        public readonly RM_UNIQUE_PROCESS Process;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = CCH_RM_MAX_APP_NAME + 1)]
        public readonly string strAppName;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = CCH_RM_MAX_SVC_NAME + 1)]
        public readonly string strServiceShortName;

        public readonly RM_APP_TYPE ApplicationType;
        public readonly uint AppStatus;
        public readonly uint TSSessionId;
        [MarshalAs(UnmanagedType.Bool)] public readonly bool bRestartable;
    }

    [Flags]
    private enum ProcessAccessFlags : uint
    {
        QueryLimitedInformation = 0x00001000
    }
}