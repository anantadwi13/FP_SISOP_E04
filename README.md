# FP_SISOP_E04

### Dependencies
- libao v1.1.0
- mpg123 v1.13.8
- FUSE

### How To Compile
``` bash
gcc -Wall `pkg-config fuse --cflags` mp3withfuse.c -o mp3withfuse `pkg-config fuse --libs` -lmpg123 -lao -pthread
```

### Penjelasan FUSE
  Pertama, tuliskan dirpath yang nantinya akan menjadi folder source untuk FUSE. Kemudian dibuat struct directory yang berisi path, filename, dan totalkembar ke dalam array listdirectory. 
  
  GetLastString
  
```  
void listdir(const char *name)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name))) return;

    while ((entry = readdir(dir)) != NULL) 
	{
        if (entry->d_type == DT_DIR) 
		{
            char path[1005];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            listdir(path);

        }

		else 
		{
            int ext = strlen(entry->d_name);
            if(entry->d_name[ext - 1] == '3' && entry->d_name[ext - 2] == 'p' && entry->d_name[ext - 3] == 'm' && entry->d_name[ext - 4] == '.')
			{        
                char temp[1005];
				int kembarKe = 0;
                
                strcpy(temp,name);
                strcat(temp,"/");
                strcat(temp,entry->d_name);

				for (int i = 0; i < totalItem; i++){
					if (strcmp(listDirectory[i].filename, entry->d_name) == 0){
						listDirectory[i].totalKembar++;
						kembarKe = listDirectory[i].totalKembar;
						break;
					}
				}
				
				sprintf(listDirectory[totalItem].path, "%s", temp);
				if (kembarKe > 0){
					strcpy(temp, entry->d_name + ext - 3);
					entry->d_name[ext - 4] = '\0';
					sprintf(listDirectory[totalItem].filename, "%s (%d).%s", entry->d_name, kembarKe, temp);
				}
				else
					sprintf(listDirectory[totalItem].filename, "%s", entry->d_name);
				listDirectory[totalItem].totalKembar = 0;
				totalItem++;
            }
        }
    }
    closedir(dir);
}
```

Kemudian dibuat fungsi listdir untuk mendapatkan list directory yang ada. Jika merupakan folder maka akan di snprintf name dan entry->d_name kemudian dimasukkan di dalam variabel path dan akan di listdir lagi. Jika merupakan file, maka dicek apakah extensionnya merupakan .mp3 atau tidak. Jika merupakan file .mp3, maka akan dicopy name nya ke temp dan akan digabungkan dengan / dan entry->d_name. Setelah itu akan dilakukan pengecekan apakah ada file yang sama. Jika ada, maka totalkembarnya akan ditambah dan namanya akan diberi tambahan angka untuk membedakan antara satu file dengan yang lain. Kemudian totalkembarnya di set 0 lagi.
