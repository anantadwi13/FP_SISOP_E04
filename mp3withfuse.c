#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <fcntl.h>
#include <ao/ao.h>
#include <mpg123.h>
#include <stdio.h>
#include<pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#define BITS 8

//-------------------------------- START FUSE --------------------------------
static const char *dirpath = "/home/anantadwi13/SISOP/fp/mp3";

struct Directory {
	char path[1024];
	char filename[1024];
	int totalKembar;
};


struct Directory listDirectory[1024];

int totalItem = 0;

pthread_t tid[3]; // 1 -> mp3player       2 -> joinfile

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

static int xmp_getattr(const char *path, struct stat *stbuf)
{
  	int res=-2;
	char fname[1024] = "";
	getLastString(path, '/', fname);
	for (int i = 0; i < totalItem; i++){
		if (strcmp(listDirectory[i].filename, fname) == 0){
			res = lstat(listDirectory[i].path, stbuf);
			break;
		}
	}

	if (res == -2)
	{
		char fpath[1000];
		sprintf(fpath,"%s%s",dirpath,path);
		res = lstat(fpath, stbuf);
	}
	

	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	int res = 0;
	(void) offset;
	(void) fi;

	for (int i=0; i<totalItem; i++){
		struct stat st;
		memset(&st, 0, sizeof(st));
		//stat(listDirectory[i].path, &st);

		res = (filler(buf, listDirectory[i].filename, &st, 0));
		if(res!=0) break;
	}
	return 0;
}

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

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
  	char fpath[1005];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);

	int res = 0;
  	int fd = 0 ;
	
	char fname[1024] = "";
	getLastString(fpath, '/', fname);
	for (int i = 0; i < totalItem; i++){
		if (strcmp(listDirectory[i].filename, fname) == 0){
			fd = open(listDirectory[i].path, O_RDONLY);
			printf("=================READ FILE %s\n", listDirectory[i].path);
			break;
		}
	}
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

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

static void* xmp_init(struct fuse_conn_info *conn)
{
	pthread_create(&tid[2],NULL,&join,NULL);

	return NULL;
}

void xmp_destroy()
{

}

static struct fuse_operations xmp_oper = {
	.init		= xmp_init,
    .getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
	.open		= xmp_open,
	.destroy	= xmp_destroy,
	
};

//-------------------------------- END OF FUSE --------------------------------

struct PlayList
{
    char name[1024];
    int listMusic[1024];
    int totalMusic;
    int deleted;
};

#define MAX_PLAYLIST 100

char type[100];
int mp3_pause=0;
int mp3_play=0;
int mp3_seek=0; // 0 normal, negative : backward, positive : forward
int select_mp3 = -1;
char now_playing[1024] = "";

char mountpoint[1024] = "";

int playListNow = 0; // 0 -> no playlist, using normal playlist
int totalPlayList = 0;

struct PlayList playListMusic[MAX_PLAYLIST + 1];

void putarPlayList();
void createPlayList();
void *player(void *arg);

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
    
    system("clear");
    while(1){
        if (playListNow ==0 && select_mp3 > 0)
            printf("Now Playing : %s\n\n", now_playing);
        else if(playListNow > 0 && playListNow <= MAX_PLAYLIST && playListMusic[playListNow].listMusic[select_mp3]>0)
            printf("PlayList %s\nNow Playing : %s\n\n", playListMusic[playListNow].name, now_playing);
        
        printf("Help :\nopen to open music\n apl to add new playlist\n ppl to select playlist\nstop to stop music\n   p to play/pause music\n   , to rewind music\n   . to forward music\nprev to play previous music\nnext to play next music\nexit to exit mp3player\n\n");
        printf("Command : \n");
        scanf("%s", type);
        system("clear");
        if (strcmp(type,"p")==0) {
            if (mp3_pause==1)
                mp3_pause = 0;
            else
                mp3_pause = 1;
        }
        else if (strcmp(type, "apl")==0)
        {
            createPlayList();
        }
        else if (strcmp(type, "ppl")==0)
        {
            putarPlayList();
        }
        
        else if (strcmp(type,"open")==0){
            printf("List Lagu :\n");
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
            playListNow = 0;
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
            int select_idx_mp3 = -1;
            if (playListNow == 0)
                select_idx_mp3 = select_mp3;
            else{
                if (select_mp3 < 0 || select_mp3 >= 1024)
                    select_idx_mp3 = -1;
                else
                    select_idx_mp3 = playListMusic[playListNow].listMusic[select_mp3];
            }

            int no = 1;
            dp = opendir(mountpoint);
            while ((de = readdir(dp)) != NULL) {
                int len = strlen(de->d_name);
                char fn[1024];
                sprintf(fn, "%s", de->d_name);
                
                if (fn[len-4] == '.' && fn[len-3] == 'm' && fn[len-2] == 'p' && fn[len-1] == '3') {
                    if (no == select_idx_mp3) {
                        sprintf(now_playing, "%s", de->d_name);
                        no++;
                        break;
                    }
                    no++;
                }
            }
            if (select_idx_mp3<1 || select_idx_mp3>=no){
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
            int select_idx_mp3 = -1;
            if (playListNow == 0)
                select_idx_mp3 = select_mp3;
            else{
                if (select_mp3 < 0 || select_mp3 >= 1024)
                    select_idx_mp3 = -1;
                else
                    select_idx_mp3 = playListMusic[playListNow].listMusic[select_mp3];
            }

            int no = 1;
            dp = opendir(mountpoint);
            while ((de = readdir(dp)) != NULL) {
                int len = strlen(de->d_name);
                char fn[1024];
                sprintf(fn, "%s", de->d_name);
                
                if (fn[len-4] == '.' && fn[len-3] == 'm' && fn[len-2] == 'p' && fn[len-1] == '3') {
                    if (no == select_idx_mp3) {
                        sprintf(now_playing, "%s", de->d_name);
                        no++;
                        break;
                    }
                    no++;
                }
            }
            if (select_idx_mp3<1 || select_idx_mp3>=no){
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

void putarPlayList(){
    struct Directory listMusic[1024];
    int totalMusic = 0;
    DIR *dp;
	struct dirent *de;
    int mapPlayList[101], no=1, select = 0;
    memset(mapPlayList, 0 , sizeof(mapPlayList));

    printf("Daftar PlayList :\n");
    for (int i = 1; i <= totalPlayList; i++)
    {
        if (playListMusic[i].deleted == 0){
            printf("%3d. %s\n", no++, playListMusic[i].name);
            mapPlayList[no-1] = i;
        }
    }
    printf("\nPilih PlayList : ");
    scanf("%d", &select);
    if(select<1 || select>=no){
        printf("PlayList Tidak Ditemukan!\n");    
        sleep(1);
        system("clear");
        return;
    }
    playListNow = mapPlayList[select];

    dp = opendir(mountpoint);
    memset(listMusic, 0, sizeof(listMusic));
    while ((de = readdir(dp)) != NULL) {
        sprintf(listMusic[totalMusic+1].filename, "%s", de->d_name);
        totalMusic++;
    }
    closedir(dp);

    printf("Opening PlayList %s\n", playListMusic[playListNow].name);
    int idxLagu = playListMusic[playListNow].listMusic[0];
    sprintf(now_playing, "%s", listMusic[idxLagu].filename);
    select_mp3 = 0;

    printf("Playing %s\n", now_playing);
    mp3_play = 0;
    sleep(1);
    mp3_play = 1;
    mp3_pause = 0;
    mp3_seek = 0;

    pthread_create(&tid[1],NULL,&player,now_playing);
    system("clear");
}

void createPlayList(){
    struct Directory listMusic[1024];
    int totalMusic = 0;
    DIR *dp;
	struct dirent *de;
    int idxPlayList = totalPlayList+1;
    memset(playListMusic[idxPlayList].listMusic, 0, sizeof(playListMusic[idxPlayList].listMusic));
    playListMusic[idxPlayList].totalMusic = 0;
    playListMusic[idxPlayList].deleted = 0;
    int idxLagu = playListMusic[idxPlayList].totalMusic;

    dp = opendir(mountpoint);
    memset(listMusic, 0, sizeof(listMusic));
    while ((de = readdir(dp)) != NULL) {
        sprintf(listMusic[totalMusic+1].filename, "%s", de->d_name);
        totalMusic++;
    }
    closedir(dp);
    
    printf("Nama PlayList : ");
    getchar();
    scanf("%[^\n]s", playListMusic[idxPlayList].name);

    while (1)
    {
        system("clear");
        int temp = 0;

        printf("TAMBAH LAGU PLAYLIST\nList Lagu :\n");
        for (int i = 1; i <= totalMusic; i++)
        {
            int len = strlen(listMusic[i].filename);
            char fn[1024];
            sprintf(fn, "%s", listMusic[i].filename);
            if (fn[len-4] == '.' && fn[len-3] == 'm' && fn[len-2] == 'p' && fn[len-1] == '3') {
                printf("%3d. %s\n", i, fn);
            }
        }
        printf("\n\nTambahkan lagu nomor (Ketik 0 utk selesai menambahkan) : ");
        scanf("%d", &temp);
        if (temp == 0)
        {
            char choice;
            system("clear");
            if (playListMusic[idxPlayList].totalMusic == 0)
                return;
            printf("Nama PlayList : %s\n\nList Lagu :\n", playListMusic[idxPlayList].name);
            for (int i = 0; i < playListMusic[idxPlayList].totalMusic; i++)
            {
                int noMusic = playListMusic[idxPlayList].listMusic[i];
                printf("%3d. %s\n", i+1, listMusic[noMusic].filename);
            }
            printf("Simpan PlayList? (y/n)\n");
            getchar();
            scanf("%c", &choice);
            if (choice == 'y' || choice == 'Y')
            {
                totalPlayList++;
            }
            system("clear");
            return;
        }
        else if (temp<1 || temp > totalMusic) {
            printf("Lagu tidak ditemukan!\n");
            sleep(2);
            system("clear");
            continue;
        }

        playListMusic[idxPlayList].listMusic[idxLagu] = temp;
        idxLagu++;
        playListMusic[idxPlayList].totalMusic = idxLagu;
    }
}

void *player(void *arg){
    mpg123_handle *mh;
    unsigned char *buffer;
    size_t buffer_size;
    size_t done;
    int err;

    int driver;
    ao_device *dev;

    ao_sample_format format;
    int channels, encoding;
    long rate;

    /* initializations */
    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = mpg123_outblock(mh);
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

    char* filename =  arg; 
    char filePath[1024];
    sprintf(filePath,"%s/%s", mountpoint, filename);

    /* open the file and get the decoding format */
    mpg123_open(mh, filePath);
    mpg123_getformat(mh, &rate, &channels, &encoding);

    /* set the output format and open the output device */
    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);

    /* decode and play */
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
    }

    /* clean up */
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
    int select_idx_mp3 = -1;
    if (playListNow == 0)
        select_idx_mp3 = select_mp3;
    else{
        if (select_mp3 < 0 || select_mp3 >= 1024)
            select_idx_mp3 = -1;
        else
            select_idx_mp3 = playListMusic[playListNow].listMusic[select_mp3];
    }

	DIR *dp;
	struct dirent *de;
    int no = 1;
    dp = opendir(mountpoint);
    while ((de = readdir(dp)) != NULL) {
        int len = strlen(de->d_name);
        char fn[1024];
        sprintf(fn, "%s", de->d_name);
        
        if (fn[len-4] == '.' && fn[len-3] == 'm' && fn[len-2] == 'p' && fn[len-1] == '3') {
            if (no == select_idx_mp3) {
                sprintf(now_playing, "%s", de->d_name);
                no++;
                break;
            }
            no++;
        }
    }
    if (select_idx_mp3<1 || select_idx_mp3>=no){
        if (playListNow == 0)
            select_mp3 = -1;
        else{
            playListNow = 0;
            select_mp3 = -1;
        }
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
    if (playListNow ==0 && select_mp3 > 0)
        printf("Now Playing : %s\n\n", now_playing);
    else if(playListNow > 0 && playListNow <= MAX_PLAYLIST && playListMusic[playListNow].listMusic[select_mp3]>0)
        printf("PlayList %s\nNow Playing : %s\n\n", playListMusic[playListNow].name, now_playing);
    
    printf("Help :\nopen to open music\n apl to add new playlist\n ppl to select playlist\nstop to stop music\n   p to play/pause music\n   , to rewind music\n   . to forward music\nprev to play previous music\nnext to play next music\nexit to exit mp3player\n\n");
    printf("Command : \n");
    return NULL;
}