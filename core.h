
#ifdef UNICODE
#undef UNICODE
#endif

#ifdef _UNICODE
#undef _UNICODE
#endif

#include "WinFramework.h"
#include <string>
#include <sstream>
#include <iostream>
#include <bitset>
#include <queue>
#include <chrono>
#include <d3d11.h>
#include "resource.h"
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "d3d11.lib")

LRESULT CALLBACK WndProc( HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam );
std::string FormatWindowsMessage(DWORD msg, LPARAM lp, WPARAM wp);
#define CHECK(expr, msg) Check(expr, msg, __FILE__, __LINE__);
inline void Check(bool expr, const char* message, const char* file, int line);

struct EngineTimer {
    std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();

    float Mark() {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> deltaTime = now - lastTime;
        lastTime = now;
        return deltaTime.count(); // Returns elapsed seconds as a float
    }

    float Peek(){
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> deltaTime = now - lastTime;
        return deltaTime.count(); // Returns elapsed seconds as a float
    }
};


namespace Graphics{ 
    bool Initialize(HWND hWnd);
    void Shutdown();
    void EndFrame();
    void ClearBuffer(float red, float green, float blue);

}

namespace Keyboard {
    // 1. Immediate State: Just a plain array or bitset of 256 virtual keys
    bool IsKeyPressed(unsigned char keycode);

    // 2. Simple Event Queue (replaces the heavy class-based ring buffers)
    struct Event {
        enum class Type { Press, Release, Invalid };
        Type type = Type::Invalid;
        unsigned char code = 0;
    };

    Event ReadKey();
    bool KeyIsEmpty();
    void FlushKey();

    // 3. Text Input Buffer
    char ReadChar();
    bool CharIsEmpty();
    void FlushChar();
    void FlushAll();

    // Internal functions your Window Procedure calls to feed the state
    void OnKeyPressed(unsigned char keycode);
    void OnKeyReleased(unsigned char keycode);
    void OnChar(char character);
    void ClearState();
}

namespace Mouse {
    struct Event {
        enum class Type { LPress, LRelease, RPress, RRelease, WheelUp, WheelDown, Move, Invalid };
        Type type = Type::Invalid;
        bool leftIsPressed = false;
        bool rightIsPressed = false;
        int x = 0;
        int y = 0;
    };

    // Immediate tracking queries
    int GetX();
    int GetY();
    bool LeftIsPressed();
    bool RightIsPressed();

    // Event Buffer queries
    Event Read();
    bool IsEmpty();
    void Flush();

    // Flat interface for WndProc to pump data in
    void OnMouseMove(int x, int y);
    void OnLeftPressed(int x, int y);
    void OnLeftReleased(int x, int y);
    void OnRightPressed(int x, int y);
    void OnRightReleased(int x, int y);
    void OnWheelUp(int x, int y);
    void OnWheelDown(int x, int y);
}