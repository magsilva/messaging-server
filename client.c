//========================================================================
// + ePanimondas Message Server +
//========================================================================
#include "client.h"

// ---------- Variaveis Globais -----------------------------------------
	char *UserList[] =	{
				"0123456789",
				"00111",
				(char *) NULL,
				};

        char *MessageList[] =	{
				"MSG01",
				"MSG02",
				(char *)NULL,
				};

//==========================================================================
int main(int argc, char *argv[]){
	int i, ch,mn;

	printf("\n\n>++++++++++ ePanimondas Message Server ++++++++++< \n\n");
	printf("Starting Client Mode: Waiting...\n");
	if (startClient(argv) == 0)
		return 0;

	/* Inicializar ncurses  */
	initscr();
	cbreak();
	noecho();
	keypad(stdscr,TRUE);

	init_windows(varWindow,WINNUMBER);

 	/* Associar WINDOW aos PANEL   */
	for (i = 0; i < WINNUMBER; i++)
	    varPanel[i] = new_panel(varWindow[i]);

	/* ocultar panel da lista de mensagens */
	hide_panel(varPanel[winMSGLIST]);


	/* ---- CRIAR THREADS ---- */
	if ( pthread_create( &sniffer, NULL, goSniffer, NULL ) == -1 ) {
		return 1;
	}

	pthread_mutex_init( &mutex_lista , NULL);
	pthread_mutex_init( &mutex_mensagem , NULL );

	/* Atualizar janela */
	update_panels();
	doupdate();

	while((ch = getch()) != KEY_F(9)) {
		switch(ch) {
			// -- LOGIN ------------------------------------------
			case KEY_F(2):
				if (status == 'N')
					if ((status = login_user(varWindow)) == 'Y')
						create_UserList();
				break;

			// -- NEW USER --------------------------------------
			case KEY_F(3):
				if (status == 'N')
					new_user(varWindow);
				break;

			// -- NEW MESSAGE -----------------------------------
			case KEY_F(4):
				if (status == 'Y') {
					while ((mn = getch()) != 10) {
						switch(mn) {
							case KEY_DOWN:
								menu_driver(varUserList,REQ_DOWN_ITEM);
								break;
							case KEY_UP:
								menu_driver(varUserList,REQ_UP_ITEM);
								break;
							case KEY_NPAGE:
								menu_driver(varUserList,REQ_SCR_DPAGE);
								break;
							case KEY_PPAGE:
								menu_driver(varUserList,REQ_SCR_UPAGE);
								break;
						}
						update_panels();
						doupdate();
					}
					new_message((char*)item_name(current_item(varUserList)));
				}
				break;

			// -- VIEW MESSAGE ----------------------------------
			case KEY_F(5):
				if (status == 'Y') {
					create_MessageList();
					hide_panel(varPanel[winHELP]);
					show_panel(varPanel[winMSGLIST]);

					update_panels();
					doupdate();

					while ((mn = getch()) != 27) {
						switch(mn) {
							case KEY_DOWN:
								menu_driver(varMsgList,REQ_DOWN_ITEM);
								break;
							case KEY_UP:
								menu_driver(varMsgList,REQ_UP_ITEM);
								break;
							case KEY_NPAGE:
								menu_driver(varMsgList,REQ_SCR_DPAGE);
								break;
							case KEY_PPAGE:
								menu_driver(varMsgList,REQ_SCR_UPAGE);

								break
								;
							case 10:
								view_message(item_name(current_item(varMsgList)));
								break;
						}
						update_panels();
						doupdate();
					}

					hide_panel(varPanel[winMSGLIST]);
					show_panel(varPanel[winHELP]);
				}
				break;

			// -- ADD USER --------------------------------------
			case KEY_F(6):
				if (status == 'Y')
					add_user();
				break;

			// -- DEL USER --------------------------------------
			case KEY_F(7):
				if (status == 'Y') {
					while ((mn = getch()) != 10) {
						switch(mn) {
							case KEY_DOWN:
								menu_driver(varUserList,REQ_DOWN_ITEM);
								break;
							case KEY_UP:
								menu_driver(varUserList,REQ_UP_ITEM);
								break;
							case KEY_NPAGE:
								menu_driver(varUserList,REQ_SCR_DPAGE);
								break;
							case KEY_PPAGE:
								menu_driver(varUserList,REQ_SCR_UPAGE);
								break;
						}
						delete_user((char*)item_name(current_item(varUserList)));
						update_panels();
						doupdate();
					}
				}
			break;
		}

	// verificar novos usuarios

 	update_panels();
	doupdate();
	}
	endwin();

	pthread_cancel( sniffer );
	pthread_mutex_destroy( &mutex_lista );
	pthread_mutex_destroy( &mutex_mensagem );

	// efetyar logoff
	sendMsgType8(myID);

	return 1;
}
//===================================================================================
/*
	* ### FUNCAO startClient (char *argv[])
	*
	* Objetivo	: Veriricar parametros (IP/Porta) informados pelo usuario na linha de
	*             comando. O cliente envia uma mensagem Tipo 0 para o servidor e aguarda
	*				  o sinal de ack.
	* Parametros: argv - IP / Porta informado na linha de comando.
	*
*/
int startClient( char *argv[] ) {
	if ( argv[ 1 ] == NULL )	{
		// Se o IP = NULL -> Impossivel conectar ao server
		printf("\nError: missing ip server number" );
		printf("\nUsage: %s [IP ou hostname]", argv[0]);
		printf("\nE.g..: %s 143.107.232.101\n", argv[0]);
		return 0;
	} else {
		// Verificar se servidor estah ON-LINE
		getaddrinfo( argv[1], NULL, NULL, &res0);

		for (res = res0; res; res = res->ai_next) {
			memcpy( &server, res->ai_addr, sizeof( struct sockaddr_in ) );
		}
		freeaddrinfo(res0);

		fd = socket( AF_INET, SOCK_DGRAM, 0 );
		if ( fd < 0 ) {
			perror( "Socket creation impossible" );
			return 0;
		}

		server.sin_family = AF_INET;
		server.sin_port = htons( SERVERPORT );
		memset( &(server.sin_zero), '\0', 8 );
/*
		// Enviar uma mensagem Tipo 0 p/ verifiacar se o servidor esta ONLINE
		if (sendMsgType0() == 1)
			// servidor recebeu a mensagem
			return 1;
		else
		  // servidor nao recebeu a mensagem
			printf("Error: Servidor não responde. Verifique o IP e try again! \n\n");
*/
	  return 1;
	}
}

/*
	* ### FUNCAO init_windows (WINDOW **xWin, int n)
	*
	* Objetivos	: Criar as janelas para o ambiente do ePanimondas Client. Sao
	*				  criadas 7 janelas:	Janela 0: tem o tamanho do terminal (25x80)
	*				  e nela é formada a borda do fundo;
	*			     Janela 1: janela utilizada para armazenar o menu dos ususarios;
	*				  Janela 2: nesta janela ficam os avisos para o usuario bem como as explciacoes
	*				  das funcoes dos menu;
	*				  Janela 3: janela inferior onde ficam as funcoes do programa (teclas de funcao);
	*             Janela 4: menu com a lista de usuarios;
	*             Janela 5: menu com o cabecalho da lista de mensagens do ususario;
   * Parametros : **xWin: estrutura das janelas
	*					n     : numero de janelas
	* Variaveis:   i		: utilizada para colocar borda em todas as janelas
*/
void init_windows (WINDOW **xWin, int n){
	int i;

	// Criar Janelas
	xWin[winBORDER]   = newwin(25, 80,  0,  0);	// Janela 0: Borda do fundo
	xWin[winUSER]     = newwin(20, 26,  1, 53);  // Janela 1: Lista dos Usuarios
	xWin[winHELP]     = newwin(20, 52,  1,  1);	// Janela 2: Avisos para os usuarios (tela inicial)
	xWin[winMENU]     = newwin( 3, 78, 21,  1);	// Janela 3: Menu inferior com as funçoes do programa
	xWin[winUSERLIST] = newwin(15, 24,  3, 54);	// Janela 5: Menu com as Usuarios
	xWin[winMSGLIST]  = newwin( 8, 50,  2,  2);  // Janela 6: Lista do cabecalho das MSG do usuario

	// Colocar borda em algumas janelas
	for (i=0; i < (n-2); i++)
        	box(xWin[i],0,0);

	// configuracoes: xWin[1] - lista de usuarios
	mvwaddch(xWin[winUSER],  0,  7, ACS_DIAMOND);
	mvwprintw(xWin[winUSER], 0,  9, "%s","USER LIST");
   mvwaddch(xWin[winUSER],  0, 19, ACS_DIAMOND);
	mvwaddch(xWin[winUSER], 17,  0, ACS_LTEE);
	mvwhline(xWin[winUSER], 17,  1, ACS_HLINE, 24);
	mvwaddch(xWin[winUSER], 17, 25, ACS_RTEE);

   // configuracoes: xWin[2]- janela de avisos
   mvwprintw(xWin[winHELP], 2,  2, "%s","    + Welcome to ePanimodas Message Server + ");
	mvwprintw(xWin[winHELP], 4,  2, "%s","> Command List:");
	mvwprintw(xWin[winHELP], 5,  2, "%s","  + F2: Login	 ");
	mvwprintw(xWin[winHELP], 6,  2, "%s","          |... F4: New Message ");
	mvwprintw(xWin[winHELP], 7,  2, "%s","          |... F5: View New Messages ");
	mvwprintw(xWin[winHELP], 8,  2, "%s","          |... F6: Add User to User List ");
	mvwprintw(xWin[winHELP], 9,  2, "%s","          |... F7: Delete User from User List ");
	mvwprintw(xWin[winHELP],10,  2, "%s","   + F3: New User		");
	mvwprintw(xWin[winHELP],11,  2, "%s","   + F9: Exit			");

   // configuracoes: xWin[3] - menu
   mvwprintw(xWin[winMENU], 1,  2, "%s","F2 LOGIN    | F3 NEW USER     | F9 LOGOUT      ");
   mvwaddch(xWin[winMENU],  2, 63, ACS_DIAMOND);
   mvwaddch(xWin[winMENU],  2, 74, ACS_DIAMOND);
   mvwprintw(xWin[winMENU], 2, 65, "%s","OFF-LINE");

	// configuracoes: xWin[winMSGLIST] - Lista do cabecalho das MSG do usuario
	mvwprintw(xWin[winMSGLIST], 7,  2, "%s","<ENTER>: View MSG  /  <ESC>: Return to Menu ");
}

/*
	* ### FUNCAO login_user(WINDOW **xWindow)
	*
	* Objetivos	: Verificar se o UserID / Password informados pelo clientes são validos. Para
	*			     isso o cliente envia ao servidor uma Mensagem Tipo 1 e aguarda uma confirmacao
	*		        do servidor (mensagem de ack). Caso confirmacao = 1, o usuario passa para o
	*			     estado ON-LINE, caso contrario continua em OFF-LINE.
	*
	* Parametros: **xWindow: estrutura das janelas para atualizacao de informacoes
	* Variaveis : userID   : Nro do usuario digitado pelo cliente.
	*				  password : Password digitado pelo cliente.
	*				  xWin     : Variavel adicional para receber informacoes do cliente
	*				  xPanel   : Variavel adicional para trabalhar com a xWin
*/

char login_user(WINDOW **xWindow){
	char userID[cUSERID];
	char password[cPASSWORD];
	WINDOW *xWin;
	PANEL *xPanel;

	// Criar janela para receber informacoes do cliente
	xWin = newwin(20, 52,  1,  1);
	box(xWin,0,0);
	xPanel = new_panel(xWin);

	echo();
	mvwprintw(xWin,1,2,"%s",">> LOGIN   ");
	mvwprintw(xWin,3,2,"%s","   User ID : ");
	mvwprintw(xWin,4,2,"%s","   Password: ");
	doupdate();

	mvwgetnstr(xWin, 3, 16, userID  , cUSERID);
	mvwgetnstr(xWin, 4, 16, password, cPASSWORD);

	doupdate();
	noecho();

	// enviar mensagem para o servidor com os dados digitados
	if ( sendMsgType1(atoi (userID),password) == 1 ) {
		// USUARIO EXITE!
		mvwprintw(xWindow[winMENU],  2, 65, "%s"," ON-LINE ");
		mvwprintw(xWindow[winMENU],  1,  2, "%s","F4 NEW MSG | F5 VIEW MSG | F6 ADD USER  | F7 DEL USER | F9 EXIT ");
		myID = atoi( userID );
		mvwprintw(xWindow[winUSER], 18,  2, "%s%s","ICQ #: ",myID);


		//  RECEBER LISTA DE USUARIOS
		//sendMsgType6( myID );

		del_panel(xPanel);
		update_panels();
		doupdate();
		return 'Y';
	} else {
		// USUARIO NAO EXITE
    mvwprintw(xWin, 6, 2,"%s"," Username or password invalid.<ENTER>");
		update_panels();
   	doupdate();
		getch();
		del_panel(xPanel);
		return 'N';
	}
}

/*
	* ### FUNCAO new_user(WINDOW **xWindow)
	*
	* Objetivos	: Cadastrar um novo usuario no servidor. Para isso o cliente envia uma mensagem
	*				  tipo 2 com as informacoes a serem cadastradas no servidor. As informacoes enviadas
	*				  sao nome completo (fullname), email, apelido (nick) e senha (password).
	* Parametros: **xWindow: estrutura das janelas para atualizacao de informacoes
	* Variaveis : fullname - nome completo do novo usuario
	*				  email - email do novo usuario
	*				  nick - apelido do novo usuario
	*				  password - senha do novo usuario
	*				  xWin     : Variavel adicional para receber informacoes do cliente
	*				  xPanel   : Variavel adicional para trabalhar com a xWin
	*
*/

void new_user(WINDOW **xWindow) {
   char fullname[cFULLNAME];
   char email[cEMAIL];
   char nick[cNICK];
   char password[cPASSWORD];

   WINDOW *xWin;
	PANEL *xPanel;

	// Criar janela para receber informacoes do cliente
	xWin = newwin(20, 52,  1,  1);
	box(xWin,0,0);
	xPanel = new_panel(xWin);

   echo();
   mvwprintw(xWin,1,2,"%s",">> NEW USER   ");
   mvwprintw(xWin,3,2,"%s","   Full Name: ");
   mvwprintw(xWin,4,2,"%s","   E-mail   : ");
   mvwprintw(xWin,5,2,"%s","   Nick     : ");
   mvwprintw(xWin,6,2,"%s","   Password : ");
   doupdate();

	// receber informacoes do teclado
   mvwgetnstr(xWin, 3, 16, fullname	, cFULLNAME);
   mvwgetnstr(xWin, 4, 16, email	, cEMAIL);
   mvwgetnstr(xWin, 5, 16, nick		, cNICK);
   mvwgetnstr(xWin, 6, 16, password	, cPASSWORD);
   noecho();

	// enviar mensagem ao servidor com os dados
	myID = sendMsgType2( fullname, email, nick, password);
	if ( myID == 0 ) {
		// Nao foi possivel registrar o usuario
	} else {
		// usuario cadastrado
		sendMsgType1( myID, password );
		mvwprintw(xWindow[winMENU],  2, 65, "%s"," ON-LINE ");
		mvwprintw(xWindow[winMENU],  1,  2, "%s","F4 NEW MSG | F5 VIEW MSG | F6 ADD USER  | F7 DEL USER | F9 EXIT ");
		mvwprintw(xWindow[winUSER], 18,  2, "%s%s","ICQ #: ",myID);
		status = 'Y';
	}
	del_panel(xPanel);
}

/*
	* ### FUNCAO new_message(char *xUserID)
	*
	* Objetivos	: Envia um mensagem para um outro usuario. Para isso o cliente deve escolher
	*			     esse usuário na User List e digitar a mensagem. Com isso um Mensagem Tipo 3
	*				  é enviada ao servidor que é responsavel por encaminhar a mensagem por cliente
	*				  ,caso este esteja ONLINE, ou guardar em uma fila de mensagens
	* Parametros: xUserID - Identificação do usuário destinatário da mensagem
	* Variaveis	: message - mensagem digital pelo cliente
	*				  xWin/xPanel - variaveis adicionais para construcao da janela
	*
*/

void new_message(char *xUserID) {
	char message[cMSGSIZE];
	WINDOW *xWin;
	PANEL *xPanel;

	// Criar janela para receber as informacoes
	xWin = newwin(18, 47,  2,  3);
	xPanel = new_panel(xWin);
	mvwprintw(xWin,  1,  2, "%s", ">> NEW MESSAGE ");
	mvwprintw(xWin,  3,  2, "   USER ID: %s",xUserID);
	mvwprintw(xWin,  4,  2, "%s", "   MESSAGE: ");
	mvwprintw(xWin, 17,  2, "%s", ">> PRESS <ENTER> TO SEND ");
	doupdate();

	echo();
	mvwgetnstr(xWin,4,14, message,	cMSGSIZE-1);

	/* ENVIAR MENSAGEM PARA OUTRO USUARIOS */
	sendMsgType3( myID, (atoi (xUserID)), message );

	doupdate();
	noecho();
	del_panel(xPanel);
}

/*
	* ### FUNCAO view_message(char *xUserID)
	*
	* Objetivos	: Visualiza as mensagens que chegaram para o cliente.
	* Parametros: xUserID: Identificacao do usuario remente
	* Variaveis	: xWin/xPanel - construcao da janela para receber as informacoes
	*
*/
int view_message(int xUserID) {
	WINDOW *xWin;
	PANEL *xPanel;

	xWin   = newwin(11, 50, 10,  2);
	xPanel = new_panel(xWin);
	mvwprintw(xWin,  1,  2, "%s", ">> XXXXXXXXXXXXXXXXX ");
	update_panels();
	doupdate();

	getch();
	del_panel(xPanel);
	return 1;
}

/*
	* ### FUNCAO add_user()
	*
	* Objetivos	: Adiciona um novo usuario a lista de contato do cliente
	* Parametros: nenhum
	* Variaveis	: userID - Identificacao do usuario a ser adicionado
	*				  xWin/xPanel - construcao da window auxiliar
	*
*/
void add_user() {
	char userID[cUSERID];
	WINDOW *xWin;
	PANEL *xPanel;

	xWin = newwin(18, 47,  2,  3); // Cria janela
	xPanel = new_panel(xWin);
	mvwprintw(xWin,  1,  2, "%s", ">> ADD USER TO USER LIST ");
	mvwprintw(xWin,  3,  2, "%s", "     USER ID: ");
	doupdate();
	echo();
	mvwgetnstr(xWin,3,16,userID,cUSERID);
	noecho();

	if ( sendMsgType4( myID, atoi(userID) ) == 1 ) {
		sendMsgType6( myID );
	}
	del_panel(xPanel);
}

/*
	* ### FUNCAO delete_message(xUserID)
	*
	* Objetivos	: Apaga um usuario da lista de contatos do cliente
	* Parametros:
	* Variaveis	:
	*
*/
void delete_user(char *xUserID) {
	sendMsgType5( myID, atoi(xUserID) );
}

/*
	* ### FUNCAO create_UserList()
	*
	* Objetivos	: Gera a lista de usuarios do cliente logado
	* Parametros:
	* Variaveis	:
	*
*/
int create_UserList() {
	int i;

	//msgType4(nroIP,nroPort,myID);

	/* RETONAR A LISTA DE USUARIOS */

	// -- Create USER LIST ------
	UserListSize = (sizeof(UserList)/sizeof(UserList[0]));
	varUserListItem = (ITEM **)calloc(UserListSize, sizeof(ITEM *));

	for (i = 0; i < UserListSize; ++i)
		varUserListItem[i] = new_item(UserList[i], "ON");

	varUserList = new_menu((ITEM **)varUserListItem);

	// Associar menu a janela
	set_menu_win(varUserList, varWindow[winUSERLIST]);
	set_menu_sub(varUserList, derwin(varWindow[winUSERLIST],6,38,3,1));
	set_menu_format(varUserList, 14, 1);	// (menu, nro linhas)
	set_menu_mark(varUserList, " * ");

	post_menu(varUserList);
	wrefresh(varWindow[winUSERLIST]);

	return 1;
}

/*
	* ### FUNCAO create_MessageList()
	*
	* Objetivos	: Gera a lista de mensagens do usuario logado.
	* Parametros:
	* Variaveis	:
	*
*/
int create_MessageList() {
	int i;

	// -- Create Message List -------
	MsgListSize = (sizeof(MessageList)/sizeof(MessageList[0]));
	varMsgListItem = (ITEM **)calloc(MsgListSize, sizeof(ITEM *));

	for (i = 0; i < MsgListSize; ++i)
		varMsgListItem[i] = new_item(MessageList[i], "A");

	varMsgList = new_menu((ITEM **)varMsgListItem);
	// Associar menu a janela
	set_menu_win(varMsgList, varWindow[winMSGLIST]);
	set_menu_sub(varMsgList, derwin(varWindow[winMSGLIST],6,38,3,1));
	set_menu_format(varMsgList, 6, 1);	// (menu, nro linhas)
	set_menu_mark(varMsgList, " FROM: ");

	post_menu(varMsgList);
	wrefresh(varWindow[winMSGLIST]);

	return 1;
}
/*
	* ### FUNCAO goSniffer()
*/
void *goSniffer(){
}
