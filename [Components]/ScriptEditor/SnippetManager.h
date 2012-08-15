#pragma once

#include "ScriptParser.h"

namespace ConstructionSetExtender
{
	namespace IntelliSense
	{
		using namespace System::Runtime::Serialization;

		[DataContract]
		ref class CodeSnippet
		{
		public:
			[DataContract]
			ref struct VariableInfo
			{
				[DataMember]
				String^								Name;
				[DataMember]
				ScriptParser::VariableType			Type;

				VariableInfo(String^ Name, ScriptParser::VariableType Type);
			};

			[DataMember]
			String^						Name;
			[DataMember]
			String^						Shorthand;
			[DataMember]
			String^						Description;
			[DataMember]
			List<VariableInfo^>^		Variables;
			[DataMember]
			String^						Code;

			CodeSnippet();
			~CodeSnippet();

			void						AddVariable(String^ Name, ScriptParser::VariableType Type);
			void						AddVariable(VariableInfo^ Var);
			void						RemoveVariable(VariableInfo^ Var);
			VariableInfo^				LookupVariable(String^ Name);

			void						Save(String^ Path);
			static CodeSnippet^			Load(String^ FullPath);
		};

		ref class CodeSnippetCollection
		{
		public:
			List<CodeSnippet^>^			LoadedSnippets;

			CodeSnippetCollection();
			CodeSnippetCollection(List<CodeSnippet^>^ Source);
			~CodeSnippetCollection();

			void						Add(CodeSnippet^ In);
			void						Remove(CodeSnippet^ In);
			CodeSnippet^				Lookup(String^ Name);

			void						Save(String^ SnippetDirectory);
			void						Load(String^ SnippetDirectory);
		};

		ref class CodeSnippetManagerDialog : public System::Windows::Forms::Form
		{
			System::ComponentModel::IContainer^		components;

			GroupBox^								GroupSnippetData;
			Button^									ButtonApply;

			TextBox^								SnippetCodeBox;
			TextBox^								SnippetDescBox;
			TextBox^								SnippetShorthandBox;
			TextBox^								SnippetNameBox;
			Label^									LabelCode;
			Label^									LabelVariables;
			Label^									LabelDescription;
			Label^									LabelShorthand;
			Label^									LabelName;

			ListView^								SnippetVarList;
			ColumnHeader^							SnippetVarListCHName;
			ColumnHeader^							SnippetVarListCHType;

			System::Windows::Forms::ContextMenuStrip^		SnippetListContextMenu;
			ToolStripMenuItem^								SnippetListCMAddSnippet;
			ToolStripMenuItem^								SnippetListCMRemoveSnippet;

			System::Windows::Forms::ContextMenuStrip^		VarListContextMenu;
			ToolStripMenuItem^								VarListCMAddVariable;
			ToolStripMenuItem^								VarListCMRemoveVariable;
			ToolStripMenuItem^								VarListCMAddVarInt;
			ToolStripMenuItem^								VarListCMAddVarFloat;
			ToolStripMenuItem^								VarListCMAddVarRef;
			ToolStripMenuItem^								VarListCMAddVarString;
			ToolStripMenuItem^								VarListCMAddVarArray;

			ListView^								SnippetList;
			ColumnHeader^							SnippetListCHName;
			ColumnHeader^							SnippetListCHShorthand;

			void									InitializeComponent();

			void									ButtonApply_Click(Object^ Sender, EventArgs^ E);
			void									SnippetListCMAddSnippet_Click(Object^ Sender, EventArgs^ E);
			void									SnippetListCMRemoveSnippet_Click(Object^ Sender, EventArgs^ E);
			void									VarListCMAddVariable_Click(Object^ Sender, EventArgs^ E);
			void									VarListCMRemoveVariable_Click(Object^ Sender, EventArgs^ E);
			void									SnippetList_SelectedIndexChanged(Object^ Sender, EventArgs^ E);
			void									SnippetList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);

			void									PopulateSnippetList();
			void									SetSnippetData(CodeSnippet^ Snippet, ListViewItem^ LVItem);
			void									GetSnippetData(CodeSnippet^ Snippet);
		public:
			CodeSnippetManagerDialog(CodeSnippetCollection^ Data);
			~CodeSnippetManagerDialog();

			property CodeSnippetCollection^			WorkingCopy;
		};
	}
}