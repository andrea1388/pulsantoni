namespace pulsantoni
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.serialPort1 = new System.IO.Ports.SerialPort(this.components);
            this.tbPorta = new System.Windows.Forms.TextBox();
            this.button2 = new System.Windows.Forms.Button();
            this.TBNumMaxSlave = new System.Windows.Forms.TextBox();
            this.tbSpeed = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.TBMsg = new System.Windows.Forms.TextBox();
            this.TBStato = new System.Windows.Forms.TextBox();
            this.lv1 = new System.Windows.Forms.ListView();
            this.BIniziaVoto = new System.Windows.Forms.Button();
            this.lv2 = new System.Windows.Forms.ListView();
            this.bIco = new System.Windows.Forms.Button();
            this.blista = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // tbPorta
            // 
            this.tbPorta.Location = new System.Drawing.Point(12, 12);
            this.tbPorta.Name = "tbPorta";
            this.tbPorta.ReadOnly = true;
            this.tbPorta.Size = new System.Drawing.Size(492, 20);
            this.tbPorta.TabIndex = 1;
            this.tbPorta.Text = "COM1";
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(614, 9);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(137, 23);
            this.button2.TabIndex = 4;
            this.button2.Text = "&Inizia Discovery";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // TBNumMaxSlave
            // 
            this.TBNumMaxSlave.Location = new System.Drawing.Point(145, 38);
            this.TBNumMaxSlave.Name = "TBNumMaxSlave";
            this.TBNumMaxSlave.ReadOnly = true;
            this.TBNumMaxSlave.Size = new System.Drawing.Size(75, 20);
            this.TBNumMaxSlave.TabIndex = 3;
            // 
            // tbSpeed
            // 
            this.tbSpeed.Location = new System.Drawing.Point(510, 12);
            this.tbSpeed.Name = "tbSpeed";
            this.tbSpeed.ReadOnly = true;
            this.tbSpeed.Size = new System.Drawing.Size(84, 20);
            this.tbSpeed.TabIndex = 5;
            this.tbSpeed.Text = "9600";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 38);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(127, 13);
            this.label1.TabIndex = 6;
            this.label1.Text = "Numero Massimo di slave";
            // 
            // TBMsg
            // 
            this.TBMsg.Location = new System.Drawing.Point(12, 64);
            this.TBMsg.Name = "TBMsg";
            this.TBMsg.ReadOnly = true;
            this.TBMsg.Size = new System.Drawing.Size(582, 20);
            this.TBMsg.TabIndex = 7;
            // 
            // TBStato
            // 
            this.TBStato.Location = new System.Drawing.Point(267, 38);
            this.TBStato.Name = "TBStato";
            this.TBStato.ReadOnly = true;
            this.TBStato.Size = new System.Drawing.Size(327, 20);
            this.TBStato.TabIndex = 8;
            // 
            // lv1
            // 
            this.lv1.AllowColumnReorder = true;
            this.lv1.BackColor = System.Drawing.Color.White;
            this.lv1.Location = new System.Drawing.Point(12, 123);
            this.lv1.MultiSelect = false;
            this.lv1.Name = "lv1";
            this.lv1.Size = new System.Drawing.Size(517, 235);
            this.lv1.Sorting = System.Windows.Forms.SortOrder.Ascending;
            this.lv1.TabIndex = 10;
            this.lv1.UseCompatibleStateImageBehavior = false;
            this.lv1.View = System.Windows.Forms.View.Details;
            // 
            // BIniziaVoto
            // 
            this.BIniziaVoto.Location = new System.Drawing.Point(614, 36);
            this.BIniziaVoto.Name = "BIniziaVoto";
            this.BIniziaVoto.Size = new System.Drawing.Size(137, 23);
            this.BIniziaVoto.TabIndex = 11;
            this.BIniziaVoto.Text = "&Inizia Voto";
            this.BIniziaVoto.UseVisualStyleBackColor = true;
            this.BIniziaVoto.Click += new System.EventHandler(this.BIniziaVoto_Click);
            // 
            // lv2
            // 
            this.lv2.AllowColumnReorder = true;
            this.lv2.BackColor = System.Drawing.SystemColors.Window;
            this.lv2.Location = new System.Drawing.Point(545, 123);
            this.lv2.MultiSelect = false;
            this.lv2.Name = "lv2";
            this.lv2.Size = new System.Drawing.Size(225, 235);
            this.lv2.Sorting = System.Windows.Forms.SortOrder.Ascending;
            this.lv2.TabIndex = 12;
            this.lv2.UseCompatibleStateImageBehavior = false;
            this.lv2.View = System.Windows.Forms.View.Details;
            // 
            // bIco
            // 
            this.bIco.Location = new System.Drawing.Point(12, 90);
            this.bIco.Name = "bIco";
            this.bIco.Size = new System.Drawing.Size(137, 23);
            this.bIco.TabIndex = 13;
            this.bIco.Text = "I&cone";
            this.bIco.UseVisualStyleBackColor = true;
            this.bIco.Click += new System.EventHandler(this.bIco_Click);
            // 
            // blista
            // 
            this.blista.Location = new System.Drawing.Point(155, 90);
            this.blista.Name = "blista";
            this.blista.Size = new System.Drawing.Size(137, 23);
            this.blista.TabIndex = 14;
            this.blista.Text = "&Lista";
            this.blista.UseVisualStyleBackColor = true;
            this.blista.Click += new System.EventHandler(this.blista_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(789, 381);
            this.Controls.Add(this.blista);
            this.Controls.Add(this.bIco);
            this.Controls.Add(this.lv2);
            this.Controls.Add(this.BIniziaVoto);
            this.Controls.Add(this.lv1);
            this.Controls.Add(this.TBStato);
            this.Controls.Add(this.TBMsg);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.tbSpeed);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.TBNumMaxSlave);
            this.Controls.Add(this.tbPorta);
            this.Name = "Form1";
            this.Text = "Form1";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.Load += new System.EventHandler(this.Form1_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.IO.Ports.SerialPort serialPort1;
        private System.Windows.Forms.TextBox tbPorta;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.TextBox TBNumMaxSlave;
        private System.Windows.Forms.TextBox tbSpeed;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox TBMsg;
        private System.Windows.Forms.TextBox TBStato;
        private System.Windows.Forms.ListView lv1;
        private System.Windows.Forms.Button BIniziaVoto;
        private System.Windows.Forms.ListView lv2;
        private System.Windows.Forms.Button bIco;
        private System.Windows.Forms.Button blista;
    }
}

