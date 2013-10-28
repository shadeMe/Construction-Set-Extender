#include "CodaScriptCommands-Form.h"

namespace ConstructionSetExtender
{
	namespace BGSEEScript
	{
		namespace Commands
		{
			namespace Form
			{
				CodaScriptCommandRegistrarDef("Form")

				CodaScriptCommandPrototypeDef(GetFormByEditorID);
				CodaScriptCommandPrototypeDef(GetFormByFormID);
				CodaScriptCommandPrototypeDef(GetEditorID);
				CodaScriptCommandPrototypeDef(GetFormType);
				CodaScriptCommandPrototypeDef(SetEditorID);
				CodaScriptCommandPrototypeDef(SetFormID);
				CodaScriptCommandPrototypeDef(MarkAsModified);
				CodaScriptCommandPrototypeDef(GetDataHandlerFormList);
				CodaScriptCommandPrototypeDef(GetCellUseList);

				CodaScriptCommandParamData(SetEditorID, 2)
				{
					{ "Form",							ICodaScriptDataStore::kDataType_Reference },
					{ "EditorID",						ICodaScriptDataStore::kDataType_String }
				};

				CodaScriptCommandParamData(SetFormID, 2)
				{
					{ "Form",							ICodaScriptDataStore::kDataType_Reference },
					{ "FormID",							ICodaScriptDataStore::kDataType_Numeric }
				};

				CodaScriptCommandParamData(MarkAsModified, 2)
				{
					{ "Form",							ICodaScriptDataStore::kDataType_Reference },
					{ "Modified Flag",					ICodaScriptDataStore::kDataType_Numeric }
				};

				CodaScriptCommandParamData(GetDataHandlerFormList, 1)
				{
					{ "Form Type",						ICodaScriptDataStore::kDataType_Numeric }
				};

				CodaScriptCommandHandler(GetFormByEditorID)
				{
					CodaScriptStringParameterTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Buffer);

					TESForm* Form = TESForm::LookupByEditorID(Buffer);
					if (Form)
						Result->SetFormID(Form->formID);
					else
						Result->SetFormID(0);

					return true;
				}

				CodaScriptCommandHandler(GetFormByFormID)
				{
					CodaScriptNumericDataTypeT FormID = 0;

					CodaScriptCommandExtractArgs(&FormID);

					TESForm* Form = TESForm::LookupByFormID((UInt32)FormID);
					if (Form)
						Result->SetFormID(Form->formID);
					else
						Result->SetFormID(0);

					return true;
				}

				CodaScriptCommandHandler(GetEditorID)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					Result->SetString((Form->editorID.c_str())?Form->editorID.c_str():"");
					return true;
				}

				CodaScriptCommandHandler(GetFormType)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					Result->SetNumber(Form->formType);
					return true;
				}

				CodaScriptCommandHandler(SetEditorID)
				{
					TESForm* Form = NULL;
					CodaScriptStringParameterTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					if (Buffer)
						Result->SetNumber(Form->SetEditorID(Buffer));

					return true;
				}

				CodaScriptCommandHandler(SetFormID)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT FormID = 0;

					CodaScriptCommandExtractArgs(&Form, &FormID);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					if (FormID && Form->formID != FormID)
						Form->SetFormID(FormID);

					return true;
				}

				CodaScriptCommandHandler(MarkAsModified)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT State = 0;

					CodaScriptCommandExtractArgs(&Form, &State);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					Form->SetFromActiveFile(State);

					return true;
				}

				CodaScriptCommandHandler(GetDataHandlerFormList)
				{
					CodaScriptNumericDataTypeT FormType = 0;

					CodaScriptCommandExtractArgs(&FormType);

					if (FormType < TESForm::kFormType_GMST || FormType > TESForm::kFormType_TOFT)
						return false;

					ICodaScriptDataStore* Array = Utilities->ArrayAllocate(200);
					SME_ASSERT(Array);

					for (ConstructionSetExtender_OverriddenClasses::NiTMapIterator Itr = TESForm::FormIDMap->GetFirstPos(); Itr;)
					{
						UInt32 FormID = NULL;
						TESForm* Form = NULL;

						TESForm::FormIDMap->GetNext(Itr, FormID, Form);
						if (FormID && Form)
						{
							if (Form->formType == (int)FormType)
							{
								Utilities->ArrayPushback(Array, (CodaScriptReferenceDataTypeT)FormID);
							}
						}
					}

					Result->SetArray(Array);

					return true;
				}

				CodaScriptCommandHandler(GetCellUseList)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					ICodaScriptDataStore* Array = Utilities->ArrayAllocate();
					SME_ASSERT(Array);

					TESCellUseList* UseList = CS_CAST(Form, TESForm, TESCellUseList);
					if (UseList)
					{
						for (TESCellUseList::CellUseInfoListT::Iterator Itr = UseList->cellUses.Begin(); !Itr.End(); ++Itr)
						{
							TESCellUseList::CellUseInfo* Data = Itr.Get();
							if (!Data)
								break;

							Utilities->ArrayPushback(Array, (CodaScriptReferenceDataTypeT)Data->cell->formID);
						}
					}

					Result->SetArray(Array);

					return true;
				}
			}
		}
	}
}