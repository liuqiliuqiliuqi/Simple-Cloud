#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<assert.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>
#include"work_pthread.h"
#include<termios.h>
#include<sys/ioctl.h>

#define ARGC 10

void printls(char *read_buff)
{
    int maxsize=0;
    int i=0;
    int count=0;
    int t=0;

    while(read_buff[i]!='\0')
    {
        if(read_buff[i]=='\n')
            {
                t+=1;
                if(count>maxsize)
                {
                    maxsize=count;

                }
                count =0;
                i++;
                continue;
            }
            
    }

    struct winsize size;
    ioctl(STDIN_FILENO,TIOCGWINSZ,&size);
    int num=(size.ws_col)/(maxsize+3);

    i=3;

    while(1)
    {
        int j=0;
        for(;j<num;j++)
        {
            int sign=maxsize;
            while(read_buff[i]!='\n')
            {
                printf("%c",read_buff[i]);
                i++;
                sign--;
            }
            while(sign!=0)
            {
                sign--;
                printf("\n");
                if(read_buff[i]=='\0')
                {
                    printf("\n");
                    return;
                }
            }
            printf("\n");
        }
    }
}



void thread_start(int c)
{
    pthread_t id;
    pthread_create(&id,NULL,work_thread,(void*)c);
}

void get_argv(char buff[],char*myargv[])
{

      char *p = NULL;
      char *s=strtok_r(buff," ",&p);
      int i=0;
      
      while(s!=NULL)
      {
          myargv[i++]=s;
          s = strtok_r(NULL," ",&p);
      }

}
int recv_file(int sockfd,char *name)
{
    char buff[128]={0};
    if(recv(sockfd,buff,127,0)<=0)
    {
        return -1;
    }

    if(strncmp(buff,"ok",2)!=0)
    {
        printf("%s\n",buff);
        return 0;
    }

    int size = 0 ;
    sscanf(buff+3,"%d",&size);
    printf("recv:file(%s):size%d\n",name,size);

    int fd= open(name,O_WRONLY|O_CREAT,0600);
    if(fd == -1)
    {
        send(sockfd,"err",3,0);
        return;
    }

    send(sockfd,"ok",2,0);

    int num=0;

  int cur_size=0;
    char data[256]={0};
    while(1)
    {
        num=recv(sockfd,data,256,0);
        if(num<=0)
        {
            return -1;

        }
        write(fd,data,num);
        cur_size=cur_size+num;

       // float f = cur_size*100.0/size;
       // printf("download:%.2f%%\r",f);
      //  fflush(stdout);

        if(cur_size>=size)
        {
            break;
        }

    }
    return 0;
}

void send_file(int c,char *myargv[])
{
    if(myargv[1]==NULL)
    {
        send(c,"get :not file name!",17,0);
        return;
    }
    int fd = open(myargv[1],O_RDONLY);
    if(fd == -1)
    {
        send(c,"Not found!",10,0);
        return;
    }

    int size = lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);

    char status[32]={0};
    sprintf(status,"ok#%d",size);
    send(c,status,strlen(status),0);


    char cli_status[32]={0};
    if(recv(c,cli_status,31,0)<=0)
    {
        return;
    }
    if(strcmp(cli_status,"ok")!=0)
    {
        return;
    }

    char data[256]={0};
    int num = 0;
   // int cur_size = 0;
    while((num=read(fd,data,256))>0)
    {
        send(c,data,num,0);
     //   cur_size=cur_size+num;

       // float f = cur_size*100.0/size;
       // printf("download:%.2f%%\r",f);
      //  fflush(stdout);
    }

    close(fd);

     return;

}

void* work_thread(void* arg)
{
    int c= (int )arg;
    while(1)
    {
        char buff[128]={0};
        int n = recv(c,buff,128,0);
        if(n<=0)
        {
            close(c);
            printf("one client over\n");
            break;
        }

        char *myargv[ARGC]={0};
        get_argv(buff,myargv);

        if(strcmp(myargv[0],"get")==0)
        {
            send_file(c,myargv);
        }
        else if((strcmp(myargv[0],"put")==0))
        {
            recv_file(c,myargv[1]);
        }
        else
        {
        
        int pipefd[2];
        pipe(pipefd);


        pid_t pid =fork();
        if(pid == 0)
        {
            
            dup2(pipefd[1],1);
            dup2(pipefd[1],2);
            execvp(myargv[0],myargv);
            perror("exec error");
            exit(0);
        }

        close(pipefd[1]);
        wait(NULL);
        


        char read_buff[1024]={0};
        strcpy(read_buff,"ok#");
        read(pipefd[0],read_buff+strlen(read_buff),1000);
        send(c,read_buff,strlen(read_buff),0);
        }
    }
}



