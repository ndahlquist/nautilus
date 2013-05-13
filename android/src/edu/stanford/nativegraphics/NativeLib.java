package edu.stanford.nativegraphics;

import android.content.Context;
import android.util.Log;

public class NativeLib {
	private static final String TAG = "NativeLib";
	private Context mContext;
	
	public NativeLib(Context context) {
		mContext = context;
	}
	
    static {
        System.loadLibrary("nativegraphics");
    }

    // Called from native
    public String stringCallback(String fileName) {
    	String splitName = fileName.split("\\.")[0];
    	Log.i(TAG, "Looking up resource " + splitName);
    	int resID = mContext.getResources().getIdentifier(splitName, "raw", "edu.stanford.nativegraphics");
    	if(resID == 0) {
    		Log.e(TAG, "Resource " + fileName + " not found.");
    		return null;
    	}
        return RawResourceReader.readTextFileFromRawResource(mContext, resID);
    }
    
    public void nativeInit(int width, int height) {
        // TODO: Unholy resource hack
        /*passResource("raptor.obj", RawResourceReader.readTextFileFromRawResource(context, R.raw.raptor));
        passResource("hex.obj", RawResourceReader.readTextFileFromRawResource(context, R.raw.hex));
        passResource("depth_f.glsl", RawResourceReader.readTextFileFromRawResource(context, R.raw.depth_f));
        passResource("standard_v.glsl", RawResourceReader.readTextFileFromRawResource(context, R.raw.standard_v));
        passResource("normals_f.glsl", RawResourceReader.readTextFileFromRawResource(context, R.raw.normals_f));
        passResource("diffuse_f.glsl", RawResourceReader.readTextFileFromRawResource(context, R.raw.diffuse_f));*/
        init(width, height);
    }   

    // Native Functions
    //private static native void passResource(String name, String contents); // TODO: Unholy resource hack
    private native void init(int width, int height);
    public native void renderFrame();
    public static native void pointerDown(float x, float y);
    public static native void pointerMove(float x, float y);
    public static native void pointerUp(float x, float y);
}
