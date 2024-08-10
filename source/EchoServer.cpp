#include <cstdio>
#include "..\..\[Libraries]\String Extension\stringExt.h"
#include "..\..\[Libraries]\WinSocket\Socket.h"
#include "..\..\[Libraries]\WinGUI\WinGUI.h"
#include "EchoServer.h"

using namespace std;


bool ServerSetup(void)
//Set up program resources
{
    WNDCLASSEX wndClassInfo;
    HINSTANCE hThisInstance = (HINSTANCE) GetModuleHandle(NULL);
    int wndWidth = int(GetSystemMetrics(SM_CXSCREEN) / 1.9);
    int wndHeight = int(GetSystemMetrics(SM_CYSCREEN) / 1.6);
    int wndLeft = (GetSystemMetrics(SM_CXSCREEN) - wndWidth) / 2;
    int wndTop = (GetSystemMetrics(SM_CYSCREEN) - wndHeight) / 2;
    
    //Setup socket support
    if(!SocketSetup())
        return false;
    
    //Setup GUI support
    if(!WinGUISetup())
        return false;
    
    //Create socket
    serverSocketPtr = new CSocket;
    
    //Set socket events
    serverSocketPtr->AddEvent(FD_ACCEPT, Socket_OnConnectionRequest);
    serverSocketPtr->AddEvent(FD_CLOSE, Socket_OnDisconnect);
    serverSocketPtr->AddEvent(FD_READ, Socket_OnDataArrival);
    serverSocketPtr->AddEvent(FD_ERROR, Socket_OnError);
    
    //Create window and controls
    serverWin.Create("Echo Server", wndLeft, wndTop, wndWidth, wndHeight, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN);
    portLabel.Create(&serverWin, "Port:", 15, 15, 35, 20, WS_VISIBLE | WS_CHILD);
    portEdit.Create(&serverWin, "", 50, 15, 70, 20, WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, WS_EX_CLIENTEDGE);
    listenBtn.Create(&serverWin, "Listen", 130, 10, 85, 30, WS_VISIBLE | WS_CHILD);
    disconnectBtn.Create(&serverWin, "Disconnect", 220, 10, 85, 30, WS_VISIBLE | WS_CHILD | WS_DISABLED);    
    statusLabel.Create(&serverWin, "Status:", 345, 15, 50, 20, WS_VISIBLE | WS_CHILD);
    statusBox.Create(&serverWin, "Disconnected", 400, 15, 120, 20, WS_VISIBLE | WS_CHILD, WS_EX_STATICEDGE);
    clearBtn.Create(&serverWin, "Clear Display", 560, 10, 100, 30, WS_VISIBLE|WS_CHILD);
    dispBox.Create(&serverWin, "", 0, 50, 0, 0,
                              WS_VISIBLE|WS_CHILD|WS_BORDER|
                              WS_VSCROLL|WS_HSCROLL|ES_READONLY|
                              ES_MULTILINE|ES_WANTRETURN|
                              ES_AUTOHSCROLL|ES_AUTOVSCROLL);
    
    //Set window events
    serverWin.AddEvent(WM_CLOSE, ServerWin_OnClose, WINEVENT_MESSAGE);
    serverWin.AddEvent(WM_SIZE, ServerWin_OnSize, WINEVENT_MESSAGE);
    serverWin.AddEvent(WM_CTLCOLORSTATIC, ServerWin_OnCtlColorStatic, WINEVENT_MESSAGE);
    listenBtn.AddEvent(BN_CLICKED, ListenBtn_OnClick, WINEVENT_COMMAND);
    disconnectBtn.AddEvent(BN_CLICKED, DisconnectBtn_OnClick, WINEVENT_COMMAND);
    clearBtn.AddEvent(BN_CLICKED, ClearBtn_OnClick, WINEVENT_COMMAND);
    
    //Set display colors
    dispBkColor = RGB(0,0,90);
    dispTextColor = RGB(255,255,255);
    dispBkColorBrush = CreateSolidBrush(dispBkColor);
    
    return true;
}

void ServerCleanup(void)
//Clean up program resources
{
    //Close the main window
    serverWin.Destroy();
    
    //Close socket
    delete serverSocketPtr;
    
    //Cleanup GUI support
    WinGUICleanup();
    
    //Cleanup socket support
    SocketCleanup();
    
    //Cleanup display color brush
    DeleteObject(dispBkColorBrush);
}

int WINAPI WinMain (HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
//Program starting point
{
    WPARAM retVal;
    
    if(!ServerSetup())
        return 0;
    
    //Display the main window
    serverWin.Show();
    
    //Run message loop
    retVal = InputWinEvents();
    
    ServerCleanup();
    
    return retVal;
}

LRESULT ServerWin_OnClose(CWindow* winPtr, const CWinEvent& eventObj)
//End the program
{
    PostQuitMessage(0);
    return 0;
}

LRESULT ServerWin_OnSize(CWindow* winPtr, const CWinEvent& eventObj)
//Resize controls with the main window
{
    int newWidth = LOWORD(eventObj.lParam);
    int newHeight = HIWORD(eventObj.lParam);
    
    dispBox.SetSize(newWidth, newHeight - 50);
    
    return 0;
}

LRESULT ServerWin_OnCtlColorStatic(CWindow* winPtr, const CWinEvent& eventObj)
//
{
    HDC hdc = (HDC) eventObj.wParam;
    HWND hwnd = (HWND) eventObj.lParam;
    
    if(hwnd == dispBox.winHandle)
    {
    	SetBkColor(hdc, dispBkColor);
	    SetTextColor(hdc, dispTextColor);
        return (LRESULT)dispBkColorBrush;
    }
    else
    {
    	return DefWindowProc(winPtr->winHandle, WM_CTLCOLORSTATIC, eventObj.wParam, eventObj.lParam);
    }
}

LRESULT ListenBtn_OnClick(CWindow* btnPtr, const CWinEvent& eventObj)
//Listen for a connection on the given port
{
	int portNum;
    string portStr;
    
    //Get the port number
    portStr = portEdit.GetText();
    portNum = StrToNum(portStr);
    
    //Listen for a connection on socket
    if(!(serverSocketPtr->Listen(portNum)))
    {
	    dispBox.AppendText("Failed to listen. \r\n\r\n");
        return 0;
	}
    
    UpdateUI();
    return 0;
}

LRESULT DisconnectBtn_OnClick(CWindow* btnPtr, const CWinEvent& eventObj)
//End the current connection
{
    
    if(!(serverSocketPtr->Disconnect()))
    {
	    dispBox.AppendText("Failed to disconnect. \r\n\r\n");
        return 0;
	}
    
    UpdateUI();
    return 0;
}

LRESULT ClearBtn_OnClick(CWindow* winPtr, const CWinEvent& eventObj)
//
{
    dispBox.SetText("");
}

void Socket_OnConnectionRequest(CSocket* socketPtr, const CSocketEvent& eventObj)
//Connects socket by accepting the connection request
{
    dispBox.AppendText("Socket received connection request. \r\n\r\n");
    
    //Accpet connection request
    dispBox.AppendText("Accepting connection... \r\n\r\n");
    if(!(socketPtr->Accept()))
    {
        dispBox.AppendText("Failed to accept connection. \r\n\r\n");
        return;
    }
    
    UpdateUI();
}

void Socket_OnDisconnect(CSocket* socketPtr, const CSocketEvent& eventObj)
//Runs when the remote socket closes the connection
{   
    UpdateUI();
}

void Socket_OnDataArrival(CSocket* socketPtr, const CSocketEvent& eventObj)
//Runs when data arrives on the socket
{
    string dataStr;
    
    //Retrieve incoming data from socketPtr
    if(!(socketPtr->GetData(dataStr)))
    {
        dispBox.AppendText("Failed to retrieve data. \r\n\r\n");
        return;
    }
    
    dispBox.AppendText("Received: \r\n");
    dispBox.AppendText(dataStr.c_str());
    dispBox.AppendText("\r\n\r\n");
    
    //Send data back to client
    if(!(socketPtr->SendData(dataStr)))
    {
        dispBox.AppendText("Failed to send data. \r\n\r\n");
        return;
    }
    
    dispBox.AppendText("Data echoed. \r\n\r\n");
}

void Socket_OnError(CSocket* socketPtr, const CSocketEvent& eventObj)
//Something went wrong with the socket
{
    dispBox.AppendText("Socket error: '");
    dispBox.AppendText((socketPtr->socketErrorStr).c_str());
    dispBox.AppendText("'. \r\n\r\n");
    
    UpdateUI();
}

void UpdateUI(void)
//Update the user interface controls
{
    //Update input controls
    if(serverSocketPtr->socketState == SOCKET_STATE_DISCONNECTED)
    {
        listenBtn.Enable();
        disconnectBtn.Disable();
        portEdit.Enable();
    }
    else
    {
        listenBtn.Disable();
        disconnectBtn.Enable();
        portEdit.Disable();
	}
    
    //Display the current socket state
    dispBox.AppendText("Socket ");
    dispBox.AppendText(serverSocketPtr->GetStateStr());
    dispBox.AppendText("\r\n\r\n");
    statusBox.SetText(serverSocketPtr->GetStateStr());
}


