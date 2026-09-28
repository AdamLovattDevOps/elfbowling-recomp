# High-resolution art and fit-to-window (native port)

The native build can show the game at 3x (1920x1440) with the bitmaps redrawn by an AI upscaler, and scales the picture to any window size. The 1x game is unchanged underneath: F9 or `ELFBOWL_CLASSIC=1` shows it.

## Build the art cache

```sh
make hires        # python3 tools/upscale.py -> build/hires/x3 (about 40 MB, about 2 minutes on an M-series GPU)
```

- Needs Pillow, numpy, and realesrgan-ncnn-vulkan. The default is `build/deps/esrgan`, downloaded automatically on first use ; `--esrgan DIR` picks another.
- Reads the art from your own exe (`--exe`, default `$ELFBOWL_EXE` or `orig/1999/Elf Bowling.exe`). Nothing it writes is committed.
- Resumable: finished images are skipped. The 4x intermediates are deleted batch by batch.

For each of the 141 bitmaps in NVDPACKFILE (tools/unpack.py's logic):

| File | Contents |
|---|---|
| `NAME.png` | 3x RGBA. Alpha is the game's mode-1 matte (fn_403f04: white connected to the border is transparent). The colour is bled into the transparent area before upscaling, so edges never pick up the white background. |
| `NAME.m2.png` | only if different: the mode-2 mask (fn_4045f0: every white pixel transparent) |
| `NAME.op.png` | only if the bitmap has white: the plain opaque picture, for casts drawn without a mask |
| `NAME.idx` | the original 1x indices (`EIDX`, w, h, pixels top-down), used to check the cast and to rebuild the masks |
| `manifest.tsv` | name, size, CRC-32 of the indices, variants |

How the images are made:
- Colour: Real-ESRGAN 4x with `realesrgan-x4plus-anime`. The bitmaps that are mostly small type (addresses, "Click here" and so on; the `TEXT` set in upscale.py) use `realesr-animevideov3-x4` instead, which keeps letters legible.
- Type only a few pixels tall (the copyright line under the logo, the NStorm buttons; the `SMALL_TEXT` set) skips the model: both models turn some of those letters into other letters. These are plain Lanczos 3x of the bled colour and of the mask. To rebuild one: delete its `NAME*.png` and run `tools/upscale.py --only NAME.bmp` (no ESRGAN needed).
- Then Lanczos to 3x. The colour's low frequencies are pinned to the original (a Gaussian of the difference), because the model tints flat areas by a few levels, which would show as boxes.
- Mask: the 1x mask goes through the model as its own image, which gives smooth contours. It is then tightened with smoothstep(0.2, 0.8) to a ramp about one source pixel wide.

## Runtime (port/src/hires.c)

The game composites in software on 8-bit DIB bits (fn_402ed4 and the fills), then BitBlts TStage's back buffer to the screen. The port keeps a 3x RGBA **canvas** beside each DIB that reaches the screen (the screen, and TStage's back and front buffers) and mirrors every draw into it.

| 1x operation | 3x mirror |
|---|---|
| cast load (fn_40d980) | The cast is registered with its asset, its X/Y flips and its crop rectangle. Its loaded pixels are compared with the crop and flip of `NAME.idx`, which checks both the asset name and the pixels; if they differ, no art is used. |
| `#n` scaled copies (fn_402bb8) | registered as every n-th pixel of the source cast; the 3x image is the source's 3x art box-filtered by n |
| SRCCOPY blit (fn_402ed4, BitBlt) | copies 3x blocks from the cast's image (`op` variant, or the art over white), or from the source canvas (back buffer to screen) |
| SRCAND mask, then SRCPAINT sprite at the same place | Alpha composite of the cast's RGBA. The variant whose 1x mask matches the runtime mask best is used, and any pixel where they disagree gets a hard 3x-nearest block. Each 1x pixel is accepted only if the 1x result is what a composite means: the sprite pixel where the mask is opaque, the old pixel elsewhere. Otherwise the block follows the 1x result (index OR maths, such as keepBg text). |
| fills (fn_40343c, fn_403048, FillRect) | the 3x rectangle is filled with the same colour (exact) |
| DrawTextA (OPAQUE) | gdi.c draws each line again with the same face and style at 3x size (SDL2_ttf, antialiased), placed on the 1x line and clipped to the 3x clip. The 1x stays authoritative: the line is used only if its ink spans what the 1x ink spans (within 2 px, 2% more across), and each 3x block only while its 1x pixel keeps the index the text left there (or, under a mask that hides it, the background the game set to the transparent index). Fills and blits over the pixels drop it. |
| anything else: crops, flood fills, unknown ROPs, pixels poked by code | Each canvas keeps `ref`, the 1x index each 3x block was made from. Any pixel whose index no longer matches is redrawn as a 3x nearest block, before the canvas is read and at every present. Cast images keep the same record, so pixels that code changed (text casts) fall back to nearest where they show. |

- **Palette:** canvases hold colours through the game's one palette (the palette of the first loaded bitmap). At present the realized system palette is compared with it. If they are equal, the canvas is shown. If it is a uniform scale (a fade), it is shown with a texture colour modulation. Any other difference shows the classic frame. The game does not fade, so today only the start-up frames, before the palette is realized, show classic.
- **Hooks:** the hooks in src_match (range_401d0c: fn_402ed4, fn_402bb8, fn_403048; range_403540: fn_40343c; packres_40d980) are `#ifdef PORT` only. `make verify` stays 650/650 with 0 xref wrong.

## Window

- The window is resizable. The renderer's logical size (the screen, 640x480) letterboxes the picture at any size and aspect, and SDL maps mouse events back to game coordinates. The 3x texture is sampled with linear filtering; the classic 1x texture uses nearest.
- Alt+Enter or F11 toggles full screen (desktop mode). F9 toggles the 3x art and the classic frame.
- With the art cache present, the first window opens at up to 2x (85% of the display); `PORT_WINDOW_SIZE=WxH` overrides this.
- Only the texture rows that changed are uploaded each frame.

## Web

The web build ships the cache as its `hires` pack and turns the art on when it arrives, at 3x, or 2x on phones; an HD button toggles it. See port/web/README.md, "Hi-res art".

## Environment

| Variable | Effect |
|---|---|
| `ELFBOWL_CLASSIC=1` | Start with the classic 1x frame. The canvases are not kept until F9, which then builds them from the 1x frame as it is redrawn. |
| `ELFBOWL_HIRES=DIR` | the cache (DIR or DIR/x3); default `build/hires/x3`, then `hires/x3` beside the executable |
| `ELFBOWL_HIRES_LATE=N[,S]` | tests: start with no art and turn it on at present N, at scale S (3 or 2), as the web build does when its pack arrives (`hires_activate`) |
| `PORT_WINDOW_SIZE`, `PORT_DUMP_WINDOW`, `PORT_DUMP_1X`, `PORT_WINPUT` | tests; see docs/SHIM.md |

Frame dumps (`PORT_DUMP_FRAMES`) write what is presented: 1920x1440 when the art shows.

## Testing

```sh
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy PORT_DUMP_FRAMES=200 PORT_DUMP_1X=1 PORT_DUMP_DIR=scratch/hires/cmp \
  PORT_INPUT="8000:click:240,430;14000:key:32" timeout 26 build/elfbowl
# letterbox: a 1920x1080 window, PLAY clicked in window coordinates (240 + 240*2.25, 430*2.25)
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy PORT_WINDOW_SIZE=1920x1080 PORT_DUMP_WINDOW=1 PORT_DUMP_FRAMES=300 \
  PORT_WINPUT="8000:click:780,967" timeout 16 build/elfbowl
```

At exit the shim logs counters: casts registered and matched, art and nearest images, composites, and pixels that followed the 1x.

## Known limits

- Text drawn at run time follows the 1x layout, so the 3x glyphs can sit up to a pixel or two from where a native 3x layout would put them; a line whose 3x ink does not match the 1x stays 3x nearest.
- The `#4` far-lane elves are the 3x art box-filtered, so they look smoother than the aliased 1x subsample. This is correct, but it differs from 1x in detail.
- Composites are clipped to the cast's bounds, so a smooth edge that lies exactly on the bounding box is cut straight.
- Memory: about 230 MB peak with the art (asset images are kept once loaded), against about 50 MB classic.
