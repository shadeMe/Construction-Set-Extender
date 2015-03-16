#include "WorkspaceView.h"
#include "[Common]\CustomInputBox.h"
#include "IntelliSenseDatabase.h"
#include "ScriptEditorPreferences.h"
#include "IntelliSenseInterface.h"
#include "[Common]/ListViewUtilities.h"

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
					bool SameTabStrip = false;
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
								Debug::Assert(Parent != nullptr);

								if (Parent != Source)
								{
									// relocate to the view under the mouse
									Source->DissociateModel(Torn, false);
									Parent->AssociateModel(Torn, true);
									Parent->Focus();

									Relocated = true;
								}
								else
									SameTabStrip = true;
							}
						}
					}

					if (Relocated == false && SameTabStrip == false)
					{
						// create new view and relocate
						Rectangle Bounds = Source->GetBounds(true);
						ConcreteWorkspaceView^ New = (ConcreteWorkspaceView^)Source->ViewFactory->CreateView(E->Location.X, E->Location.Y,
																											 Bounds.Width, Bounds.Height);
						Source->DissociateModel(Torn, false);
						New->AssociateModel(Torn, true);
						New->Focus();

						if (Torn->Initialized)
							New->Enabled = true;
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

			ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &ConcreteWorkspaceView::ScriptEditorPreferences_Saved);

			NavigationStackBackward = gcnew Stack < IWorkspaceModel^ > ;
			NavigationStackForward = gcnew Stack < IWorkspaceModel^ > ;
			FreezeNavigationStacks = false;

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
			EditorTabStrip->Dock = DockStyle::Top;
			EditorTabStrip->MaximumSize = Size(32000, 26);
	//		EditorTabStrip->Size = Size(500, 26);
	//		EditorTabStrip->Anchor = AnchorStyles::Top | AnchorStyles::Right;
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

			ConcreteWorkspaceViewDefineClickHandler(NewTabButton);
			ConcreteWorkspaceViewDefineClickHandler(SortTabsButton);

			DotNetBar::RibbonPredefinedColorSchemes::ChangeOffice2010ColorTable(EditorForm, DotNetBar::Rendering::eOffice2010ColorScheme::Black);
			EditorTabStrip->TabStripColor->Background = gcnew DotNetBar::Rendering::SuperTabLinearGradientColorTable(TabStripGradientColorStart,
																													 TabStripGradientColorEnd);
			TODO("remove tabs on left");

			EditorForm->HelpButton = false;
			EditorForm->Text = SCRIPTEDITOR_TITLE;

			AllowDisposal = false;
			DisallowBinding = false;

			ModelStateChangedDirty = gcnew IWorkspaceModel::StateChangeEventHandler(this, &ConcreteWorkspaceView::ModelStateChangeHandler_Dirty);
			ModelStateChangedByteCodeSize = gcnew IWorkspaceModel::StateChangeEventHandler(this, &ConcreteWorkspaceView::ModelStateChangeHandler_ByteCodeSize);
			ModelStateChangedType = gcnew IWorkspaceModel::StateChangeEventHandler(this, &ConcreteWorkspaceView::ModelStateChangeHandler_Type);
			ModelStateChangedDescription = gcnew IWorkspaceModel::StateChangeEventHandler(this, &ConcreteWorkspaceView::ModelStateChangeHandler_Description);

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
			PREFERENCES->PreferencesSaved += ScriptEditorPreferencesSavedHandler;
			ConcreteWorkspaceViewSubscribeClickEvent(NewTabButton);
			ConcreteWorkspaceViewSubscribeClickEvent(SortTabsButton);

			WorkspaceSplitter = gcnew SplitContainer();
			MessageList = gcnew DoubleBufferedListView();
			FindList = gcnew DoubleBufferedListView();
			BookmarkList = gcnew DoubleBufferedListView();
			SpoilerText = gcnew Label();

			AttachPanel = gcnew Panel();
			AttachPanel->Dock = DockStyle::Fill;
			AttachPanel->BorderStyle = BorderStyle::None;

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

			ToolBarEditMenu = gcnew ToolStripDropDownButton();
			ToolBarEditMenuContents = gcnew ToolStripDropDown();
			ToolBarEditMenuContentsFindReplace = gcnew ToolStripButton();
			ToolBarEditMenuContentsGotoLine = gcnew ToolStripButton();
			ToolBarEditMenuContentsGotoOffset = gcnew ToolStripButton();

			ToolBarRefactorMenu = gcnew ToolStripDropDownButton();
			ToolBarRefactorMenuContents = gcnew ToolStripDropDown();
			ToolBarRefactorMenuContentsDocumentScript = gcnew ToolStripButton();
			ToolBarRefactorMenuContentsRenameVariables = gcnew ToolStripButton();
			ToolBarRefactorMenuContentsModifyVariableIndices = gcnew ToolStripButton();

			WorkspaceSecondaryToolBar = gcnew ToolStrip();
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

			ScriptListBox = gcnew ScriptListDialog();
			FindReplaceBox = gcnew FindReplaceDialog(this);

			Color ForegroundColor = PREFERENCES->LookupColorByKey("ForegroundColor");
			Color BackgroundColor = PREFERENCES->LookupColorByKey("BackgroundColor");
			Color HighlightColor = Color::Maroon;
			Font^ CustomFont = gcnew Font(PREFERENCES->FetchSettingAsString("Font", "Appearance"), PREFERENCES->FetchSettingAsInt("FontSize", "Appearance"), (FontStyle)PREFERENCES->FetchSettingAsInt("FontStyle", "Appearance"));

			OffsetTextViewer = gcnew TextEditors::ScriptOffsetViewer(CustomFont, ForegroundColor, BackgroundColor, HighlightColor, WorkspaceSplitter->Panel1);
			PreprocessorTextViewer = gcnew TextEditors::SimpleTextViewer(CustomFont, ForegroundColor, BackgroundColor, HighlightColor, WorkspaceSplitter->Panel1);
			IntelliSenseView = gcnew IntelliSense::IntelliSenseInterfaceView;

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

			SetupControlImage(ToolBarScriptTypeContentsObject);
			SetupControlImage(ToolBarScriptTypeContentsQuest);
			SetupControlImage(ToolBarScriptTypeContentsMagicEffect);

			SetupControlImage(ToolBarEditMenu);
			SetupControlImage(ToolBarEditMenuContentsFindReplace);
			SetupControlImage(ToolBarEditMenuContentsGotoLine);
			SetupControlImage(ToolBarEditMenuContentsGotoOffset);
			ToolBarEditMenuContentsGotoOffset->ImageTransparentColor = Color::White;

			SetupControlImage(ToolBarRefactorMenu);
			SetupControlImage(ToolBarRefactorMenuContentsDocumentScript);
			SetupControlImage(ToolBarRefactorMenuContentsRenameVariables);
			SetupControlImage(ToolBarRefactorMenuContentsModifyVariableIndices);

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

			ConcreteWorkspaceViewDefineClickHandler(NewTabButton);
			ConcreteWorkspaceViewDefineClickHandler(SortTabsButton);

			ConcreteWorkspaceViewDefineClickHandler(ToolBarNavigationBack);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarNavigationForward);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarSaveAll);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarOptions);

			ConcreteWorkspaceViewDefineClickHandler(ToolBarDumpAllScripts);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarLoadScriptsToTabs);

			ConcreteWorkspaceViewDefineClickHandler(ToolBarMessageList);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarFindList);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarBookmarkList);

			ConcreteWorkspaceViewDefineClickHandler(ToolBarNewScript);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarOpenScript);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarPreviousScript);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarNextScript);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarSaveScript);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarSaveScriptNoCompile);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarSaveScriptAndPlugin);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarRecompileScripts);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarCompileDependencies);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarDeleteScript);

			ConcreteWorkspaceViewDefineClickHandler(ToolBarScriptTypeContentsObject);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarScriptTypeContentsQuest);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarScriptTypeContentsMagicEffect);

			ConcreteWorkspaceViewDefineClickHandler(ToolBarEditMenuContentsFindReplace);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarEditMenuContentsGotoLine);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarEditMenuContentsGotoOffset);

			ConcreteWorkspaceViewDefineClickHandler(ToolBarRefactorMenuContentsDocumentScript);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarRefactorMenuContentsRenameVariables);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarRefactorMenuContentsModifyVariableIndices);

			ConcreteWorkspaceViewDefineClickHandler(ToolBarDumpScript);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarLoadScript);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarShowOffsets);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarShowPreprocessedText);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarSanitizeScriptText);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarBindScript);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarSnippetManager);

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
			ToolBarScriptType->Padding = ToolBarButtonPaddingLarge;

			UpdateScriptTypeControls(IWorkspaceModel::ScriptType::Object);

			ToolBarEditMenuContentsFindReplace->Text = "Find/Replace";
			ToolBarEditMenuContentsGotoLine->Text = "Goto Line";
			ToolBarEditMenuContentsGotoOffset->Text = "Goto Offset";
			ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsFindReplace);
			ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsGotoLine);
			ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsGotoOffset);
			ToolBarEditMenu->Text = "Edit";
			ToolBarEditMenu->DropDown = ToolBarEditMenuContents;
			ToolBarEditMenu->Padding = Padding(0);
			ToolBarEditMenu->Alignment = ToolStripItemAlignment::Right;

			ToolBarRefactorMenuContentsDocumentScript->Text = "Document Script";
			ToolBarRefactorMenuContentsRenameVariables->Text = "Rename Variables";
			ToolBarRefactorMenuContentsModifyVariableIndices->Text = "Modify Variable Indices";
			ToolBarRefactorMenuContents->Items->Add(ToolBarRefactorMenuContentsDocumentScript);
			ToolBarRefactorMenuContents->Items->Add(ToolBarRefactorMenuContentsRenameVariables);
			ToolBarRefactorMenuContents->Items->Add(ToolBarRefactorMenuContentsModifyVariableIndices);
			ToolBarRefactorMenu->Text = "Refactor";
			ToolBarRefactorMenu->DropDown = ToolBarRefactorMenuContents;
			ToolBarRefactorMenu->Padding = Padding(0);
			ToolBarRefactorMenu->Alignment = ToolStripItemAlignment::Right;

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
			WorkspaceSecondaryToolBar->Items->Add(ToolBarRefactorMenu);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarScriptType);
			WorkspaceSecondaryToolBar->ShowItemToolTips = true;

			SpoilerText->Dock = DockStyle::Fill;
			SpoilerText->TextAlign = ContentAlignment::MiddleCenter;
			SpoilerText->Text = "Right, everybody out! Smash the Spinning Jenny! Burn the rolling Rosalind! Destroy the going-up-and-down-a-bit-and-then-moving-along Gertrude! And death to the stupid Prince who grows fat on the profits!";

			MessageList->Dock = DockStyle::Fill;
			MessageList->BorderStyle = BorderStyle::Fixed3D;
			MessageList->Visible = false;
			MessageList->View = View::Details;
			MessageList->MultiSelect = false;
			MessageList->CheckBoxes = false;
			MessageList->FullRowSelect = true;
			MessageList->HideSelection = false;

			FindList->Dock = DockStyle::Fill;
			FindList->BorderStyle = BorderStyle::Fixed3D;
			FindList->Visible = false;
			FindList->View = View::Details;
			FindList->MultiSelect = false;
			FindList->CheckBoxes = false;
			FindList->FullRowSelect = true;
			FindList->HideSelection = false;

			BookmarkList->Dock = DockStyle::Fill;
			BookmarkList->BorderStyle = BorderStyle::Fixed3D;
			BookmarkList->Visible = false;
			BookmarkList->View = View::Details;
			BookmarkList->MultiSelect = false;
			BookmarkList->CheckBoxes = false;
			BookmarkList->FullRowSelect = true;
			BookmarkList->HideSelection = false;

			// ideally, the main toolbar should be the main form's child but that screws with the tab strip's layout
			WorkspaceSplitter->Panel1->Controls->Add(AttachPanel);
			WorkspaceSplitter->Panel1->Controls->Add(WorkspaceMainToolBar);

			WorkspaceSplitter->Panel2->Controls->Add(WorkspaceSecondaryToolBar);
			WorkspaceSplitter->Panel2->Controls->Add(MessageList);
			WorkspaceSplitter->Panel2->Controls->Add(FindList);
			WorkspaceSplitter->Panel2->Controls->Add(BookmarkList);
			WorkspaceSplitter->Panel2->Controls->Add(SpoilerText);

			EditorForm->Controls->Add(WorkspaceSplitter);
			EditorForm->Controls->Add(EditorTabStrip);

			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarNewScript);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarOpenScript);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarPreviousScript);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarNextScript);
			ToolBarSaveScript->ButtonClick += ToolBarSaveScriptClickHandler;
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarSaveScriptNoCompile);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarSaveScriptAndPlugin);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarRecompileScripts);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarDeleteScript);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarCompileDependencies);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarNavigationBack);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarNavigationForward);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarSaveAll);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarOptions);

			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarEditMenuContentsFindReplace);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarEditMenuContentsGotoLine);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarEditMenuContentsGotoOffset);

			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarScriptTypeContentsObject);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarScriptTypeContentsQuest);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarScriptTypeContentsMagicEffect);

			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarRefactorMenuContentsDocumentScript);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarRefactorMenuContentsRenameVariables);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarRefactorMenuContentsModifyVariableIndices);

			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarMessageList);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarFindList);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarBookmarkList);

			ToolBarDumpScript->ButtonClick += ToolBarDumpScriptClickHandler;
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarDumpAllScripts);
			ToolBarLoadScript->ButtonClick += ToolBarLoadScriptClickHandler;
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarLoadScriptsToTabs);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarShowOffsets);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarShowPreprocessedText);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarSanitizeScriptText);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarBindScript);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarSnippetManager);

			if (PREFERENCES->FetchSettingAsInt("UseCSParent", "General"))
			{
				EditorForm->ShowInTaskbar = false;
				EditorForm->Show(gcnew WindowHandleWrapper((IntPtr)NativeWrapper::g_CSEInterfaceTable->EditorAPI.GetMainWindowHandle()));
			}
			else
				EditorForm->Show();

			Enabled = false;

			EditorForm->Location = Point(Bounds.Left, Bounds.Top);
			EditorTabStrip->ResumeLayout();
			EditorForm->ResumeLayout();

			try { WorkspaceSplitter->SplitterDistance = GetBounds(true).Height; }
			catch (...) {}

			AssociatedModels = gcnew ModelTabTableT;

			EditorForm->Tag = int(1);			// safe to handle events
		}

		ConcreteWorkspaceView::~ConcreteWorkspaceView()
		{
			NavigationStackBackward->Clear();
			NavigationStackForward->Clear();
			FreezeNavigationStacks = true;

			for each (auto Itr in AssociatedModels)
			{
				Itr.Value->Tag = nullptr;
				ModelUnsubscribeEvents(Itr.Key);
				delete Itr.Key;
			}

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
			ConcreteWorkspaceViewUnsubscribeClickEvent(NewTabButton);
			ConcreteWorkspaceViewUnsubscribeClickEvent(SortTabsButton);
			PREFERENCES->PreferencesSaved -= ScriptEditorPreferencesSavedHandler;

			delete EditorTabStrip;
			delete NewTabButton->Image;
			delete NewTabButton;
			delete SortTabsButton;

			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarNewScript);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarOpenScript);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarPreviousScript);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarNextScript);
			ToolBarSaveScript->ButtonClick -= ToolBarSaveScriptClickHandler;
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarSaveScriptNoCompile);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarSaveScriptAndPlugin);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarRecompileScripts);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarDeleteScript);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarCompileDependencies);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarNavigationBack);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarNavigationForward);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarSaveAll);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarOptions);

			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarEditMenuContentsFindReplace);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarEditMenuContentsGotoLine);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarEditMenuContentsGotoOffset);

			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarScriptTypeContentsObject);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarScriptTypeContentsQuest);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarScriptTypeContentsMagicEffect);

			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarRefactorMenuContentsDocumentScript);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarRefactorMenuContentsRenameVariables);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarRefactorMenuContentsModifyVariableIndices);

			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarMessageList);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarFindList);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarBookmarkList);

			ToolBarDumpScript->ButtonClick -= ToolBarDumpScriptClickHandler;
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarDumpAllScripts);
			ToolBarLoadScript->ButtonClick -= ToolBarLoadScriptClickHandler;
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarLoadScriptsToTabs);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarShowOffsets);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarShowPreprocessedText);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarSanitizeScriptText);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarBindScript);
			ConcreteWorkspaceViewUnsubscribeClickEvent(ToolBarSnippetManager);

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
			DisposeControlImage(ToolBarEditMenuContentsFindReplace);
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
			delete ToolBarEditMenuContentsFindReplace;
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

			delete OffsetTextViewer;
			delete PreprocessorTextViewer;
			delete ScriptListBox;
			delete FindReplaceBox;
			delete IntelliSenseView;

			ScriptListBox = nullptr;
			FindReplaceBox = nullptr;
			OffsetTextViewer = nullptr;
			PreprocessorTextViewer = nullptr;
			IntelliSenseView = nullptr;

			delete AttachPanel;

			WorkspaceSplitter->Panel1->Controls->Clear();
			WorkspaceSplitter->Panel2->Controls->Clear();

			delete WorkspaceSplitter;
			delete MessageList;
			delete FindList;
			delete BookmarkList;
			delete SpoilerText;

			ViewFactory->Remove(this);

			ViewController = nullptr;
			ViewFactory = nullptr;

			PREFERENCES->SaveINI();

			EditorForm->ForceClose();
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

			CloseAll();
		}

		void ConcreteWorkspaceView::EditorForm_KeyDown(Object^ Sender, KeyEventArgs^ E)
		{
			Controller->BubbleKeyDownEvent(this, E);
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

			IWorkspaceModel^ Model = GetModel((DotNetBar::SuperTabItem^)E->Tab);	// clicking on the close button doesn't change the active tab
			if (Model->Dirty)
				SelectTab(GetTab(Model));

			if (Model->Controller->Close(Model))
				DissociateModel(Model, true);
		}

		void ConcreteWorkspaceView::ScriptStrip_SelectedTabChanged(Object^ Sender, DotNetBar::SuperTabStripSelectedTabChangedEventArgs^ E)
		{
			if (DisallowBinding)
				return;

			if (E->NewValue == nullptr)
				return;

			IWorkspaceModel^ Old = nullptr;
			IWorkspaceModel^ New = GetModel((DotNetBar::SuperTabItem^)E->NewValue);

			if (E->OldValue)
				Old = GetModel((DotNetBar::SuperTabItem^)E->OldValue);

			if (FreezeNavigationStacks == false)
			{
				if (Old)
					NavigationStackBackward->Push(Old);

				NavigationStackForward->Clear();
			}

			BeginUpdate();

			if (Old)
				Old->Controller->Unbind(Old);

			New->Controller->Bind(New, this);

			EndUpdate();
		}

		void ConcreteWorkspaceView::ScriptStrip_TabRemoved(Object^ Sender, DotNetBar::SuperTabStripTabRemovedEventArgs^ E)
		{
			;//
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
						if (Model->Dirty)
							SelectTab(GetTab(Model));

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

		void ConcreteWorkspaceView::ToolBarNavigationBack_Click(Object^ Sender, EventArgs^ E)
		{
			try
			{
				FreezeNavigationStacks = true;

				IWorkspaceModel^ Current = GetActiveModel();
				if (NavigationStackBackward->Count)
				{
					IWorkspaceModel^ Previous = NavigationStackBackward->Pop();

					NavigationStackForward->Push(Current);
					SelectTab(GetTab(Previous));
				}
			}
			finally
			{
				FreezeNavigationStacks = false;
			}
		}

		void ConcreteWorkspaceView::ToolBarNavigationForward_Click(Object^ Sender, EventArgs^ E)
		{
			try
			{
				FreezeNavigationStacks = true;

				IWorkspaceModel^ Current = GetActiveModel();
				if (NavigationStackForward->Count)
				{
					IWorkspaceModel^ Next = NavigationStackForward->Pop();

					NavigationStackBackward->Push(Current);
					SelectTab(GetTab(Next));
				}
			}
			finally
			{
				FreezeNavigationStacks = false;
			}
		}

		void ConcreteWorkspaceView::ToolBarSaveAll_Click(Object^ Sender, EventArgs^ E)
		{
			SaveAll();
		}

		void ConcreteWorkspaceView::ToolBarOptions_Click(Object^ Sender, EventArgs^ E)
		{
			PREFERENCES->LoadINI();
			PREFERENCES->Show();
		}

		void ConcreteWorkspaceView::ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E)
		{
			Font^ CustomFont = gcnew Font(PREFERENCES->FetchSettingAsString("Font", "Appearance"),
										  PREFERENCES->FetchSettingAsInt("FontSize", "Appearance"),
										  (FontStyle)PREFERENCES->FetchSettingAsInt("FontStyle", "Appearance"));
			Color ForegroundColor = PREFERENCES->LookupColorByKey("ForegroundColor");
			Color BackgroundColor = PREFERENCES->LookupColorByKey("BackgroundColor");

			OffsetTextViewer->SetFont(CustomFont);
			OffsetTextViewer->SetForegroundColor(ForegroundColor);
			OffsetTextViewer->SetBackgroundColor(BackgroundColor);

			PreprocessorTextViewer->SetFont(CustomFont);
			PreprocessorTextViewer->SetForegroundColor(ForegroundColor);
			PreprocessorTextViewer->SetBackgroundColor(BackgroundColor);

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

		void ConcreteWorkspaceView::ToolBarMessageList_Click(Object^ Sender, EventArgs^ E)
		{
			BeginUpdate();

			ToggleBookmarkList(false);
			ToggleFindResultList(false);

			ToggleMessageList(MessageList->Visible == false);
			ToggleSecondaryPanel(MessageList->Visible);

			EndUpdate();
		}

		void ConcreteWorkspaceView::ToolBarFindList_Click(Object^ Sender, EventArgs^ E)
		{
			BeginUpdate();

			ToggleBookmarkList(false);
			ToggleMessageList(false);

			ToggleFindResultList(FindList->Visible == false);
			ToggleSecondaryPanel(FindList->Visible);

			EndUpdate();
		}

		void ConcreteWorkspaceView::ToolBarBookmarkList_Click(Object^ Sender, EventArgs^ E)
		{
			BeginUpdate();

			ToggleFindResultList(false);
			ToggleMessageList(false);

			ToggleBookmarkList(BookmarkList->Visible == false);
			ToggleSecondaryPanel(BookmarkList->Visible);

			EndUpdate();
		}

		void ConcreteWorkspaceView::ToolBarNewScript_Click(Object^ Sender, EventArgs^ E)
		{
			switch (Control::ModifierKeys)
			{
			case Keys::Control:
				{
					NewTabOperationArgs^ Args = gcnew NewTabOperationArgs;
					Args->PostCreationOperation = NewTabOperationArgs::PostNewTabOperation::New;
					NewTab(Args);
				}

				break;
			case Keys::Shift:
				{
					Rectangle Bounds = GetBounds(false);
					ConcreteWorkspaceView^ New = (ConcreteWorkspaceView^)ViewFactory->CreateView(Bounds.X, Bounds.Y, Bounds.Width, Bounds.Height);
					IWorkspaceModel^ Model = ModelFactory()->CreateModel(nullptr);

					New->AssociateModel(Model, true);
				}

				break;
			default:
				ModelController()->New(GetActiveModel());
				break;
			}
		}

		void ConcreteWorkspaceView::ToolBarOpenScript_Click(Object^ Sender, EventArgs^ E)
		{
			switch (Control::ModifierKeys)
			{
			case Keys::Control:
				{
					NewTabOperationArgs^ Args = gcnew NewTabOperationArgs;
					Args->PostCreationOperation = NewTabOperationArgs::PostNewTabOperation::OpenList;
					NewTab(Args);
				}

				break;
			default:
				ShowOpenDialog();
				break;
			}
		}

		void ConcreteWorkspaceView::ToolBarPreviousScript_Click(Object^ Sender, EventArgs^ E)
		{
			ModelController()->Previous(GetActiveModel());
		}

		void ConcreteWorkspaceView::ToolBarNextScript_Click(Object^ Sender, EventArgs^ E)
		{
			ModelController()->Next(GetActiveModel());
		}

		void ConcreteWorkspaceView::ToolBarSaveScript_Click(Object^ Sender, EventArgs^ E)
		{
			if (ModelController()->Save(GetActiveModel(), IWorkspaceModel::SaveOperation::Default) == false)
				ShowMessageList();
		}

		void ConcreteWorkspaceView::ToolBarSaveScriptNoCompile_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();
			if (Active->New == true || Active->Initialized == false)
			{
				MessageBox::Show("You may only perform this operation on an existing script.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Exclamation);

				return;
			}

			if (ModelController()->Save(GetActiveModel(), IWorkspaceModel::SaveOperation::NoCompile) == false)
				ShowMessageList();
		}

		void ConcreteWorkspaceView::ToolBarSaveScriptAndPlugin_Click(Object^ Sender, EventArgs^ E)
		{
			if (ModelController()->Save(GetActiveModel(), IWorkspaceModel::SaveOperation::SavePlugin) == false)
				ShowMessageList();
		}

		void ConcreteWorkspaceView::ToolBarRecompileScripts_Click(Object^ Sender, EventArgs^ E)
		{
			if (MessageBox::Show("Are you sure you want to recompile all the scripts in the active plugin?",
				SCRIPTEDITOR_TITLE,
				MessageBoxButtons::YesNo,
				MessageBoxIcon::Exclamation) == DialogResult::Yes)
			{
				NativeWrapper::g_CSEInterfaceTable->ScriptEditor.RecompileScripts();
				MessageBox::Show("All active scripts recompiled. Results have been logged to the console.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Information);
			}
		}

		void ConcreteWorkspaceView::ToolBarCompileDependencies_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();
			if (Active->New == false && Active->Initialized == true)
			{
				ModelController()->CompileDepends(Active);
				MessageBox::Show("Operation complete!\n\nScript variables used as condition parameters will need to be corrected manually. The results have been logged to the console.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Information);
			}
			else if (Active->Initialized == true)
			{
				MessageBox::Show("The current script needs to be compiled before its dependencies can be updated.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Exclamation);
			}
		}

		void ConcreteWorkspaceView::ToolBarDeleteScript_Click(Object^ Sender, EventArgs^ E)
		{
			ShowDeleteDialog();
		}

		void ConcreteWorkspaceView::ToolBarScriptTypeContentsObject_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();
			ModelController()->SetType(Active, IWorkspaceModel::ScriptType::Object);
		}

		void ConcreteWorkspaceView::ToolBarScriptTypeContentsQuest_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();
			ModelController()->SetType(Active, IWorkspaceModel::ScriptType::Quest);
		}

		void ConcreteWorkspaceView::ToolBarScriptTypeContentsMagicEffect_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();
			ModelController()->SetType(Active, IWorkspaceModel::ScriptType::MagicEffect);
		}

		void ConcreteWorkspaceView::ToolBarEditMenuContentsFindReplace_Click(Object^ Sender, EventArgs^ E)
		{
			ShowFindReplaceDialog(false);
		}

		void ConcreteWorkspaceView::ToolBarEditMenuContentsGotoLine_Click(Object^ Sender, EventArgs^ E)
		{
			GotoLine();
		}

		void ConcreteWorkspaceView::ToolBarEditMenuContentsGotoOffset_Click(Object^ Sender, EventArgs^ E)
		{
			GotoOffset();
		}

		void ConcreteWorkspaceView::ToolBarDumpScript_Click(Object^ Sender, EventArgs^ E)
		{
			SaveFileDialog^ SaveManager = gcnew SaveFileDialog();
			IWorkspaceModel^ Active = GetActiveModel();

			SaveManager->DefaultExt = "*.txt";
			SaveManager->Filter = "Text Files|*.txt|All files (*.*)|*.*";
			SaveManager->FileName = Active->LongDescription;
			SaveManager->RestoreDirectory = true;

			if (SaveManager->ShowDialog() == DialogResult::OK && SaveManager->FileName->Length > 0)
				ModelController()->SaveToDisk(Active, SaveManager->FileName, true, gcnew String("txt"));
		}

		void ConcreteWorkspaceView::ToolBarLoadScript_Click(Object^ Sender, EventArgs^ E)
		{
			OpenFileDialog^ LoadManager = gcnew OpenFileDialog();
			IWorkspaceModel^ Active = GetActiveModel();

			LoadManager->DefaultExt = "*.txt";
			LoadManager->Filter = "Text Files|*.txt|All files (*.*)|*.*";
			LoadManager->RestoreDirectory = true;

			if (LoadManager->ShowDialog() == DialogResult::OK && LoadManager->FileName->Length > 0)
				ModelController()->LoadFromDisk(Active, LoadManager->FileName);
		}

		void ConcreteWorkspaceView::ToolBarShowOffsets_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();
			if (ToolBarShowOffsets->Checked)
			{
				int Caret = OffsetTextViewer->Hide();
				ModelController()->SetCaret(Active, Caret);
				ModelController()->AcquireInputFocus(Active);
				ToolBarShowOffsets->Checked = false;
			}
			else
			{
				String^ Text = "";
				UInt32 Data = 0, Length = 0;

				if (ModelController()->GetOffsetViewerData(Active, Text, Data, Length))
				{
					OffsetTextViewer->InitializeViewer(Text, Data, Length);
					int Caret = ModelController()->GetCaret(Active);
					if (Caret == -1)
						Caret = 0;

					if (OffsetTextViewer->Show(Caret))
					{
						ToolBarShowOffsets->Checked = true;
						return;
					}
				}

				MessageBox::Show("Offset Viewer couldn't be initialized!\n\nPlease recompile the current script.",
								 SCRIPTEDITOR_TITLE, MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
			}
		}

		void ConcreteWorkspaceView::ToolBarShowPreprocessedText_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();
			if (ToolBarShowPreprocessedText->Checked)
			{
				PreprocessorTextViewer->Hide();
				ModelController()->AcquireInputFocus(Active);
				ToolBarShowPreprocessedText->Checked = false;
			}
			else
			{
				bool Preprocess = false;
				String^ PreprocessedText = ModelController()->GetText(Active, true, Preprocess);
				if (Preprocess)
				{
					int Caret = ModelController()->GetCaret(Active);
					if (Caret == -1)
						Caret = 0;

					PreprocessorTextViewer->Show(PreprocessedText, Caret);
					ToolBarShowPreprocessedText->Checked = true;
				}
				else
				{
					MessageBox::Show("The preprocessing operation was unsuccessful.", SCRIPTEDITOR_TITLE, MessageBoxButtons::OK, MessageBoxIcon::Error);
					ShowMessageList();
				}
			}
		}

		void ConcreteWorkspaceView::ToolBarSanitizeScriptText_Click(Object^ Sender, EventArgs^ E)
		{
			if (ModelController()->Sanitize(GetActiveModel()) == false)
			{
				MessageBox::Show("The script contains structural errors that must be fixed before this operation can be performed.",
								 SCRIPTEDITOR_TITLE, MessageBoxButtons::OK, MessageBoxIcon::Error);
			}
		}

		void ConcreteWorkspaceView::ToolBarBindScript_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();
			if (Active->New == true || Active->Initialized == false)
			{
				MessageBox::Show("You may only perform this operation on an existing script.", SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
			}
			else
			{
				ModelController()->BindToForm(Active);
			}
		}

		void ConcreteWorkspaceView::ToolBarSnippetManager_Click(Object^ Sender, EventArgs^ E)
		{
			IntelliSense::ISDB->ShowCodeSnippetManager();
		}

		void ConcreteWorkspaceView::ToolBarRefactorMenuContentsDocumentScript_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();

			if (Active->Dirty)
			{
				MessageBox::Show("The current script must be compiled to perform this operation.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Exclamation);

				return;
			}

			ModelController()->ApplyRefactor(Active, IWorkspaceModel::RefactorOperation::DocumentScript, nullptr);
		}

		void ConcreteWorkspaceView::ToolBarRefactorMenuContentsRenameVariables_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();

			if (Active->Dirty)
			{
				MessageBox::Show("The current script must be compiled to perform this operation.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Exclamation);

				return;
			}

			if (ModelController()->ApplyRefactor(Active, IWorkspaceModel::RefactorOperation::RenameVariables, nullptr))
			{
				MessageBox::Show("Variables have been renamed. Scripts referencing them (current script included) will have to be manually updated with the new identifiers.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Information);
			}
		}

		void ConcreteWorkspaceView::ToolBarRefactorMenuContentsModifyVariableIndices_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();

			if (Active->Dirty)
			{
				MessageBox::Show("The current script must be compiled to perform this operation.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Exclamation);

				return;
			}

			if (ModelController()->ApplyRefactor(Active, IWorkspaceModel::RefactorOperation::ModifyVariableIndices, nullptr))
			{
				if (PREFERENCES->FetchSettingAsInt("RecompileVarIdx", "General"))
				{
					ToolBarSaveScript->PerformButtonClick();
					ToolBarCompileDependencies->PerformClick();
				}
			}
		}

		void ConcreteWorkspaceView::ModelStateChangeHandler_Dirty(IWorkspaceModel^ Sender, IWorkspaceModel::StateChangeEventArgs^ E)
		{
			DotNetBar::SuperTabItem^ Tab = GetTab(Sender);
			Debug::Assert(Tab != nullptr);

			Tab->ImageIndex = (int)E->Dirty;
		}

		void ConcreteWorkspaceView::ModelStateChangeHandler_ByteCodeSize(IWorkspaceModel^ Sender, IWorkspaceModel::StateChangeEventArgs^ E)
		{
			if (GetActiveModel() == Sender)
			{
				ToolBarByteCodeSize->Value = E->ByteCodeSize;
				ToolBarByteCodeSize->ToolTipText = String::Format("Compiled Script Size: {0:F2} KB", (float)(E->ByteCodeSize / 1024.0));
			}
		}

		void ConcreteWorkspaceView::ModelStateChangeHandler_Type(IWorkspaceModel^ Sender, IWorkspaceModel::StateChangeEventArgs^ E)
		{
			if (GetActiveModel() == Sender)
				UpdateScriptTypeControls(E->Type);
		}

		void ConcreteWorkspaceView::ModelStateChangeHandler_Description(IWorkspaceModel^ Sender, IWorkspaceModel::StateChangeEventArgs^ E)
		{
			DotNetBar::SuperTabItem^ Tab = GetTab(Sender);
			Debug::Assert(Tab != nullptr);

			Tab->Text = E->ShortDescription;
			Tab->Tooltip = E->LongDescription;

			if (GetActiveModel() == Sender)
				EditorForm->Text = E->LongDescription + " - " + SCRIPTEDITOR_TITLE;
		}

		IWorkspaceModelController^ ConcreteWorkspaceView::ModelController()
		{
			Debug::Assert(AssociatedModels->Count != 0);
			ModelTabTableT::Enumerator Itr = AssociatedModels->GetEnumerator();
			Itr.MoveNext();

			return Itr.Current.Key->Controller;
		}

		IWorkspaceModelFactory^ ConcreteWorkspaceView::ModelFactory()
		{
			Debug::Assert(AssociatedModels->Count != 0);
			ModelTabTableT::Enumerator Itr = AssociatedModels->GetEnumerator();
			Itr.MoveNext();

			return Itr.Current.Key->Factory;
		}

		bool ConcreteWorkspaceView::IsModelAssociated(IWorkspaceModel^ Model)
		{
			return AssociatedModels->ContainsKey(Model);
		}

		void ConcreteWorkspaceView::AssociateModel(IWorkspaceModel^ Model, bool Bind)
		{
			Debug::Assert(IsModelAssociated(Model) == false);

			DotNetBar::SuperTabItem^ Tab = gcnew DotNetBar::SuperTabItem;
			Tab->Text = Model->ShortDescription;
			Tab->ImageIndex = (int)Model->Dirty;
			Tab->Tag = Model;

			ModelSubscribeEvents(Model);
			AssociatedModels->Add(Model, Tab);

			// when the model being associated is the very first one (for the view), the addition of the tab to the strip automatically triggers its selection
			// so it must be added to the table beforehand
			EditorTabStrip->Tabs->Add(Tab);

			if (Bind)
				SelectTab(Tab);
		}

		void ConcreteWorkspaceView::DissociateModel(IWorkspaceModel^ Model, bool Destroy)
		{
			Debug::Assert(IsModelAssociated(Model) == true);

			Model->Controller->Unbind(Model);

			RemoveFromNavigationStacks(Model);
			ModelUnsubscribeEvents(Model);

			DotNetBar::SuperTabItem^ Tab = GetTab(Model);

			Tab->Tag = nullptr;
			delete Tab;

			AssociatedModels->Remove(Model);

			if (Destroy)
				delete Model;

			EditorTabStrip->Tabs->Remove(Tab);

			// close form if there are no more tabs left
			if (GetTabCount() == 0 && DisallowBinding == false)
			{
				AllowDisposal = true;
				EditorForm->Close();
			}
		}

		IWorkspaceModel^ ConcreteWorkspaceView::GetModel(DotNetBar::SuperTabItem^ Tab)
		{
			return (IWorkspaceModel^)Tab->Tag;
		}

		IWorkspaceModel^ ConcreteWorkspaceView::GetModel(String^ Description)
		{
			for each (auto Itr in AssociatedModels)
			{
				if (String::Compare(Itr.Key->ShortDescription, Description, true) == 0)
					return Itr.Key;
			}

			return nullptr;
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

		void ConcreteWorkspaceView::ToggleSecondaryPanel(bool State)
		{
			if (State)
				WorkspaceSplitter->SplitterDistance = GetBounds(false).Height / 1.5;
			else
				WorkspaceSplitter->SplitterDistance = GetBounds(false).Height;
		}

		void ConcreteWorkspaceView::UpdateScriptTypeControls(IWorkspaceModel::ScriptType New)
		{
			switch (New)
			{
			case IWorkspaceModel::ScriptType::Object:
				ToolBarScriptType->Text = "Object Script";
				ToolBarScriptType->Image = ToolBarScriptTypeContentsObject->Image;
				break;
			case IWorkspaceModel::ScriptType::Quest:
				ToolBarScriptType->Text = "Quest Script";
				ToolBarScriptType->Image = ToolBarScriptTypeContentsQuest->Image;
				break;
			case IWorkspaceModel::ScriptType::MagicEffect:
				ToolBarScriptType->Text = "Magic Effect Script";
				ToolBarScriptType->Image = ToolBarScriptTypeContentsMagicEffect->Image;
				break;
			}
		}

		void ConcreteWorkspaceView::ShowOpenDialog()
		{
			List<String^>^ Selection = gcnew List < String^ > ;
			if (ScriptListBox->Show(ScriptListDialog::ShowOperation::Open, GetActiveModel()->LongDescription, Selection))
			{
				BeginUpdate();
				NewTabOperationArgs^ E = gcnew NewTabOperationArgs;
				int i = 0;
				for each (auto Itr in Selection)
				{
					CString EID(Itr);

					if (i == 0)
					{
						ModelController()->Open(GetActiveModel(),
												NativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(EID.c_str()));
					}
					else
					{
						E->PostCreationOperation = NewTabOperationArgs::PostNewTabOperation::Open;
						E->OpenArgs = NativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(EID.c_str());
						E->BindPostCreation = false;
						Debug::Assert(E->OpenArgs != nullptr);
						NewTab(E);
					}

					i++;
				}
				EndUpdate();
			}
		}

		void ConcreteWorkspaceView::ShowDeleteDialog()
		{
			List<String^>^ Selection = gcnew List < String^ >;
			if (ScriptListBox->Show(ScriptListDialog::ShowOperation::Delete, "", Selection))
			{
				for each (auto Itr in Selection)
				{
					CString EID(Itr);
					NativeWrapper::g_CSEInterfaceTable->ScriptEditor.DeleteScript(EID.c_str());
				}
			}
		}

		void ConcreteWorkspaceView::ShowFindReplaceDialog(bool PerformSearch)
		{
			IWorkspaceModel^ Active = GetActiveModel();
			String^ Query = Active->Controller->GetSelection(Active);
			if (Query == "")
				Query = Active->Controller->GetCaretToken(Active);

			FindReplaceBox->Show(Query, false, PerformSearch);
		}

		void ConcreteWorkspaceView::GotoLine()
		{
			if (ToolBarShowOffsets->Checked)
			{
				MessageBox::Show("This operation can only be performed in the text editor and the preprocessed text viewer.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Exclamation);
			}
			else
			{
				IWorkspaceModel^ Active = GetActiveModel();
				UInt32 LineCount = 0;
				if (ToolBarShowPreprocessedText->Checked)
					LineCount = PreprocessorTextViewer->GetLineCount();
				else
					LineCount = ModelController()->GetLineCount(Active);

				InputBoxes::InputBoxResult^ Result = InputBoxes::InputBox::Show("Line Number (1 - " + LineCount + ")", "Go To Line");
				if (Result->ReturnCode == DialogResult::Cancel || Result->Text == "")
					return;

				if (ToolBarShowPreprocessedText->Checked)
					PreprocessorTextViewer->JumpToLine(Result->Text);
				else
				{
					int Line = 0;
					if (int::TryParse(Result->Text, Line))
						ModelController()->GotoLine(Active, Line);
				}
			}
		}

		void ConcreteWorkspaceView::GotoOffset()
		{
			if (ToolBarShowOffsets->Checked)
			{
				InputBoxes::InputBoxResult^ Result = InputBoxes::InputBox::Show("Offset (0000 - " + OffsetTextViewer->GetLastOffset().ToString("X4") + ")", "Go To Offset");
				if (Result->ReturnCode == DialogResult::Cancel || Result->Text == "")
					return;

				OffsetTextViewer->JumpToLine(Result->Text);
			}
			else
			{
				MessageBox::Show("This operation can only be performed in the offset viewer.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Exclamation);
			}
		}

		void ConcreteWorkspaceView::SaveAll()
		{
			for each (auto Itr in AssociatedModels)
				ModelController()->Save(Itr.Key, IWorkspaceModel::SaveOperation::Default);
		}

		void ConcreteWorkspaceView::CloseAll()
		{
			List<IWorkspaceModel^>^ Dissociate = gcnew List < IWorkspaceModel^ > ;

			for each (auto Itr in AssociatedModels)
			{
				// switch to dirty models to enable user interaction
				if (Itr.Key->Dirty)
					SelectTab(GetTab(Itr.Key));

				if (ModelController()->Close(Itr.Key))
					Dissociate->Add(Itr.Key);
			}

			DisallowBinding = true;
			BeginUpdate();

			for each (auto Itr in Dissociate)
				DissociateModel(Itr, true);

			Dissociate->Clear();
			DisallowBinding = false;
			EndUpdate();

			if (GetTabCount() == 0)
			{
				AllowDisposal = true;
				EditorForm->Close();
			}
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
				SelectTab(TabIndex);
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
				AssociateModel(New, E->BindPostCreation);
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
				ShowOpenDialog();
				break;
			case NewTabOperationArgs::PostNewTabOperation::LoadFromDisk:
				{
					if (PREFERENCES->FetchSettingAsInt("LoadScriptUpdateExistingScripts", "General") == 0)
					{
						New = ModelFactory()->CreateModel(nullptr);
						ModelController()->New(New);
						AssociateModel(New, E->BindPostCreation);
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
						AssociateModel(New, E->BindPostCreation);
					}
				}

				break;
			case NewTabOperationArgs::PostNewTabOperation::SetText:
				ModelController()->New(New);
				ModelController()->SetText(New, E->NewText, true);
				break;
			}
		}

		void ConcreteWorkspaceView::ModelSubscribeEvents(IWorkspaceModel^ Model)
		{
			Model->StateChangedDirty += ModelStateChangedDirty;
			Model->StateChangedByteCodeSize += ModelStateChangedByteCodeSize;
			Model->StateChangedType += ModelStateChangedType;
			Model->StateChangedDescription += ModelStateChangedDescription;
		}

		void ConcreteWorkspaceView::ModelUnsubscribeEvents(IWorkspaceModel^ Model)
		{
			Model->StateChangedDirty -= ModelStateChangedDirty;
			Model->StateChangedByteCodeSize -= ModelStateChangedByteCodeSize;
			Model->StateChangedType -= ModelStateChangedType;
			Model->StateChangedDescription -= ModelStateChangedDescription;
		}

		void ConcreteWorkspaceView::RemoveFromNavigationStacks(IWorkspaceModel^ Model)
		{
			Stack<IWorkspaceModel^>^ Buffer = gcnew Stack < IWorkspaceModel^ > ;

			while (NavigationStackBackward->Count)
			{
				IWorkspaceModel^ Current = NavigationStackBackward->Pop();
				if (Current != Model)
					Buffer->Push(Current);
			}

			while (Buffer->Count)
				NavigationStackBackward->Push(Buffer->Pop());

			while (NavigationStackForward->Count)
			{
				IWorkspaceModel^ Current = NavigationStackForward->Pop();
				if (Current != Model)
					Buffer->Push(Current);
			}

			while (Buffer->Count)
				NavigationStackForward->Push(Buffer->Pop());
		}

		void ConcreteWorkspaceView::Focus()
		{
			EditorForm->Focus();
		}

		void ConcreteWorkspaceView::ToggleMessageList(bool State)
		{
			if (State)
			{
				ToolBarMessageList->Checked = true;
				MessageList->Visible = true;
				MessageList->BringToFront();
			}
			else
			{
				ToolBarMessageList->Checked = false;
				MessageList->Visible = false;
				MessageList->SendToBack();
			}
		}

		void ConcreteWorkspaceView::ToggleBookmarkList(bool State)
		{
			if (State)
			{
				ToolBarBookmarkList->Checked = true;
				BookmarkList->Visible = true;
				BookmarkList->BringToFront();
			}
			else
			{
				ToolBarBookmarkList->Checked = false;
				BookmarkList->Visible = false;
				BookmarkList->SendToBack();
			}
		}

		void ConcreteWorkspaceView::ToggleFindResultList(bool State)
		{
			if (State)
			{
				ToolBarFindList->Checked = true;
				FindList->Visible = true;
				FindList->BringToFront();
			}
			else
			{
				ToolBarFindList->Checked = false;
				FindList->Visible = false;
				FindList->SendToBack();
			}
		}

		void ConcreteWorkspaceView::ShowMessageList()
		{
			ToggleBookmarkList(false);
			ToggleFindResultList(false);

			ToggleMessageList(true);
			ToggleSecondaryPanel(true);
		}

		// ConcreteWorkspaceViewController
		void ConcreteWorkspaceViewController::AttachModelInternalView(IWorkspaceView^ View, IWorkspaceModel^ Model)
		{
			Debug::Assert(View != nullptr);
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceView^ Concrete = (ConcreteWorkspaceView^)View;

			// we need multiple levels of indirection when swapping the textview to keep the form's layout intact
			// directly adding it to the splitter panel will screw up the toolbar's docking, among other things
			Concrete->AttachPanel->Controls->Add(Model->InternalView);
		}

		void ConcreteWorkspaceViewController::DettachModelInternalView(IWorkspaceView^ View, IWorkspaceModel^ Model)
		{
			Debug::Assert(View != nullptr);
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceView^ Concrete = (ConcreteWorkspaceView^)View;

			Concrete->AttachPanel->Controls->Remove(Model->InternalView);
		}

		void ConcreteWorkspaceViewController::BubbleKeyDownEvent(IWorkspaceView^ View, KeyEventArgs^ E)
		{
			Debug::Assert(View != nullptr);
			ConcreteWorkspaceView^ Concrete = (ConcreteWorkspaceView^)View;
			IWorkspaceModel^ Active = Concrete->GetActiveModel();

			switch (E->KeyCode)
			{
			case Keys::N:
				if (E->Modifiers == Keys::Control)
					Concrete->ModelController()->New(Active);

				break;
			case Keys::O:
				if (E->Modifiers == Keys::Control)
					Concrete->ShowOpenDialog();

				break;
			case Keys::S:
				if (E->Modifiers == Keys::Control)
				{
					if (Concrete->ModelController()->Save(Active, IWorkspaceModel::SaveOperation::Default) == false)
						Concrete->ShowMessageList();
				}
				else if (E->Control && E->Shift)
					Concrete->SaveAll();

				break;
			case Keys::D:
				if (E->Modifiers == Keys::Control)
					Concrete->ShowDeleteDialog();

				break;
			case Keys::Left:
				if (E->Control && E->Alt)
					Concrete->ModelController()->Previous(Active);

				break;
			case Keys::Right:
				if (E->Control && E->Alt)
					Concrete->ModelController()->Next(Active);

				break;
			case Keys::F:
			case Keys::H:
				if (E->Modifiers == Keys::Control)
					Concrete->ShowFindReplaceDialog(true);

				break;
			case Keys::G:
				if (E->Modifiers == Keys::Control)
					Concrete->GotoLine();

				break;
			case Keys::E:
				if (E->Modifiers == Keys::Control)
					Concrete->GotoOffset();

				break;
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
			case Keys::F4:
				if (E->Modifiers == Keys::Control)
				{
					IWorkspaceModel^ Active = Concrete->GetActiveModel();
					if (Active->Controller->Close(Active))
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

		void ConcreteWorkspaceViewController::Jump(IWorkspaceView^ View, IWorkspaceModel^ From, String^ ScriptEditorID)
		{
			Debug::Assert(View != nullptr);
			ConcreteWorkspaceView^ Concrete = (ConcreteWorkspaceView^)View;

			IWorkspaceModel^ Existing = Concrete->GetModel(ScriptEditorID);
			if (Existing)
				Concrete->SelectTab(Concrete->GetTab(Existing));
			else
			{
				CString EID(ScriptEditorID);
				NewTabOperationArgs^ E = gcnew NewTabOperationArgs;

				E->PostCreationOperation = NewTabOperationArgs::PostNewTabOperation::Open;
				E->OpenArgs = NativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(EID.c_str());

				if (E->OpenArgs);
					Concrete->NewTab(E);
			}
		}

		int ConcreteWorkspaceViewController::FindReplace(IWorkspaceView^ View, TextEditors::IScriptTextEditor::FindReplaceOperation Operation,
														  String^ Query, String^ Replacement, UInt32 Options, bool Global)
		{
			Debug::Assert(View != nullptr);
			ConcreteWorkspaceView^ Concrete = (ConcreteWorkspaceView^)View;

			if (Global)
			{
				for each (auto Itr in Concrete->AssociatedModels)
					Concrete->ModelController()->FindReplace(Itr.Key, Operation, Query, Replacement, Options);

				return -1;
			}
			else
			{
				int Hits = Concrete->GetActiveModel()->Controller->FindReplace(Concrete->GetActiveModel(), Operation, Query, Replacement, Options);
				if (Hits)
					Concrete->ToolBarFindList->PerformClick();

				return Hits;
			}
		}

		void ConcreteWorkspaceViewController::Redraw(IWorkspaceView^ View)
		{
			Debug::Assert(View != nullptr);
			ConcreteWorkspaceView^ Concrete = (ConcreteWorkspaceView^)View;
			Concrete->Redraw();
		}

		System::Windows::Forms::DialogResult ConcreteWorkspaceViewController::MessageBox(String^ Message, MessageBoxButtons Buttons, MessageBoxIcon Icon)
		{
			return MessageBox::Show(Message, SCRIPTEDITOR_TITLE, Buttons, Icon);
		}

		void ConcreteWorkspaceViewController::NewTab(IWorkspaceView^ View, NewTabOperationArgs^ E)
		{
			Debug::Assert(View != nullptr);
			ConcreteWorkspaceView^ Concrete = (ConcreteWorkspaceView^)View;
			Concrete->NewTab(E);
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

		void ConcreteWorkspaceViewFactory::Clear()
		{
			// this is totally unbecoming...
			List<ConcreteWorkspaceView^>^ Buffer = gcnew List<ConcreteWorkspaceView^>(Allocations);
			for each (ConcreteWorkspaceView^ Itr in Buffer)
				delete Itr;

			Debug::Assert(Allocations->Count == 0);
			Buffer->Clear();
		}
	}
}