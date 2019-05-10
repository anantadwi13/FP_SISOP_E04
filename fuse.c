#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

struct Directory {
	char path[1024];
	char filename[1024];
	int totalKembar;
};

struct Directory listDirectory[1024];

int totalItem = 0;

static const char *dirpath = "/home/anantadwi13/SISOP/fp/mp3";
pthread_t tid;

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
	pthread_create(&tid,NULL,&join,NULL);

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

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}