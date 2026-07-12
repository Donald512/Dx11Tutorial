
#include "core.h"

void UpdateAndRender(float dt) {
	
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ){

	const auto pClassName = "hw3dbutts";
	// register window class
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof( wc );
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = (HICON) (LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 32, 32, 0));
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = pClassName;
	wc.hIconSm = (HICON) (LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, 0));
	RegisterClassEx( &wc );
	// create window instance
	HWND hWnd = CreateWindowEx( 0, pClassName, "Happy Hard Window", WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, 200,200,640,480, nullptr,nullptr,hInstance,nullptr);
	CHECK(hWnd != nullptr, "Failed to create window");

	ShowWindow( hWnd,SW_SHOW );

	CHECK(Graphics::Initialize(hWnd), "Failed to initialize D3d11");

	EngineTimer timer;
	float totalTime = 0.0f;

	MSG msg;
	while (true){
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
    	}

		if (msg.message == WM_QUIT) break;

		float dt = timer.Mark();
		totalTime += dt;
		
		const float c = sin(totalTime) / 2.0f + 0.5f;

		Graphics::ClearBuffer(c, c, 1.0f);

		UpdateAndRender(dt);
		Graphics::EndFrame();
	}
	
	Graphics::Shutdown();
	UnregisterClass(pClassName, hInstance);
	DestroyWindow(hWnd);


    return msg.wParam;
}

LRESULT CALLBACK WndProc( HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam ){
	std::string message = FormatWindowsMessage(msg, wParam, lParam);
	OutputDebugString( message.c_str() );

	switch( msg ){
	case WM_CLOSE:{
        PostQuitMessage( 69 );
		break;
    }
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:{
		// filter out autorepeat
		if (!(lParam & 0x40000000)) { 
        	Keyboard::OnKeyPressed(static_cast<unsigned char>(wParam));
    	}
		break;
    }
	case WM_KEYUP:
	case WM_SYSKEYUP:{
		Keyboard::OnKeyReleased(static_cast<unsigned char>(wParam));
		break;
    }
	case WM_CHAR:{
		Keyboard::OnChar(static_cast<char>(wParam));
		break;
    }
	case WM_MOUSEMOVE:{
		const POINTS pt = MAKEPOINTS(lParam);
		Mouse::OnMouseMove(pt.x, pt.y);
		break;
	}
	case WM_LBUTTONDOWN:{
		const POINTS pt = MAKEPOINTS(lParam);
		Mouse::OnLeftPressed(pt.x, pt.y);
		break;
	}
	case WM_RBUTTONDOWN:{
		const POINTS pt = MAKEPOINTS(lParam);
		Mouse::OnRightPressed(pt.x, pt.y);
		break;
	}
	case WM_LBUTTONUP:{
		const POINTS pt = MAKEPOINTS(lParam);
		Mouse::OnLeftReleased(pt.x, pt.y);
		break;
	}
	case WM_RBUTTONUP:{
		const POINTS pt = MAKEPOINTS(lParam);
		Mouse::OnRightReleased(pt.x, pt.y);
		break;
	}
	case WM_MOUSEWHEEL:{
		const POINTS pt = MAKEPOINTS(lParam);
		if (GET_WHEEL_DELTA_WPARAM(wParam) > 0) {
			Mouse::OnWheelUp(pt.x, pt.y);
		} 
		else if (GET_WHEEL_DELTA_WPARAM(wParam) < 0) {
			Mouse::OnWheelDown(pt.x, pt.y);
		}
		break;
	}
	}

	return DefWindowProc( hWnd,msg,wParam,lParam );
}

inline void Check(bool expr, const char* message, const char* file, int line){
	if (!expr){
		std::cout << message << "\n\nFile: " << file << "\nLine: " << line;
		PostQuitMessage(-1);
	}
}




namespace Keyboard {
    // Hidden internal state
    constexpr unsigned int nKeys = 256u;
    constexpr unsigned int bufferSize = 16u;

    static std::bitset<nKeys> keystates;
    static std::queue<Event> keybuffer;
    static std::queue<char> charbuffer;

    // Helper to keep buffers from growing infinitely
    template<typename T>
    static void TrimBuffer(std::queue<T>& buffer) {
        while (buffer.size() > bufferSize) {
            buffer.pop();
        }
    }

    bool IsKeyPressed(unsigned char keycode) {
        return keystates[keycode];
    }

    Event ReadKey() {
        if (!keybuffer.empty()) {
            Event e = keybuffer.front();
            keybuffer.pop();
            return e;
        }
        return Event{};
    }

    bool KeyIsEmpty() { return keybuffer.empty(); }
    void FlushKey() { keybuffer = std::queue<Event>(); }

    char ReadChar() {
        if (!charbuffer.empty()) {
            char charcode = charbuffer.front();
            charbuffer.pop();
            return charcode;
        }
        return 0;
    }

    bool CharIsEmpty() { return charbuffer.empty(); }
    void FlushChar() { charbuffer = std::queue<char>(); }
    
    void FlushAll() {
        FlushKey();
        FlushChar();
    }

    // These get called inside your flat Window Stuff WndProc
    void OnKeyPressed(unsigned char keycode) {
        keystates[keycode] = true;
        keybuffer.push(Event{ Event::Type::Press, keycode });
        TrimBuffer(keybuffer);
    }

    void OnKeyReleased(unsigned char keycode) {
        keystates[keycode] = false;
        keybuffer.push(Event{ Event::Type::Release, keycode });
        TrimBuffer(keybuffer);
    }

    void OnChar(char character) {
        charbuffer.push(character);
        TrimBuffer(charbuffer);
    }

    void ClearState() {
        keystates.reset();
    }
}

namespace Mouse {

    constexpr unsigned int bufferSize = 16u;

    // File-scoped state variables
    static int mouseX = 0;
    static int mouseY = 0;
    static bool bLeftPressed = false;
    static bool bRightPressed = false;
    static std::queue<Event> eventBuffer;

    static void TrimBuffer() {
        while (eventBuffer.size() > bufferSize) {
            eventBuffer.pop();
        }
    }

    static Event CreateEvent(Event::Type type) {
        return Event{ type, bLeftPressed, bRightPressed, mouseX, mouseY };
    }

    int GetX() { return mouseX; }
    int GetY() { return mouseY; }
    bool LeftIsPressed() { return bLeftPressed; }
    bool RightIsPressed() { return bRightPressed; }

    Event Read() {
        if (!eventBuffer.empty()) {
            Event e = eventBuffer.front();
            eventBuffer.pop();
            return e;
        }
        return Event{};
    }

    bool IsEmpty() { return eventBuffer.empty(); }
    void Flush() { eventBuffer = std::queue<Event>(); }

    void OnMouseMove(int x, int y) {
        mouseX = x; mouseY = y;
        eventBuffer.push(CreateEvent(Event::Type::Move));
        TrimBuffer();
    }

    void OnLeftPressed(int x, int y) {
        mouseX = x; mouseY = y; bLeftPressed = true;
        eventBuffer.push(CreateEvent(Event::Type::LPress));
        TrimBuffer();
    }

    void OnLeftReleased(int x, int y) {
        mouseX = x; mouseY = y; bLeftPressed = false;
        eventBuffer.push(CreateEvent(Event::Type::LRelease));
        TrimBuffer();
    }

    void OnRightPressed(int x, int y) {
        mouseX = x; mouseY = y; bRightPressed = true;
        eventBuffer.push(CreateEvent(Event::Type::RPress));
        TrimBuffer();
    }

    void OnRightReleased(int x, int y) {
        mouseX = x; mouseY = y; bRightPressed = false;
        eventBuffer.push(CreateEvent(Event::Type::RRelease));
        TrimBuffer();
    }

    void OnWheelUp(int x, int y) {
        mouseX = x; mouseY = y;
        eventBuffer.push(CreateEvent(Event::Type::WheelUp));
        TrimBuffer();
    }

    void OnWheelDown(int x, int y) {
        mouseX = x; mouseY = y;
        eventBuffer.push(CreateEvent(Event::Type::WheelDown));
        TrimBuffer();
    }
}

