using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Amethyst.Utils
{
    public static class StringUtils
    {
        public static string[] SplitStatusString(string status)
        {
            return status.Split('\n');

            //return new[]
            //{
            //    status[..status.IndexOf('\n')],
            //    status[(status.IndexOf('\n') + 1)..(status.LastIndexOf('\n') - (status.IndexOf('\n') + 1))],
            //    status[..status.LastIndexOf('\n')]
            //};

            //return new[]
            //{
            //    "INVALID STATUS STRING", "E_FIX_YOUR_DAMN_SHIT",
            //    "The status string was invalid, please format it as: [title]\\n[code]\\n[text]"
            //};
        }
    }
}
