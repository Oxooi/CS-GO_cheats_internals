#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstdint>
#include <thread>

namespace offset
{
	//Client
	constexpr std::ptrdiff_t dwLocalPlayer = 0xDEA964;
	constexpr std::ptrdiff_t dwForceJump = 0x52BBC7C;

	//Player
	constexpr std::ptrdiff_t m_iHealth = 0x100;
	constexpr std::ptrdiff_t m_fFlags = 0x104;
}

// hack function
void BunnyHop(const HMODULE instance) noexcept
{
	const auto client = reinterpret_cast<std::uintptr_t>(GetModuleHandle("client.dll"));

	//Hack Loop
	while (!GetAsyncKeyState(VK_END))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		// Make sure Space is pressed
		if (!GetAsyncKeyState(VK_SPACE))
			continue;

		//Get local player
		const auto localPlayer = *reinterpret_cast<std::uintptr_t*>(client + offset::dwLocalPlayer);
	
		if (!localPlayer)
			continue;

		const auto health = *reinterpret_cast<std::int32_t*>(localPlayer + offset::m_iHealth);
	
		//Is Alive
		if (!health)
			continue;

		const auto flags = *reinterpret_cast<std::int32_t*>(localPlayer + offset::m_fFlags);

		// On Ground
		(flags & (1 << 0)) ?
			*reinterpret_cast<std::uintptr_t*>(client + offset::dwForceJump) = 30 : //Force Jump
			*reinterpret_cast<std::uintptr_t*>(client + offset::dwForceJump) = 4 ; //Reset
	}	


	//Uninject
	FreeLibraryAndExitThread(instance, 0);
}

// Entry point
int __stdcall DllMain(
	const HMODULE instance,
	const std::uintptr_t reason,
	const void* reserved
) 
{
	// DLL_Process_Attach
	if (reason == 1) 
	{
		DisableThreadLibraryCalls(instance);

		// Create Hack Thread
		const auto thread = CreateThread(
			nullptr,
			0,
			reinterpret_cast<LPTHREAD_START_ROUTINE>(BunnyHop),
			instance,
			0,
			nullptr
		);

		if (thread)
			CloseHandle(thread);
	}

	return 1;
}