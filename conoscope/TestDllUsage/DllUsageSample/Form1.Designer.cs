namespace TestDllUsage
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
            this.txtLog = new System.Windows.Forms.TextBox();
            this.btnSdk = new System.Windows.Forms.Button();
            this.cboSdk = new System.Windows.Forms.ComboBox();
            this.btnClear = new System.Windows.Forms.Button();
            this.btnCapture = new System.Windows.Forms.Button();
            this.btnCaptureSequence = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // txtLog
            // 
            this.txtLog.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.txtLog.ForeColor = System.Drawing.SystemColors.WindowText;
            this.txtLog.Location = new System.Drawing.Point(228, 3);
            this.txtLog.Multiline = true;
            this.txtLog.Name = "txtLog";
            this.txtLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.txtLog.Size = new System.Drawing.Size(808, 466);
            this.txtLog.TabIndex = 1;
            // 
            // btnSdk
            // 
            this.btnSdk.Location = new System.Drawing.Point(180, 31);
            this.btnSdk.Name = "btnSdk";
            this.btnSdk.Size = new System.Drawing.Size(42, 23);
            this.btnSdk.TabIndex = 2;
            this.btnSdk.Text = "Run";
            this.btnSdk.UseVisualStyleBackColor = true;
            // 
            // cboSdk
            // 
            this.cboSdk.FormattingEnabled = true;
            this.cboSdk.Location = new System.Drawing.Point(6, 32);
            this.cboSdk.Name = "cboSdk";
            this.cboSdk.Size = new System.Drawing.Size(168, 21);
            this.cboSdk.TabIndex = 4;
            // 
            // btnClear
            // 
            this.btnClear.Location = new System.Drawing.Point(4, 3);
            this.btnClear.Name = "btnClear";
            this.btnClear.Size = new System.Drawing.Size(218, 23);
            this.btnClear.TabIndex = 5;
            this.btnClear.Text = "Clear";
            this.btnClear.UseVisualStyleBackColor = true;
            // 
            // btnCapture
            // 
            this.btnCapture.Location = new System.Drawing.Point(6, 81);
            this.btnCapture.Name = "btnCapture";
            this.btnCapture.Size = new System.Drawing.Size(216, 23);
            this.btnCapture.TabIndex = 6;
            this.btnCapture.Text = "Capture";
            this.btnCapture.UseVisualStyleBackColor = true;
            // 
            // btnCaptureSequence
            // 
            this.btnCaptureSequence.Location = new System.Drawing.Point(6, 110);
            this.btnCaptureSequence.Name = "btnCaptureSequence";
            this.btnCaptureSequence.Size = new System.Drawing.Size(216, 23);
            this.btnCaptureSequence.TabIndex = 7;
            this.btnCaptureSequence.Text = "Capture Sequence";
            this.btnCaptureSequence.UseVisualStyleBackColor = true;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1063, 481);
            this.Controls.Add(this.btnCaptureSequence);
            this.Controls.Add(this.btnCapture);
            this.Controls.Add(this.btnClear);
            this.Controls.Add(this.cboSdk);
            this.Controls.Add(this.btnSdk);
            this.Controls.Add(this.txtLog);
            this.Name = "Form1";
            this.Text = "Form1";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.TextBox txtLog;
        private System.Windows.Forms.Button btnSdk;
        private System.Windows.Forms.ComboBox cboSdk;
        private System.Windows.Forms.Button btnClear;
        private System.Windows.Forms.Button btnCapture;
        private System.Windows.Forms.Button btnCaptureSequence;
    }
}

