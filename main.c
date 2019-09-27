

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define LED_GPIO 13
#define DIMMING_GPIO 12
#define BLANKING_GPIO 23
#define HV_EN_GPIO 24

#define SPI_PATH "/dev/spidev0.0"
#define SPI_MODE_0 (0 | SPI_CPHA)

#define IN 0
#define OUT 1

#define LOW 0
#define HIGH 1

static int GPIOExport(int pin);
static int GPIOUnexport(int pin);
static int GPIODirection(int pin, int dir);
static int GPIORead(int pin);
static int GPIOWrite(int pin, int value);

int main()
{
   int fd_spi;
   unsigned int speed = 500000;
   uint8_t mode = SPI_MODE_0;

   /*
	 * Enable GPIO pins
	 */
   if (-1 == GPIOExport(LED_GPIO) || -1 == GPIOExport(DIMMING_GPIO) || -1 == GPIOExport(BLANKING_GPIO) || -1 == GPIOExport(HV_EN_GPIO))
      return (1);
   sleep(1);
   /*

	 * Set GPIO directions
	 */
   if (-1 == GPIODirection(LED_GPIO, OUT) || -1 == GPIODirection(DIMMING_GPIO, OUT) || -1 == GPIODirection(BLANKING_GPIO, OUT) || -1 == GPIODirection(HV_EN_GPIO, OUT))
      return (2);
   sleep(1);

   fd_spi = open("/dev/spidev0.0", O_RDWR);

   if (ioctl(fd_spi, SPI_IOC_WR_MAX_SPEED_HZ, &speed) != 0)
   {
      perror("ioctl");
      exit(EXIT_FAILURE);
   }

   if (ioctl(fd_spi, SPI_IOC_WR_MODE, &mode) != 0)
   {
      perror("ioctl");
      exit(EXIT_FAILURE);
   }

   if (-1 == GPIOWrite(BLANKING_GPIO, HIGH) || -1 == GPIOWrite(HV_EN_GPIO, LOW) || -1 == GPIOWrite(DIMMING_GPIO, HIGH))
      return (3);

   printf("Turning on\n");

   uint8_t dataon[4] = {1, 1, 1, 1};

   for (uint8_t i = 0; i < 255; i++)
   {
      printf("%d\n",i);   
      write(fd_spi, dataon, 4);
      if (-1 == GPIOWrite(LED_GPIO, HIGH))
         return (3);
      usleep(500000);
   }

   uint8_t dataoff[4] = {0, 0, 0, 0};
   printf("turning off\n");

   write(fd_spi, dataoff, 4);
   if (-1 == GPIOWrite(BLANKING_GPIO, HIGH))
      return (3);
   sleep(1);

   /*
	 * Disable GPIO pins
	 */
   if (-1 == GPIOUnexport(LED_GPIO) || -1 == GPIOUnexport(DIMMING_GPIO) || -1 == GPIOUnexport(BLANKING_GPIO) || -1 == GPIOUnexport(HV_EN_GPIO))
      return (4);
   return 0;
}

static int GPIOExport(int pin)
{
#define BUFFER_MAX 3
   char buffer[BUFFER_MAX];
   ssize_t bytes_written;
   int fd;

   fd = open("/sys/class/gpio/export", O_WRONLY);
   if (-1 == fd)
   {
      fprintf(stderr, "Failed to open export for writing!\n");
      return (-1);
   }

   bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
   write(fd, buffer, bytes_written);
   close(fd);
   return (0);
}

static int GPIOUnexport(int pin)
{
   char buffer[BUFFER_MAX];
   ssize_t bytes_written;
   int fd;

   fd = open("/sys/class/gpio/unexport", O_WRONLY);
   if (-1 == fd)
   {
      fprintf(stderr, "Failed to open unexport for writing!\n");
      return (-1);
   }

   bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
   write(fd, buffer, bytes_written);
   close(fd);
   return (0);
}

static int GPIODirection(int pin, int dir)
{
   static const char s_directions_str[] = "in\0out";

#define DIRECTION_MAX 35
   char path[DIRECTION_MAX];
   int fd;

   snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
   fd = open(path, O_WRONLY);
   if (-1 == fd)
   {
      fprintf(stderr, "Failed to open gpio direction for writing!\n");
      return (-1);
   }

   if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3))
   {
      fprintf(stderr, "Failed to set direction!\n");
      return (-1);
   }

   close(fd);
   return (0);
}

static int GPIORead(int pin)
{
#define VALUE_MAX 30
   char path[VALUE_MAX];
   char value_str[3];
   int fd;

   snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
   fd = open(path, O_RDONLY);
   if (-1 == fd)
   {
      fprintf(stderr, "Failed to open gpio value for reading!\n");
      return (-1);
   }

   if (-1 == read(fd, value_str, 3))
   {
      fprintf(stderr, "Failed to read value!\n");
      return (-1);
   }

   close(fd);

   return (atoi(value_str));
}

static int GPIOWrite(int pin, int value)
{
   static const char s_values_str[] = "01";

   char path[VALUE_MAX];
   int fd;

   snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
   fd = open(path, O_WRONLY);
   if (-1 == fd)
   {
      fprintf(stderr, "Failed to open gpio value for writing!\n");
      return (-1);
   }

   if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1))
   {
      fprintf(stderr, "Failed to write value!\n");
      return (-1);
   }

   close(fd);
   return (0);
}
