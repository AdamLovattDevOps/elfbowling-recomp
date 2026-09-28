// Elf Bowling on SDL2 for Android (port/pkg/README.md). SDLActivity plus an on-screen BOWL button
// that sends the Space key (the game's "press the space bar" throw), and pickExe(), the first-run
// document picker (pkg/firstrun.c). Touches on the game arrive as mouse clicks (pkg/pkg_main.cpp).
package com.mussyg.elfbowling;

import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.GradientDrawable;
import android.os.Bundle;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.RelativeLayout;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import org.libsdl.app.SDLActivity;

public class ElfBowlActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] {"SDL2", "SDL2_ttf", "main"};
    }

    private static final int PICK_EXE = 0x4eb;
    private final Object mPickLock = new Object();
    private boolean mPickDone;
    private String mPicked;

    // Called from the SDL thread (JNI, pkg/firstrun.c): the Storage Access Framework picker, then a
    // copy of the chosen document in the cache dir, whose path is returned (null when cancelled).
    public String pickExe() {
        synchronized (mPickLock) {
            mPickDone = false;
            mPicked = null;
        }
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Intent i = new Intent(Intent.ACTION_OPEN_DOCUMENT);
                i.addCategory(Intent.CATEGORY_OPENABLE);
                i.setType("*/*");
                startActivityForResult(i, PICK_EXE);
            }
        });
        synchronized (mPickLock) {
            while (!mPickDone) {
                try {
                    mPickLock.wait();
                } catch (InterruptedException e) {
                    return null;
                }
            }
            return mPicked;
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode != PICK_EXE) {
            super.onActivityResult(requestCode, resultCode, data);
            return;
        }
        String path = null;
        if (resultCode == RESULT_OK && data != null && data.getData() != null) {
            File out = new File(getCacheDir(), "picked.exe");
            try (InputStream in = getContentResolver().openInputStream(data.getData());
                 OutputStream os = new FileOutputStream(out)) {
                byte[] buf = new byte[65536];
                int n;
                while ((n = in.read(buf)) > 0)
                    os.write(buf, 0, n);
                path = out.getAbsolutePath();
            } catch (Exception e) {
                path = null;
            }
        }
        synchronized (mPickLock) {
            mPicked = path;
            mPickDone = true;
            mPickLock.notifyAll();
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (mLayout == null)
            return;
        Button bowl = new Button(this);
        bowl.setText("BOWL");
        bowl.setTextColor(Color.WHITE);
        bowl.setTextSize(18);
        GradientDrawable bg = new GradientDrawable();
        bg.setShape(GradientDrawable.OVAL);
        bg.setColor(0xB0C01818);
        bg.setStroke(4, Color.WHITE);
        bowl.setBackground(bg);
        bowl.setGravity(Gravity.CENTER);
        bowl.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent e) {
                switch (e.getActionMasked()) {
                case MotionEvent.ACTION_DOWN:
                    SDLActivity.onNativeKeyDown(KeyEvent.KEYCODE_SPACE);
                    v.setAlpha(0.6f);
                    return true;
                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_CANCEL:
                    SDLActivity.onNativeKeyUp(KeyEvent.KEYCODE_SPACE);
                    v.setAlpha(1f);
                    return true;
                }
                return false;
            }
        });
        int size = (int) (88 * getResources().getDisplayMetrics().density);
        RelativeLayout.LayoutParams lp = new RelativeLayout.LayoutParams(size, size);
        lp.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
        lp.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
        lp.setMargins(0, 0, size / 4, size / 4);
        mLayout.addView(bowl, lp);
    }
}
