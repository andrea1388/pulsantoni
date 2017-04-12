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
            this.Hide();
        }

        private void FVoto_Load(object sender, EventArgs e)
        {
            lvoti.Columns.Add("Numero pulsante", -2, HorizontalAlignment.Center);
            lvoti.Columns.Add("Secondi dall'inizio voto", -2, HorizontalAlignment.Center);
            lvoti.Columns[0].Width = this.Width / 2;
            lvoti.Columns[1].Width = this.Width / 2;
            lvoti.Columns[0].TextAlign = HorizontalAlignment.Center;
            lvoti.Columns[1].TextAlign = HorizontalAlignment.Center;
            lvoti.ListViewItemSorter = new ListViewItemComparer(1);



        }
        class ListViewItemComparer : IComparer
        {
            private int col;
            public ListViewItemComparer()
            {
                col = 0;
            }
            public ListViewItemComparer(int column)
            {
                col = column;
            }
            public int Compare(object x, object y)
            {
                float xi, yi;
                ListViewItem xl = (ListViewItem)x;
                ListViewItem yl = (ListViewItem)y;

                xi = float.Parse(xl.SubItems[1].Text );
                yi = float.Parse(yl.SubItems[1].Text);
                return (int)(xi - yi);

                // positivo se x>1, 0 se uguai, neg else
            }
        }
    }
}
