package edu.stanford.nativegraphics;

import android.util.Log;
import android.content.Context;

public class NativeLib {

    static {
        System.loadLibrary("nativegraphics");
    }

    // Called from native
    public void stringCallback() { // TODO
        Log.v("NativeLib", "Java");
        //return "Test successful";
    }
    
    public static void nativeInit(Context context, int width, int height) {
        // TODO: Unholy resource hack
        passResource("raptor.obj", RawResourceReader.readTextFileFromRawResource(context, R.raw.raptor));
        passResource("hex.obj", RawResourceReader.readTextFileFromRawResource(context, R.raw.hex));
        passResource("depth_f.glsl", RawResourceReader.readTextFileFromRawResource(context, R.raw.depth_f));
        passResource("standard_v.glsl", RawResourceReader.readTextFileFromRawResource(context, R.raw.standard_v));
        init(width, height);
    }   

    // Native Functions
    private static native void passResource(String name, String contents); // TODO: Unholy resource hack
    private static native void init(int width, int height);
    public static native void renderFrame();
}
