/**
 * sweet80s.c
 * daniele.olmisani@gmail.com
 * 
 * compile using:
 *     cl65 -t c64 -O sweet80sc -o sweet80s.prg
 * 
 * see LINCESE file
 */


#include <c64.h>
#include <cbm.h>
#include <errno.h>

#include <string.h>
#include <stdint.h>
#include <stdbool.h>


/* KOALA image structure */
#define BITMAP_FILE_OFFSET        0
#define BITMAP_FILE_LENGHT     8000
#define SCREEN_FILE_OFFSET     8000
#define SCREEN_FILE_LENGTH     1000
#define COLMAP_FILE_OFFSET     9000
#define COLMAP_FILE_LENGTH     1000
#define BKGCOL_FILE_OFFSET    10000
#define BKGCOL_FILE_LENGTH        1

/* */
#define BITMAP_ADDRESS       ((void*)0x2000)
#define SCREEN_ADDRESS       ((void*)0x0400)
#define COLMAP_ADDRESS       ((void*)0xD800)



#define DEVICE_NUM                8


const char* FILES[] = { "ylenia", "marta", (const char*)0 };


void initGraphics()
{
    /* enable bitmap mode */
    VIC.ctrl1 = 0x3B;

    /* enable multicolor graphic-mode */ 
    VIC.ctrl2 = 0x18;

    /* set screen at $0400 and bitmap at $2000 */
    VIC.addr = 0x1F;

    /* set black background */
    VIC.bgcolor0 = COLOR_BLACK;

    /* set black border */ 
    VIC.bordercolor = COLOR_BLACK;

    /* clear bitmap area */
    memset(BITMAP_ADDRESS, 0x00, BITMAP_FILE_LENGHT);
}


uint32_t loadKOA(const char* fileName, uint8_t device)
{
    int32_t result = 0;
    result = cbm_load(fileName, device ,(void*)0);

    return (result == 0) ? _oserror : 0;
}


void renderKOA(uint8_t* imageBuffer) 
{
    /* load bitmap data */
    memcpy(BITMAP_ADDRESS, imageBuffer+BITMAP_FILE_OFFSET, BITMAP_FILE_LENGHT);

    /* load screen data */
    memcpy(SCREEN_ADDRESS, imageBuffer+SCREEN_FILE_OFFSET, SCREEN_FILE_LENGTH);

    /* load color map */
    memcpy(COLMAP_ADDRESS, imageBuffer+COLMAP_FILE_OFFSET, COLMAP_FILE_LENGTH);

    /* load background colour */
    VIC.bgcolor0 = imageBuffer[BKGCOL_FILE_OFFSET];

    /* set black border */ 
    VIC.bordercolor = COLOR_BLACK;
}


int main() 
{
    uint8_t index = 0;

    /* initialize multi-color graphics mode */
    initGraphics();

    while (true) {

        /* restart from begin if end reached */
        if (FILES[index] == (const char*)0) {
            index = 0;
        }

        /* load Koala Image image file into RAM */
        loadKOA(FILES[index], DEVICE_NUM);
    
        /* retrieve and render KOALA image from RAM */
        renderKOA((uint8_t*)0x6000);

        /* next file name */
        index++;
    }

    /* done */
    return 0;
}
