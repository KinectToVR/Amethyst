using System;
using System.IO;
using System.Threading.Tasks;
using Windows.Media.Core;
using Windows.Media.Playback;
using Amethyst.Classes;

namespace Amethyst.Utils;

public static class AppSounds
{
    // Sound types
    public enum AppSoundType
    {
        CalibrationAborted,
        CalibrationComplete,
        CalibrationPointCaptured,
        CalibrationStart,
        CalibrationTick,
        Error,
        Focus,
        GoBack,
        Hide,
        Invoke,
        MoveNext,
        MovePrevious,
        Show,
        ToggleOff,
        ToggleOn,
        TrackersConnected,
        TrackersDisconnected
    }

    // Play a desired sound
    public static void PlayAppSound(AppSoundType sound)
    {
        Task.Run(async () =>
        {
            try
            {
                // Check if the sound file even exists & if sounds are on
                if (AppData.Settings.EnableAppSounds &&
                    File.Exists(Path.Join(Interfacing.GetProgramLocation().Directory.ToString(),
                        "Assets", "Sounds", sound + ".wav")))
                {
                    // Load the sound file into a player
                    using var player = new MediaPlayer
                    {
                        Source = MediaSource.CreateFromUri(
                            new Uri(Path.Join(Interfacing.GetProgramLocation().Directory.ToString(),
                                "Assets", "Sounds", sound + ".wav"))),

                        // Set the desired volume
                        Volume = AppData.Settings.AppSoundsVolume
                    };

                    player.CommandManager.IsEnabled = false;
                    player.Play(); // Play the sound

                    // Wait for the sound to complete
                    await Task.Delay(2000);
                }
                else
                {
                    Logger.Warn(
                        $"Sound file {Path.Join(Interfacing.GetProgramLocation().Directory.ToString(), "Assets", "Sounds", sound + ".wav")}" +
                        "could not be played because it does not exist, please check if it's there.");
                }
            }
            catch (Exception e)
            {
                Logger.Warn($"A sound file with type: {sound} " +
                            $"could not be played due to an unexpected error: {e.Message}");
            }
        });
    }
}