using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Windows.Forms;

namespace TestDllUsage
{
    class FormHelper
    {

        public static void Async(Control form, Action action)
        {
            if (form.InvokeRequired)
            {
                form.Invoke(new Action(() => { action(); }), null);
            }
            else
            {
                try
                {
                    action();
                }
                catch (Exception)
                {
                }
            }
        }
    }
}
