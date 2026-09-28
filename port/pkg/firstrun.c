/* First run for the packages (pkg/README.md): find the user's original Elf Bowling.exe, check it,
 * and keep a copy in the app's own storage. No package ships the exe or anything from it.
 *
 * Looked for, in order: the command-line path, $ELFBOWL_EXE, the copy in app storage
 * (SDL_GetPrefPath), next to the app (SDL_GetBasePath), the working directory, and the
 * platform's user-visible folder (iOS Documents, Android external files dir). If none is the
 * right file, the platform's file picker asks for it (elfbowl_pick_file: pick_win.c, pick_linux in
 * this file, ios/bowl_button.m, android/ElfBowlActivity.java through JNI here). SDL2 has no
 * SDL_ShowOpenFileDialog (that is SDL3), hence the native pickers. */
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __ANDROID__
#include <jni.h>
#elif defined(__linux__)
#include <sys/wait.h>
#endif

#include "firstrun.h"

static const char k_sha256[] = "dceb5b89544b20744091f55c8ec49d2baaf59bf530ce50a4c2a14d12b069de0c";
#define EXE_NAME "Elf Bowling.exe"
#define WANT "the original Elf Bowling.exe (Elf Bowling, NStorm 1999; 1,130,496 bytes, SHA-256 " \
             "dceb5b89544b20744091f55c8ec49d2baaf59bf530ce50a4c2a14d12b069de0c)"

/* ---- SHA-256 (FIPS 180-4) ------------------------------------------------------------------ */
typedef struct { Uint32 h[8]; Uint64 len; unsigned char buf[64]; size_t n; } Sha;
static const Uint32 K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
#define ROR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
static void sha_block(Sha *s, const unsigned char *p)
{
    Uint32 w[64], a[8];
    for (int i = 0; i < 16; i++)
        w[i] = (Uint32)p[4 * i] << 24 | (Uint32)p[4 * i + 1] << 16 | (Uint32)p[4 * i + 2] << 8 | p[4 * i + 3];
    for (int i = 16; i < 64; i++) {
        Uint32 s0 = ROR(w[i - 15], 7) ^ ROR(w[i - 15], 18) ^ (w[i - 15] >> 3);
        Uint32 s1 = ROR(w[i - 2], 17) ^ ROR(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    memcpy(a, s->h, sizeof a);
    for (int i = 0; i < 64; i++) {
        Uint32 t1 = a[7] + (ROR(a[4], 6) ^ ROR(a[4], 11) ^ ROR(a[4], 25)) + ((a[4] & a[5]) ^ (~a[4] & a[6])) + K[i] + w[i];
        Uint32 t2 = (ROR(a[0], 2) ^ ROR(a[0], 13) ^ ROR(a[0], 22)) + ((a[0] & a[1]) ^ (a[0] & a[2]) ^ (a[1] & a[2]));
        memmove(a + 1, a, 7 * sizeof a[0]);
        a[4] += t1;
        a[0] = t1 + t2;
    }
    for (int i = 0; i < 8; i++)
        s->h[i] += a[i];
}
static void sha_init(Sha *s)
{
    static const Uint32 h0[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                                 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    memcpy(s->h, h0, sizeof h0);
    s->len = 0;
    s->n = 0;
}
static void sha_add(Sha *s, const unsigned char *p, size_t n)
{
    s->len += n;
    while (n) {
        size_t k = 64 - s->n < n ? 64 - s->n : n;
        memcpy(s->buf + s->n, p, k);
        s->n += k, p += k, n -= k;
        if (s->n == 64) {
            sha_block(s, s->buf);
            s->n = 0;
        }
    }
}
static void sha_hex(Sha *s, char out[65])
{
    Uint64 bits = s->len * 8;
    unsigned char pad = 0x80, z = 0, l[8];
    sha_add(s, &pad, 1);
    while (s->n != 56)
        sha_add(s, &z, 1);
    for (int i = 0; i < 8; i++)
        l[i] = (unsigned char)(bits >> (56 - 8 * i));
    sha_add(s, l, 8);
    for (int i = 0; i < 8; i++)
        sprintf(out + 8 * i, "%08x", (unsigned)s->h[i]);
}

/* 1 = the right exe, 0 = some other file, -1 = cannot read */
static int check(const char *path)
{
    SDL_RWops *f = path && *path ? SDL_RWFromFile(path, "rb") : NULL;
    if (!f)
        return -1;
    Sha s;
    sha_init(&s);
    unsigned char buf[65536];
    size_t n;
    while ((n = SDL_RWread(f, buf, 1, sizeof buf)) > 0)
        sha_add(&s, buf, n);
    SDL_RWclose(f);
    char hex[65];
    sha_hex(&s, hex);
    return strcmp(hex, k_sha256) == 0;
}

static int copy_file(const char *from, const char *to)
{
    SDL_RWops *in = SDL_RWFromFile(from, "rb"), *out = in ? SDL_RWFromFile(to, "wb") : NULL;
    int ok = in && out;
    unsigned char buf[65536];
    size_t n;
    while (ok && (n = SDL_RWread(in, buf, 1, sizeof buf)) > 0)
        ok = SDL_RWwrite(out, buf, 1, n) == n;
    if (in)
        SDL_RWclose(in);
    if (out && SDL_RWclose(out) != 0)
        ok = 0;
    return ok ? 0 : -1;
}

static void error_box(const char *msg)
{
    fprintf(stderr, "elfbowl: %s\n", msg);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Elf Bowling", msg, NULL);
}

/* ---- pickers ------------------------------------------------------------------------------ */
#if defined(__linux__) && !defined(__ANDROID__)
int elfbowl_pick_file(char *out, size_t outsz)
{
    static const char *const cmds[] = {
        "zenity --file-selection --title='Choose the original Elf Bowling.exe' --file-filter='*.exe *.EXE' 2>/dev/null",
        "kdialog --getopenfilename . '*.exe *.EXE' --title 'Choose the original Elf Bowling.exe' 2>/dev/null",
        "yad --file --title='Choose the original Elf Bowling.exe' 2>/dev/null", NULL};
    for (int i = 0; cmds[i]; i++) {
        FILE *p = popen(cmds[i], "r");
        if (!p)
            continue;
        out[0] = 0;
        char *got = fgets(out, (int)outsz, p);
        int rc = pclose(p);
        if (rc == -1 || (rc != 0 && !got && WEXITSTATUS(rc) == 127))
            continue;                         /* tool not installed: try the next */
        if (!got)
            return -1;                        /* cancelled */
        out[strcspn(out, "\r\n")] = 0;
        return out[0] ? 0 : -1;
    }
    return -2;                                /* no picker available */
}
#elif defined(__ANDROID__)
/* ElfBowlActivity.pickExe(): SAF ACTION_OPEN_DOCUMENT, copies the document to the cache dir and
 * returns that path (null when cancelled). Blocks this (SDL main) thread, not the UI thread. */
int elfbowl_pick_file(char *out, size_t outsz)
{
    JNIEnv *env = (JNIEnv *)SDL_AndroidGetJNIEnv();
    jobject act = (jobject)SDL_AndroidGetActivity();
    if (!env || !act)
        return -2;
    jclass cls = (*env)->GetObjectClass(env, act);
    jmethodID m = (*env)->GetMethodID(env, cls, "pickExe", "()Ljava/lang/String;");
    int rc = -2;
    if (m) {
        jstring s = (jstring)(*env)->CallObjectMethod(env, act, m);
        rc = -1;
        if (s) {
            const char *c = (*env)->GetStringUTFChars(env, s, NULL);
            SDL_strlcpy(out, c, outsz);
            (*env)->ReleaseStringUTFChars(env, s, c);
            (*env)->DeleteLocalRef(env, s);
            rc = 0;
        }
    }
    if ((*env)->ExceptionCheck(env))
        (*env)->ExceptionClear(env);
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, act);
    return rc;
}
#endif

/* ---- the search ---------------------------------------------------------------------------- */
int elfbowl_first_run(const char *given, const char *user_dir, char *out, size_t outsz)
{
    char *pref = SDL_GetPrefPath("NStorm", "Elf Bowling");
    char *base = SDL_GetBasePath();
    char store[1024], cand[6][1024], msg[2048];
    int nc = 0;
    snprintf(store, sizeof store, "%s%s", pref ? pref : "", EXE_NAME);
    if (given && *given)
        SDL_strlcpy(cand[nc++], given, sizeof cand[0]);
    if (getenv("ELFBOWL_EXE") && *getenv("ELFBOWL_EXE"))
        SDL_strlcpy(cand[nc++], getenv("ELFBOWL_EXE"), sizeof cand[0]);
    SDL_strlcpy(cand[nc++], store, sizeof cand[0]);
    if (base)
        snprintf(cand[nc++], sizeof cand[0], "%s%s", base, EXE_NAME);
    SDL_strlcpy(cand[nc++], EXE_NAME, sizeof cand[0]);
    if (user_dir)
        snprintf(cand[nc++], sizeof cand[0], "%s/%s", user_dir, EXE_NAME);
    SDL_free(base);
    SDL_free(pref);

    const char *found = NULL;
    for (int i = 0; i < nc && !found; i++) {
        int r = check(cand[i]);
        if (r == 1)
            found = cand[i];
        else if (r == 0 && i < 2) {           /* a file the user named explicitly: say why it is ignored */
            snprintf(msg, sizeof msg, "%s is not the expected file.\n\nElf Bowling needs %s.", cand[i], WANT);
            error_box(msg);
        }
    }
    char picked[1024];
    while (!found) {
        int r = elfbowl_pick_file(picked, sizeof picked);
        if (r == -2) {
            snprintf(msg, sizeof msg, "Elf Bowling needs %s.\n\nPut it next to the game or in %s, or set "
                     "ELFBOWL_EXE to its path, then start the game again.", WANT, store);
            error_box(msg);
            return -1;
        }
        if (r != 0) {
            snprintf(msg, sizeof msg, "Elf Bowling needs %s to run. Start the game again to choose it.", WANT);
            error_box(msg);
            return -1;
        }
        if (check(picked) == 1)
            found = picked;
        else {
            snprintf(msg, sizeof msg, "That file is not the expected one.\n\nChoose %s.", WANT);
            error_box(msg);
        }
    }
    if (strcmp(found, store) != 0 && copy_file(found, store) == 0 && check(store) == 1)
        found = store;
    SDL_strlcpy(out, found, outsz);
    return 0;
}
