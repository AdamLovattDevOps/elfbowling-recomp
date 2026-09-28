// loader.js: get the game data into the Emscripten file system, start the game on a tap, fit the
// canvas to the screen, and drive the on-screen buttons. See port/web/README.md.
//
// Data: data/manifest.json lists the files ({name, path, size}); each is fetched once and kept in
// IndexedDB (IDBFS at /cache), keyed by the manifest's version, so later visits start offline-fast.
// `path` is where the game reads it (ELFBOWL_EXE, PORT_FONT_DIR). manifest.packs holds optional
// sets that are fetched after the game has started and cached the same way ("hires": the ESRGAN
// art, build/hires/x3); when one is in the FS the page calls the exported C function
// web_pack_ready(name). For "hires" that switches the renderer to the art (hires.c) and shows the
// HD button, which toggles it (web_hd). Phones use 2x canvases instead of 3x, to save memory.
// Everything is inside one function: elfbowl.js is a classic script with many globals of its own
// (FS, ENV, ...), and only `Module` may be shared with it.
(() => {
'use strict';

const BUILD = '@BUILD@';
const $ = (id) => document.getElementById(id);
const status = (t) => { $('status').textContent = t; };
const bar = (f) => { $('bar').firstElementChild.style.width = (100 * Math.max(0, Math.min(1, f))).toFixed(1) + '%'; };
const canvas = $('canvas');
const touch = matchMedia('(pointer: coarse)').matches || navigator.maxTouchPoints > 0;
if (touch) document.body.classList.add('touch');
// the hi-res canvases take ~230 MB at 3x (docs/HIRES.md); phones and small-memory devices use 2x
const phone = touch && Math.min(screen.width, screen.height) < 600;
const HIRES_SCALE = (phone || (navigator.deviceMemory && navigator.deviceMemory <= 4)) ? 2 : 3;
const store = { get: (k) => { try { return localStorage.getItem(k); } catch (_) { return null; } },
                set: (k, v) => { try { localStorage.setItem(k, v); } catch (_) {} } };
let hdReady = false, hdOn = store.get('elfbowling-hd') !== '0';

// ---- no page scrolling, rubber-banding or zooming -------------------------------------------------
for (const ev of ['gesturestart', 'gesturechange', 'gestureend']) document.addEventListener(ev, (e) => e.preventDefault(), { passive: false });
document.addEventListener('touchmove', (e) => e.preventDefault(), { passive: false });
document.addEventListener('dblclick', (e) => e.preventDefault(), { passive: false });

// ---- fit: the largest 4:3 rectangle in the stage, never stretched ---------------------------------
// The canvas keeps its 640x480 backing store (the SDL window); only its CSS size changes. SDL scales
// mouse and touch positions by 640 / CSS width, so they land on the right game pixels.
function fit() {
  const st = $('stage');
  const s = Math.min(st.clientWidth / 640, st.clientHeight / 480);
  if (!(s > 0)) return;
  canvas.style.width = Math.floor(640 * s) + 'px';
  canvas.style.height = Math.floor(480 * s) + 'px';
  canvas.classList.toggle('px', s >= 2 && !(hdReady && hdOn));   // crisp 1x pixels once each is 2+ screen pixels
}
addEventListener('resize', fit);
addEventListener('orientationchange', () => setTimeout(fit, 300));
if (window.visualViewport) visualViewport.addEventListener('resize', fit);
fit();

// ---- audio: iOS only plays WebAudio started or resumed inside a user gesture ---------------------
function audioCtx() { return Module.SDL2 && Module.SDL2.audioContext; }
function unlockAudio() {
  const ctx = audioCtx();
  if (ctx && ctx.state !== 'running' && ctx.state !== 'closed') ctx.resume().catch(() => {});
}
for (const ev of ['pointerdown', 'touchend', 'keydown']) document.addEventListener(ev, unlockAudio, { capture: true, passive: true });
document.addEventListener('visibilitychange', () => { if (!document.hidden) unlockAudio(); });
// iOS 17+: play through the ring/silent switch, like a game app
try { if (navigator.audioSession) navigator.audioSession.type = 'playback'; } catch (_) {}

// ---- data -----------------------------------------------------------------------------------------
let FS;
const syncfs = (populate) => new Promise((res) => FS.syncfs(populate, (err) => { if (err) console.warn('syncfs', err); res(); }));
const key = (name) => '/cache/' + name.replace(/\//g, '__');

function mkdirs(path) {
  const parts = path.split('/').slice(1, -1); let p = '';
  for (const d of parts) { p += '/' + d; try { FS.mkdir(p); } catch (_) {} }
}

// Fetch the files of one set (the base data or a pack) unless /cache already holds that version,
// then link each to its path. onProgress(fraction) while downloading.
async function loadSet(tag, set, onProgress) {
  const vfile = '/cache/.version-' + tag;
  let have = '';
  try { have = FS.readFile(vfile, { encoding: 'utf8' }); } catch (_) {}
  const cached = have === set.version && set.files.every((f) => FS.analyzePath(key(f.name)).exists);
  if (!cached) {
    const total = set.files.reduce((a, f) => a + f.size, 0) || 1; let got = 0;
    for (const f of set.files) {
      const r = await fetch('data/' + f.name.split('/').map(encodeURIComponent).join('/') + '?v=' + set.version, { cache: 'no-store', credentials: 'same-origin' });
      if (!r.ok) throw new Error('could not load ' + f.name + ' (' + r.status + ')');
      const reader = r.body.getReader(); const parts = []; let n = 0;
      for (;;) {
        const { done, value } = await reader.read();
        if (done) break;
        parts.push(value); n += value.length; got += value.length;
        if (onProgress) onProgress(got / total);
      }
      const buf = new Uint8Array(n); let o = 0;
      for (const p of parts) { buf.set(p, o); o += p.length; }
      FS.writeFile(key(f.name), buf);
    }
    FS.writeFile(vfile, set.version);
    await syncfs(false);
  }
  for (const f of set.files) {
    mkdirs(f.path);
    try { FS.unlink(f.path); } catch (_) {}
    FS.symlink(key(f.name), f.path);
  }
  return !cached;
}

let manifest = null;
async function prepare() {
  FS = Module.FS;
  try { FS.mkdir('/cache'); } catch (_) {}
  FS.mount(Module.IDBFS || FS.filesystems.IDBFS, {}, '/cache');
  await syncfs(true);
  const r = await fetch('data/manifest.json?b=' + BUILD + '&t=' + Date.now(), { cache: 'no-store', credentials: 'same-origin' });
  if (!r.ok) throw new Error('no game data on this server (' + r.status + ')');
  manifest = await r.json();
  if (!manifest.files.some((f) => f.path === '/data/Elf Bowling.exe')) throw new Error('the server has no Elf Bowling.exe');
  status('Loading the game…');
  await loadSet('base', manifest, bar);
  bar(1);
  status(touch ? 'Ready.' : 'Ready. Press Space or Enter to bowl.');
  $('go').classList.add('show');
}

async function loadPacks() {
  for (const [name, pack] of Object.entries((manifest && manifest.packs) || {})) {
    try {
      await loadSet('pack-' + name, pack, null);
      const on = Module['_web_pack_ready'] && Module.ccall('web_pack_ready', 'number', ['string'], [name]);
      if (name === 'hires' && on) {
        hdReady = true;
        Module.ccall('web_hd', null, ['number'], [hdOn ? 1 : 0]);
        showHd();
        console.log('hires: on at ' + HIRES_SCALE + 'x, canvas ' + canvas.width + 'x' + canvas.height);
      }
    } catch (e) { console.warn('pack', name, e); }
  }
}

// ---- HD button: the ESRGAN art (hires.c) or the classic 1x picture ------------------------------
function showHd() {
  const b = $('hd');
  b.classList.add('show');
  b.setAttribute('aria-pressed', hdOn ? 'true' : 'false');
  b.textContent = hdOn ? 'HD on' : 'HD off';
  fit();
}
$('hd').addEventListener('click', (e) => {
  e.preventDefault();
  if (!hdReady) return;
  hdOn = !hdOn;
  store.set('elfbowling-hd', hdOn ? '1' : '0');
  Module.ccall('web_hd', null, ['number'], [hdOn ? 1 : 0]);
  showHd();
  canvas.focus();
});

// ---- start ----------------------------------------------------------------------------------------
let started = false;
$('go').addEventListener('click', () => {
  if (started) { location.reload(); return; }
  started = true;
  document.body.classList.add('playing');
  fit();
  canvas.focus();
  // main() runs inside this tap, so SDL creates its AudioContext during a user gesture (iOS).
  try { Module.callMain([]); } catch (e) { if (e !== 'unwind' && !(e && e.name === 'ExitStatus')) fail(e); }
  unlockAudio();
  fit();
  setTimeout(loadPacks, 3000);
});

function fail(e) {
  console.error(e);
  document.body.classList.remove('playing');
  $('go').classList.remove('show');
  status('Sorry, the game stopped: ' + String((e && e.message) || e));
}

// ---- on-screen buttons: a key held while touched --------------------------------------------------
let webKey = null;
for (const b of document.querySelectorAll('#pad button')) {
  const sym = +b.dataset.sym;
  const down = (e) => {
    e.preventDefault();
    if (b.classList.contains('on')) return;
    if (!webKey) webKey = Module.cwrap('web_key', null, ['number', 'number', 'number']);
    try { b.setPointerCapture(e.pointerId); } catch (_) {}
    b.classList.add('on'); webKey(sym, 1, 0);
  };
  const up = (e) => {
    e.preventDefault();
    if (!b.classList.contains('on')) return;
    b.classList.remove('on'); webKey && webKey(sym, 0, 0);
  };
  b.addEventListener('pointerdown', down);
  for (const ev of ['pointerup', 'pointercancel', 'lostpointercapture']) b.addEventListener(ev, up);
  b.addEventListener('contextmenu', (e) => e.preventDefault());
}
// the stage resizes when the pad appears and on rotation
if (window.ResizeObserver) new ResizeObserver(fit).observe($('stage'));

// ---- the Emscripten module ------------------------------------------------------------------------
const Module = {
  noInitialRun: true,
  canvas,
  locateFile: (path) => path + '?v=' + BUILD,
  print: (t) => console.log(t),
  printErr: (t) => console.warn(t),
  preRun: [() => {
    // before static constructors: environ is built from ENV on its first use
    const env = Module.ENV || window.ENV;
    env.ELFBOWL_EXE = '/data/Elf Bowling.exe';
    env.PORT_FONT_DIR = '/fonts';
    env.PORT_NO_MSGBOX = '1';               // MessageBox logs to the console instead of alert()
    env.ELFBOWL_HIRES_SCALE = String(HIRES_SCALE);
  }],
  onRuntimeInitialized: () => { prepare().catch(fail); },
  onAbort: (what) => fail(new Error('aborted: ' + what)),
  // the game quit (its own Exit screen): forms.cpp WebFrame
  onGameExit: () => {
    document.body.classList.remove('playing');
    status('Thanks for playing!'); $('go').textContent = 'Play again'; $('go').classList.add('show');
  },
};
window.Module = Module;
window.addEventListener('error', (e) => { if (started) fail(e.error || e.message); });

const s = document.createElement('script');
s.src = 'elfbowl.js?v=' + BUILD;
s.onerror = () => fail(new Error('could not load elfbowl.js'));
document.body.appendChild(s);
})();
