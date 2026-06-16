#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#define MAXDEV 16
#define MAXPATH 128
struct usbdevice {
	char path[MAXPATH];
	unsigned long long size;
};
int scan_usb(struct usbdevice usb[]); 								// передаем массив структур
int main(void){
	int choice;
	struct usbdevice dev[MAXDEV]; 								//массив структур
	printf("Добро пожаловать в usbwipe v 1.0\n\n");
	printf("Пожалуйста выбирете действие:\n"
	"1 - начать работу.\n"
	"0 - закончить работу программы.\n");
	scanf("%d", &choice);
	if (choice == 1){
		printf("Отлично. Сканирую USB устройства...\n\n");
		int i = scan_usb(dev);
		printf("\nНайдено %d USB устройств", i);
	}	
	else if (choice == 0){
		printf("Программа завершена.\n");
	}
	/* Пользовательское меню */
	
	return 0;
}

int scan_usb(struct usbdevice usb[]){
	char fullpath[MAXPATH]; 								// буфер для сборки полного пути
	char dirpath[MAXPATH] = "/sys/block/";
	char respath[MAXPATH];
	int dev_count = 0;
	DIR *dir;
	struct dirent *entry; 
	
	dir = opendir(dirpath); 								//открытие директории для работы
	/* Проверка */
	if (dir == NULL) {
	
		perror("Ошибка открытия директории");
		return 1;
	}
	printf("Содержимое дирерктории /sys/block/:\n");
	while ((entry = readdir(dir))!= NULL) 							// чтение директории /sys/block/
	{
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
		{ 
            		continue;								//пропуск каталогов . и ..
        	}
        	snprintf(fullpath, sizeof(fullpath), "%s%s", dirpath, entry -> d_name);		//создание полного пути к устройсвам
        	
		if (realpath(fullpath, respath) != NULL) {
			if (strstr(respath, "usb") == NULL)					//Пропуск устройств не USB
				continue;
			snprintf(usb[dev_count].path, sizeof(usb[dev_count].path), "%s", fullpath);
            		printf("Абсолютный путь: %s\n", respath);
            		dev_count++;	
        	} 
            	else {
			perror("Ошибка realpath");
		}	
	}
	
	
	closedir(dir);
	return dev_count;
	
}
