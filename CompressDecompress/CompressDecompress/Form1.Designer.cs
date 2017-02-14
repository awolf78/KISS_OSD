namespace CompressDecompress
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
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.OffsetUpDown = new System.Windows.Forms.NumericUpDown();
            this.LengthUpDown = new System.Windows.Forms.NumericUpDown();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.radioButton1 = new System.Windows.Forms.RadioButton();
            this.radioButton2 = new System.Windows.Forms.RadioButton();
            ((System.ComponentModel.ISupportInitialize)(this.OffsetUpDown)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.LengthUpDown)).BeginInit();
            this.SuspendLayout();
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(44, 30);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(177, 58);
            this.button1.TabIndex = 0;
            this.button1.Text = "Compress...";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(44, 147);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(177, 73);
            this.button2.TabIndex = 1;
            this.button2.Text = "Decompress...";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // OffsetUpDown
            // 
            this.OffsetUpDown.Location = new System.Drawing.Point(382, 39);
            this.OffsetUpDown.Maximum = new decimal(new int[] {
            9,
            0,
            0,
            0});
            this.OffsetUpDown.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.OffsetUpDown.Name = "OffsetUpDown";
            this.OffsetUpDown.Size = new System.Drawing.Size(42, 22);
            this.OffsetUpDown.TabIndex = 2;
            this.OffsetUpDown.Value = new decimal(new int[] {
            7,
            0,
            0,
            0});
            // 
            // LengthUpDown
            // 
            this.LengthUpDown.Location = new System.Drawing.Point(382, 78);
            this.LengthUpDown.Maximum = new decimal(new int[] {
            9,
            0,
            0,
            0});
            this.LengthUpDown.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.LengthUpDown.Name = "LengthUpDown";
            this.LengthUpDown.Size = new System.Drawing.Size(42, 22);
            this.LengthUpDown.TabIndex = 3;
            this.LengthUpDown.Value = new decimal(new int[] {
            4,
            0,
            0,
            0});
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(324, 39);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(52, 18);
            this.label1.TabIndex = 4;
            this.label1.Text = "Offset:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label2.Location = new System.Drawing.Point(324, 78);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(56, 18);
            this.label2.TabIndex = 5;
            this.label2.Text = "Length:";
            // 
            // radioButton1
            // 
            this.radioButton1.AutoSize = true;
            this.radioButton1.Checked = true;
            this.radioButton1.Location = new System.Drawing.Point(327, 147);
            this.radioButton1.Name = "radioButton1";
            this.radioButton1.Size = new System.Drawing.Size(80, 21);
            this.radioButton1.TabIndex = 6;
            this.radioButton1.TabStop = true;
            this.radioButton1.Text = "mcm file";
            this.radioButton1.UseVisualStyleBackColor = true;
            // 
            // radioButton2
            // 
            this.radioButton2.AutoSize = true;
            this.radioButton2.Location = new System.Drawing.Point(327, 175);
            this.radioButton2.Name = "radioButton2";
            this.radioButton2.Size = new System.Drawing.Size(73, 21);
            this.radioButton2.TabIndex = 7;
            this.radioButton2.Text = "hex file";
            this.radioButton2.UseVisualStyleBackColor = true;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(503, 276);
            this.Controls.Add(this.radioButton2);
            this.Controls.Add(this.radioButton1);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.LengthUpDown);
            this.Controls.Add(this.OffsetUpDown);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.button1);
            this.Name = "Form1";
            this.Text = "Form1";
            ((System.ComponentModel.ISupportInitialize)(this.OffsetUpDown)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.LengthUpDown)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.NumericUpDown OffsetUpDown;
        private System.Windows.Forms.NumericUpDown LengthUpDown;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.RadioButton radioButton1;
        private System.Windows.Forms.RadioButton radioButton2;
    }
}

