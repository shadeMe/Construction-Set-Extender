#include "BGSEEAchievements.h"
#include "BGSEEConsole.h"
#include "BGSEEUIManager.h"
#include "BGSEditorExtenderBase_Resource.h"

namespace BGSEditorExtender
{
	namespace Extras
	{
		BGSEEAchievement::BGSEEAchievement( const char* Title, const char* Desc, UInt32 IconID, const char* GUID ) :
			BaseIDString(GUID),
			Name(Title),
			Description(Desc),
			Unlocked(false),
			IconID(IconID)
		{
			SME_ASSERT(Name && Desc && GUID);

			ZeroMemory(&BaseID, sizeof(BaseID));
			RPC_STATUS Result = UuidFromString((RPC_CSTR)GUID, &BaseID);

			SME_ASSERT(Result == RPC_S_OK);
		}

		BGSEEAchievement::~BGSEEAchievement()
		{
			;//
		}

		bool BGSEEAchievement::UnlockCallback( BGSEEAchievementManager* Parameter )
		{
			return true;
		}

		bool BGSEEAchievement::GetUnlocked( void ) const
		{
			return Unlocked;
		}

		BGSEEAchievementManager*			BGSEEAchievementManager::Singleton = NULL;

		BGSEEAchievementManager::BGSEEAchievementManager() :
			RegistryKey(""),
			AchievementDepot(),
			ResourceInstance(0),
			Initialized(false)
		{
			;//
		}

		void BGSEEAchievementManager::SaveAchievementState( BGSEEAchievement* Achievement )
		{
			SetRegValue(Achievement->BaseIDString.c_str(), Achievement->Unlocked);
		}

		void BGSEEAchievementManager::LoadAchievementState( BGSEEAchievement* Achievement )
		{
			UInt32 UnlockedState = 0;

			if (GetRegValue(Achievement->BaseIDString.c_str(), &UnlockedState))
			{
				if (UnlockedState)
					Achievement->Unlocked = true;
			}
			else
			{
				SetRegValue(Achievement->BaseIDString.c_str(), 0);
				Achievement->Unlocked = false;
			}
		}

		bool BGSEEAchievementManager::GetRegValue( const char* Name, UInt32* OutValue )
		{
			HKEY BaseAchievementKey = NULL;

			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegistryKey.c_str(), NULL, KEY_ALL_ACCESS, &BaseAchievementKey) == ERROR_SUCCESS)
			{
				UInt32 Type = 0, Size = 4;

				if (RegQueryValueEx(BaseAchievementKey, Name, NULL, &Type, (LPBYTE)OutValue, &Size) == ERROR_SUCCESS)
					return true;
			}

			return false;
		}

		bool BGSEEAchievementManager::SetRegValue( const char* Name, UInt32 Value )
		{
			HKEY BaseAchievementKey = NULL;

			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegistryKey.c_str(), NULL, KEY_ALL_ACCESS, &BaseAchievementKey) == ERROR_SUCCESS)
			{
				if (RegSetValueEx(BaseAchievementKey, Name, NULL, REG_DWORD, (const BYTE*)&Value, REG_DWORD) == ERROR_SUCCESS)
					return true;
			}

			return false;
		}

		BGSEEAchievementManager::~BGSEEAchievementManager()
		{
			Singleton = NULL;

			AchievementDepot.clear();
			Initialized = false;
		}

		BGSEEAchievementManager* BGSEEAchievementManager::GetSingleton( void )
		{
			if (Singleton == NULL)
				Singleton = new BGSEEAchievementManager();

			return Singleton;
		}

		bool BGSEEAchievementManager::Initialize( const char* ExtenderLongName, HINSTANCE ResourceInstance, ExtenderAchievementListT& Achievements )
		{
			if (Initialized)
				return false;

			SME_ASSERT(ExtenderLongName && ResourceInstance);

			this->Initialized = true;
			this->RegistryKey = "Software\\Imitation Camel\\" + std::string(ExtenderLongName) + "\\Achievements\\";
			this->ResourceInstance = ResourceInstance;
			this->AchievementDepot = Achievements;

			HKEY BaseAchievementKey = NULL;
			if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, RegistryKey.c_str(), NULL, NULL, NULL, KEY_ALL_ACCESS , NULL, &BaseAchievementKey, NULL) != ERROR_SUCCESS)
			{
				BGSEECONSOLE_ERROR("Couldn't create base registry key!");
			}

			UInt32 UnlockedCount = 0;
			for (ExtenderAchievementListT::iterator Itr = AchievementDepot.begin(); Itr != AchievementDepot.end(); Itr++)
			{
				LoadAchievementState(*Itr);

				if ((*Itr)->Unlocked)
					UnlockedCount++;
			}

			BGSEECONSOLE_MESSAGE("Unlocked Achievements: %d/%d", UnlockedCount, AchievementDepot.size());
			BGSEECONSOLE->Indent();
			for (ExtenderAchievementListT::iterator Itr = AchievementDepot.begin(); Itr != AchievementDepot.end(); Itr++)
			{
				if ((*Itr)->Unlocked)
					BGSEECONSOLE_MESSAGE((*Itr)->Name);
			}
			BGSEECONSOLE->Exdent();

			return Initialized;
		}

		void BGSEEAchievementManager::Unlock( BGSEEAchievement* Achievement )
		{
			if (Initialized == false)
				return;

			if (Achievement->Unlocked || Achievement->UnlockCallback(this) == false)
				return;

			Achievement->Unlocked = true;
			SaveAchievementState(Achievement);

			BGSEECONSOLE_MESSAGE("New Achievement Unlocked: %s", Achievement->Name);

			NotificationUserData* UserData = new NotificationUserData();
			UserData->Instance = this;
			UserData->Achievement = Achievement;

			BGSEEUI->ModalDialog(ResourceInstance,
								MAKEINTRESOURCE(IDD_BGSEE_ACHIEVEMENTUNLOCKED),
								NULL,
								NotificationDlgProc,
								(LPARAM)UserData);
		}

#define TIMERID_VISIBLE			8500

		INT_PTR CALLBACK BGSEEAchievementManager::NotificationDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			NotificationUserData* UserData = (NotificationUserData*)GetWindowLong(hWnd, GWL_USERDATA);
			BGSEEAchievementManager* Instance = UserData->Instance;
			BGSEEAchievement* Unlocked = UserData->Achievement;

			switch (uMsg)
			{
			case WM_TIMER:
				switch (wParam)
				{
				case TIMERID_VISIBLE:
					SendMessage(hWnd, WM_CLOSE, NULL, NULL);
					break;
				}

				break;
			case WM_KEYDOWN:
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
				SendMessage(hWnd, WM_CLOSE, NULL, NULL);
				break;
			case WM_CTLCOLORDLG:
				return (INT_PTR)UserData->BGBrush;
			case WM_CTLCOLORSTATIC:
				if (GetDlgItem(hWnd, IDC_BGSEE_ACHIEVEMENTUNLOCKED_TITLE) == (HWND)lParam)
					SetTextColor((HDC)wParam, RGB(109, 142, 64));
				else
					SetTextColor((HDC)wParam, RGB(255, 255, 255));

				SetBkMode((HDC)wParam, TRANSPARENT);
				return (INT_PTR)UserData->BGBrush;
			case WM_INITDIALOG:
				{
					SetWindowLong(hWnd, GWL_USERDATA, (LONG)lParam);
					UserData = (NotificationUserData*)lParam;
					Instance = UserData->Instance;
					Unlocked = UserData->Achievement;

					UserData->BGBrush = CreateSolidBrush(RGB(46, 48, 54));
					UserData->Icon = LoadImage(Instance->ResourceInstance, MAKEINTRESOURCE(Unlocked->IconID), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);

					SetDlgItemText(hWnd, IDC_BGSEE_ACHIEVEMENTUNLOCKED_TITLE, Unlocked->Name);
					SetDlgItemText(hWnd, IDC_BGSEE_ACHIEVEMENTUNLOCKED_JINGLE, Unlocked->Description);
					SendDlgItemMessage(hWnd, IDC_BGSEE_ACHIEVEMENTUNLOCKED_ICON, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)UserData->Icon);

					SetTimer(hWnd, TIMERID_VISIBLE, 5500, NULL);
					AnimateWindow(hWnd, 150, AW_BLEND);
				}

				break;
			case WM_CLOSE:
				AnimateWindow(hWnd, 150, AW_HIDE|AW_BLEND);
				DestroyWindow(hWnd);
				break;
			case WM_DESTROY:
				{
					DeleteObject(UserData->Icon);
					DeleteObject(UserData->BGBrush);

					KillTimer(hWnd, TIMERID_VISIBLE);
					delete UserData;
				}

				break;
			}

			return FALSE;
		}
	}
}