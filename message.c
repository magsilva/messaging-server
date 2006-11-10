#include "message.h"

/**
 * Retorna o tamanho (em bytes) do cabeçalho da mensagem (tamanho quando se trata
 * de enviar pela rede, o tamanho real é de messageHeaderSize() + sizeof(char *) ).
 */
unsigned int messageHeaderSize() {
	unsigned int size = 0;

	size += 5 * sizeof( unsigned int );
	size += 2 * sizeof( unsigned short );
	return size;
}

/**
 * Retorna o tamanho real da mensagem (cabeçalhos mais dados que dependem
 * de seus campos internos. Novamente, o tamanho quando se trata de enviar pela rede.
 */
unsigned int messageSize( struct message_t *message ) {
	unsigned int size = 0;

	size += messageHeaderSize();
	size += message->argumentsSize * sizeof( char );
	return size;
}

/**
 * Constroi um pacote (buffer) para uma dada mensagem.
 */
char *buildBuffer( struct message_t *message ) {
	char *buffer;
	buffer = (char *)malloc( messageHeaderSize() + message->argumentsSize );
	if ( buffer == NULL ) {
		return NULL;
	}
	message->sequence = 0;
	message->crc16 = get_crc16( buffer, messageHeaderSize() );
	memcpy( buffer, message, messageHeaderSize() );
	memcpy( buffer + messageHeaderSize() + 1, message->arguments, message->argumentsSize * sizeof( char ) );
	return buffer;		
}

/**
 * Libera a memória utilizada por uma mensagem (inclusive seus argumentos).
 */
int freeMessage( struct message_t *message ) {
	free( message->arguments );
	free( message );
	return 1;
}

/**
 * Constroi uma mensagem a partir dos dados no pacote (buffer).
 */
struct message_t *buildMessage( char *buffer ) {
	struct message_t *message;

	message = (struct message_t *)malloc( sizeof( struct message_t ) );
	memcpy( message, buffer, messageHeaderSize() );
//	if ( message->crc16 == get_crc16( buffer, messageHeaderSize() ) ) {
		message->arguments = (char *)malloc( message->argumentsSize * sizeof( char ) );
		memcpy( message->arguments, buffer + messageHeaderSize(), message->argumentsSize );
		return message;
//	} else {
//		free( message );
//		return NULL;
//	}
}

/**
 * Envia uma mensagem e espera o acknowledge. Esta função é utilizada pelos clientes.
 */
int sendMessage( int fd, struct sockaddr *to, struct message_t *message, struct fifo_t *messages ) {
	// Estrutura utilizada pelo select (conjunto de descritores de arquivo)
	fd_set rfds;
	
	int result = 0;

	// Timeout do select
	struct timeval tv;
	
	// Mensagem de confirmação
	struct message_t *acknowledge;

	// Utilizado para criar o pacote para o envio da mensagem de confirmação.
	char *buffer;
	
	int length = 0;
 
	printf( "\n\n\n%d\n\n", messageSize( message ) );
	
	// Prepara as estruturas para o select
	FD_ZERO( &rfds );
	FD_SET( fd, &rfds );

	length = sizeof( struct sockaddr );
	
	// Prepara a mensagem a ser enviada.
	buffer = buildBuffer( message );
	
	// Tenta enviar a mensagem.
	if ( sendto( fd, buffer, messageSize( message ), 0, to, length ) == - 1 ) {
		free( buffer );
		return -1;
	}

	// Configura o timeout do select.
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	result = 1;

	// Espera a confirmação (acknowledge).
	while ( result != 0 ) {
		result = select( fd + 1, &rfds, NULL, NULL, &tv );

		// Acknowledge não foi recebido (timed out). Desiste.
		if ( result != 0 ) {
			// Trata mensagem que chegou.
			acknowledge = receiveMessage( fd );
			if ( acknowledge != NULL ) {
				// If the received message was the sent's one acknowledge, ok (but free it, please)
				if ( message->to == acknowledge->from && message->messageId == acknowledge->messageId ) {
					// Libera argumentos da mensagem para poder colocar a resposta do acknowledge em seu lugar
					free( message->arguments );
					message->arguments = acknowledge->arguments;	
					free( acknowledge );
					free( buffer );
					return 1;
				// If not, puts into the queue (if there's one...) and keep on the select loop.
				} else {
			 		fifo_put( messages, acknowledge );
				}
			}
		}
	}
	
	// No one can't tell we haven't tried our best...
	free( buffer );
	return -1;
}

/**
 * Recebe uma mensagem.
 */
struct message_t *receiveMessage( int fd ) {
	char buffer[ MAX_BUFFER_SIZE ];
	struct sockaddr_in *fromAddress;
	int length = 0;
	struct message_t *message;

	length = sizeof( struct sockaddr_in );	
	fromAddress = (struct sockaddr_in *)malloc( length );
	if ( recvfrom( fd, buffer, sizeof( buffer ), 0, (struct sockaddr *)fromAddress, &length ) == -1 ) {
		perror( "\nreceiveMessage" );
		return NULL;
	} else {
		printf( "\nPorta: %i  Endereço: %s", ntohs( fromAddress->sin_port ), inet_ntoa( fromAddress->sin_addr ) );

		length = sizeof( struct sockaddr_in );
		printf( "\nTamanho do sockaddr_in em bytes: %d", length );
		message = buildMessage( buffer );
		if ( message->type == 2 ) {
			message->argumentsSize = message->argumentsSize + sizeof( struct sockaddr_in );
			message->arguments = (char *)realloc( message->arguments, message->argumentsSize );
			memcpy( message->arguments + 76, &fromAddress, length );
			fromAddress = (struct sockaddr_in *)message->arguments + 76;
	    printf( "\nPorta: %i  Endereço: %s", ntohs( fromAddress->sin_port ), inet_ntoa( fromAddress->sin_addr ) );	
		}
		if ( message->type == 1 ) {
			message->argumentsSize = message->argumentsSize + sizeof( struct sockaddr_in );
			message->arguments = (char *)realloc( message->arguments, message->argumentsSize );
			memcpy( message->arguments + 10, &fromAddress, length );
			fromAddress = (struct sockaddr_in *)message->arguments + 76;
			printf( "\nPorta: %i  Endereço: %s", ntohs( fromAddress->sin_port ), inet_ntoa( fromAddress->sin_addr ) );	
		}
		
#ifdef DEBUG
		printf( "\n%d | %d | %d | %d | %d | %d | %d | ", message->from, message->to, message->type, message->messageId, message->sequence, message->argumentsSize, message->crc16 );
		for ( length = 0; length < message->argumentsSize; length++ ) {
			printf( "%c", message->arguments[ length ] );
		}
#endif
		return message;
	}
}

/**
 * Envia uma mensagem de acknowledge. Espera-se que "argument" e "argumentsSize
 * estejam corretamente configurados.
 */ 
int sendAcknowledgeMessage( int fd, struct message_t *message ) {
	char *buffer;
	struct sockaddr *to;
	int result;
	
	to = get_user_address( message->to );
	// Someone not registrered officially trying to send a package.
	if ( to == NULL ) {
		return -1;
	}
	
	message->type = 7;
	
	// Prepara a mensagem a ser enviada.
	buffer = buildBuffer( message );

#ifdef DEBUG
	printf( "\n%d | %d | %d | %d | %d | %d | %d | ", message->from, message->to, message->type, message->messageId, message->sequence, message->argumentsSize, message->crc16 );
	for ( result = 0; result < message->argumentsSize; result++ ) {
		printf( "%c", message->arguments[ result ] );
	}
#endif
	
	result =  sendto( fd, buffer, messageSize( message ), 0, to, sizeof( struct sockaddr ) );
	free( buffer );
	return result;
}


