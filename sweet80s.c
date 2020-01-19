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
#include <peekpoke.h>
#include <conio.h>

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>


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
#define BITMAP_ADDRESS        ((void*)0x2000)
#define SCREEN_ADDRESS        ((void*)0x0400)
#define COLMAP_ADDRESS        ((void*)0xD800)


#define DEVICE_NUM             8

#define MAX_FILES              25

char files[MAX_FILES][17];
uint8_t filesIndex = 0;

unsigned char floppy_bin[] = {
  0x15, 0x55, 0x55, 0x15, 0x55, 0x55, 0x15, 0x55, 0xa9, 0x15, 0x55, 0xa9,
  0x15, 0x55, 0xa9, 0x15, 0x55, 0x55, 0x15, 0x55, 0x55, 0x15, 0x59, 0x55,
  0x15, 0x6a, 0x55, 0x15, 0x6a, 0x55, 0x15, 0x59, 0x55, 0x15, 0x55, 0x55,
  0x15, 0x55, 0x55, 0x15, 0x55, 0x55, 0x15, 0x55, 0x55, 0x15, 0x59, 0x55,
  0x15, 0x59, 0x55, 0x15, 0x59, 0x55, 0x15, 0x59, 0x55, 0x15, 0x55, 0x55,
  0x00, 0x00, 0x00, 0x00
};
unsigned int floppy_bin_len = 64;


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


void initIcons()
{
    /* hide sprite 0 */
    VIC.spr_ena = 0x00;

    /* sprite 0 data at 12288 */
    POKE(2040, 13);

    /* sprite 0 data */
    memcpy((void*)832, floppy_bin, floppy_bin_len);

    /* sprite 0 in multi-color */
    VIC.spr_mcolor = 0x01;

    /* sprite 0 color */
    VIC.spr0_color = 1;

    /* sprite 0 position */
    VIC.spr_hi_x = 0x01;
    VIC.spr0_x = 50;
    VIC.spr0_y = 220;
}


void waitakey()
{
    while (!kbhit())
        ;
    cgetc();
}


void renderTitle()
{
    clrscr();

    bgcolor(COLOR_BLACK);
    bordercolor(COLOR_BLACK);
    textcolor(COLOR_YELLOW);

    cputs("Sweet80s\n\r");
    cputs("--------\n\n\r");

    cputs("\n\r");
}


void buildFileList()
{
    uint8_t result = 0;
    struct cbm_dirent entry;

    /* what're doing */
    cputs("Reading image list from disk...\n\n\r");

    /* open directory list */
    result = cbm_opendir(1, DEVICE_NUM, "$");

    /* verify operation result */
    if (result != 0) {
        cputs("ERROR reading directory list!!\n\r");
        return;
    }

    /* check each directory entry */
    result = 0;
    while ((result = cbm_readdir(1, &entry)) == 0) {
        if (entry.name[0] == '!') {
            strncpy(files[filesIndex], entry.name, 17);
            cprintf(" FOUND %s\n\r", files[filesIndex]);
            filesIndex++;
        }
    }

    /* close directory lis */
    cbm_closedir(1);
}

int main() 
{
    bool graphicsInitialized = false;
    uint8_t index = 0;

    /* write something cool */
    renderTitle();

    /* initializa icons graphics */
    initIcons();

    /* looking for images on disk */
    buildFileList();

    while (true) {

        /* show sprite 0 */
        VIC.spr_ena = 0x01;

        /* load Koala Image image file into RAM */
        loadKOA(files[index%filesIndex], DEVICE_NUM);

        /* hide sprite 0 */
        VIC.spr_ena = 0x00;

        /* verify if graphics is initialized */
        if (!graphicsInitialized) {

            /* mark graphics as initialized */
            graphicsInitialized = true;

            /* initialize multi-color graphics mode */
            initGraphics();
        }

        /* retrieve and render KOALA image from RAM */
        renderKOA((uint8_t*)0x6000);

        /* next file name */
        index++;

        /* wait until a key is pressed */
        waitakey();
    }

    /* done */
    return 0;
}
