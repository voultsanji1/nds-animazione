# NDS Animazione Sprite

Animazione sprite per Nintendo DS usando devkitARM + libnds.

## Requisiti

- devkitARM (devkitPro)
- libnds
- libfat

### Installazione (Windows)
```bash
# Via pacman (MSYS2/devkitPro)
pacman -S nds-dev
```

### Installazione (Linux/macOS)
```bash
# Via pacman
sudo dkp-pacman -S nds-dev
```

Oppure scarica l'installer da [devkitPro.org](https://devkitpro.org/wiki/Getting_Started).

## Build

```bash
# Configura environment
export DEVKITARM=/opt/devkitpro/devkitarm

# Compila
make
```

L'output sarà in `animazione.nds`.

## Esecuzione

- **Emulatore**: melonDS, DeSmuME, NO$GBA
- **Hardware**: Flashcart (R4, Acekard, EZ-Flash, etc.) su DS/DSi/3DS

## Controlli

| Tasto | Azione |
|-------|--------|
| START | Esci dal programma |

## Struttura

```
.
├── Makefile              # Build system devkitARM
├── source/
│   └── main.c            # Codice principale animazione sprite
├── graphics/             # Asset grafici
├── build/                # Output build (gitignored)
└── .github/workflows/    # CI/CD GitHub Actions
```

## Sprite Animation

Il demo mostra 8 sprite 32x32 che rimbalzano sui bordi dello schermo con animazione a 4 frame.

- **Video Mode**: Mode 0 2D (main + sub)
- **Sprite**: 1D mapping, 16 colori, 32x32
- **VRAM**: Bank A (main sprite), Bank D (sub sprite)
- **Palette**: 16 colori condivisi

## CI/CD

GitHub Actions compila automaticamente ad ogni push e crea artifact scaricabile.

Per creare una release con il file `.nds`:
```bash
git tag v1.0.0
git push origin v1.0.0
```

## Licenza

MIT