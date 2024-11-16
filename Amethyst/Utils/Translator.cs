using System.Collections.Generic;
using System.ComponentModel;
using Amethyst.Classes;
using Newtonsoft.Json;

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

public class LocalisationFileJson
{
    [JsonProperty("language")] public string Language = string.Empty;
    [JsonProperty("messages")] public List<LocalizedMessage> Messages = [];
}

public class LocalizedMessage
{
    [JsonProperty("translatorComment")] public string Comment = string.Empty;
    [JsonProperty("id")] public string Id = string.Empty;
    [JsonProperty("translation")] public string Translation = string.Empty;
}