using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace K2CrashHandler.Helpers
{
    class JsonFile
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
}