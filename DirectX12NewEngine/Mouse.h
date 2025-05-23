#pragma once
#include <Windows.h>
#include <queue>
#include <iostream>

struct MousePoint
{
	int x, y;
};

//made from Jpres https://www.youtube.com/channel/UC5Lxe7GAsk_f8qMBsNmlOJg
class mouseEvent {
public:
	enum EventType
	{
		LPress,
		LRelease,
		RPress,
		RRelease,
		MPress,
		MRelease,
		WheelUp,
		WheelDown,
		Move,
		RAW_MOVE,
		INVALID
	};
private:
	EventType type;
	int x;
	int y;

public:
	mouseEvent();
	mouseEvent(const EventType type, const int x, const int y);
	bool IsValid();
	EventType getType() const;
	MousePoint getPos() const;
	int getPosX() const;
	int getPosY() const;
};

class Mouse {
public:
	Mouse();
	virtual ~Mouse();
	void update();

	bool IsLeftDown();
	bool isMiddleDown();
	bool isRightDown();

	bool isLeftPressed();
	bool isMiddlePressed();
	bool isRightPressed();

	MousePoint getPos();
	MousePoint getDeltaPos();
	int getPosX();
	int getPosY();
	bool EventBufferEmpty();
	mouseEvent ReadEvent();

	void changeSense(float newSence);
	float getSense();
	bool getMouseActive();
	void onMouseMoveRaw(int x, int y);
	void onLeftPressed(int x, int y);
	void onLeftReleased(int x, int y);
	void onRightPressed(int x, int y);
	void onRightReleased(int x, int y);
	void onMiddlePressed(int x, int y);
	void onMiddleReleased(int x, int y);
	void onWheelUp(int x, int y);
	void onWheelDown(int x, int y);
	void onMouseMove(int x, int y);
	void activateMouse(bool activate);
	void clear();
	void clearEventBuffer(); //Do this before game start...	
	void set_captureEvent(bool status);
private:
	bool mouse_active;
	bool once;
	int x, y;
	int LastX, LastY;
	int DeltaX, DeltaY;
	bool leaftIsDown, rightIsDown, midIsDown;
	bool leaftDown, rightDown, midDown;
	float mouseSense;
	bool captureEvent = true;
	std::queue<mouseEvent>mouseBuffer;
};