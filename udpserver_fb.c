#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>

#define MAXLINE 384000
#define SERV_PORT 8886

char fb_data[2000000];

int fb_get()
{
        int fbfd = 0;
        struct fb_var_screeninfo vinfo;
        struct fb_fix_screeninfo finfo;
        long int screensize = 0;
        char *fbp = 0;
        int x = 0, y = 0;
        long int location = 0;
        int sav=0;
		
 
        /* open device*/
        fbfd = open("/dev/fb0", O_RDWR);
        if (!fbfd) {
                printf("Error: cannot open framebuffer device.\n");
                exit(1);
        }
        printf("The framebuffer device was opened successfully.\n");
       
        /* Get fixed screen information */
        if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
                printf("Error reading fixed information.\n");
                //exit(2);
        }
 
        /* Get variable screen information */
        if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
                printf("Error reading variable information.\n");
                //exit(3);
        }
/* show these information*/
 printf("vinfo.xres=%d\n",vinfo.xres);
 printf("vinfo.yres=%d\n",vinfo.yres);
 printf("vinfo.xres_virtual=%d\n",vinfo.xres_virtual);
 printf("vinfo.yres_virtual=%d\n",vinfo.yres_virtual);
 printf("vinfo.height=%d\n",vinfo.height);
 printf("vinfo.width=%d\n",vinfo.width);
 printf("vinfo.bits_per_bits=%d\n",vinfo.bits_per_pixel);
 printf("vinfo.xoffset=%d\n",vinfo.xoffset);
 printf("vinfo.yoffset=%d\n",vinfo.yoffset);
 printf("finfo.line_length=%d\n",finfo.line_length);
 printf("finfo.smem_len=%d\n",finfo.smem_len);

        /* Figure out the size of the screen in bytes */
        screensize = vinfo.yres_virtual * finfo.line_length;
	//screensize = finfo.smem_len;  //same to above formula
        int bytes_per_pixel = vinfo.bits_per_pixel/8;
        
	/* Map the device to memory */
        fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
                fbfd, 0);      
        if ((int)fbp == -1) 
		{ 
			printf("Error: failed to map framebuffer device to memory.\n"); 
			exit(4);
        	}
        printf("The framebuffer device was mapped to memory successfully.\n");
	printf("fbp=%p\n",fbp);

	//memset(fbp,0,screensize);
        
	/* Where we are going to put the pixel */
	// wrong formula: location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * finfo.line_length; 

	for(location=0;location<screensize;location++)	    
	{	
        	fb_data[location] = *(fbp + location); //*(fbp + screensize -2 -location); //0xbb; /* blue color depth */
	} 
  
      munmap(fbp, screensize);  /* release the memory */
      close(fbfd); 
      return screensize;
}


int main(void)
{
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
	int n,i,m;
    socklen_t len;
    char mesg[MAXLINE];

    //SOCK_DGRAM: Connectionless, unreliable datagrams of fixed maximum length.
    //IPPROTO_UDP = 17, User Datagram Protocol.
    //IPPROTO_IP = 0, Dummy protocol for TCP.
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); /* create a socket */

    /* init servaddr */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    //Convert the host byte order to network byte order
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    /* bind address and port to socket */
    if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("bind error");
        exit(1);
    }


    len = sizeof(cliaddr);
	
	sleep(2);
	n = fb_get();
	printf("the screen size is %d\n", n);
		/* sent data back to client */
	for(i=0;i<n;i=i+100){
		m =	sendto(sockfd, (fb_data+i), 100, 0, (struct sockaddr *)&cliaddr, len);
		printf("really send %d bytes\n",m);
	}

    return 0;
}

