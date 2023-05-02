/*
 * sd_spi_device.h
 *
 *  Created on: 17 dic. 2021
 *      Author: JoseCarlos
 */

#ifndef MAIN_SD_SPI_DEVICE_H_
#define MAIN_SD_SPI_DEVICE_H_


/*INCLUDES*******************************************************************************/
#include "stdint.h"
#include "string.h"
#include "stdbool.h"
#include "stdio.h"

/*DEFINES********************************************************************************/
#define STORAGE_MOUNT					"/sd"
//#define INIT_FILE_CONFIG_NAME 			"/sd/config.txt"
//#define SPI_DMA_CHAN    				2
#define MAX_LENGH_NAME_FILE 			50

#ifndef	TYPE_FILE
#define FILE_TYPE_UNKOWN	0
#define FILE_TYPE_FOLDER	1
#define FILE_TYPE_TXT		10
#define FILE_TYPE_BMP		11
#define FILE_TYPE_WAV		12
#endif

#ifndef SPI_BUS
#define SPI_BUS
#define SYS_SPI_HOST				SPI2_HOST
#define SPI_MOSI_PIN				23
#define SPI_MISO_PIN				38
#define SPI_CLK_PIN					18
#endif
#define SD_CS_PIN   				4

#ifndef MAX_BUFFER_FILE
#define MAX_BUFFER_FILE			512
#endif

/*TYPEDEF********************************************************************************/
typedef enum{
	UNKOWN	=0,
	FOLDER,
	TXT,
	BMP,
	WAV
}file_extension_t;

struct file_linked_list{
#ifndef MAX_LENGH_NAME_FILE
#define MAX_LENGH_NAME_FILE 256
#endif
	char name[MAX_LENGH_NAME_FILE];
	file_extension_t type;
	struct file_linked_list *next;
	struct file_linked_list *prev;
};
typedef struct file_linked_list file_linked_list_t;


/*PROTOTYPES*****************************************************************************/
int init_sd_spi_master();
uint16_t scan_directory(char * dir_name, file_linked_list_t ** file_list);
void delete_file_list(file_linked_list_t **file_list, uint8_t size);
file_extension_t get_file_ext(const char *file_name);

FILE * fs_open_f(char *file_name, char *mode);


#endif /* MAIN_SD_SPI_DEVICE_H_ */
