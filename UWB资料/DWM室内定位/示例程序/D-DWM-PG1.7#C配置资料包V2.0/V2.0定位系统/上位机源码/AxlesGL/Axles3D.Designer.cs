﻿namespace AxlesGL
{
	partial class Axles3D
	{
		/// <summary> 
		/// 必需的设计器变量。
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary> 
		/// 清理所有正在使用的资源。
		/// </summary>
		/// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region 组件设计器生成的代码

		/// <summary> 
		/// 设计器支持所需的方法 - 不要
		/// 使用代码编辑器修改此方法的内容。
		/// </summary>
		private void InitializeComponent()
		{
            this.components = new System.ComponentModel.Container();
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.SuspendLayout();
            // 
            // timer1
            // 
            this.timer1.Enabled = true;
            this.timer1.Interval = 50;
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // Axles3D
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Name = "Axles3D";
            this.Load += new System.EventHandler(this.Axles3D_Load);
            this.Paint += new System.Windows.Forms.PaintEventHandler(this.Axles3D_Paint);
            this.MouseDown += new System.Windows.Forms.MouseEventHandler(this.Axles3D_MouseDown);
            this.MouseMove += new System.Windows.Forms.MouseEventHandler(this.Axles3D_MouseMove);
            this.Resize += new System.EventHandler(this.Axles3D_Resize);
            this.ResumeLayout(false);

		}

		#endregion

        private System.Windows.Forms.Timer timer1;





	}
}
