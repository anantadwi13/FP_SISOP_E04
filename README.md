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

Kemudian sekarang beralih ke mp3 playernya
```
ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = mpg123_outblock(mh);
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));
```
Digunakan untuk initialization dari mp3 playernya.

```
mpg123_open(mh, filePath);
    mpg123_getformat(mh, &rate, &channels, &encoding);
```
Digunakan untuk membuka file dan decoding format

```
 format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);
```
Digunakan untuk set output format dan membuka output device

```
while (1){
        if (mp3_pause==1){
            sleep(1);
            continue;
        }
        if (mp3_play==0) {
            break;
        }

        if (mp3_seek<0) {
            mpg123_seek_frame(mh, mp3_seek, SEEK_CUR);
            mp3_seek = 0;
        }
        else if (mp3_seek>0){
            mpg123_seek_frame(mh, mp3_seek, SEEK_CUR);
            mp3_seek = 0;
        }
        
        
        if(mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
            ao_play(dev, buffer, done);
        else
            break;
        
        //printf("%d %d %d\n", (int) buffer, (int) buffer_size, (int) done);
```
Digunakan untuk decode dan juga untuk memainkan mp3nya

```
free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();

    if (mp3_play==0) {
        return NULL;
    }
    
    select_mp3++;
	DIR *dp;
	struct dirent *de;
    int no = 1;
    dp = opendir(mountpoint);
    while ((de = readdir(dp)) != NULL) {
        int len = strlen(de->d_name);
        char fn[1024];
        sprintf(fn, "%s", de->d_name);
        
        if (fn[len-4] == '.' && fn[len-3] == 'm' && fn[len-2] == 'p' && fn[len-1] == '3') {
            if (no == select_mp3) {
                sprintf(now_playing, "%s", de->d_name);
                no++;
                break;
            }
            no++;
        }
    }
    if (select_mp3<1 || select_mp3>=no){
        select_mp3 = -1;
    }
    else{
        mp3_play = 0;
        sleep(1);
        mp3_play = 1;
        mp3_pause = 0;
        mp3_seek = 0;
        pthread_create(&tid[1],NULL,&player,now_playing);
    }
    system("clear");
    if (select_mp3 > 0)
        printf("Now Playing : %s\n\n", now_playing);
    
    printf("Help :\nopen to open music\nstop to stop music\n   p to play/pause music\n   , to rewind music\n   . to forward music\nprev to play previous music\nnext to play next music\nexit to exit mp3player\n\n");
    printf("Command : \n");
    return NULL;
```
Digunakan untuk membersihkan mp3 setelah selesai dan juga untuk menampilkan menu pada saat di mp3 player

Untuk fungsi mainnya, pertama dijalankan dulu FUSEnya, kemudian baru menjalankan mp3 player. Sebagian besar di main hanya merupakan user interface untuk mp3 playernya.
```
int main(int argc, char *argv[])
{
	DIR *dp;
	struct dirent *de;
    struct stat st_mountpoint = {0};

    if (argc!=2){
        printf("Usage : %s /path/mountpoint_directory\n", argv[0]);
        return 0;
    }


    if (stat(argv[1], &st_mountpoint) == -1) {
        mkdir(argv[1], 0777);
    }
    
    
    if (fork() == 0)
    {
        umask(0);
        fuse_main(argc, argv, &xmp_oper, NULL);
    }

    strcpy(mountpoint,argv[1]);
    
    //system("clear");
    while(1){
        if (select_mp3 > 0)
            printf("Now Playing : %s\n\n", now_playing);
        
        printf("Help :\nopen to open music\nstop to stop music\n   p to play/pause music\n   , to rewind music\n   . to forward music\nprev to play previous music\nnext to play next music\nexit to exit mp3player\n\n");
        printf("Command : \n");
        scanf("%s", type);
        system("clear");
        if (strcmp(type,"p")==0) {
            if (mp3_pause==1)
                mp3_pause = 0;
            else
                mp3_pause = 1;
        }
        else if (strcmp(type,"open")==0){
            printf("PlayList :\n");
            int no = 1;
            dp = opendir(mountpoint);
            while ((de = readdir(dp)) != NULL) {
                int len = strlen(de->d_name);
                char fn[1024];
                sprintf(fn, "%s", de->d_name);
                
                if (fn[len-4] == '.' && fn[len-3] == 'm' && fn[len-2] == 'p' && fn[len-1] == '3') {
                    printf("%3d. %s\n", no++, fn);
                }
            }

            printf("\n\nPutar nomor : ");
            int selected_before = select_mp3;
            scanf("%d", &select_mp3);
            if (select_mp3<1 || select_mp3 >= no) {
                printf("Lagu tidak ditemukan!\n");
                select_mp3 = selected_before;
                sleep(2);
                system("clear");
                continue;
            }
            

            no=1;
            dp = opendir(mountpoint);
            while ((de = readdir(dp)) != NULL) {
                int len = strlen(de->d_name);
                char fn[1024];
                sprintf(fn, "%s", de->d_name);
                
                if (fn[len-4] == '.' && fn[len-3] == 'm' && fn[len-2] == 'p' && fn[len-1] == '3') {
                    if (no == select_mp3) {
                        sprintf(now_playing, "%s", de->d_name);
                        break;
                    }
                    no++;
                }
            }

            printf("Opening %s\n", now_playing);
            mp3_play = 0;
            sleep(1);
            mp3_play = 1;
            mp3_pause = 0;
            mp3_seek = 0;
            pthread_create(&tid[1],NULL,&player,now_playing);
            system("clear");
        }
        else if (strcmp(type,"next")==0){
            select_mp3++;
            int no = 1;
            dp = opendir(mountpoint);
            while ((de = readdir(dp)) != NULL) {
                int len = strlen(de->d_name);
                char fn[1024];
                sprintf(fn, "%s", de->d_name);
                
                if (fn[len-4] == '.' && fn[len-3] == 'm' && fn[len-2] == 'p' && fn[len-1] == '3') {
                    if (no == select_mp3) {
                        sprintf(now_playing, "%s", de->d_name);
                        no++;
                        break;
                    }
                    no++;
                }
            }
            if (select_mp3<1 || select_mp3>=no){
                printf("Lagu tidak ditemukan!\n");
                select_mp3--;
                sleep(2);
                system("clear");
                continue;
            }

            printf("Opening %s\n", now_playing);
            mp3_play = 0;
            sleep(1);
            mp3_play = 1;
            mp3_pause = 0;
            mp3_seek = 0;
            pthread_create(&tid[1],NULL,&player,now_playing);
            system("clear");
        }
        else if (strcmp(type,"prev")==0){
            select_mp3--;
            int no = 1;
            dp = opendir(mountpoint);
            while ((de = readdir(dp)) != NULL) {
                int len = strlen(de->d_name);
                char fn[1024];
                sprintf(fn, "%s", de->d_name);
                
                if (fn[len-4] == '.' && fn[len-3] == 'm' && fn[len-2] == 'p' && fn[len-1] == '3') {
                    if (no == select_mp3) {
                        sprintf(now_playing, "%s", de->d_name);
                        no++;
                        break;
                    }
                    no++;
                }
            }
            if (select_mp3<1 || select_mp3>=no){
                printf("Lagu tidak ditemukan!\n");
                select_mp3++;
                sleep(2);
                system("clear");
                continue;
            }

            printf("Opening %s\n", now_playing);
            mp3_play = 0;
            sleep(1);
            mp3_play = 1;
            mp3_pause = 0;
            mp3_seek = 0;
            pthread_create(&tid[1],NULL,&player,now_playing);
            system("clear");
        }
        else if (strcmp(type,",")==0)
            mp3_seek -= 200;
        else if (strcmp(type,".")==0)
            mp3_seek += 200;
        else if (strcmp(type,"exit")==0){
            mp3_play = 0;
            printf("Closing mp3player\n");
            sleep(2);
            fuse_unmount(mountpoint, NULL);
            break;
        }
        else if (strcmp(type,"stop")==0){
            mp3_play = 0;
            printf("Stop playing %s\n", now_playing);
            select_mp3 = -1;
            sleep(1);
            system("clear");
        }
    }
}
```
