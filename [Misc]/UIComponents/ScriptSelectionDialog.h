#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace UIComponents {

	/// <summary>
	/// Summary for ScriptEditorSelectScripts
	/// </summary>
	public ref class ScriptSelectionDialog : public DevComponents::DotNetBar::Metro::MetroForm
	{
	public:
		ScriptSelectionDialog(void)
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
		~ScriptSelectionDialog()
		{
			if (components)
			{
				delete components;
			}
		}
	private: BrightIdeasSoftware::FastObjectListView^ ScriptList;
	private: BrightIdeasSoftware::OLVColumn^ ScriptListCFlags;
	private: BrightIdeasSoftware::OLVColumn^ ScriptListCScriptName;
	private: BrightIdeasSoftware::OLVColumn^ ScriptListCFormID;
	private: BrightIdeasSoftware::OLVColumn^ ScriptListCType;
	private: BrightIdeasSoftware::OLVColumn^ ScriptListCParentPlugin;
	private: DevComponents::DotNetBar::Controls::TextBoxX^ ScriptTextPreview;

	private: DevComponents::DotNetBar::PanelEx^ BottomToolbarContainer;
	private: DevComponents::DotNetBar::Bar^ BottomToolbar;

	private: DevComponents::DotNetBar::TextBoxItem^ BottomToolbarTextboxFilter;
	private: DevComponents::DotNetBar::ButtonItem^ BottomToolbarSelectAll;
	private: DevComponents::DotNetBar::ButtonItem^ MenuSelectAllActive;
	private: DevComponents::DotNetBar::ButtonItem^ MenuSelectAllDeleted;
	private: DevComponents::DotNetBar::ButtonItem^ MenuSelectAllUncompiled;
	private: DevComponents::DotNetBar::LabelItem^ BottomToolbarSelectionCount;
	private: DevComponents::DotNetBar::ButtonX^ BottomToolbarCompleteSelection;
	private: System::Windows::Forms::Timer^ DeferredSelectionUpdateTimer;
	private: System::ComponentModel::IContainer^ components;













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
			System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(ScriptSelectionDialog::typeid));
			this->BottomToolbarCompleteSelection = (gcnew DevComponents::DotNetBar::ButtonX());
			this->BottomToolbarContainer = (gcnew DevComponents::DotNetBar::PanelEx());
			this->BottomToolbar = (gcnew DevComponents::DotNetBar::Bar());
			this->BottomToolbarTextboxFilter = (gcnew DevComponents::DotNetBar::TextBoxItem());
			this->BottomToolbarSelectAll = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->MenuSelectAllActive = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->MenuSelectAllDeleted = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->MenuSelectAllUncompiled = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->BottomToolbarSelectionCount = (gcnew DevComponents::DotNetBar::LabelItem());
			this->ScriptTextPreview = (gcnew DevComponents::DotNetBar::Controls::TextBoxX());
			this->ScriptList = (gcnew BrightIdeasSoftware::FastObjectListView());
			this->ScriptListCFlags = (gcnew BrightIdeasSoftware::OLVColumn());
			this->ScriptListCScriptName = (gcnew BrightIdeasSoftware::OLVColumn());
			this->ScriptListCFormID = (gcnew BrightIdeasSoftware::OLVColumn());
			this->ScriptListCType = (gcnew BrightIdeasSoftware::OLVColumn());
			this->ScriptListCParentPlugin = (gcnew BrightIdeasSoftware::OLVColumn());
			this->DeferredSelectionUpdateTimer = (gcnew System::Windows::Forms::Timer(this->components));
			this->BottomToolbarContainer->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->BottomToolbar))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ScriptList))->BeginInit();
			this->SuspendLayout();
			//
			// BottomToolbarCompleteSelection
			//
			this->BottomToolbarCompleteSelection->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
			this->BottomToolbarCompleteSelection->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->BottomToolbarCompleteSelection->ColorTable = DevComponents::DotNetBar::eButtonColor::OrangeWithBackground;
			this->BottomToolbarCompleteSelection->Location = System::Drawing::Point(404, 539);
			this->BottomToolbarCompleteSelection->Name = L"BottomToolbarCompleteSelection";
			this->BottomToolbarCompleteSelection->Size = System::Drawing::Size(81, 29);
			this->BottomToolbarCompleteSelection->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->BottomToolbarCompleteSelection->TabIndex = 2;
			this->BottomToolbarCompleteSelection->Text = L"OK";
			//
			// BottomToolbarContainer
			//
			this->BottomToolbarContainer->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->BottomToolbarContainer->CanvasColor = System::Drawing::SystemColors::Control;
			this->BottomToolbarContainer->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->BottomToolbarContainer->Controls->Add(this->BottomToolbar);
			this->BottomToolbarContainer->DisabledBackColor = System::Drawing::Color::Empty;
			this->BottomToolbarContainer->Location = System::Drawing::Point(3, 539);
			this->BottomToolbarContainer->Name = L"BottomToolbarContainer";
			this->BottomToolbarContainer->Size = System::Drawing::Size(395, 29);
			this->BottomToolbarContainer->Style->Alignment = System::Drawing::StringAlignment::Center;
			this->BottomToolbarContainer->Style->BackColor1->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground;
			this->BottomToolbarContainer->Style->Border = DevComponents::DotNetBar::eBorderType::SingleLine;
			this->BottomToolbarContainer->Style->BorderColor->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBorder;
			this->BottomToolbarContainer->Style->ForeColor->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelText;
			this->BottomToolbarContainer->Style->GradientAngle = 90;
			//
			// BottomToolbar
			//
			this->BottomToolbar->AntiAlias = true;
			this->BottomToolbar->Dock = System::Windows::Forms::DockStyle::Fill;
			this->BottomToolbar->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
			this->BottomToolbar->IsMaximized = false;
			this->BottomToolbar->Items->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(3) {
				this->BottomToolbarTextboxFilter,
					this->BottomToolbarSelectAll, this->BottomToolbarSelectionCount
			});
			this->BottomToolbar->ItemSpacing = 7;
			this->BottomToolbar->Location = System::Drawing::Point(0, 0);
			this->BottomToolbar->Name = L"BottomToolbar";
			this->BottomToolbar->Size = System::Drawing::Size(395, 29);
			this->BottomToolbar->Stretch = true;
			this->BottomToolbar->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->BottomToolbar->TabIndex = 0;
			this->BottomToolbar->TabStop = false;
			this->BottomToolbar->Text = L"bar1";
			//
			// BottomToolbarTextboxFilter
			//
			this->BottomToolbarTextboxFilter->Name = L"BottomToolbarTextboxFilter";
			this->BottomToolbarTextboxFilter->TextBoxWidth = 165;
			this->BottomToolbarTextboxFilter->WatermarkColor = System::Drawing::SystemColors::GrayText;
			this->BottomToolbarTextboxFilter->WatermarkText = L"Filter";
			//
			// BottomToolbarSelectAll
			//
			this->BottomToolbarSelectAll->AutoExpandOnClick = true;
			this->BottomToolbarSelectAll->BeginGroup = true;
			this->BottomToolbarSelectAll->ButtonStyle = DevComponents::DotNetBar::eButtonStyle::ImageAndText;
			this->BottomToolbarSelectAll->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"BottomToolbarSelectAll.Image")));
			this->BottomToolbarSelectAll->ImagePaddingHorizontal = 15;
			this->BottomToolbarSelectAll->ImagePaddingVertical = 10;
			this->BottomToolbarSelectAll->Name = L"BottomToolbarSelectAll";
			this->BottomToolbarSelectAll->SubItems->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(3) {
				this->MenuSelectAllActive,
					this->MenuSelectAllDeleted, this->MenuSelectAllUncompiled
			});
			this->BottomToolbarSelectAll->Text = L"Select All...";
			//
			// MenuSelectAllActive
			//
			this->MenuSelectAllActive->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"MenuSelectAllActive.Image")));
			this->MenuSelectAllActive->Name = L"MenuSelectAllActive";
			this->MenuSelectAllActive->Text = L"Active Scripts";
			//
			// MenuSelectAllDeleted
			//
			this->MenuSelectAllDeleted->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"MenuSelectAllDeleted.Image")));
			this->MenuSelectAllDeleted->Name = L"MenuSelectAllDeleted";
			this->MenuSelectAllDeleted->Text = L"Deleted Scripts";
			//
			// MenuSelectAllUncompiled
			//
			this->MenuSelectAllUncompiled->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"MenuSelectAllUncompiled.Image")));
			this->MenuSelectAllUncompiled->Name = L"MenuSelectAllUncompiled";
			this->MenuSelectAllUncompiled->Text = L"Uncompiled Scripts";
			//
			// BottomToolbarSelectionCount
			//
			this->BottomToolbarSelectionCount->ItemAlignment = DevComponents::DotNetBar::eItemAlignment::Far;
			this->BottomToolbarSelectionCount->Name = L"BottomToolbarSelectionCount";
			this->BottomToolbarSelectionCount->Text = L"0 selected";
			//
			// ScriptTextPreview
			//
			this->ScriptTextPreview->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->ScriptTextPreview->BackColor = System::Drawing::Color::Black;
			//
			//
			//
			this->ScriptTextPreview->Border->Class = L"TextBoxBorder";
			this->ScriptTextPreview->Border->CornerType = DevComponents::DotNetBar::eCornerType::Square;
			this->ScriptTextPreview->DisabledBackColor = System::Drawing::Color::Black;
			this->ScriptTextPreview->Font = (gcnew System::Drawing::Font(L"Consolas", 9));
			this->ScriptTextPreview->ForeColor = System::Drawing::Color::White;
			this->ScriptTextPreview->Location = System::Drawing::Point(491, 0);
			this->ScriptTextPreview->MaxLength = 65535;
			this->ScriptTextPreview->Multiline = true;
			this->ScriptTextPreview->Name = L"ScriptTextPreview";
			this->ScriptTextPreview->PreventEnterBeep = true;
			this->ScriptTextPreview->ReadOnly = true;
			this->ScriptTextPreview->ScrollBars = System::Windows::Forms::ScrollBars::Both;
			this->ScriptTextPreview->Size = System::Drawing::Size(406, 568);
			this->ScriptTextPreview->TabStop = false;
			this->ScriptTextPreview->WordWrap = false;
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
			this->ScriptList->BackColor = System::Drawing::Color::Black;
			this->ScriptList->CellEditUseWholeCell = false;
			this->ScriptList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(5) {
				this->ScriptListCFlags,
					this->ScriptListCScriptName, this->ScriptListCFormID, this->ScriptListCType, this->ScriptListCParentPlugin
			});
			this->ScriptList->Cursor = System::Windows::Forms::Cursors::Default;
			this->ScriptList->ForeColor = System::Drawing::Color::White;
			this->ScriptList->FullRowSelect = true;
			this->ScriptList->GridLines = true;
			this->ScriptList->HideSelection = false;
			this->ScriptList->Location = System::Drawing::Point(3, 0);
			this->ScriptList->Name = L"ScriptList";
			this->ScriptList->ShowGroups = false;
			this->ScriptList->Size = System::Drawing::Size(482, 533);
			this->ScriptList->TabIndex = 1;
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
			// DeferredSelectionUpdateTimer
			//
			this->DeferredSelectionUpdateTimer->Enabled = true;
			//
			// ScriptEditorSelectScripts
			//
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(900, 574);
			this->Controls->Add(this->BottomToolbarCompleteSelection);
			this->Controls->Add(this->BottomToolbarContainer);
			this->Controls->Add(this->ScriptTextPreview);
			this->Controls->Add(this->ScriptList);
			this->DoubleBuffered = true;
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::SizableToolWindow;
			this->Name = L"ScriptEditorSelectScripts";
			this->ShowIcon = false;
			this->Text = L"Select Scripts";
			this->BottomToolbarContainer->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->BottomToolbar))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ScriptList))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion
	};
}
