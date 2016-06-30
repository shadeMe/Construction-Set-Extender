#pragma once

namespace cse
{
	namespace renderWindow
	{
		class PathGridUndoManager
		{
		public:
			typedef std::vector<TESPathGridPoint*>		PathGridPointArrayT;

			struct PathGridPointUndoProxy
			{
				static int								GIC;

				TESPathGridPoint*						Parent;
				Vector3									Position;
				TESObjectREFR*							LinkedRef;
				PathGridPointArrayT					LinkedPoints;
				TESPathGrid*							ParentPathGrid;
				TESObjectCELL*							ParentCell;
				UInt8									Operation;
				bool									Deleted;
			private:
				void									SyncWithPoint(TESPathGridPoint* Point, bool Update3D = false);
			public:
				PathGridPointUndoProxy(const PathGridPointUndoProxy& rhs);
				PathGridPointUndoProxy(UInt8 Operation, TESPathGridPoint* Parent);
				~PathGridPointUndoProxy();

				void									HandlePathGridPointDeletion(TESPathGridPoint* Point);
				void									Undo(PathGridUndoManager* Manager, TESPathGridPoint** CreatedPointOut = NULL);
			};
		private:
			typedef boost::shared_ptr<PathGridPointUndoProxy>		UndoProxyHandleT;
			typedef std::vector<UndoProxyHandleT>					UndoProxyArrayT;
			typedef std::stack<UndoProxyArrayT*>					UndoProxyStackT;

			UndoProxyStackT								UndoStack;
			UndoProxyStackT								RedoStack;
			bool										CanReset;
			bool										WalkingStacks;

			void										ResetStack(UndoProxyStackT* Stack);
			void										HandlePointDeletionOnStack(UndoProxyStackT* Stack, PathGridPointListT* Selection);
			void										WalkUndoStack(UndoProxyStackT* Stack, UndoProxyStackT* Alternate);
		public:
			PathGridUndoManager();
			~PathGridUndoManager();

			enum
			{
				kOperation_DataChange = 0,
				kOperation_PointCreation,
				kOperation_PointDeletion
			};

			void										RecordOperation(UInt8 Operation, PathGridPointListT* Selection);
			void										PerformUndo(void);
			void										PerformRedo(void);
			void										HandlePathGridPointDeletion(PathGridPointListT* Selection);
			void										ResetRedoStack(void);
			void										ResetUndoStack(void);
			void										SetCanReset(bool State);
		};
	}
}