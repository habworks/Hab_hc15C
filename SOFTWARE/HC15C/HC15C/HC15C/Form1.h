/****************************************************************************************************
 * File name:    	FORM1.H
 * File type:		Visual C++ Windows Form CLI
 * Description:  	This is the main form of the HC15C Win App.  The prupose of this program is to act
 * as the Win App interface to the HC15C (Hab's Super Calculator).  The Win App shows the Volts and Ohms reading.
 * It also allows the calculator date and time to be set.  The HC15C connnects via a virtual COM port.
 * It would be unwise to try and understand this software without first reviewing the HC15C firmware as the two work hand in hand.
 * 
 * Author:			Hab S. Collector
 * Date of Orgin:	12/12/11
 * Last Edited By:  Hab S. Collector
 * Last Edit Date:  7/07/12
 * REV:             1.0
 * HC15C FIRMWARE:  REV 1.0 OR NEWER
 * Hardware:		Windows PC running Win7 OS
 * Development:		Visual Studion VC++ 2010 Express
 * Notes:           This is a Win32 Application.
 *					It will be necessary to consult the reference documents, associated HC15C firmware and perhaps the schematic
 *                  to understand the operations of this firmware.  
****************************************************************************************************/ 



#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <WinBase.h>
#include "SUPPORT.h"

// GLOBALS
// USED IN FILE LOAD AND FORM LOAD
// HAB FIX - THIS WOULD BE NICE IN A STRUCTURE
bool BpsRateFlag = FALSE;
bool PortNameFlag = FALSE;
int HourBias = 0,
	ErrPacketCount = 0;

// USED WITH TEXT BOX SCROLLING
int OhmLinearBufferCount = 0,
	VoltLinearBufferCount = 0;
bool OhmTextBoxEmpty = TRUE,
	 VoltTextBoxEmpty = TRUE;



namespace HC15C {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

    // FUNCTION PROTOTYPES USED HERE
	bool runShell(String ^, String ^);

	// HAB ADD STRUCTS AND UNIONS FOR USE IN THIS FORM
	typedef union
	{
		double DoubleValue;
		unsigned char ByteValue[sizeof(double)];
    } Union_DoubleInBytes;
	Union_DoubleInBytes DoubleInBytes;

	typedef struct
	{
		double TypeValue;
		unsigned char MsgType;
		bool ParseOK;
	} Type_Parse;


	/// <summary>
	/// Summary for Form1
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{


/**********************************************************************
 * CODE ADDED BY HAB
 * DELEGATE PROTOTYPES
 * NECESSARY TO AVOID CROSS THREAD EXCEPTIONS
 *********************************************************************/
	delegate void SetTextDelegate(String^ text);
	delegate void SetBarDelegate(int Value);
	delegate void SetBarMinMaxDelegate(int Value, bool HighLow);
	delegate void SetClockSetActiveDelegate(bool SetEnable);

	public:
		Form1(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Label^  lbl_VoltMeter;
	protected: 

	protected: 

	protected: 
	private: System::IO::Ports::SerialPort^  serialPort1;
	private: System::Windows::Forms::ProgressBar^  progressBarV;

	private: System::Windows::Forms::Label^  lbl_Low;
	private: System::Windows::Forms::Label^  lbl_Med;
	private: System::Windows::Forms::Label^  lbl_Max;
	private: System::Windows::Forms::TabControl^  tabControl1;




	private: System::Windows::Forms::TabPage^  tabPage1;
	private: System::Windows::Forms::TabPage^  tabPage2;
	private: System::Windows::Forms::TabPage^  lbl_Setup;

	private: System::Windows::Forms::Button^  btn_CommClose;
	private: System::Windows::Forms::Button^  btn_CommOpen;
	private: System::Windows::Forms::ComboBox^  cbx_Port;
	private: System::Windows::Forms::ComboBox^  cbx_BPS;
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::Label^  label3;
	private: System::Windows::Forms::Label^  lbl_ErrorPackets;
	private: System::Windows::Forms::Button^  btn_Test;
	private: System::Windows::Forms::TextBox^  txb_Vmeter;
	private: System::Windows::Forms::Label^  lbl_Active;

	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::Label^  label5;
	private: System::Windows::Forms::TextBox^  tbx_OhmMeter;
	private: System::Windows::Forms::Label^  lbl_OhmMeter;
	private: System::Windows::Forms::Label^  label6;
	private: System::Windows::Forms::Label^  label4;
	private: System::Windows::Forms::Label^  lbl_Active2;
	private: System::Windows::Forms::Button^  btn_SetTime;
	private: System::Windows::Forms::Label^  label7;
	private: System::Windows::Forms::MenuStrip^  menuStrip1;
	private: System::Windows::Forms::ToolStripMenuItem^  fileToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  exitToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  helpToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  revisionToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  userManualToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  softwareDDToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  howToConnectToolStripMenuItem;


	private: System::ComponentModel::IContainer^  components;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


/**********************************************************************
 * CODE ADDED BY HAB
 * FUNCTION: SetTextOmeter
 * NECESSARY TO AVOID CROSS THREAD EXCEPTIONS ON CALLS OF tbx_OhmMeter
 * FROM serialPort1_DataReceived
 *********************************************************************/
	private:void SetTextOmeter(String^ text)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
			if (this->txb_Vmeter->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::SetTextOmeter);
				this->Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				this->tbx_OhmMeter->Text += text;
				OhmLinearBufferCount++;
				if (OhmLinearBufferCount == LINEAR_TEXTBOX_BUFFER_SIZE)
				{
					PushText_FIFO(TEXTBOX_EOL_CHAR, OHM);
					OhmLinearBufferCount = 0;
				}
				// FORCE TEXT TO BE SCROLLED TO THE END OF THE DISPLAY
				this->tbx_OhmMeter->SelectionStart = this->tbx_OhmMeter->Text->Length;
				this->tbx_OhmMeter->ScrollToCaret();
			}
	
        }


/**********************************************************************
 * CODE ADDED BY HAB
 * FUNCTION: SetTextO
 * NECESSARY TO AVOID CROSS THREAD EXCEPTIONS ON CALLS OF lbl_OhmMeter
 * FROM serialPort1_DataReceived
 *********************************************************************/
	private:void SetTextO(String^ text)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
			if (this->lbl_VoltMeter->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::SetTextO);
				this->Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				this->lbl_OhmMeter->Text = text;
			}
	
        }


/**********************************************************************
 * CODE ADDED BY HAB
 * FUNCTION: SetActiveLabel
 * NECESSARY TO AVOID CROSS THREAD EXCEPTIONS ON CALLS OF lbl_Active
 * FROM serialPort1_DataReceived
 *********************************************************************/
	private:void SetActiveLabel2(String^ text)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
			if (this->lbl_Max->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::SetActiveLabel2);
				this->Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				this->lbl_Active2->Text = text;
			}
	
        }


/**********************************************************************
 * CODE ADDED BY HAB
 * FUNCTION: SetTextVmeter
 * NECESSARY TO AVOID CROSS THREAD EXCEPTIONS ON CALLS OF tbx_Vmeter
 * FROM serialPort1_DataReceived
 *********************************************************************/
	private:void SetTextVmeter(String^ text)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
			if (this->txb_Vmeter->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::SetTextVmeter);
				this->Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				this->txb_Vmeter->Text += text;
				VoltLinearBufferCount++;
				if (VoltLinearBufferCount == LINEAR_TEXTBOX_BUFFER_SIZE)
				{
					PushText_FIFO(TEXTBOX_EOL_CHAR, VOLT);
					OhmLinearBufferCount = 0;
				}
				// FORCE TEXT TO BE SCROLLED TO THE END OF THE DISPLAY
				this->txb_Vmeter->SelectionStart = this->txb_Vmeter->Text->Length;
				this->txb_Vmeter->ScrollToCaret();
			}
	
        }


/**********************************************************************
 * CODE ADDED BY HAB
 * FUNCTION: SetText
 * NECESSARY TO AVOID CROSS THREAD EXCEPTIONS ON CALLS OF lbl_VoltMeter
 * FROM serialPort1_DataReceived
 *********************************************************************/
	private:void SetText(String^ text)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
			if (this->lbl_VoltMeter->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::SetText);
				this->Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				this->lbl_VoltMeter->Text = text;
			}
	
        }


/**********************************************************************
 * CODE ADDED BY HAB
 * FUNCTION: SetLabelLow
 * NECESSARY TO AVOID CROSS THREAD EXCEPTIONS ON CALLS OF lbl_Low
 * FROM serialPort1_DataReceived
 *********************************************************************/
	private:void SetLabelLow(String^ text)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
			if (this->lbl_Low->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::SetLabelLow);
				this->Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				this->lbl_Low->Text = text;
			}
	
        }


/**********************************************************************
 * CODE ADDED BY HAB
 * FUNCTION: SetLabelMed
 * NECESSARY TO AVOID CROSS THREAD EXCEPTIONS ON CALLS OF lbl_Med
 * FROM serialPort1_DataReceived
 *********************************************************************/
	private:void SetLabelMed(String^ text)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
			if (this->lbl_Med->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::SetLabelMed);
				this->Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				this->lbl_Med->Text = text;
			}
	
        }


/**********************************************************************
 * CODE ADDED BY HAB
 * FUNCTION: SetLabelMax
 * NECESSARY TO AVOID CROSS THREAD EXCEPTIONS ON CALLS OF lbl_Max
 * FROM serialPort1_DataReceived
 *********************************************************************/
	private:void SetLabelMax(String^ text)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
			if (this->lbl_Max->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::SetLabelMax);
				this->Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				this->lbl_Max->Text = text;
			}
	
        }


/**********************************************************************
 * CODE ADDED BY HAB
 * FUNCTION: SetActiveLabel
 * NECESSARY TO AVOID CROSS THREAD EXCEPTIONS ON CALLS OF lbl_Active
 * FROM serialPort1_DataReceived
 *********************************************************************/
	private:void SetActiveLabel(String^ text)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
			if (this->lbl_Max->InvokeRequired)
			{
				SetTextDelegate^ d = 
					gcnew SetTextDelegate(this, &Form1::SetActiveLabel);
				this->Invoke(d, gcnew array<Object^> { text });
			}
			else
			{
				this->lbl_Active->Text = text;
			}
	
        }
			

/**********************************************************************
 * CODE ADDED BY HAB
 * FUNCTION: SetBar
 * NECESSARY TO AVOID CROSS THREAD EXCEPTIONS ON CALLS OF progressBarV
 * FROM serialPort1_DataReceived
 *********************************************************************/
	private:void SetBar(int Value)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
			if (this->lbl_VoltMeter->InvokeRequired)
			{
				SetBarDelegate^ d = 
					gcnew SetBarDelegate(this, &Form1::SetBar);
				this->Invoke(d, gcnew array<Object^> { Value });
			}
			else
			{
				this->progressBarV->Value = Value;
			}
	
        }


/**********************************************************************
 * CODE ADDED BY HAB
 * FUNCTION: SetBarMinMax
 * NECESSARY TO AVOID CROSS THREAD EXCEPTIONS ON CALLS OF progressBarV
 * FROM serialPort1_DataReceived
 *********************************************************************/
	private:void SetBarMinMax(int Value, bool HighLow)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
			if (this->progressBarV->InvokeRequired)
			{
				SetBarMinMaxDelegate^ d = 
					gcnew SetBarMinMaxDelegate(this, &Form1::SetBarMinMax);
				this->Invoke(d, gcnew array<Object^> { Value, HighLow });
			}
			else
			{
				if (HighLow)
					this->progressBarV->Maximum = Value;
				else
					this->progressBarV->Minimum = Value;
			}
	
        }


/**********************************************************************
 * CODE ADDED BY HAB
 * FUNCTION: SetClockSetActive
 * NECESSARY TO AVOID CROSS THREAD EXCEPTIONS ON CALLS OF lbl_Active
 * FROM serialPort1_DataReceived
 *********************************************************************/
	private:void SetClockSetActive(bool SetEnable)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
			if (this->btn_SetTime->InvokeRequired)
			{
				SetClockSetActiveDelegate^ d = 
					gcnew SetClockSetActiveDelegate(this, &Form1::SetClockSetActive);
				this->Invoke(d, gcnew array<Object^> { SetEnable });
			}
			else
			{
				if (SetEnable)
					this->btn_SetTime->Enabled = TRUE;
				else
					this->btn_SetTime->Enabled = FALSE;
			}
	
        }
/***********************END OF CODE ADDED BY HAB************************/


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(Form1::typeid));
			this->lbl_VoltMeter = (gcnew System::Windows::Forms::Label());
			this->serialPort1 = (gcnew System::IO::Ports::SerialPort(this->components));
			this->progressBarV = (gcnew System::Windows::Forms::ProgressBar());
			this->lbl_Low = (gcnew System::Windows::Forms::Label());
			this->lbl_Med = (gcnew System::Windows::Forms::Label());
			this->lbl_Max = (gcnew System::Windows::Forms::Label());
			this->tabControl1 = (gcnew System::Windows::Forms::TabControl());
			this->tabPage1 = (gcnew System::Windows::Forms::TabPage());
			this->label6 = (gcnew System::Windows::Forms::Label());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->lbl_Active = (gcnew System::Windows::Forms::Label());
			this->txb_Vmeter = (gcnew System::Windows::Forms::TextBox());
			this->btn_Test = (gcnew System::Windows::Forms::Button());
			this->tabPage2 = (gcnew System::Windows::Forms::TabPage());
			this->lbl_Active2 = (gcnew System::Windows::Forms::Label());
			this->label5 = (gcnew System::Windows::Forms::Label());
			this->tbx_OhmMeter = (gcnew System::Windows::Forms::TextBox());
			this->lbl_OhmMeter = (gcnew System::Windows::Forms::Label());
			this->lbl_Setup = (gcnew System::Windows::Forms::TabPage());
			this->btn_SetTime = (gcnew System::Windows::Forms::Button());
			this->label7 = (gcnew System::Windows::Forms::Label());
			this->lbl_ErrorPackets = (gcnew System::Windows::Forms::Label());
			this->btn_CommClose = (gcnew System::Windows::Forms::Button());
			this->btn_CommOpen = (gcnew System::Windows::Forms::Button());
			this->cbx_Port = (gcnew System::Windows::Forms::ComboBox());
			this->cbx_BPS = (gcnew System::Windows::Forms::ComboBox());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->menuStrip1 = (gcnew System::Windows::Forms::MenuStrip());
			this->fileToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->exitToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->helpToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->revisionToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->userManualToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->softwareDDToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->howToConnectToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->tabControl1->SuspendLayout();
			this->tabPage1->SuspendLayout();
			this->tabPage2->SuspendLayout();
			this->lbl_Setup->SuspendLayout();
			this->menuStrip1->SuspendLayout();
			this->SuspendLayout();
			// 
			// lbl_VoltMeter
			// 
			this->lbl_VoltMeter->BackColor = System::Drawing::Color::PowderBlue;
			this->lbl_VoltMeter->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->lbl_VoltMeter->Font = (gcnew System::Drawing::Font(L"HP15C Simulator Font", 71.99999F, System::Drawing::FontStyle::Regular, 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->lbl_VoltMeter->Location = System::Drawing::Point(7, 28);
			this->lbl_VoltMeter->Name = L"lbl_VoltMeter";
			this->lbl_VoltMeter->Size = System::Drawing::Size(354, 157);
			this->lbl_VoltMeter->TabIndex = 0;
			this->lbl_VoltMeter->Text = L"00.000";
			this->lbl_VoltMeter->TextAlign = System::Drawing::ContentAlignment::TopRight;
			// 
			// progressBarV
			// 
			this->progressBarV->Location = System::Drawing::Point(20, 149);
			this->progressBarV->Maximum = 10000;
			this->progressBarV->Name = L"progressBarV";
			this->progressBarV->Size = System::Drawing::Size(323, 36);
			this->progressBarV->TabIndex = 1;
			// 
			// lbl_Low
			// 
			this->lbl_Low->AutoSize = true;
			this->lbl_Low->BackColor = System::Drawing::Color::PowderBlue;
			this->lbl_Low->Font = (gcnew System::Drawing::Font(L"HP15C Simulator Font", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->lbl_Low->Location = System::Drawing::Point(17, 130);
			this->lbl_Low->Name = L"lbl_Low";
			this->lbl_Low->Size = System::Drawing::Size(29, 16);
			this->lbl_Low->TabIndex = 2;
			this->lbl_Low->Text = L"0.0";
			// 
			// lbl_Med
			// 
			this->lbl_Med->AutoSize = true;
			this->lbl_Med->BackColor = System::Drawing::Color::PowderBlue;
			this->lbl_Med->Font = (gcnew System::Drawing::Font(L"HP15C Simulator Font", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->lbl_Med->Location = System::Drawing::Point(162, 130);
			this->lbl_Med->Name = L"lbl_Med";
			this->lbl_Med->Size = System::Drawing::Size(29, 16);
			this->lbl_Med->TabIndex = 3;
			this->lbl_Med->Text = L"5.0";
			// 
			// lbl_Max
			// 
			this->lbl_Max->AutoSize = true;
			this->lbl_Max->BackColor = System::Drawing::Color::PowderBlue;
			this->lbl_Max->Font = (gcnew System::Drawing::Font(L"HP15C Simulator Font", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->lbl_Max->Location = System::Drawing::Point(305, 130);
			this->lbl_Max->Name = L"lbl_Max";
			this->lbl_Max->Size = System::Drawing::Size(38, 16);
			this->lbl_Max->TabIndex = 4;
			this->lbl_Max->Text = L"10.0";
			// 
			// tabControl1
			// 
			this->tabControl1->Controls->Add(this->tabPage1);
			this->tabControl1->Controls->Add(this->tabPage2);
			this->tabControl1->Controls->Add(this->lbl_Setup);
			this->tabControl1->Location = System::Drawing::Point(2, 28);
			this->tabControl1->Name = L"tabControl1";
			this->tabControl1->SelectedIndex = 0;
			this->tabControl1->Size = System::Drawing::Size(375, 324);
			this->tabControl1->TabIndex = 5;
			// 
			// tabPage1
			// 
			this->tabPage1->BackColor = System::Drawing::Color::SteelBlue;
			this->tabPage1->Controls->Add(this->label6);
			this->tabPage1->Controls->Add(this->label4);
			this->tabPage1->Controls->Add(this->label1);
			this->tabPage1->Controls->Add(this->lbl_Active);
			this->tabPage1->Controls->Add(this->txb_Vmeter);
			this->tabPage1->Controls->Add(this->lbl_Low);
			this->tabPage1->Controls->Add(this->lbl_Med);
			this->tabPage1->Controls->Add(this->lbl_Max);
			this->tabPage1->Controls->Add(this->progressBarV);
			this->tabPage1->Controls->Add(this->lbl_VoltMeter);
			this->tabPage1->Controls->Add(this->btn_Test);
			this->tabPage1->Location = System::Drawing::Point(4, 22);
			this->tabPage1->Name = L"tabPage1";
			this->tabPage1->Padding = System::Windows::Forms::Padding(3);
			this->tabPage1->Size = System::Drawing::Size(367, 298);
			this->tabPage1->TabIndex = 0;
			this->tabPage1->Text = L"Vmeter";
			// 
			// label6
			// 
			this->label6->AutoSize = true;
			this->label6->BackColor = System::Drawing::Color::PowderBlue;
			this->label6->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label6->Location = System::Drawing::Point(8, 79);
			this->label6->Name = L"label6";
			this->label6->Size = System::Drawing::Size(29, 16);
			this->label6->TabIndex = 10;
			this->label6->Text = L"DC";
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->BackColor = System::Drawing::Color::PowderBlue;
			this->label4->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label4->Location = System::Drawing::Point(8, 95);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(47, 16);
			this->label4->TabIndex = 9;
			this->label4->Text = L"VOLT";
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->BackColor = System::Drawing::Color::PowderBlue;
			this->label1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label1->Location = System::Drawing::Point(334, 95);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(18, 16);
			this->label1->TabIndex = 8;
			this->label1->Text = L"V";
			// 
			// lbl_Active
			// 
			this->lbl_Active->AutoSize = true;
			this->lbl_Active->BackColor = System::Drawing::Color::PowderBlue;
			this->lbl_Active->Font = (gcnew System::Drawing::Font(L"Courier New", 20.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)), 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->lbl_Active->ForeColor = System::Drawing::Color::Red;
			this->lbl_Active->Location = System::Drawing::Point(14, 37);
			this->lbl_Active->Name = L"lbl_Active";
			this->lbl_Active->Size = System::Drawing::Size(30, 31);
			this->lbl_Active->TabIndex = 7;
			this->lbl_Active->Text = L"*";
			// 
			// txb_Vmeter
			// 
			this->txb_Vmeter->BackColor = System::Drawing::Color::LightBlue;
			this->txb_Vmeter->Font = (gcnew System::Drawing::Font(L"Courier New", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->txb_Vmeter->Location = System::Drawing::Point(6, 191);
			this->txb_Vmeter->Multiline = true;
			this->txb_Vmeter->Name = L"txb_Vmeter";
			this->txb_Vmeter->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->txb_Vmeter->Size = System::Drawing::Size(355, 76);
			this->txb_Vmeter->TabIndex = 6;
			// 
			// btn_Test
			// 
			this->btn_Test->Location = System::Drawing::Point(304, 273);
			this->btn_Test->Name = L"btn_Test";
			this->btn_Test->Size = System::Drawing::Size(57, 22);
			this->btn_Test->TabIndex = 5;
			this->btn_Test->Text = L"Test TX";
			this->btn_Test->UseVisualStyleBackColor = true;
			this->btn_Test->Visible = false;
			this->btn_Test->Click += gcnew System::EventHandler(this, &Form1::btn_Test_Click);
			// 
			// tabPage2
			// 
			this->tabPage2->BackColor = System::Drawing::Color::SteelBlue;
			this->tabPage2->Controls->Add(this->lbl_Active2);
			this->tabPage2->Controls->Add(this->label5);
			this->tabPage2->Controls->Add(this->tbx_OhmMeter);
			this->tabPage2->Controls->Add(this->lbl_OhmMeter);
			this->tabPage2->Location = System::Drawing::Point(4, 22);
			this->tabPage2->Name = L"tabPage2";
			this->tabPage2->Padding = System::Windows::Forms::Padding(3);
			this->tabPage2->Size = System::Drawing::Size(367, 298);
			this->tabPage2->TabIndex = 1;
			this->tabPage2->Text = L"Ometer";
			// 
			// lbl_Active2
			// 
			this->lbl_Active2->AutoSize = true;
			this->lbl_Active2->BackColor = System::Drawing::Color::PowderBlue;
			this->lbl_Active2->Font = (gcnew System::Drawing::Font(L"Courier New", 20.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)), 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->lbl_Active2->ForeColor = System::Drawing::Color::Red;
			this->lbl_Active2->Location = System::Drawing::Point(11, 27);
			this->lbl_Active2->Name = L"lbl_Active2";
			this->lbl_Active2->Size = System::Drawing::Size(30, 31);
			this->lbl_Active2->TabIndex = 8;
			this->lbl_Active2->Text = L"*";
			// 
			// label5
			// 
			this->label5->AutoSize = true;
			this->label5->BackColor = System::Drawing::Color::PowderBlue;
			this->label5->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label5->Location = System::Drawing::Point(14, 84);
			this->label5->Name = L"label5";
			this->label5->Size = System::Drawing::Size(43, 13);
			this->label5->TabIndex = 3;
			this->label5->Text = L"OHMS";
			// 
			// tbx_OhmMeter
			// 
			this->tbx_OhmMeter->BackColor = System::Drawing::Color::LightBlue;
			this->tbx_OhmMeter->Font = (gcnew System::Drawing::Font(L"Courier New", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->tbx_OhmMeter->Location = System::Drawing::Point(17, 120);
			this->tbx_OhmMeter->Multiline = true;
			this->tbx_OhmMeter->Name = L"tbx_OhmMeter";
			this->tbx_OhmMeter->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->tbx_OhmMeter->Size = System::Drawing::Size(333, 150);
			this->tbx_OhmMeter->TabIndex = 2;
			// 
			// lbl_OhmMeter
			// 
			this->lbl_OhmMeter->BackColor = System::Drawing::Color::PowderBlue;
			this->lbl_OhmMeter->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->lbl_OhmMeter->Font = (gcnew System::Drawing::Font(L"HP15C Simulator Font", 71.99999F, System::Drawing::FontStyle::Regular, 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->lbl_OhmMeter->Location = System::Drawing::Point(6, 15);
			this->lbl_OhmMeter->Name = L"lbl_OhmMeter";
			this->lbl_OhmMeter->Size = System::Drawing::Size(354, 267);
			this->lbl_OhmMeter->TabIndex = 1;
			this->lbl_OhmMeter->Text = L"00.000";
			this->lbl_OhmMeter->TextAlign = System::Drawing::ContentAlignment::TopRight;
			// 
			// lbl_Setup
			// 
			this->lbl_Setup->BackColor = System::Drawing::SystemColors::Control;
			this->lbl_Setup->Controls->Add(this->btn_SetTime);
			this->lbl_Setup->Controls->Add(this->label7);
			this->lbl_Setup->Controls->Add(this->lbl_ErrorPackets);
			this->lbl_Setup->Controls->Add(this->btn_CommClose);
			this->lbl_Setup->Controls->Add(this->btn_CommOpen);
			this->lbl_Setup->Controls->Add(this->cbx_Port);
			this->lbl_Setup->Controls->Add(this->cbx_BPS);
			this->lbl_Setup->Controls->Add(this->label2);
			this->lbl_Setup->Controls->Add(this->label3);
			this->lbl_Setup->Location = System::Drawing::Point(4, 22);
			this->lbl_Setup->Name = L"lbl_Setup";
			this->lbl_Setup->Padding = System::Windows::Forms::Padding(3);
			this->lbl_Setup->Size = System::Drawing::Size(367, 298);
			this->lbl_Setup->TabIndex = 2;
			this->lbl_Setup->Text = L"Setup";
			// 
			// btn_SetTime
			// 
			this->btn_SetTime->Enabled = false;
			this->btn_SetTime->Location = System::Drawing::Point(6, 236);
			this->btn_SetTime->Name = L"btn_SetTime";
			this->btn_SetTime->Size = System::Drawing::Size(131, 56);
			this->btn_SetTime->TabIndex = 32;
			this->btn_SetTime->Text = L"HC15C SET TIME";
			this->btn_SetTime->UseVisualStyleBackColor = true;
			this->btn_SetTime->Click += gcnew System::EventHandler(this, &Form1::btn_SetTime_Click);
			// 
			// label7
			// 
			this->label7->AutoSize = true;
			this->label7->Location = System::Drawing::Point(232, 20);
			this->label7->Name = L"label7";
			this->label7->Size = System::Drawing::Size(74, 13);
			this->label7->TabIndex = 31;
			this->label7->Text = L"Error Packets:";
			// 
			// lbl_ErrorPackets
			// 
			this->lbl_ErrorPackets->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->lbl_ErrorPackets->ForeColor = System::Drawing::Color::Red;
			this->lbl_ErrorPackets->Location = System::Drawing::Point(312, 20);
			this->lbl_ErrorPackets->Name = L"lbl_ErrorPackets";
			this->lbl_ErrorPackets->Size = System::Drawing::Size(33, 15);
			this->lbl_ErrorPackets->TabIndex = 30;
			this->lbl_ErrorPackets->Text = L"0000";
			// 
			// btn_CommClose
			// 
			this->btn_CommClose->Enabled = false;
			this->btn_CommClose->Location = System::Drawing::Point(7, 82);
			this->btn_CommClose->Name = L"btn_CommClose";
			this->btn_CommClose->Size = System::Drawing::Size(131, 56);
			this->btn_CommClose->TabIndex = 26;
			this->btn_CommClose->Text = L"DISCONNECT";
			this->btn_CommClose->UseVisualStyleBackColor = true;
			this->btn_CommClose->Click += gcnew System::EventHandler(this, &Form1::btn_CommClose_Click);
			// 
			// btn_CommOpen
			// 
			this->btn_CommOpen->Enabled = false;
			this->btn_CommOpen->Location = System::Drawing::Point(7, 20);
			this->btn_CommOpen->Name = L"btn_CommOpen";
			this->btn_CommOpen->Size = System::Drawing::Size(131, 56);
			this->btn_CommOpen->TabIndex = 24;
			this->btn_CommOpen->Text = L"CONNECT";
			this->btn_CommOpen->UseVisualStyleBackColor = true;
			this->btn_CommOpen->Click += gcnew System::EventHandler(this, &Form1::btn_CommOpen_Click);
			// 
			// cbx_Port
			// 
			this->cbx_Port->FormattingEnabled = true;
			this->cbx_Port->Items->AddRange(gcnew cli::array< System::Object^  >(8) {L"COM1", L"COM2", L"COM3", L"COM4", L"COM5", L"COM6", 
				L"COM7", L"COM8"});
			this->cbx_Port->Location = System::Drawing::Point(216, 117);
			this->cbx_Port->Name = L"cbx_Port";
			this->cbx_Port->Size = System::Drawing::Size(129, 21);
			this->cbx_Port->TabIndex = 27;
			this->cbx_Port->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cbx_Port_SelectedIndexChanged);
			// 
			// cbx_BPS
			// 
			this->cbx_BPS->FormattingEnabled = true;
			this->cbx_BPS->Items->AddRange(gcnew cli::array< System::Object^  >(9) {L"1200", L"2400", L"4800", L"9600", L"14400", L"19200", 
				L"38400", L"57600", L"115200"});
			this->cbx_BPS->Location = System::Drawing::Point(216, 55);
			this->cbx_BPS->Name = L"cbx_BPS";
			this->cbx_BPS->Size = System::Drawing::Size(129, 21);
			this->cbx_BPS->TabIndex = 25;
			this->cbx_BPS->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cbx_BPS_SelectedIndexChanged);
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label2->Location = System::Drawing::Point(165, 56);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(45, 20);
			this->label2->TabIndex = 28;
			this->label2->Text = L"BPS:";
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label3->Location = System::Drawing::Point(148, 118);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(62, 20);
			this->label3->TabIndex = 29;
			this->label3->Text = L"COMM:";
			// 
			// menuStrip1
			// 
			this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->fileToolStripMenuItem, 
				this->helpToolStripMenuItem});
			this->menuStrip1->Location = System::Drawing::Point(0, 0);
			this->menuStrip1->Name = L"menuStrip1";
			this->menuStrip1->Size = System::Drawing::Size(378, 24);
			this->menuStrip1->TabIndex = 6;
			this->menuStrip1->Text = L"menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			this->fileToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->exitToolStripMenuItem});
			this->fileToolStripMenuItem->Name = L"fileToolStripMenuItem";
			this->fileToolStripMenuItem->Size = System::Drawing::Size(37, 20);
			this->fileToolStripMenuItem->Text = L"File";
			// 
			// exitToolStripMenuItem
			// 
			this->exitToolStripMenuItem->Name = L"exitToolStripMenuItem";
			this->exitToolStripMenuItem->Size = System::Drawing::Size(92, 22);
			this->exitToolStripMenuItem->Text = L"Exit";
			this->exitToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::exitToolStripMenuItem_Click);
			// 
			// helpToolStripMenuItem
			// 
			this->helpToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {this->revisionToolStripMenuItem, 
				this->userManualToolStripMenuItem, this->softwareDDToolStripMenuItem, this->howToConnectToolStripMenuItem});
			this->helpToolStripMenuItem->Name = L"helpToolStripMenuItem";
			this->helpToolStripMenuItem->Size = System::Drawing::Size(44, 20);
			this->helpToolStripMenuItem->Text = L"Help";
			// 
			// revisionToolStripMenuItem
			// 
			this->revisionToolStripMenuItem->Name = L"revisionToolStripMenuItem";
			this->revisionToolStripMenuItem->Size = System::Drawing::Size(161, 22);
			this->revisionToolStripMenuItem->Text = L"Revision";
			this->revisionToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::revisionToolStripMenuItem_Click);
			// 
			// userManualToolStripMenuItem
			// 
			this->userManualToolStripMenuItem->Name = L"userManualToolStripMenuItem";
			this->userManualToolStripMenuItem->Size = System::Drawing::Size(161, 22);
			this->userManualToolStripMenuItem->Text = L"User Manual";
			this->userManualToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::userManualToolStripMenuItem_Click);
			// 
			// softwareDDToolStripMenuItem
			// 
			this->softwareDDToolStripMenuItem->Name = L"softwareDDToolStripMenuItem";
			this->softwareDDToolStripMenuItem->Size = System::Drawing::Size(161, 22);
			this->softwareDDToolStripMenuItem->Text = L"Software DD";
			this->softwareDDToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::softwareDDToolStripMenuItem_Click);
			// 
			// howToConnectToolStripMenuItem
			// 
			this->howToConnectToolStripMenuItem->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"howToConnectToolStripMenuItem.Image")));
			this->howToConnectToolStripMenuItem->Name = L"howToConnectToolStripMenuItem";
			this->howToConnectToolStripMenuItem->Size = System::Drawing::Size(161, 22);
			this->howToConnectToolStripMenuItem->Text = L"How to Connect";
			this->howToConnectToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::howToConnectToolStripMenuItem_Click);
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(378, 355);
			this->Controls->Add(this->tabControl1);
			this->Controls->Add(this->menuStrip1);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::Fixed3D;
			this->MaximizeBox = false;
			this->Name = L"Form1";
			this->Text = L"hc 15C";
			this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
			this->tabControl1->ResumeLayout(false);
			this->tabPage1->ResumeLayout(false);
			this->tabPage1->PerformLayout();
			this->tabPage2->ResumeLayout(false);
			this->tabPage2->PerformLayout();
			this->lbl_Setup->ResumeLayout(false);
			this->lbl_Setup->PerformLayout();
			this->menuStrip1->ResumeLayout(false);
			this->menuStrip1->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

/************************************************************************************************************************/
/**************************************************LOAD FORM EVENT*******************************************************/
/************************************************************************************************************************/
private: System::Void Form1_Load(System::Object^  sender, System::EventArgs^  e) {

//		TIME_ZONE_INFORMATION TimeZoneInfo;

		// STEP 1
		// SET BPS STARTUP VALUE
		cbx_BPS->SelectedIndex = 5;  // HAB FIX MAGIC NUMBER

		// STEP 2
		// CREATE THE SERIAL PORT DATA RECEIVE EVENT
		this->serialPort1->DataReceived += gcnew System::IO::Ports::SerialDataReceivedEventHandler(this, &Form1::serialPort1_DataReceived);

		// STEP 3
		// SET THE TAB TO SETUP
		this->tabControl1->SelectedTab = lbl_Setup;

		DoubleInBytes.DoubleValue = 0.123;
			 
	}

/************************************************************************************************************************/
/***********************************************SETUP TAB CONTROLS*******************************************************/
/************************************************************************************************************************/

/* BUTTON OPEN COM PORT
 * DESCRIPTION: OPEN THE COM PORT AND ENABLE OTHER COMM TX RELATED FUNCTIONS
 * STEP 1: TRY TO SELECTED COMM PORT CATCH ANY ERRORS AND RETRUN IF ERROR WITH MSG
 * STEP 2: COMM IS OPEN - ENABLE BUTTONS ASSOCIATED WITH COMM TX
 * STEP 3: DISABLE THE COMM OPEN BUTTON - IT IS OPEN ALREADY 
 * STEP 4: CLEAR THE BUFFER WITH A DUMMY READ */
private: System::Void btn_CommOpen_Click(System::Object^  sender, System::EventArgs^  e) {
			 // STEP 1
			 // OPEN COMM PORT
			 try
			 {
				 serialPort1->Open();
			 }

			 // CATCH ANY ERROR
			 catch(Exception ^Error)
			 {
				 MessageBox::Show("Error opening Port: " + serialPort1->PortName + "\r\nDescription: " + Error->Message,		// DISPLAY MSG
				                  "ERROR",					// MSGBOX CAPTION
							      MessageBoxButtons::OK,	// USER RESPONSE
							      MessageBoxIcon::Error);	// ICON TYPE
				 return;
			 }

			 // STEP 2
			 // ENABLE BUTTONS
			 if (BpsRateFlag && PortNameFlag)
			 {
				btn_CommClose->Enabled = TRUE;
			 }
			

			 // STEP 3
			 // DISABLE OPEN
			 btn_CommOpen->Enabled = FALSE;

			 // STEP 4
			 // CLEAR THE BUFFER OF TRASH
			 String^ message = serialPort1->ReadExisting();
		}


/* BUTTON CLOSE COM PORT
 * DESCRIPTION: CLOSE THE COM PORT AND DISABLE OTHER COMM TX RELATED FUNCTIONS
 * STEP 1: CLOSE THE SERIAL PORT
 * STEP 2: ENABLE THE OPEN COMM BUTTON (SO IT CAN BE RE-OPENED)
 * STEP 3: DISABLE COMM BUTTONS ASSOTICED WITH COMM TX */
private: System::Void btn_CommClose_Click(System::Object^  sender, System::EventArgs^  e) {

			// STEP 1
			try
			{
				serialPort1->Close();
			}

			// CATCH ANY ERROR
			 catch(Exception ^Error)
			 {
				 MessageBox::Show("Port may already be disconnected: " + serialPort1->PortName + "\r\nDescription: " + Error->Message,		// DISPLAY MSG
				                  "Alert",					// MSGBOX CAPTION
							      MessageBoxButtons::OK,	// USER RESPONSE
							      MessageBoxIcon::Information);	// ICON TYPE
				 //return;
			 }
			
			// STEP 2
			// ENABLE TO RE-OPEN
			btn_CommOpen->Enabled = TRUE;

			// STEP 3
			// DISABLE TO AVOID UNNECESSARY ERRORS
			btn_CommClose->Enabled = FALSE;

		 }


/* BUTTON SET HC15C TIME
 * DESCRIPTION: TRANSMITS A MESSAGE THAT CAUSES THE HC15C TO SET ITS TIME.  THE MSG LOOKS LIKE
 * XXXX:XXXX:XXXX:XXXX:XXXX:XXXX\R = MONTH:DATE:YEAR:HOUR:MINUTE:SECOND (30 BYTES). FOR EXAMPLE THE STRING
 * 0007:0004:2012:0022:0010:0015 IS A DATE OF 7/4/2012, 10:10:15PM - THE MSG IS TRANSMITTED WITH
 * A CR AT THE END OF THE SEQUENCE. THE MSG MUST BE DONE THIS WAY TO WORK WITH THE USB VCOM LIB 
 * VCOM_gets FUNCTION.  NOTE THIS BUTTON IS ENABLE BY A MSG PACKET FROM THE HC15C (TYPE_TIME)
 * STEP 1: CREATE MANAGED ARRAY TO USE WITH SERIAL PORT TRANSMISSION
 * STEP 2: CREATE A STRING: LOCAL TIME, IN THE NEEDED FORMAT
 * STEP 3: COPY STRING TO MANAGED ARRAY
 * STEP 4: TRANSMIT THE ARRAY AND SELF DISABLE*/
private: System::Void btn_SetTime_Click(System::Object^  sender, System::EventArgs^  e) {

			 SYSTEMTIME LocalTime;
			 String^ TimeString = "";
			 int SizeOfPayLoad = 30; // NUMBER OF CHARS IN XXXX:XXXX:XXXX:XXXX:XXXX:XXXX\R

			 // STEP 1
			 // CREATE A MANAGED ARRAY TX BUFFER FOR SERIAL PORT CONTROL AND LOAD IT WITH TX BUFFER
	         array<unsigned char>^ TxBuffer = gcnew array<unsigned char>(SizeOfPayLoad);

			 // STEP 2
			 // GET THE PRESENT TIME
			 GetLocalTime(&LocalTime);
			 // FORMAT THE TIME STRING
			 TimeString =  System::String::Format("{0:d4}",LocalTime.wMonth) + ":"
						   + System::String::Format("{0:d4}",LocalTime.wDay) + ":"
						   + System::String::Format("{0:d4}",LocalTime.wYear) + ":"
				           + System::String::Format("{0:d4}",LocalTime.wHour) + ":"
						   + System::String::Format("{0:d4}",LocalTime.wMinute) + ":"
						   + System::String::Format("{0:d4}",LocalTime.wSecond) + "\r";

			 // STEP 3
			 // COPY STRING TO ARRAY
			 for (int TimeCharCount = 0; TimeCharCount < SizeOfPayLoad; TimeCharCount++)
			 {
				 TxBuffer[TimeCharCount] = (unsigned char)TimeString[TimeCharCount];
			 }

			 // STEP 4
			 // TRANSMIT THE TIME PAY LOAD
			 serialPort1->Write(TxBuffer, 0, SizeOfPayLoad);
			 btn_SetTime->Enabled = FALSE;
			 
		 }



/************************************************************************************************************************/
/*****************************************************MENU ITEMS*********************************************************/
/************************************************************************************************************************/
/* MENU HELP REVISION
 * DESCRIPTION: Ends the program
 * STEP 1: End program by closing form */
private: System::Void revisionToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
	MessageBox::Show(FORM_NAME + "\r\nAUTHOR: Hab Collector\r\nLAST EDIT: HSC" + "\r\nREV: " + REV_MAJOR + "." + REV_MINOR, // DISPLAY MSG
				                 "SOFTWARE REVISION",			// MSGBOX CAPTION
								 MessageBoxButtons::OK,			// USER RESPONSE
								 MessageBoxIcon::Information);	// ICON TYPE
		 }



/* MENU HELP SOFTWARE DD 
 * DESCRIPTION: Shows the Software DD in a shell application
 * STEP 1: Launch Software DD */
private: System::Void softwareDDToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {

			 String ^Target,
				    ^TargetAppType;

			 // STEP 1
			 Target = SOFTWARE_DD_FILE_NAME;
			 TargetAppType = DOCUMENT_SHELL_NAME;
			 runShell(Target, TargetAppType);
		 }



/* MENU HELP USER MANUAL 
 * DESCRIPTION: Shows the Software DD in a shell application
 * STEP 1: Launch Software DD */
private: System::Void userManualToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
			 
			String ^Target,
				    ^TargetAppType;

			 // STEP 1
			 Target = USER_MANUAL_FILE_NAME;
			 TargetAppType = DOCUMENT_SHELL_NAME;
			 runShell(Target, TargetAppType);
		 }


/* MENU HELP HOW TO VIDEO 
 * DESCRIPTION: Shows the Software DD in a shell application
 * STEP 1: Launch Software DD */
private: System::Void howToConnectToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {

			 String ^Target,
				    ^TargetAppType;

			 // STEP 1
			 Target = VIDEO_HELP_FILE_NAME;
			 TargetAppType = VIDEO_HELP_SHELL_NAME;
			 runShell(Target, TargetAppType);
		 }



/* MENU FILE EXIT
 * DESCRIPTION: Ends the program
 * STEP 1: End program by closing form */
private: System::Void exitToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {

			 this->Close();
		 }


/************************************************************************************************************************/
/********************************************FORM GLOBAL FUNCTIONS*******************************************************/
/************************************************************************************************************************/

/*************************************************************************
 * Function Name: SerialDataReceivedEventArgs
 * Description: Capture valid packets of NGLF protocol.  A valid packet starts with SYN, then has
 * a valid TYPE byte (7 possible).  The lenght (LEN) follows next and indicates the lenght
 * of the payload in bytes.  The last byte of the packet is the CS (Check Sum CRC-8).  Because Windows
 * data receive event can IRQ before the entire packet is recieved the code must take account for 
 * not all of the packet bytes being read in a single read COMM stream.  A packet might span several serial
 * port reads.  The code also takes into account that there maybe a combination of packets (end of first packet
 * start of second packet) in a single read and be able to process all bytes.  The code valides the byte by checking
 * the TYPE is valid, and the check sum is valid.  There is no timeout called out at this time (FYI neither
 * is there any in the NGLF 0.21 protocol).  
 * STEP 1: If this is a new packet (SYN has yet to be received) Clear the Packet
 * receive buffer and create a managed array needed by the serial port control for reading and
 * read the buffer contents.  If there was previous unprocessed data make the buffer contents the previous data +
 * the new data.
 * STEP 2: Check for the SYN (start of packet byte)
 * STEP 3: Check for TYPE byte and verify its validity.  If TYPE not valid - reset to start again
 * STEP 4: Check for lenght - lenght will determine how many payload bytes to expect
 * STEP 5: Capture the payload bytes until the check sum (the byte after the payload byte)
 * STEP 6: Check for completed packet. Verify if packet is valid (CS) or not.  Print accordingly to the
 * output screen.  
 * STEP 7: If a valid packet was recieved and parcing is enabled then parce the packet.  Check if data still
 * needs to be processed and preform recursion of this function if necessary.
 *************************************************************************/
private: System::Void serialPort1_DataReceived(System::Object^ sender, System::IO::Ports::SerialDataReceivedEventArgs^  e)
{
	String^ sHexString = "";	
	int BytesToRead = 0,
		RxBufferIndex = 0,
		OhmValue = 0;
	unsigned char CheckSum = 0;
	static unsigned char PacketBuffer[MAX_PACKET_SIZE],
		                 SaveForNextRxBuffer[MAX_PACKET_SIZE];
	static int PayLoadLength = 0,
			   PayLoadCount = 0,
			   BytesForNextRxBuffer = 0,
			   VoltRange = LOW_RNG_MIN,
			   ActiveLabelCount = 0;
	static bool SYN_Received = FALSE,
				TYPE_Received = FALSE,
				LEN_Received = FALSE,
				PacketComplete = FALSE,
				GetTypeOnNextRead = FALSE,
				GetLenOnNextRead = FALSE,
				ParcePacket = FALSE,
				BytesForNextRxBufferFlag = FALSE;

	
	// STEP 1
	//CLEAR THE PACKET BUFFER IF NOT PRESENTLY FILLING THE BUFFER
	if (!SYN_Received && !TYPE_Received && !LEN_Received)
	{
		for (int ByteCount = 0; ByteCount < MAX_PACKET_SIZE; ByteCount++)
		{
			PacketBuffer[ByteCount] = 0x00;
		}
	}		
	// CREATE A MANAGED ARRAY BUFFER FOR SERIAL PORT CONTROL AND LOAD THE RX BUFFER
	BytesToRead = serialPort1->BytesToRead;
	if ((BytesToRead < 1) && (BytesForNextRxBuffer == 0))
		return;
	// HAB FIX WITH RECURSION
	BUFFER_NOT_EMPTY: 
	array<unsigned char>^ RxBuffer = gcnew array<unsigned char>(BytesToRead + BytesForNextRxBuffer);
	serialPort1->Read(RxBuffer, 0, BytesToRead);
	/* CHECK IF PREVIOUS READ HAS UNPROCESSED DATA.  IF SO COMBINE PREVIOUS AND NEW DATA.
	 * TAKE THE PRESENT SERIAL READ AND CONCANTANATE IT TO THE PREVIOUS SERIAL READ IF THERE WAS SOMETHING LEFT OVER */
	if (BytesForNextRxBuffer !=0)
	{
		// PLACE ALL THE BYTES IN THE SaveForNextRxBuffer 
		for (int ByteCount = 0; ByteCount < BytesToRead; ByteCount++)
		{
			SaveForNextRxBuffer[(BytesForNextRxBuffer + ByteCount)] = RxBuffer[ByteCount];
		}
		// COPY TO RxBuffer
		for (int ByteCount = 0; ByteCount < (BytesToRead + BytesForNextRxBuffer); ByteCount++)
		{
			RxBuffer[ByteCount] = SaveForNextRxBuffer[ByteCount];
		}
		BytesToRead += BytesForNextRxBuffer;
		BytesForNextRxBuffer = 0;
	}


	// STEP 2
	// CHECK IF SYN BYTE RECEIVED
	if (!SYN_Received)
	{
		for (RxBufferIndex = 0; RxBufferIndex < BytesToRead; RxBufferIndex++)
		{
			if (RxBuffer[RxBufferIndex] == START_OF_PACKET)
			{
				// SET FLAG, STORE THE SYN, AND INCREMENT INDEX
				SYN_Received = TRUE;
				PacketBuffer[START_INDEX] = START_OF_PACKET;
				RxBufferIndex++;
				break;
			}
		}	
	}

	// STEP 3
	// CHECK FOR TYPE BYTE AND IF TYPE BYTE VALID
	if (!TYPE_Received && SYN_Received)
	{
		if (GetTypeOnNextRead) // IF LAST STREAM WAS TOO SHORT THIS FLAG WAS SET TO GET THE TYPE AS THE FIRST BYTE
		{
			if ((RxBuffer[RxBufferIndex] == TYPE_STOP) ||
				(RxBuffer[RxBufferIndex] == TYPE_VOLT) ||
				(RxBuffer[RxBufferIndex] == TYPE_OHMS) ||
				(RxBuffer[RxBufferIndex] == TYPE_TIME) ||
				(RxBuffer[RxBufferIndex] == TYPE_TBD1) ||
				(RxBuffer[RxBufferIndex] == TYPE_TBD2) ||
				(RxBuffer[RxBufferIndex] == TYPE_TBD3))
			{
				// SET FLAG, STORE TYPE, AND INCREMENT INDEX
				TYPE_Received = TRUE;
				PacketBuffer[MSG_TYPE_INDEX] = RxBuffer[0];
				RxBufferIndex++;
			}
			else
			{
				SYN_Received = FALSE;
				BytesForNextRxBuffer = 0;
			}
			GetTypeOnNextRead = FALSE;
		}
		else
		{	
			if ((RxBufferIndex) >= BytesToRead) 		// CHECK IF TYPE IS AVAILABE IN THIS RECEIVE STREAM
			{
				GetTypeOnNextRead = TRUE;
				BytesForNextRxBuffer = 0;
				return;
			}
			else
			{
				if ((RxBuffer[RxBufferIndex] == TYPE_STOP) ||
					(RxBuffer[RxBufferIndex] == TYPE_VOLT) ||
					(RxBuffer[RxBufferIndex] == TYPE_OHMS) ||
					(RxBuffer[RxBufferIndex] == TYPE_TIME) ||
					(RxBuffer[RxBufferIndex] == TYPE_TBD1) ||
					(RxBuffer[RxBufferIndex] == TYPE_TBD2) ||
					(RxBuffer[RxBufferIndex] == TYPE_TBD3))
				{
					// SET FLAG, STORE TYPE, AND INCREMENT INDEX
					TYPE_Received = TRUE;
					PacketBuffer[MSG_TYPE_INDEX] = RxBuffer[RxBufferIndex];
					RxBufferIndex++;
				}
				else
				{
					SYN_Received = FALSE;
					BytesForNextRxBuffer = 0;
					return;
				}
			}
		}
	}

	// STEP 4
	// CHECK FOR LENGHT
	if (!LEN_Received && SYN_Received && TYPE_Received)
	{
		if (GetLenOnNextRead) // IF LAST STREAM WAS TOO SHORT THIS FLAG WAS SET TO GET THE TYPE AS THE FIRST BYTE
		{
			// SET FLAG, STORE LEN, AND INCREMENT INDEX
			LEN_Received = TRUE;
			PacketBuffer[LENGTH_INDEX] = RxBuffer[RxBufferIndex];
			GetLenOnNextRead = FALSE;
			RxBufferIndex++;
		}
		else
		{
			if ((RxBufferIndex) >= BytesToRead) 		// CHECK IF LENGHT IS AVAILABE IN THIS RECEIVE STREAM
			{
				GetTypeOnNextRead = TRUE;
				BytesForNextRxBuffer = 0;
				return;
			}
			else
			{
				// SET FLAG, STORE LEN, AND INCREMENT INDEX
				LEN_Received = TRUE;
				PacketBuffer[LENGTH_INDEX] = PayLoadLength = RxBuffer[RxBufferIndex];
				// READY PACKET INDEX TO RECEIVE PAYLOAD - PAYLOAD ALWAYS BEGINS ON BYTE INDEX 3
				PayLoadCount = 0; 
				RxBufferIndex++;
			}
		}
	}

	// STEP 5
	// EXTRACT THE PAYLOAD
	if (SYN_Received && TYPE_Received && LEN_Received)
	{
		while(((RxBufferIndex) < BytesToRead) && (PayLoadCount < (PacketBuffer[LENGTH_INDEX] + 1)))
		{
			PacketBuffer[RX_PAYLOAD_INDEX + PayLoadCount] = RxBuffer[RxBufferIndex];
			RxBufferIndex++;
			PayLoadCount++;
			if (PayLoadCount == (PacketBuffer[LENGTH_INDEX] + 1))  // +1 TO GET THE CHECKSUM BYTE ALSO TO COMPLETE THE PACKET
			{
				PacketComplete = TRUE;
				/* A FULL PACKET HAS BEEN READ AT THIS POINT
				 * BUT THERE MAYBE ADDITIONAL BYTES IN THE BUFFER TO READ FOR THE NEXT PACKET
				 * STORE THOSE SO THEY CAN BE USED WITH THE NEXT RXBUFFER READ FROM THE SERIAL PORT */
				if (RxBufferIndex != BytesToRead)
				{
					BytesForNextRxBuffer = BytesToRead - RxBufferIndex;
					BytesForNextRxBufferFlag = TRUE;
					for (int ByteCount = 0; ByteCount < BytesForNextRxBuffer; ByteCount++)
					{
						SaveForNextRxBuffer[ByteCount] = RxBuffer[RxBufferIndex + ByteCount];
					}
					BytesToRead = 0;
				}
				else
					BytesForNextRxBuffer = 0; 
			}
		} 
	}

	// STEP 6
	// CHECK IF PACKET COMPLETE IF SO PRINT TO PACKET TEXTBOX
	if (PacketComplete)
	{
		if (check_packet_crc(PacketBuffer, (PacketBuffer[LENGTH_INDEX] + 4)) == FALSE) // 4 = SYN+TYPE+LEN+CS = 1+1+1+1
		{
			ErrPacketCount++;
			//SetErrPacketText(System::String::Format("{0:d4} ", ErrPacketCount));
			//sHexString += "***ERROR PACKET: " + System::String::Format("{0:d4} ", ErrPacketCount) + "\r\n";
		}
		else
		{
			for (int ByteCount = 0; ByteCount < (PacketBuffer[LENGTH_INDEX] + 4); ByteCount++)
			{
				sHexString += System::String::Format("{0:X2} ", PacketBuffer[ByteCount]);
			}
			ParcePacket = TRUE;
		}
		// PARSE THE PACKET
		Type_Parse hc15C;
		HC15C_ParsingEngine(PacketBuffer, (PacketBuffer[LENGTH_INDEX] + 4), &hc15C);
		// PACK IS TIME TYHPE
		if (hc15C.ParseOK && (hc15C.MsgType == TYPE_TIME))
		{
			SetClockSetActive(TRUE);
		}
		// PACKET IS OHM METER READING
		if (hc15C.ParseOK && (hc15C.MsgType == TYPE_OHMS))
		{
			// DISPLAY TO TEXT BOX
			OhmValue = (int)hc15C.TypeValue;
			if (OhmValue != OPEN_CIRCUIT_READING)
				sHexString += ":  " + OhmValue.ToString("0000") + "\r\n";
			else
				sHexString += ":  " + "OPEN" + "\r\n";
			SetTextOmeter(sHexString);
			// DISPLAY TO LABEL
			if (OhmValue != OPEN_CIRCUIT_READING)
				sHexString = OhmValue.ToString();
			else
				sHexString = "OL";
			SetTextO(sHexString);
			ActiveLabelCount++;
			if (ActiveLabelCount > ACTIVE_LABEL_UPDATE)
			{
				ActiveLabelCount = 0;
				if (lbl_Active2->Text == "*")
					SetActiveLabel2("");
				else
					SetActiveLabel2("*");
			}
		}

		// PACKET IS A VOLT METER READING
		if (hc15C.ParseOK && (hc15C.MsgType == TYPE_VOLT))
		{
			// CHECK FOR ERROR IN VALUE - RANGE CAN NEVER BE < LOW_RNG_MIN OR > MAX_RNG_MAX
			if (hc15C.TypeValue > MAX_RNG_MAX)
				hc15C.TypeValue = MAX_RNG_MAX;
			if (hc15C.TypeValue < LOW_RNG_MIN)
				hc15C.TypeValue = LOW_RNG_MIN;

			// UPDATE THE LOW, MED, MAX RANGE LABELS
			// CHECK THE RANGE FOR LOW
			if ((hc15C.TypeValue > LOW_RNG_MIN) && (hc15C.TypeValue < LOW_RNG_MAX))
			{
				if (VoltRange != LOW_RNG_MIN)
				{
					VoltRange = LOW_RNG_MIN;
					SetLabelLow("0.0");
					SetLabelMed("5.0");
					SetLabelMax("10.0");
					SetBarMinMax(LOW_RNG_MIN * 1000, false);
					SetBarMinMax(LOW_RNG_MAX * 1000, true);
				}
			}
			// CHECK THE RANGE MEDIUM
			if ((hc15C.TypeValue > MED_RNG_MIN) && (hc15C.TypeValue < MED_RNG_MAX))
			{
				if (VoltRange != MED_RNG_MIN)
				{
					VoltRange = MED_RNG_MIN;
					SetLabelLow("10.0");
					SetLabelMed("15.0");
					SetLabelMax("20.0");
					SetBarMinMax(MED_RNG_MIN * 1000, false);
					SetBarMinMax(MED_RNG_MAX * 1000, true);
				}
			}
			// CHECK THE RANGE HIGH
			if ((hc15C.TypeValue > MAX_RNG_MIN) && (hc15C.TypeValue < MAX_RNG_MAX))
			{
				if (VoltRange != MAX_RNG_MIN)
				{
					VoltRange = MAX_RNG_MIN;
					SetLabelLow("20.0");
					SetLabelMed("25.0");
					SetLabelMax("30.0");
					SetBarMinMax(MAX_RNG_MIN * 1000, false);
					SetBarMinMax(MAX_RNG_MAX * 1000, true);
				}
			}
			// DISPLAY THE PROGRESS BAR
			SetBar((int)(1000 * hc15C.TypeValue)); // DISPLAY AS mV
			// DISPLAY TO TEXT BOX
			sHexString += ":  " + hc15C.TypeValue.ToString("0.000") + "\r\n";
			SetTextVmeter(sHexString);
			// DISPLAY TO LABEL
			sHexString = hc15C.TypeValue.ToString("0.000");
			SetText(sHexString);
			// DISPLAY THE ACTIVE LABEL
			ActiveLabelCount++;
			if (ActiveLabelCount > ACTIVE_LABEL_UPDATE)
			{
				ActiveLabelCount = 0;
				if (lbl_Active->Text == "*")
					SetActiveLabel("");
				else
					SetActiveLabel("*");
			}
		}
		
		ParcePacket = FALSE;
		SYN_Received = FALSE;
		TYPE_Received = FALSE;
		LEN_Received = FALSE;
		PacketComplete = FALSE;
		GetTypeOnNextRead = FALSE;
		GetLenOnNextRead = FALSE;
		// CHECK IF BUFFER EMPTY
		BytesToRead = serialPort1->BytesToRead;
		if(BytesForNextRxBufferFlag) 
		{
			BytesForNextRxBufferFlag = FALSE;
			goto BUFFER_NOT_EMPTY; // HAB FIX - MAY BE A GOOD USE FOR RECURRSION
		}
	}

} // END OF FUNCTION serialPort1_DataReceived


/* COMBOX BPS SELECT
 * DESCRIPTION: COMM BPS select rate. 
 * STEP 1: CHECK IF COMM NOT SET FOR DEFULAT SERIAL RATE AND GIVE WARNING MSG IF SO
 * STEP 2: ASSIGN THE BPS TO THE SERIAL PORT */
private: System::Void cbx_BPS_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {

			 			 
			 // TYPE FOR NOW TO USE LATER
			 Object^ selectedItem = cbx_BPS->SelectedItem;
			 Object^ selectedIndex = cbx_BPS->SelectedIndex;
			 
			 // STEP 1
			 // CHECK IF BPS RATE NOT 4800
			 if (selectedItem->ToString() != DEFAULT_SERIAL_BPS)
				MessageBox::Show("BPS Rate Selected: " + selectedItem->ToString() + "bps" + "\n19200bps needed for NGLF\n" + "Index Selected: " + selectedIndex->ToString(), // DISPLAY MSG
				                 "WARNING",					// MSGBOX CAPTION
								 MessageBoxButtons::OK,		// USER RESPONSE
								 MessageBoxIcon::Warning);	// ICON TYPE

			 // STEP 2
			 // ASSIGN THE SERIAL PORT BPS
			 serialPort1->BaudRate = System::Convert::ToInt32(cbx_BPS->SelectedItem);
			 BpsRateFlag = TRUE;
			 if (BpsRateFlag && PortNameFlag)
				 btn_CommOpen->Enabled = TRUE;
		 }


/* COMBOX PORT SELECT
 * DESCRIPTION: COMM Port select 
 * STEP 1: ASSIGN COMM PORT NAME FROM LIST 
 * HAB FIX ME... IN LOAD ASK THE OS FOR COMM PORTS AND BUILD AND INTELLIGENT LIST 
 * PRESENTLY LIST IS HARD CODED AT COMPILE TIME - NEED A RUN TIME LIST */
private: System::Void cbx_Port_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {

			 // TYPE FOR NOW TO USE LATER
			 Object^ selectedItem = cbx_Port->SelectedItem;

			 // STEP 1
			 // ASSIGN THE PORT NAME
			 serialPort1->PortName = System::Convert::ToString(cbx_Port->SelectedItem);
			 PortNameFlag = TRUE;
			 if (BpsRateFlag && PortNameFlag)
			 {
				 btn_CommOpen->Enabled = TRUE;
			 }
		 }


private: System::Void btn_Test_Click(System::Object^  sender, System::EventArgs^  e) {


			 unsigned char TX_VoltTest[sizeof(double)],
				           TX_VoltPacket[20];
			 
			 DoubleInBytes.DoubleValue += 2;
			 for (int ByteCount = 0; ByteCount < sizeof(double); ByteCount++)
			 {
				TX_VoltTest[ByteCount] = DoubleInBytes.ByteValue[ByteCount];
			 }
			   
			 createTX_Buffer(TX_VoltTest, sizeof(double), TX_VoltPacket, TYPE_VOLT);
			 generateSerialOutput(TX_VoltPacket, sizeof(double) + 4); // 4 = SYN + TYPE + LEN + CS

		 }



/*************************************************************************
 * Function Name: generateSerialOutput
 * Parameters: unsigned char *, int
 * Return: 
 *
 * Description: Creates a managed array of the buffer contents to be written to the serial port.  It 
 * also displays the contents of the buffer to the textbox.
 * STEP 1: Create function vars for use to included the managed array needed by the serial port control
 * STEP 2: Load the managed array and print the contents of the array to the textbox in 2 digit hex format
 * STEP 3: Write port to write the contents of the managed array
 *************************************************************************/
void generateSerialOutput(unsigned char TransmitBuffer[], int SizeOfTX_Buffer)
{
	// STEP 1
	System::String^ sHexString = "";
	unsigned char *ptr_Packet_Buffer;
	ptr_Packet_Buffer = TransmitBuffer;
	// CREATE A MANAGED ARRAY TX BUFFER FOR SERIAL PORT CONTROL AND LOAD IT WITH TX BUFFER
	array<unsigned char>^ TxBuffer = gcnew array<unsigned char>(SizeOfTX_Buffer);

	// STEP 2
	for (int CharCount = 0; CharCount < SizeOfTX_Buffer; CharCount++)
	{
		TxBuffer[CharCount] = *ptr_Packet_Buffer;
		sHexString = System::String::Format("{0:X2} ", *ptr_Packet_Buffer);
		//txtMsg->Text += sHexString + " ";
		ptr_Packet_Buffer++;
	}
	
	// STEP 3
	serialPort1->Write(TxBuffer, 0, SizeOfTX_Buffer);

} // END OF generateSerialOutput


/*************************************************************************
 * Function Name: HC15C_ParsingEngine
 * Parameters: unsigned char *, int, *Type_Parse
 * Return: 
 *
 * Description: Parses a valid packet in accordance with HC15C REV1.X parsing protocol.  Packet
 * information is sent from the HC15C and it contains information that the HC15C is sending.  The
 * kind of informaiton will be determined by the TypeID byte (message type).  Volt information and 
 * ohm information is sent as a double value and is extracted via a union for reconstruction.
 * The function receives a pointer to a valid packet (packet must be valid) and the packet's size. It also
 * receives a pointer to Type_Parse which is loaded with the message type and the status of the parsing.
 * The parcing is conducted via the packet TYPE and PAYLOAD.  The SYN, LEN, and CS bytes are not parced for information.   
 * STEP 1: Assign pointers.  Start parsing via a loop count the size of the passed packet.  Check each byte 
 * (based on its byte count) for what it is and take the appropiate action.
 * STEP 2: Check to see if present ByteCount is the CheckSum Byte - If so parcing is ended - the count will force loop exit.
 * STEP 3: Check for SYN (Start of Packet) and LEN (Lenght of Payload in bytes) - Do nothing but increment Byte Count.
 * STEP 4: Check TYPE index and take appropiate actions - based on Type: if not a valid
 * type end by return.  
 * STEP 5: This is the payload.  For Volt and OHM type Parse the (SINGLE) payload to its double value.
 * STEP 6: Packet has been parsed OK
 *************************************************************************/
void HC15C_ParsingEngine(unsigned char *PacketToParce, int SizeOfPacket, Type_Parse *MeterValue)
{

	Union_DoubleInBytes ValueToConvert;
	unsigned char TypeID;
	unsigned char *ptr_PacketToParce;
	int ByteIndex = 0;
	String ^DummyString = "";
	DummyString = "DummyString";
	
	// STEP 1
	ptr_PacketToParce = PacketToParce;

	
	for (int ByteCount = 0; ByteCount < SizeOfPacket; ByteCount++)
	{
		// STEP 2
		if (ByteCount == (SizeOfPacket - 1)) // THIS IS THE CHECK SUM BYTE AND END OF PACKET
		{
			// FOR WILL EXIT ON BYTECOUNT MAX
		}
		
		// STEP 3
		if ((ByteCount == START_INDEX) || (ByteCount == LENGTH_INDEX))
		{
			ptr_PacketToParce++;
			continue;
		}

		// STEP 4
		if (ByteCount == MSG_TYPE_INDEX)
		{
			TypeID = *ptr_PacketToParce;
			MeterValue->MsgType = TypeID;
			switch(TypeID)
			{
			case TYPE_VOLT:
				break;
			case TYPE_OHMS:
				break;
			case TYPE_TIME:
				break;
			// UNKNOWN OR UNSUPPORTED TYPE 
			default:
				MeterValue->ParseOK = false;
				return;
			}
			ptr_PacketToParce++;
			continue;
		}

		// STEP 5
		if (ByteCount == RX_PAYLOAD_INDEX) 
		{
			// EXTRACT THE VOLT OR OHM METER VALUE 
			if ((TypeID == TYPE_VOLT) || (TypeID == TYPE_OHMS))
			{
				ByteIndex = 0;
				for(int ByteCount2 = RX_PAYLOAD_INDEX; ByteCount2 < (RX_PAYLOAD_INDEX + sizeof(double)); ByteCount2++)
				{
					ValueToConvert.ByteValue[ByteIndex] = *ptr_PacketToParce;
					ptr_PacketToParce++;
					ByteIndex++;
				}
				MeterValue->TypeValue = ValueToConvert.DoubleValue;
			}
		}

	} // END OF FOR PARCE

	// STEP 6
	// IF CODE GET HERE THEN PARSE WAS SUCESSFULL
	MeterValue->ParseOK = true;

} // END OF HC15C_ParsingEngine




/*************************************************************************
 * Function Name: PushText_FIFO
 * Parameters: void
 * Return: void
 *
 * Description: As the tbx_OhmMeter or tbx_VoltMeter fills up the system response slows, it becomes more likely to crash, 
 * and it becomes possible to loose data.  As the data in the textbox can grow very
 * quickly this function implements a simplified "chunck" FIFO that is called every LINEAR_TEXTBOX_BUFFER_SIZE.
 * The purpose of this function is not to let the textbox grow to be more than 2X LINEAR_TEXTBOX_BUFFER_SIZE
 * lines long.  If the function is being called for the first time since the text box was cleared
 * the var RawRawTextBoxEmpty is true (there are only LINEAR_TEXTBOX_BUFFER_SIZE lines in the text box - var name a bit miss-leading) - function returns.
 * However if the RawRawTextBoxEmpty is false (there are now 2x LINEAR_TEXTBOX_BUFFER_SIZE lines), then the function copies
 * the bottom half LINEAR_TEXTBOX_BUFFER_SIZE lines and writes that to the textbox.  Hence the textbox can be filled to 
 * a max on 2x LINEAR_TEXTBOX_BUFFER_SIZE.  When it gets larger than that the textbox is cleard 
 * and only the last LINEAR_TEXTBOX_BUFFER_SIZE lines are displayed.   
 * NOTE: The OP Code names are abbreviated from NGLF Protocol Document REV 0.24
 * STEP 1: Check if there are only LINEAR_TEXTBOX_BUFFER_SIZE lines in text box
 * STEP 2: Find and copy the bottom LINEAR_TEXTBOX_BUFFER_SIZE lines to a buffer
 * STEP 3: Load the bottom LINEAR_TEXTBOX_BUFFER_SIZE lines to the textbox
 *************************************************************************/
void PushText_FIFO(char ParseCharForEndOfLine, SELECTED_SET_TEXT SelectedTextBox)
{

	String ^FullString = "";
	String ^HalfString = "";
	int Count_NL = 0;

	// STEP 1
	// IS THIS THE FIRST LINEAR_TEXTBOX_BUFFER_SIZE write TO THE TEXT BOX - IF SO ALLOW
	if ((SelectedTextBox == OHM) && (OhmTextBoxEmpty))
	{
		OhmTextBoxEmpty = FALSE;
		return;
	}
	if ((SelectedTextBox == VOLT) && (VoltTextBoxEmpty))
	{
		VoltTextBoxEmpty = FALSE;
		return;
	}


	// STEP 2
	// COPY THE BOTTOM 50 LINES TO A BUFFER
	if (SelectedTextBox == OHM)
	{
		FullString = tbx_OhmMeter->Text;
	}
	else
	{
		FullString = txb_Vmeter->Text;
	}

	for (int i=0; i<FullString->Length; i++) 
	{
		if (Count_NL == LINEAR_TEXTBOX_BUFFER_SIZE)
		{
			HalfString += FullString[i];
		}
		else
		{
		if (FullString[i] == ParseCharForEndOfLine)
			Count_NL++;
		}
	}

	// STEP 3
	// LOAD THE BOTTOM LINEAR_TEXTBOX_BUFFER_SIZE TO THE TEXTBOX
	if (SelectedTextBox == OHM)
		tbx_OhmMeter->Text = HalfString;
	else
		txb_Vmeter->Text = HalfString;

} // END OF PushText_FIFO




/*************************************************************************
 * Function Name: runShell
 * Parameters: String, String
 * Return: bool
 *
 * Description: Function attempts to launch the specified file name (Target)
 * into the default application shell associated with that file extension.  If
 * it fails it will display an error. Return type is true / false
 * STEP 1: Launch the Target into its associated shell
 * STEP 2: Catch any exception that may occur. 
 *************************************************************************/
bool runShell(String ^Target, String ^ShellName)
{
	// STEP 1
	try
    {
		System::Diagnostics::Process::Start(Target);
    }

	// STEP 2
	catch(Exception ^Error)
	{
		MessageBox::Show("Error opening shell for " + ShellName + "\r\nDescription: " + Error->Message,		// DISPLAY MSG
				         "ERROR",				// MSGBOX CAPTION
						 MessageBoxButtons::OK,	// USER RESPONSE
						 MessageBoxIcon::Error);// ICON TYPE
		return(FALSE);
	}

	return(TRUE);

} // END OF runShell




};
}

