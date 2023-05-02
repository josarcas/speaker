/*
 * sd_spi_device.c
 *
 *  Created on: 17 dic. 2021
 *      Author: JoseCarlos
 */

/*INCLUDES******************************************************************************/
#include "sd_spi_device.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include <sys/dirent.h>
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "driver/spi_master.h"

/*GLOBAL VARIABLES************************************************************************************/

/*PROTOTYPES******************************************************************************************/

/*FUNCTIONS*******************************************************************************************/
/*	@brief	Initializate SPI bus and SD cofiguration.
 *
 */
int init_sd_spi_master()
{
	esp_vfs_fat_sdmmc_mount_config_t mount_config = {
		.format_if_mount_failed = false,
		.max_files = 5,
		.allocation_unit_size = 16 * 1024
	};
	sdmmc_card_t* card;
	const char mount_point[] = STORAGE_MOUNT;

	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	host.max_freq_khz = 20000;

	sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
	slot_config.gpio_cs = SD_CS_PIN;
	slot_config.host_id = host.slot;

	esp_err_t status = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

	return status;
}


/*	@brief	Scan directory of SD storage
 *
 * 	@param	dir_name		Name of directory for scan.
 * 	@param	file_list		Linked list for write name of files and type.
 *
 */
uint16_t scan_directory(char * dir_name, file_linked_list_t ** file_list)
{
	file_linked_list_t *aux = (*file_list);
	file_linked_list_t *aux_prev;

	static struct dirent *dir;
	uint16_t size_dir=0;
	char buffer[3];
	uint8_t i;

	void  *d = opendir(dir_name);

	while(aux != NULL)
	{
		(*file_list) = aux->next;
		vPortFree(aux);
		aux = (*file_list);
	}

	if(d)
	{
		while ((dir = readdir(d)) != NULL)
		{

			if((*file_list) == NULL)
			{
				(*file_list) = aux = (file_linked_list_t *)pvPortMalloc(sizeof(file_linked_list_t));
				aux_prev = NULL;
			}
			else
			{
				aux->next = (file_linked_list_t *)pvPortMalloc(sizeof(file_linked_list_t));
				aux_prev= aux;
				aux = aux->next;
			}
			strcpy(aux->name, dir->d_name);
			aux->next = NULL;
			aux->prev = aux_prev;
			size_dir++;

			for(i=0; i<MAX_LENGH_NAME_FILE; i++)
			{
				if(aux->name[i] == '.')
					break;
			}

			if(i<MAX_LENGH_NAME_FILE)
			{
				buffer[0] = aux->name[i+1];
				buffer[1] = aux->name[i+2];
				buffer[2] = aux->name[i+3];

				if(strcmp("txt", buffer) == 0)
					aux->type = TXT;
				else if(strcmp("bmp", buffer) == 0)
					aux->type = BMP;
				else if(strcmp("wav", buffer) == 0)
					aux->type = WAV;
				else
					aux->type = UNKOWN;
			}
			else
			{
				strcat(aux->name, "/");
				aux->type = FILE_TYPE_FOLDER;
			}

		}
		closedir(d);
	}

	aux->next = (*file_list);
	aux_prev = aux;
	aux = aux->next;
	aux->prev = aux_prev;

	return size_dir;
}


/*	@brief	Free memory used by list file
 *
 * 	@param	file_list		Pointer to linked list for delete.
 * 	@param		 size		Size of list.
 *
 * 	@return	State of operation.
 *
 */
void delete_file_list(file_linked_list_t **file_list, uint8_t size)
{

	file_linked_list_t *aux = (*file_list);
	for(uint8_t i=0; i<size; i++)
	{
		(*file_list) = aux->next;
		vPortFree(aux);
		aux = (*file_list);
	}
}

file_extension_t get_file_ext(const char *file_name)
{
	uint16_t len = strlen(file_name);
	int16_t i;

	for(i=len-1; i>=0; i--)
		if(file_name[i] == '.')
			break;

	char buffer[4];

	buffer[0] = file_name[i+1];
	buffer[1] = file_name[i+2];
	buffer[2] = file_name[i+3];
	buffer[3] = 0;

	if(strcmp("txt", buffer) == 0)
		return TXT;
	else if(strcmp("bmp", buffer) == 0)
		return BMP;
	else if(strcmp("wav", buffer) == 0)
		return WAV;
	else
		return UNKOWN;
}

FILE * fs_open_f(char *file_name, char *mode)
{
	FILE * r_file = NULL;
	char file_dir[MAX_BUFFER_FILE];
	sprintf(file_dir, "%s/%s", STORAGE_MOUNT, file_name);

	r_file = fopen(file_dir, mode);

	return r_file;
}

