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
            lv1.Columns.Add("Device number", -2, HorizontalAlignment.Center );
            lv1.Columns.Add("V Batt.", -2, HorizontalAlignment.Center );
            lv1.Columns.Add("Slave Sign.", -2, HorizontalAlignment.Center);
            lv1.Columns.Add("Master Sign.", -2, HorizontalAlignment.Center);
            lv1.View = View.Details;
            ImageList imageListSmall = new ImageList();
            ImageList imageListStatus = new ImageList();
            //imageListSmall.Images.Add(Image.FromFile("antennarossa.jpg"));
            imageListSmall.Images.Add(pulsantoni.Properties.Resources.IconOk );
            imageListSmall.Images.Add(pulsantoni.Properties.Resources.IconAntKo );
            imageListStatus.Images.Add(pulsantoni.Properties.Resources.IconBatt );
            //imageListSmall.Images.Add(Image.FromFile("antennablu.jpg"));
            lv1.SmallImageList = imageListSmall;
            lv1.StateImageList = imageListStatus;
            lv1.CheckBoxes = true;
            lv2.Columns.Add("Device number", -2, HorizontalAlignment.Center);
            lv2.View = View.Details;
            fv = new FVoto();
            fv.Owner = this;


        }
        ~Form1()
        {
          
        }
        void NuovoClient(object sender, DiscoveryEventArgs e)
        {
            String ovs = e.batteria .ToString("#.#");
            ListViewItem i = new ListViewItem(e.indirizzo.ToString());
            i.SubItems.Add(ovs);
            i.SubItems.Add(e.rssislave .ToString());
            i.SubItems.Add(e.rssimaster.ToString());
            i.ImageIndex = 0;
            if (e.batteria < 2.8) i.Checked = false; else i.Checked = true;
            if (e.rssimaster  <= 200) i.ImageIndex = 1;
            this.Invoke((MethodInvoker)delegate { lv1.Items.Add(i); });

            //AddLvItem(i, lv1);
        }
        void DiscFail(object sender, DiscFailEventArgs e)
        {
            ListViewItem i = new ListViewItem(e.indirizzo.ToString());
            this.Invoke((MethodInvoker)delegate { lv2.Items.Add(i); });
        }
        void Voto(object sender, VotoEventArgs e)
        {
            float ov = (float)e.oravoto / (float)1000000;
            String ovs=ov.ToString("0.####");
            ListViewItem i = new ListViewItem(" ");
            i.SubItems.Add(e.indirizzo.ToString());
            i.SubItems.Add(ovs);
            this.Invoke((MethodInvoker)delegate { fv.lvoti.Items.Add(i); });
        }
        void RaggiuntoStato0(object sender, EventArgs e)
        {
            SetText("Ready", TBStato );
            this.Invoke((MethodInvoker)delegate { fv.lstato.Text = "Poll ended"; });
            this.Invoke((MethodInvoker)delegate {  fv.bstoppoll.Text  = "Start Poll"; });
        }
        void InviaSync(object sender, EventArgs e)
        {
            SetText("Synching", TBStato);
        }
        void InizioVoto(object sender, EventArgs e)
        {
            this.Invoke((MethodInvoker)delegate { fv.Show(); });
            this.Invoke((MethodInvoker)delegate { fv.lstato.Text = "Poll started"; });
            this.Invoke((MethodInvoker)delegate { fv.lvoti.Items.Clear(); });
            SetText("Poll started", TBStato);
            this.Invoke((MethodInvoker)delegate { fv.bstoppoll.Text = "Stop Poll"; });
        }
        void IniziatoDiscovery(object sender, EventArgs e)
        {
            this.Invoke((MethodInvoker)delegate { fv.Hide(); });
            SetText("Discovery started", TBStato);
            this.Invoke((MethodInvoker)delegate { lv1.Items.Clear(); });
            this.Invoke((MethodInvoker)delegate { lv2.Items.Clear(); });
        }
        void RxNumSlave(object sender, NumSlaveEventArgs e)
        {
            SetText(e.numslave.ToString(),TBNumMaxSlave );
        }
        void Messaggio(object sender, MsgEventArgs e)
        {
            SetText(e.msg, TBMsg );
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
            try
            {
                Program.master.Open();
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message);
                this.Close();
            }
            tbPorta.Text = Program.master.portname;
            tbSpeed.Text = Program.master.BaudRate.ToString();
            Program.master.GetMAxSlave();
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

        private void bIco_Click(object sender, EventArgs e)
        {
            lv1.View= System.Windows.Forms.View.SmallIcon;
        }

        private void blista_Click(object sender, EventArgs e)
        {
            lv1.View = System.Windows.Forms.View.Details;
        }

        private void bcambianumslave_Click(object sender, EventArgs e)
        {
            string input = TBNumMaxSlave.Text;
            int n;
            if(int.TryParse(input,out n))
            {
                if (n > 1 && n < 255)
                    Program.master.SetMAxSlave(n);
                else MessageBox.Show("Error");
            }

        }

        private void bdiscstop_Click(object sender, EventArgs e)
        {
            Program.master.StopDiscovery();
        }
    }
}
