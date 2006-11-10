#ifndef DATA_H
#define DATA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define USER_DATA_FILENAME "usuarios.text"

#include "common.h"
#include "message.h"
#include "message_type.h"

//Estrutura dos usuarios
struct user_t {
	int id;
	unsigned short status;
	char fullname[ FULLNAME ];
	char nick[ NICK ];
	char password[ PASSWORD ];
	char email[ EMAIL ];
	struct user_t *next;
	struct list_t *list;
	struct sockaddr *address;
};

struct list_t {
	int id;
	char nick[ USERID ];
	struct list_t *next;

};

struct user_t *users;

struct message_t *list_message;

int data_init();

/*
 * Função que aloca memoria, retornando um ponteiro para a estrutura usuario_t.
 */
struct user_t *alloc_memory_user();

/*
 * Função que aloca memória, retornando um ponteiro para a estrutura contato_t.
 */
struct list_t *alloc_memory_list();

/*
 * Função que converte um inteiro em uma String. Usada para gerar o nome do arquivo
 * baseado no numero do usuario. Este arquivo representa a lista dos usuarios.
 */
char *convert_id2char( unsigned int num );

/*
 * Função que carregaga o conteudo do arquivo usuarios.text e das listas de usuarios para
 * a memoria.
 */
int load_users();

/*
 * Função usada por load_users para carregar informações do usuário em memória.
 */
void load_user( struct user_t *user );

/*
 * Função usada por load_user() para obter as listas de contato do usuario.
 */
struct list_t *load_user_list( unsigned int id );

/*
 * Adiciona um usuário na lista de contato de um usuário.
 */
int add_user_list( unsigned int userId, int added_userId, char *nick );

/*
 * Aqui pode ser passado o ponteiro da estrutura dos usuarios como parametro.
 */
int save_user();

int user_login( unsigned int userId, char *senha );

unsigned int user_register( char *nick, char *senha, char *fullname, char *email );

int remove_user_list( unsigned int userId, unsigned int rm_userId );

int set_user_status( unsigned int userId, unsigned short status );

struct sockaddr *get_user_address( unsigned int userId );

unsigned int get_list( int userId );

int user_logoff( unsigned int userId );

unsigned int set_user_address( unsigned int id, struct sockaddr *address );

struct sockaddr *get_user_address( unsigned int id );

char *get_list_buffer ( int userId);

#endif

