using Amethyst.Classes;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Amethyst.Utils
{
    public class Translator : INotifyPropertyChanged
    {
        public static Translator Get { get; } = new();

        public string String(string key)
        {
            return Interfacing.LocalizedJsonString(key);
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public void OnPropertyChanged(string propName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
        }
    }
}
