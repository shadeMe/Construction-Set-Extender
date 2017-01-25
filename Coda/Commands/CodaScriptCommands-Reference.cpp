#include "CodaScriptCommands-Reference.h"
#include "Render Window\RenderWindowManager.h"

#define PI	3.151592653589793

namespace cse
{
	namespace script
	{
		namespace commands
		{
			namespace reference
			{
				CodaScriptCommandRegistrarDef("Reference and Cell")

				CodaScriptCommandPrototypeDef(CreateRef);
				CodaScriptCommandPrototypeDef(GetRefPosition);
				CodaScriptCommandPrototypeDef(GetRefRotation);
				CodaScriptCommandPrototypeDef(GetRefScale);
				CodaScriptCommandPrototypeDef(GetRefPersistent);
				CodaScriptCommandPrototypeDef(GetRefDisabled);
				CodaScriptCommandPrototypeDef(GetRefVWD);
				CodaScriptCommandPrototypeDef(GetRefBaseForm);
				CodaScriptCommandPrototypeDef(GetRefCell);
				CodaScriptCommandPrototypeDef(SetRefRotation);
				CodaScriptCommandPrototypeDef(SetRefPosition);
				CodaScriptCommandPrototypeDef(SetRefScale);
				CodaScriptCommandPrototypeDef(SetRefPersistent);
				CodaScriptCommandPrototypeDef(SetRefDisabled);
				CodaScriptCommandPrototypeDef(SetRefVWD);
				CodaScriptCommandPrototypeDef(GetCellObjects);
				CodaScriptCommandPrototypeDef(GetCellWorldspace);
				CodaScriptCommandPrototypeDef(GetCurrentRenderWindowSelection);
				CodaScriptCommandPrototypeDef(GetLoadedRenderWindowCells);
				CodaScriptCommandPrototypeDef(AddRefToRenderWindowSelection);
				CodaScriptCommandPrototypeDef(RemoveRefFromRenderWindowSelection);
				CodaScriptCommandPrototypeDef(CreateRenderWindowSelectionGroup);
				CodaScriptCommandPrototypeDef(DissolveRenderWindowSelectionGroup);
				CodaScriptCommandPrototypeDef(FloorRef);
				CodaScriptCommandPrototypeDef(LoadRefIntoRenderWindow);
				CodaScriptCommandPrototypeDef(DeleteRef);
				CodaScriptCommandPrototypeDef(GetCellCoordinates);
				CodaScriptCommandPrototypeDef(GetLandscapeTextureAtCoordinates);
				CodaScriptCommandPrototypeDef(GetCellLandscapeTextures);
				CodaScriptCommandPrototypeDef(GetLinkedDoor);

				CodaScriptCommandParamData(CreateRef, 9)
				{
					{ "Base Form", ICodaScriptDataStore::kDataType_Reference },
					{ "Pos X", ICodaScriptDataStore::kDataType_Numeric },
					{ "Pos Y", ICodaScriptDataStore::kDataType_Numeric },
					{ "Pos Z", ICodaScriptDataStore::kDataType_Numeric },
					{ "Rot X", ICodaScriptDataStore::kDataType_Numeric },
					{ "Rot Y", ICodaScriptDataStore::kDataType_Numeric },
					{ "Rot Z", ICodaScriptDataStore::kDataType_Numeric },
					{ "Cell", ICodaScriptDataStore::kDataType_Reference },
					{ "Worldspace", ICodaScriptDataStore::kDataType_Reference }
				};

				CodaScriptCommandParamData(GetRefPosition, 2)
				{
					{ "Reference", ICodaScriptDataStore::kDataType_Reference },
					{ "Axis", ICodaScriptDataStore::kDataType_String }
				};

				CodaScriptCommandParamData(GetRefRotation, 2)
				{
					{ "Reference", ICodaScriptDataStore::kDataType_Reference },
					{ "Axis", ICodaScriptDataStore::kDataType_String }
				};

				CodaScriptCommandParamData(SetRefPosition, 3)
				{
					{ "Reference", ICodaScriptDataStore::kDataType_Reference },
					{ "Axis", ICodaScriptDataStore::kDataType_String },
					{ "Value", ICodaScriptDataStore::kDataType_Numeric }
				};

				CodaScriptCommandParamData(SetRefRotation, 3)
				{
					{ "Reference", ICodaScriptDataStore::kDataType_Reference },
					{ "Axis", ICodaScriptDataStore::kDataType_String },
					{ "Value", ICodaScriptDataStore::kDataType_Numeric }
				};

				CodaScriptCommandParamData(GetCellObjects, 1)
				{
					{ "Cell", ICodaScriptDataStore::kDataType_Reference }
				};

				CodaScriptCommandParamData(GetCellWorldspace, 1)
				{
					{ "Cell", ICodaScriptDataStore::kDataType_Reference }
				};

				CodaScriptCommandParamData(CreateRenderWindowSelectionGroup, 2)
				{
					{ "References", ICodaScriptDataStore::kDataType_Array },
					{ "Group Name", ICodaScriptDataStore::kDataType_String }
				};

				CodaScriptCommandParamData(GetCellCoordinates, 1)
				{
					{ "Cell", ICodaScriptDataStore::kDataType_Reference }
				};

				CodaScriptCommandParamData(GetLandscapeTextureAtCoordinates, 3)
				{
					{ "Cell", ICodaScriptDataStore::kDataType_Reference },
					{ "X Coord", ICodaScriptDataStore::kDataType_Numeric },
					{ "Y Coord", ICodaScriptDataStore::kDataType_Numeric }
				};

				CodaScriptCommandParamData(GetCellLandscapeTextures, 2)
				{
					{ "Cell", ICodaScriptDataStore::kDataType_Reference },
					{ "Quad", ICodaScriptDataStore::kDataType_Numeric }
				};

				CodaScriptCommandParamData(GetLinkedDoor, 1)
				{
					{ "Door", ICodaScriptDataStore::kDataType_Reference }
				};

				CodaScriptCommandHandler(CreateRef)
				{
					TESForm* BaseForm = nullptr;
					TESForm* Cell = nullptr;
					TESForm* WorldSpace = nullptr;
					struct
					{
						double x, y, z;		// can't use a regular Vector3 here as CodaScriptNumericDataTypeT is a double internally
					} Position, Rotation;

					CodaScriptCommandExtractArgs(&BaseForm, &Position.x, &Position.y, &Position.z, &Rotation.x, &Rotation.y, &Rotation.z, &Cell, &WorldSpace);
					ExtractFormArguments(3, &BaseForm, &Cell, &WorldSpace);

					if (!BaseForm || !Cell)
						return false;

					TESObject* Base = CS_CAST(BaseForm, TESForm, TESObject);
					TESObjectCELL* ParentCell = CS_CAST(Cell, TESForm, TESObjectCELL);
					TESWorldSpace* ParentWorldspace = CS_CAST(WorldSpace, TESForm, TESWorldSpace);

					if (!Base || !ParentCell || (ParentCell->IsInterior() == false && !ParentWorldspace))
						return false;

					// convert rotation to radians first
					Vector3 OutRotation(Rotation.x, Rotation.y, Rotation.z);
					OutRotation.Scale(0.01745329238474369);

					TESObjectREFR* NewRef = _DATAHANDLER->PlaceObjectRef(Base,
																		&(Vector3(Position.x, Position.y, Position.z)),
																		&OutRotation,
																		ParentCell, ParentWorldspace, nullptr);
					if (NewRef)
						Result->SetFormID(NewRef->formID);
					else
						Result->SetFormID(0);

					return true;
				}

				CodaScriptCommandHandler(GetRefPosition)
				{
					TESForm* Form = nullptr;
					CodaScriptStringParameterTypeT Buffer = nullptr;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (!Form)
						return false;

					TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
					if (!Reference)
						return false;

					if (!_stricmp(Buffer, "x"))
						Result->SetNumber(Reference->position.x);
					else if (!_stricmp(Buffer, "y"))
						Result->SetNumber(Reference->position.y);
					else
						Result->SetNumber(Reference->position.z);

					return true;
				}

				CodaScriptCommandHandler(GetRefRotation)
				{
					TESForm* Form = nullptr;
					CodaScriptStringParameterTypeT Buffer = nullptr;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (!Form)
						return false;

					TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
					if (!Reference)
						return false;

					if (!_stricmp(Buffer, "x"))
						Result->SetNumber(Reference->rotation.x * 57.2957763671875);
					else if (!_stricmp(Buffer, "y"))
						Result->SetNumber(Reference->rotation.y * 57.2957763671875);
					else
						Result->SetNumber(Reference->rotation.z * 57.2957763671875);

					return true;
				}

				CodaScriptCommandHandler(GetRefScale)
				{
					TESForm* Form = nullptr;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == nullptr)
						return false;

					TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
					if (!Reference)
						return false;

					Result->SetNumber(Reference->scale);
					return true;
				}

				CodaScriptCommandHandler(GetRefPersistent)
				{
					TESForm* Form = nullptr;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == nullptr)
						return false;

					TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
					if (!Reference)
						return false;

					Result->SetNumber((Reference->formFlags & TESForm::kFormFlags_QuestItem) ? 1 : 0);
					return true;
				}

				CodaScriptCommandHandler(GetRefDisabled)
				{
					TESForm* Form = nullptr;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == nullptr)
						return false;

					TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
					if (!Reference)
						return false;

					Result->SetNumber((Reference->formFlags & TESForm::kFormFlags_Disabled) ? 1 : 0);
					return true;
				}

				CodaScriptCommandHandler(GetRefVWD)
				{
					TESForm* Form = nullptr;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == nullptr)
						return false;

					TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
					if (!Reference)
						return false;

					Result->SetNumber((Reference->formFlags & TESForm::kFormFlags_VisibleWhenDistant) ? 1 : 0);
					return true;
				}

				CodaScriptCommandHandler(GetRefBaseForm)
				{
					TESForm* Form = nullptr;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == nullptr)
						return false;

					TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
					if (!Reference)
						return false;

					if (Reference->baseForm)
						Result->SetFormID(Reference->baseForm->formID);
					else
						Result->SetFormID(0);

					return true;
				}

				CodaScriptCommandHandler(GetRefCell)
				{
					TESForm* Form = nullptr;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == nullptr)
						return false;

					TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
					if (!Reference)
						return false;

					if (Reference->parentCell)
						Result->SetFormID(Reference->parentCell->formID);
					else
						Result->SetFormID(0);

					return true;
				}

				CodaScriptCommandHandler(SetRefPosition)
				{
					TESForm* Form = nullptr;
					CodaScriptStringParameterTypeT Buffer = nullptr;
					CodaScriptNumericDataTypeT Value = 0.0;

					CodaScriptCommandExtractArgs(&Form, &Buffer, &Value);
					ExtractFormArguments(1, &Form);

					if (Form == nullptr)
						return false;

					TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
					if (!Reference)
						return false;

					Vector3 Pos(Reference->position);
					if (!_stricmp(Buffer, "x"))
						Pos.x = Value;
					else if (!_stricmp(Buffer, "y"))
						Pos.y = Value;
					else
						Pos.z = Value;

					Reference->SetPosition(Pos.x, Pos.y, Pos.z);
					return true;
				}

				CodaScriptCommandHandler(SetRefRotation)
				{
					TESForm* Form = nullptr;
					CodaScriptStringParameterTypeT Buffer = nullptr;
					CodaScriptNumericDataTypeT Value = 0.0;

					CodaScriptCommandExtractArgs(&Form, &Buffer, &Value);
					ExtractFormArguments(1, &Form);

					if (Form == nullptr)
						return false;

					TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
					if (!Reference)
						return false;

					Vector3 Rot(Reference->rotation);
					if (!_stricmp(Buffer, "x"))
						Rot.x = Value * 0.01745329238474369;
					else if (!_stricmp(Buffer, "y"))
						Rot.y = Value * 0.01745329238474369;
					else
						Rot.z = Value * 0.01745329238474369;

					Reference->SetRotation(Rot.x, Rot.y, Rot.z, true);
					return true;
				}

				CodaScriptCommandHandler(SetRefScale)
				{
					TESForm* Form = nullptr;
					CodaScriptNumericDataTypeT Value = 0.0;

					CodaScriptCommandExtractArgs(&Form, &Value);
					ExtractFormArguments(1, &Form);

					if (Form == nullptr)
						return false;

					TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
					if (!Reference)
						return false;

					Reference->SetScale(Value);
					return true;
				}

				CodaScriptCommandHandler(SetRefPersistent)
				{
					TESForm* Form = nullptr;
					CodaScriptNumericDataTypeT Value = 0.0;

					CodaScriptCommandExtractArgs(&Form, &Value);
					ExtractFormArguments(1, &Form);

					if (Form == nullptr)
						return false;

					TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
					if (!Reference)
						return false;

					SME::MiscGunk::ToggleFlag(&Reference->formFlags, TESForm::kFormFlags_QuestItem, (bool)Value);
					return true;
				}

				CodaScriptCommandHandler(SetRefDisabled)
				{
					TESForm* Form = nullptr;
					CodaScriptNumericDataTypeT Value = 0.0;

					CodaScriptCommandExtractArgs(&Form, &Value);
					ExtractFormArguments(1, &Form);

					if (Form == nullptr)
						return false;

					TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
					if (!Reference)
						return false;

					SME::MiscGunk::ToggleFlag(&Reference->formFlags, TESForm::kFormFlags_Disabled, (bool)Value);
					return true;
				}

				CodaScriptCommandHandler(SetRefVWD)
				{
					TESForm* Form = nullptr;
					CodaScriptNumericDataTypeT Value = 0.0;

					CodaScriptCommandExtractArgs(&Form, &Value);
					ExtractFormArguments(1, &Form);

					if (Form == nullptr)
						return false;

					TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
					if (!Reference)
						return false;

					SME::MiscGunk::ToggleFlag(&Reference->formFlags, TESForm::kFormFlags_VisibleWhenDistant, (bool)Value);
					return true;
				}

				CodaScriptCommandHandler(GetCellObjects)
				{
					TESForm* Form = nullptr;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == nullptr)
						return false;

					TESObjectCELL* Cell = CS_CAST(Form, TESForm, TESObjectCELL);
					if (!Cell)
						return false;

					ICodaScriptDataStore* CellObjects = Utilities->ArrayAllocate(Cell->objectList.Count());
					SME_ASSERT(CellObjects);

					for (TESObjectCELL::ObjectREFRList::Iterator Itr = Cell->objectList.Begin(); !Itr.End() && Itr.Get(); ++Itr)
					{
						if (Itr.Get()->formID)
							Utilities->ArrayPushback(CellObjects, (CodaScriptReferenceDataTypeT)Itr.Get()->formID);
					}

					Result->SetArray(CellObjects);

					return true;
				}

				CodaScriptCommandHandler(GetCellWorldspace)
				{
					TESForm* Form = nullptr;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == nullptr)
						return false;

					TESObjectCELL* Cell = CS_CAST(Form, TESForm, TESObjectCELL);
					if (!Cell)
						return false;

					TESWorldSpace* Worldspace = Cell->GetParentWorldSpace();
					Result->SetFormID((Worldspace ? Worldspace->formID : 0));

					return true;
				}

				CodaScriptCommandHandler(GetCurrentRenderWindowSelection)
				{
					ICodaScriptDataStore* Array = Utilities->ArrayAllocate(_RENDERSEL->selectionCount);
					SME_ASSERT(Array);

					for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
					{
						TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);

						if (Ref)
							Utilities->ArrayPushback(Array, (CodaScriptReferenceDataTypeT)Ref->formID);
					}

					Result->SetArray(Array);

					return true;
				}

				CodaScriptCommandHandler(GetLoadedRenderWindowCells)
				{
					ICodaScriptDataStore* Array = Utilities->ArrayAllocate();
					SME_ASSERT(Array);

					if (_TES->currentInteriorCell)
					{
						Utilities->ArrayPushback(Array, (CodaScriptReferenceDataTypeT)_TES->currentInteriorCell->formID);
					}
					else if (*TESRenderWindow::ActiveCell)
					{
						for (int i = 0; i < _TES->gridCellArray->size; i++)
						{
							for (int j = 0; j < _TES->gridCellArray->size; j++)
							{
								TESObjectCELL* Cell = _TES->gridCellArray->GetCellEntry(i, j)->cell;
								if (Cell)
									Utilities->ArrayPushback(Array, (CodaScriptReferenceDataTypeT)Cell->formID);
							}
						}
					}

					Result->SetArray(Array);

					return true;
				}

				CodaScriptCommandHandler(AddRefToRenderWindowSelection)
				{
					TESForm* Form = nullptr;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == nullptr)
						return false;

					TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
					if (!Reference)
						return false;

					_RENDERSEL->AddToSelection(Reference, true);
					return true;
				}

				CodaScriptCommandHandler(RemoveRefFromRenderWindowSelection)
				{
					TESForm* Form = nullptr;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == nullptr)
						return false;

					TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
					if (!Reference)
						return false;

					_RENDERSEL->RemoveFromSelection(Reference, true);
					return true;
				}

				CodaScriptCommandHandler(CreateRenderWindowSelectionGroup)
				{
					ICodaScriptDataStore* Array = nullptr;
					CodaScriptStringParameterTypeT GroupName = nullptr;

					CodaScriptCommandExtractArgs(&Array, &GroupName);
					SME_ASSERT(Array && GroupName);

					Result->SetNumber(0);

					if (Utilities->ArraySize(Array) > 1)
					{
						std::vector<TESObjectREFR*> Members;
						for (int i = 0, j = Utilities->ArraySize(Array); i < j; i++)
						{
							ICodaScriptDataStore* Current = nullptr;
							if (Utilities->ArrayAt(Array, i, &Current))
							{
								if (Current->IsReference())
								{
									TESObjectREFR* Ref = CS_CAST(TESForm::LookupByFormID(Current->GetFormID()), TESForm, TESObjectREFR);
									if (Ref)
										Members.push_back(Ref);
									else
										return false;
								}
								else
									return false;
							}
							else
								return false;
						}

						TESRenderSelection* SelBuffer = TESRenderSelection::CreateInstance();
						for (int i = 0, j = Members.size(); i < j; i++)
							SelBuffer->AddToSelection(Members[i], false);

						if (_RENDERWIN_MGR.GetGroupManager()->AddGroup(GroupName, SelBuffer))
							Result->SetNumber(1);

						SelBuffer->DeleteInstance();
					}

					return true;
				}

				CodaScriptCommandHandler(DissolveRenderWindowSelectionGroup)
				{
					ICodaScriptDataStore* Array = nullptr;

					CodaScriptCommandExtractArgs(&Array);

					SME_ASSERT(Array);

					Result->SetNumber(0);

					if (Utilities->ArraySize(Array) > 1)
					{
						std::vector<TESObjectREFR*> Members;
						for (int i = 0, j = Utilities->ArraySize(Array); i < j; i++)
						{
							ICodaScriptDataStore* Current = nullptr;
							if (Utilities->ArrayAt(Array, i, &Current))
							{
								if (Current->IsReference())
								{
									TESObjectREFR* Ref = CS_CAST(TESForm::LookupByFormID(Current->GetFormID()), TESForm, TESObjectREFR);
									if (Ref)
										Members.push_back(Ref);
									else
										return false;
								}
								else
									return false;
							}
							else
								return false;
						}

						TESRenderSelection* Buffer = TESRenderSelection::CreateInstance();
						for (int i = 0, j = Members.size(); i < j; i++)
							Buffer->AddToSelection(Members[i], false);

						_RENDERWIN_MGR.GetGroupManager()->RemoveSelectionGroups(Buffer);
						Result->SetNumber(1);

						Buffer->DeleteInstance();
					}

					return true;
				}

				CodaScriptCommandHandler(FloorRef)
				{
					TESForm* Form = nullptr;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == nullptr)
						return false;

					TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
					if (!Reference || Reference->formType != TESForm::kFormType_REFR)
						return false;

					Reference->Floor();

					return true;
				}

				CodaScriptCommandHandler(LoadRefIntoRenderWindow)
				{
					TESForm* Form = nullptr;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == nullptr)
						return false;

					TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
					if (!Reference || Reference->formType != TESForm::kFormType_REFR)
						return false;

					_TES->LoadCellIntoViewPort(Reference->GetPosition(), Reference);

					// clean up the selection buffer as the above call selects the ref
					_RENDERSEL->ClearSelection(true);

					return true;
				}

				CodaScriptCommandHandler(DeleteRef)
				{
					TESForm* Form = nullptr;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == nullptr)
						return false;

					TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
					if (!Reference || Reference->formType != TESForm::kFormType_REFR)
						return false;

					Reference->Delete();
					return true;
				}

				CodaScriptCommandHandler(GetCellCoordinates)
				{
					TESForm* Form = nullptr;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == nullptr)
						return false;

					TESObjectCELL* Cell = CS_CAST(Form, TESForm, TESObjectCELL);
					if (!Cell)
						return false;

					ICodaScriptDataStore* Array = Utilities->ArrayAllocate();
					SME_ASSERT(Array);

					if (Cell->IsInterior() == false)
					{
						Utilities->ArrayPushback(Array, (CodaScriptNumericDataTypeT)Cell->cellData.coords->x);
						Utilities->ArrayPushback(Array, (CodaScriptNumericDataTypeT)Cell->cellData.coords->y);
					}

					Result->SetArray(Array);

					return true;
				}

				CodaScriptCommandHandler(GetLandscapeTextureAtCoordinates)
				{
					TESObjectCELL* Cell = nullptr;
					CodaScriptNumericDataTypeT XCoord, YCoord;

					CodaScriptCommandExtractArgs(&Cell, &XCoord, &YCoord);
					ExtractFormArguments(1, &Cell);

					Cell = CS_CAST(Cell, TESForm, TESObjectCELL);
					if (!Cell)
						return false;

					Result->SetFormID(0);

					if (Cell->IsInterior() == false)
					{
						TESObjectLAND* Land = Cell->GetLand();
						if (Land)
						{
							TESLandTexture* Texture = Land->GetLandTextureAt(&Vector3(XCoord, YCoord, 0.f));
							if (Texture)
								Result->SetFormID(Texture->formID);
						}
					}

					return true;
				}

				CodaScriptCommandHandler(GetCellLandscapeTextures)
				{
					TESObjectCELL* Cell = nullptr;
					CodaScriptNumericDataTypeT Quad;

					CodaScriptCommandExtractArgs(&Cell, &Quad);
					ExtractFormArguments(1, &Cell);

					Cell = CS_CAST(Cell, TESForm, TESObjectCELL);
					if (!Cell)
						return false;

					ICodaScriptDataStore* Array = Utilities->ArrayAllocate();
					SME_ASSERT(Array);

					if (Cell->IsInterior() == false)
					{
						TESObjectLAND* Land = Cell->GetLand();
						if (Land)
						{
							if (Quad < 0)
								Quad = 0;
							else if (Quad > 3)
								Quad = 3;

							if (Land->landData)
							{
								auto LandTexData = Land->landData->quadTextures[(int)Quad];
								if (LandTexData)
								{
									for (auto Itr : LandTexData->textures)
									{
										if (Itr)
											Utilities->ArrayPushback(Array, (CodaScriptReferenceDataTypeT)Itr->formID);
									}
								}
							}
						}
					}

					Result->SetArray(Array);

					return true;
				}

				CodaScriptCommandHandler(GetLinkedDoor)
				{
					TESObjectREFR* Ref = nullptr;

					CodaScriptCommandExtractArgs(&Ref);
					ExtractFormArguments(1, &Ref);

					if (Ref == nullptr)
						return false;

					Ref = CS_CAST(Ref, TESForm, TESObjectREFR);
					if (!Ref || !Ref->baseForm)
						return false;
					else if (Ref->baseForm->formType != TESForm::kFormType_Door)
						return false;

					Result->SetFormID(NULL);

					auto* xTeleport = CS_CAST(Ref->extraData.GetExtraDataByType(ExtraTeleport::kExtra_Teleport), BSExtraData, ExtraTeleport);
					if (xTeleport && xTeleport->data && xTeleport->data->linkedDoor)
						Result->SetFormID(xTeleport->data->linkedDoor->formID);

					return true;
				}
			}
		}
	}
}