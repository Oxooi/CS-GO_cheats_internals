#pragma once
#include <d3d9.h>

namespace gui
{
	// Show Menu
	inline bool open = true;

	// Is menu setup
	inline bool setup = false;

	// Winapi related
	inline HWND window = nullptr;
	inline WNDCLASSEX windowClass = { };
	inline WNDPROC orignialWindowProcess = nullptr;

	// Dx stuff
	inline LPDIRECT3DDEVICE9 device = nullptr;
	inline LPDIRECT3D9 d3d9 = nullptr;

	bool SetupWindowClass(const char* windowClassName) noexcept;
	void DestroyWindowClass() noexcept;

	bool SetupWindow(const char* windowName) noexcept;
	void DestroyWindow() noexcept;

	bool SetupDirectX() noexcept;
	void DestroyDirectX() noexcept;

	// Setup device
	void Setup();

	void SetupMenu(LPDIRECT3DDEVICE9 device) noexcept;
	void Destroy() noexcept;

	void Render() noexcept;
}