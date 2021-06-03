#include "Achievements.h"
#include "Console.h"
#include "UIManager.h"
#include "BGSEditorExtenderBase_Resource.h"

namespace bgsee
{
	namespace extras
	{
		Achievement::Achievement( const char* Title, const char* Desc, UInt32 IconID, const char* GUID ) :
			BaseIDString(GUID),
			Name(Title),
			Description(Desc),
			State(kState_Locked),
			ExtraData(0),
			IconID(IconID)
		{
			SME_ASSERT(GUID);

			ZeroMemory(&BaseID, sizeof(BaseID));
			RPC_STATUS Result = UuidFromString((RPC_CSTR)GUID, &BaseID);

			SME_ASSERT(Result == RPC_S_OK);
		}

		Achievement::~Achievement()
		{
			;//
		}

		bool Achievement::UnlockCallback( AchievementManager* Parameter )
		{
			return true;
		}

		bool Achievement::GetUnlocked( void ) const
		{
			if (GetUnlockable() == false)
				return false;

			return State == kState_Unlocked;
		}

		bool Achievement::SaveCallback( AchievementManager* Parameter )
		{
			return true;
		}

		bool Achievement::GetTriggered( void ) const
		{
			return State == kState_Triggered;
		}

		void Achievement::GetName( std::string& OutBuffer ) const
		{
			SME_ASSERT(Name.length());

			OutBuffer = Name;
		}

		void Achievement::GetDescription( std::string& OutBuffer ) const
		{
			SME_ASSERT(Description.length());

			OutBuffer = Description;
		}

		bool Achievement::GetUnlockable( void ) const
		{
			return true;
		}

		AchievementManager*			AchievementManager::Singleton = nullptr;

		AchievementManager::AchievementManager(const char* ExtenderLongName, HINSTANCE ResourceInstance, ExtenderAchievementArrayT& Achievements) :
			RegistryKeyRoot(""),
			RegistryKeyExtraData(""),
			AchievementDepot(),
			ResourceInstance(0),
			Initialized(false)
		{
			SME_ASSERT(Singleton == nullptr);
			Singleton = this;

			SME_ASSERT(ExtenderLongName && ResourceInstance);

			this->Initialized = true;
			this->RegistryKeyRoot = "Software\\Imitation Camel\\" + std::string(ExtenderLongName) + "\\Achievements\\";
			this->RegistryKeyExtraData = RegistryKeyRoot + "ExtraData\\";
			this->ResourceInstance = ResourceInstance;
			this->AchievementDepot = Achievements;

			HKEY BaseAchievementKey = nullptr, ExtraDataAchievementKey = nullptr;
			if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
							   RegistryKeyRoot.c_str(),
							   NULL, nullptr, NULL,
							   KEY_ALL_ACCESS,
							   nullptr,
							   &BaseAchievementKey,
							   nullptr) != ERROR_SUCCESS)
			{
				BGSEECONSOLE_ERROR("Couldn't create root registry key!");
			}

			if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
							   RegistryKeyExtraData.c_str(),
							   NULL, nullptr, NULL,
							   KEY_ALL_ACCESS,
							   nullptr,
							   &ExtraDataAchievementKey,
							   nullptr) != ERROR_SUCCESS)
			{
				BGSEECONSOLE_ERROR("Couldn't create extradata registry key!");
			}

			UInt32 UnlockedCount = 0;
			for (ExtenderAchievementArrayT::iterator Itr = AchievementDepot.begin(); Itr != AchievementDepot.end(); Itr++)
			{
				LoadAchievementState(*Itr);

				if ((*Itr)->GetUnlocked())
					UnlockedCount++;
				else if ((*Itr)->State == Achievement::kState_Triggered)
				{
					Unlock(*Itr, true);
					UnlockedCount++;
				}
			}

			BGSEECONSOLE_MESSAGE("Unlocked Achievements: %d/%d", UnlockedCount, GetTotalAchievements());
			BGSEECONSOLE->Indent();
			for (ExtenderAchievementArrayT::iterator Itr = AchievementDepot.begin(); Itr != AchievementDepot.end(); Itr++)
			{
				if ((*Itr)->GetUnlocked())
					BGSEECONSOLE_MESSAGE((*Itr)->Name.c_str());
			}
			BGSEECONSOLE->Outdent();
		}

		void AchievementManager::SaveAchievementState( Achievement* Achievement, bool StateOnly )
		{
			if (Achievement->SaveCallback(this) == false)
				return;

			SetRegValue<UInt32>(Achievement->BaseIDString.c_str(), Achievement->State, RegistryKeyRoot.c_str());

			if (StateOnly)
				return;

			SetRegValue<UInt64>(Achievement->BaseIDString.c_str(), Achievement->ExtraData, RegistryKeyExtraData.c_str());
		}

		void AchievementManager::LoadAchievementState( Achievement* Achievement, bool StateOnly )
		{
			UInt32 State = 0;
			UInt64 ExtraData = 0;

			if (GetRegValue<UInt32>(Achievement->BaseIDString.c_str(), &State, RegistryKeyRoot.c_str()))
			{
				if (State)
					Achievement->State = State;
			}
			else
			{
				SetRegValue<UInt32>(Achievement->BaseIDString.c_str(), 0, RegistryKeyRoot.c_str());
				Achievement->State = Achievement::kState_Locked;
			}

			if (StateOnly)
				return;

			if (GetRegValue<UInt64>(Achievement->BaseIDString.c_str(), &ExtraData, RegistryKeyExtraData.c_str()) == FALSE)
			{
				Achievement->ExtraData = 0;
				SetRegValue<UInt64>(Achievement->BaseIDString.c_str(), 0, RegistryKeyExtraData.c_str());
			}
			else
			{
				Achievement->ExtraData = ExtraData;
			}
		}

		template <typename T>
		bool AchievementManager::GetRegValue( const char* Name, T* OutValue, const char* Key )
		{
			HKEY AchievementKey = nullptr;

			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, Key, NULL, KEY_ALL_ACCESS, &AchievementKey) == ERROR_SUCCESS)
			{
				UInt32 Type = REG_DWORD, Size = sizeof(T);
				if (Size > Type)
					Type = REG_QWORD;			// you lazy bum!

				if (RegQueryValueEx(AchievementKey, Name, NULL, &Type, (LPBYTE)OutValue, &Size) == ERROR_SUCCESS)
					return true;
			}

			return false;
		}

		template <typename T>
		bool AchievementManager::SetRegValue( const char* Name, T Value, const char* Key )
		{
			HKEY AchievementKey = nullptr;

			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, Key, NULL, KEY_ALL_ACCESS, &AchievementKey) == ERROR_SUCCESS)
			{
				UInt32 Size = REG_DWORD;
				if (sizeof(T) > Size)
					Size = REG_QWORD;

				if (RegSetValueEx(AchievementKey, Name, NULL, Size, (const BYTE*)&Value, REG_DWORD) == ERROR_SUCCESS)
					return true;
			}

			return false;
		}

		AchievementManager::~AchievementManager()
		{
			for (ExtenderAchievementArrayT::iterator Itr = AchievementDepot.begin(); Itr != AchievementDepot.end(); Itr++)
			{
				SaveAchievementState(*Itr);
				delete *Itr;
			}

			AchievementDepot.clear();
			Initialized = false;

			Singleton = nullptr;
		}

		AchievementManager* AchievementManager::Get( void )
		{
			return Singleton;
		}

		bool AchievementManager::Initialize( const char* ExtenderLongName, HINSTANCE ResourceInstance, ExtenderAchievementArrayT& Achievements )
		{
			if (Singleton)
				return false;

			AchievementManager* Buffer = new AchievementManager(ExtenderLongName, ResourceInstance, Achievements);
			return Buffer->Initialized;
		}

		void AchievementManager::Deinitialize()
		{
			SME_ASSERT(Singleton);
			delete Singleton;
		}

		void AchievementManager::Unlock(Achievement* Achievement, bool ForceUnlock, bool TriggerOnly, bool PreserveUnlockState)
		{
			if (Achievement->GetUnlocked())
				return;

			if (PreserveUnlockState == false)
				Achievement->State = Achievement::kState_Triggered;

			if (TriggerOnly == false)
			{
				if (ForceUnlock == false && Achievement->UnlockCallback(this) == false)
				{
					if (PreserveUnlockState == false)
						Achievement->State = Achievement::kState_Locked;

					return;
				}

				if (PreserveUnlockState == false)
				{
					Achievement->State = Achievement::kState_Unlocked;
					BGSEECONSOLE_MESSAGE("New Achievement Unlocked: %s", Achievement->Name.c_str());
				}

				NotificationUserData* UserData = new NotificationUserData();
				UserData->Instance = this;
				UserData->Achievement = Achievement;

				BGSEEUI->ModelessDialog(ResourceInstance,
					MAKEINTRESOURCE(IDD_BGSEE_ACHIEVEMENTUNLOCKED),
					nullptr,
					NotificationDlgProc,
					(LPARAM)UserData, false);
			}

			SaveAchievementState(Achievement, true);
		}

#define TIMERID_VISIBLE			8500

		INT_PTR CALLBACK AchievementManager::NotificationDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			NotificationUserData* UserData = (NotificationUserData*)GetWindowLongPtr(hWnd, GWL_USERDATA);
			AchievementManager* Instance = nullptr;
			Achievement* UnlockedAchievement = nullptr;

			if (UserData)
			{
				Instance = UserData->Instance;
				UnlockedAchievement = UserData->Achievement;
			}

			switch (uMsg)
			{
			case WM_TIMER:
				switch (wParam)
				{
				case TIMERID_VISIBLE:
		//			SendMessage(hWnd, WM_CLOSE, NULL, NULL);
					break;
				}

				break;
			case WM_KEYUP:
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			case WM_MBUTTONUP:
				SendMessage(hWnd, WM_CLOSE, NULL, NULL);
				break;
			case WM_CTLCOLORDLG:
				return (INT_PTR)UserData->BGBrush;
			case WM_CTLCOLORSTATIC:
				if (GetDlgItem(hWnd, IDC_BGSEE_ACHIEVEMENTUNLOCKED_TITLE) == (HWND)lParam)
					SetTextColor((HDC)wParam, RGB(113, 185, 223));
				else
					SetTextColor((HDC)wParam, RGB(255, 255, 255));

				SetBkMode((HDC)wParam, TRANSPARENT);
				return (INT_PTR)UserData->BGBrush;
			case WM_INITDIALOG:
				{
					SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)lParam);
					UserData = (NotificationUserData*)lParam;
					Instance = UserData->Instance;
					UnlockedAchievement = UserData->Achievement;

					UserData->BGBrush = CreateSolidBrush(RGB(46, 48, 54));
					if (UnlockedAchievement->IconID)
						UserData->Icon = LoadImage(Instance->ResourceInstance, MAKEINTRESOURCE(UnlockedAchievement->IconID), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
					else
						UserData->Icon = LoadImage(Instance->ResourceInstance, MAKEINTRESOURCE(IDB_BGSEE_GENERIC), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);

					HFONT TitleFont = CreateFont(28,
												0,
												0,
												0,
												FW_BOLD,
												FALSE,
												FALSE,
												FALSE,
												ANSI_CHARSET,
												OUT_DEFAULT_PRECIS,
												CLIP_DEFAULT_PRECIS,
												CLEARTYPE_QUALITY,
												FF_DONTCARE,
												"Segoe UI Semibold");

					HFONT JingleFont = CreateFont(18,
												0,
												0,
												0,
												FW_REGULAR,
												FALSE,
												FALSE,
												FALSE,
												ANSI_CHARSET,
												OUT_DEFAULT_PRECIS,
												CLIP_DEFAULT_PRECIS,
												CLEARTYPE_QUALITY,
												FF_DONTCARE,
												"Segoe UI Semibold");

					SendDlgItemMessage(hWnd, IDC_BGSEE_ACHIEVEMENTUNLOCKED_TITLE, WM_SETFONT, (WPARAM)TitleFont, (LPARAM)TRUE);
					SendDlgItemMessage(hWnd, IDC_BGSEE_ACHIEVEMENTUNLOCKED_JINGLE, WM_SETFONT, (WPARAM)JingleFont, (LPARAM)TRUE);
					SendDlgItemMessage(hWnd, 1, WM_SETFONT, (WPARAM)JingleFont, (LPARAM)TRUE);

					std::string Buffer;

					UnlockedAchievement->GetName(Buffer);
					SetDlgItemText(hWnd, IDC_BGSEE_ACHIEVEMENTUNLOCKED_TITLE, Buffer.c_str());

					UnlockedAchievement->GetDescription(Buffer);
					SetDlgItemText(hWnd, IDC_BGSEE_ACHIEVEMENTUNLOCKED_JINGLE, Buffer.c_str());

					SendDlgItemMessage(hWnd, IDC_BGSEE_ACHIEVEMENTUNLOCKED_ICON, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)UserData->Icon);

					SetTimer(hWnd, TIMERID_VISIBLE, 6500, nullptr);
					AnimateWindow(hWnd, 155, AW_BLEND);
				}

				break;
			case WM_CLOSE:
				AnimateWindow(hWnd, 100, AW_HIDE|AW_BLEND);
				DestroyWindow(hWnd);
				break;
			case WM_DESTROY:
				{
					DeleteObject((HFONT)SendDlgItemMessage(hWnd, IDC_BGSEE_ACHIEVEMENTUNLOCKED_TITLE, WM_GETFONT, NULL, NULL));
					DeleteObject((HFONT)SendDlgItemMessage(hWnd, IDC_BGSEE_ACHIEVEMENTUNLOCKED_JINGLE, WM_GETFONT, NULL, NULL));

					DeleteObject(UserData->Icon);
					DeleteObject(UserData->BGBrush);

					KillTimer(hWnd, TIMERID_VISIBLE);
					delete UserData;
				}

				break;
			}

			return FALSE;
		}

		UInt32 AchievementManager::GetTotalAchievements( void ) const
		{
			UInt32 Result = 0;

			for (ExtenderAchievementArrayT::const_iterator Itr = AchievementDepot.begin(); Itr != AchievementDepot.end(); Itr++)
			{
				if ((*Itr)->GetUnlockable())
					Result++;
			}

			return Result;
		}

		UInt32 AchievementManager::GetUnlockedAchievements( void ) const
		{
			UInt32 Result = 0;

			for (ExtenderAchievementArrayT::const_iterator Itr = AchievementDepot.begin(); Itr != AchievementDepot.end(); Itr++)
			{
				if ((*Itr)->GetUnlocked())
					Result++;
			}

			return Result;
		}
	}
}