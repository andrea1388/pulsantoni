using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace pulsantoni
{

    static class Program
    {
        public static MasterClass master;
        public static bool chiusura;
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            try
            {
                master = new MasterClass();
                string line;
                System.IO.StreamReader file = new System.IO.StreamReader(args[0]);
                while ((line = file.ReadLine()) != null)
                {
                    line = line.Trim();
                    String[] sottocomandi = line.Split('=');
                    switch(sottocomandi[0])
                    {
                        case "porta":
                            master.portname = sottocomandi[1];
                            break;
                        case "baudrate":
                            master.BaudRate  = int.Parse(sottocomandi[1]);
                            break;
                    }
                }

                file.Close();


            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
                return;
            }
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1());
        }
    }
}
