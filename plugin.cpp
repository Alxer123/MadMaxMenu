#include "plugin.h"
#include "mm/imgui/imgui.h"

DLLATTATCH;

void PluginAttach(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
}


void PluginHooks() {
	ImGuiRenderer::Install();
}