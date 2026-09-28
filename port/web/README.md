# Web build (WebAssembly)

The native port (docs/PORT.md, docs/SHIM.md, docs/VCL_PORT.md), compiled with Emscripten so that it runs in a browser, iPhone Safari included. It uses the same sources as `make native`: every `src_match/*.cpp`, the port glue, the VCL subset and the Win32 shim. The web-only code is in `#ifdef __EMSCRIPTEN__` blocks and in `port/web/`.

```sh
make -C port web                       # or: python3 port/web/build.py --exe "orig/1999/Elf Bowling.exe"
cd build/web/dist && python3 -m http.server 8000     # then open http://localhost:8000/
```

Emscripten must be on PATH (`emsdk_env`). The flags are in `build.py`, so the build runs the same on hosts without make (emsdk on Windows). `port/web.mk` only calls it.

## Files

| Path | Contents |
|---|---|
| `port/web/build.py` | compile, link and assemble `build/web/dist` (objects in `build/web/obj`, rebuilt when the source, a header or the flags change) |
| `port/web/index.html`, `loader.js`, `manifest.webmanifest` | the page: letterboxed canvas, tap to start, on-screen buttons, data loading and caching |
| `port/web/web_glue.cpp` | `web_key(sym, down, mod)`, called by the on-screen buttons |
| `port/web/fonts/` | the bundled fonts and their licences (see "Fonts") |
| `port/web.mk` | `make -C port web` / `web-clean` (included from port/Makefile) |
| `port/vcl/thread.cpp`, `forms.cpp`, `main.cpp` | the `__EMSCRIPTEN__` sections: cooperative TThread, browser main loop, no teardown after WinMain |

`build/web/dist` then holds:
- `index.html`, `loader.js`, `manifest.webmanifest`, `elfbowl.js`, `elfbowl.wasm`;
- `data/manifest.json`, `data/elfbowl.exe` (copied from `--exe`) and `data/fonts/*`.

Every URL carries the build id (`?v=`), so a new deploy never mixes old and new files.

## The main loop and the game's thread

On Windows `Application->Run()` loops forever, and the game has one real thread. TMyThread's `Execute` (fn_40f684) is `while (!Terminated) Synchronize(Update)`. A browser's main thread may not block, so neither can stay as it is. The three options:

| Option | Why it was not chosen, or was |
|---|---|
| `-sASYNCIFY` with yields in the VCL loop | It instruments every function that can reach a yield, which is most of the game: larger and slower code. It also does not solve the thread; that would need fibers on top. |
| pthreads (`-pthread`, SharedArrayBuffer) | It needs COOP/COEP headers on every response, and Safari 15.2 or later. The main thread still cannot wait on the Synchronize condition, because `Atomics.wait` is not allowed there, so Run() would also need ASYNCIFY or PROXY_TO_PTHREAD (SDL on a worker, OffscreenCanvas). Too many moving parts for an iPhone. |
| **cooperative TThread (chosen)** | No new runtime machinery and no special headers. It works on any browser with wasm exceptions (iOS 15.2+). |

How it works:

1. `TApplication::Run()` shows the main form and registers `WebFrame` with `emscripten_set_main_loop` (requestAnimationFrame). Then it returns.
2. WinMain has nothing after `Run()` except `return 0`, so it returns. `main()` skips `Vcl::Shutdown` on the web, and the runtime stays alive with all its objects.
3. Each frame, `WebFrame` handles the SDL events, then runs one slice of every started TThread. It then paints the form if it was invalidated, and presents once.
4. A slice enters `Execute()` on the main thread. When `Synchronize(Method)` is called from inside it, Method runs at once, and then a `CoopYield` exception unwinds back to the scheduler. So each frame runs exactly one `Update` (fn_40f674 → fn_40c1b4, the frame tick).
5. The game's timing is by the clock: fn_409dac advances `(now - start) / period` scene ticks and catches up by up to 3 per call. So one Update per animation frame plays at the original speed.
6. `Terminate` followed by the next slice ends `Execute` normally. `WaitFor` runs slices until it does. When the game quits (its own Exit screen), the loop stops and the page offers "Play again".

The limit: an `Execute` that keeps state in local variables across `Synchronize` calls, or that catches `(...)` around them, would break under this scheme. The game's only thread does neither.

## Screen and input

- The SDL window is 640×480 and not resizable, so the canvas's backing store stays 640×480. `loader.js` sets only its CSS size, to the largest 4:3 rectangle that fits, centred. Above 2× it uses crisp pixels.
- SDL scales mouse and touch positions by 640 / CSS width, so they land on the right game pixels. SDL2's Emscripten backend turns a touch on the canvas into a left-button move, press and release (`SDL_HINT_TOUCH_MOUSE_EVENTS`, on by default), and prevents the browser's own gesture. No web code is needed for touch.
- Keys the game uses:

  | Key | Where | Purpose |
  |---|---|---|
  | Space or Enter | score_4120cc fn_412904 | bowl while aiming |
  | Esc | fn_412904, the menus and the about screen | back or exit |
  | Enter | menu, opening, game_41c674 | confirm |
  | Ctrl+X, D, S, G, N | fn_41296c | cheats (keyboard only) |

- On a touch screen the page shows a big **BOWL** button (Space) with **Esc** and **Enter** beside it. It sits below the game in portrait and down the right edge in landscape. They go through `web_key` into the SDL queue, so the game sees them exactly like hardware keys.
- A hint suggests turning the phone sideways.
- Page scrolling, rubber-banding, pinch and double-tap zoom are blocked (`touch-action: none`, the viewport meta, `gesturestart`/`touchmove` handlers).
- iOS home-screen web-app meta and `manifest.webmanifest` give `display: standalone`. Add the page to the home screen for full screen: Safari on iPhone has no Fullscreen API for pages.

## Audio

- WinMM runs on SDL audio as natively. On the web SDL uses a WebAudio `AudioContext`, and its callback (the shim's mixer) runs on the main thread.
- iOS only lets an AudioContext play if it is created or resumed during a user gesture. The page therefore waits for **Tap to play** and calls `main()` inside that tap handler, so SDL opens its audio device during the gesture.
- The page also resumes the context on every later touch or key and when the page becomes visible again, because iOS suspends it in the background. SDL's own `autoResumeAudioContext` does the same.
- `navigator.audioSession.type = 'playback'` lets it play with the ring/silent switch on silent (iOS 17 and later).

## Fonts

The game asks for Arial (five sizes), Lucida Handwriting and Lucida Calligraphy (docs/SHIM.md, "Text"). These are Microsoft fonts and cannot be redistributed. The web build bundles free substitutes, which the shim finds by the file names it already searches for, in `PORT_FONT_DIR=/fonts`:

| Game face | Bundled font | Licence | FS path |
|---|---|---|---|
| Arial, regular and bold | Liberation Sans 2.1.5 Regular and Bold: metric-compatible with Arial, so text lays out the same | SIL Open Font License 1.1 (`fonts/LICENSE-LiberationSans.txt`) | `/fonts/LiberationSans-Regular.ttf`, `-Bold.ttf` |
| Lucida Handwriting, Lucida Calligraphy | TeX Gyre Chorus Medium Italic: a Zapf Chancery design, the same stand-in the shim uses on Linux (URW Chancery L / Z003) | GUST Font License (LPPL 1.3c) (`fonts/LICENSE-TeXGyreChorus.txt`) | `/fonts/Z003-MediumItalic.ttf` |

TeX Gyre Chorus is filed under Z003's name only inside the browser's virtual FS, because that is a name gdi.c's face table already tries. The published file keeps its real name, `data/fonts/texgyrechorus-mediumitalic.otf`. Chancery is narrower than the Lucida faces, so a line in those faces can wrap differently from the original.

## Game data

- `Elf Bowling.exe` is not in the repo or in the wasm. `build.py --exe` copies it to `dist/data/elfbowl.exe`.
- `data/manifest.json` lists each data file with the path where the game reads it (`/data/Elf Bowling.exe` = `ELFBOWL_EXE`, and the fonts).
- `loader.js` fetches each file once and keeps it in IndexedDB (IDBFS at `/cache`), keyed by the manifest version. It then symlinks each file to its path. Later visits start without downloading.
- `game_data.cpp` (the `.data` initial values) is generated from the same exe at build time, as `make native` does.

### Hi-res art (the "hires" pack)

`manifest.packs.hires` is the ESRGAN art cache (`build/hires/x3`, from `make hires`; never committed).

- `build.py --hires DIR` (default `build/hires/x3`, only when it has `.done`) copies it to `dist/data/hires/x3/` and lists it as `{"version", "files": [{"name", "path", "size"}]}`, paths under `/hires/x3/`. It sits under the same cookie gate as everything else.
- `loader.js` fetches every pack 3 s after the game starts, caches it the same way, links it into the FS, and calls `web_pack_ready(name)` (web_glue.cpp).
- For `hires` that calls `hires_activate("/hires/x3", scale)`: the casts loaded before the pack arrived are registered then (hires.c keeps a list while it waits), and the canvas's backing store grows to the hi-res frame (`port_hires_window`).
- Scale: 3x; 2x on phones (touch and a short side under 600 CSS px) or when `navigator.deviceMemory` is 4 GB or less (`$ELFBOWL_HIRES_SCALE`). hires.c resamples the 3x art to 2x as it loads it, so the canvases and art take about 4/9 of the memory.
- The **HD** button (top right of the game) appears then and toggles the art and the classic 1x picture (`web_hd`). The choice is kept in localStorage.
- The game starts at 1x and never waits for the pack. Text drawn before the pack arrived (the first story page) stays 3x nearest until it is drawn again.

## Hosting

Serve `build/web/dist/` from any static host. The page, wasm and data are all static files. Put the game's exe (supplied by you) where `data/manifest.json` expects it, or add a first-run picker.
