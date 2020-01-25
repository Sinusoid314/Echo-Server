//Echo Server v1.01
//Written by Andrew Sturges
//April 2015

#ifndef _ECHO_SERVER_H
#define _ECHO_SERVER_H

CMainWindow serverWin;
CEditBox dispBox;
CLabel statusBox;
CLabel statusLabel;
CButton listenBtn;
CButton disconnectBtn;
CEditBox portEdit;
CLabel portLabel;
CSocket* serverSocketPtr = NULL;

bool ServerSetup(void);
void ServerCleanup(void);
int WINAPI WinMain (HINSTANCE, HINSTANCE, LPSTR, int);

LRESULT ServerWin_OnClose(CWindow*, const CWinEvent&);
LRESULT ServerWin_OnSize(CWindow*, const CWinEvent&);
LRESULT ListenBtn_OnClick(CWindow*, const CWinEvent&);
LRESULT DisconnectBtn_OnClick(CWindow*, const CWinEvent&);

void Socket_OnConnectionRequest(CSocket*, const CSocketEvent&);
void Socket_OnDisconnect(CSocket*, const CSocketEvent&);
void Socket_OnDataArrival(CSocket*, const CSocketEvent&);
void Socket_OnError(CSocket*, const CSocketEvent&);

void UpdateUI(void);

#endif
