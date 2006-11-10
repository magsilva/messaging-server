#ifndef CLIENT_H
#define CLIENT_H

// LIBS conferidas
#include <ncurses.h>
#include <panel.h>
#include <menu.h>
#include <sys/types.h>
#include <sys/socket.h>

// -----
#include <string.h>
#include <stdio.h>
#include "sendmsg.h"
#include <pthread.h>

// ---------- Constantes ------------------------------------------------
#define WINNUMBER		6	// Nro de janelas

// Definicao do tamanho dos dados
#define cUSERID	  10
#define cPASSWORD	  10
#define cFULLNAME	  30
#define cEMAIL		  25
#define cNICK		  10
#define cMSGSIZE   500
// Especificacao das Janelas
#define winBORDER    0
#define winUSER      1
#define winHELP	   2
#define winMENU	   3
#define winUSERLIST  4
#define winMSGLIST   5

#define SERVERPORT 3456

// ---------- Variaveis Globais ------------------------------------------------
	pthread_t sniffer;
	pthread_mutex_t mutex_lista;
	pthread_mutex_t mutex_mensagem;

	char status = 'N';
	unsigned int myID;

	// Variaveis para ncurses
	WINDOW 	*varWindow[WINNUMBER];
	PANEL 	*varPanel[WINNUMBER];
	MENU		*varUserList,*varMsgList;
	ITEM 		**varUserListItem, **varMsgListItem;
	int UserListSize, MsgListSize;


// ---------- Funcoes ----------------------------------------------------------

int startClient (char *argv[]);

void init_windows (WINDOW **xWin, int n);

char login_user(WINDOW **xWindow);

void new_user(WINDOW **xWindow);

void new_message(char *xUserID);

void add_user();

void delete_user(char *xUserID);

int view_message();

int create_UserList();

int create_MessageList();

void *goSniffer();

#endif
