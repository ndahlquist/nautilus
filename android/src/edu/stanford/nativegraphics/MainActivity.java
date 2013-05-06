/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package edu.stanford.nativegraphics;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

public class MainActivity extends Activity {

	private static String TAG = "GL2JNIActivity";
	
    public static GL2JNIView mView;
	public static OverlayUpdater overlayUpdater;
	public static TextView FPSmeter;

    @Override protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mView = new GL2JNIView(getApplication());
        setContentView(mView);
        
        View overlay = getLayoutInflater().inflate(R.layout.overlay, null);
		addContentView(overlay, new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
		FPSmeter = (TextView)findViewById(R.id.FPS_meter);
		overlayUpdater = new OverlayUpdater();
    }

    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
    }

    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
    }
    
	static class OverlayUpdater extends Handler {
		public final int FPS_UPDATE = 0;

		@Override
		public void handleMessage(Message msg) {
			switch(msg.arg1) {
			case FPS_UPDATE:
				FPSmeter.setText(msg.arg2 + " FPS");
				return;
			default:
				Log.e(TAG, "Unrecognized message " + msg.arg1);
				return;
			}
		}
	}
}
