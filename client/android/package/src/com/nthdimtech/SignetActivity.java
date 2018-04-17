package com.nthdimtech;

import android.os.Bundle;
import android.os.Handler;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.Context;
import android.app.PendingIntent;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import android.hardware.usb.UsbEndpoint;
import android.hardware.usb.UsbInterface;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbConstants;
import android.content.BroadcastReceiver;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;
import android.app.Application;
import android.app.Service;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.Binder;
import java.util.HashMap;
import java.util.Iterator;
import android.content.ComponentName;

import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.content.pm.PackageManager;
import android.Manifest;

import org.qtproject.qt5.android.bindings.QtActivity;
import com.nthdimtech.SignetService;

public class SignetActivity extends QtActivity {
	private static SignetActivity mInstance;
	private String TAG = "SIGNET_ACTIVITY";
	private static final String ACTION_USB_PERMISSION = "com.nthdimtech.SignetClient.USB_PERMISSION";
	private PendingIntent mPermissionIntent;
	private UsbManager mManager;
	private UsbDeviceConnection mConnection;
	private UsbInterface mSignetHidInterface;
	private UsbEndpoint mSignetHidOutEp;
	private UsbEndpoint mSignetHidInEp;
	private HashMap<Integer, Integer> mConnectedDevices;
	private static final int VID=0x5E2A;
	private static final int PID=1;
	private boolean mSignetHasKeyboard;

	private static Context mContext;

	public SignetActivity () {
		mInstance = this;
		mConnectedDevices = new HashMap<Integer, Integer>();
	}

        private void deviceReady(UsbDevice device) {
		mConnection = mManager.openDevice(device);
		if (mConnection != null) {
			mConnection.claimInterface(mSignetHidInterface, true);
			mConnectedDevices.put(device.getDeviceId(), mConnection.getFileDescriptor());
			int fd = mConnection.getFileDescriptor();
			Log.d(TAG,"deviceReady(): FD " + fd + " Out endpoint " + mSignetHidInEp.getEndpointNumber() + ", IN Endpoint " + mSignetHidOutEp.getEndpointNumber()
			        + ", " + (mSignetHasKeyboard ? "Has keyboard" : "No keyboard"));
			notifyDeviceAttached(fd, mSignetHidInEp.getEndpointNumber(), mSignetHidOutEp.getEndpointNumber(), mSignetHasKeyboard);
		} else {
		        Log.d(TAG, "deviceReady(): Couldn't open connection: requesting permission");
			mManager.requestPermission(device, mPermissionIntent);
		}
	}

        private final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(final Context context, final Intent intent) {
			final String action = intent.getAction();
			if (ACTION_USB_PERMISSION.equals(action)) {
				Log.d(TAG, "onUsbPermission");
				synchronized (this) {
					final UsbDevice device = (UsbDevice) intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
					if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
						if (device != null) {
							Log.d(TAG, "onUsbPermission: Permission granted");
							deviceReady(device);
						}
					} else {
					        Log.d(TAG, "onUsbPermission: Permission denied");
					}
				}
			}
		        if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action)) {
				Log.d(TAG, "onDeviceConnected");
				synchronized(this)
				{
					UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
					if (device != null) {
						findHIDInterfaces(device);
						deviceReady(device);
					}
				}
			}
		        if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action)) {
				Log.d(TAG, "onDeviceDisconnected");
				synchronized(this)
				{
					UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
					Object val = mConnectedDevices.get(device.getDeviceId());
					if (val != null) {
						int fd = mConnectedDevices.get(device.getDeviceId());
						Log.d(TAG, "onDeviceDisconnected: Disconnecting device: ID " + device.getDeviceId() + ", FD " + fd);
						notifyDeviceDetached(fd);
						mConnectedDevices.remove(device.getDeviceId());
					}
				}
			}

		}
	};

        @Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		Intent serviceIntent = new Intent(this, SignetService.class);
		startService(serviceIntent);
		mManager = (UsbManager) getSystemService(Context.USB_SERVICE);
		registerReceiver(mUsbReceiver, new IntentFilter(UsbManager.ACTION_USB_DEVICE_ATTACHED));
		registerReceiver(mUsbReceiver, new IntentFilter(UsbManager.ACTION_USB_DEVICE_DETACHED));
		registerReceiver(mUsbReceiver, new IntentFilter(ACTION_USB_PERMISSION));
		mPermissionIntent = PendingIntent.getBroadcast(this, 0, new Intent(ACTION_USB_PERMISSION), 0);
		final Handler handler = new Handler();
		handler.postDelayed(new Runnable() {
			        @Override
				public void run()
				{
					checkForDevices();
				}
			}, 1000);
	}

        private static native void notifyDeviceAttached(int fd, int inEp, int outEp, boolean hasKeyboard);
	private static native void notifyDeviceDetached(int fd);

	@Override
	public void onDestroy() {
		super.onDestroy();
	}

        @Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);
	}

        private void checkForDevices() {
		HashMap<String, UsbDevice> deviceList = mManager.getDeviceList();
		Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();

		while(deviceIterator.hasNext()) {
			UsbDevice device = deviceIterator.next();
			if (device.getVendorId() == VID && device.getProductId() == PID) {
				Log.d(TAG, "checkForDevices(): Found " + device);
				if (findHIDInterfaces(device)) {
					Log.d(TAG, "Found the HID interface: " + mSignetHidInterface);
					if (mManager.hasPermission(device)) {
						Log.d(TAG, "checkForDevices() hasPermission");
						deviceReady(device);
					} else {
					        Log.d(TAG, "checkForDevices() requestPermission");
						mManager.requestPermission(device, mPermissionIntent);
					}
				}
			}
		}
	}

        private boolean findHIDInterfaces(UsbDevice device) {
		mSignetHasKeyboard = false;
		mSignetHidInterface = null;
		for (int i = 0; i < device.getInterfaceCount(); i++) {
			UsbInterface tempInterface = device.getInterface(i);
			if (mSignetHidInterface == null && tempInterface.getInterfaceClass() == UsbConstants.USB_CLASS_HID &&
			    tempInterface.getInterfaceProtocol() == 0 &&
			    tempInterface.getInterfaceSubclass() == 0) {
				mSignetHidOutEp = null;
				mSignetHidInEp = null;
				for (int j = 0; j < tempInterface.getEndpointCount(); j++) {
					if (tempInterface.getEndpoint(j).getDirection() == UsbConstants.USB_DIR_OUT) {
						mSignetHidOutEp = tempInterface.getEndpoint(j);
					} else {
					        mSignetHidInEp = tempInterface.getEndpoint(j);
					}
				}
			        if (mSignetHidOutEp != null && mSignetHidInEp != null) {
					mSignetHidInterface = tempInterface;
					Log.d(TAG, "findHIDInterfaces(): found HID communication interface");
				} else {
				        mSignetHidInEp = null;
					mSignetHidOutEp = null;
				}
			}
		        if (tempInterface.getInterfaceClass() == UsbConstants.USB_CLASS_HID &&
			    tempInterface.getInterfaceProtocol() == 1 &&
			    tempInterface.getInterfaceSubclass() == 1) {
				mSignetHasKeyboard = true;
				Log.d(TAG, "findHIDInterfaces(): found HID keyboard");
			}
		}
	        return mSignetHidInterface != null;
	}
}
