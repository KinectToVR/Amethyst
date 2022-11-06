using System.Collections.Generic;

namespace Amethyst.Classes;

public static class Shared
{
    public static string[] SplitStatusString(string status)
    {
        if (status.IndexOf('\n') + 1 >= 0 && status.IndexOf('\n') + 1 <= status.Length &&
            status.LastIndexOf('\n') - (status.IndexOf('\n') + 1) >= 0 &&
            status.LastIndexOf('\n') - (status.IndexOf('\n') + 1) <= status.Length)

            return new[]
            {
                status[..status.IndexOf('\n')],
                status[(status.IndexOf('\n') + 1)..(status.LastIndexOf('\n') - (status.IndexOf('\n') + 1))],
                status[..status.LastIndexOf('\n')]
            };

        return new[]
        {
            "INVALID STATUS STRING", "E_FIX_YOUR_DAMN_SHIT",
            "The status string was invalid, please format it as: [title]\\n[code]\\n[text]"
        };
    }


}