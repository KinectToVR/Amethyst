using System.IO;
using Newtonsoft.Json;

namespace K2CrashHandler.Helpers;

internal class JsonFile
{
    public static T Read<T>(string path)
    {
        var jsonSerializer = new JsonSerializer();
        jsonSerializer.DefaultValueHandling = DefaultValueHandling.Populate;
        using (var textReader = new StreamReader(path))
        {
            using (var jsonTextReader = new JsonTextReader(textReader))
            {
                return jsonSerializer.Deserialize<T>(jsonTextReader);
            }
        }
    }

    public static void Write(string path, object obj, int identation, char identChar)
    {
        var jsonSerializer = new JsonSerializer();
        using (var textWriter = new StreamWriter(path))
        {
            using (var jsonTextWriter = new JsonTextWriter(textWriter))
            {
                jsonTextWriter.Formatting = Formatting.Indented;
                jsonTextWriter.Indentation = identation;
                jsonTextWriter.IndentChar = identChar;
                jsonSerializer.Serialize(jsonTextWriter, obj);
            }
        }
    }
}