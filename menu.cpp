#include "plugin.h"
#include "mm/imgui/imgui.h"

#include "mm/core/avasingle.h"
#include "mm/core/graphics/graphicsengine.h"
#include "mm/core/input.h"

#include "mm/game/spawnsystem.h"
#include "mm/game/charactermanager.h"
#include "mm/game/game.h"

class ImGuiMenu : public ImGuiRenderer {
public:
	ImGuiMenu() : ImGuiRenderer() {

	}

	void Game() override {
		if (CGameState::m_InMainMenu || CGameState::m_State != CGameState::E_GAME_RUN || IsGuiOccludingMainDraw()) {
			SendRenderEvent([this]() { force_hide = true; });
			return;
		}

		SendRenderEvent([this]() { force_hide = false; });
	}

	void GameHandleEvent(Event const& _event) override {
		if (_event.hash == HASHSTR("focus")) {
			CAvaSingleInstance_EXE(CDeviceManager, ->GetInputManager()->SetInFocus(!_event.Arg<bool>(0)));
			CAvaSingle<NGraphicsEngine::CGraphicsEngine>::Instance->SetCursor(_event.Arg<bool>(0) ? 0 : -1);
		}
	};

	bool force_hide = true;
	bool show = false;
	char input_buffer[256] = { 0 };
	void Render() override {

		static float current_health = 1000.0f;

		if (force_hide)
			return;

		CCharacter* pPlayer = nullptr;
		if (CAvaSingle<CCharacterManager>::Instance != nullptr) {
			pPlayer = CAvaSingle<CCharacterManager>::Instance->GetPlayerCharacter();
		}

		if (ImGui::IsKeyJustDown(ImGuiKey_Backslash)) {
			show = !show;
			memset(input_buffer, 0, 256);
			SendGameEvent(Event("focus").Add(show));

			if (show && pPlayer != nullptr)
				current_health = pPlayer->GetHealth();
		}

		if (!show)
			return;

		ImGui::Begin("MadMaxMenu");

		ImGui::SliderFloat("Health", &current_health, 1, 2000);
		ImGui::SameLine();

		if (ImGui::Button("Set"))
		{
			if (pPlayer != nullptr) {
				pPlayer->SetHealth(current_health);
				//Log("Your health has been set to %g", current_health);
			}
			else
				Log("Can't grab Player Object.");
		}

		ImGui::End();
	}
};

ImGuiMenu Menu;