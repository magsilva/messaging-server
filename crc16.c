#include "crc16.h"

unsigned short get_crc16( char *data, unsigned int size ) {
	register unsigned int i;
	register unsigned short crc = 0;
	
	for ( i  = 0; i < size; i++ ) {
		crc = ( crc << 8 ) ^ crc16_table[ ( ( crc >> 8 ) ^ data[ i ] ) & 0x00FF ];
	}
	return crc;
}
