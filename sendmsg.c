#include "sendmsg.h"

//** precisa NULL o msgSend

/*
	* ### Mensagem Tipo 0 ###
	* A mensagem de tipo 0 é utilizada para teste, de maneira analoga ao RPC NULL.
	* Ao recebe-la, o cliente (ou servidor) devera enviar uma mensagem de
	* acknowledge ao rementente.
*/

int sendMsgType0(){
	struct message_t msgSend;

	msgSend.type = 0;
	msgSend.argumentsSize = 0;
	msgSend.messageId = 777;

	if ( sendMessage( fd, (struct sockaddr *)&server, &msgSend, NULL ) == 1 )	{
		// mensagem enviada com sucesso
		// *** verificar sinal de ACK
		return 1;
	}
	else {
		// erro ao enviar mensagem
		return 0;
	}
}
/*
	* ### Mensagem Tipo 1 ###
	* Utilizada para se registrar (dar login) no sistema. A mensagem
	* de ida contem o identificador do usu?rio e sua senha. A confirmacao
	* nao contem dado nenhum em especial (confirmacao simples).
*/
int sendMsgType1( unsigned int userID, char *userPassword) {
	struct message_t msgSend;
	
	msgSend.from = userID;
	msgSend.to = 0;
	msgSend.type = 1;
	msgSend.messageId = 0;
	msgSend.arguments = userPassword;
	msgSend.argumentsSize = sizeof( char ) * 10;

	if ( sendMessage( fd, (struct sockaddr *)&server, &msgSend, NULL ) == 1 ) {
		int result = 0;
		memcpy( &result, msgSend.arguments, sizeof( result ) );
		return result;
	}	else {
		// erro ao enviar mensagem
		return 0;
	}
}

/*
	* ### Mensagem Tipo 2 ###
	* Utilizada para se cadastrar no sistema. A mensagem de ida contem o nome do
	* usuario, apelido, email e a senha. A confirma??o cont?m o identificador. Se
	* o identificador for 0, significa que nao foi possivel criar a conta do
	* usuario.
*/
unsigned int sendMsgType2( char *fullname, char *email, char *nick, char *password ) {
	struct message_t msgSend;
	unsigned int id = 0;

	
	msgSend.from = 0;
	msgSend.to = 0;
	msgSend.type = 2;
	msgSend.messageId = 0;
	msgSend.argumentsSize = 76 * sizeof( char );
	msgSend.arguments = (char *)malloc( msgSend.argumentsSize );
	strcat( msgSend.arguments, fullname );
	strcat( msgSend.arguments, email );
	strcat( msgSend.arguments, nick );
	strcat( msgSend.arguments, password );
	if ( sendMessage( fd, (struct sockaddr *)&server, &msgSend, NULL) == 1)	{
		// Mensagem enviada com sucesso
		memcpy( &id, (int *)msgSend.arguments, sizeof( int ) );
		return id;
	}	else {
		// erro ao enviar mensagem
		return 0;
	}
}

/*
	* ### Mensagem Tipo 3 ###
	* O envio/recebimento de mensagens para/de outros usuarios é feito por meio
	* deste tipo de mensagem. Esta mensagem vai do remetente para o servidor e
	* depois do servidor para o destinatario. A mensagem contem o texto da
	* mensagem. O rementente recebera uma confirmacao simples do servidor. O
	* destinatario devera enviar uma confirmacao simples ao servidor ao receber a
	* mensagem.
*/
int sendMsgType3( unsigned int myID, unsigned int toID, char *message) {
	struct message_t msgSend;

	msgSend.from = myID;
	msgSend.to = toID;
	msgSend.arguments = (char*)message;
	msgSend.argumentsSize = strlen(message);
	msgSend.type = 3;

	if (sendMessage(fd,(struct sockaddr *) &server,(struct message_t *) &msgSend, NULL) == 1)	{
		// mensagem enviada com sucesso
		// *** verificar sinal de ACK
		return 1;
	}
	else {
		// erro ao enviar mensagem
		return 0;
	}
}

/*
	* ### Mensagem Tipo 4 ###
	* Esse tipo de mensagem é utilizada quando o cliente adiciona um novo contato
	* a sua lista de usuarios. Uma mensagem contendo o codigo de identificacao
	* (userID) do novo contato é enviada ao servidor. O servidor verifica se o
	* userID é valido, ou seja, se esta cadastrado no servidor, e entao retorna
	* uma mensagem de confirmacao (tipo 7) contendo 1 se positivo (o userID do
	* novo contato exite) ou -1 caso contrario.
*/
int sendMsgType4( unsigned int myID, unsigned int addID) {
	struct message_t msgSend;

	msgSend.from = myID;
	msgSend.arguments = (char *)addID;
	msgSend.argumentsSize = sizeof( addID );

	msgSend.type = 4;

	if (sendMessage(fd,(struct sockaddr *) &server,(struct message_t *) &msgSend, NULL) == 1)	{
		// mensagem enviada com sucesso
		// *** verificar sinal de ACK
		return 1;
	}
	else {
		// erro ao enviar mensagem
		return 0;
	}
}

/*
	* ### Mensagem Tipo 5 ###
	* Esse tipo de mensagem é utilizada quando o cliente apaga um novo contato
	* da sua lista de usuarios. Uma mensagem contendo o codigo de identificacao
	* (userID) do contato removido é enviado ao servidor. O servidor verifica se o
	* userID é valido, ou seja, se esta cadastrado no servidor, e entao retorna
	* uma mensagem de confirmacao (tipo 7) contendo 1 se positivo (o userID do
	* novo contato exite) ou -1 caso contrario.
*/
int sendMsgType5( unsigned int myID, unsigned int delID ) {
	struct message_t msgSend;

	msgSend.from = myID;
	msgSend.arguments = (char *)delID;
	msgSend.argumentsSize = sizeof( delID );
	msgSend.type = 5;

	if (sendMessage(fd,(struct sockaddr *) &server,(struct message_t *) &msgSend, NULL) == 1) {
		// mensagem enviada com sucesso
		// *** verificar sinal de ACK
		return 1;
	}
	else {
		// erro ao enviar mensagem
		return 0;
	}
}
/*
	* ### Mensagem Tipo 6 ###
	* Mensagem de requisicao da lista de contatos do cliente. O servidor retorna
	* uma mensagem de acknowledge simples.
*/
int sendMsgType6( unsigned int myID ) {
	struct message_t msgSend;

	msgSend.from = myID;
	msgSend.type = 6;

	if (sendMessage(fd,(struct sockaddr *) &server,(struct message_t *) &msgSend, NULL) == 1)	{
		// mensagem enviada com sucesso
		// *** verificar sinal de ACK
		return 1;
	}
	else {
		// erro ao enviar mensagem
		return 0;
	}
}

/*
	* ### Mensagem Tipo 7 ###
	* Mensagem de acknowledge. Contem como argumento o resultado da operacao (que
	* depende do tipo de mensagem para a qual se fez o acknowledge, mas
	* geralmente é um numero inteiro).
*/
int sendMsgType7( unsigned int myID ) {
	struct message_t msgSend;

	msgSend.from = myID;
	msgSend.type = 7;

	// ***** msgSend.arguments = *****

	if (sendMessage(fd,(struct sockaddr *) &server,(struct message_t *) &msgSend, NULL) == 1)	{
		// mensagem enviada com sucesso
		// *** verificar sinal de ACK
		return 1;
	}
	else {
		// erro ao enviar mensagem
		return 0;
	}
}

/*
	* ### Mensagem tipo 8 ###
	* Mensagem de logoff. Nao contem argumentos e o cliente recebe uma confirmacao
	* sem argumentos.
*/
int sendMsgType8( unsigned int myID ) {
	struct message_t msgSend;

	msgSend.from = myID;
	msgSend.type = 8;


	if (sendMessage(fd,(struct sockaddr *) &server,(struct message_t *) &msgSend, NULL) == 1)	{
		// mensagem enviada com sucesso
		// *** verificar sinal de ACK
		return 1;
	}
	else {
		// erro ao enviar mensagem
		return 0;
	}
}


