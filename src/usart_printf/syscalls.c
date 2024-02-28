/**
 *****************************************************************************
 **
 **  File        : syscalls.c
 **
 **  Author	    : Auto-generated by STM32CubeIDE
 **
 **  Abstract    : STM32CubeIDE Minimal System calls file
 **
 ** 		          For more information about which c-functions
 **                need which of these lowlevel functions
 **                please consult the Newlib libc-manual
 **
 **  Environment : STM32CubeIDE MCU
 **
 **  Distribution: The file is distributed as is, without any warranty
 **                of any kind.
 **const char *restrict __format, ...
 *****************************************************************************
 **
 ** <h2><center>&copy; COPYRIGHT(c) 2018 STMicroelectronics</center></h2>
 **
 ** Redistribution and use in source and binary forms, with or without modification,
 ** are permitted provided that the following conditions are met:
 **   1. Redistributions of source code must retain the above copyright notice,
 **      this list of conditions and the following disclaimer.
 **   2. Redistributions in binary form must reproduce the above copyright notice,
 **      this list of conditions and the following disclaimer in the documentation
 **      and/or other materials provided with the distribution.
 **   3. Neither the name of STMicroelectronics nor the names of its contributors
 **      may be used to endorse or promote products derived from this software
 **      without specific prior written permission.
 **
 ** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 ** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 ** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 ** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 ** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 ** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 ** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 ** CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 ** OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 ** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **
 **
 *****************************************************************************
 */

#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include "usart_printf.h"

extern int errno;
extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

register char * stack_ptr asm("sp");

char *__env[1] = { 0 };
char **environ = __env;

int _getpid(void)
{
    return 1;
}

int _kill(int pid, int sig)
{
    errno = EINVAL;
    return -1;
}

void _exit (int status)
{
    _kill(status, -1);
    while (1) {}		/* Make sure we hang here */
}


__attribute__((weak)) int _read(int file, char *ptr, int len)
{
    (void) file, (void) ptr, (void) len;

    return -1;
}

void write_byte(uint8_t byte) {
    // We check if there are any new data using the USART_SR register,
    // if the bit 7 is 1, it means that the data has finished writing.
    // If so, we can use the USART_DR register to write the new data.
    while(!((USART2->USART_SR & (1 << 7)))); 

    // Sets the data register to the ASCII code of the 
    // character 'x' by bitwise ANDing it with 0xFF
    // to ensure only the lower 8 bits (byte that we wanna transmit) are considered.
    // To see the actual bytes sent, I used picocom, but any dumb-terminal emulation works
    // fine:
    //
    // picocom -b 9600 /dev/ttyACM0.
    USART2->USART_DR = (byte & 0xFF); 
}

__attribute__((weak)) int _write(int file, char *ptr, int len)
{
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        write_byte(*ptr++);
    }

    return len;
}

int _close(int file)
{
    return -1;
}


int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file)
{
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    return 0;
}

int _open(char *path, int flags, ...)
{
    /* Pretend like we always fail */
    return -1;
}

int _wait(int *status)
{
    errno = ECHILD;
    return -1;
}

int _unlink(char *name)
{
    errno = ENOENT;
    return -1;
}

int _times(struct tms *buf)
{
    return -1;
}

int _stat(char *file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _link(char *old, char *new)
{
    errno = EMLINK;
    return -1;
}

int _fork(void)
{
    errno = EAGAIN;
    return -1;
}

int _execve(char *name, char **argv, char **env)
{
    errno = ENOMEM;
    return -1;
}

/**
  _sbrk
  Increase program data space. Malloc and related functions depend on this
 **/
caddr_t _sbrk(int incr)
{
    extern char end asm("end");
    static char *heap_end;
    char *prev_heap_end;

    if (heap_end == 0)
        heap_end = &end;

    prev_heap_end = heap_end;
    if (heap_end + incr > stack_ptr)
    {
        errno = ENOMEM;
        return (caddr_t) -1;
    }

    heap_end += incr;

    return (caddr_t) prev_heap_end;
}
