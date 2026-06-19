#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define BLOCKSIZE 1048576
#define MAXNAME 64
#define MAXDEV 16
#define MAXPATH 128

struct usbdevice {
	char vendor[MAXNAME]; 
	char model[MAXNAME];
	char path[MAXPATH];
	unsigned long long size;
};

int scan_usb(struct usbdevice usb[]); 								// передаем массив структур
unsigned long long get_usb_size(char spath[]);
void get_usb_vendor(const char usbnamepath[], char usbname[], size_t max_len);
void get_usb_model(const char usbmodelpath[], char usbmodel[], size_t max_len);
void wipe(struct usbdevice *usb);

int main(void) {
	int choice;
	int count = 0;
	int i;
	struct usbdevice dev[MAXDEV]; 
	int dev_choice = -1;
	struct usbdevice *selected_dev = NULL;
    
	printf("Добро пожаловать в usbwipe v 1.0\n\n");
    
	while (1) {
		printf("\nПожалуйста, выберите действие:\n"
		"1 - Начать сканирование USB.\n"
		"0 - Выйти из программы.\n"
		"> ");
               
		if (scanf("%d", &choice) != 1) {
			printf("Ошибка: Введите число 1 или 0.\n");
			while (getchar() != '\n');
			continue;
		}
		
		if (choice == 0) {
			printf("Программа завершена.\n");
			break;
		}
		
		if (choice == 1) {
			printf("\nОтлично. Сканирую USB устройства...\n");
			count = scan_usb(dev);

			printf("Количество найденных USB устройств: %d\n", count);

			if (count == 0) {
				printf("USB устройств не найдено. Попробуйте еще раз\n");
				continue;
		    	}
		    
			for (i = 0; i < count; i++) {
				printf("\n[%d] %s %s\n"
				"USB path:   %s\n"
				"USB size:   %llu Байт\n", 
				i + 1, dev[i].vendor, dev[i].model, dev[i].path, dev[i].size);
			}
		    
			while (1) {
				printf("\nВведите номер устройства (или 0 для возврата в меню): ");
		        
		        	if (scanf("%d", &dev_choice) != 1) {
					printf("Ошибка: Введите число (номер устройства).\n");
					while (getchar() != '\n');
					continue;
		        	}
		        
				if (dev_choice == 0) {
				    	break;
				}
		        
				if (dev_choice < 1 || dev_choice > count) {
				    	printf("Неверный номер устройства. Выберите от 1 до %d.\n", count);
				} 
				else {
					selected_dev = &dev[dev_choice - 1];
					
					printf("\nВНИМАНИЕ! Вы выбрали: %s %s (%s)\n", selected_dev->vendor, 
						selected_dev->model, selected_dev->path);
					printf("Все данные на этом устройстве будут УНИЧТОЖЕНЫ.\n");
					printf("Вы уверены? (1 - Да, Очистить / 0 - Отмена):");
					
					int confirm = 0;
					if (scanf("%d", &confirm) == 1 && confirm == 1) {
						wipe(selected_dev);
					} else {
						printf("Очистка отменена.\n");
					}
					break;
				}
			}
		}
    	}
    
	return 0;
}


int scan_usb(struct usbdevice usb[]){
	char fullpath[MAXPATH]; 								
	char vendorpath[MAXPATH];
	char modelpath[MAXPATH];
	char usbname[MAXNAME];
	char usbmodel[MAXNAME];
	char dirpath[MAXPATH] = "/sys/block/";
	char respath[MAXPATH];
	char sizepath[MAXPATH];
	unsigned long long usbsize;
	int dev_count = 0;
	DIR *dir;
	struct dirent *entry; 
	
	dir = opendir(dirpath); 								//открытие директории для работы
	/* Проверка */
	if (dir == NULL) {
	
		perror("Ошибка открытия директории");
		return 1;
	}
	//printf("Содержимое дирерктории /sys/block/:\n");
	while ((entry = readdir(dir))!= NULL) 							// чтение директории /sys/block/
	{
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
		{ 
            		continue;								//пропуск каталогов . и ..
        	}
        	if (strcmp(entry->d_name, "loop") == 0 || strcmp(entry->d_name, "ram") == 0) {
			continue;
		}
        	snprintf(fullpath, sizeof(fullpath), "%s%s", dirpath, entry -> d_name);		//создание полного пути к устройсвам
		if (realpath(fullpath, respath) != NULL) {
			if (strstr(respath, "usb") == NULL)					//Пропуск устройств не USB
				continue;
			
			snprintf(vendorpath, sizeof(vendorpath), "%s%s", fullpath,"/device/vendor"); 
			snprintf(modelpath, sizeof(modelpath), "%s%s", fullpath,"/device/model"); 
			snprintf(sizepath, sizeof(sizepath), "%s%s", fullpath, "/size");
        		//printf("\nsizepath: %s\n", sizepath);
        		//printf("vendorpath: %s\n", vendorpath);				проверки
        		//printf("modelpath: %s\n", modelpath);
        		
        		get_usb_vendor(vendorpath, usbname, sizeof(usbname));
        		get_usb_model(modelpath, usbmodel, sizeof(usbmodel));
        		snprintf(usb[dev_count].vendor, sizeof(usb[dev_count].vendor), "%s", usbname);
        		snprintf(usb[dev_count].model, sizeof(usb[dev_count].model), "%s", usbmodel);
			snprintf(usb[dev_count].path, sizeof(usb[dev_count].path), "/dev/%s", entry -> d_name);	//формирование пути для wipe()
			usb[dev_count].size = get_usb_size(sizepath);
			
			//printf("\nUSBNAME = %s\n", usbname);
        		//printf("USBMODEL = %s\n", usbmodel);
        		//printf("USBSIZE = %llu байт\n", get_usb_size(sizepath));		проверки
            		//printf("Абсолютный путь: %s\n", respath);
            		dev_count++;	
        	} 
            	else {
			perror("Ошибка realpath");
		}	
	}
	closedir(dir);
	return dev_count;
	
}
unsigned long long get_usb_size(char spath[]){
	unsigned long long sectors;
	FILE *fp;
	
	if ((fp = fopen(spath, "r")) == NULL){
		fprintf(stderr, "Не удается открыть %s\n", spath);
		exit(EXIT_FAILURE);
	}
	if (fscanf(fp, "%llu", &sectors) != 1) {
		fprintf(stderr, "Ошибка: не удалось прочесть число.\n");
		fclose(fp); 
		return 0; 
	}
	
	fclose(fp);
	
	return sectors * 512;
}
    
void get_usb_vendor(const char usbnamepath[], char usbname[], size_t max_len){
	FILE *fp;
	if ((fp = fopen(usbnamepath, "r")) == NULL){
		snprintf(usbname, max_len, "Generic"); 
		return;
	} 
	if (fgets(usbname, max_len, fp) != NULL){
		size_t len = strlen(usbname);
		if (len > 0 && usbname[len-1] == '\n'){
			usbname[len-1] = '\0';
		}
	}
	else{
		snprintf(usbname, max_len, "Generic");
	}

	fclose(fp);
}

void get_usb_model(const char usbmodelpath[], char usbmodel[], size_t max_len){
	FILE *fp;
	if ((fp = fopen(usbmodelpath, "r")) == NULL){
		snprintf(usbmodel, max_len, "USB_Drive");
		return;
	} 
	if (fgets(usbmodel, max_len, fp) != NULL){
		size_t len = strlen(usbmodel);
		if (len > 0 && usbmodel[len-1] == '\n'){
			usbmodel[len-1] = '\0';
		}
	}
	else{
		snprintf(usbmodel, max_len, "USB_Drive");
	}

	fclose(fp);
}

void wipe(struct usbdevice *usb){
	printf("\nЗапускаю очистку данных с устройства %s %s ...\n", usb -> vendor, usb -> model);
	
	int fd = open(usb->path, O_WRONLY);					// открытие устройства для записи
	if (fd == -1){
		fprintf(stderr, "Ошибка открытия %s\n", usb->path);
		fprintf(stderr, "Запустите программу с правами суперпользователя!\n");
		return;
	}
	
	unsigned char *buffer = malloc(BLOCKSIZE);
	if (!buffer) {
		perror("Ошибка выделения памяти для буфера");			// подготовка буфера 
		close(fd);
		return;
	}
	memset(buffer, 0x00, BLOCKSIZE);					
	
	unsigned long long total_bytes_written = 0;
	unsigned long long last_reported_bytes = 0;				
	ssize_t bytes_written;							
	
	printf("Обработано: 0 МБ / %llu МБ", usb->size / (1024 * 1024));
	fflush(stdout);

	while (total_bytes_written < usb->size) {
        
		size_t to_write = BLOCKSIZE;
		if (usb->size - total_bytes_written < BLOCKSIZE) {  		// сколько байт осталось до конца флешки
			to_write = usb->size - total_bytes_written;
		}

		bytes_written = write(fd, buffer, to_write);
        
		if (bytes_written <= 0) {
			if (errno == ENOSPC) { 					// ENOSPC - нет места на диске
				break; 
			}
			perror("\nКритическая ошибка при записи");
			break;
		}

		total_bytes_written += bytes_written;

		if (total_bytes_written - last_reported_bytes >= (100 * 1024 * 1024) || total_bytes_written == usb->size) {
			last_reported_bytes = total_bytes_written;

			printf("\rОбработано: %llu МБ / %llu МБ", 
			       total_bytes_written / (1024 * 1024), 
			       usb->size / (1024 * 1024));
			fflush(stdout);
		}
	}
	printf("\nВНИМАНИЕ!!! НЕ ДОСТАВАЙТЕ USB УСТРОЙСТВО! РАБОТАЕТ FSYNC()...\n");
	if (fsync(fd) == -1) {
		perror("Ошибка выполнения fsync");
	} else {
		printf("Успешно!\n");
	}

	printf("Очистка завершена. Уничтожено: %llu байт.\n", total_bytes_written);

	free(buffer);
	close(fd);
}
