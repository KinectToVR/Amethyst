using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Runtime.CompilerServices;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using System.Threading.Tasks;
using Amethyst.Classes;
using Amethyst.Plugins.Contract;
using Amethyst.Popups;
using Amethyst.Utils;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Media.Animation;
using WinRT;
using Windows.Devices.Sensors;
using Windows.Storage;
using Windows.UI;
using LibMMD;
using Microsoft.UI.Xaml.Input;

namespace Amethyst.Controls;

public sealed partial class ReplayManagerControl : Grid, INotifyPropertyChanged
{
    private bool _listViewChangeBlock = false;
    private ReplayManager Manager => Interfacing.ReplayManager;

    public ReplayManagerControl()
    {
        InitializeComponent();
    }

    public TrackingRecording SelectedRecording { get; set; }
    public RecordingStageType RecordingStage { get; set; } = RecordingStageType.Waiting;
    public bool IsAddingNewRecording { get; set; }
    public bool IsAddingNewRecordingInverse => !IsAddingNewRecording;
    public string SelectedRecordingFilename => SelectedRecording?.FolderName ?? string.Empty;
    public bool RecordingValid => !string.IsNullOrEmpty(SelectedRecording?.Name) && AllowInteractions;
    public bool AllowInteractions => !Manager.IsWorking && !BlockInteractions;
    public double ActionButtonOpacity => IsAddingNewRecording && AllowInteractions ? 1.0 : 0.0;
    private bool BlockInteractions { get; set; }
    private Exception ServiceException { get; set; }
    public bool SelectionInvalid => SelectedRecording is null;
    public bool ShowPlayButton => !IsAddingNewRecording && !BlockInteractions;

    private bool _allowCancelButton = true;
    public bool ShowCancelButton => !AllowInteractions && _allowCancelButton;
    public double CancelButtonOpacity => ShowCancelButton ? 1.0 : 0.0;
    public bool ShowBlockedButton => !_allowCancelButton;
    public double BlockedButtonOpacity => _allowCancelButton ? 0.0 : 1.0;
    public bool ShowFolderName => !string.IsNullOrEmpty(SelectedRecordingFilename);

    public IEnumerable<TrackingRecordingBase> Recordings =>
        Manager.Recordings.Select(TrackingRecordingBase (x) => x);

    private string CancelButtonText => Interfacing.LocalizedJsonString(
        Manager.IsRecording ? "/GeneralPage/Buttons/Cancel" : "/Main/InfoBars/Playback/Stop");

    public string ActionButtonContent => Interfacing.LocalizedJsonString(
        $"/Recordings/Buttons/{(IsAddingNewRecording ? "Record" : "Play")}");

    public string[] InstructionsText
    {
        get
        {
            return RecordingStage switch
            {
                RecordingStageType.Transform =>
                [
                    Interfacing.LocalizedJsonString("9007B4A4-5C0E-4A27-8F9A-5D96879F84CF"),
                    Interfacing.LocalizedJsonString("068FBBEB-BC01-4C45-BB13-4F7450670879")
                ],
                RecordingStageType.Recording =>
                [
                    Interfacing.LocalizedJsonString("E62D6030-3C71-4252-9027-6AA9DCA428BF"),
                    Interfacing.LocalizedJsonString("F0F8FD10-F62D-4AC1-928C-D2E78C2D2803")
                ],
                RecordingStageType.Finished =>
                [
                    Interfacing.LocalizedJsonString("15DCA6D0-9966-4E20-9667-30761DBB10BA"),
                    Interfacing.LocalizedJsonString("9AADE704-1D3D-4DCB-8EA1-01CD92F8AD17")
                ],
                RecordingStageType.Cancelled =>
                [
                    Interfacing.LocalizedJsonString("AC5C8EE1-93FC-470B-95D9-A8AFFEA2377F"),
                    Interfacing.LocalizedJsonString("825957E4-F95C-477B-8C8A-FEA9156559D9")
                ],
                RecordingStageType.Exception =>
                [
                    Interfacing.LocalizedJsonString("810C3FF8-85B6-4518-9575-849314CFD442"),
                    ServiceException.Message
                ],
                _ =>
                [
                    Interfacing.LocalizedJsonString("B2863E3E-4399-4CAD-A68C-C97B22DE87F8"),
                    Interfacing.LocalizedJsonString("F3BBC592-73BB-4D12-855E-9E9B5F7CD969")
                ]
            };
        }
    }

    public string SelectedRecordingName
    {
        get => IsAddingNewRecording && SelectedRecording is not null
            ? SelectedRecording?.Name
            : SelectedRecording?.Name ?? Interfacing
                .LocalizedJsonString("/Recordings/Title/NoSelection");
        set
        {
            if (!IsAddingNewRecording || SelectedRecording is null) return;
            SelectedRecording.Name = value;
            OnPropertyChanged();
        }
    }

    private void ReplayManagerControl_OnLoaded(object sender, RoutedEventArgs e)
    {
        if (!AllowInteractions) return;
        ReloadRecordings();
    }

    private void ReloadRecordings()
    {
        SelectedRecording = null;
        IsAddingNewRecording = !Manager.Recordings.Any();
        if (IsAddingNewRecording)
            SelectedRecording = new TrackingRecording();

        RecordingsListView.SelectionMode = ListViewSelectionMode.None;
        RecordingsListView.SelectionMode = ListViewSelectionMode.Single;

        OnPropertyChanged();
    }

    private async void RecordingsListView_OnSelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if (sender is not ListView view || _listViewChangeBlock) return;
        if (e.AddedItems.FirstOrDefault() is not TrackingRecordingBase recordingBase)
        {
            await Tree_LaunchTransition();
            AppSounds.PlayAppSound(AppSounds.AppSoundType.Focus);
            return; // Give up now...
        }

        var recording = Manager.Recordings
            .FirstOrDefault(x => x.Name == recordingBase.Name);

        var shouldAnimate = SelectedRecording != recording;
        SelectedRecording = recording;
        IsAddingNewRecording = false;
        OnPropertyChanged();

        if (!shouldAnimate) return;
        await Tree_LaunchTransition();
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
    }

    private async void NewRecordingItem_OnClick(object sender, RoutedEventArgs e)
    {
        if (!RecordingsListView.IsLoaded) return;

        RecordingsListView.SelectionMode = ListViewSelectionMode.None;
        RecordingsListView.SelectionMode = ListViewSelectionMode.Single;

        SelectedRecording = new TrackingRecording();
        IsAddingNewRecording = true;
        RecordingStage = RecordingStageType.Waiting;

        PointCaptureStabilityBorder.BorderThickness = new Thickness(4.0);
        PointCaptureStabilityBorder.BorderBrush = Application.Current
            .Resources["NoThemeColorSolidColorBrushOpposite"].As<SolidColorBrush>();

        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
        OnPropertyChanged();
        await Tree_LaunchTransition();
    }

    private async void ExportRecording_OnClick(object sender, RoutedEventArgs e)
    {
        if (SelectedRecording is null) return;

        var savePicker = new Windows.Storage.Pickers.FileSavePicker
            { SuggestedStartLocation = Windows.Storage.Pickers.PickerLocationId.DocumentsLibrary };

        WinRT.Interop.InitializeWithWindow.Initialize(savePicker, Shared.Main.AppWindowId);
        savePicker.FileTypeChoices.Add("Vocaloid Motion Data files", [".vmd"]);
        savePicker.SuggestedFileName = SelectedRecording.Name;

        var file = await savePicker.PickSaveFileAsync();
        if (file is not null)
            try
            {
                CachedFileManager.DeferUpdates(file);
                VmdParser.Save(SelectedRecording.Motion, file.Path);
            }
            catch (Exception ex)
            {
                Logger.Error(ex);
            }

        RecordingRemoveFlyout.Hide();
        await Tree_LaunchTransition();
    }

    private async void RemoveRecording_OnClick(object sender, RoutedEventArgs e)
    {
        if (SelectedRecording is null) return;

        try
        {
            Directory.Delete(Path.Join((await Interfacing.LocalFolder
                    .CreateFolderAsync("Recordings", CreationCollisionOption.OpenIfExists))
                .Path, SelectedRecording.FolderName), true);
        }
        catch (Exception ex)
        {
            Logger.Error(ex);
        }

        Manager.Recordings.RemoveAll(x => x.FolderName == SelectedRecording.FolderName);
        await Manager.WriteSavedRecordings();

        SelectedRecording = null;
        RecordingRemoveFlyout.Hide();

        OnPropertyChanged();
        await Tree_LaunchTransition();
    }

    private void RecordingPlayButton_OnClick(SplitButton sender, SplitButtonClickEventArgs args)
    {
        if (SelectedRecording is null) return;

        // ReSharper disable once AsyncVoidLambda
        DispatcherQueue.TryEnqueue(async () =>
        {
            AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

            BlockInteractions = true;
            _allowCancelButton = false;
            OnPropertyChanged();

            try
            {
                _ = Task.Run(async () => await Manager.Play(SelectedRecording));

                await Task.Delay(100); // Wait for the token
                while (Manager.IsWorking)
                {
                    // Block the button while loading data
                    _allowCancelButton = Manager.IsPlaying;

                    OnPropertyChanged();
                    await Task.Delay(500);
                }
            }
            catch (Exception ex)
            {
                Logger.Error(ex);
            }

            BlockInteractions = false;
            OnPropertyChanged();
        });
    }

    private void ImportRecordingItem_OnClick(object sender, RoutedEventArgs e)
    {
        throw new NotImplementedException();
    }

    private void StartRecordingButton_OnClick(object sender, RoutedEventArgs e)
    {
        // ReSharper disable once AsyncVoidLambda
        DispatcherQueue.TryEnqueue(async () =>
        {
            AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
            BlockInteractions = true;
            RecordingStage = RecordingStageType.Transform;

            var playspaceRotation = new Quaternion();
            Exception exception = null;

            OnPropertyChanged();

            // Stage 1 - Capture the headset orientation
            try
            {
                var signaledStabilityOnce = false;
                var moveController = new JointStabilityDetector();

                _preCancellationRequested = false;
                while (!_preCancellationRequested)
                {
                    var neutralBrush = Application.Current
                        .Resources["NoThemeColorSolidColorBrushOpposite"].As<SolidColorBrush>();
                    var accentBrush = Application.Current
                        .Resources["SystemFillColorAttentionBrush"].As<SolidColorBrush>();
                    var stability = moveController.Update(
                        AppPlugins.CurrentServiceEndpoint.HeadsetPose?.Orientation.EulerAngles() ?? Vector3.Zero);

                    PointCaptureStabilityBorder.BorderThickness = new Thickness(
                        stability.Map(0.0, 0.9, 4.0, 20.0));
                    PointCaptureStabilityBorder.BorderBrush = neutralBrush
                        .Blend(accentBrush, stability); // Make the border more colorful too

                    switch (stability)
                    {
                        case > 0.9 when !signaledStabilityOnce:
                            AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationTick);
                            signaledStabilityOnce = true;
                            break;
                        case < 0.3 when signaledStabilityOnce:
                            signaledStabilityOnce = false;
                            break;
                    }

                    if (stability > 0.95)
                    {
                        playspaceRotation = Quaternion.Inverse(AppPlugins
                            .CurrentServiceEndpoint.HeadsetPose?.Orientation ?? Quaternion.Identity);

                        break;
                    }

                    await Task.Delay(100);
                }
            }
            catch (Exception ex)
            {
                Logger.Error(ex);
                exception = ex;
                goto post;
            }

            if (_preCancellationRequested)
            {
                RecordingStage = RecordingStageType.Cancelled;
                goto post;
            }

            // Stage 2 - Start the actual recording
            RecordingStage = RecordingStageType.Recording;
            OnPropertyChanged();

            try
            {
                var handle = Task.Run(async () => await Manager.Record(SelectedRecording, playspaceRotation));

                var signaledStabilityOnce = false;
                var moveController = new JointStabilityDetector();

                await Task.Delay(100); // Wait for the token
                while (!_preCancellationRequested && Manager.IsRecording)
                {
                    var neutralBrush = Application.Current
                        .Resources["NoThemeColorSolidColorBrushOpposite"].As<SolidColorBrush>();
                    var accentBrush = Application.Current
                        .Resources["SystemFillColorAttentionBrush"].As<SolidColorBrush>();
                    var stability = moveController.Update(AppData.Settings
                        .TrackersVector.Aggregate(Vector3.Zero, (acc, x) => acc + x.Position));

                    try
                    {
                        PointCaptureStabilityBorder.BorderThickness = new Thickness(
                            stability.Map(0.0, 0.9, 4.0, 20.0));
                        PointCaptureStabilityBorder.BorderBrush = neutralBrush
                            .Blend(accentBrush, stability); // Make the border more colorful too
                    }
                    catch (Exception ex)
                    {
                        Logger.Warn(ex);
                    }

                    switch (stability)
                    {
                        case > 0.9 when !signaledStabilityOnce:
                            AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationTick);
                            signaledStabilityOnce = true;
                            break;
                        case < 0.3 when signaledStabilityOnce:
                            signaledStabilityOnce = false;
                            break;
                    }

                    if (stability > 0.95)
                    {
                        Manager.Stop();
                        break;
                    }

                    await Task.Delay(100);
                }
            }
            catch (Exception ex)
            {
                Logger.Error(ex);
                exception = ex;
            }

            // Stage 3 - Show post information
            post:

            ServiceException = exception;

            if (RecordingStage is not RecordingStageType.Cancelled)
                RecordingStage = exception is null
                    ? RecordingStageType.Finished
                    : RecordingStageType.Exception;

            AppSounds.PlayAppSound(RecordingStage is RecordingStageType.Finished
                ? AppSounds.AppSoundType.CalibrationComplete
                : AppSounds.AppSoundType.CalibrationAborted);

            // Bring back the standard stability border (optional)
            PointCaptureStabilityBorder.BorderThickness = new Thickness(RecordingStage is RecordingStageType.Finished ? 20 : 4);
            PointCaptureStabilityBorder.BorderBrush = RecordingStage is RecordingStageType.Finished
                ? Application.Current.Resources["SystemFillColorAttentionBrush"].As<SolidColorBrush>()
                : new SolidColorBrush(Application.Current.Resources["SystemFillColorCritical"].As<Color>());

            OnPropertyChanged();
            await Task.Delay(1500);

            RecordingStage = RecordingStageType.Waiting;
            BlockInteractions = false;
            IsAddingNewRecording = false;
            OnPropertyChanged();

            RecordingsListView.ItemContainerTransitions.Clear();
            ReloadRecordings();
            RecordingsListView.ItemContainerTransitions = [];

            if (RecordingsListView.Items.Any())
                RecordingsListView.SelectedItem = RecordingsListView.Items.Last();

            await Tree_LaunchTransition();
        });
    }

    private async Task Tree_LaunchTransition()
    {
        // Action stuff reload animation
        try
        {
            // Remove the only one child of our outer main content grid
            // (What a bestiality it is to do that!!1)
            OuterGrid.Children.Remove(PreviewGrid);
            PreviewGrid.Transitions.Add(
                new EntranceThemeTransition { IsStaggeringEnabled = false });

            // Sleep peacefully pretending that noting happened
            await Task.Delay(10);

            // Re-add the child for it to play our funky transition
            // (Though it's not the same as before...)
            OuterGrid.Children.Add(PreviewGrid);

            // Remove the transition
            await Task.Delay(100);
            PreviewGrid.Transitions.Clear();
        }
        catch (Exception e)
        {
            Logger.Error(e);
        }
    }

    public event PropertyChangedEventHandler PropertyChanged;

    private void OnPropertyChanged(string propertyName = null)
    {
        try
        {
            _listViewChangeBlock = true;
            var itemBackup = RecordingsListView.SelectedItem;
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));

            if (RecordingsListView.Items.Contains(itemBackup))
                RecordingsListView.SelectedItem = itemBackup;
            _listViewChangeBlock = false;
        }
        catch (Exception)
        {
            // ignored
        }
    }

    private bool _preCancellationRequested = false;

    private void CancelButton_OnClick(object sender, RoutedEventArgs e)
    {
        _preCancellationRequested = true;

        if (Manager.IsRecording)
            Manager.Stop();
        else
            Manager.Cancel();

        OnPropertyChanged();
    }

    private void FolderName_OnTapped(object sender, TappedRoutedEventArgs e)
    {
        SystemShell.OpenFolderAndSelectItem(Path.Join(Interfacing
            .GetAppDataFilePath(""), "Recordings", SelectedRecording.FolderName));
    }
}

public enum RecordingStageType
{
    Waiting, // 'Press record'
    Transform, // Run stability
    Recording, // End stability
    Finished, // Post message
    Cancelled,
    Exception
}

public static class QuaternionExtensions
{
    public static float ComputeXAngle(this Quaternion q)
    {
        return MathF.Atan2(2 * (q.W * q.X + q.Y * q.Z), 1 - 2 * (q.X * q.X + q.Y * q.Y));
    }

    public static float ComputeYAngle(this Quaternion q)
    {
        var sin = 2 * (q.W * q.Y - q.Z * q.X);
        if (MathF.Abs(sin) >= 1)
            return MathF.PI / 2.0f * MathF.Sign(sin);

        return MathF.Asin(sin);
    }

    public static float ComputeZAngle(this Quaternion q)
    {
        return MathF.Atan2(2 * (q.W * q.Z + q.X * q.Y), 1 - 2 * (q.Y * q.Y + q.Z * q.Z));
    }

    public static Vector3 EulerAngles(this Quaternion q)
    {
        return new Vector3(ComputeXAngle(q), ComputeYAngle(q), ComputeZAngle(q));
    }
}