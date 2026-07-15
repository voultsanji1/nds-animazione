#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <string.h>
#include <stdarg.h>

#define NUM_SPRITES 8
#define SPRITE_SIZE 32
#define LOG_FILE "fat:/nds_anim.log"

static FILE *log_file = NULL;

void log_init(void) {
    if (log_file == NULL) {
        log_file = fopen(LOG_FILE, "w");
        if (log_file) {
            fprintf(log_file, "=== NDS Animazione - Log ===\n");
            fprintf(log_file, "Build: %s %s\n", __DATE__, __TIME__);
            fprintf(log_file, "===========================\n\n");
            fflush(log_file);
        }
    }
}

void log_close(void) {
    if (log_file) {
        fprintf(log_file, "\n=== Fine sessione ===\n");
        fflush(log_file);
        fclose(log_file);
        log_file = NULL;
    }
}

void log_write(const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (log_file) {
        fprintf(log_file, "%s\n", buf);
        fflush(log_file);
    }
    iprintf("%s\n", buf);
}

static u16 *spriteGfx[4];
static int spriteX[NUM_SPRITES];
static int spriteY[NUM_SPRITES];
static int spriteVX[NUM_SPRITES];
static int spriteVY[NUM_SPRITES];
static int spriteFrame[NUM_SPRITES];

void create_sprite_frames(void) {
    for (int i = 0; i < 4; i++) {
        spriteGfx[i] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
        if (!spriteGfx[i]) {
            log_write("[ERROR] Errore allocazione gfx sprite %d", i);
            continue;
        }
        for (int py = 0; py < 32; py++) {
            for (int px = 0; px < 32; px++) {
                int idx = py * 16 + (px / 2);
                u8 pixel = 0;
                switch (i) {
                    case 0: if ((px + py) % 8 < 4) pixel = 1; break;
                    case 1: if ((px * py) % 7 < 3) pixel = 2; break;
                    case 2: if (abs(px - 16) + abs(py - 16) < 12) pixel = 3; break;
                    case 3: if (((px / 4) + (py / 4)) % 2 == 0) pixel = 1; break;
                }
                if (px % 2 == 0)
                    spriteGfx[i][idx] = pixel | (pixel << 8);
                else
                    spriteGfx[i][idx] = (spriteGfx[i][idx] & 0xFF) | (pixel << 8);
            }
        }
    }
    log_write("[INFO] Frame sprite creati (4 pattern)");
}

void load_palettes(void) {
    u16 palette[16] = {
        0x0000,
        RGB15(31, 0, 0), RGB15(0, 31, 0), RGB15(0, 0, 31),
        RGB15(31, 31, 0), RGB15(31, 0, 31), RGB15(0, 31, 31),
        RGB15(31, 31, 31), RGB15(16, 16, 16), RGB15(8, 8, 8),
        RGB15(24, 8, 8), RGB15(8, 24, 8), RGB15(8, 8, 24),
        RGB15(24, 24, 8), RGB15(24, 8, 24), RGB15(8, 24, 24),
    };
    dmaCopy(palette, SPRITE_PALETTE, 32);
    log_write("[INFO] Palette caricate");
}

int main(void) {
    consoleDemoInit();
    log_init();

    log_write("NDS Animazione Sprite - Avvio");
    log_write("Premi START per uscire");

    if (!fatInitDefault()) {
        log_write("[ERROR] FatInit fallito - log su file non disponibile");
    } else {
        log_write("[INFO] Filesystem FAT inizializzato");
    }

    videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);

    vramSetBankA(VRAM_A_MAIN_SPRITE);
    vramSetBankD(VRAM_D_SUB_SPRITE);

    oamInit(&oamMain, SpriteMapping_1D_128, false);
    oamInit(&oamSub, SpriteMapping_1D_128, false);

    log_write("[INFO] VRAM e OAM configurati");

    load_palettes();
    create_sprite_frames();

    for (int i = 0; i < NUM_SPRITES; i++) {
        spriteX[i] = (i % 4) * 60 + 10;
        spriteY[i] = (i / 4) * 55 + 10;
        spriteVX[i] = (i % 2 == 0 ? 1 : -1) * (1 + (i % 3));
        spriteVY[i] = (i % 3 == 0 ? 1 : -1) * (1 + (i % 2));
        spriteFrame[i] = i % 4;
    }

    log_write("[INFO] %d sprite inizializzati", NUM_SPRITES);
    log_write("[INFO] Avvio animazione...");

    int frame_count = 0;

    while (1) {
        scanKeys();
        u16 keys = keysHeld();

        if (keys & KEY_START) {
            log_write("[INFO] Premuto START - uscita");
            break;
        }

        for (int i = 0; i < NUM_SPRITES; i++) {
            spriteX[i] += spriteVX[i];
            spriteY[i] += spriteVY[i];

            if (spriteX[i] <= 0 || spriteX[i] >= 256 - SPRITE_SIZE) {
                spriteVX[i] = -spriteVX[i];
                spriteX[i] += spriteVX[i];
            }
            if (spriteY[i] <= 0 || spriteY[i] >= 192 - SPRITE_SIZE) {
                spriteVY[i] = -spriteVY[i];
                spriteY[i] += spriteVY[i];
            }

            oamSet(&oamMain, i,
                   spriteX[i], spriteY[i],
                   0, spriteFrame[i],
                   SpriteSize_32x32,
                   SpriteColorFormat_16Color,
                   spriteGfx[spriteFrame[i]],
                   -1, false, false,
                   spriteVX[i] > 0, false, false);
        }

        oamUpdate(&oamMain);

        frame_count++;
        if (frame_count % 60 == 0) {
            log_write("[DEBUG] Frame %d - sprite attivi: %d", frame_count, NUM_SPRITES);
        }

        swiWaitForVBlank();
    }

    log_write("[INFO] Chiusura programma");
    log_close();

    oamClear(&oamMain, 0, 128);
    oamUpdate(&oamMain);

    return 0;
}