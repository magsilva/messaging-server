#include "message_type.h"

int messageType0( int fd, struct message_t *incomingMessage ) {
	struct message_t nullAck;
#ifdef DEBUG
	printf( "\nNULL message " );
#endif
	
	nullAck.to = incomingMessage->from;
	nullAck.messageId = incomingMessage->messageId;
	nullAck.argumentsSize = 0;
	nullAck.arguments = NULL;
	sendAcknowledgeMessage( fd, &nullAck );
	
	return 1;
}

int messageType1( int fd, struct message_t *incomingMessage ) {
	struct message_t loginAck;
	char password[ PASSWORD ];
	unsigned int id, ack;

#ifdef DEBUG
	printf( "\nLogin message" );
#endif

	id = incomingMessage->from;
	memcpy( password, incomingMessage->arguments, sizeof( password ) );

	loginAck.to = incomingMessage->from;
	loginAck.messageId = incomingMessage->messageId;
	loginAck.argumentsSize = sizeof( int );

	ack = user_login( id, password );
	if ( ack == 1 ) {
		sendAcknowledgeMessage( fd, &loginAck );
	} 
	return ack;
}

/**
 * Atende uma mensagem de cadastramento de novo usuário no servidor.
 */
int messageType2( int fd, struct message_t *incomingMessage ) {
	struct message_t registerUserAck;
	struct sockaddr_in *userAddress;
	char fullname[ FULLNAME ];
	char nick[ NICK ];
	char email[ EMAIL ];
	char password[ PASSWORD ];
	char *buffer;
	int ack;

	userAddress = (struct sockaddr_in *)malloc( sizeof( struct sockaddr_in ) );
	buffer = incomingMessage->arguments;
	strncpy( fullname, buffer, sizeof( fullname ) );
	strncpy( email, buffer + sizeof( fullname ) + 1, sizeof(email) );
	strncpy( nick, buffer + sizeof( fullname ) + sizeof( email ) + 1, sizeof( nick ) );
	strncpy( password, buffer + sizeof( fullname ) + sizeof( email ) + sizeof( nick ) + 1, sizeof( password ) );
	memcpy( userAddress, buffer + sizeof( fullname ) + sizeof( email ) + sizeof( nick ) + sizeof( password ) + 1, sizeof( userAddress ) );
	printf( "\nPorta: %i  Endereço: %s", ntohs( userAddress->sin_port ), inet_ntoa( userAddress->sin_addr ) ); 
	
					
	fullname[ FULLNAME - 1 ] = '\0';
	nick[ NICK - 1 ] = '\0';
	email[ EMAIL - 1 ] = '\0';
	password[ PASSWORD - 1 ] = '\0';
	
	registerUserAck.to = incomingMessage->from;
	registerUserAck.messageId = incomingMessage->messageId;
	registerUserAck.argumentsSize = sizeof ( unsigned int );

	ack = user_register( nick , password, fullname, email );
	set_user_address( ack, (struct sockaddr *)userAddress );

	*registerUserAck.arguments = ack;
	sendAcknowledgeMessage( fd, &registerUserAck );
	return 1;
}

int messageType4( int fd, struct message_t *incomingMessage ) {
	struct message_t add_user_listAck;
	int userId;
	int addId;
	char nick[ NICK ];
	int ack;

#ifdef DEBUG
	printf( "\nAdicionar contato" );
#endif

	userId = incomingMessage->to;
	memcpy( &addId, incomingMessage->arguments, sizeof( addId ) );
	memcpy( nick, incomingMessage->arguments + sizeof( addId ), sizeof( nick ) ); 

	add_user_listAck.to = incomingMessage->from;
	add_user_listAck.messageId = incomingMessage->messageId;
	add_user_listAck.argumentsSize = sizeof ( int );

	ack = add_user_list( userId, addId, nick );
	if (ack == 1) {
		//envia mensagem 7 com argumento = 1
	} else {
		//envia mensagem 7 com argumento -1

	}

	return 1;

}

int messageType5( int fd, struct message_t *incomingMessage ) {
	struct message_t removeAck;
	unsigned int userId;
	unsigned int rmId;
	int ack;

#ifdef DEBUG
	printf( "\nApagar contato" );
#endif

	userId = incomingMessage->from;
	rmId = atoi( incomingMessage->arguments );

	removeAck.to = incomingMessage->from;
	removeAck.messageId = incomingMessage->messageId;
	removeAck.argumentsSize = 0;

	ack = remove_user_list( userId, rmId );
	if (ack == 1) {
		sendAcknowledgeMessage( fd, &removeAck);
	}

	return 1;
}

int messageType6( int fd, struct message_t *incomingMessage ) {

	struct message_t getlistAck;
	int userId;
	int ack;

#ifdef DEBUG
	printf( "\nRequisitar lista de contato" );
#endif
	
	userId = incomingMessage->from;

	getlistAck.to = incomingMessage->from;
	getlistAck.messageId = incomingMessage->messageId;
	getlistAck.argumentsSize = sizeof( int );

	ack = get_list( userId );
	getlistAck.arguments = (char *)malloc( sizeof ack );
	memcpy( getlistAck.arguments, &ack, sizeof( int ) );

	sendAcknowledgeMessage( fd, &getlistAck);
	return 1;
	
}

int messageType7( int fd, struct message_t *incomingMessage ) {
#ifdef DEBUG
	printf( "\nMensagem de confirmação" );
#endif
	return 1;
}

int messageType8( int fd, struct message_t *incomingMessage ) {
	struct message_t logoffAck;
	unsigned int userId;
#ifdef DEBUG
	printf( "\nLogoff" );
#endif
	
  userId = incomingMessage->from;

	logoffAck.to = incomingMessage->from;
	logoffAck.messageId = incomingMessage->messageId;
	logoffAck.argumentsSize = 0;

	user_logoff( userId );
	sendAcknowledgeMessage( fd, &logoffAck);
	return 1;

}

int messageType3( int fd, struct message_t *incomingMessage ) {
	struct message_t send_messageAck;
	struct sockaddr *to;
#ifdef DEBUG
	printf( "\nEnviar mensagem" );
#endif

	memcpy( &send_messageAck, incomingMessage, sizeof( send_messageAck ) );
	send_messageAck.to = incomingMessage->from;
	
	// envia ACK
	sendAcknowledgeMessage( fd, &send_messageAck);
	
	  
	// envia mensagem
	to = get_user_address( send_messageAck.to );
	sendMessage( fd, to, &send_messageAck, &messages ); 
	return 1;
}


int messageType9( int fd, struct message_t *incomingMessage ) {
	struct message_t getlistAck;
	struct sockaddr *to;
	
	int userId;

#ifdef DEBUG
	printf( "\nRetorna a lista de contato de um usuário" );
#endif
	
	userId = incomingMessage->from;
	
	getlistAck.to = incomingMessage->from;
	getlistAck.messageId = incomingMessage->messageId;
	getlistAck.arguments = get_list_buffer( userId );
	getlistAck.argumentsSize = sizeof( getlistAck.arguments );
	to = get_user_address( getlistAck.to );
	sendMessage( fd, to,  &getlistAck, &messages );
	return 1;
}
