#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

// GPIO Pin Numbers for the led lights 
#define ACTRED  10
#define ACTYELLOW  11
#define ACTGREEN  13

#ifndef	TRUE
#define	TRUE	(1==1)
#define	FALSE	(1==2)
#endif

#define	PAGE_SIZE		(4*1024)
#define	BLOCK_SIZE		(4*1024)

#define	INPUT			 0
#define	OUTPUT			 1

#define	LOW			 0
#define	HIGH			 1


static volatile unsigned int gpiobase ;
static volatile uint32_t *gpio ;

int failure (int fatal, const char *message, ...)
{
  va_list argp ;
  char buffer [1024] ;

  if (!fatal) //  && wiringPiReturnCodes)
    return -1 ;

  va_start (argp, message) ;
  vsnprintf (buffer, 1023, message, argp) ;
  va_end (argp) ;

  fprintf (stderr, "%s", buffer) ;
  exit (EXIT_FAILURE) ;

  return 0 ;
}

//Headers
void delay (int time);
int blinkRed();
int blinkYellow();
int blinkGreen();
int closeRed();
int closeYellow();
int closeGreen();
int blinkingYellow(int times);



int main (void)
{
  int pinACTRED = ACTRED; 
  int pinACTYELLOW = ACTYELLOW; 
  int pinACTGREEN = ACTGREEN;
  int fSelRed, shiftRed, pin, clrOff, setOff;
  int fSelGreen, shiftGreen,fSelYellow, shiftYellow;

  int   fd ;
  int   j;
  int theValue, thePin;
  uint32_t res;


  printf ("Raspberry Pi blinking LED %d\n", ACTRED) ;
  printf ("Raspberry Pi blinking LED %d\n", ACTYELLOW) ;
  printf ("Raspberry Pi blinking LED %d\n", ACTGREEN) ;

  if (geteuid () != 0)
    fprintf (stderr, "setup: Must be root. (Did you forget sudo?)\n") ;

  // -----------------------------------------------------------------------------
  // constants for RPi2
  gpiobase = 0x3F200000 ;

  // -----------------------------------------------------------------------------
  // memory mapping 
  // Open the master /dev/memory device

  if ((fd = open ("/dev/mem", O_RDWR | O_SYNC | O_CLOEXEC) ) < 0)
    return failure (FALSE, "setup: Unable to open /dev/mem: %s\n", strerror (errno)) ;

  // GPIO:
  gpio = (uint32_t *)mmap(0, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, gpiobase) ;
  if ((int32_t)gpio == -1)
    return failure (FALSE, "setup: mmap (GPIO) failed: %s\n", strerror (errno)) ;
  else
    fprintf(stderr, "NB: gpio = %x for gpiobase %x\n", gpio, gpiobase);


// -----------------------------------------------------------------------------to be changed
  // setting the mode
  fprintf(stderr, "setting pin %d to %d ...\n", pinACTRED, OUTPUT);
  fprintf(stderr, "setting pin %d to %d ...\n", pinACTYELLOW, OUTPUT);
  fprintf(stderr, "setting pin %d to %d ...\n", pinACTGREEN, OUTPUT);
  fSelRed =  1;    // GPIO 10 lives in register 1 (GPFSEL)
  shiftRed =  0;  // GPIO 10 sits in slot 0 of register 1, thus shift by 0*3 (3 bits per pin)
  *(gpio + fSelRed) = (*(gpio + fSelRed) & ~(7 << shiftRed)) | (1 << shiftRed) ;  // Sets bits to one = output

  fSelYellow =  1;    // GPIO 11 lives in register 1 (GPFSEL)
  shiftYellow =  3;  // GPIO 11 sits in slot 1 of register 1, thus shift by 1*3 (3 bits per pin)
  *(gpio + fSelYellow) = (*(gpio + fSelYellow) & ~(7 << shiftYellow)) | (1 << shiftYellow) ;  // Sets bits to one = output

  fSelGreen =  1;    // GPIO 13 lives in register 1 (GPFSEL)
  shiftGreen =  9;  // GPIO 13 sits in slot 3 of register 1, thus shift by 3*3 (3 bits per pin)
  *(gpio + fSelGreen) = (*(gpio + fSelGreen) & ~(7 << shiftGreen)) | (1 << shiftGreen) ;  // Sets bits to one = output

  // -----------------------------------------------------------------------------

  closeRed();
  closeYellow();
  closeGreen();
  
  printf("Starting loop for traffic signal");
    for (j=0;j<1000;j++)
    {
      blinkRed();
      delay(700);
      blinkYellow();
      delay(500);
      closeRed();
      closeYellow();
      blinkGreen();
      delay(1200);
      closeGreen();
      blinkingYellow(8);
      closeYellow();
    }
  
   
  // Clean up: write LOW
  clrOff = 10; 
  *(gpio + clrOff) = 1024 | 2048 | 8192;
 
  fprintf(stderr, "end main.\n");

}

//function to delay
void delay (int time)
{
  struct timespec sleeper, dummy ;
  sleeper.tv_sec  = (time_t)(time / 1000) ;
  sleeper.tv_nsec = (long)(time % 1000) * 1000000 ;
  nanosleep(&sleeper,NULL);
}

//opening red light
int blinkRed()
{
  *(gpio + 7) = 1024;
  return 0;
}
//opening yellow light
int blinkYellow()
{
  *(gpio + 7) = 2048;
  return 0;
}
//opening green light
int blinkGreen()
{
  *(gpio + 7) = 8192;
  return 0;
}
//closing red light 
int closeRed()
{
  *(gpio + 10) = 1024;
  return 0;
}
//closing yellow light
int closeYellow()
{
  *(gpio + 10) = 2048;
  return 0;
}
//closing green light
int closeGreen()
{
  *(gpio + 10) = 8192;
  return 0;
}
//blinking yellow led light
int blinkingYellow(int times)
{
  for (int t=0;t<=times;t++)
  {
    int theValue = ((t % 2) == 0) ? HIGH : LOW;
    int off = (theValue == LOW) ? 10 : 7;
    *(gpio + off) = 2048;
    delay(200);
  }
}
