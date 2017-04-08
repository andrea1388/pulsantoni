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
    public partial class Form1 : Form
    
    {
        String txt;
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

        }
        ~Form1()
        {
          
        }
        void NuovoClient(object sender, DiscoveryEventArgs e)
        {
            String s = e.indirizzo + " " + e.batteria + " " + e.rssi;
            AddItem(s, listBox1);
            //SetText("Pronto", TBStato);
        }
        void Voto(object sender, VotoEventArgs e)
        {
            String s = e.indirizzo  + " " + e.oravoto;
            AddItem(s, listBox1);
            //SetText("Pronto", TBStato);
        }
        void RaggiuntoStato0(object sender, EventArgs e)
        {
            SetText("Pronto", TBStato );
        }
        void InviaSync(object sender, EventArgs e)
        {
            SetText("InviaSync", TBStato);
        }
        void InizioVoto(object sender, EventArgs e)
        {
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
        private void Form1_Load(object sender, EventArgs e)
        {
        }


       

       

        private void button2_Click(object sender, EventArgs e)
        {
            Program.master.StartDiscovery();
        }


        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            Program.master.Close();
        }
    }
}
