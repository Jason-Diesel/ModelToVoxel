#pragma once
#include "WindowContainer.h"

class Window : public WindowContainer
{
public:
	bool Initialize(HINSTANCE hInstance, const std::string& window_title, const std::string& window_class, const int width, const int height);
	bool ProcessMessages();
	void Update();
	void HideCoursor();
	void ShowCoursor();
	const uint32_t WindowHeight() const;
	const uint32_t WindowWidth() const;
private:
	void confineCoursor();
	uint32_t windowWidth = 1400;
	uint32_t windowHeight = 960;
};