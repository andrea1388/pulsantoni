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
            this.label2 = new System.Windows.Forms.Label();
            this.bcambianumslave = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.bdiscstop = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // tbPorta
            // 
            this.tbPorta.Location = new System.Drawing.Point(193, 15);
            this.tbPorta.Margin = new System.Windows.Forms.Padding(4);
            this.tbPorta.Name = "tbPorta";
            this.tbPorta.ReadOnly = true;
            this.tbPorta.Size = new System.Drawing.Size(1009, 23);
            this.tbPorta.TabIndex = 1;
            this.tbPorta.Text = "COM1";
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(1332, 12);
            this.button2.Margin = new System.Windows.Forms.Padding(4);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(183, 28);
            this.button2.TabIndex = 4;
            this.button2.Text = "&Discovery start";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // TBNumMaxSlave
            // 
            this.TBNumMaxSlave.BackColor = System.Drawing.Color.White;
            this.TBNumMaxSlave.Location = new System.Drawing.Point(193, 47);
            this.TBNumMaxSlave.Margin = new System.Windows.Forms.Padding(4);
            this.TBNumMaxSlave.Name = "TBNumMaxSlave";
            this.TBNumMaxSlave.Size = new System.Drawing.Size(99, 23);
            this.TBNumMaxSlave.TabIndex = 3;
            // 
            // tbSpeed
            // 
            this.tbSpeed.Location = new System.Drawing.Point(1212, 16);
            this.tbSpeed.Margin = new System.Windows.Forms.Padding(4);
            this.tbSpeed.Name = "tbSpeed";
            this.tbSpeed.ReadOnly = true;
            this.tbSpeed.Size = new System.Drawing.Size(89, 23);
            this.tbSpeed.TabIndex = 5;
            this.tbSpeed.Text = "9600";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(16, 55);
            this.label1.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(169, 17);
            this.label1.TabIndex = 6;
            this.label1.Text = "Numero Massimo di slave";
            // 
            // TBMsg
            // 
            this.TBMsg.Location = new System.Drawing.Point(193, 80);
            this.TBMsg.Margin = new System.Windows.Forms.Padding(4);
            this.TBMsg.Name = "TBMsg";
            this.TBMsg.ReadOnly = true;
            this.TBMsg.Size = new System.Drawing.Size(1108, 23);
            this.TBMsg.TabIndex = 7;
            // 
            // TBStato
            // 
            this.TBStato.Location = new System.Drawing.Point(785, 48);
            this.TBStato.Margin = new System.Windows.Forms.Padding(4);
            this.TBStato.Name = "TBStato";
            this.TBStato.ReadOnly = true;
            this.TBStato.Size = new System.Drawing.Size(516, 23);
            this.TBStato.TabIndex = 8;
            // 
            // lv1
            // 
            this.lv1.AllowColumnReorder = true;
            this.lv1.BackColor = System.Drawing.Color.White;
            this.lv1.Location = new System.Drawing.Point(16, 146);
            this.lv1.Margin = new System.Windows.Forms.Padding(4);
            this.lv1.MultiSelect = false;
            this.lv1.Name = "lv1";
            this.lv1.Size = new System.Drawing.Size(1285, 474);
            this.lv1.Sorting = System.Windows.Forms.SortOrder.Ascending;
            this.lv1.TabIndex = 10;
            this.lv1.UseCompatibleStateImageBehavior = false;
            this.lv1.View = System.Windows.Forms.View.Details;
            // 
            // BIniziaVoto
            // 
            this.BIniziaVoto.Location = new System.Drawing.Point(1330, 89);
            this.BIniziaVoto.Margin = new System.Windows.Forms.Padding(4);
            this.BIniziaVoto.Name = "BIniziaVoto";
            this.BIniziaVoto.Size = new System.Drawing.Size(183, 28);
            this.BIniziaVoto.TabIndex = 11;
            this.BIniziaVoto.Text = "&Poll start";
            this.BIniziaVoto.UseVisualStyleBackColor = true;
            this.BIniziaVoto.Click += new System.EventHandler(this.BIniziaVoto_Click);
            // 
            // lv2
            // 
            this.lv2.AllowColumnReorder = true;
            this.lv2.BackColor = System.Drawing.Color.White;
            this.lv2.Location = new System.Drawing.Point(1332, 146);
            this.lv2.Margin = new System.Windows.Forms.Padding(4);
            this.lv2.MultiSelect = false;
            this.lv2.Name = "lv2";
            this.lv2.Size = new System.Drawing.Size(181, 474);
            this.lv2.Sorting = System.Windows.Forms.SortOrder.Ascending;
            this.lv2.TabIndex = 12;
            this.lv2.UseCompatibleStateImageBehavior = false;
            this.lv2.View = System.Windows.Forms.View.Details;
            // 
            // bIco
            // 
            this.bIco.Location = new System.Drawing.Point(16, 111);
            this.bIco.Margin = new System.Windows.Forms.Padding(4);
            this.bIco.Name = "bIco";
            this.bIco.Size = new System.Drawing.Size(183, 28);
            this.bIco.TabIndex = 13;
            this.bIco.Text = "I&cons";
            this.bIco.UseVisualStyleBackColor = true;
            this.bIco.Click += new System.EventHandler(this.bIco_Click);
            // 
            // blista
            // 
            this.blista.Location = new System.Drawing.Point(207, 111);
            this.blista.Margin = new System.Windows.Forms.Padding(4);
            this.blista.Name = "blista";
            this.blista.Size = new System.Drawing.Size(183, 28);
            this.blista.TabIndex = 14;
            this.blista.Text = "&List";
            this.blista.UseVisualStyleBackColor = true;
            this.blista.Click += new System.EventHandler(this.blista_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(16, 18);
            this.label2.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(149, 17);
            this.label2.TabIndex = 15;
            this.label2.Text = "Serial port parameters";
            // 
            // bcambianumslave
            // 
            this.bcambianumslave.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.bcambianumslave.Location = new System.Drawing.Point(301, 46);
            this.bcambianumslave.Margin = new System.Windows.Forms.Padding(4);
            this.bcambianumslave.Name = "bcambianumslave";
            this.bcambianumslave.Size = new System.Drawing.Size(183, 28);
            this.bcambianumslave.TabIndex = 16;
            this.bcambianumslave.Text = "C&hange";
            this.bcambianumslave.UseVisualStyleBackColor = true;
            this.bcambianumslave.Click += new System.EventHandler(this.bcambianumslave_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(735, 57);
            this.label3.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(41, 17);
            this.label3.TabIndex = 17;
            this.label3.Text = "State";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(16, 89);
            this.label4.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(72, 17);
            this.label4.TabIndex = 18;
            this.label4.Text = "Messages";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(443, 123);
            this.label5.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(138, 17);
            this.label5.TabIndex = 19;
            this.label5.Text = "List of devices found";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(1328, 127);
            this.label6.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(197, 17);
            this.label6.TabIndex = 20;
            this.label6.Text = "List of not responding devices";
            // 
            // bdiscstop
            // 
            this.bdiscstop.Location = new System.Drawing.Point(1332, 49);
            this.bdiscstop.Margin = new System.Windows.Forms.Padding(4);
            this.bdiscstop.Name = "bdiscstop";
            this.bdiscstop.Size = new System.Drawing.Size(183, 28);
            this.bdiscstop.TabIndex = 21;
            this.bdiscstop.Text = "Discovery st&op";
            this.bdiscstop.UseVisualStyleBackColor = true;
            this.bdiscstop.Click += new System.EventHandler(this.bdiscstop_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1551, 636);
            this.Controls.Add(this.bdiscstop);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.bcambianumslave);
            this.Controls.Add(this.label2);
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
            this.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "Form1";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Main";
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
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button bcambianumslave;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Button bdiscstop;
    }
}

