// iOS UI for the package (pkg/README.md): the on-screen BOWL button, a UIKit button over SDL's
// view that sends Space (the game's throw key), and the first-run document picker
// (elfbowl_pick_file, pkg/firstrun.c). Touches on the game arrive as mouse clicks (pkg/pkg_main.cpp).
#import <UIKit/UIKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#include "SDL.h"
#include "SDL_syswm.h"
#include "../firstrun.h"

static UIButton *s_bowl;

static void send_space(Uint32 type)
{
    SDL_Event e;
    SDL_zero(e);
    e.type = type;
    e.key.state = type == SDL_KEYDOWN ? SDL_PRESSED : SDL_RELEASED;
    e.key.keysym.sym = SDLK_SPACE;
    e.key.keysym.scancode = SDL_SCANCODE_SPACE;
    SDL_PushEvent(&e);
}

@interface EBBowlTarget : NSObject
@end
@implementation EBBowlTarget
- (void)down:(UIButton *)b { b.alpha = 0.6; send_space(SDL_KEYDOWN); }
- (void)up:(UIButton *)b { b.alpha = 1.0; send_space(SDL_KEYUP); }
- (void)tick:(NSTimer *)t
{
    SDL_Window *sw = SDL_GetKeyboardFocus();     // the game's window, once SDL has made it
    SDL_SysWMinfo wm;
    SDL_VERSION(&wm.version);
    if (!sw || !SDL_GetWindowWMInfo(sw, &wm) || wm.subsystem != SDL_SYSWM_UIKIT)
        return;
    UIView *root = wm.info.uikit.window.rootViewController.view;
    if (!root)
        return;
    if (!s_bowl) {
        CGFloat sz = 88;
        s_bowl = [UIButton buttonWithType:UIButtonTypeCustom];
        [s_bowl setTitle:@"BOWL" forState:UIControlStateNormal];
        s_bowl.titleLabel.font = [UIFont boldSystemFontOfSize:20];
        s_bowl.backgroundColor = [UIColor colorWithRed:0.75 green:0.1 blue:0.1 alpha:0.7];
        s_bowl.layer.cornerRadius = sz / 2;
        s_bowl.layer.borderWidth = 3;
        s_bowl.layer.borderColor = UIColor.whiteColor.CGColor;
        [s_bowl addTarget:self action:@selector(down:) forControlEvents:UIControlEventTouchDown];
        [s_bowl addTarget:self action:@selector(up:)
            forControlEvents:UIControlEventTouchUpInside | UIControlEventTouchUpOutside | UIControlEventTouchCancel];
        s_bowl.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleTopMargin;
        s_bowl.frame = CGRectMake(0, 0, sz, sz);
    }
    if (s_bowl.superview != root)
        [root addSubview:s_bowl];
    CGRect b = root.bounds;
    UIEdgeInsets in = root.safeAreaInsets;
    s_bowl.center = CGPointMake(b.size.width - in.right - 24 - 44, b.size.height - in.bottom - 24 - 44);
    [root bringSubviewToFront:s_bowl]; // SDL's Metal/GL view may be added after us
}
@end

void elfbowl_ios_setup(void)
{
    static EBBowlTarget *target;
    target = [EBBowlTarget new];
    [NSTimer scheduledTimerWithTimeInterval:0.5 target:target selector:@selector(tick:) userInfo:nil repeats:YES];
}

// ---- first-run picker: a document picker in a window of its own (SDL's comes later), run
// modally by spinning the run loop until the user picks or cancels. asCopy: the copy lands in
// the app's tmp; firstrun.c verifies it and copies it into app storage.
@interface EBPicker : NSObject <UIDocumentPickerDelegate>
@property(nonatomic) BOOL done;
@property(nonatomic, copy) NSString *path;
@end
@implementation EBPicker
- (void)documentPicker:(UIDocumentPickerViewController *)c didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls
{
    self.path = urls.firstObject.path;
    self.done = YES;
}
- (void)documentPickerWasCancelled:(UIDocumentPickerViewController *)c { self.done = YES; }
@end

int elfbowl_pick_file(char *out, size_t outsz)
{
    @autoreleasepool {
        UIWindow *win = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
        for (UIScene *sc in UIApplication.sharedApplication.connectedScenes)
            if ([sc isKindOfClass:UIWindowScene.class]) { win.windowScene = (UIWindowScene *)sc; break; }
        win.rootViewController = [UIViewController new];
        win.rootViewController.view.backgroundColor = UIColor.blackColor;
        [win makeKeyAndVisible];
        EBPicker *d = [EBPicker new];
        UIDocumentPickerViewController *pick =
            [[UIDocumentPickerViewController alloc] initForOpeningContentTypes:@[UTTypeItem] asCopy:YES];
        pick.delegate = d;
        pick.allowsMultipleSelection = NO;
        [win.rootViewController presentViewController:pick animated:YES completion:nil];
        while (!d.done)
            [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.05]];
        win.hidden = YES;
        if (!d.path)
            return -1;
        SDL_strlcpy(out, d.path.fileSystemRepresentation, outsz);
        return 0;
    }
}
