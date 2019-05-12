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
  
 ```
 void getLastString(const char *source, char separator, char result[]){
	int pos = -1;
	for (int i = 0; i < strlen(source); i++)
	{
		if (source[i] == separator)
		{
			pos = i;
		}
		
	}

	if (pos>=0){
		strcpy(result, source + pos + 1);
	}
}
 ```
  
GetLastString digunakan untuk mendapatkan string yang paling belakang dengan cara menentukan separatornya dimana dan akan copy string setelah separator ke result
  
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

```
getLastString(path, '/', fname);
	for (int i = 0; i < totalItem; i++){
		if (strcmp(listDirectory[i].filename, fname) == 0){
			res = lstat(listDirectory[i].path, stbuf);
			break;
		}
```
di getattr, last string digunakan untuk get attribute dari list directory

```
static int xmp_open(const char *path, struct fuse_file_info *fi)
{
  	char fpath[1005];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res= -2;

	char fname[1024] = "";
	getLastString(fpath, '/', fname);
	for (int i = 0; i < totalItem; i++){
		if (strcmp(listDirectory[i].filename, fname) == 0){
			res = open(listDirectory[i].path, fi->flags);
			printf("=================OPEN FILE %s\n", listDirectory[i].path);
			break;
		}
	}
	if (res == -2)
		res = open(path, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}
```
Fungsi ini digunakan untuk membuka file yang ada di listdirectory

```
char fname[1024] = "";
	getLastString(fpath, '/', fname);
	for (int i = 0; i < totalItem; i++){
		if (strcmp(listDirectory[i].filename, fname) == 0){
			fd = open(listDirectory[i].path, O_RDONLY);
			printf("=================READ FILE %s\n", listDirectory[i].path);
			break;
		}
	}
```
Pada fungsi read, juga digunakan getlaststring untuk mendapatkan string paling belakang agar bisa diread.

```
void* join()
{	
	listdir(dirpath);

	for (int i = 0; i < totalItem; i++)
	{
		struct stat st;
		memset(&st, 0, sizeof(st));
		stat(listDirectory[i].path, &st);
		printf("%d. %s %s %d\n", i+1, listDirectory[i].filename,listDirectory[i].path,listDirectory[i].totalKembar);
	}
 	return NULL;
}
```
Fungsi ini digunakan untuk menampilkan nama file .mp3 nya di mp3 player.


