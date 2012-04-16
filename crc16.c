/*
 * crc16.c - generate a ccitt 16 bit cyclic redundancy check (crc) for a
 * block of data.
 *
 *
 *                                16  12  5
 * The CCITT CRC 16 polynomial is X + X + X + 1.
 * In binary, this is the bit pattern 1 0001 0000 0010 0001, and in hex it
 * is 0x11021.
 *
 * A 17 bit register is simulated by testing the MSB before shifting
 * the data, which affords us the luxury of specifiy the polynomial as a
 * 16 bit value, 0x1021.
 * Due to the way in which we process the CRC, the bits of the polynomial
 * are stored in reverse order. This makes the polynomial 0x8408.
 */

#include "crc16.h"

#define CRC16_CCITT_POLY 0x8408


unsigned short crc16_table( char *data, unsigned int size )
{
	register unsigned int i;
	register unsigned short crc = 0;
	
	for ( i  = 0; i < size; i++ ) {
		crc = ( crc << 8 ) ^ crc16_table[ ( ( crc >> 8 ) ^ data[ i ] ) & 0x00FF ];
	}
	return crc;
}

/*
 * Generate a 16 bit crc. This routine generates the 16 bit remainder of a
 * block of data using the ccitt polynomial generator.
 *
 * The CRC is preset to all 1's to detect errors involving a loss of leading
 * zero's. The CRC (a 16 bit value) is generated in LSB MSB order. Calculate
 * the crc on the data, and compare it to the crc calculated previously. The
 * location of the saved crc must be known.
 * 
 * @param data Address of start of data block.
 * @param len Length of data block.
 * @return CRC6 value. Data is calcuated using the 16 bit ccitt polynomial.
 *
 */
unsigned short crc16_poly(char *data_p, unsigned short length)
{
       unsigned char i;
       unsigned int data;
       unsigned int crc;
       
       crc = 0xffff;
       
       if (length == 0)
              return (~crc);
       
       do {
              for (i = 0, data = (unsigned int)0xff & *data_p++;
                  i < 8;
                  i++, data >>= 1) {
                    if ((crc & 0x0001) ^ (data & 0x0001))
                           crc = (crc >> 1) ^ CRC16_CCITT_POLY;
                    else
                           crc >>= 1;
              }
       } while (--length);
       
       crc = ~crc;
       
       data = crc;
       crc = (crc << 8) | (data >> 8 & 0xFF);
       
       return (crc);
}

