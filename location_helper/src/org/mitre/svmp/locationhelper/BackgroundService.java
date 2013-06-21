/*
 Copyright 2013 The MITRE Corporation, All Rights Reserved.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this work except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */
package org.mitre.svmp.locationhelper;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;
import com.google.protobuf.InvalidProtocolBufferException;
import org.mitre.svmp.protocol.SVMPProtocol.Response;

/**
 * @author Joe Portner
 */
public class BackgroundService extends Service {
    private static final String TAG = BackgroundService.class.getName();

    private NettyClient nettyClient;

    @Override
    public void onCreate() {
        // start the socket connection
        int eventServerPort = getResources().getInteger(R.integer.event_server_port);
        nettyClient = new NettyClient(getApplicationContext(), eventServerPort);
        nettyClient.setDaemon(true);
        nettyClient.start();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        try {
            Log.d(TAG, "Received start command");

            // get intent extra
            byte[] responseData = intent.getByteArrayExtra("responseData");

            // deserialize the Response from the byte array
            Response response = Response.parseFrom(responseData);

            // send the Response to the Event Server
            nettyClient.sendMessage(response);
        } catch( InvalidProtocolBufferException e ) {
            Log.e(TAG, e.getMessage());
        }

        // We want this service to continue running until it is explicitly
        // stopped, so return sticky.
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        // interrupt the socket loop so it shuts down gracefully, then the thread can end
        if( nettyClient != null )
            nettyClient.setInterrupt(true);
    }

    public IBinder onBind(Intent intent) {
        return null;
    }
}
