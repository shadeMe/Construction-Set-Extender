#include "WorkspaceView.h"
#include "[Common]\CustomInputBox.h"
#include "[Common]\AuxiliaryWindowsForm.h"
#include "ScriptEditorPreferences.h"

using namespace GlobalInputMonitor;

namespace ConstructionSetExtender
{
	namespace ScriptEditor
	{	
		void WorkspaceViewTabTearing::End()
		{
			Debug::Assert(InProgress == true);

			InProgress = false;
			Torn = nullptr;
			Source = nullptr;

			HookManager::MouseUp -= TearingEventDelegate;
		}

		void WorkspaceViewTabTearing::Begin(IWorkspaceModel^ Tearing, ConcreteWorkspaceView^ From)
		{
			Debug::Assert(InProgress == false);
			Debug::Assert(Tearing != nullptr);
			Debug::Assert(From != nullptr);

			InProgress = true;
			Torn = Tearing;
			Source = From;

			HookManager::MouseUp += TearingEventDelegate;
		}

		void WorkspaceViewTabTearing::TearingEventHandler(Object^ Sender, MouseEventArgs^ E)
		{
			switch (E->Button)
			{
			case MouseButtons::Left:
				{
					Debug::Assert(Torn != nullptr);
					Debug::Assert(Source != nullptr);

					bool Relocated = false;
					IntPtr Wnd = NativeWrapper::WindowFromPoint(E->Location);
					if (Wnd != IntPtr::Zero)					
					{
						Control^ UnderMouse = Control::FromHandle(Wnd);
						if (UnderMouse)
						{
							DotNetBar::SuperTabStrip^ Strip = nullptr;
							try { Strip = (DotNetBar::SuperTabStrip^)UnderMouse; }
							catch (...) {}

							if (Strip && Strip->Tag)
							{
								ConcreteWorkspaceView^ Parent = (ConcreteWorkspaceView^)Strip->Tag;
								if (Parent && Parent != Source)
								{
									// relocate to the view under the mouse
									Source->DissociateModel(Torn, false);
									Parent->AssociateModel(Torn, true);
									Relocated = true;
								}
							}
						}
					}

					if (Relocated == false)
					{
						// create new view and relocate
						Rectangle Bounds = Source->GetBounds(true);
						ConcreteWorkspaceView^ New = (ConcreteWorkspaceView^)Source->ViewFactory->CreateView(E->Location.X, E->Location.Y,
																											 Bounds.Width, Bounds.Height);
						Source->DissociateModel(Torn, false);
						New->AssociateModel(Torn, true);
					}

					End();
				}

				break;
			default:
				End();
				break;
			}
		}

		

		ConcreteWorkspaceView::ConcreteWorkspaceView(ConcreteWorkspaceViewController^ Controller, ConcreteWorkspaceViewFactory^ Factory, Rectangle Bounds)
		{
			Application::EnableVisualStyles();

			ViewController = Controller;
			ViewFactory = Factory;

			EditorFormCancelHandler = gcnew CancelEventHandler(this, &ConcreteWorkspaceView::EditorForm_Cancel);
			EditorFormKeyDownHandler = gcnew KeyEventHandler(this, &ConcreteWorkspaceView::EditorForm_KeyDown);
			EditorFormPositionChangedHandler = gcnew EventHandler(this, &ConcreteWorkspaceView::EditorForm_PositionChanged);
			EditorFormSizeChangedHandler = gcnew EventHandler(this, &ConcreteWorkspaceView::EditorForm_SizeChanged);

			ScriptStripTabItemCloseHandler = gcnew EventHandler<DotNetBar::SuperTabStripTabItemCloseEventArgs^>(this, &ConcreteWorkspaceView::ScriptStrip_TabItemClose);
			ScriptStripSelectedTabChangedHandler = gcnew EventHandler<DotNetBar::SuperTabStripSelectedTabChangedEventArgs^>(this, &ConcreteWorkspaceView::ScriptStrip_SelectedTabChanged);
			ScriptStripTabRemovedHandler = gcnew EventHandler<DotNetBar::SuperTabStripTabRemovedEventArgs^>(this, &ConcreteWorkspaceView::ScriptStrip_TabRemoved);
			ScriptStripMouseClickHandler = gcnew EventHandler<MouseEventArgs^>(this, &ConcreteWorkspaceView::ScriptStrip_MouseClick);
			ScriptStripTabMovingHandler = gcnew EventHandler<DotNetBar::SuperTabStripTabMovingEventArgs^>(this, &ConcreteWorkspaceView::ScriptStrip_TabMoving);

			NewTabButtonClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceView::NewTabButton_Click);
			SortTabsButtonClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceView::SortTabsButton_Click);
			ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &ConcreteWorkspaceView::ScriptEditorPreferences_Saved);

			EditorForm = gcnew AnimatedForm(0.10);
			EditorForm->SuspendLayout();

			EditorForm->FormBorderStyle = FormBorderStyle::Sizable;
			EditorForm->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			EditorForm->AutoScaleMode = AutoScaleMode::Font;
			EditorForm->Size = Size(Bounds.Width, Bounds.Height);
			EditorForm->KeyPreview = true;
			EditorForm->Tag = nullptr;

			EditorTabStrip = gcnew DotNetBar::SuperTabControl();
			EditorTabStrip->SuspendLayout();
			EditorTabStrip->Dock = DockStyle::Fill;
			EditorTabStrip->Location = Point(0, 0);
			EditorTabStrip->TabAlignment = DotNetBar::eTabStripAlignment::Top;
			EditorTabStrip->TabLayoutType = DotNetBar::eSuperTabLayoutType::SingleLine;
			EditorTabStrip->TextAlignment = DotNetBar::eItemAlignment::Far;
			EditorTabStrip->Font = gcnew Font("Segoe UI", 10, FontStyle::Regular);
			EditorTabStrip->TabFont = gcnew Font("Segoe UI", 9, FontStyle::Bold);
			EditorTabStrip->SelectedTabFont = gcnew Font("Segoe UI", 9, FontStyle::Bold);
			EditorTabStrip->FixedTabSize = Size(0, 23);

			Color TabStripGradientColorStart = Color::FromArgb(/*255, 59, 59, 59*/255, 85, 85, 85);
			Color TabStripGradientColorEnd = Color::FromArgb(/*255, 60, 60, 60*/255, 70, 70, 70);

			EditorTabStrip->CloseButtonOnTabsVisible = true;
			EditorTabStrip->CloseButtonOnTabsAlwaysDisplayed = false;
			EditorTabStrip->AntiAlias = true;
			EditorTabStrip->TabStop = false;
			EditorTabStrip->ImageList = gcnew ImageList;
			EditorTabStrip->ImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("ModifiedFlagOff"));
			EditorTabStrip->ImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("ModifiedFlagOn"));
			EditorTabStrip->ImageList->ImageSize = Size(12, 12);
			EditorTabStrip->ImageList->TransparentColor = Color::White;
			EditorTabStrip->ReorderTabsEnabled = true;
			EditorTabStrip->TabStyle = DotNetBar::eSuperTabStyle::Office2010BackstageBlue;
			EditorTabStrip->TabStripColor->OuterBorder = TabStripGradientColorStart;
			EditorTabStrip->TabStripColor->InnerBorder = TabStripGradientColorStart;

			EditorTabStrip->TabStrip->Tag = this;

			NewTabButton = gcnew DotNetBar::ButtonItem();
			NewTabButton->Image = Globals::ScriptEditorImageResourceManager->CreateImageFromResource("NewTabButton");
			NewTabButton->ButtonStyle = DotNetBar::eButtonStyle::Default;
			NewTabButton->Style = DotNetBar::eDotNetBarStyle::Office2007;
			NewTabButton->ColorTable = DotNetBar::eButtonColor::Office2007WithBackground;
			NewTabButton->Text = "New Tab";
			NewTabButton->Tooltip = "New Tab";

			SortTabsButton = gcnew DotNetBar::ButtonItem();
			SortTabsButton->ButtonStyle = DotNetBar::eButtonStyle::TextOnlyAlways;
			SortTabsButton->Text = "Sort Tabs";
			SortTabsButton->Tooltip = "Sort Tabs";

			NewTabButton->SubItems->Add(SortTabsButton);
			EditorTabStrip->ControlBox->SubItems->Add(NewTabButton);
			EditorTabStrip->ControlBox->Visible = true;

			DotNetBar::RibbonPredefinedColorSchemes::ChangeOffice2010ColorTable(EditorForm, DotNetBar::Rendering::eOffice2010ColorScheme::Black);
			EditorTabStrip->TabStripColor->Background = gcnew DotNetBar::Rendering::SuperTabLinearGradientColorTable(TabStripGradientColorStart,
																													 TabStripGradientColorEnd);

			if (!PREFERENCES->FetchSettingAsInt("TabsOnTop", "Appearance"))
			{
				EditorTabStrip->TabAlignment = DotNetBar::eTabStripAlignment::Left;
				EditorTabStrip->TabLayoutType = DotNetBar::eSuperTabLayoutType::SingleLine;
				EditorTabStrip->TextAlignment = DotNetBar::eItemAlignment::Near;
				EditorTabStrip->FixedTabSize = Size(150, 40);
			}

			EditorForm->HelpButton = false;
			EditorForm->Text = SCRIPTEDITOR_TITLE;

			EditorForm->Controls->Add(EditorTabStrip);

			AllowDisposal = false;

			EditorForm->Closing += EditorFormCancelHandler;
			EditorForm->KeyDown += EditorFormKeyDownHandler;
			EditorForm->Move += EditorFormPositionChangedHandler;
			EditorForm->SizeChanged += EditorFormSizeChangedHandler;
			EditorForm->MaximizedBoundsChanged += EditorFormSizeChangedHandler;
			EditorTabStrip->TabItemClose += ScriptStripTabItemCloseHandler;
			EditorTabStrip->SelectedTabChanged += ScriptStripSelectedTabChangedHandler;
			EditorTabStrip->TabRemoved += ScriptStripTabRemovedHandler;
			EditorTabStrip->TabStripMouseClick += ScriptStripMouseClickHandler;
			EditorTabStrip->TabMoving += ScriptStripTabMovingHandler;
			NewTabButton->Click += NewTabButtonClickHandler;
			SortTabsButton->Click += SortTabsButtonClickHandler;
			PREFERENCES->PreferencesSaved += ScriptEditorPreferencesSavedHandler;

			WorkspaceSplitter = gcnew SplitContainer();
			MessageList = gcnew ListView();
			FindList = gcnew ListView();
			BookmarkList = gcnew ListView();
			SpoilerText = gcnew Label();

			WorkspaceMainToolBar = gcnew ToolStrip();
			ToolBarNewScript = gcnew ToolStripButton();
			ToolBarOpenScript = gcnew ToolStripButton();
			ToolBarPreviousScript = gcnew ToolStripButton();
			ToolBarNextScript = gcnew ToolStripButton();
			ToolBarSaveScript = gcnew ToolStripSplitButton();
			ToolBarSaveScriptDropDown = gcnew ToolStripDropDown();
			ToolBarSaveScriptNoCompile = gcnew ToolStripButton();
			ToolBarSaveScriptAndPlugin = gcnew ToolStripButton();
			ToolBarRecompileScripts = gcnew ToolStripButton();
			ToolBarCompileDependencies = gcnew ToolStripButton();
			ToolBarDeleteScript = gcnew ToolStripButton();
			ToolBarNavigationBack = gcnew ToolStripButton();
			ToolBarNavigationForward = gcnew ToolStripButton();
			ToolBarSaveAll = gcnew ToolStripButton();
			ToolBarOptions = gcnew ToolStripButton();

			ToolBarScriptType = gcnew ToolStripDropDownButton();
			ToolBarScriptTypeContents = gcnew ToolStripDropDown();
			ToolBarScriptTypeContentsObject = gcnew ToolStripButton();
			ToolBarScriptTypeContentsQuest = gcnew ToolStripButton();
			ToolBarScriptTypeContentsMagicEffect = gcnew ToolStripButton();

			WorkspaceSecondaryToolBar = gcnew ToolStrip();
			ToolBarEditMenu = gcnew ToolStripDropDownButton();
			ToolBarEditMenuContents = gcnew ToolStripDropDown();
			ToolBarEditMenuContentsFind = gcnew ToolStripButton();
			ToolBarEditMenuContentsReplace = gcnew ToolStripButton();
			ToolBarEditMenuContentsGotoLine = gcnew ToolStripButton();
			ToolBarEditMenuContentsGotoOffset = gcnew ToolStripButton();
			ToolBarMessageList = gcnew ToolStripButton();
			ToolBarFindList = gcnew ToolStripButton();
			ToolBarBookmarkList = gcnew ToolStripButton();
			ToolBarDumpScript = gcnew ToolStripSplitButton();
			ToolBarDumpScriptDropDown = gcnew ToolStripDropDown();
			ToolBarDumpAllScripts = gcnew ToolStripButton();
			ToolBarLoadScript = gcnew ToolStripSplitButton();
			ToolBarLoadScriptDropDown = gcnew ToolStripDropDown();
			ToolBarLoadScriptsToTabs = gcnew ToolStripButton();
			ToolBarShowOffsets = gcnew ToolStripButton();
			ToolBarShowPreprocessedText = gcnew ToolStripButton();
			ToolBarSanitizeScriptText = gcnew ToolStripButton();
			ToolBarBindScript = gcnew ToolStripButton();
			ToolBarSnippetManager = gcnew ToolStripButton();
			ToolBarByteCodeSize = gcnew ToolStripProgressBar();

			ScriptListBox = gcnew ScriptListDialog(this);
			FindReplaceBox = gcnew FindReplaceDialog(this);

			Color ForeColor = Color::Black;
			Color BackColor = Color::White;
			Color HighlightColor = Color::Maroon;
			Font^ CustomFont = gcnew Font(PREFERENCES->FetchSettingAsString("Font", "Appearance"), PREFERENCES->FetchSettingAsInt("FontSize", "Appearance"), (FontStyle)PREFERENCES->FetchSettingAsInt("FontStyle", "Appearance"));

			OffsetViewer = gcnew TextEditors::ScriptOffsetViewer(CustomFont, ForeColor, BackColor, HighlightColor, WorkspaceSplitter->Panel1);
			PreprocessedTextViewer = gcnew TextEditors::SimpleTextViewer(CustomFont, ForeColor, BackColor, HighlightColor, WorkspaceSplitter->Panel1);

			SetupControlImage(ToolBarNewScript);
			SetupControlImage(ToolBarOpenScript);
			SetupControlImage(ToolBarPreviousScript);
			SetupControlImage(ToolBarNextScript);
			SetupControlImage(ToolBarSaveScript);
			SetupControlImage(ToolBarSaveScriptNoCompile);
			SetupControlImage(ToolBarSaveScriptAndPlugin);
			SetupControlImage(ToolBarRecompileScripts);
			SetupControlImage(ToolBarCompileDependencies);
			ToolBarCompileDependencies->ImageTransparentColor = Color::White;
			SetupControlImage(ToolBarDeleteScript);
			SetupControlImage(ToolBarNavigationBack);
			SetupControlImage(ToolBarNavigationForward);
			SetupControlImage(ToolBarSaveAll);
			SetupControlImage(ToolBarOptions);

			SetupControlImage(ToolBarEditMenu);
			SetupControlImage(ToolBarEditMenuContentsFind);
			SetupControlImage(ToolBarEditMenuContentsReplace);
			SetupControlImage(ToolBarEditMenuContentsGotoLine);
			SetupControlImage(ToolBarEditMenuContentsGotoOffset);

			SetupControlImage(ToolBarMessageList);
			SetupControlImage(ToolBarFindList);
			SetupControlImage(ToolBarBookmarkList);
			SetupControlImage(ToolBarDumpScript);
			SetupControlImage(ToolBarDumpAllScripts);
			SetupControlImage(ToolBarLoadScript);
			SetupControlImage(ToolBarLoadScriptsToTabs);
			SetupControlImage(ToolBarShowOffsets);
			SetupControlImage(ToolBarShowPreprocessedText);
			SetupControlImage(ToolBarSanitizeScriptText);
			SetupControlImage(ToolBarBindScript);
			SetupControlImage(ToolBarSnippetManager);

			SetupControlImage(ToolBarScriptTypeContentsObject);
			SetupControlImage(ToolBarScriptTypeContentsQuest);
			SetupControlImage(ToolBarScriptTypeContentsMagicEffect);

			ToolBarNavigationBackClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceView::ToolBarNavigationBack_Click);
			ToolBarNavigationForwardClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceView::ToolBarNavigationForward_Click);
			ToolBarSaveAllClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceView::ToolBarSaveAll_Click);
			ToolBarOptionsClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceView::ToolBarOptions_Click);

			ToolBarDumpAllScriptsClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceView::ToolBarDumpAllScripts_Click);
			ToolBarLoadScriptsToTabsClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceView::ToolBarLoadScriptsToTabs_Click);

			Padding ToolBarButtonPaddingLarge = Padding(16, 0, 16, 0);
			Padding ToolBarButtonPaddingRegular = Padding(10, 0, 10, 0);

			ToolStripStatusLabel^ ToolBarSpacerA = gcnew ToolStripStatusLabel();
			ToolBarSpacerA->Spring = true;

			WorkspaceSplitter->Dock = DockStyle::Fill;
			WorkspaceSplitter->SplitterWidth = 2;
			WorkspaceSplitter->Orientation = Orientation::Horizontal;

			WorkspaceMainToolBar->GripStyle = ToolStripGripStyle::Hidden;

			ToolBarNewScript->ToolTipText = "New Script";
			ToolBarNewScript->AutoSize = true;
			ToolBarNewScript->Margin = Padding(0, 0, ToolBarButtonPaddingLarge.Right, 0);

			ToolBarOpenScript->ToolTipText = "Open Script";
			ToolBarOpenScript->AutoSize = true;
			ToolBarOpenScript->Margin = ToolBarButtonPaddingLarge;

			ToolBarSaveScript->ToolTipText = "Save And Compile Script";
			ToolBarSaveScript->AutoSize = true;
			ToolBarSaveScript->Margin = ToolBarButtonPaddingLarge;
			ToolBarSaveScriptNoCompile->ToolTipText = "Save But Do Not Compile Script";
			ToolBarSaveScriptNoCompile->Text = "Save But Do Not Compile Script";
			ToolBarSaveScriptNoCompile->AutoSize = true;
			ToolBarSaveScriptAndPlugin->Text = "Save Script And Active Plugin";
			ToolBarSaveScriptAndPlugin->ToolTipText = "Save Script And Active Plugin";
			ToolBarSaveScriptAndPlugin->AutoSize = true;
			ToolBarSaveScriptDropDown->Items->Add(ToolBarSaveScriptNoCompile);
			ToolBarSaveScriptDropDown->Items->Add(ToolBarSaveScriptAndPlugin);
			ToolBarSaveScript->DropDown = ToolBarSaveScriptDropDown;

			ToolBarPreviousScript->ToolTipText = "Previous Script";
			ToolBarPreviousScript->AutoSize = true;
			ToolBarPreviousScript->Margin = Padding(ToolBarButtonPaddingLarge.Left, 0, ToolBarButtonPaddingRegular.Right, 0);

			ToolBarNextScript->ToolTipText = "Next Script";
			ToolBarNextScript->AutoSize = true;
			ToolBarNextScript->Margin = Padding(ToolBarButtonPaddingRegular.Left, 0, ToolBarButtonPaddingLarge.Right, 0);

			ToolBarRecompileScripts->ToolTipText = "Recompile Active Scripts";
			ToolBarRecompileScripts->AutoSize = true;
			ToolBarRecompileScripts->Margin = Padding(ToolBarButtonPaddingLarge.Left, 0, ToolBarButtonPaddingRegular.Right, 0);

			ToolBarCompileDependencies->ToolTipText = "Recompile Script Dependencies";
			ToolBarCompileDependencies->AutoSize = true;
			ToolBarCompileDependencies->Margin = Padding(ToolBarButtonPaddingRegular.Left, 0, ToolBarButtonPaddingLarge.Right, 0);

			ToolBarDeleteScript->ToolTipText = "Delete Script";
			ToolBarDeleteScript->AutoSize = true;
			ToolBarDeleteScript->Margin = ToolBarButtonPaddingLarge;

			ToolBarSaveAll->ToolTipText = "Save All Open Scripts";
			ToolBarSaveAll->AutoSize = true;
			ToolBarSaveAll->Margin = ToolBarButtonPaddingLarge;
			ToolBarSaveAll->Alignment = ToolStripItemAlignment::Right;

			ToolBarNavigationBack->ToolTipText = "Navigate Backward";
			ToolBarNavigationBack->AutoSize = true;
			ToolBarNavigationBack->Margin = Padding(ToolBarButtonPaddingLarge.Left, 0, ToolBarButtonPaddingRegular.Right, 0);
			ToolBarNavigationBack->Alignment = ToolStripItemAlignment::Right;

			ToolBarNavigationForward->ToolTipText = "Navigate Forward";
			ToolBarNavigationForward->AutoSize = true;
			ToolBarNavigationForward->Margin = Padding(ToolBarButtonPaddingRegular.Left, 0, ToolBarButtonPaddingLarge.Right, 0);
			ToolBarNavigationForward->Alignment = ToolStripItemAlignment::Right;

			ToolBarOptions->ToolTipText = "Preferences";
			ToolBarOptions->Alignment = ToolStripItemAlignment::Right;
			ToolBarOptions->Margin = Padding(ToolBarButtonPaddingLarge.Left, 0, 0, 0);

			ToolBarMessageList->ToolTipText = "Messages";
			ToolBarMessageList->AutoSize = true;
			ToolBarMessageList->Margin = Padding(0, 0, ToolBarButtonPaddingRegular.Right, 0);

			ToolBarFindList->ToolTipText = "Find/Replace Results";
			ToolBarFindList->AutoSize = true;
			ToolBarFindList->Margin = ToolBarButtonPaddingRegular;

			ToolBarBookmarkList->ToolTipText = "Bookmarks";
			ToolBarBookmarkList->AutoSize = true;
			ToolBarBookmarkList->Margin = Padding(ToolBarButtonPaddingRegular.Left, 0, ToolBarButtonPaddingLarge.Right, 0);

			ToolBarDumpScript->ToolTipText = "Dump Script";
			ToolBarDumpScript->AutoSize = true;
			ToolBarDumpScript->Margin = Padding(ToolBarButtonPaddingLarge.Left, 0, ToolBarButtonPaddingRegular.Right, 0);
			ToolBarDumpAllScripts->ToolTipText = "Dump All Tabs";
			ToolBarDumpAllScripts->Text = "Dump All Tabs";
			ToolBarDumpAllScripts->AutoSize = true;
			ToolBarDumpScriptDropDown->Items->Add(ToolBarDumpAllScripts);
			ToolBarDumpScript->DropDown = ToolBarDumpScriptDropDown;

			ToolBarLoadScript->ToolTipText = "Load Script";
			ToolBarLoadScript->AutoSize = true;
			ToolBarLoadScript->Margin = Padding(ToolBarButtonPaddingRegular.Left, 0, ToolBarButtonPaddingLarge.Right, 0);
			ToolBarLoadScriptsToTabs->Text = "Load Multiple Scripts Into Tabs";
			ToolBarLoadScriptsToTabs->ToolTipText = "Load Multiple Scripts Into Tabs";
			ToolBarLoadScriptsToTabs->AutoSize = true;
			ToolBarLoadScriptDropDown->Items->Add(ToolBarLoadScriptsToTabs);
			ToolBarLoadScript->DropDown = ToolBarLoadScriptDropDown;

			ToolBarShowOffsets->ToolTipText = "Toggle Offset Viewer";
			ToolBarShowOffsets->AutoSize = true;
			ToolBarShowOffsets->Margin = Padding(ToolBarButtonPaddingLarge.Left, 0, ToolBarButtonPaddingRegular.Right, 0);

			ToolBarShowPreprocessedText->ToolTipText = "Toggle Preprocessed Text Viewer";
			ToolBarShowPreprocessedText->AutoSize = true;
			ToolBarShowPreprocessedText->Margin = Padding(ToolBarButtonPaddingRegular.Left, 0, ToolBarButtonPaddingLarge.Right, 0);

			ToolBarSanitizeScriptText->ToolTipText = "Sanitize Script Text";
			ToolBarSanitizeScriptText->AutoSize = true;
			ToolBarSanitizeScriptText->Margin = ToolBarButtonPaddingLarge;

			ToolBarBindScript->ToolTipText = "Bind Script";
			ToolBarBindScript->AutoSize = true;
			ToolBarBindScript->Margin = ToolBarButtonPaddingLarge;

			ToolBarSnippetManager->ToolTipText = "Code Snippet Manager";
			ToolBarSnippetManager->AutoSize = true;
			ToolBarSnippetManager->Margin = ToolBarButtonPaddingLarge;

			ToolBarByteCodeSize->Minimum = 0;
			ToolBarByteCodeSize->Maximum = 0x8000;
			ToolBarByteCodeSize->AutoSize = false;
			ToolBarByteCodeSize->Size = Size(125, 14);
			ToolBarByteCodeSize->ToolTipText = "Compiled Script Size";
			ToolBarByteCodeSize->Alignment = ToolStripItemAlignment::Left;
			ToolBarByteCodeSize->Margin = Padding(8, 0, 8, 0);

			ToolBarScriptTypeContentsObject->Text = "Object                   ";
			ToolBarScriptTypeContentsObject->ToolTipText = "Object";
			ToolBarScriptTypeContentsQuest->Text = "Quest                    ";
			ToolBarScriptTypeContentsQuest->ToolTipText = "Quest";
			ToolBarScriptTypeContentsMagicEffect->Text = "Magic Effect        ";
			ToolBarScriptTypeContentsMagicEffect->ToolTipText = "Magic Effect";
			ToolBarScriptTypeContents->Items->Add(ToolBarScriptTypeContentsObject);
			ToolBarScriptTypeContents->Items->Add(ToolBarScriptTypeContentsQuest);
			ToolBarScriptTypeContents->Items->Add(ToolBarScriptTypeContentsMagicEffect);
			//	ToolBarScriptType->ImageTransparentColor = Color::White;
			ToolBarScriptType->DropDown = ToolBarScriptTypeContents;
			ToolBarScriptType->Alignment = ToolStripItemAlignment::Right;

			ToolBarEditMenuContentsFind->Text = "Find";
			ToolBarEditMenuContentsReplace->Text = "Replace";
			ToolBarEditMenuContentsGotoLine->Text = "Goto Line";
			ToolBarEditMenuContentsGotoOffset->Text = "Goto Offset";
			ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsFind);
			ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsReplace);
			ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsGotoLine);
			ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsGotoOffset);
			ToolBarEditMenu->Text = "Edit";
			ToolBarEditMenu->DropDown = ToolBarEditMenuContents;
			ToolBarEditMenu->Padding = Padding(0);
			ToolBarEditMenu->Alignment = ToolStripItemAlignment::Right;

			ToolBarDumpAllScripts->Click += ToolBarDumpAllScriptsClickHandler;
			ToolBarLoadScriptsToTabs->Click += ToolBarLoadScriptsToTabsClickHandler;
			ToolBarOptions->Click += ToolBarOptionsClickHandler;
			ToolBarNavigationBack->Click += ToolBarNavigationBackClickHandler;
			ToolBarNavigationForward->Click += ToolBarNavigationForwardClickHandler;
			ToolBarSaveAll->Click += ToolBarSaveAllClickHandler;

			WorkspaceMainToolBar->Dock = DockStyle::Top;
			WorkspaceMainToolBar->Items->Add(ToolBarNewScript);
			WorkspaceMainToolBar->Items->Add(ToolBarOpenScript);
			WorkspaceMainToolBar->Items->Add(ToolBarSaveScript);
			WorkspaceMainToolBar->Items->Add(ToolBarPreviousScript);
			WorkspaceMainToolBar->Items->Add(ToolBarNextScript);
			WorkspaceMainToolBar->Items->Add(ToolBarRecompileScripts);
			WorkspaceMainToolBar->Items->Add(ToolBarCompileDependencies);
			WorkspaceMainToolBar->Items->Add(ToolBarDeleteScript);
			WorkspaceMainToolBar->Items->Add(ToolBarSpacerA);
			WorkspaceMainToolBar->Items->Add(ToolBarOptions);
			WorkspaceMainToolBar->Items->Add(ToolBarNavigationForward);
			WorkspaceMainToolBar->Items->Add(ToolBarNavigationBack);
			WorkspaceMainToolBar->Items->Add(ToolBarSaveAll);
			WorkspaceMainToolBar->ShowItemToolTips = true;

			WorkspaceSecondaryToolBar->GripStyle = ToolStripGripStyle::Hidden;
			WorkspaceSecondaryToolBar->Dock = DockStyle::Bottom;
			WorkspaceSecondaryToolBar->Items->Add(ToolBarMessageList);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarFindList);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarBookmarkList);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarDumpScript);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarLoadScript);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarShowOffsets);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarShowPreprocessedText);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarSanitizeScriptText);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarBindScript);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarSnippetManager);
			WorkspaceSecondaryToolBar->Items->Add(gcnew ToolStripSeparator());
			WorkspaceSecondaryToolBar->Items->Add(ToolBarByteCodeSize);
			WorkspaceSecondaryToolBar->Items->Add(gcnew ToolStripSeparator());
			WorkspaceSecondaryToolBar->Items->Add(ToolBarEditMenu);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarScriptType);
			WorkspaceSecondaryToolBar->ShowItemToolTips = true;

			SpoilerText->Dock = DockStyle::Fill;
			SpoilerText->TextAlign = ContentAlignment::MiddleCenter;
			SpoilerText->Text = "Right, everybody out! Smash the Spinning Jenny! Burn the rolling Rosalind! Destroy the going-up-and-down-a-bit-and-then-moving-along Gertrude! And death to the stupid Prince who grows fat on the profits!";

			WorkspaceSplitter->Panel2->Controls->Add(WorkspaceSecondaryToolBar);
			WorkspaceSplitter->Panel2->Controls->Add(MessageList);
			WorkspaceSplitter->Panel2->Controls->Add(FindList);
			WorkspaceSplitter->Panel2->Controls->Add(BookmarkList);
			WorkspaceSplitter->Panel2->Controls->Add(SpoilerText);

			EditorForm->Controls->Add(WorkspaceMainToolBar);
			EditorForm->Controls->Add(WorkspaceSplitter);

			try { WorkspaceSplitter->SplitterDistance = GetBounds(true).Height; }
			catch (...) {}

			if (PREFERENCES->FetchSettingAsInt("UseCSParent", "General"))
			{
				EditorForm->ShowInTaskbar = false;
				EditorForm->Show(gcnew WindowHandleWrapper((IntPtr)NativeWrapper::g_CSEInterfaceTable->EditorAPI.GetMainWindowHandle()));
			}
			else
			{
				EditorForm->Show();
			}

			EditorForm->Location = Point(Bounds.Left, Bounds.Top);
			EditorTabStrip->ResumeLayout();
			EditorForm->ResumeLayout();

			AssociatedModels = gcnew ModelTabTableT;

			EditorForm->Tag = int(1);			// safe to handle events

			TODO("track the instance");
		}
			

		ConcreteWorkspaceView::~ConcreteWorkspaceView()
		{
			AllowDisposal = true;

			ViewController = nullptr;
			ViewFactory = nullptr;

			for each (auto Itr in AssociatedModels)
				delete Itr.Key;

			AssociatedModels->Clear();

			for each (Image^ Itr in EditorTabStrip->ImageList->Images)
				delete Itr;

			EditorTabStrip->ImageList->Images->Clear();
			EditorTabStrip->ImageList = nullptr;
			EditorTabStrip->TabStrip->Tag = nullptr;
			EditorTabStrip->Tabs->Clear();

			EditorForm->Closing -= EditorFormCancelHandler;
			EditorForm->KeyDown -= EditorFormKeyDownHandler;
			EditorForm->Move -= EditorFormPositionChangedHandler;
			EditorForm->SizeChanged -= EditorFormSizeChangedHandler;
			EditorForm->MaximizedBoundsChanged -= EditorFormSizeChangedHandler;
			EditorTabStrip->TabItemClose -= ScriptStripTabItemCloseHandler;
			EditorTabStrip->SelectedTabChanged -= ScriptStripSelectedTabChangedHandler;
			EditorTabStrip->TabRemoved -= ScriptStripTabRemovedHandler;
			EditorTabStrip->TabStripMouseClick -= ScriptStripMouseClickHandler;
			EditorTabStrip->TabMoving -= ScriptStripTabMovingHandler;
			NewTabButton->Click -= NewTabButtonClickHandler;
			SortTabsButton->Click -= SortTabsButtonClickHandler;
			PREFERENCES->PreferencesSaved -= ScriptEditorPreferencesSavedHandler;

			delete EditorTabStrip;
			delete NewTabButton->Image;
			delete NewTabButton;
			delete SortTabsButton;
						
			ToolBarDumpAllScripts->Click -= ToolBarDumpAllScriptsClickHandler;
			ToolBarLoadScriptsToTabs->Click -= ToolBarLoadScriptsToTabsClickHandler;
			ToolBarOptions->Click -= ToolBarOptionsClickHandler;
			ToolBarNavigationBack->Click -= ToolBarNavigationBackClickHandler;
			ToolBarNavigationForward->Click -= ToolBarNavigationForwardClickHandler;
			ToolBarSaveAll->Click -= ToolBarSaveAllClickHandler;

			DisposeControlImage(ToolBarNewScript);
			DisposeControlImage(ToolBarOpenScript);
			DisposeControlImage(ToolBarPreviousScript);
			DisposeControlImage(ToolBarNextScript);
			DisposeControlImage(ToolBarSaveScript);
			DisposeControlImage(ToolBarSaveScriptNoCompile);
			DisposeControlImage(ToolBarSaveScriptAndPlugin);
			DisposeControlImage(ToolBarRecompileScripts);
			DisposeControlImage(ToolBarCompileDependencies);
			DisposeControlImage(ToolBarDeleteScript);
			DisposeControlImage(ToolBarNavigationBack);
			DisposeControlImage(ToolBarNavigationForward);
			DisposeControlImage(ToolBarSaveAll);
			DisposeControlImage(ToolBarOptions);

			DisposeControlImage(ToolBarEditMenu);
			DisposeControlImage(ToolBarEditMenuContentsFind);
			DisposeControlImage(ToolBarEditMenuContentsReplace);
			DisposeControlImage(ToolBarEditMenuContentsGotoLine);
			DisposeControlImage(ToolBarEditMenuContentsGotoOffset);

			DisposeControlImage(ToolBarMessageList);
			DisposeControlImage(ToolBarFindList);
			DisposeControlImage(ToolBarBookmarkList);
			DisposeControlImage(ToolBarDumpScript);
			DisposeControlImage(ToolBarDumpAllScripts);
			DisposeControlImage(ToolBarLoadScript);
			DisposeControlImage(ToolBarLoadScriptsToTabs);
			DisposeControlImage(ToolBarShowOffsets);
			DisposeControlImage(ToolBarShowPreprocessedText);
			DisposeControlImage(ToolBarSanitizeScriptText);
			DisposeControlImage(ToolBarBindScript);
			DisposeControlImage(ToolBarSnippetManager);

			DisposeControlImage(ToolBarScriptTypeContentsObject);
			DisposeControlImage(ToolBarScriptTypeContentsQuest);
			DisposeControlImage(ToolBarScriptTypeContentsMagicEffect);

			delete WorkspaceMainToolBar;
			delete ToolBarNewScript;
			delete ToolBarOpenScript;
			delete ToolBarPreviousScript;
			delete ToolBarNextScript;
			delete ToolBarSaveScript;
			delete ToolBarSaveScriptDropDown;
			delete ToolBarSaveScriptNoCompile;
			delete ToolBarSaveScriptAndPlugin;
			delete ToolBarRecompileScripts;
			delete ToolBarCompileDependencies;
			delete ToolBarDeleteScript;
			delete ToolBarNavigationBack;
			delete ToolBarNavigationForward;
			delete ToolBarSaveAll;
			delete ToolBarOptions;

			delete ToolBarScriptType;
			delete ToolBarScriptTypeContents;
			delete ToolBarScriptTypeContentsObject;
			delete ToolBarScriptTypeContentsQuest;
			delete ToolBarScriptTypeContentsMagicEffect;

			delete WorkspaceSecondaryToolBar;
			delete ToolBarEditMenu;
			delete ToolBarEditMenuContents;
			delete ToolBarEditMenuContentsFind;
			delete ToolBarEditMenuContentsReplace;
			delete ToolBarEditMenuContentsGotoLine;
			delete ToolBarEditMenuContentsGotoOffset;
			delete ToolBarMessageList;
			delete ToolBarFindList;
			delete ToolBarBookmarkList;
			delete ToolBarDumpScript;
			delete ToolBarDumpScriptDropDown;
			delete ToolBarDumpAllScripts;
			delete ToolBarLoadScript;
			delete ToolBarLoadScriptDropDown;
			delete ToolBarLoadScriptsToTabs;
			delete ToolBarShowOffsets;
			delete ToolBarShowPreprocessedText;
			delete ToolBarSanitizeScriptText;
			delete ToolBarBindScript;
			delete ToolBarSnippetManager;
			delete ToolBarByteCodeSize;

			delete OffsetViewer;
			delete PreprocessedTextViewer;
			delete ScriptListBox;
			delete FindReplaceBox;

			ScriptListBox = nullptr;
			FindReplaceBox = nullptr;
			OffsetViewer = nullptr;
			PreprocessedTextViewer = nullptr;

			WorkspaceSplitter->Panel1->Controls->Clear();
			WorkspaceSplitter->Panel2->Controls->Clear();

			delete WorkspaceSplitter;
			delete MessageList;
			delete FindList;
			delete BookmarkList;
			delete SpoilerText;

			EditorForm->ForceClose();
			
			PREFERENCES->SaveINI();

			ViewFactory->Remove(this);
		}		

		void ConcreteWorkspaceView::EditorForm_Cancel(Object^ Sender, CancelEventArgs^ E)
		{
			if (AllowDisposal)
			{
				Debug::Assert(EditorTabStrip == nullptr || GetTabCount() == 0);

				Rectangle Bounds = GetBounds(true);
				NativeWrapper::g_CSEInterfaceTable->ScriptEditor.SaveEditorBoundsToINI(Bounds.Left, Bounds.Top, Bounds.Width, Bounds.Height);

				// yuck!
				delete this;
				return;
			}

			E->Cancel = true;
			if (GetTabCount() > 1)
			{
				if (MessageBox::Show("Are you sure you want to close all open tabs?",
					SCRIPTEDITOR_TITLE,
					MessageBoxButtons::YesNo,
					MessageBoxIcon::Question,
					MessageBoxDefaultButton::Button2) == DialogResult::No)
				{
					return;
				}
			}

			AllowDisposal = true;
			CloseAll();
			AllowDisposal = false;
		}

		void ConcreteWorkspaceView::EditorForm_KeyDown(Object^ Sender, KeyEventArgs^ E)
		{
			TODO("Tunnel key events down");
		}

		void ConcreteWorkspaceView::EditorForm_SizeChanged(Object^ Sender, EventArgs^ E)
		{
			;//
		}

		void ConcreteWorkspaceView::EditorForm_PositionChanged(Object^ Sender, EventArgs^ E)
		{
			;//
		}

		void ConcreteWorkspaceView::ScriptStrip_TabItemClose(Object^ Sender, DotNetBar::SuperTabStripTabItemCloseEventArgs ^ E)
		{
			E->Cancel = true;

			IWorkspaceModel^ Active = GetModel((DotNetBar::SuperTabItem^)E->Tab);	// clicking on the close button doesn't change the active tab
			if (ModelController()->Close(Active))
				DissociateModel(Active, true);
		}

		void ConcreteWorkspaceView::ScriptStrip_SelectedTabChanged(Object^ Sender, DotNetBar::SuperTabStripSelectedTabChangedEventArgs^ E)
		{
			if (E->OldValue == nullptr || E->NewValue == nullptr)
				return;

			IWorkspaceModel^ Old = GetModel((DotNetBar::SuperTabItem^)E->OldValue);
			IWorkspaceModel^ New = GetModel((DotNetBar::SuperTabItem^)E->NewValue);

			BeginUpdate();
			ModelController()->Unbind(Old);
			ModelController()->Bind(New, this);
			EndUpdate();
		}

		void ConcreteWorkspaceView::ScriptStrip_TabRemoved(Object^ Sender, DotNetBar::SuperTabStripTabRemovedEventArgs^ E)
		{
			if (GetTabCount() == 0)
			{
				EditorForm->Close();
			}
		}

		void ConcreteWorkspaceView::ScriptStrip_MouseClick(Object^ Sender, MouseEventArgs^ E)
		{
			switch (E->Button)
			{
			case MouseButtons::Middle:
				{
					DotNetBar::SuperTabItem^ MouseOverTab = GetMouseOverTab();
					if (MouseOverTab)
					{
						IWorkspaceModel^ Model = GetModel(MouseOverTab);
						if (ModelController()->Close(Model))
							DissociateModel(Model, true);
					}

					break;
				}
			}
		}

		void ConcreteWorkspaceView::ScriptStrip_TabMoving(Object^ Sender, DotNetBar::SuperTabStripTabMovingEventArgs^ E)
		{
			if (WorkspaceViewTabTearing::InProgress == false)
			{
				DotNetBar::SuperTabItem^ MouseOverTab = GetMouseOverTab();
				if (MouseOverTab)
					WorkspaceViewTabTearing::Begin(GetModel(MouseOverTab), this);
			}
		}

		void ConcreteWorkspaceView::NewTabButton_Click(Object^ Sender, EventArgs^ E)
		{
			NewTab(gcnew NewTabOperationArgs);
		}

		void ConcreteWorkspaceView::SortTabsButton_Click(Object^ Sender, EventArgs^ E)
		{
			array<String^>^ Keys = gcnew array<String^>(EditorTabStrip->Tabs->Count);
			array<DotNetBar::BaseItem^>^ Tabs = gcnew array<DotNetBar::BaseItem^>(EditorTabStrip->Tabs->Count);

			for (int i = 0; i < EditorTabStrip->Tabs->Count; i++)
			{
				Tabs[i] = EditorTabStrip->Tabs[i];
				Keys[i] = EditorTabStrip->Tabs[i]->PlainText;
			}

			Array::Sort(Keys, Tabs);

			for (int i = 0; i < EditorTabStrip->Tabs->Count; i++)
				EditorTabStrip->Tabs[i] = Tabs[i];

			EditorTabStrip->RecalcLayout();
		}

		void ConcreteWorkspaceView::ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E)
		{
			EditorTabStrip->TabAlignment = DotNetBar::eTabStripAlignment::Top;
			EditorTabStrip->TabLayoutType = DotNetBar::eSuperTabLayoutType::SingleLine;
			EditorTabStrip->TextAlignment = DotNetBar::eItemAlignment::Far;
			EditorTabStrip->FixedTabSize = Size(0, 23);

			if (!PREFERENCES->FetchSettingAsInt("TabsOnTop", "Appearance"))
			{
				EditorTabStrip->TabAlignment = DotNetBar::eTabStripAlignment::Left;
				EditorTabStrip->TabLayoutType = DotNetBar::eSuperTabLayoutType::SingleLine;
				EditorTabStrip->TextAlignment = DotNetBar::eItemAlignment::Near;
				EditorTabStrip->FixedTabSize = Size(150, 40);
			}

			Redraw();
		}

		void ConcreteWorkspaceView::ToolBarDumpAllScripts_Click(Object^ Sender, EventArgs^ E)
		{
			FolderBrowserDialog^ SaveManager = gcnew FolderBrowserDialog();

			SaveManager->Description = "All open scripts in this window will be dumped to the selected folder.";
			SaveManager->ShowNewFolderButton = true;
			SaveManager->SelectedPath = gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetDefaultCachePath());

			if (SaveManager->ShowDialog() == DialogResult::OK && SaveManager->SelectedPath->Length > 0)
			{
				String^ FileExtension = "txt";
				InputBoxes::InputBoxResult^ Result = InputBoxes::InputBox::Show("Enter The File Extension To Use", "Dump Scripts", FileExtension);
				if (Result->ReturnCode == DialogResult::Cancel || Result->Text == "")
					return;
				else
					FileExtension = Result->Text;

				DumpAllToDisk(SaveManager->SelectedPath, FileExtension);
			}
		}

		void ConcreteWorkspaceView::ToolBarLoadScriptsToTabs_Click(Object^ Sender, EventArgs^ E)
		{
			OpenFileDialog^ LoadManager = gcnew OpenFileDialog();

			LoadManager->DefaultExt = "*.txt";
			LoadManager->Filter = "Text Files|*.txt|All files (*.*)|*.*";
			LoadManager->Multiselect = true;
			LoadManager->RestoreDirectory = true;

			if (LoadManager->ShowDialog() == DialogResult::OK && LoadManager->FileNames->Length > 0)
			{
				LoadAllFromDisk(LoadManager->FileNames);
			}
		}


		IWorkspaceModelController^ ConcreteWorkspaceView::ModelController()
		{
			Debug::Assert(AssociatedModels->Count != 0);

			return AssociatedModels->GetEnumerator().Current.Key->ModelController;
		}

		IWorkspaceModelFactory^ ConcreteWorkspaceView::ModelFactory()
		{
			Debug::Assert(AssociatedModels->Count != 0);

			return AssociatedModels->GetEnumerator().Current.Key->ModelFactory;
		}

		bool ConcreteWorkspaceView::IsModelAssociated(IWorkspaceModel^ Model)
		{
			return AssociatedModels->ContainsKey(Model);
		}

		void ConcreteWorkspaceView::AssociateModel(IWorkspaceModel^ Model, bool Bind)
		{
			Debug::Assert(IsModelAssociated(Model) == false);

			DotNetBar::SuperTabItem^ Tab = gcnew DotNetBar::SuperTabItem;
			DotNetBar::SuperTabControlPanel^ Container = gcnew DotNetBar::SuperTabControlPanel;

		//	Container->Dock = DockStyle::Fill;
			Container->Location = Point(-5, -55);
			Container->Size = Size(1, 1);
			Container->Padding = Padding(1);
			Container->TabItem = Tab;

			Tab->AttachedControl = Container;
			Tab->Tooltip = Description;
			Tab->Text = " " + Description;
			Tab->Tag = this;

			EditorTabStrip->Tabs->Add(Tab);
			EditorTabStrip->Controls->Add(Container);

			AssociatedModels->Add(Model, Tab);

			if (Bind)
				SelectTab(Tab);
		}

		void ConcreteWorkspaceView::DissociateModel(IWorkspaceModel^ Model, bool Destroy)
		{
			Debug::Assert(IsModelAssociated(Model) == true);

			ModelController()->Unbind(Model);
			DotNetBar::SuperTabItem^ Tab = GetTab(Model);
			DotNetBar::SuperTabControlPanel^ Container = (DotNetBar::SuperTabControlPanel^)Tab->AttachedControl;

			EditorTabStrip->Tabs->Remove(Tab);
			EditorTabStrip->Controls->Remove(Container);

			Tab->AttachedControl = nullptr;
			Tab->Tag = nullptr;

			delete Tab;
			delete Container;

			AssociatedModels->Remove(Model);

			if (Destroy)
				delete Model;
		}

		IWorkspaceModel^ ConcreteWorkspaceView::GetModel(DotNetBar::SuperTabItem^ Tab)
		{
			return (IWorkspaceModel^)Tab->Tag;
		}

		DotNetBar::SuperTabItem^ ConcreteWorkspaceView::GetTab(IWorkspaceModel^ Model)
		{
			if (IsModelAssociated(Model))
				return AssociatedModels[Model];
			else
				return nullptr;
		}

		DotNetBar::SuperTabItem^ ConcreteWorkspaceView::GetMouseOverTab()
		{
			for each (DotNetBar::SuperTabItem^ Itr in EditorTabStrip->Tabs)
			{
				if (Itr->IsMouseOver)
					return Itr;
			}

			return nullptr;
		}

		DotNetBar::SuperTabItem^ ConcreteWorkspaceView::GetActiveTab()
		{
			return EditorTabStrip->SelectedTab;
		}

		IWorkspaceModel^ ConcreteWorkspaceView::GetActiveModel()
		{
			return GetModel(GetActiveTab());
		}

		Rectangle ConcreteWorkspaceView::GetBounds(bool UseRestoreBounds)
		{
			if (EditorForm->WindowState == FormWindowState::Normal || UseRestoreBounds == false)
				return EditorForm->Bounds;
			else
				return EditorForm->RestoreBounds;
		}

		void ConcreteWorkspaceView::SaveAll()
		{
			for each (auto Itr in AssociatedModels)
			{
				ModelController()->Save(Itr.Key);
			}
		}

		void ConcreteWorkspaceView::CloseAll()
		{
			List<IWorkspaceModel^>^ Dissociate = gcnew List < IWorkspaceModel^ > ;

			for each (auto Itr in AssociatedModels)
			{
				if (ModelController()->Close(Itr.Key))
					Dissociate->Add(Itr.Key);
			}

			for each (auto Itr in Dissociate)
				DissociateModel(Itr, true);
		}

		void ConcreteWorkspaceView::DumpAllToDisk(String^ OutputDirectory, String^ FileExtension)
		{
			for each (auto Itr in AssociatedModels)
			{
				if (Itr.Key->New == false && Itr.Key->Initialized == true)
					ModelController()->SaveToDisk(Itr.Key, OutputDirectory, false, FileExtension);
			}
		}

		void ConcreteWorkspaceView::LoadAllFromDisk(array<String^>^ FilePaths)
		{
			NewTabOperationArgs^ E = gcnew NewTabOperationArgs;
			for each (auto Path in FilePaths)
			{
				E->PostCreationOperation = NewTabOperationArgs::PostNewTabOperation::LoadFromDisk;
				E->PathToFile = Path;
				NewTab(E);
			}
		}

		void ConcreteWorkspaceView::BeginUpdate()
		{
			EditorForm->SuspendLayout();
		}

		void ConcreteWorkspaceView::EndUpdate()
		{
			EditorForm->ResumeLayout(true);
		}

		void ConcreteWorkspaceView::Redraw()
		{
			EditorForm->Invalidate(true);
		}

		int ConcreteWorkspaceView::GetTabCount()
		{
			return EditorTabStrip->Tabs->Count;
		}

		void ConcreteWorkspaceView::SelectTab(DotNetBar::SuperTabItem^ Tab)
		{
			EditorTabStrip->SelectedTab = Tab;
			EditorTabStrip->TabStrip->EnsureVisible(Tab);
		}

		void ConcreteWorkspaceView::SelectTab(Keys Index)
		{
			int TabIndex = -1;

			switch (Index)
			{
			case Keys::D1:
				TabIndex = 0;
				break;
			case Keys::D2:
				TabIndex = 1;
				break;
			case Keys::D3:
				TabIndex = 2;
				break;
			case Keys::D4:
				TabIndex = 3;
				break;
			case Keys::D5:
				TabIndex = 4;
				break;
			case Keys::D6:
				TabIndex = 5;
				break;
			case Keys::D7:
				TabIndex = 6;
				break;
			case Keys::D8:
				TabIndex = 7;
				break;
			case Keys::D9:
				TabIndex = 8;
				break;
			default:
				break;
			}

			if (TabIndex != -1)
			{
				SelectTab(TabIndex);
			}
		}

		void ConcreteWorkspaceView::SelectTab(int Index)
		{
			if (Index == -1)
				SelectTab((DotNetBar::SuperTabItem^)(EditorTabStrip->Tabs[GetTabCount() - 1]));
			else if (Index < EditorTabStrip->Tabs->Count)
				SelectTab((DotNetBar::SuperTabItem^)(EditorTabStrip->Tabs[Index]));
		}

		void ConcreteWorkspaceView::SelectPreviousTab()
		{
			if (GetTabCount() < 2)
				return;

			if (EditorTabStrip->SelectPreviousTab() == false)
				SelectTab(-1);
		}

		void ConcreteWorkspaceView::SelectNextTab()
		{
			if (GetTabCount() < 2)
				return;

			if (EditorTabStrip->SelectNextTab() == false)
				SelectTab(0);
		}
		
		void ConcreteWorkspaceView::NewTab(NewTabOperationArgs^ E)
		{
			IWorkspaceModel^ New = nullptr;

			if (E->PostCreationOperation != NewTabOperationArgs::PostNewTabOperation::LoadFromDisk)
			{
				New = ModelFactory()->CreateModel(E->OpenArgs);
				AssociateModel(New, true);
			}

			switch (E->PostCreationOperation)
			{
			case NewTabOperationArgs::PostNewTabOperation::None:
				break;
			case NewTabOperationArgs::PostNewTabOperation::New:
				ModelController()->New(New);
				break;
			case NewTabOperationArgs::PostNewTabOperation::Open:
				Debug::Assert(E->OpenArgs != nullptr);
				break;
			case NewTabOperationArgs::PostNewTabOperation::OpenList:
				ModelController()->Open(New);
				break;
			case NewTabOperationArgs::PostNewTabOperation::LoadFromDisk:
				{
					if (PREFERENCES->FetchSettingAsInt("LoadScriptUpdateExistingScripts", "General") == 0)
					{
						New = ModelFactory()->CreateModel(nullptr);
						ModelController()->New(New);
						AssociateModel(New, true);
					}
					else
					{
						try
						{
							StreamReader^ FileParser = gcnew StreamReader(E->PathToFile);
							String^ Contents = FileParser->ReadToEnd()->Replace("\r\n", "\n");
							FileParser->Close();

							ObScriptSemanticAnalysis::AnalysisData^ Data = gcnew ObScriptSemanticAnalysis::AnalysisData;
							Data->PerformAnalysis(Contents, ObScriptSemanticAnalysis::ScriptType::None,
												  ObScriptSemanticAnalysis::AnalysisData::Operation::None, nullptr);

							if (Data->Name != "")
							{
								CString CEID(Data->Name);
								ComponentDLLInterface::ScriptData* Data = NativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(CEID.c_str());

								if (Data)
								{
									New = ModelFactory()->CreateModel(Data);
								}
								else
								{
									New = ModelFactory()->CreateModel(nullptr);
									ModelController()->New(New);
								}
							}
						}
						catch (Exception^ Excp)
						{
							DebugPrint("Couldn't read script name from file " + E->PathToFile + " for script updating!\n\tException: " + Excp->Message, true);
							New = ModelFactory()->CreateModel(nullptr);
							ModelController()->New(New);
						}

						ModelController()->LoadFromDisk(New, E->PathToFile);
						AssociateModel(New, true);
					}
				}

				break;
			case NewTabOperationArgs::PostNewTabOperation::SetText:
				ModelController()->New(New);
				ModelController()->SetText(New, E->NewText, true);
				break;
			}
		}


		// ConcreteWorkspaceViewController
		void ConcreteWorkspaceViewController::SetModifiedIndicator(IWorkspaceView^ View, IWorkspaceModel^ Model, bool Modified)
		{
			Debug::Assert(View != nullptr);
			Debug::Assert(Model != nullptr);

			DotNetBar::SuperTabItem^ Tab = ((ConcreteWorkspaceView^)View)->GetTab(Model);
			Debug::Assert(Tab != nullptr);
			Tab->ImageIndex = (int)Modified;
		}

		void ConcreteWorkspaceViewController::BubbleKeyDownEvent(IWorkspaceView^ View, KeyEventArgs^ E)
		{
			Debug::Assert(View != nullptr);
			ConcreteWorkspaceView^ Concrete = (ConcreteWorkspaceView^)View;

			switch (E->KeyCode)
			{
			case Keys::T:
				if (E->Modifiers == Keys::Control)
					Concrete->NewTab(gcnew NewTabOperationArgs);

				break;
			case Keys::PageDown:
				if (E->Modifiers == Keys::Control)
					Concrete->SelectNextTab();

				break;
			case Keys::PageUp:
				if (E->Modifiers == Keys::Control)
					Concrete->SelectPreviousTab();

				break;
			case Keys::Tab:
				if (E->Control == true && E->Shift == false)
					Concrete->SelectNextTab();
				else if (E->Control == true && E->Shift == true)
					Concrete->SelectPreviousTab();

				break;
			case Keys::S:
				if (E->Control && E->Shift)
					Concrete->SaveAll();

				break;
			case Keys::F4:
				if (E->Modifiers == Keys::Control)
				{
					IWorkspaceModel^ Active = Concrete->GetActiveModel();
					if (Concrete->ModelController->Close(Active))
						Concrete->DissociateModel(Active, true);
				}

				break;
			case Keys::D1:
			case Keys::D2:
			case Keys::D3:
			case Keys::D4:
			case Keys::D5:
			case Keys::D6:
			case Keys::D7:
			case Keys::D8:
			case Keys::D9:
				if (E->Modifiers == Keys::Control)
					Concrete->SelectTab(E->KeyCode);

				break;
			}
		}

		void ConcreteWorkspaceViewController::Jump(IWorkspaceView^ View, String^ ScriptEditorID)
		{
			TODO("implement jumping");
			Debug::Assert(View != nullptr);
			ConcreteWorkspaceView^ Concrete = (ConcreteWorkspaceView^)View;
		}

		void ConcreteWorkspaceViewController::FindReplaceAll(IWorkspaceView^ View, TextEditors::IScriptTextEditor::FindReplaceOperation Operation, String^ Query, String^ Replacement, UInt32 Options)
		{
			Debug::Assert(View != nullptr);
			ConcreteWorkspaceView^ Concrete = (ConcreteWorkspaceView^)View;

			for each (auto Itr in Concrete->AssociatedModels)
				Concrete->ModelController->FindReplace(Itr.Key, Operation, Query, Replacement, Options);
		}

		void ConcreteWorkspaceViewController::Redraw(IWorkspaceView^ View)
		{
			Debug::Assert(View != nullptr);
			ConcreteWorkspaceView^ Concrete = (ConcreteWorkspaceView^)View;
			Concrete->Redraw();
		}

		// ConcreteWorkspaceViewFactory
		ConcreteWorkspaceViewFactory::ConcreteWorkspaceViewFactory()
		{
			Allocations = gcnew List < ConcreteWorkspaceView^ >;
		}

		ConcreteWorkspaceViewFactory::~ConcreteWorkspaceViewFactory()
		{
			for each (auto Itr in Allocations)
				delete Itr;

			Allocations->Clear();
		}

		IWorkspaceView^ ConcreteWorkspaceViewFactory::CreateView(int X, int Y, int Width, int Height)
		{
			Rectangle Bounds(X, Y, Width, Height);
			ConcreteWorkspaceView^ New = gcnew ConcreteWorkspaceView((ConcreteWorkspaceViewController^)CreateController(),
																	 this,
																	 Bounds);
			Allocations->Add(New);
			return New;
		}

		IWorkspaceViewController^ ConcreteWorkspaceViewFactory::CreateController()
		{
			return gcnew ConcreteWorkspaceViewController();
		}

		void ConcreteWorkspaceViewFactory::Remove(ConcreteWorkspaceView^ Allocation)
		{
			Allocations->Remove(Allocation);
		}
	}
}