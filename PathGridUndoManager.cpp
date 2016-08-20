#include "PathGridUndoManager.h"

namespace cse
{
	namespace renderWindow
	{
		int							PathGridUndoManager::PathGridPointUndoProxy::GIC = 0;

		PathGridUndoManager::PathGridPointUndoProxy::PathGridPointUndoProxy(UInt8 Operation, TESPathGridPoint* Parent) :
			LinkedPoints()
		{
			GIC++;

			this->Operation = Operation;
			this->Parent = Parent;
			this->Position = Parent->position;
			this->LinkedRef = Parent->linkedRef;
			this->ParentPathGrid = Parent->parentGrid;
			this->ParentCell = ParentPathGrid->parentCell;
			this->LinkedPoints.reserve(5);
			this->Deleted = false;

			for (PathGridPointListT::Iterator Itr = Parent->linkedPoints.Begin(); !Itr.End() && Itr.Get(); ++Itr)
			{
				if (Operation == kOperation_PointDeletion && TESRenderWindow::SelectedPathGridPoints->IndexOf(Itr.Get()) != -1)
					continue;

				this->LinkedPoints.push_back(Itr.Get());
			}
		}

		PathGridUndoManager::PathGridPointUndoProxy::PathGridPointUndoProxy(const PathGridPointUndoProxy& rhs) :
			LinkedPoints()
		{
			GIC++;

			this->Operation = rhs.Operation;
			this->Parent = rhs.Parent;
			this->Position = rhs.Position;
			this->LinkedRef = rhs.LinkedRef;
			this->ParentPathGrid = rhs.ParentPathGrid;
			this->ParentCell = rhs.ParentCell;
			this->LinkedPoints = rhs.LinkedPoints;
			this->Deleted = rhs.Deleted;
		}

		void PathGridUndoManager::PathGridPointUndoProxy::HandlePathGridPointDeletion(TESPathGridPoint* Point)
		{
			SME_ASSERT(Deleted == false);

			for (PathGridPointArrayT::iterator Itr = LinkedPoints.begin(); Itr != LinkedPoints.end(); Itr++)
			{
				if (*Itr == Point)
				{
					LinkedPoints.erase(Itr);
					break;
				}
			}
		}

		void PathGridUndoManager::PathGridPointUndoProxy::SyncWithPoint(TESPathGridPoint* Point, bool Update3D)
		{
			SME_ASSERT(Deleted == false);

			if (Point)
			{
				Point->UnlinkFromReference();
				Point->UnlinkAllPoints();

				Point->position = Position;
				if (LinkedRef)
					Point->LinkToReference(LinkedRef);

				for (PathGridPointArrayT::iterator Itr = LinkedPoints.begin(); Itr != LinkedPoints.end(); Itr++)
				{
					Point->LinkPoint(*Itr);
				}

				if (Update3D)
					Point->parentGrid->GenerateNiNode();
			}
		}

		PathGridUndoManager::PathGridPointUndoProxy::~PathGridPointUndoProxy()
		{
			LinkedPoints.clear();

			GIC--;
			SME_ASSERT(GIC >= 0);
		}

		void PathGridUndoManager::PathGridPointUndoProxy::Undo(PathGridUndoManager* Manager, TESPathGridPoint** CreatedPointOut)
		{
			SME_ASSERT(Deleted == false);

			switch (Operation)
			{
			case kOperation_DataChange:
				SyncWithPoint(Parent, true);
				break;
			case kOperation_PointCreation:
				{
					Parent->HideSelectionRing();
					Parent->UnlinkAllPoints();
					ParentPathGrid->RemovePoint(Parent);

					if (ParentPathGrid->gridPointCount == 0)
					{
						if (ParentPathGrid->GetOverrideFile(-1) == nullptr || (ParentPathGrid->GetOverrideFile(0) && ParentPathGrid->GetOverrideFile(0)->IsActive()))
						{
							ParentPathGrid->parentCell->SetPathGrid(nullptr);
						}
						else
						{
							ParentPathGrid->SetDeleted(true);
							ParentPathGrid->ReleaseAllPoints();
							ParentPathGrid->CleanupLinkedReferences();
						}
					}
					else
						ParentPathGrid->GenerateNiNode();

					break;
				}
			case kOperation_PointDeletion:
				TESPathGridPoint* NewPoint = TESPathGridPoint::CreateInstance();
				TESPathGrid* CurrentPathGrid = ParentCell->pathGrid;

				if (CurrentPathGrid == nullptr)
				{
					TESPathGrid* NewPathGrid = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_PathGrid), TESForm, TESPathGrid);
					NewPathGrid->SetFromActiveFile(true);
					ParentCell->SetPathGrid(NewPathGrid);
					NewPathGrid->SetParentCell(ParentCell);
					CurrentPathGrid = NewPathGrid;
				}

				CurrentPathGrid->AddPoint(NewPoint);
				SyncWithPoint(NewPoint);

				NewPoint->GenerateNiNode();
				CurrentPathGrid->GenerateNiNode();

				if (CreatedPointOut)
					*CreatedPointOut = NewPoint;

				break;
			}
		}

		PathGridUndoManager::PathGridUndoManager() :
			UndoStack(),
			RedoStack(),
			CanReset(true),
			WalkingStacks(false)
		{
			;//
		}

		PathGridUndoManager::~PathGridUndoManager()
		{
			CanReset = true;

			ResetUndoStack();
			ResetRedoStack();

			SME_ASSERT(PathGridPointUndoProxy::GIC == 0);
		}

		void PathGridUndoManager::ResetStack(UndoProxyStackT* Stack)
		{
			if (CanReset == false)
				return;

			while (Stack->size())
			{
				UndoProxyArrayT* ProxyList = Stack->top();
				Stack->pop();

				ProxyList->clear();
				delete ProxyList;
			}
		}

		void PathGridUndoManager::HandlePointDeletionOnStack(UndoProxyStackT* Stack, PathGridPointListT* Selection)
		{
			std::list<UndoProxyArrayT*> StackBuffer;

			while (Stack->size())
			{
				UndoProxyArrayT* ProxyList = Stack->top();
				Stack->pop();

				for (UndoProxyArrayT::iterator Itr = ProxyList->begin(); Itr != ProxyList->end(); Itr++)
				{
					UndoProxyHandleT Proxy(*Itr);

					for (PathGridPointListT::Iterator ItrEx = Selection->Begin(); !ItrEx.End() && ItrEx.Get(); ++ItrEx)
					{
						if (Proxy->Parent == ItrEx.Get())
							Proxy->Deleted = true;
						else if (Proxy->Deleted == false)
							Proxy->HandlePathGridPointDeletion(ItrEx.Get());
					}
				}

				StackBuffer.push_back(ProxyList);
			}

			for (std::list<UndoProxyArrayT*>::reverse_iterator Itr = StackBuffer.rbegin(); Itr != StackBuffer.rend(); Itr++)
				Stack->push(*Itr);
		}

		void PathGridUndoManager::RecordOperation(UInt8 Operation, PathGridPointListT* Selection)
		{
			UndoProxyArrayT* ProxyList = new UndoProxyArrayT();

			for (PathGridPointListT::Iterator Itr = Selection->Begin(); !Itr.End() && Itr.Get(); ++Itr)
			{
				UndoProxyHandleT Proxy(new PathGridPointUndoProxy(Operation, Itr.Get()));
				ProxyList->push_back(Proxy);
			}

			UndoStack.push(ProxyList);
		}

		void PathGridUndoManager::WalkUndoStack(UndoProxyStackT* Stack, UndoProxyStackT* Alternate)
		{
			SME_ASSERT(WalkingStacks == false);
			SME::MiscGunk::ScopedSetter<bool> GuardStackWalker(WalkingStacks, true);

			if (Stack->size())
			{
				UndoProxyArrayT* ProxyList = nullptr;

				do
				{
					ProxyList = Stack->top();
					Stack->pop();

					for (UndoProxyArrayT::iterator Itr = ProxyList->begin(); Itr != ProxyList->end();)
					{
						if ((*Itr)->Deleted)
							Itr = ProxyList->erase(Itr);
						else
							Itr++;
					}

					if (ProxyList->size())
					{
						break;
					}
					else
					{
						delete ProxyList;
					}

					ProxyList = nullptr;
				} while (Stack->size());

				if (ProxyList == nullptr)
					return;

				UndoProxyArrayT* AltProxyList = new UndoProxyArrayT();

				for (UndoProxyArrayT::iterator Itr = ProxyList->begin(); Itr != ProxyList->end(); Itr++)
				{
					SME_ASSERT((*Itr)->Deleted == false);

					switch ((*Itr)->Operation)
					{
					case kOperation_PointCreation:
						{
							UndoProxyHandleT Proxy(new PathGridPointUndoProxy(*(*Itr).get()));
							Proxy->Operation = kOperation_PointDeletion;
							AltProxyList->push_back(Proxy);

							(*Itr)->Undo(this);

							break;
						}
					case kOperation_PointDeletion:
						{
							TESPathGridPoint* NewPoint = nullptr;

							(*Itr)->Undo(this, &NewPoint);

							UndoProxyHandleT Proxy(new PathGridPointUndoProxy(kOperation_PointCreation, NewPoint));
							AltProxyList->push_back(Proxy);

							break;
						}
					case kOperation_DataChange:
						{
							UndoProxyHandleT Proxy(new PathGridPointUndoProxy(kOperation_DataChange, (*Itr)->Parent));
							AltProxyList->push_back(Proxy);

							(*Itr)->Undo(this);

							break;
						}
					}
				}

				ProxyList->clear();
				delete ProxyList;

				Alternate->push(AltProxyList);
			}
		}

		void PathGridUndoManager::PerformUndo(void)
		{
			WalkUndoStack(&UndoStack, &RedoStack);
		}

		void PathGridUndoManager::PerformRedo(void)
		{
			WalkUndoStack(&RedoStack, &UndoStack);
		}

		void PathGridUndoManager::HandlePathGridPointDeletion(PathGridPointListT* Selection)
		{
			HandlePointDeletionOnStack(&UndoStack, Selection);
			HandlePointDeletionOnStack(&RedoStack, Selection);
		}

		void PathGridUndoManager::ResetRedoStack(void)
		{
			ResetStack(&RedoStack);
		}

		void PathGridUndoManager::ResetUndoStack(void)
		{
			ResetStack(&UndoStack);
		}

		void PathGridUndoManager::SetCanReset(bool State)
		{
			CanReset = State;
		}
	}
}