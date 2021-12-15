
namespace noborder_setup
{
    partial class MainForm
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
                //components.Dispose();
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
            System.Windows.Forms.MenuItem miFile;
            System.Windows.Forms.GroupBox groupBox2;
            System.Windows.Forms.Label label1;
            System.Windows.Forms.MenuItem miFileSep1;
            this.miNewFile = new System.Windows.Forms.MenuItem();
            this.miOpenFile = new System.Windows.Forms.MenuItem();
            this.miSaveFile = new System.Windows.Forms.MenuItem();
            this.miSaveFileAs = new System.Windows.Forms.MenuItem();
            this.rdoNeverOnTop = new System.Windows.Forms.RadioButton();
            this.rdoAutoOnTop = new System.Windows.Forms.RadioButton();
            this.rdoAlwaysOnTop = new System.Windows.Forms.RadioButton();
            this.menu1 = new System.Windows.Forms.MainMenu(this.components);
            this.miHelp = new System.Windows.Forms.MenuItem();
            this.miAbout = new System.Windows.Forms.MenuItem();
            this.chkUseDwmFormula = new System.Windows.Forms.CheckBox();
            this.chkExcludeTaskbar = new System.Windows.Forms.CheckBox();
            this.chkUseLetterbox = new System.Windows.Forms.CheckBox();
            this.btnLetterboxColor = new System.Windows.Forms.Button();
            this.miExit = new System.Windows.Forms.MenuItem();
            miFile = new System.Windows.Forms.MenuItem();
            groupBox2 = new System.Windows.Forms.GroupBox();
            label1 = new System.Windows.Forms.Label();
            miFileSep1 = new System.Windows.Forms.MenuItem();
            groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // miFile
            // 
            miFile.Index = 0;
            miFile.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.miNewFile,
            this.miOpenFile,
            this.miSaveFile,
            this.miSaveFileAs,
            miFileSep1,
            this.miExit});
            miFile.Text = "&File";
            // 
            // miNewFile
            // 
            this.miNewFile.Index = 0;
            this.miNewFile.Shortcut = System.Windows.Forms.Shortcut.CtrlN;
            this.miNewFile.Text = "&New";
            this.miNewFile.Click += new System.EventHandler(this.miNewFile_Click);
            // 
            // miOpenFile
            // 
            this.miOpenFile.Index = 1;
            this.miOpenFile.Shortcut = System.Windows.Forms.Shortcut.CtrlO;
            this.miOpenFile.Text = "&Open";
            this.miOpenFile.Click += new System.EventHandler(this.miOpenFile_Click);
            // 
            // miSaveFile
            // 
            this.miSaveFile.Index = 2;
            this.miSaveFile.Shortcut = System.Windows.Forms.Shortcut.CtrlS;
            this.miSaveFile.Text = "&Save";
            this.miSaveFile.Click += new System.EventHandler(this.miSaveFile_Click);
            // 
            // miSaveFileAs
            // 
            this.miSaveFileAs.Index = 3;
            this.miSaveFileAs.Shortcut = System.Windows.Forms.Shortcut.F12;
            this.miSaveFileAs.Text = "Save &As";
            this.miSaveFileAs.Click += new System.EventHandler(this.miSaveFileAs_Click);
            // 
            // groupBox2
            // 
            groupBox2.Controls.Add(this.rdoNeverOnTop);
            groupBox2.Controls.Add(this.rdoAutoOnTop);
            groupBox2.Controls.Add(this.rdoAlwaysOnTop);
            groupBox2.Location = new System.Drawing.Point(21, 53);
            groupBox2.Name = "groupBox2";
            groupBox2.Size = new System.Drawing.Size(277, 71);
            groupBox2.TabIndex = 3;
            groupBox2.TabStop = false;
            groupBox2.Text = "On Top";
            // 
            // rdoNeverOnTop
            // 
            this.rdoNeverOnTop.AutoSize = true;
            this.rdoNeverOnTop.Location = new System.Drawing.Point(190, 31);
            this.rdoNeverOnTop.Name = "rdoNeverOnTop";
            this.rdoNeverOnTop.Size = new System.Drawing.Size(65, 22);
            this.rdoNeverOnTop.TabIndex = 3;
            this.rdoNeverOnTop.Text = "Never";
            this.rdoNeverOnTop.UseVisualStyleBackColor = true;
            // 
            // rdoAutoOnTop
            // 
            this.rdoAutoOnTop.AutoSize = true;
            this.rdoAutoOnTop.Checked = true;
            this.rdoAutoOnTop.Location = new System.Drawing.Point(12, 31);
            this.rdoAutoOnTop.Name = "rdoAutoOnTop";
            this.rdoAutoOnTop.Size = new System.Drawing.Size(56, 22);
            this.rdoAutoOnTop.TabIndex = 1;
            this.rdoAutoOnTop.TabStop = true;
            this.rdoAutoOnTop.Text = "Auto";
            this.rdoAutoOnTop.UseVisualStyleBackColor = true;
            // 
            // rdoAlwaysOnTop
            // 
            this.rdoAlwaysOnTop.AutoSize = true;
            this.rdoAlwaysOnTop.Location = new System.Drawing.Point(93, 31);
            this.rdoAlwaysOnTop.Name = "rdoAlwaysOnTop";
            this.rdoAlwaysOnTop.Size = new System.Drawing.Size(70, 22);
            this.rdoAlwaysOnTop.TabIndex = 2;
            this.rdoAlwaysOnTop.Text = "Always";
            this.rdoAlwaysOnTop.UseVisualStyleBackColor = true;
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Location = new System.Drawing.Point(18, 209);
            label1.Margin = new System.Windows.Forms.Padding(3);
            label1.Name = "label1";
            label1.Size = new System.Drawing.Size(110, 18);
            label1.TabIndex = 6;
            label1.Text = "Letterbox color:";
            // 
            // menu1
            // 
            this.menu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            miFile,
            this.miHelp});
            // 
            // miHelp
            // 
            this.miHelp.Index = 1;
            this.miHelp.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.miAbout});
            this.miHelp.Text = "&Help";
            // 
            // miAbout
            // 
            this.miAbout.Index = 0;
            this.miAbout.Text = "&About...";
            this.miAbout.Click += new System.EventHandler(this.miAbout_Click);
            // 
            // chkUseDwmFormula
            // 
            this.chkUseDwmFormula.AutoSize = true;
            this.chkUseDwmFormula.Location = new System.Drawing.Point(21, 142);
            this.chkUseDwmFormula.Name = "chkUseDwmFormula";
            this.chkUseDwmFormula.Size = new System.Drawing.Size(147, 22);
            this.chkUseDwmFormula.TabIndex = 1;
            this.chkUseDwmFormula.Text = "Use DWM formula";
            this.chkUseDwmFormula.UseVisualStyleBackColor = true;
            // 
            // chkExcludeTaskbar
            // 
            this.chkExcludeTaskbar.AutoSize = true;
            this.chkExcludeTaskbar.Location = new System.Drawing.Point(21, 16);
            this.chkExcludeTaskbar.Margin = new System.Windows.Forms.Padding(12);
            this.chkExcludeTaskbar.Name = "chkExcludeTaskbar";
            this.chkExcludeTaskbar.Size = new System.Drawing.Size(134, 22);
            this.chkExcludeTaskbar.TabIndex = 4;
            this.chkExcludeTaskbar.Text = "Exclude Taskbar";
            this.chkExcludeTaskbar.UseVisualStyleBackColor = true;
            // 
            // chkUseLetterbox
            // 
            this.chkUseLetterbox.AutoSize = true;
            this.chkUseLetterbox.Location = new System.Drawing.Point(21, 175);
            this.chkUseLetterbox.Name = "chkUseLetterbox";
            this.chkUseLetterbox.Size = new System.Drawing.Size(203, 22);
            this.chkUseLetterbox.TabIndex = 5;
            this.chkUseLetterbox.Text = "Use Letterbox (DWM only)";
            this.chkUseLetterbox.UseVisualStyleBackColor = true;
            // 
            // btnLetterboxColor
            // 
            this.btnLetterboxColor.BackColor = System.Drawing.Color.Black;
            this.btnLetterboxColor.Location = new System.Drawing.Point(134, 206);
            this.btnLetterboxColor.Name = "btnLetterboxColor";
            this.btnLetterboxColor.Size = new System.Drawing.Size(48, 48);
            this.btnLetterboxColor.TabIndex = 7;
            this.btnLetterboxColor.UseVisualStyleBackColor = false;
            // 
            // miFileSep1
            // 
            miFileSep1.Index = 4;
            miFileSep1.Text = "-";
            // 
            // miExit
            // 
            this.miExit.Index = 5;
            this.miExit.Text = "E&xit";
            this.miExit.Click += new System.EventHandler(this.miExit_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(96F, 96F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
            this.BackColor = System.Drawing.SystemColors.Window;
            this.ClientSize = new System.Drawing.Size(330, 282);
            this.Controls.Add(this.btnLetterboxColor);
            this.Controls.Add(label1);
            this.Controls.Add(this.chkUseLetterbox);
            this.Controls.Add(this.chkExcludeTaskbar);
            this.Controls.Add(groupBox2);
            this.Controls.Add(this.chkUseDwmFormula);
            this.Font = new System.Drawing.Font("Tahoma", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.MaximizeBox = false;
            this.Menu = this.menu1;
            this.Name = "MainForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "noborder setup";
            groupBox2.ResumeLayout(false);
            groupBox2.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MainMenu menu1;
        private System.Windows.Forms.MenuItem miNewFile;
        private System.Windows.Forms.MenuItem miOpenFile;
        private System.Windows.Forms.MenuItem miSaveFile;
        private System.Windows.Forms.MenuItem miSaveFileAs;
        private System.Windows.Forms.MenuItem miAbout;
        private System.Windows.Forms.CheckBox chkUseDwmFormula;
        private System.Windows.Forms.RadioButton rdoNeverOnTop;
        private System.Windows.Forms.RadioButton rdoAutoOnTop;
        private System.Windows.Forms.RadioButton rdoAlwaysOnTop;
        private System.Windows.Forms.CheckBox chkExcludeTaskbar;
        private System.Windows.Forms.CheckBox chkUseLetterbox;
        private System.Windows.Forms.Button btnLetterboxColor;
        private System.Windows.Forms.MenuItem miHelp;
        private System.Windows.Forms.MenuItem miExit;
    }
}

