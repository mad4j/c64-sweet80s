/**
 * sweet80s.c
 * https://github.com/mad4j/c64-sweet80s
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


/* KOALA image file format */
#define KAOLA_BITMAP_OFFSET             0
#define KAOLA_BITMAP_LENGHT          8000
#define KAOLA_SCREEN_OFFSET          8000
#define KAOLA_SCREEN_LENGTH          1000
#define KAOLA_COLMAP_OFFSET          9000
#define KAOLA_COLMAP_LENGTH          1000
#define KAOLA_BKGCOL_OFFSET         10000
#define KAOLA_BKGCOL_LENGTH             1

/* KOALA RAM address */
#define KAOLA_RAM_ADDRESS           ((uint8_t*)0x6000)

/* Multi-color RAM addresses */
#define BITMAP_RAM_ADDRESS          ((uint8_t*)0x2000)
#define SCREEN_RAM_ADDRESS          ((uint8_t*)0x0400)
#define COLMAP_RAM_ADDRESS          ((uint8_t*)0xD800)

/* Sprite data pointers base address*/
#define SPRITE_DATA_PTR_RAM_ADDRESS (SCREEN_RAM_ADDRESS+1016)

/* Sprite related infomration */
#define SPRITE_MEMORY_SLOT_SIZE       64

#define SPRITE_0_MEMORY_SLOT          13
#define SPRITE_0_POS_X               306
#define SPRITE_0_POS_Y               220

/* disk drive number */
#define LAST_DEVICE_NUM_RAM_ADDRESS  186

/* file system constants */
#define MAX_FILES                     25
#define MAX_FILE_NAME_LEN             17

/* names of image files on disk */
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


/**
 * initialize multi-color graphic mode
 */
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
    memset(BITMAP_RAM_ADDRESS, 0x00, KAOLA_BITMAP_LENGHT);
}


/**
 * load an image stored using KOALA file format
 */
uint32_t loadKOA(const char* fileName)
{
    /* retrieve last used device number */
    uint8_t device = PEEK(LAST_DEVICE_NUM_RAM_ADDRESS);

    int32_t result = 0;

    /* load image data at specific address ignoring embedded PRG info */
    result = cbm_load(fileName, device, KAOLA_RAM_ADDRESS);

    return (result == 0) ? _oserror : 0;
}


/**
 * display a KAOLA image store in RAM
 */
void renderKOA() 
{
    /* load bitmap data */
    memcpy(BITMAP_RAM_ADDRESS, KAOLA_RAM_ADDRESS+KAOLA_BITMAP_OFFSET, KAOLA_BITMAP_LENGHT);

    /* load screen data */
    memcpy(SCREEN_RAM_ADDRESS, KAOLA_RAM_ADDRESS+KAOLA_SCREEN_OFFSET, KAOLA_SCREEN_LENGTH);

    /* load color map */
    memcpy(COLMAP_RAM_ADDRESS, KAOLA_RAM_ADDRESS+KAOLA_COLMAP_OFFSET, KAOLA_COLMAP_LENGTH);

    /* load background colour */
    VIC.bgcolor0 = KAOLA_RAM_ADDRESS[KAOLA_BKGCOL_OFFSET];

    /* set black border */ 
    VIC.bordercolor = COLOR_BLACK;
}


/**
 * configure and initialize icon data
 */
void initIcons()
{
    /* hide sprite 0 */
    VIC.spr_ena = 0x00;

    /* update sprite 0 data memory pointer */
    POKE(SPRITE_DATA_PTR_RAM_ADDRESS, SPRITE_0_MEMORY_SLOT);

    /* sprite 0 data */
    memcpy(SPRITE_MEMORY_SLOT_SIZE*SPRITE_0_MEMORY_SLOT, floppy_bin, floppy_bin_len);

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

    /* retrieve last used device number */
    uint8_t device = PEEK(LAST_DEVICE_NUM_RAM_ADDRESS);

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
        loadKOA(files[index%filesIndex]);

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

        /* next file name */
        index++;

        /* wait until a key is pressed */
        waitakey();
    }

    /* done */
    return 0;
}
