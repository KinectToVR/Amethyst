using System;
using System.Collections.Generic;
using System.ComponentModel;
using Amethyst.Classes;
using Microsoft.UI.Xaml.Controls;

namespace Amethyst.MVVM;

public class StorePlugin : INotifyPropertyChanged
{
    public string Name { get; set; }
    public bool Official { get; set; } = false;

    public bool Installing { get; set; } = false;
    public bool InstallSuccess { get; set; } = false;
    public bool InstallError { get; set; } = false;

    public IEnumerable<Contributor> Contributors { get; set; }
    public PluginRepository Repository { get; set; }
    public PluginRelease LatestRelease { get; set; }

    public event PropertyChangedEventHandler PropertyChanged;

    // MVVM stuff
    public string TrimString(string s, int l)
    {
        return s?[..Math.Min(s.Length, l)] +
               (s?.Length > l ? "..." : "");
    }

    public double BoolToOpacity(bool value)
    {
        return value ? 1.0 : 0.0;
    }

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

    public string FormatResourceString(string resourceName)
    {
        return string.Format(Interfacing.LocalizedJsonString(resourceName), Name);
    }

    public class PluginRepository
    {
        public string Name { get; set; }
        public string FullName { get; set; }
        public string Owner { get; set; }
        public string Description { get; set; }
    }

    public class PluginRelease
    {
        public string Name { get; set; }
        public string Version { get; set; }
        public DateTime Date { get; set; }
        public string Description { get; set; }
        public string Changelog { get; set; }
        public string Download { get; set; }
    }

    public class Contributor
    {
        public string Name { get; set; }
        public Uri Avatar { get; set; }
        public Uri Url { get; set; }
    }
}