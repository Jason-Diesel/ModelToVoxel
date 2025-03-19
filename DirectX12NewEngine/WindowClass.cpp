#include "Library/Imgui/imgui_impl_dx12.h"
#include "Library/Imgui/imgui_impl_win32.h"
#include "Library/Imgui/imgui.h"
#include "WindowClass.h"

bool Window::Initialize(HINSTANCE hInstance,const std::string& window_title, const std::string& window_class, const int width, const int height)
{
	return this->render_window.Initialize(this, hInstance, window_title, window_class, width, height);
}

bool Window::ProcessMessages()
{
	return this->render_window.ProcessMessages();
}

void Window::Update()
{
	//nothing here yet

	//read mouse
}

void Window::HideCoursor()
{
	confineCoursor();
	while (::ShowCursor(FALSE) >= 0);
}

void Window::ShowCoursor()
{
	ClipCursor(nullptr);
	while (::ShowCursor(TRUE) < 0);
}

const uint32_t Window::WindowHeight() const
{
	return this->windowHeight;
}

const uint32_t Window::WindowWidth() const
{
	return this->windowWidth;
}

void Window::confineCoursor()
{
	RECT rect;
	GetClientRect(this->getRenderWindow().getHandle(), &rect);
	MapWindowPoints(this->getRenderWindow().getHandle(), nullptr, reinterpret_cast<POINT*>(&rect), 2);
	ClipCursor(&rect);
}