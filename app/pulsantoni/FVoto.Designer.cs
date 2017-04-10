namespace pulsantoni
{
    partial class FVoto
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
            this.lstato = new System.Windows.Forms.TextBox();
            this.BClose = new System.Windows.Forms.Button();
            this.lvoti = new System.Windows.Forms.ListView();
            this.SuspendLayout();
            // 
            // lstato
            // 
            this.lstato.BackColor = System.Drawing.SystemColors.ButtonFace;
            this.lstato.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.lstato.Font = new System.Drawing.Font("Microsoft Sans Serif", 40F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lstato.Location = new System.Drawing.Point(12, 12);
            this.lstato.Name = "lstato";
            this.lstato.Size = new System.Drawing.Size(438, 61);
            this.lstato.TabIndex = 8;
            this.lstato.Text = ".............";
            // 
            // BClose
            // 
            this.BClose.Location = new System.Drawing.Point(555, 21);
            this.BClose.Name = "BClose";
            this.BClose.Size = new System.Drawing.Size(347, 52);
            this.BClose.TabIndex = 10;
            this.BClose.Text = "&Close";
            this.BClose.UseVisualStyleBackColor = true;
            this.BClose.Click += new System.EventHandler(this.BClose_Click);
            // 
            // lvoti
            // 
            this.lvoti.BackColor = System.Drawing.Color.White;
            this.lvoti.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.lvoti.Font = new System.Drawing.Font("Microsoft Sans Serif", 30F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lvoti.GridLines = true;
            this.lvoti.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.lvoti.Location = new System.Drawing.Point(0, 114);
            this.lvoti.MultiSelect = false;
            this.lvoti.Name = "lvoti";
            this.lvoti.Size = new System.Drawing.Size(1628, 783);
            this.lvoti.Sorting = System.Windows.Forms.SortOrder.Ascending;
            this.lvoti.TabIndex = 11;
            this.lvoti.UseCompatibleStateImageBehavior = false;
            this.lvoti.View = System.Windows.Forms.View.Details;
            // 
            // FVoto
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1628, 897);
            this.ControlBox = false;
            this.Controls.Add(this.lvoti);
            this.Controls.Add(this.BClose);
            this.Controls.Add(this.lstato);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Name = "FVoto";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "FVoto";
            this.WindowState = System.Windows.Forms.FormWindowState.Maximized;
            this.Load += new System.EventHandler(this.FVoto_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        public System.Windows.Forms.TextBox lstato;
        private System.Windows.Forms.Button BClose;
        public System.Windows.Forms.ListView lvoti;
    }
}