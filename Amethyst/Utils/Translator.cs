using System.ComponentModel;
using Amethyst.Classes;

namespace Amethyst.Utils;

public class Translator : INotifyPropertyChanged
{
    public static Translator Get { get; } = new();

    public event PropertyChangedEventHandler PropertyChanged;

    public string String(string key)
    {
        return Interfacing.LocalizedJsonString(key);
    }

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }
}