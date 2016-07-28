#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>

#define BLOCK_WIDTH 2
#define BLOCK_HEIGHT 2

#define APPNAME "24BPP_ASCII"

char szAppName[] = APPNAME; // The name of this application
char szTitle[]   = APPNAME; // The title bar text
char *pWindowText;

HINSTANCE g_hInst;          // current instance

long width, height;
BITMAPFILEHEADER bitmapFileHeader;
BITMAPINFOHEADER bitmapInfoHeader;
char *filecontents;
int segments, offset, y;

char table[8] = { ' ', '.', ':', 'o', 'O', '8', '#', '@' };

char *LoadFile(char *pszFileName, long *lgSize)
{
   	FILE *TARGET;
	char *mem;
	char *ptr2;
	long h = 0;

	fopen_s(&TARGET, pszFileName, "rb");

	if(!TARGET)
	{
		printf("LoadFile(): Unable to open file specified. %s\n", pszFileName);
		*lgSize = 0;
		return 0;
	}

	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER),1,TARGET);
	fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER),1,TARGET);

	if(bitmapInfoHeader.biBitCount != 24){
	    printf("LoadFile(): Not a 24-bit BMP File!\n");
	    *lgSize = 0;
	    return 0;
	}

	fseek(TARGET, bitmapFileHeader.bfOffBits, SEEK_SET);
	h = (long) bitmapFileHeader.bfSize-bitmapFileHeader.bfOffBits;

	mem = (char*) malloc(h+1);
	ptr2 = &mem[0];
	if(mem == NULL)
	{
		printf("LoadFile(): Could Not Allocate Memory.\n");
		*lgSize = 0;
		return 0;
	}
	fread(mem,sizeof(BYTE),h,TARGET);
	*lgSize = h;
	width = bitmapInfoHeader.biWidth;
	height = bitmapInfoHeader.biHeight;
	fclose(TARGET);
	return mem;
}

double CalcLuma(char red, char green, char blue)
{
	double nits;
	nits = (.27 * (BYTE)red) + (.67 * (BYTE)green) + (.06 * (BYTE)blue);
	return nits;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int u, localY, o, block;
	switch (message)
	{
		case WM_CREATE:
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_RBUTTONUP:
			DestroyWindow(hwnd);
			break;
		case WM_KEYDOWN:
			if (VK_ESCAPE == wParam)
				DestroyWindow(hwnd);
			break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC         hdc;
			RECT        rc;
			HFONT hFont, hTmp;
			hdc = BeginPaint(hwnd, &ps);

			hFont = CreateFont(8,4,0,0,FW_DONTCARE,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,TEXT("Small Fonts"));
            SelectObject(hdc, hFont);
			GetClientRect(hwnd, &rc);			
			SetBkMode(hdc, TRANSPARENT);

			for(u = segments; u > -1; u--)
				for(o = 0; o < y; o += (3 * BLOCK_WIDTH)){
        			int iterator = 0, r = 0, g = 0, b = 0;
					double luma = 0;
					char out;
        			for(localY = u * BLOCK_HEIGHT; localY < (u + 1) * BLOCK_HEIGHT; localY++)
						for(block = o; block < o + (3 * BLOCK_WIDTH); block++){
							luma += CalcLuma(
								filecontents[(localY*y) + o + 2 + (localY*offset)] & 0x00FF,
								filecontents[(localY*y) + o + 1 + (localY*offset)] & 0x00FF,
								filecontents[(localY*y) + o     + (localY*offset)] & 0x00FF
							);
							r += filecontents[(localY*y) + o + 2 + (localY*offset)] & 0x00FF;
							g += filecontents[(localY*y) + o + 1 + (localY*offset)] & 0x00FF;
							b += filecontents[(localY*y) + o     + (localY*offset)] & 0x00FF;
							iterator++;
        				}
					luma /= iterator; r /= iterator; g /= iterator; b /= iterator;
					out = table[(int)floor(luma / 32)];
					SetTextColor(hdc, RGB(r,g,b));					
					TextOut(hdc, ((o/(3*BLOCK_WIDTH))*7), (segments-u)*7, &out, 1);
				}
			DeleteObject(hFont);
			EndPaint(hwnd, &ps);
			break;
		}
		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

int main(int argc, char **argv)
{
	long w, h, filesize;
	MSG msg;
	WNDCLASS wc;
	HWND hwnd;
	
	offset = 0;

	if (argc != 2)
	{
		printf("usage: file.bmp\n");
		return -1;
	}

	filecontents = LoadFile(argv[1], &filesize);
    if(filecontents == 0)
        return -1;
	
	w = width;
	h = height;
	if((w*3) % 4 != 0)
		offset = (4-((w*3)%4));
	y = (w * 3); // Width of Row (minus Offset)
	segments = ((int)(h / BLOCK_HEIGHT)) - 1;

	ZeroMemory(&wc, sizeof wc);
	wc.hInstance     = GetModuleHandle(0);
	wc.lpszClassName = szAppName;
	wc.lpfnWndProc   = (WNDPROC)WndProc;
	wc.style         = CS_DBLCLKS|CS_VREDRAW|CS_HREDRAW;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	
	if (FALSE == RegisterClass(&wc)){
		printf("Could not register window class\n");
		return -1;
	}

	hwnd = CreateWindow(
		szAppName,
		szTitle,
		WS_OVERLAPPEDWINDOW|WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		800,
		600,
		0,
		0,
		g_hInst,
		0);

	if (NULL == hwnd){
		printf("Could not create window\n");
		return -1;
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	free(filecontents);
	return msg.wParam;
}