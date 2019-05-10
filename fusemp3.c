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

static const char *dirpath = "/home/arisatox/tempfolder";
pthread_t tid;

void listdir(const char *name)
{
    if(strcmp(name, dirpath) == 0) return;

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
                char command[1005], temp[1005];
                
                strcpy(temp,name);
                strcat(temp,"/");
                strcat(temp,entry->d_name);

                sprintf(command, "cp '%s' '%s'", temp, dirpath);

                system(command);
            }
        }
    }
    closedir(dir);
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
  int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);
	res = lstat(fpath, stbuf);

	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
  	char fpath[1005];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = 0;

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;

		int ext = strlen(de->d_name);
		printf("%s\n",de->d_name);
		if(de->d_name[ext - 1] == '3' && de->d_name[ext - 2] == 'p' && de->d_name[ext - 3] == 'm' && de->d_name[ext - 4] == '.')
		{
				
		}

		res = (filler(buf, de->d_name, &st, 0));
		if(res!=0) break;
	}

	closedir(dp);
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

	(void) fi;
	fd = open(fpath, O_RDONLY);
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
	char start[1005] = "/home/arisatox";
	
	listdir(start);
 	return NULL;
}

static void* xmp_init(struct fuse_conn_info *conn)
{
	int status;
	struct stat mystat;
	if(stat(dirpath, &mystat) == 0 && S_ISDIR(mystat.st_mode))
    {
      
    }
    else
    {
        mkdir(dirpath, 0750);
    }

	pthread_create(&tid,NULL,&join,NULL);
	
	while((wait(&status)) > 0);

	return NULL;
}

void xmp_destroy()
{
	char fpath[1005];

 	DIR *dir;
	struct dirent *de;
	dir = opendir(dirpath);

 	while((de = readdir(dir)) != NULL){
		if (de->d_type == DT_REG) {
			sprintf(fpath, "%s/%s", dirpath, de->d_name);
			remove(fpath);
		}
	}
	remove(dirpath);
}

static struct fuse_operations xmp_oper = {
	.init		= xmp_init,
    .getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
	.destroy	= xmp_destroy,
	
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}