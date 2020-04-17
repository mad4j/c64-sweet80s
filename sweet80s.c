/**
 * sweet80s.c
 * https://github.com/mad4j/c64-sweet80s
 * 
 * compile using:
 *     cl65 -t c64 -O sweet80sc -o sweet80s.prg
 * 
 * see LINCESE file
 */


/* cc65 specific libraries */
#include <6502.h>
#include <c64.h>
#include <cbm.h>
#include <peekpoke.h>
#include <conio.h>
#include <device.h>

/* standard specific libraries */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>


/* return RAMO Bank number from memory absolute address */
#define BANK_INDEX(X)               ((uint32_t)X / 0x4000)

/* return offset within RAM Bank from absolute memory address */
#define BANK_OFFSET(X)              ((uint32_t)X % 0x4000)

/* KOALA image file format */
#define KOALA_BITMAP_OFFSET             0
#define KOALA_BITMAP_LENGHT          8000
#define KOALA_SCREEN_OFFSET          8000
#define KOALA_SCREEN_LENGTH          1000
#define KOALA_COLMAP_OFFSET          9000
#define KOALA_COLMAP_LENGTH          1000
#define KOALA_BKGCOL_OFFSET         10000
#define KOALA_BKGCOL_LENGTH             1

#define KOALA_FILE_SIZE             10003

/* Multi-color RAM addresses */
#define BITMAP_RAM_ADDRESS          ((uint8_t*)0xE000)
#define SCREEN_RAM_ADDRESS          ((uint8_t*)0xD000)
#define COLMAP_RAM_ADDRESS          ((uint8_t*)0xD800)

/* Sprite data pointers base address*/
#define SPRITE_DATA_PTR_RAM_ADDRESS (SCREEN_RAM_ADDRESS+1016)

/* Sprite related infomration */
#define SPRITE_0_RAM_ADDRESS         (uint8_t*)(0xFFC0)
#define SPRITE_0_POS_X               306
#define SPRITE_0_POS_Y               220

/* File system constants */
#define MAX_FILES                     25
#define MAX_FILE_NAME_LEN             17

/* Names of image files on disk */
char files[MAX_FILES][MAX_FILE_NAME_LEN];
uint8_t filesIndex = 0;


/* FLOPPY Icon sprite data */
unsigned char floppy_bin[] = {
  0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x56, 0x55, 0x55, 0x56,
  0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x69, 0x55,
  0x55, 0x96, 0x55, 0x55, 0x96, 0x55, 0x55, 0x96, 0x55, 0x55, 0x69, 0x55,
  0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x69, 0x55,
  0x55, 0x69, 0x55, 0x55, 0x69, 0x55, 0x55, 0x69, 0x55, 0x55, 0x55, 0x55,
  0x55, 0x55, 0x55, 0x00
};
unsigned int floppy_bin_len = 64;


uint8_t* koalaBuffer = 0;


/**
 * initialize multi-color graphic mode
 */
void initGraphics()
{

    CIA2.ddra |= 0x03;
    CIA2.pra &= 0xFC;

    /* enable bitmap mode */
    VIC.ctrl1 = 0x3B;

    /* enable multicolor graphic-mode */ 
    VIC.ctrl2 = 0x18;

    /* $D018/53272: set screen at $0400 and bitmap at $2000 */
    //VIC.addr = 0x1F;

    VIC.addr = 0x48;

    /* set black background */
    VIC.bgcolor0 = COLOR_BLACK;

    /* set black border */ 
    VIC.bordercolor = COLOR_BLACK;

    /* clear bitmap area */
    memset(BITMAP_RAM_ADDRESS, 0x00, KOALA_BITMAP_LENGHT);
}


/**
 * load an image stored using KOALA file format
 */
int32_t loadKOA(const char* fileName)
{
    int32_t result = 0;

    /* retrieve last used device number (as specified in location $00BA) */
    uint8_t device = getcurrentdevice();

    /* load image data at specific address ignoring embedded PRG info */
    result = cbm_load(fileName, device, koalaBuffer);

    /* FIX: it seems that cbm_load returns always 0 */

    return result;
}


/**
 * display a KOALA image store in RAM
 */
void renderKOA() 
{

    uint8_t temp = 0;

    /* load bitmap data */
    memcpy(BITMAP_RAM_ADDRESS, koalaBuffer+KOALA_BITMAP_OFFSET, KOALA_BITMAP_LENGHT);

    temp = PEEK(0x0001);

    SEI();

    /* enable RAM access in the following memory areas $A000-$BFFF, $D000-$DFFF and $E000-$FFFF */
    POKE(0x0001, temp & 0xFC);

    /* load screen data */
    memcpy(SCREEN_RAM_ADDRESS, koalaBuffer+KOALA_SCREEN_OFFSET, KOALA_SCREEN_LENGTH);

    POKE(0x0001, temp);

    CLI();


    /* load color map */
    memcpy(COLMAP_RAM_ADDRESS, koalaBuffer+KOALA_COLMAP_OFFSET, KOALA_COLMAP_LENGTH);

    /* load background colour */
    VIC.bgcolor0 = koalaBuffer[KOALA_BKGCOL_OFFSET];

    /* set black border */ 
    VIC.bordercolor = COLOR_BLACK;
}


/**
 * configure and initialize icon data
 */
void initIcons()
{
    uint8_t temp = 0;

    /* hide sprite 0 */
    VIC.spr_ena = 0x00;

    temp = PEEK(0x0001);
    SEI();

    /* update sprite 0 data memory pointer */
    POKE(SPRITE_DATA_PTR_RAM_ADDRESS, BANK_OFFSET(SPRITE_0_RAM_ADDRESS) / 64); 

    /* sprite 0 data */
    memcpy(SPRITE_0_RAM_ADDRESS, floppy_bin, floppy_bin_len);

    POKE(0x0001, temp);
    CLI();

    /* sprite 0 in multi-color */
    VIC.spr_mcolor = 0x01;

    /* sprite 0 color */
    VIC.spr0_color = COLOR_BLACK;
    VIC.spr_mcolor0 = COLOR_YELLOW;

    /* sprite 0 position */
    VIC.spr_hi_x |= (SPRITE_0_POS_X & 0x100) >> 8; 
    VIC.spr0_x = SPRITE_0_POS_X & 0xFF;
    VIC.spr0_y = SPRITE_0_POS_Y;
}


/**
 * wait for a key pressed event
 */
void waitakey()
{
    while (!kbhit())
        ;
    cgetc();
}


/**
 * clear keyboard buffer
 */
void clearkey()
{
    while (kbhit()) {
        cgetc();
    }
}

/**
 * write opening information
 */
void renderTitle()
{
    clrscr();

    bgcolor(COLOR_BLACK);
    bordercolor(COLOR_BLACK);
    textcolor(COLOR_YELLOW);

    cputs("Sweet80s\n\r");
    cputs("--------\n\r");

    cputs("\n\r");
}


/**
 * looking for image files on disk (i.e. file with name starting with '!')
 */
void buildFileList()
{
    const uint8_t fileHandle = 1;

    uint8_t result = 0;
    struct cbm_dirent entry;

    /* retrieve last used device number (as specified in location $00BA) */
    uint8_t device = getcurrentdevice();

    /* what're doing */
    cputs("Reading image list from disk...\n\n\r");

    /* open directory list */
    result = cbm_opendir(fileHandle, device, "$");

    /* verify operation result */
    if (result != 0) {
        cputs("ERROR reading directory list!!\n\r");
        return;
    }

    /* check each directory entry */
    result = 0;
    while ((result = cbm_readdir(fileHandle, &entry)) == 0) {
        if (entry.name[0] == '!') {
            strncpy(files[filesIndex], entry.name, MAX_FILE_NAME_LEN);
            cprintf(" FOUND %s\n\r", files[filesIndex]);
            filesIndex++;
        }
    }

    /* close directory list */
    cbm_closedir(fileHandle);
}


/**
 * program entry point
 */
int main() 
{
    bool graphicsInitialized = false;
    uint8_t index = 0;
    int32_t result = 0;

    /* verify coherence of RAM address assignments */
    assert(BANK_INDEX(SCREEN_RAM_ADDRESS) == BANK_INDEX(BITMAP_RAM_ADDRESS));
    assert(BANK_INDEX(SCREEN_RAM_ADDRESS) == BANK_INDEX(SPRITE_0_RAM_ADDRESS));

    koalaBuffer = malloc(KOALA_FILE_SIZE);

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
        result = loadKOA(files[index%filesIndex]);    

        /* put image on screen only if on succesufully load */
        if (result < 0) {
            bordercolor(COLOR_RED);
            continue;
        } else {
            bordercolor(COLOR_BLACK);
        }

        /* hide sprite 0 */
        VIC.spr_ena = 0x00;

        /* verify if graphics is initialized */
        if (!graphicsInitialized) {

            /* mark graphics as initialized */
            graphicsInitialized = true;

            /* initialize multi-color graphics mode */
            initGraphics();
        }

        /* empty keyboard buffer */
        clearkey();

        /* retrieve and render KOALA image from RAM */
        renderKOA();

        /* wait until a key is pressed */
        waitakey();

        /* next file name */
        index++;
    }

    /* done */
    return 0;
}
