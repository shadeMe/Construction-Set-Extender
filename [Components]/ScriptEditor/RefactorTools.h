#pragma once

namespace ConstructionSetExtender
{
	namespace ScriptEditor
	{
		namespace Refactoring
		{
			public ref class CreateUDFImplementationData
			{
			public:
				ref struct ParameterData
				{
					String^								Name;
					String^								Type;

					ParameterData(String^ Name, String^ Type) :	Name(Name), Type(Type) {}
				};

				List<ParameterData^>^					ParameterList;		// sorted

				CreateUDFImplementationData() : ParameterList(gcnew List<ParameterData^>()) {}
			};

			public ref class CreateUDFImplementationDialog : public System::Windows::Forms::Form
			{
				System::ComponentModel::Container^		components;

				ListView^								ParameterBox;
				Button^									ImplementBox;
				ColumnHeader^							ParameterBoxCHName;
				ColumnHeader^							ParameterBoxCHType;

				void									InitializeComponent();

				void									ImplementBox_Click(Object^ Sender, EventArgs^ E);
				void									ParameterBox_ItemActivate(Object^ Sender, EventArgs^ E);
			public:
				property CreateUDFImplementationData^	ResultData;
				property bool							HasResult;

				CreateUDFImplementationDialog(IntPtr ParentHandle);
			protected:
				~CreateUDFImplementationDialog()
				{
					if (components)
					{
						delete components;
					}
				}
			};

			public ref class EditScriptComponentData
			{
			public:
				ref struct ScriptComponent
				{
					String^								ElementName;
					String^								EditData;

					ScriptComponent(String^ Name, String^ EditData) :	ElementName(Name), EditData(EditData) {}
				};

				List<ScriptComponent^>^				ScriptComponentList;

				EditScriptComponentData() : ScriptComponentList(gcnew List<ScriptComponent^>()) {}

				bool								LookupEditDataByName(String^ ElementName, String^% OutEditData);
			};

			public ref class EditScriptComponentDialog : public System::Windows::Forms::Form
			{
				System::ComponentModel::Container^		components;

				ListView^								ElementList;
				TextBox^								ElementListEditBox;
				Button^									ImplementBox;
				ColumnHeader^							ElementListCHElementName;
				ColumnHeader^							ElementListCHEditData;

				void									InitializeComponent();

				void									ImplementBox_Click(Object^ Sender, EventArgs^ E);
				void									ElementList_ItemActivate(Object^ Sender, EventArgs^ E);
				void									ElementListEditBox_LostFocus(Object^ Sender, EventArgs^ E);

				String^									DefaultItemString;
			public:
				property EditScriptComponentData^		ResultData;
				property bool							HasResult;

				static enum class						OperationType
				{
					e_DocumentScript = 0,
					e_RenameVariables
				};

				EditScriptComponentDialog(IntPtr ParentHandle, String^ ScriptEditorID, OperationType Operation, String^ DefaultItemString);
			protected:
				~EditScriptComponentDialog()
				{
					if (components)
					{
						delete components;
					}
				}
			};
		}
	}
}