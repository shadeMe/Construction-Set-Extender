#pragma once


namespace UIComponents {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for Form1
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
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
	private: System::Windows::Forms::TextBox^  PreviewBox;






	private: BrightIdeasSoftware::FastObjectListView^ ScriptList;
	private: BrightIdeasSoftware::OLVColumn^ ScriptListCFlags;
	private: BrightIdeasSoftware::OLVColumn^ ScriptListCScriptName;
	private: BrightIdeasSoftware::OLVColumn^ ScriptListCFormID;
	private: BrightIdeasSoftware::OLVColumn^ ScriptListCType;
	private: BrightIdeasSoftware::OLVColumn^ ScriptListCParentPlugin;



	private: System::Windows::Forms::ToolStrip^ BottomToolStrip;
	private: System::Windows::Forms::ToolStripLabel^ ToolStripLabelFilter;
	private: System::Windows::Forms::ToolStripTextBox^ ToolStripFilterTextBox;

	private: System::Windows::Forms::ToolStripDropDownButton^ ToolStripLabelSelect;
	private: System::Windows::Forms::ToolStripMenuItem^ ActiveScriptsToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^ UncompiledScriptsToolStripMenuItem;

	private: System::Windows::Forms::ToolStripLabel^ ToolStripLabelSelectionCount;
	private: System::Windows::Forms::Button^ ButtonCompleteSelection;
	private: System::Windows::Forms::Timer^ DeferredSelectionUpdateTimer;
	private: System::ComponentModel::IContainer^ components;











































	protected:

	protected:

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->PreviewBox = (gcnew System::Windows::Forms::TextBox());
			this->ScriptList = (gcnew BrightIdeasSoftware::FastObjectListView());
			this->ScriptListCFlags = (gcnew BrightIdeasSoftware::OLVColumn());
			this->ScriptListCScriptName = (gcnew BrightIdeasSoftware::OLVColumn());
			this->ScriptListCFormID = (gcnew BrightIdeasSoftware::OLVColumn());
			this->ScriptListCType = (gcnew BrightIdeasSoftware::OLVColumn());
			this->ScriptListCParentPlugin = (gcnew BrightIdeasSoftware::OLVColumn());
			this->BottomToolStrip = (gcnew System::Windows::Forms::ToolStrip());
			this->ToolStripLabelFilter = (gcnew System::Windows::Forms::ToolStripLabel());
			this->ToolStripFilterTextBox = (gcnew System::Windows::Forms::ToolStripTextBox());
			this->ToolStripLabelSelect = (gcnew System::Windows::Forms::ToolStripDropDownButton());
			this->ActiveScriptsToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->UncompiledScriptsToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->ToolStripLabelSelectionCount = (gcnew System::Windows::Forms::ToolStripLabel());
			this->ButtonCompleteSelection = (gcnew System::Windows::Forms::Button());
			this->DeferredSelectionUpdateTimer = (gcnew System::Windows::Forms::Timer(this->components));
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ScriptList))->BeginInit();
			this->BottomToolStrip->SuspendLayout();
			this->SuspendLayout();
			// 
			// PreviewBox
			// 
			this->PreviewBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->PreviewBox->Font = (gcnew System::Drawing::Font(L"Consolas", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->PreviewBox->Location = System::Drawing::Point(484, 12);
			this->PreviewBox->Multiline = true;
			this->PreviewBox->Name = L"PreviewBox";
			this->PreviewBox->ReadOnly = true;
			this->PreviewBox->ScrollBars = System::Windows::Forms::ScrollBars::Both;
			this->PreviewBox->Size = System::Drawing::Size(406, 552);
			this->PreviewBox->TabIndex = 0;
			this->PreviewBox->WordWrap = false;
			// 
			// ScriptList
			// 
			this->ScriptList->AllColumns->Add(this->ScriptListCFlags);
			this->ScriptList->AllColumns->Add(this->ScriptListCScriptName);
			this->ScriptList->AllColumns->Add(this->ScriptListCFormID);
			this->ScriptList->AllColumns->Add(this->ScriptListCType);
			this->ScriptList->AllColumns->Add(this->ScriptListCParentPlugin);
			this->ScriptList->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left));
			this->ScriptList->CellEditUseWholeCell = false;
			this->ScriptList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(5) {
				this->ScriptListCFlags,
					this->ScriptListCScriptName, this->ScriptListCFormID, this->ScriptListCType, this->ScriptListCParentPlugin
			});
			this->ScriptList->Cursor = System::Windows::Forms::Cursors::Default;
			this->ScriptList->FullRowSelect = true;
			this->ScriptList->HideSelection = false;
			this->ScriptList->Location = System::Drawing::Point(12, 13);
			this->ScriptList->Name = L"ScriptList";
			this->ScriptList->ShowGroups = false;
			this->ScriptList->Size = System::Drawing::Size(466, 505);
			this->ScriptList->TabIndex = 9;
			this->ScriptList->TabStop = false;
			this->ScriptList->UseCompatibleStateImageBehavior = false;
			this->ScriptList->View = System::Windows::Forms::View::Details;
			this->ScriptList->VirtualMode = true;
			// 
			// ScriptListCFlags
			// 
			this->ScriptListCFlags->MaximumWidth = 20;
			this->ScriptListCFlags->MinimumWidth = 20;
			this->ScriptListCFlags->Text = L"";
			this->ScriptListCFlags->Width = 20;
			// 
			// ScriptListCScriptName
			// 
			this->ScriptListCScriptName->Text = L"Name";
			this->ScriptListCScriptName->Width = 180;
			// 
			// ScriptListCFormID
			// 
			this->ScriptListCFormID->Text = L"Form ID";
			this->ScriptListCFormID->Width = 75;
			// 
			// ScriptListCType
			// 
			this->ScriptListCType->Text = L"Type";
			this->ScriptListCType->Width = 54;
			// 
			// ScriptListCParentPlugin
			// 
			this->ScriptListCParentPlugin->Text = L"Parent Plugin";
			this->ScriptListCParentPlugin->Width = 114;
			// 
			// BottomToolStrip
			// 
			this->BottomToolStrip->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->BottomToolStrip->AutoSize = false;
			this->BottomToolStrip->Dock = System::Windows::Forms::DockStyle::None;
			this->BottomToolStrip->GripStyle = System::Windows::Forms::ToolStripGripStyle::Hidden;
			this->BottomToolStrip->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {
				this->ToolStripLabelFilter,
					this->ToolStripFilterTextBox, this->ToolStripLabelSelect, this->ToolStripLabelSelectionCount
			});
			this->BottomToolStrip->Location = System::Drawing::Point(12, 532);
			this->BottomToolStrip->Name = L"BottomToolStrip";
			this->BottomToolStrip->RenderMode = System::Windows::Forms::ToolStripRenderMode::System;
			this->BottomToolStrip->Size = System::Drawing::Size(379, 32);
			this->BottomToolStrip->TabIndex = 0;
			// 
			// ToolStripLabelFilter
			// 
			this->ToolStripLabelFilter->Name = L"ToolStripLabelFilter";
			this->ToolStripLabelFilter->Size = System::Drawing::Size(33, 29);
			this->ToolStripLabelFilter->Text = L"Filter";
			// 
			// ToolStripFilterTextBox
			// 
			this->ToolStripFilterTextBox->AutoSize = false;
			this->ToolStripFilterTextBox->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
			this->ToolStripFilterTextBox->Name = L"ToolStripFilterTextBox";
			this->ToolStripFilterTextBox->Size = System::Drawing::Size(150, 23);
			// 
			// ToolStripLabelSelect
			// 
			this->ToolStripLabelSelect->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {
				this->ActiveScriptsToolStripMenuItem,
					this->UncompiledScriptsToolStripMenuItem
			});
			this->ToolStripLabelSelect->Name = L"ToolStripLabelSelect";
			this->ToolStripLabelSelect->Size = System::Drawing::Size(75, 29);
			this->ToolStripLabelSelect->Text = L"Select all...";
			// 
			// ActiveScriptsToolStripMenuItem
			// 
			this->ActiveScriptsToolStripMenuItem->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->ActiveScriptsToolStripMenuItem->Name = L"ActiveScriptsToolStripMenuItem";
			this->ActiveScriptsToolStripMenuItem->Size = System::Drawing::Size(177, 22);
			this->ActiveScriptsToolStripMenuItem->Text = L"Active Scripts";
			this->ActiveScriptsToolStripMenuItem->TextImageRelation = System::Windows::Forms::TextImageRelation::TextAboveImage;
			// 
			// UncompiledScriptsToolStripMenuItem
			// 
			this->UncompiledScriptsToolStripMenuItem->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->UncompiledScriptsToolStripMenuItem->Name = L"UncompiledScriptsToolStripMenuItem";
			this->UncompiledScriptsToolStripMenuItem->Size = System::Drawing::Size(177, 22);
			this->UncompiledScriptsToolStripMenuItem->Text = L"Uncompiled Scripts";
			// 
			// ToolStripLabelSelectionCount
			// 
			this->ToolStripLabelSelectionCount->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
			this->ToolStripLabelSelectionCount->Name = L"ToolStripLabelSelectionCount";
			this->ToolStripLabelSelectionCount->Size = System::Drawing::Size(59, 29);
			this->ToolStripLabelSelectionCount->Text = L"0 selected";
			// 
			// ButtonCompleteSelection
			// 
			this->ButtonCompleteSelection->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->ButtonCompleteSelection->ImageAlign = System::Drawing::ContentAlignment::MiddleLeft;
			this->ButtonCompleteSelection->Location = System::Drawing::Point(412, 532);
			this->ButtonCompleteSelection->Name = L"ButtonCompleteSelection";
			this->ButtonCompleteSelection->Size = System::Drawing::Size(66, 32);
			this->ButtonCompleteSelection->TabIndex = 5;
			this->ButtonCompleteSelection->Text = L"OK";
			this->ButtonCompleteSelection->UseVisualStyleBackColor = true;
			// 
			// DeferredSelectionUpdateTimer
			// 
			this->DeferredSelectionUpdateTimer->Enabled = true;
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(900, 573);
			this->Controls->Add(this->ButtonCompleteSelection);
			this->Controls->Add(this->BottomToolStrip);
			this->Controls->Add(this->ScriptList);
			this->Controls->Add(this->PreviewBox);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::SizableToolWindow;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"Form1";
			this->StartPosition = System::Windows::Forms::FormStartPosition::Manual;
			this->Text = L"Select Scripts";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ScriptList))->EndInit();
			this->BottomToolStrip->ResumeLayout(false);
			this->BottomToolStrip->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	};
}

