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
#include <peekpoke.h>

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <stdio.h>



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
#define BITMAP_ADDRESS       0x2000
#define SCREEN_ADDRESS       0x0400
#define COLMAP_ADDRESS       0xd800
#define BKGCOL_ADDRESS       0xd021


#define DEVICE_NUM                8


unsigned char* FILES[] = { "ylenia" };

int main() 
{

    unsigned char* imageBuffer = (unsigned char*)0x6000;

    uint32_t result = 0;

    printf("LOADING: '%s'\n", FILES[0]);

    result = cbm_load(FILES[0], DEVICE_NUM ,(void*)0);
    if (result == 0) {
        printf("ERROR: %d\n", _oserror);
        return 0;
    }
    
    /* load bitmap data */
    memcpy((void*)BITMAP_ADDRESS, (void*)(imageBuffer+BITMAP_FILE_OFFSET), BITMAP_FILE_LENGHT);

    /* load screen data */
    memcpy((void*)SCREEN_ADDRESS, (void*)(imageBuffer+SCREEN_FILE_OFFSET), SCREEN_FILE_LENGTH);

    /* load color map */
    memcpy((void*)COLMAP_ADDRESS, (void*)(imageBuffer+COLMAP_FILE_OFFSET), COLMAP_FILE_LENGTH);

    /* load background colour */
    POKE(BKGCOL_ADDRESS, imageBuffer[BKGCOL_FILE_OFFSET]);

        /* enable bitmap mode */
    POKE(0xd011, 0x3b); 

    /* enable multicolor graphic-mode */
    POKE(0xd016, 0x18); 

    /* set screen at $0400 and bitmap at $2000 */
    POKE(0xd018, 0x1f);

    /* set black border */ 
    POKE(0xd020, 0x00);

    /* never-ending loop */
    while (true) 
        ;

    /* done */
    return 0;
}
