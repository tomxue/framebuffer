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

#define MAXLINE 80
#define SERV_PORT 8888
#define debug 0

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
                if(debug) printf("Error: cannot open framebuffer device.\n");
                exit(1);
        }
        if(debug) printf("The framebuffer device was opened successfully.\n");
       
        /* Get fixed screen information */
        if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
                if(debug) printf("Error reading fixed information.\n");
                //exit(2);
        }
 
        /* Get variable screen information */
        if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
                if(debug) printf("Error reading variable information.\n");
                //exit(3);
        }
/* show these information*/
 if(debug) printf("vinfo.xres=%d\n",vinfo.xres);
 if(debug) printf("vinfo.yres=%d\n",vinfo.yres);
 if(debug) printf("vinfo.xres_virtual=%d\n",vinfo.xres_virtual);
 if(debug) printf("vinfo.yres_virtual=%d\n",vinfo.yres_virtual);
 if(debug) printf("vinfo.height=%d\n",vinfo.height);
 if(debug) printf("vinfo.width=%d\n",vinfo.width);
 if(debug) printf("vinfo.bits_per_bits=%d\n",vinfo.bits_per_pixel);
 if(debug) printf("vinfo.xoffset=%d\n",vinfo.xoffset);
 if(debug) printf("vinfo.yoffset=%d\n",vinfo.yoffset);
 if(debug) printf("finfo.line_length=%d\n",finfo.line_length);
 if(debug) printf("finfo.smem_len=%d\n",finfo.smem_len);

        /* Figure out the size of the screen in bytes */
        screensize = vinfo.yres_virtual * finfo.line_length;
		if(debug) printf("screensize is %d\n", screensize);
	//screensize = finfo.smem_len;  //same to above formula
        int bytes_per_pixel = vinfo.bits_per_pixel/8;
        
	/* Map the device to memory */
        fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
                fbfd, 0);      
        if ((int)fbp == -1) 
		{ 
			if(debug) printf("Error: failed to map framebuffer device to memory.\n"); 
			exit(4);
        	}
        if(debug) printf("The framebuffer device was mapped to memory successfully.\n");
	if(debug) printf("fbp=%p\n",fbp);

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
	int n,m,screensize,i;
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
	else
		if(debug) printf("bind ok...\n");

	len = sizeof(cliaddr);
        /* waiting for receiving data */
        n = recvfrom(sockfd, mesg, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
		if(debug) printf("recv from udp client: %s\n", mesg);
		
        /* sent data back to client */
        sendto(sockfd, mesg, n, 0, (struct sockaddr *)&cliaddr, len);
		if(debug) printf("send to udp client: %s\n", mesg);
		if(debug) printf("\n");
	
	for(;;)
    {	
		screensize = fb_get();
		if(debug) printf("the screen size is %d\n", screensize);
		
		for(i=0;i<(screensize-1000);i=i+1000){
		/* sent data back to client */
		m =	sendto(sockfd, (fb_data+i), 1000, 0, (struct sockaddr *)&cliaddr, len);
		if(debug) printf("send %d bytes of framebuffer data\n",m);
		if(debug) printf("\n");
		}
    }


    return 0;
}

