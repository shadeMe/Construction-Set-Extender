#pragma once
#include "Serialization.h"

namespace cse
{
	namespace renderWindow
	{
		class RenderWindowManager;
		class RenderWindowOSD;
		class ImGuiDX9;

		// provides logic for executing context specific operations
		class ExecutionContext
		{
		public:
			// render window modes in which the action can be executed
			enum
			{
				kMode_ReferenceEdit = 1 << 0,
				kMode_PathGridEdit = 1 << 1,
				kMode_LandscapeEdit = 1 << 2,

				kMode_All = kMode_ReferenceEdit | kMode_PathGridEdit | kMode_LandscapeEdit,
			};
		private:
			UInt8			Context;
		public:
			ExecutionContext(UInt8 Context);

			bool			operator==(const ExecutionContext& RHS);
			bool			operator!=(const ExecutionContext& RHS);

			bool			IsExecutable() const;			// returns true if the current render window mode matches the context

			bool			HasReferenceEdit() const;
			bool			HasPathGridEdit() const;
			bool			HasLandscapeEdit() const;
		};

		// functor that performs a render window action/operation
		class IRenderWindowAction
		{
		protected:
			std::string				Name;
			std::string				Description;
			ExecutionContext		Context;
		public:
			IRenderWindowAction(std::string Name, std::string Desc, UInt8 Context);
			virtual ~IRenderWindowAction() = 0;

			virtual bool					operator()() = 0;			// returns false if the operation was not performed, true otherwise
			virtual const char*				GetName() const;
			virtual const char*				GetDescription() const;
			virtual const ExecutionContext&	GetExecutionContext() const;
		};

		namespace input
		{
			// hardcoded stuff in the editor
			struct BuiltIn
			{
				enum
				{
					kModifier_Control = 1 << 0,
					kModifier_Shift = 1 << 1,
					kModifier_Alt = 1 << 2,

					// special modifiers
					kModifier_Space = 1 << 3,
					kModifier_Z = 1 << 4,
				};

				// keys that store their pressed state
				enum
				{
					kHoldable_Control = VK_CONTROL,
					kHoldable_Shift = VK_SHIFT,
					kHoldable_Space = VK_SPACE,
					kHoldable_X = 'X',
					kHoldable_Y = 'Y',
					kHoldable_Z = 'Z',
					kHoldable_S = 'S',
					kHoldable_V = 'V',
				};

				class ModifierOverride
				{
					enum
					{
						kKey_CTRL = 0,
						kKey_SHIFT = 1,
						kKey_ALT = 2,
						kKey_SPACE = 3,
						kKey_Z = 4,

						kKey__MAX,
					};

					struct State
					{
						UInt8*	BaseState;
						UInt8	StateBuffer;

						bool	Active;
						bool	NewState;
					};

					State		Data[kKey__MAX];
				public:
					ModifierOverride();
					~ModifierOverride();

					bool		IsActive(UInt8 Modifier) const;
					bool		GetOverrideState(UInt8 Modifier) const;

					void		Activate(UInt8 Modifier, bool NewState);
					void		Deactivate(UInt8 Modifier);

					static ModifierOverride		Instance;
				};

				class KeyBinding
				{
					UInt8		Modifiers;
					SHORT		KeyCode;
				public:
					KeyBinding(SHORT Key, UInt8 Modifiers = NULL);

					void		Trigger() const;
					bool		IsActivated(SHORT Key) const;

					SHORT		GetKeyCode() const;
					UInt8		GetModifiers(bool StripSpecialModifiers) const;
				};
			};

		}

		typedef UInt32										TESObjectREFRSafeHandleT;
		typedef std::vector<TESObjectREFRSafeHandleT>		TESObjectREFRSafeArrayT;


		// stores refs by formID and supports validation
		class NamedReferenceCollection
		{
		public:
			typedef std::string									CollectionIDT;
		protected:
			CollectionIDT				Name;
			TESObjectREFRSafeArrayT		Members;

			bool					IsMember(TESObjectREFRSafeHandleT Ref, TESObjectREFRSafeArrayT::iterator& Match);
			static bool				GetValidRef(TESObjectREFRSafeHandleT Ref, TESObjectREFR*& OutResolvedRef);		// returns false if the ref handle didn't point to a valid reference
		public:
			NamedReferenceCollection(const char* Name);
			explicit NamedReferenceCollection(const std::string& InSerialized);
			inline virtual ~NamedReferenceCollection() = default;

			void					ValidateMembers(TESObjectREFRArrayT* OutValidMembers = nullptr, TESObjectREFRSafeArrayT* OutDelinquents = nullptr);
			void					AddMember(TESObjectREFR* Ref);
			void					AddMember(TESObjectREFRSafeHandleT Ref);
			void					RemoveMember(TESObjectREFR* Ref);
			void					RemoveMember(TESObjectREFRSafeHandleT Ref);
			void					ClearMembers();
			void					ConvertToSelection(TESRenderSelection* Selection, bool ClearSelection = true, bool ShowSelectionBox = false);

			const char*							GetName() const;
			const UInt32						GetSize() const;
			const TESObjectREFRSafeArrayT&		GetMembers() const;

			int						Serialize(std::string& OutSerialized);		// returns the no of members serialized
			void					Deserialize(const std::string& InSerialized);
		};

		// manages distinct named collections, one-to-one mapping b'ween refs and collections
		class NamedReferenceCollectionManager
		{
			class CosaveHandler : public serialization::PluginCosaveManager::IEventHandler
			{
				NamedReferenceCollectionManager*	Parent;
			public:
				CosaveHandler(NamedReferenceCollectionManager* Parent);

				virtual void						HandleLoad(const char* PluginName, const char* CosaveDirectory);
				virtual void						HandleSave(const char* PluginName, const char* CosaveDirectory);
				virtual void						HandleShutdown(const char* PluginName, const char* CosaveDirectory);
			};

			friend class CosaveHandler;
		protected:
			typedef std::unique_ptr<NamedReferenceCollection>									CollectionHandleT;
			typedef std::vector<CollectionHandleT>												CollectionArrayT;
			typedef std::unordered_map<TESObjectREFRSafeHandleT, NamedReferenceCollection*>		Ref2CollectionMapT;

			static const char*						kSigilBeginCollection;
			static const char*						kSigilEndCollection;

			enum
			{
				kValidationPolicy_Default			= 0,
				kValidationPolicy_DissolveWhenEmpty	= 1,	// dissolve collections that are empty
				kValidationPolicy_DissolveWhenSize	= 2,	// dissolve collections that are smalled than the threshold size
			};

			virtual bool							GetCollectionExists(const char* Name) const;
			virtual NamedReferenceCollection*		LookupCollection(const char* Name) const;
			virtual NamedReferenceCollection*		GetParentCollection(TESObjectREFR* Ref) const;
			virtual NamedReferenceCollection*		GetParentCollection(TESObjectREFRSafeHandleT Ref) const;

			virtual bool							CheckCollsions(NamedReferenceCollection* Collection, bool CheckMembers) const;		// returns false if a collection of the name already exists or if any of the refs are already in another collection
			virtual bool							ValidateCollection(NamedReferenceCollection* Collection,
																	   TESObjectREFRArrayT* OutValidMembers = nullptr);					// returns false if the collection was dissolved
			virtual void							RegisterCollection(NamedReferenceCollection* Collection, bool RegisterRefs);		// takes ownership of pointer
			virtual void							DeregisterCollection(NamedReferenceCollection* Collection, bool DeregisterRefs);	// releases the pointer

			virtual void							StandardOutput(const char* Fmt, ...) const = 0;
			virtual const char*						GetSaveFileName() const = 0;
			virtual NamedReferenceCollection*		DeserializeCollection(const std::string& In) const = 0;

			virtual void							Save(const char* PluginName, const char* DirPath);
			virtual void							Load(const char* PluginName, const char* DirPath);

			virtual void							ClearCollections();
			virtual bool							AddReference(TESObjectREFR* Ref, NamedReferenceCollection* To);		// returns false if unsuccessful
			virtual bool							AddReference(TESObjectREFRSafeHandleT Ref, NamedReferenceCollection* To);
			virtual void							RemoveReference(TESObjectREFR* Ref);
			virtual void							RemoveReference(TESObjectREFRSafeHandleT Ref);

			CollectionArrayT						RegisteredCollections;
			Ref2CollectionMapT						ReferenceTable;
			CosaveHandler*							CosaveInterface;
			UInt8									ValidationPolicy;
			int										DissolveThreshold;
			bool									Initialized;
		public:
			NamedReferenceCollectionManager(UInt8 Policy, int DissolveThreshold = -1);
			virtual ~NamedReferenceCollectionManager() = 0;

			virtual void							Initialize();
			virtual void							Deinitialize();
		};
	}
}