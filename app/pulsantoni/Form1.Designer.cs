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
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.TBNumMaxSlave = new System.Windows.Forms.TextBox();
            this.tbSpeed = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.TBMsg = new System.Windows.Forms.TextBox();
            this.TBStato = new System.Windows.Forms.TextBox();
            this.listBox1 = new System.Windows.Forms.ListBox();
            this.lv1 = new System.Windows.Forms.ListView();
            this.SuspendLayout();
            // 
            // tbPorta
            // 
            this.tbPorta.Location = new System.Drawing.Point(12, 12);
            this.tbPorta.Name = "tbPorta";
            this.tbPorta.Size = new System.Drawing.Size(168, 20);
            this.tbPorta.TabIndex = 1;
            this.tbPorta.Text = "COM1";
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(475, 12);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 2;
            this.button1.Text = "&Connect";
            this.button1.UseVisualStyleBackColor = true;
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(186, 270);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(75, 23);
            this.button2.TabIndex = 4;
            this.button2.Text = "&Send";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // TBNumMaxSlave
            // 
            this.TBNumMaxSlave.Location = new System.Drawing.Point(186, 38);
            this.TBNumMaxSlave.Name = "TBNumMaxSlave";
            this.TBNumMaxSlave.ReadOnly = true;
            this.TBNumMaxSlave.Size = new System.Drawing.Size(75, 20);
            this.TBNumMaxSlave.TabIndex = 3;
            // 
            // tbSpeed
            // 
            this.tbSpeed.Location = new System.Drawing.Point(186, 12);
            this.tbSpeed.Name = "tbSpeed";
            this.tbSpeed.Size = new System.Drawing.Size(168, 20);
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
            this.TBMsg.Location = new System.Drawing.Point(267, 64);
            this.TBMsg.Name = "TBMsg";
            this.TBMsg.ReadOnly = true;
            this.TBMsg.Size = new System.Drawing.Size(327, 20);
            this.TBMsg.TabIndex = 7;
            // 
            // TBStato
            // 
            this.TBStato.Location = new System.Drawing.Point(267, 90);
            this.TBStato.Name = "TBStato";
            this.TBStato.ReadOnly = true;
            this.TBStato.Size = new System.Drawing.Size(327, 20);
            this.TBStato.TabIndex = 8;
            // 
            // listBox1
            // 
            this.listBox1.FormattingEnabled = true;
            this.listBox1.Location = new System.Drawing.Point(12, 64);
            this.listBox1.Name = "listBox1";
            this.listBox1.Size = new System.Drawing.Size(249, 199);
            this.listBox1.TabIndex = 9;
            // 
            // lv1
            // 
            this.lv1.BackColor = System.Drawing.Color.White;
            this.lv1.Location = new System.Drawing.Point(267, 116);
            this.lv1.MultiSelect = false;
            this.lv1.Name = "lv1";
            this.lv1.Size = new System.Drawing.Size(327, 147);
            this.lv1.TabIndex = 10;
            this.lv1.UseCompatibleStateImageBehavior = false;
            this.lv1.View = System.Windows.Forms.View.Details;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(656, 307);
            this.Controls.Add(this.lv1);
            this.Controls.Add(this.listBox1);
            this.Controls.Add(this.TBStato);
            this.Controls.Add(this.TBMsg);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.tbSpeed);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.TBNumMaxSlave);
            this.Controls.Add(this.button1);
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
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.TextBox TBNumMaxSlave;
        private System.Windows.Forms.TextBox tbSpeed;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox TBMsg;
        private System.Windows.Forms.TextBox TBStato;
        private System.Windows.Forms.ListBox listBox1;
        private System.Windows.Forms.ListView lv1;
    }
}

