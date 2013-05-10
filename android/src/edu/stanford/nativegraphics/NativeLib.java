
package edu.stanford.nativegraphics;

import android.util.Log;

public class NativeLib {

    static {
        System.loadLibrary("nativegraphics");
    }

    // Called from native
    public String stringCallback(String filename) {
        Log.v("NativeLib", "Java" + filename);
        return "Success!";
    }
    
    public void nativeInit(int width, int height) {
        init(width, height);
    }   

    // Native Functions
    private native void init(int width, int height);
    public native void step();
}
