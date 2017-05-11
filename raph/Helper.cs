using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Hashcode.Qualif
{
    public static class Helper
    {
        public static void Assert(Func<bool> test, string message = "")
        {
#if DEBUG
            Debug.Assert(test(), message);
#endif
        }
    }
}
