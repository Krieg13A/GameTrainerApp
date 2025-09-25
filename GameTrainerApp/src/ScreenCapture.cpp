#include <windows.h>
#include <iostream>
#include <fstream>
#include "ScreenCapture.h"

bool CaptureScreenToBMP(const char* filename) {
    int screenX = GetSystemMetrics(SM_CXSCREEN);
    int screenY = GetSystemMetrics(SM_CYSCREEN);

    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenX, screenY);
    SelectObject(hMemoryDC, hBitmap);

    BitBlt(hMemoryDC, 0, 0, screenX, screenY, hScreenDC, 0, 0, SRCCOPY);

    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = bmp.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = bmp.bmWidth * bmp.bmHeight * 4;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    DWORD dwSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bi.biSizeImage;
    BYTE* lpBitmapData = new BYTE[bi.biSizeImage];

    GetDIBits(hMemoryDC, hBitmap, 0, bmp.bmHeight, lpBitmapData, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file) {
        std::cerr << "Error al crear el archivo BMP." << std::endl;
        return false;
    }

    bmfHeader.bfType = 0x4D42;
    bmfHeader.bfSize = dwSize;
    bmfHeader.bfReserved1 = 0;
    bmfHeader.bfReserved2 = 0;
    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    file.write((char*)&bmfHeader, sizeof(BITMAPFILEHEADER));
    file.write((char*)&bi, sizeof(BITMAPINFOHEADER));
    file.write((char*)lpBitmapData, bi.biSizeImage);
    file.close();

    ReleaseDC(NULL, hScreenDC);
    DeleteDC(hMemoryDC);
    DeleteObject(hBitmap);
    delete[] lpBitmapData;

    std::cout << "Captura guardada en: " << filename << std::endl;
    return true;
}