namespace Lens_Driver_GUI
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
            this.port_combo = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.connect_btn = new System.Windows.Forms.Button();
            this.label4 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.textConsole = new System.Windows.Forms.TextBox();
            this.LED = new System.Windows.Forms.PictureBox();
            this.baudRateBox = new System.Windows.Forms.ComboBox();
            this.label6 = new System.Windows.Forms.Label();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.voltage_Number = new System.Windows.Forms.NumericUpDown();
            this.volt_GB = new System.Windows.Forms.GroupBox();
            this.voltage_TB = new System.Windows.Forms.TextBox();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
            this.status_PIC_SN = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStripStatusLabel2 = new System.Windows.Forms.ToolStripStatusLabel();
            this.status_PIC_FW = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStripStatusLabel3 = new System.Windows.Forms.ToolStripStatusLabel();
            this.status_DRV_TYP = new System.Windows.Forms.ToolStripStatusLabel();
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.LED)).BeginInit();
            this.groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.voltage_Number)).BeginInit();
            this.volt_GB.SuspendLayout();
            this.statusStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // port_combo
            // 
            this.port_combo.FormattingEnabled = true;
            this.port_combo.Location = new System.Drawing.Point(92, 21);
            this.port_combo.Name = "port_combo";
            this.port_combo.Size = new System.Drawing.Size(100, 24);
            this.port_combo.TabIndex = 0;
            this.port_combo.DropDown += new System.EventHandler(this.port_combo_DropDown);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(52, 24);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(34, 17);
            this.label1.TabIndex = 1;
            this.label1.Text = "Port";
            // 
            // connect_btn
            // 
            this.connect_btn.Location = new System.Drawing.Point(92, 83);
            this.connect_btn.Name = "connect_btn";
            this.connect_btn.Size = new System.Drawing.Size(100, 28);
            this.connect_btn.TabIndex = 2;
            this.connect_btn.Text = "Connect";
            this.connect_btn.UseVisualStyleBackColor = true;
            this.connect_btn.Click += new System.EventHandler(this.connect_btn_Click);
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(268, 109);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(208, 19);
            this.label4.TabIndex = 12;
            this.label4.Text = "Click on LED to turn On/Off";
            this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.textConsole);
            this.groupBox1.Location = new System.Drawing.Point(12, 138);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(538, 150);
            this.groupBox1.TabIndex = 14;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Message Console";
            // 
            // textConsole
            // 
            this.textConsole.Location = new System.Drawing.Point(6, 21);
            this.textConsole.Multiline = true;
            this.textConsole.Name = "textConsole";
            this.textConsole.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.textConsole.Size = new System.Drawing.Size(526, 120);
            this.textConsole.TabIndex = 0;
            // 
            // LED
            // 
            this.LED.Image = global::Lens_Driver_GUI.Properties.Resources.LED_Off;
            this.LED.Location = new System.Drawing.Point(231, 102);
            this.LED.Name = "LED";
            this.LED.Size = new System.Drawing.Size(30, 30);
            this.LED.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.LED.TabIndex = 8;
            this.LED.TabStop = false;
            this.LED.Click += new System.EventHandler(this.LED_Click);
            // 
            // baudRateBox
            // 
            this.baudRateBox.FormattingEnabled = true;
            this.baudRateBox.Items.AddRange(new object[] {
            "4800",
            "9600",
            "19200",
            "38400",
            "57600",
            "115200",
            "200000",
            "250000"});
            this.baudRateBox.Location = new System.Drawing.Point(92, 53);
            this.baudRateBox.Name = "baudRateBox";
            this.baudRateBox.Size = new System.Drawing.Size(100, 24);
            this.baudRateBox.TabIndex = 19;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(11, 60);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(75, 17);
            this.label6.TabIndex = 20;
            this.label6.Text = "Baud Rate";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.port_combo);
            this.groupBox2.Controls.Add(this.label6);
            this.groupBox2.Controls.Add(this.label1);
            this.groupBox2.Controls.Add(this.baudRateBox);
            this.groupBox2.Controls.Add(this.connect_btn);
            this.groupBox2.Location = new System.Drawing.Point(12, 12);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(205, 120);
            this.groupBox2.TabIndex = 21;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Connection Settings";
            // 
            // voltage_Number
            // 
            this.voltage_Number.Location = new System.Drawing.Point(10, 24);
            this.voltage_Number.Maximum = new decimal(new int[] {
            255,
            0,
            0,
            0});
            this.voltage_Number.Name = "voltage_Number";
            this.voltage_Number.Size = new System.Drawing.Size(70, 22);
            this.voltage_Number.TabIndex = 22;
            this.voltage_Number.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.voltage_Number.UseWaitCursor = true;
            this.voltage_Number.ValueChanged += new System.EventHandler(this.numeric_Change);
            // 
            // volt_GB
            // 
            this.volt_GB.Controls.Add(this.voltage_TB);
            this.volt_GB.Controls.Add(this.voltage_Number);
            this.volt_GB.Location = new System.Drawing.Point(231, 12);
            this.volt_GB.Name = "volt_GB";
            this.volt_GB.Size = new System.Drawing.Size(175, 63);
            this.volt_GB.TabIndex = 24;
            this.volt_GB.TabStop = false;
            this.volt_GB.Text = "Voltage Control";
            // 
            // voltage_TB
            // 
            this.voltage_TB.Location = new System.Drawing.Point(93, 24);
            this.voltage_TB.Name = "voltage_TB";
            this.voltage_TB.Size = new System.Drawing.Size(70, 22);
            this.voltage_TB.TabIndex = 24;
            this.voltage_TB.Text = "0.000";
            this.voltage_TB.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // statusStrip1
            // 
            this.statusStrip1.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel1,
            this.status_PIC_SN,
            this.toolStripStatusLabel2,
            this.status_PIC_FW,
            this.toolStripStatusLabel3,
            this.status_DRV_TYP});
            this.statusStrip1.Location = new System.Drawing.Point(0, 294);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(562, 29);
            this.statusStrip1.TabIndex = 25;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // toolStripStatusLabel1
            // 
            this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
            this.toolStripStatusLabel1.Size = new System.Drawing.Size(81, 24);
            this.toolStripStatusLabel1.Text = "Driver S/N:";
            // 
            // status_PIC_SN
            // 
            this.status_PIC_SN.AutoSize = false;
            this.status_PIC_SN.BorderSides = ((System.Windows.Forms.ToolStripStatusLabelBorderSides)((((System.Windows.Forms.ToolStripStatusLabelBorderSides.Left | System.Windows.Forms.ToolStripStatusLabelBorderSides.Top) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Right) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Bottom)));
            this.status_PIC_SN.BorderStyle = System.Windows.Forms.Border3DStyle.Etched;
            this.status_PIC_SN.Name = "status_PIC_SN";
            this.status_PIC_SN.Size = new System.Drawing.Size(50, 24);
            // 
            // toolStripStatusLabel2
            // 
            this.toolStripStatusLabel2.BorderStyle = System.Windows.Forms.Border3DStyle.Etched;
            this.toolStripStatusLabel2.Name = "toolStripStatusLabel2";
            this.toolStripStatusLabel2.Size = new System.Drawing.Size(117, 24);
            this.toolStripStatusLabel2.Text = "Driver Firmware:";
            // 
            // status_PIC_FW
            // 
            this.status_PIC_FW.AutoSize = false;
            this.status_PIC_FW.BorderSides = ((System.Windows.Forms.ToolStripStatusLabelBorderSides)((((System.Windows.Forms.ToolStripStatusLabelBorderSides.Left | System.Windows.Forms.ToolStripStatusLabelBorderSides.Top) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Right) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Bottom)));
            this.status_PIC_FW.BorderStyle = System.Windows.Forms.Border3DStyle.Etched;
            this.status_PIC_FW.Name = "status_PIC_FW";
            this.status_PIC_FW.Size = new System.Drawing.Size(50, 24);
            // 
            // toolStripStatusLabel3
            // 
            this.toolStripStatusLabel3.Name = "toolStripStatusLabel3";
            this.toolStripStatusLabel3.Size = new System.Drawing.Size(88, 24);
            this.toolStripStatusLabel3.Text = "Driver Type:";
            // 
            // status_DRV_TYP
            // 
            this.status_DRV_TYP.AutoSize = false;
            this.status_DRV_TYP.BorderSides = ((System.Windows.Forms.ToolStripStatusLabelBorderSides)((((System.Windows.Forms.ToolStripStatusLabelBorderSides.Left | System.Windows.Forms.ToolStripStatusLabelBorderSides.Top) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Right) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Bottom)));
            this.status_DRV_TYP.BorderStyle = System.Windows.Forms.Border3DStyle.Etched;
            this.status_DRV_TYP.Name = "status_DRV_TYP";
            this.status_DRV_TYP.Size = new System.Drawing.Size(150, 24);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(562, 323);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.volt_GB);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.LED);
            this.MinimumSize = new System.Drawing.Size(580, 370);
            this.Name = "Form1";
            this.Text = "Lens Driver GUI";
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.LED)).EndInit();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.voltage_Number)).EndInit();
            this.volt_GB.ResumeLayout(false);
            this.volt_GB.PerformLayout();
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox port_combo;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button connect_btn;
        private System.Windows.Forms.PictureBox LED;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.TextBox textConsole;
        private System.Windows.Forms.ComboBox baudRateBox;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.NumericUpDown voltage_Number;
        private System.Windows.Forms.GroupBox volt_GB;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
        private System.Windows.Forms.ToolStripStatusLabel status_PIC_SN;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel2;
        private System.Windows.Forms.ToolStripStatusLabel status_PIC_FW;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel3;
        private System.Windows.Forms.ToolStripStatusLabel status_DRV_TYP;
        private System.Windows.Forms.TextBox voltage_TB;
    }
}

