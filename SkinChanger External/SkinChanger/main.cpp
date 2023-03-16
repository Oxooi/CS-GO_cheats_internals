#include "memory.h"

#include <thread>
#include <array>

namespace offset
{
	// client
	constexpr ::std::ptrdiff_t dwLocalPlayer = 0xDEA964;
	constexpr ::std::ptrdiff_t dwEntityList = 0x4DFFEF4;
	constexpr ::std::ptrdiff_t dwClientState = 0x59F19C;

	// player
	constexpr ::std::ptrdiff_t m_hMyWeapons = 0x2E08;

	// base attributable
	constexpr ::std::ptrdiff_t m_flFallbackWear = 0x31E0;
	constexpr ::std::ptrdiff_t m_nFallbackPaintKit = 0x31D8;
	constexpr ::std::ptrdiff_t m_nFallbackSeed = 0x31DC;
	constexpr ::std::ptrdiff_t m_nFallbackStatTrak = 0x31E4;
	constexpr ::std::ptrdiff_t m_iItemDefinitionIndex = 0x2FBA;
	constexpr ::std::ptrdiff_t m_iItemIDHigh = 0x2FD0;
	constexpr ::std::ptrdiff_t m_iEntityQuality = 0x2FBC;
	constexpr ::std::ptrdiff_t m_iAccountID = 0x2FD8;
	constexpr ::std::ptrdiff_t m_OriginalOwnerXuidLow = 0x31D0;

}

// Set skins to apply here
constexpr const int GetWeaponPaint(const short& itemDefinition)
{
	switch (itemDefinition)
	{
	case 1: return 586; // Deagle
	case 4: return 586; // Glock
	case 7: return 586; // Ak47
	case 9: return 344; // Awp
	case 61: return 653; // Usp
	default: return 0;
	}
}

int main()
{

	const auto memory = Memory("csgo.exe");

	// Get our module addresses
	const auto client = memory.GetModuleAddress("client.dll");
	const auto engine = memory.GetModuleAddress("engine.dll");

	// hack loop
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(2));

		// Get our local player
		const auto localPlayer = memory.Read<std::uintptr_t>(client + offset::dwLocalPlayer);
		// Get our local player's weapons
		const auto& weapons = memory.Read<std::array<unsigned long, 8>>(localPlayer + offset::m_hMyWeapons);

		// Loop through our weapons
		for (const auto& handle : weapons)
		{
			const auto& weapon = memory.Read<std::uintptr_t>((client + offset::dwEntityList + (handle & 0xFFF) * 0x10) - 0x10);

			// Make sure the weapon is valid
			if(!weapon)
				continue;

			// If we want to apply a skin to this weapon
			if (const auto paint = GetWeaponPaint(memory.Read<short>(weapon + offset::m_iItemDefinitionIndex)))
			{
				const bool shouldUpdate = memory.Read<std::int32_t>(weapon + offset::m_nFallbackPaintKit) != paint;

				// force weapon to use fallback values
				memory.Write<std::int32_t>(weapon + offset::m_iItemIDHigh, -1);

				// set fallback values
				memory.Write<std::int32_t>(weapon + offset::m_nFallbackPaintKit, paint);
				memory.Write<std::int32_t>(weapon + offset::m_flFallbackWear, 0.1f);

				// Add some foolish
				memory.Write<std::int32_t>(weapon + offset::m_nFallbackSeed, 0);
				memory.Write<std::int32_t>(weapon + offset::m_nFallbackStatTrak, 1337);
				memory.Write<std::int32_t>(weapon + offset::m_iAccountID, memory.Read<std::int32_t>(weapon + offset::m_OriginalOwnerXuidLow));

				if (shouldUpdate)
					memory.Write<std::int32_t>(memory.Read<std::uintptr_t>(engine + offset::dwClientState) + 0x174, -1);

			}

		}
	}

	return 0;
}