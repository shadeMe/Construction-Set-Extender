#pragma once

class PathGridUndoManager
{
	typedef std::vector<TESPathGridPoint*>		PathGridPointVectorT;

	struct PathGridPointUndoProxy
	{
		TESPathGridPoint*						Parent;
		Vector3									Position;
		TESObjectREFR*							LinkedRef;
		PathGridPointVectorT					LinkedPoints;
		TESPathGrid*							ParentPathGrid;
		TESObjectCELL*							ParentCell;
		UInt8									Operation;
	private:
		void									CopyToPoint(TESPathGridPoint* Point, bool Update3D = false);
	public:
		PathGridPointUndoProxy(UInt8 Operation, TESPathGridPoint* Parent);
		PathGridPointUndoProxy(const PathGridPointUndoProxy& rhs);
		~PathGridPointUndoProxy();

		void									HandlePathGridPointDeletion(TESPathGridPoint* Point);
		void									Undo(PathGridUndoManager* Manager, TESPathGridPoint** CreatedPointOut = NULL);
	};

	typedef tListBase<PathGridPointUndoProxy, false>		UndoProxyListT;
	typedef std::stack<UndoProxyListT*>						UndoProxyStackT;

	UndoProxyStackT								UndoStack;
	UndoProxyStackT								RedoStack;
	bool										CanReset;

	void										ResetStack(UndoProxyStackT* Stack);
	void										HandlePointDeletionOnStack(UndoProxyStackT* Stack, PathGridPointListT* Selection);
	void										WalkUndoStack(UndoProxyStackT* Stack, UndoProxyStackT* Alternate);
public:
	PathGridUndoManager() : UndoStack(), RedoStack(), CanReset(true) {}

	enum
	{
		kOperation_DataChange	= 0x0,
		kOperation_PointCreation,
		kOperation_PointDeletion
	};

	void										RecordOperation(UInt8 Operation, PathGridPointListT* Selection);
	void										PerformUndo(void);
	void										PerformRedo(void);
	void										HandlePathGridPointDeletion(PathGridPointListT* Selection);
	void										ResetRedoStack(void);
	void										ResetUndoStack(void);
	void										SetCanReset(bool State) { CanReset = State; }
};

extern PathGridUndoManager			g_PathGridUndoManager;