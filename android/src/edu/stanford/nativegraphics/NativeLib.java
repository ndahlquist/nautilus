
package edu.stanford.nativegraphics;

import android.util.Log;

public class NativeLib {

    static {
        System.loadLibrary("nativegraphics");
    }

    // Called from native
    public void stringCallback() {
        Log.v("NativeLib", "Java");
        //return "Test successful";
    }
    
    public void nativeInit(int width, int height) {
        init(width, height);
    }   

    // Native Functions
    private static native void init(int width, int height);
    public static native void step();
}
