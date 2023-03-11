#include "hooks.h"

// include minhook for epic hookage
#include "../../ext/minhook/minhook.h"

#include <intrin.h>

#include "../hacks/aimbot.h"
#include "../hacks/misc.h"

void hooks::Setup() noexcept
{
	MH_Initialize();

	// AllocKeyValuesMemory hook
	MH_CreateHook(
		memory::Get(interfaces::keyValuesSystem, 1),
		&AllocKeyValuesMemory,
		reinterpret_cast<void**>(&AllocKeyValuesMemoryOriginal)
	);

	// CreateMove hook
	MH_CreateHook(
		memory::Get(interfaces::clientMode, 24),
		&CreateMove,
		reinterpret_cast<void**>(&CreateMoveOriginal)
	);

	// PaintTraverse hook
	MH_CreateHook(
		memory::Get(interfaces::panel, 41),
		&PaintTraverse,
		reinterpret_cast<void**>(&PaintTraverseOriginal)
	);

	MH_EnableHook(MH_ALL_HOOKS);
}

void hooks::Destroy() noexcept
{
	// restore hooks
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);

	// uninit minhook
	MH_Uninitialize();
}

void* __stdcall hooks::AllocKeyValuesMemory(const std::int32_t size) noexcept
{
	// if function is returning to speficied addresses, return nullptr to "bypass"
	if (const std::uint32_t address = reinterpret_cast<std::uint32_t>(_ReturnAddress());
		address == reinterpret_cast<std::uint32_t>(memory::allocKeyValuesEngine) ||
		address == reinterpret_cast<std::uint32_t>(memory::allocKeyValuesClient)) 
		return nullptr;

	// return original
	return AllocKeyValuesMemoryOriginal(interfaces::keyValuesSystem, size);
}

bool __stdcall hooks::CreateMove(float frameTime, CUserCmd* cmd) noexcept
{
	// make sure this function is being called from CInput::CreateMove
	if (!cmd->commandNumber)
		return CreateMoveOriginal(interfaces::clientMode, frameTime, cmd);

	
	// get our local player here
	globals::UpdateLocalPlayer();

	if (globals::localPlayer && globals::localPlayer->IsAlive())
	{
		// example bhop
		hacks::RunBunnyHop(cmd);

		//run aimbot
		hacks::RunAimbot(cmd);
	}

	// this would be done anyway by returning true
	if (CreateMoveOriginal(interfaces::clientMode, frameTime, cmd))
		interfaces::engine->SetViewAngles(cmd->viewAngles);


	return false;
}

// ESP Hook
void __stdcall hooks::PaintTraverse(std::uintptr_t vguiPanel, bool forceRepaint, bool allowForce) noexcept
{
	// make sure we have the correct panel
	if (vguiPanel == interfaces::engineVGui->GetPanel(PANEL_TOOLS))
	{
		// Make sure local player is valid && we are in-game
		if (globals::localPlayer && interfaces::engine->IsInGame())
		{
			// Loop though the player list
			for (int i = 1; i <= interfaces::globals->maxClients; ++i)
			{
				// Get the player pointer
				CEntity* player = interfaces::entityList->GetEntityFromIndex(i);

				// Make sure the player is not nullptr
				if (!player)
					continue;

				// make sure the player is not dormant && is alive
				if (player->IsDormant() || !player->IsAlive())
					continue;

				//// no ESP on teamates
				//if (player->GetTeam() == globals::localPlayer->GetTeam())
				//	continue;

				// Dont do ESP on player we are spectating
				if (!globals::localPlayer->IsAlive())
					if (globals::localPlayer->GetObserverTarget() == player)
						continue;

				//Player's bone matrix
				CMatrix3x4 bones[128];
				if (!player->SetupBones(bones, 128, 0x7FF00, interfaces::globals->currentTime))
					continue;

				// Screen position of head
				CVector top;
				if (interfaces::debugOverlay->ScreenPosition(bones[8].Origin() + CVector{ 0.f, 0.f, 11.f }, top))
					continue;

				// Screen position of feet
				CVector bottom;
				if (interfaces::debugOverlay->ScreenPosition(player->GetAbsOrigin() - CVector{0.f, 0.f, 9.f}, bottom))
					continue;

				// Height of box
				const float h = bottom.y - top.y;

				// Use the height to determinate a width.
				const float w = h * 0.3f;

				const auto left = static_cast<int>(top.x - w);
				const auto right = static_cast<int>(top.x + w);

				// Set drawing color to white
				interfaces::surface->DrawSetColor(255, 255, 255, 255);

				// Draw normal box 
				interfaces::surface->DrawOutlinedRect(left, top.y, right, bottom.y);
			
				// Set the color to black for outlines
				interfaces::surface->DrawSetColor(0, 0, 0, 255);

				// Draw outlines
				interfaces::surface->DrawOutlinedRect(left - 1, top.y - 1, right + 1, bottom.y + 1);
				interfaces::surface->DrawOutlinedRect(left + 1, top.y + 1, right - 1, bottom.y - 1);

				//Health bar outlines (drawing color is alreay black)
				interfaces::surface->DrawOutlinedRect(left - 6, top.y - 1, left - 3, bottom.y + 1);

				// health bar is an int from 100 -> 0, get percentage
				const float healthFrac = player->GetHealth() * 0.01f;

				// set the health bar to a split between red / green
				interfaces::surface->DrawSetColor((1.f - healthFrac) * 255, 255 * healthFrac, 0, 255);

				// Draw the health bar
				interfaces::surface->DrawFilledRect(left - 5, bottom.y - (h * healthFrac), left - 4, bottom.y);

			}
		}
	}

	// Call the original function
	PaintTraverseOriginal(interfaces::panel, vguiPanel, forceRepaint, allowForce);
}