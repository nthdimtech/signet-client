package com.nthdimtech;

import android.app.Service;
import android.content.Intent;
import android.app.PendingIntent;
import android.content.IntentFilter;
import android.os.IBinder;
import android.util.Log;
import android.content.Context;
import android.content.BroadcastReceiver;

public class SignetService extends Service {

	private String TAG = "SIGNET_SERVICE";
	private static String FOREGROUND_ACTION = "com.nthdimtech.fg";

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		return Service.START_NOT_STICKY;
	}

        private static Context mContext;
	private static SignetService mService;

	private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(final Context context, final Intent intent) {
			final String action = intent.getAction();
			if (action.equals(new String(SignetService.FOREGROUND_ACTION))) {
				Intent startIntent = new Intent(mContext, SignetActivity.class);
				startIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
				startIntent.addFlags(Intent.FLAG_FROM_BACKGROUND);
				mContext.startActivity(startIntent);
			}
		}
	};

        public static void foregroundActivity() {
		Intent fgIntent =new Intent(SignetService.FOREGROUND_ACTION);
		mService.getApplicationContext().sendBroadcast(fgIntent);
	}

        @Override
	public void onCreate() {
		mContext = this;
		mService = this;
		registerReceiver(mReceiver, new IntentFilter("com.nthdimtech.fg"));
	}

        @Override
	public IBinder onBind(Intent intent) {
		return null;
	}
}
