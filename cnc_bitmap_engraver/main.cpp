#include <windows.h>
#include <atlimage.h>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>

std::string& operator += (std::string& str, const float &value){
	char number[30];
	sprintf(number, "%.3f", value);
	str += number;
	return str;
}

std::string operator + (const float &value, const std::string &text){
	std::string t;
	t += value;
	t += text;
	return t;
}

std::string operator + (const std::string &text, const float &value){
	std::string t;
	t += value;
	t += text;
	return t;
}

class Point3D{
public:
	float x;
	float y;
	float z;

	inline Point3D() : x(0), y(0), z(0){}
	inline Point3D(float x, float y, float z) : x(x), y(y), z(z){}

	std::string toString(){
		std::string point3d = "x";
		point3d += x;
		point3d += " y";
		point3d += y;
		point3d += " z";
		point3d += z;
		point3d += "\n";
		return point3d;
	}
};

HWND hWnd;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	static ATL::CImage img;

	static int x0 = 0;
	static int y0 = 0;
	static int z0 = 0;

	switch(msg){
		case WM_CREATE:
			{
				img.Load("bitmap.jpg");

				if(!img.IsNull() && img.GetBPP() == 24){
					
					std::fstream file("file.ngc", std::ios_base::out);

					BYTE *byte = (BYTE*)img.GetBits();

					int width = img.GetWidth();
					int height = img.GetHeight();

					float z = 0.;
					float max_deep = 1.;
					int pitch = img.GetPitch();

					file << "G1 M3 S800 F300 G90" << std::endl << std::endl;

					std::vector<Point3D> points;
					

					for(int y = 0; y < height; y += 2){
						for(int x = 0; x < width; x++){
							z = (float)byte[x * 3 + y * pitch] / 255. * -max_deep;
							if(x){
								if(x > 1 && (points.end() - 2)->z == points.back().z && points.back().z == z){
									points.back().x = x;
								}else{
									points.push_back(Point3D(x, y, z));
								}
							}else{
								points.push_back(Point3D(x, y, 0));
								points.push_back(Point3D(x, y, z));
							}
							
						}
						points.push_back(Point3D(points.back().x, points.back().y, 0));
						if(y + 1 < height){
							points.push_back(Point3D(points.back().x, y + 1, 0));
							for(int x = width - 1; x > -1; x--){
								z = (float)byte[x * 3 + y * pitch] / 255. * -max_deep;
								if(x < width - 2 && (points.end() - 2)->z == points.back().z && points.back().z == z){
									points.back().x = x;
								}else{
									points.push_back(Point3D(x, y + 1, z));
								}
							}
							points.push_back(Point3D(points.back().x, points.back().y, 0));
						}
					}

					/*for(int i = 1; i < points.size() - 1; i++){
						while(i < points.size() - 1 && points[i - 1].z == points[i].z && points[i + 1].z == points[i].z){
							points.erase(points.begin() + i);
						}
					}*/

					for(std::vector<Point3D>::iterator i = points.begin(); i < points.end(); i++){
						file << i->toString();
					}

					file << "M30";

					file.close();
				}

			}
			break;
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);

				if(!img.IsNull()){
					img.BitBlt(hdc, 0, 0, SRCCOPY);
				}

				EndPaint(hWnd, &ps);
			}
			break;
		case WM_DESTROY:
			{
				PostQuitMessage(0);
			}
			break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int  WinMain(HINSTANCE hInstance, HINSTANCE , LPSTR lpCmdLine, int nShowCmd){
	
	char wndName[] = "EngravingWindow";

	WNDCLASS wnd;

	wnd.cbClsExtra = NULL;
	wnd.cbWndExtra = NULL;
	wnd.hbrBackground = NULL;
	wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
	wnd.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wnd.hInstance = hInstance;
	wnd.lpfnWndProc = WndProc;
	wnd.lpszClassName = wndName;
	wnd.lpszMenuName = NULL;
	wnd.style = CS_VREDRAW|CS_HREDRAW;

	if(!RegisterClass(&wnd)){
		MessageBox(NULL, "Something wrong with WNDCLASS", "ERROR", MB_OK);
		return 0;
	}

	hWnd = CreateWindow(wndName, "Engraving bitmap G-code generator", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, SW_NORMAL);
	UpdateWindow(hWnd);

	MSG msg;

	while(GetMessage(&msg, NULL, 0, 0)){
		DispatchMessage(&msg);
		TranslateMessage(&msg);
	};
	
	return 1;
}