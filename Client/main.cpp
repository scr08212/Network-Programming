#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WIN32_WINNT 0X0601

#pragma once
#include <WinSock2.h>
#include <string>
#include <windowsx.h>
#include <atlconv.h>
#include "Client.h"
#include "resource2.h"

#pragma comment(lib, "ws2_32")


Client client;
COLORREF color;
POINT prevPoint;
bool isCanvasInitialized;

string getStringFromEditText(HWND hDlg, int id)
{
    string text = "";
    // UTF-16 ���ڿ��� ���� ����� ũ���� ���� ����
    wchar_t addrW[1024] = { 0 }; // �ִ� 512���� UTF-16 ���ڸ� ���� �� ����.
    GetDlgItemTextW(hDlg, id, addrW, sizeof(addrW) / sizeof(wchar_t));
    wstring wstr = wstring(addrW);

    USES_CONVERSION;
    text = string(W2A(wstr.c_str()));

    return text;
}

void clearCanvas(HWND hDlg)
{
    HWND hCanvas = GetDlgItem(hDlg, IDC_CANVAS);
    HDC hdc = GetDC(hCanvas);

    // ĵ���� ���� ��������
    RECT rect;
    GetClientRect(hCanvas, &rect);

    rect.top += 1;
    rect.bottom -= 1;
    rect.left += 1;
    rect.right -= 1;

    // ������� ĵ������ ä��
    HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdc, &rect, hBrush);

    DeleteObject(hBrush);
    ReleaseDC(hCanvas, hdc);
}

// DlgProc���� WM_COMMAND �κи� ���� ��
INT_PTR CommandProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    bool isIPv6 = false;
    string msg;

    switch (LOWORD(wParam))
    {
    case IDC_CONNECT:
        isIPv6 = IsDlgButtonChecked(hDlg, IDC_RADIO_IPV6);
        client.setDlgHandle(hDlg);
        client.connectToServer(getStringFromEditText(hDlg, IDC_EDIT_ADDRESS), 9000, isIPv6);
        
        return TRUE;
    case IDC_SEND_MESSAGE:
        msg = getStringFromEditText(hDlg, IDC_EDIT_MESSAGE);
        if (msg == "")
            return TRUE;
        client.sendMessage(msg);
        SetDlgItemText(hDlg, IDC_EDIT_MESSAGE, L"");
        return TRUE;
    case IDC_SEND_FILE:
    {
        msg = getStringFromEditText(hDlg, IDC_EDIT_MESSAGE); // read file path
        client.sendFile(msg);
        return TRUE;
    }
    case IDC_RADIO_IPV6:
        if (!IsDlgButtonChecked(hDlg, IDC_RADIO_IPV6))
            CheckDlgButton(hDlg, IDC_RADIO_IPV6, BST_CHECKED);
        else
            CheckDlgButton(hDlg, IDC_RADIO_IPV6, BST_UNCHECKED);
        return TRUE;
    case IDC_RADIO_UDP:
        if (!IsDlgButtonChecked(hDlg, IDC_RADIO_UDP))
            CheckDlgButton(hDlg, IDC_RADIO_UDP, BST_CHECKED);
        else
            CheckDlgButton(hDlg, IDC_RADIO_UDP, BST_UNCHECKED);
        return TRUE;
    case IDC_COLOR_RED:
        color = RGB(255, 0, 0);
        CheckDlgButton(hDlg, IDC_COLOR_RED, BST_CHECKED);
        CheckDlgButton(hDlg, IDC_COLOR_GREEN, BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_COLOR_BLUE, BST_UNCHECKED);
        return TRUE;
    case IDC_COLOR_GREEN:
        color = RGB(0, 255, 0);
        CheckDlgButton(hDlg, IDC_COLOR_RED, BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_COLOR_GREEN, BST_CHECKED);
        CheckDlgButton(hDlg, IDC_COLOR_BLUE, BST_UNCHECKED);
        return TRUE;
    case IDC_COLOR_BLUE:
        color = RGB(0, 0, 255);
        CheckDlgButton(hDlg, IDC_COLOR_RED, BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_COLOR_GREEN, BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_COLOR_BLUE, BST_CHECKED);
        return TRUE;
    case IDC_CLEAR_CANVAS:
        if (!client.stopFlag)
        {
            clearCanvas(hDlg);
            client.sendClearCanvas();
        }
        return TRUE;
    case IDCANCEL:
        client.disconnect();
        EndDialog(hDlg, IDCANCEL);
        return TRUE;
    }

    return FALSE;
}

void DrawLine(HDC hdc, COLORREF colorRef, POINT from, POINT to)
{
    HPEN hPen = CreatePen(PS_SOLID, 1, colorRef);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen); 

    MoveToEx(hdc, from.x, from.y, NULL);
    LineTo(hdc, to.x, to.y);

    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hCanvas = GetDlgItem(hDlg, IDC_CANVAS);
    RECT rect;
    POINT curPoint;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        color = RGB(255, 0, 0);  // �⺻ �� ��: ������
        CheckDlgButton(hDlg, IDC_COLOR_RED, BST_CHECKED);
        EnableWindow(GetDlgItem(hDlg, IDC_SEND_FILE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_SEND_MESSAGE), FALSE);

        // �̺�Ʈ �ݹ� ����
        client.onDrawingReceived = [](HWND hDlg, COLORREF colorRef, POINT from, POINT to)
            {
                HWND hCanvas = GetDlgItem(hDlg, IDC_CANVAS);
                HDC hdc = GetDC(hCanvas);
                DrawLine(hdc, colorRef, from, to);
                ReleaseDC(hCanvas, hdc);
            };
        client.onClearCanvasReceived = [](HWND hDlg)
            {
                clearCanvas(hDlg);
            };

        return TRUE;
    case WM_CTLCOLORSTATIC:
        if (!isCanvasInitialized)
        {
            clearCanvas(hDlg);
            isCanvasInitialized = true;
        }
        return TRUE;

    case WM_COMMAND:
        return CommandProc(hDlg, uMsg, wParam, lParam);

    case WM_MOUSEMOVE:
        if (client.stopFlag)
            break;
        GetCursorPos(&curPoint);
        GetClientRect(hCanvas, &rect);
        ScreenToClient(hCanvas, &curPoint);

        HDC hdc = GetDC(hCanvas);

        if (wParam == MK_LBUTTON)
        {
            if (curPoint.x >= 1 && curPoint.x <= rect.right - 1 && curPoint.y >= 1 && curPoint.y <= rect.bottom - 1)
            {
                prevPoint.x = clamp(prevPoint.x, 1L, rect.right - 1);
                prevPoint.y = clamp(prevPoint.y, 1L, rect.bottom - 1);

                HPEN newPen = CreatePen(PS_SOLID, 1, color);
                HPEN oldPen = (HPEN)SelectObject(hdc, newPen);

                MoveToEx(hdc, prevPoint.x, prevPoint.y, NULL);
                LineTo(hdc, curPoint.x, curPoint.y);
                SelectObject(hdc, oldPen);
                DeleteObject(newPen);

                client.sendDrawing(color, prevPoint, curPoint);
            }
        }

        prevPoint = curPoint;
        ReleaseDC(hCanvas, hdc);
    }

    return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    isCanvasInitialized = false;
    
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_CLIENT), NULL, DlgProc);
}