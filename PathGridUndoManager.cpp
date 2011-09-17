#include "PathGridUndoManager.h"
#include "RenderWindowTextPainter.h"

PathGridUndoManager			g_PathGridUndoManager;

PathGridUndoManager::PathGridPointUndoProxy::PathGridPointUndoProxy( UInt8 Operation, TESPathGridPoint* Parent ) : LinkedPoints()
{
	this->Operation = Operation;
	this->Parent = Parent;
	this->Position = Parent->position;
	this->LinkedRef = Parent->linkedRef;
	this->ParentPathGrid = Parent->parentGrid;
	this->ParentCell = ParentPathGrid->parentCell;
	this->LinkedPoints.reserve(5);

	for (PathGridPointListT::Iterator Itr = Parent->linkedPoints.Begin(); !Itr.End() && Itr.Get(); ++Itr)
	{
		if (Operation == kOperation_PointDeletion && g_RenderWindowSelectedPathGridPoints->IndexOf(Itr.Get()) != -1)
			continue;

		this->LinkedPoints.push_back(Itr.Get());
	}
}

PathGridUndoManager::PathGridPointUndoProxy::PathGridPointUndoProxy( const PathGridPointUndoProxy& rhs ) : LinkedPoints()
{
	this->Operation = rhs.Operation;
	this->Parent = rhs.Parent;
	this->Position = rhs.Position;
	this->LinkedRef = rhs.LinkedRef;
	this->ParentPathGrid = rhs.ParentPathGrid;
	this->ParentCell = rhs.ParentCell;
	this->LinkedPoints.reserve(5);

	for (PathGridPointVectorT::const_iterator Itr = rhs.LinkedPoints.begin(); Itr != rhs.LinkedPoints.end(); Itr++)
		this->LinkedPoints.push_back(*Itr);
}

void PathGridUndoManager::PathGridPointUndoProxy::HandlePathGridPointDeletion( TESPathGridPoint* Point )
{
	for (PathGridPointVectorT::iterator Itr = LinkedPoints.begin(); Itr != LinkedPoints.end(); Itr++)
	{
		if (*Itr == Point)
		{
			LinkedPoints.erase(Itr);
			break;
		}
	}
}

void PathGridUndoManager::PathGridPointUndoProxy::CopyToPoint(TESPathGridPoint* Point, bool Update3D)
{
	if (Point)
	{
		Point->UnlinkFromReference();
		Point->UnlinkAllPoints();

		Point->position = Position;
		if (LinkedRef)
			Point->LinkToReference(LinkedRef);

		for (PathGridPointVectorT::iterator Itr = LinkedPoints.begin(); Itr != LinkedPoints.end(); Itr++)
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
}

void PathGridUndoManager::PathGridPointUndoProxy::Undo( PathGridUndoManager* Manager, TESPathGridPoint** CreatedPointOut )
{
	switch (Operation)
	{
	case kOperation_DataChange:
		CopyToPoint(Parent, true);
		break;
	case kOperation_PointCreation:
		{
			Parent->HideSelectionRing();
			Parent->UnlinkAllPoints();
			ParentPathGrid->RemovePoint(Parent);

			if (ParentPathGrid->gridPointCount == 0)
			{
				if (ParentPathGrid->GetOverrideFile(-1) == NULL || (ParentPathGrid->GetOverrideFile(0) && ParentPathGrid->GetOverrideFile(0)->IsActive()))
				{
					ParentPathGrid->parentCell->SetPathGrid(NULL);
				}
				else
				{
					ParentPathGrid->SetDeleted(true);
					thisCall<void>(0x0054E440, ParentPathGrid);
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

		if (CurrentPathGrid == NULL)
		{
			TESPathGrid* NewPathGrid = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_PathGrid), TESForm, TESPathGrid);
			NewPathGrid->SetFromActiveFile(true);
			ParentCell->SetPathGrid(NewPathGrid);
			NewPathGrid->SetParentCell(ParentCell);
			CurrentPathGrid = NewPathGrid;
		}

		CurrentPathGrid->AddPoint(NewPoint);
		CopyToPoint(NewPoint);

		NewPoint->GenerateNiNode();
		CurrentPathGrid->GenerateNiNode();

		if (CreatedPointOut)
			*CreatedPointOut = NewPoint;

		break;
	}
}

void PathGridUndoManager::ResetStack( UndoProxyStackT* Stack )
{
	if (CanReset == false)
		return;

	while (Stack->size())
	{
		UndoProxyListT* ProxyList = Stack->top();
		Stack->pop();

		for (UndoProxyListT::Iterator Itr = ProxyList->Begin(); !Itr.End() && Itr.Get(); ++Itr)
		{
			delete Itr.Get();
		}

		ProxyList->RemoveAll();
		FormHeap_Free(ProxyList);
	}
}

void PathGridUndoManager::HandlePointDeletionOnStack( UndoProxyStackT* Stack, PathGridPointListT* Selection )
{
	std::vector<UndoProxyListT*> StackBuffer;

	while (Stack->size())
	{
		UndoProxyListT* ProxyList = Stack->top();
		std::vector<PathGridPointUndoProxy*> Delinquents;

		Stack->pop();
		for (UndoProxyListT::Iterator Itr = ProxyList->Begin(); !Itr.End() && Itr.Get(); ++Itr)
		{
			PathGridPointUndoProxy* Proxy = Itr.Get();

			for (PathGridPointListT::Iterator ItrEx = Selection->Begin(); !ItrEx.End() && ItrEx.Get(); ++ItrEx)
			{
				if (Proxy->Parent == ItrEx.Get())
					Delinquents.push_back(Proxy);
				else
					Proxy->HandlePathGridPointDeletion(ItrEx.Get());
			}
		}

		for (std::vector<PathGridPointUndoProxy*>::iterator Itr = Delinquents.begin(); Itr != Delinquents.end(); Itr++)
		{
			ProxyList->Remove(*Itr);
			delete *Itr;
		}

		if (ProxyList->Count() == 0)
			FormHeap_Free(ProxyList);
		else
			StackBuffer.push_back(ProxyList);
	}

	for (std::vector<UndoProxyListT*>::reverse_iterator Itr = StackBuffer.rbegin(); Itr != StackBuffer.rend(); Itr++)
		Stack->push(*Itr);
}

void PathGridUndoManager::RecordOperation( UInt8 Operation, PathGridPointListT* Selection )
{
	UndoProxyListT* ProxyList = UndoProxyListT::Create(&FormHeap_Allocate);

	for (PathGridPointListT::Iterator Itr = Selection->Begin(); !Itr.End() && Itr.Get(); ++Itr)
		ProxyList->AddAt(new PathGridPointUndoProxy(Operation, Itr.Get()), eListEnd);

	UndoStack.push(ProxyList);
}

void PathGridUndoManager::WalkUndoStack( UndoProxyStackT* Stack, UndoProxyStackT* Alternate )
{
	if (Stack->size())
	{
		UndoProxyListT* ProxyList = Stack->top();
		UndoProxyListT* AltProxyList = UndoProxyListT::Create(&FormHeap_Allocate);

		Stack->pop();
		for (UndoProxyListT::Iterator Itr = ProxyList->Begin(); !Itr.End() && Itr.Get(); ++Itr)
		{
			switch (Itr.Get()->Operation)
			{
			case kOperation_PointCreation:
				{
					PathGridPointUndoProxy* Proxy = new PathGridPointUndoProxy(*Itr.Get());
					Proxy->Operation = kOperation_PointDeletion;
					AltProxyList->AddAt(Proxy, eListEnd);

					TESPathGridPoint* Parent = Itr.Get()->Parent;
					Itr.Get()->Undo(this);

					PathGridPointListT* DeletionList = (PathGridPointListT*)PathGridPointListT::Create(&FormHeap_Allocate);
					DeletionList->AddAt(Parent, eListEnd);
					HandlePathGridPointDeletion(DeletionList);
					DeletionList->RemoveAll();
					FormHeap_Free(DeletionList);

					delete Itr.Get();
					break;
				}
			case kOperation_PointDeletion:
				{
					TESPathGridPoint* NewPoint = NULL;
					Itr.Get()->Undo(this, &NewPoint);
					delete Itr.Get();

					PathGridPointUndoProxy* Proxy = new PathGridPointUndoProxy(kOperation_PointCreation, NewPoint);
					AltProxyList->AddAt(Proxy, eListEnd);
					break;
				}
			case kOperation_DataChange:
				{
					AltProxyList->AddAt(new PathGridPointUndoProxy(kOperation_DataChange, Itr.Get()->Parent), eListEnd);

					Itr.Get()->Undo(this);
					delete Itr.Get();
					break;
				}
			}
		}

		ProxyList->RemoveAll();
		FormHeap_Free(ProxyList);

		Alternate->push(AltProxyList);
	}
}

void PathGridUndoManager::PerformUndo( void )
{
	WalkUndoStack(&UndoStack, &RedoStack);
}

void PathGridUndoManager::PerformRedo( void )
{
	WalkUndoStack(&RedoStack, &UndoStack);
}

void PathGridUndoManager::HandlePathGridPointDeletion( PathGridPointListT* Selection )
{
	HandlePointDeletionOnStack(&UndoStack, Selection);
	HandlePointDeletionOnStack(&RedoStack, Selection);
}

void PathGridUndoManager::ResetRedoStack( void )
{
	ResetStack(&RedoStack);
}

void PathGridUndoManager::ResetUndoStack( void )
{
	ResetStack(&UndoStack);
}