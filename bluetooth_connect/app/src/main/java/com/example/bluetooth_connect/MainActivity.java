package com.example.bluetooth_connect;

import androidx.appcompat.app.AppCompatActivity;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;

import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Set;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {

    // Logcat
    private static final String TAG = "DEBUG_MA" ;

    //Bluetooth connection constants.
    static public final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    static public final int BT_STATUS_NOT_CONNECTED     = 0;
    static public final int BT_STATUS_CONNECTING        = 1;
    static public final int BT_STATUS_CONNECTED         = 2;
    static public final int BT_STATUS_FAILED_CONNECTION = 3;
    static public final int BT_STATUS_LOST_CONNECTION   = 4;
    static final int BT_STATE_LISTENING            = 1;
    static final int BT_STATE_CONNECTING           = 2;
    static final int BT_STATE_CONNECTED            = 3;
    static final int BT_STATE_CONNECTION_FAILED    = 4;
    static final int BT_STATE_MESSAGE_RECEIVED     = 5;

    // UI declarations.
    Button btConnectButton;
    TextView repsDisplayTxtView;
    Spinner spinnerBTPairedDevices;
    TextView repsDisplayTxtView_V2;

    //Bluetooth connection declarations.
    static public int btConnectionStatus = BT_STATUS_NOT_CONNECTED;
    BluetoothSocket btSocket = null;
    BluetoothAdapter btAdaptor = null;
    Set<BluetoothDevice> btPairedDevices = null;
    boolean btConnected = false;
    BluetoothDevice btDevice = null;
    InitBTDataCommunication btInitDataComm =null;

    // For using only a boolean signal, and keeping track of the rep count in the app.
    int reps;

    // Handles incoming Messages [BT state (1 & 2) + buffer data (only 2)] sent by the (1)BTConnection
    // thread & (2)InitBTDataCommunication thread, to be handled respectively.
    // (1)BTConnection thread: Displays "Disconnect" + creates (2)InitBTDataCommunication thread.
    // (2)InitBTDataCommunication thread: Handles and displays repetition data in TextView.
    // This constructor is depreciated. Implicitly choosing a Looper leads to bugs.
    // If the implicit thread local behavior is required for compatibility, use: new Handler(Looper.myLooper(), callback).
    // https://developer.android.com/reference/android/os/Handler
    Handler handler = new Handler(new Handler.Callback() {
        @Override
        public boolean handleMessage(Message msg) {
            switch (msg.what) {
                case BT_STATE_LISTENING:
                    break;

                case BT_STATE_CONNECTING:
                    btConnectionStatus = BT_STATUS_CONNECTING;
                    btConnectButton.setText("Connecting..");
                    break;

                // Initiating data communication between btSocket and application.
                case BT_STATE_CONNECTED:
                    btConnectionStatus = BT_STATUS_CONNECTED;
                    btConnectButton.setText("Disconnect");

                    btInitDataComm = new InitBTDataCommunication(btSocket);
                    btInitDataComm.start();

                    btConnected = true;
                    break;

                case BT_STATE_CONNECTION_FAILED:
                    btConnectionStatus = BT_STATUS_FAILED_CONNECTION;
                    btConnected = false;
                    break;

                // Display processed data from buffer into application.
                case BT_STATE_MESSAGE_RECEIVED:
                    // Why transfer number of reps through buffer, when we can use only a boolean signal
                    //  and keep track of the rep count in the app?
                    byte[] readBuff = (byte[]) msg.obj;
                    String signal = new String(readBuff,0,msg.arg1);
//                    repsDisplayTxtView.append(signal);
                    if(signal.equals("T")) {
                        reps++;
                        repsDisplayTxtView_V2.setText(String.valueOf(reps));
                    }
                    break;
            }
            return true;
        }
    });

    @Override
    // TODO: Fix permission error; check https://developer.android.com/training/permissions/requesting.html
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        reps = 0;

        // Link UI components.
        repsDisplayTxtView = findViewById(R.id.repsDisplayTxtView);
        repsDisplayTxtView.setMovementMethod(new ScrollingMovementMethod());
        spinnerBTPairedDevices = findViewById(R.id.btPairedDevicesSpinner);
        btConnectButton = findViewById(R.id.btConnectButton);
        repsDisplayTxtView.setText("App Loaded");
        repsDisplayTxtView_V2 = findViewById(R.id.repsDisplayTxtView_V2);
//        repsDisplayTxtView_V2.setText(0);

        btConnectButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                // Check if bluetooth module is already connected.
                // If already connected, disconnects the socket.
                // If not already connected, creates a new BTConnection with the module selected
                //  from the spinner list. If no device is selected from the spinner list, notifies
                //  the user to select a device and does nothing.
                if(btConnected ==false) {
                    if (spinnerBTPairedDevices.getSelectedItemPosition() == 0) {
                        Toast.makeText(getApplicationContext(), "Select Bluetooth Device", Toast.LENGTH_SHORT).show();
                        return;
                    }

                    // Picks the bluetooth device from the list of "paired" bluetooth devices. So
                    //  the user must pair with all machines before using.
                    // TODO: Change name of bluetooth module to an "Exercise" name related to the
                    //  machine on which the module is installed on rather than "HC-05"
                    //  This way, the user selects his exercise rather than a random module name.
                    //  This can be integrated with the scroll down list of exercises already present
                    //  in the MainActivity of the main app.
                    String sSelectedDevice = spinnerBTPairedDevices.getSelectedItem().toString();
                    for (BluetoothDevice BTDev : btPairedDevices) {
                        if (sSelectedDevice.equals(BTDev.getName())) {
                            btDevice = BTDev;
                            Log.d(TAG, "Selected device UUID = " + btDevice.getAddress());

                            // Creating instance of bluetooth module, and initializing the coonection.
                            BTConnection btConnection = new BTConnection(btDevice);
                            btConnection.start();
                        }
                    }
                } else {
                    if(btSocket !=null && btSocket.isConnected()) {
                        try {
                            btSocket.close();
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                    btConnectButton.setText("Connect");
                    btConnected = false;
                }
            }
        });
    }

    // (Thread 1) New BTConnection thread for establishing the Handler to send and process Messages and
    // Runnable Objects (bluetooth connection socket).
    public class BTConnection extends Thread {
        private BluetoothDevice btDevice;

        public BTConnection(BluetoothDevice btDevice) {
            this.btDevice = btDevice;
            try {
                btSocket = this.btDevice.createRfcommSocketToServiceRecord(MY_UUID);
            } catch (Exception exp) {
                exp.printStackTrace();
            }
        }

        public void run() {
            try {
                btSocket.connect();
                // Message containing a description and arbitrary data object to be sent to a Handler.
                // Message.obtain() returns new Message instance from global pool, avoiding need for constructor.
                Message message = Message.obtain();
                message.what = BT_STATE_CONNECTED;
                // Pushes message onto end of message queue, where it'll be received in handleMessage(Message),
                // in the thread attached to this handler.
                handler.sendMessage(message);
            } catch (IOException e) {
                e.printStackTrace();
                Message message = Message.obtain();
                message.what = BT_STATE_CONNECTION_FAILED;
                handler.sendMessage(message);
            }
        }

    }

    // (Thread 2) Main repetitions reading thread using a buffer.
    public class InitBTDataCommunication extends Thread {
        private final BluetoothSocket bluetoothSocket;
        private  InputStream inputStream = null;

        // Setup the input stream associated with this socket.
        public InitBTDataCommunication(BluetoothSocket socket) {
            bluetoothSocket = socket;
            try {
                inputStream = bluetoothSocket.getInputStream();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        // Continuously read bluetooth buffer, and send buffer data as Message to Handler.
        public void run() {
            byte[] buffer = new byte[1024];
            int bytes;

            while (btSocket.isConnected()) {
                try {
                    bytes = inputStream.read(buffer);
                    // Sends this Message (buffer content) to the Handler.
                    handler.obtainMessage(BT_STATE_MESSAGE_RECEIVED,bytes,-1,buffer).sendToTarget();
                } catch (IOException e) {
                    e.printStackTrace();
                    Log.e(TAG, "BT disconnect from decide end, exp " + e.getMessage());
                    btConnectionStatus = BT_STATUS_LOST_CONNECTION;

                    try {
                        // Disconnect bluetooth
                        Log.d(TAG, "Disconnecting BTConnection");
                        if(btSocket !=null && btSocket.isConnected()) {
                            btSocket.close();
                        }
                        btConnectButton.setText("Connect");
                        btConnected = false;
                    } catch (IOException ex) {
                        ex.printStackTrace();
                    }

                }
            }
        }
    }

    // Instantiates the Set of bluetooth devices of the phone.
    // TODO: Refactor code logic of the if function, and put following function inside as one.
    protected void getBTPairedDevices() {
        btAdaptor = BluetoothAdapter.getDefaultAdapter();
        if(btAdaptor == null) {
            Log.e(TAG, "getBTPairedDevices , BTAdaptor null ");
            return;

        } else if(!btAdaptor.isEnabled()) {
            return;
        }

        btPairedDevices = btAdaptor.getBondedDevices();
    }

    protected void populateSpinnerWithBTPairedDevices() {
        ArrayList<String> alPairedDevices = new ArrayList<>();
        alPairedDevices.add("Select");
        for (BluetoothDevice BTDev : btPairedDevices) {
            alPairedDevices.add(BTDev.getName());
        }

        final ArrayAdapter<String> aaPairedDevices = new ArrayAdapter<String>(this,R.layout.support_simple_spinner_dropdown_item,alPairedDevices);
        aaPairedDevices.setDropDownViewResource(R.layout.support_simple_spinner_dropdown_item);
        spinnerBTPairedDevices.setAdapter(aaPairedDevices);
    }

    @Override
    protected void onResume() {
        super.onResume();

        getBTPairedDevices();
        populateSpinnerWithBTPairedDevices();
    }

    @Override
    protected void onPause() {super.onPause();}

    @Override
    protected void onDestroy() {super.onDestroy();}
}

