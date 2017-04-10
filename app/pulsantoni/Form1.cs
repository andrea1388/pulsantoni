using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace pulsantoni
{
    //delegate void SetTextCallback(string text);
    delegate void SetTextCallback(string text,TextBox t);
    delegate void SetTextCallback2(string text, ListBox t);
    delegate void SetTextCallback3(ListViewItem i, ListView l);
    delegate void SetTextCallback4();
    public partial class Form1 : Form
    
    {
        int stato;
        FVoto fv;
        public Form1()
        {
            InitializeComponent();
            Program.master.EventoStatoZero += RaggiuntoStato0;
            Program.master.EventoInviaSync  += InviaSync ;
            Program.master.EventoInizioPoll  += InizioVoto ;
            Program.master.EventoInizioDiscovery += IniziatoDiscovery;
            Program.master.EventoRxNumMaxSlave  += RxNumSlave;
            Program.master.EventoNuovoMessaggio += Messaggio;
            Program.master.EventoNuovoClient += NuovoClient;
            Program.master.EventoVotoAcquisito += Voto;
            Program.master.EventoDiscFail += DiscFail;
            lv1.Columns.Add("Indirizzo", -2, HorizontalAlignment.Center );
            lv1.Columns.Add("V Batt.", -2, HorizontalAlignment.Center );
            lv1.Columns.Add("Slave Sign.", -2, HorizontalAlignment.Center);
            lv1.Columns.Add("Master Sign.", -2, HorizontalAlignment.Center);
            fv = new FVoto();
            fv.Owner = this;


        }
        ~Form1()
        {
          
        }
        void NuovoClient(object sender, DiscoveryEventArgs e)
        {
            String s = e.indirizzo + " " + e.batteria + " " + e.rssislave + " " + e.rssimaster;
            AddItem(s, listBox1);
            ListViewItem i = new ListViewItem(e.indirizzo.ToString());
            i.SubItems.Add(e.batteria.ToString());
            i.SubItems.Add(e.rssislave .ToString());
            i.SubItems.Add(e.rssimaster.ToString());
            AddLvItem(i, lv1);

            //SetText("Pronto", TBStato);
        }
        void DiscFail(object sender, DiscFailEventArgs e)
        {
            String s = e.indirizzo.ToString()+ " ko";
            AddItem(s, listBox1);
            //SetText("Pronto", TBStato);
        }
        void Voto(object sender, VotoEventArgs e)
        {
            String s = e.indirizzo  + " " + e.oravoto;
            float ov = (float)e.oravoto / (float)1000000;
            String ovs=ov.ToString("#.###");

            AddItem(s, listBox1);
            ListViewItem i = new ListViewItem(e.indirizzo.ToString());
            i.SubItems.Add(ovs);
            this.Invoke((MethodInvoker)delegate { fv.lvoti.Items.Add(i); });
                //SetText("Pronto", TBStato);
            }
        void RaggiuntoStato0(object sender, EventArgs e)
        {
            SetText("Pronto", TBStato );
            this.Invoke((MethodInvoker)delegate { fv.lstato.Text = "Voto conluso"; });
        }
        void InviaSync(object sender, EventArgs e)
        {
            SetText("InviaSync", TBStato);
        }
        void InizioVoto(object sender, EventArgs e)
        {
            //ShowFormVoto();
            this.Invoke((MethodInvoker)delegate { fv.Show(); });
            this.Invoke((MethodInvoker)delegate { fv.lstato.Text = "Voto in corso"; });
            this.Invoke((MethodInvoker)delegate { fv.lvoti.Items.Clear(); });

            DelItem("", listBox1);
            SetText("InizioVoto", TBStato);
        }
        void IniziatoDiscovery(object sender, EventArgs e)
        {
            DelItem("", listBox1);
            SetText("Discovery", TBStato);

        }
        void RxNumSlave(object sender, NumSlaveEventArgs e)
        {
            SetText(e.numslave.ToString(),TBNumMaxSlave );
            //SetNumSlaveText(e.numslave.ToString());

        }
        void Messaggio(object sender, MsgEventArgs e)
        {
            SetText(e.msg, TBMsg );
            //SetMsgText(e.msg );

        }

        private void ShowFormVoto()
        {
            if (fv.InvokeRequired)
            {
                SetTextCallback4 d = new SetTextCallback4(ShowFormVoto);
                this.Invoke(d, new object[] {  });
            }
            else
            {
                fv.Show();
            }
        }
        private void SetText(string text, TextBox tb)
        {
            if (tb.InvokeRequired)
            {
                SetTextCallback d = new SetTextCallback(SetText);
                this.Invoke(d, new object[] { text , tb});
            }
            else
            {
                tb.Text = text;
            }
        }
        private void AddItem(string text, ListBox lb)
        {
            if (lb.InvokeRequired)
            {
                SetTextCallback2 d = new SetTextCallback2(AddItem);
                this.Invoke(d, new object[] { text, lb });
            }
            else
            {
                lb.Items.Add( text);
            }
        }
        private void DelItem(string text, ListBox lb)
        {
            if (lb.InvokeRequired)
            {
                SetTextCallback2 d = new SetTextCallback2(DelItem );
                this.Invoke(d, new object[] { text, lb });
            }
            else
            {
                lb.Items.Clear();
            }
        }
        private void AddLvItem(ListViewItem i, ListView l)
        {
            if (l.InvokeRequired)
            {
                SetTextCallback3 d = new SetTextCallback3(AddLvItem);
                this.Invoke(d, new object[] { i, l });
            }
            else
            {
                l.Items.Add(i);
            }
        }
        private void Form1_Load(object sender, EventArgs e)
        {
        }


       

       

        private void button2_Click(object sender, EventArgs e)
        {
            //if (Program.master.Stato="s0")
            Program.master.StartDiscovery();
        }


        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            Program.master.Close();
        }

        private void BIniziaVoto_Click(object sender, EventArgs e)
        {
            Program.master.StartVoto();
        }
    }
}
