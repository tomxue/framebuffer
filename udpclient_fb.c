#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>

#define BUFSIZE 2000
#define SERV_PORT 8888

char recv_data[BUFSIZE];

int fb_set(char *fb_data, int size, int pos)
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

	
	for(location=pos;location<(pos+size);location++)	    
	{	
        	*(fbp + location) = fb_data[location-pos]; //*(fbp + screensize -2 -location); //0xbb; /* blue color depth */
	}  
  
      munmap(fbp, screensize);  /* release the memory */
      close(fbfd); 
      return 0;
}


int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in servaddr;
	int n,m,size, pos;
    char sendline[80] = "from N900 udp client";	
    
	size = 1000;
	pos = 0;
	memset(recv_data, 0x33, BUFSIZE);
	
    /* check args */
    if(argc != 2)
    {
        printf("usage: udpclient <IPaddress>\n");
        exit(1);
    }
    
    /* init servaddr */
    bzero(&servaddr, sizeof(servaddr));
    //#define AF_INET 2, Internet IP Protocol
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
    {
        printf("[%s] is not a valid IPaddress\n", argv[1]);
        exit(1);
    }
    
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    /* connect to server */
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("connect error");
        exit(1);
    }
	else
		printf("connect OK...\n");

		
        /* send to server */
        m = write(sockfd, sendline, strlen(sendline));
		printf("The real sent data account is %d\n", m);
		
        /* receive "from N900 udp client" from server */
        n = read(sockfd, recv_data, m);
		printf("The real recv data account is %d\n", n);
		
		while(1){
		/* receive "from N900 udp client" from server */
        n = read(sockfd, recv_data, size);
		pos = pos + size;
		printf("UDP client recv frame buffer data %d bytes\n",n);
		printf("\n");		
		
		fb_set(recv_data, size, pos);
		}
	
    return 0;
}
