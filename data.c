#include "data.h"

/**
 * Inicializa a estrutura da lista de usuários e o semáforo de controle.
 */
int data_user_init() {
	users = NULL;
	return 1;
}

/*
 * Função que aloca memoria, retornando um ponteiro para a estrutura usuario_t.
 */
struct user_t *alloc_memory_user() {
	struct user_t *ptr;
	
	ptr = (struct user_t *)malloc( sizeof( struct user_t ) );
	ptr->next = NULL;
	ptr->list = NULL;
	return ptr;
}

/*
 * Função que aloca memória, retornando um ponteiro para a estrutura contato_t.
 */
struct list_t *alloc_memory_list(){
	struct list_t *ptr;
	
	ptr = (struct list_t *)malloc( sizeof( struct list_t ) );
	ptr->next = NULL;
	return ptr;
}


/*
 * Função que converte um inteiro em uma String. Usada para gerar o nome do arquivo
 * baseado no numero do usuario. Este arquivo representa a lista dos usuarios.
 */
char *convert_id2char( unsigned int num ) {
	char *arq;

	arq = (char *)malloc( sizeof( char ) * USERID );
	sprintf( arq, "%d", num );
	return arq;
}

/*
 * Função que carregaga o conteudo do arquivo usuarios.text e das listas de usuarios para
 * a memoria.
 */
int load_users() {
	struct user_t *ptr;
	FILE *fp;

	if ( ( fp = fopen( USER_DATA_FILENAME, "r" ) ) == NULL ) {
		return -1;
	} else {
		ptr = alloc_memory_user();
		while ( fread( ptr, sizeof( struct user_t ), 1, fp ) ) {
			load_user( ptr );
		}
		fclose( fp );
	}
	return 1;
}

/*
 * Função usada por load_users para carregar informações do usuário em memória.
 */
void load_user( struct user_t *user ) {
	struct user_t *current; 
	
	// list é uma variavel global que corresponde ao inicio da lista mantida na memoria
	if ( users == NULL ) {		
		users = user;
		current = user;
	} else {
		current = users;
		while ( current->next != NULL ) {
			current = current->next;
		}
	}
	current->next = user;
	user->list = load_user_list( current->id );
}

/*
 * Função usada por load_user() para obter as listas de contato do usuario.
 */
struct list_t *load_user_list( unsigned int id ) {
	FILE *fp;
	char *arquivo;
	struct list_t *list = NULL, *current = NULL, *aux = NULL;

	arquivo = convert_id2char( id );
	if ( ( fp = fopen( arquivo, "r" ) ) == NULL ){
		return NULL;
	} else {
		current = alloc_memory_list();
		while ( fread( current, sizeof( struct list_t ), 1, fp ) == 1 ) {
			if ( list == NULL ) {
				list = current;
				aux = current;
			} else {
				aux->next = current;
				aux = current;
			}
		}
		fclose( fp );
		return list;
	}
}

/*
 * Aqui pode ser passado o ponteiro da estrutura dos usuarios como parametro.
 */
int save_user() {
	struct user_t *user;
	struct list_t *list;
	FILE *fp_user, *fp_list;
	char *arquivo;

	if ( ( fp_user = fopen( USER_DATA_FILENAME, "w" ) ) == NULL ) {
		return -1;
	} else {
		user = users;
		while ( user != NULL ) {
			// Grava usuarios
			if ( fwrite( user, sizeof( struct user_t ), 1, fp_user ) != 1 ) {
				fclose( fp_user );
				return -1;
			}
			list = user->list;

			arquivo = convert_id2char( list->id );
			if ( ( fp_list = fopen( arquivo, "w" ) ) == NULL ) {
				return -1;
			}	else {
				while ( list != NULL ) {
					// grava lista de usuarios
					if ( fwrite( list, sizeof( struct list_t ), 1, fp_list ) != 1 ) {
						fclose( fp_list );
						return -1;
					}
					list = list->next;
				}
				fclose( fp_list );
			}
			user = user->next;
		}
		fclose( fp_user );
	}
	return 1;
}


unsigned int user_register( char *nick, char *userPassword, char *fullname, char *email ) {
	struct user_t *user;
	unsigned int user_number = 0;
	
	user = users;
	while ( user != NULL ) {
		user_number++;
		user = user->next;
	}
	if ( user == NULL ) {
		users = user;
	}
	user = alloc_memory_user();
	user->id = user_number;
	strncpy( user->nick, nick, strlen( nick ) );
	strncpy( user->password, userPassword, strlen( userPassword ) );
	strncpy( user->fullname, fullname, strlen( fullname ) );
	strncpy( user->email, nick, strlen( email ) );
	return 1;
}

int remove_user_list( unsigned int userId, unsigned int rm_userId ) {
	struct user_t *user;
	struct list_t *current, *aux;

	user = users;
	while ( user != NULL ) {
		if ( user->id == userId )  {
			aux = user->list;
			current = aux;
			while ( aux != NULL ) {
				if ( aux->id == rm_userId ) {
					current->next = aux->next;
					if ( current == aux ) {
						user->list = current;
					}
					free( aux );
					return 1;
				}
				current = aux;
				aux = aux->next;
			}
		}
		user = user->next;
	}
	return -1;
}

/*
int insert_message( struct message_t *message  ) {
	//list_message - variável global para lista de mensagens
	message = list_message;

	if ( message == NULL ) {
		message = alloc_message_memory();
		//message recebe dados
		list_message = message;
	}
	
	message = alloc_memory_message();
	// message recebe dados
	return 1;
}
*/

/*
 * Adiciona um usuário na lista de contato de um usuário.
 */
int add_user_list( unsigned int userId, int added_userId, char *nick ) {
	struct user_t *user;
	struct list_t *current, *aux;
	user = users;

	while ( user != NULL) {
		if (user->id == userId) {
			current = alloc_memory_list();
			strncpy( current->nick, nick, strlen( nick ) );
			current->id = added_userId;
			if ( user->list == NULL) {
				user->list = current;
			} else {
				aux = user->list;
				while ( aux->next != NULL ) {
					aux = aux->next;
				}
				aux->next = current;
			}
		return 1;
		}
		user = user->next;
	}
	return -1;
}

unsigned int get_list( int userId ) {
	struct user_t *user;

	user = users;
	while ( user != NULL ) {
		if ( user->id == userId ) {
			return 1;
		}
		user = user->next;
	}
	return 0;
}


int user_login ( unsigned int userId, char *userPassword ) {
	struct user_t *user;
	struct message_t *current;
                
	user = users;
	while ( user != NULL ) {
		if ( user->id == userId && strncmp( user->password, userPassword, strlen( user->password ) ) == 0 ) {
			user->status = 1;
			current = list_message;
			while ( current != NULL ) {
				if (current->to  == userId) {
					fifo_put( &messages, current );
					return 1;
				}
        current = current->next;
			}
			return 1;
		}
		user = user->next;
	}
	return -1;
}

int user_logoff( unsigned int userId ) {
        struct user_t *user, *search;
        struct list_t *change;
        struct message_t message;
                                 
                        
        user = users;
        while ( user != NULL ) {
                if ( user->id == userId ) {
                        user->status = 0;
                        free( user->address );
                 
                        search = users;
                        while ( search != NULL ) {
                                change = search->list;
                                while (change != NULL) {
                                        if (change->id == userId) {
                                                message.from = userId;
                                                message.to = search->id;
                                                message.type = 6;
                                                fifo_put(&messages,&message);
                                        }
                                change = change->next;
                                }
                        search = search->next;
                        }
                         // Notifica a alteração de estados para todos os usuários que
                        // incluem este usuário em sua lista  // TODO
                        return 1;
                }
                user = user->next;
        }
                                
        return -1;
}      

/**
 * Atualiza o endereço em que o usuário se encontra na Internet.
 */
unsigned int set_user_address( unsigned int id, struct sockaddr *address ) {
	struct user_t *user;

	user = users;
	while ( user != NULL ) {
		if ( user->id != id ) {
			user = user->next;
		} else {
			free( user->address );
			user->address = address;
			return 1;
		}
	}
	
	return 0;
}

/**
 * Retorna o endereço IP e porta do usuário.
 */
struct sockaddr *get_user_address( unsigned int id ) {
	struct user_t *user;
	user = users;
	while ( user != NULL ) {
		if ( user->id != id ) {
			user = user->next;
		} else {
			return user->address;
		}
	}
	return NULL;
}
						

char *get_list_buffer( int userId) {
	struct user_t *user,*user2;
	struct list_t *current;
	char *buffer = NULL;

	user = users;
	while ( user != NULL ) {
		if (user->id == userId); {
			current = user->list;
			//percorre a lista de usuarios
			while (current != NULL) {
				char *arq = (char *)malloc( sizeof( char ) * USERID );
				sprintf( arq, "%d", current->id );
				strcat(buffer, arq);
				strcat(buffer, current->nick);
				user2 = users;
				// encontra o estado
				while (user2 != NULL) {
					if (user2->id == current->id) {
						char *arq = (char *)malloc( sizeof( char ) * USERID );
						sprintf( arq, "%d", user2->status );
						strcat(buffer, arq);
					}
					user2 = user2->next;		
				}

				current = current->next;
			}
		}
		user = user->next;
	}
	return buffer;
}
