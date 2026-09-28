// Native entry point: main() -> WinMain, as c0w32.obj does on Windows.
//
//   elfbowl [path/to/Elf Bowling.exe]
// The exe path defaults to $ELFBOWL_EXE, then "Elf Bowling.exe" (port_res_open).
#include <vcl/vcl.h>

#include <cstring>
#include <string>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
// native/game_data.cpp: the g_ globals' initial values, from the exe image (no exe bytes in the build)
extern "C" int elf_game_data_load(const unsigned char *img, size_t size);

int main(int argc, char **argv)
{
    // On Android static initialisation runs on the Java thread, not the one that runs main()
    Vcl::SetMainThread();
    const char *exe = argc > 1 ? argv[1] : nullptr;
    if (port_res_open(exe) != 0)
        port_log("could not load the original Elf Bowling.exe (%s); resources will be missing",
                 exe ? exe : "$ELFBOWL_EXE or ./Elf Bowling.exe");
    size_t imgSize = 0;
    const unsigned char *img = port_res_image(&imgSize);
    if (elf_game_data_load(img, imgSize) != 0)
        port_log("game data: this is not the Elf Bowling.exe (NStorm, 1999) the port was built for; "
                 "the game's initial values are missing");
    std::string cmd;
    for (int i = 2; i < argc; i++) {
        if (!cmd.empty())
            cmd += ' ';
        cmd += argv[i];
    }
    int rc = WinMain(System::HInstance, nullptr, &cmd[0], SW_SHOWNORMAL);
#ifdef __EMSCRIPTEN__
    // Application->Run() only registered the browser main loop (forms.cpp, WebRun), so the game is
    // still running: keep the runtime and every object alive (no EXIT_RUNTIME).
    return rc;
#endif
    Vcl::Shutdown();
    port_shutdown();
    return rc;
}
