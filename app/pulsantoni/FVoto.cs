using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Collections;

namespace pulsantoni
{
    public partial class FVoto : Form
    {
        public FVoto()
        {
            InitializeComponent();
        }

        private void BClose_Click(object sender, EventArgs e)
        {
            Program.master.StopVoto();
            this.Hide();
        }

        private void FVoto_Load(object sender, EventArgs e)
        {
            lvoti.Columns.Add(" ", -2, HorizontalAlignment.Center);
            lvoti.Columns.Add("Device number", -2, HorizontalAlignment.Center );
            lvoti.Columns.Add("Seconds from start", -2, HorizontalAlignment.Center);
            lvoti.Columns.Add(" ", -2, HorizontalAlignment.Center);
            lvoti.Columns[0].Width = (int)(this.Width * 0.1);
            lvoti.Columns[1].Width = (int)(this.Width * 0.4);
            lvoti.Columns[2].Width = (int)(this.Width * 0.4);
            lvoti.Columns[3].Width = (int)(this.Width * 0.1);

            lvoti.Columns[0].TextAlign = HorizontalAlignment.Center;
            lvoti.Columns[1].TextAlign = HorizontalAlignment.Center;
            lvoti.ListViewItemSorter = new ListViewItemComparer(2);
            this.FormBorderStyle = FormBorderStyle.FixedSingle;
            this.MinimizeBox = false;
            this.MaximizeBox = false;
            this.Location = new Point(0, 0);
            //this.Size = Screen.PrimaryScreen.Bounds.Size;
            this.Width = Screen.PrimaryScreen.Bounds.Width;
            this.Height = Screen.PrimaryScreen.Bounds.Height;




        }
        public void primorosso()
        {
            foreach (ListViewItem i in lvoti.Items) {
                if (i.Index == 0) i.ForeColor = Color.Red; else i.ForeColor = Color.Green;
            }

        }
        class ListViewItemComparer : IComparer
        {
            private int col;
            /*
            public ListViewItemComparer()
            {
                col = 0;
            }
            */
            public ListViewItemComparer(int column)
            {
                col = column;
            }
            public int Compare(object x, object y)
            {
                float xi, yi;
                ListViewItem xl = (ListViewItem)x;
                ListViewItem yl = (ListViewItem)y;

                xi = float.Parse(xl.SubItems[col].Text );
                yi = float.Parse(yl.SubItems[col].Text);
                if (xi > yi) return 1;
                if (xi == yi) return 0;
                return -1;
                //return (int)(xi - yi);

                // positivo se x>1, 0 se uguai, neg else
            }
        }

        private void bstoppoll_Click(object sender, EventArgs e)
        {
            if (Program.master.VotoInCorso ) Program.master.StopVoto(); else Program.master.StartVoto();
        }

        private void FVoto_FormClosing(object sender, FormClosingEventArgs e)
        {
            e.Cancel = !Program.chiusura; 
        }
    }
}
