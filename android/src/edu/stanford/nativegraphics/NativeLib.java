package edu.stanford.nativegraphics;

import android.util.Log;
import android.content.Context;

public class NativeLib {

    static {
        System.loadLibrary("nativegraphics");
    }

    // Called from native
    public void stringCallback() {
        Log.v("NativeLib", "Java");
        //return "Test successful";
    }
    
    public void nativeInit(Context context, int width, int height) {
        String raptor = RawResourceReader.readTextFileFromRawResource(context, R.raw.raptor);
        passResource("raptor", raptor);
        init(width, height);
    }   

    // Native Functions
    private static native void passResource(String name, String contents); // TODO: Unholy resource hack
    private static native void init(int width, int height);
    public static native void step();
}
