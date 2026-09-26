#include "plugin.h"
#include "mm/imgui/imgui.h"

#include "mm/core/avasingle.h"
#include "mm/core/graphics/graphicsengine.h"
#include "mm/core/input.h"

#include "mm/game/spawnsystem.h"
#include "mm/game/charactermanager.h"
#include "mm/game/game.h"
#include "mm/game/go/vehicle.h"

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
		static bool bInvulnerable = false;

		if (force_hide)
			return;

		CCharacter* pPlayer = nullptr;
		CVehicle* pVehicle = nullptr;
		CVector3f* pPos = nullptr;

		if (CAvaSingle<CCharacterManager>::Instance != nullptr) {
			pPlayer = CAvaSingle<CCharacterManager>::Instance->GetPlayerCharacter();
		}

		if (pPlayer != nullptr) {
			pVehicle = pPlayer->GetVehiclePtr();
			pPos = pPlayer->GetPosition();
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


		if (ImGui::BeginTabBar("DevMenuTabs"))
		{
			
			if (ImGui::BeginTabItem("Player"))
			{
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

				ImGui::Checkbox("Invulnerable (Only foot only)", &bInvulnerable);

				if (pPlayer != nullptr)
				{
					pPlayer->SetInvulnerable(bInvulnerable);
				}

				ImGui::EndTabItem();
			}



			if (ImGui::BeginTabItem("World"))
			{
				struct FastTravelPoint {
					const char* Name;
					CVector3f Coords;
				};
				
				static FastTravelPoint locations[] = {
					{ "Jeet's Stronghold",   { -3611.0f, 500.0f, 3880.0f } },
					{ "Gutgash's Stronghold",{ -7015.0f, 365.0f, 4870.0f } },
					{ "Pink Eye's Silo",     { -6890.0f, 511.0f, 36.0f } },
					{ "Gastown",             { -3780.0f, 577.0f, -3090.0f } },
					{ "Deep Friah's Temple", { -3938.0f, 476.0f, -1431.0f } },
					{ "Chum's Hideout",      { -3139.0f, 321.0f, 6441.0f } },
					{ "The Dunes Region",    { -509.0f, 484.0f, -732.0f } },
					{ "a00_tests_sound_explosions",    { 4792.0f, 248, 13466.0f } },
					{ "a00_tests_sound_room_types",    { 4789.0f, 248, 13790.0f } },
					{ "a00_tests_spawn_vehicle",    { 2131.0f, 248, 13453.0f } }
				};

				static int selected_idx = 0;

				ImGui::Text("Fast Travel");
				ImGui::Spacing();

				if (ImGui::BeginCombo("##LocationCombo", locations[selected_idx].Name))
				{
					for (int i = 0; i < IM_ARRAYSIZE(locations); i++)
					{
						bool is_selected = (selected_idx == i);
						if (ImGui::Selectable(locations[i].Name, is_selected)) {
							selected_idx = i;
						}
						if (is_selected) {
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				ImGui::Spacing();

				ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Target XYZ: %.1f, %.1f, %.1f",
					locations[selected_idx].Coords.x,
					locations[selected_idx].Coords.y,
					locations[selected_idx].Coords.z);

				ImGui::Spacing();

				if (ImGui::Button("Teleport")) {
					CVector3f targetCoords = locations[selected_idx].Coords;

					SendGameEvent([targetCoords]() {
						CCharacter* tPlayer = CAvaSingle<CCharacterManager>::Instance->GetPlayerCharacter();

						if (tPlayer) {
							CMatrix4f targetMatrix;
							CVehicle* tVehicle = tPlayer->GetVehiclePtr();
							CGameObject* pTargetGO = nullptr;

							if (tVehicle) {
								pTargetGO = tVehicle;
								tVehicle->SetVelocity(CVector3f());
								tVehicle->GetTransform(&targetMatrix);
							}
							else {
								pTargetGO = tPlayer;
								tPlayer->ForceNeutralState();
								tPlayer->GetTransform(&targetMatrix);
							}

							targetMatrix.SetPosition(targetCoords);
							pTargetGO->SetTransform(&targetMatrix);
						}
					});
				}

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Text("Custom Teleport");
				ImGui::Spacing();

				static CVector3f custom_coords = { 0.0f, 0.0f, 0.0f };

				ImGui::InputFloat3("X / Y / Z", &custom_coords.x);

				if (ImGui::Button("Teleport to Custom XYZ"))
				{
					CVector3f targetCoords = custom_coords;

					SendGameEvent([targetCoords]() {
						CCharacter* tPlayer = CAvaSingle<CCharacterManager>::Instance->GetPlayerCharacter();
						if (tPlayer != nullptr) {
							CMatrix4f targetMatrix;
							CVehicle* tVehicle = tPlayer->GetVehiclePtr();
							CGameObject* pTargetGO = nullptr;

							if (tVehicle) {
								pTargetGO = (CGameObject*)tVehicle;
								tVehicle->SetVelocity(CVector3f());
								tVehicle->GetTransform(&targetMatrix);
							}
							else {
								pTargetGO = (CGameObject*)tPlayer;
								tPlayer->ForceNeutralState();
								tPlayer->GetTransform(&targetMatrix);
							}

							targetMatrix.SetPosition(targetCoords);
							pTargetGO->SetTransform(&targetMatrix);
						}
						});
				}


				if (pPos != nullptr) {
					ImGui::Text("Current X: %.1f, Y(height): %.1f, Z: %.1f", pPos->x, pPos->y, pPos->z);
				}
				else {
					ImGui::Text("Waiting for Player object...");
				}

				ImGui::EndTabItem();
			}



			if (ImGui::BeginTabItem("Debug"))
			{
				ImGui::Text("Player Base Address: 0x%llX", (uintptr_t)pPlayer);
				ImGui::SameLine();
				if (ImGui::Button("Copy##Player"))
				{
					char hexString[32];
					snprintf(hexString, sizeof(hexString), "0x%llX", (uintptr_t)pPlayer);
					ImGui::SetClipboardText(hexString);
				}

				ImGui::Text("Vehicle Base Address: 0x%llX", (uintptr_t)pVehicle);
				ImGui::SameLine();
				if (ImGui::Button("Copy##Vehicle"))
				{
					char hexString[32];
					snprintf(hexString, sizeof(hexString), "0x%llX", (uintptr_t)pVehicle);
					ImGui::SetClipboardText(hexString);
				}


				ImGui::EndTabItem();
			}

		}

		ImGui::End();
	}
};

ImGuiMenu Menu;