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

import android.content.Context;
import android.location.Location;
import android.location.LocationManager;
import android.os.Bundle;
import android.util.Log;
import io.netty.bootstrap.Bootstrap;
import io.netty.channel.Channel;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.nio.NioSocketChannel;
import org.mitre.svmp.protocol.SVMPProtocol.Request;
import org.mitre.svmp.protocol.SVMPProtocol.LocationRequest;
import org.mitre.svmp.protocol.SVMPProtocol.LocationProviderInfo;
import org.mitre.svmp.protocol.SVMPProtocol.LocationProviderStatus;
import org.mitre.svmp.protocol.SVMPProtocol.LocationProviderEnabled;
import org.mitre.svmp.protocol.SVMPProtocol.LocationUpdate;
import org.mitre.svmp.protocol.SVMPProtocol.Response;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * @author Joe Portner
 */
public class NettyClient extends Thread {
    private static final String TAG = NettyClient.class.getName();

    private Context context;
    private String host;
    private int port;
    protected BlockingQueue<Response> sendQueue = new LinkedBlockingQueue<Response>();
    protected BlockingQueue<Request> receiveQueue = new LinkedBlockingQueue<Request>();
    private NettyClientSender sender;

    LocationManager locationManager;
    private boolean interrupt;

    public NettyClient(Context context, int port) {
        this.context = context;
        this.host = context.getString(R.string.send_host);
        this.port = port;
        this.locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
    }

    protected void setInterrupt( boolean interrupt ) {
        this.interrupt = interrupt;
        sender.setInterrupt(interrupt);
    }

    public void run() {
        // Create one EventLoopGroup (resource intensive)
        EventLoopGroup group = new NioEventLoopGroup();

        // Try to send the LocationRequest
        try {
            // create a new Bootstrap
            Bootstrap b = new Bootstrap()
                    .group(group)
                    .channel(NioSocketChannel.class)
                    .handler(new NettyClientInitializer(receiveQueue));

            // Make a new Channel (connection)
            Channel ch = b.connect(host, port).sync().channel();

            // Get the handler instance to initiate the request
            NettyClientHandler handler = ch.pipeline().get(NettyClientHandler.class);

            // the sender will watch the Blocking Queue and send a Response when one is added
            sender = new NettyClientSender(sendQueue, handler);
            sender.start();

            while(!interrupt) {
                try {
                    Request request = receiveQueue.take();
                    if( request.getType() == Request.RequestType.LOCATION )
                        handleMessage(request);
                } catch( InterruptedException e ) {
                    Log.d(TAG, "NettyClient.run() interrupted exception: " + e.getMessage());
                }
            }

            // Close the connection.
            ch.close();
        } catch( Exception e ) {
            e.printStackTrace();
        } finally {
            // Free resources from EventLoopGroup
            group.shutdownGracefully();
        }
    }

    private void handleMessage(Request request) {
        if( request.hasLocationRequest() ) {
            Log.d(TAG, "Received LocationRequest from EventServer");
            LocationRequest locationRequest = request.getLocationRequest();

            switch( locationRequest.getType() ) {
                case PROVIDERINFO:
                    if( locationRequest.hasProviderInfo() )
                        handleProviderInfo(locationRequest.getProviderInfo());
                    else
                        Log.d(TAG, "LocationProviderInfo not found");
                    break;
                case PROVIDERSTATUS:
                    if( locationRequest.hasProviderStatus() )
                        handleProviderStatus(locationRequest.getProviderStatus());
                    else
                        Log.d(TAG, "LocationProviderStatus not found");
                    break;
                case PROVIDERENABLED:
                    if( locationRequest.hasProviderEnabled() )
                        handleProviderEnabled(locationRequest.getProviderEnabled());
                    else
                        Log.d(TAG, "LocationProviderEnabled not found");
                    break;
                case LOCATIONUPDATE:
                    if( locationRequest.hasUpdate() )
                        handleUpdate(locationRequest.getUpdate());
                    else
                        Log.d(TAG, "LocationUpdate not found");
                    break;
            }
        }
    }

    private void handleProviderInfo(LocationProviderInfo locationProviderInfo) {
        // get provider name
        String provider = locationProviderInfo.getProvider();

        if( validProvider(provider) ) {
            // try to add a new test provider (there's no way to check and see if one already exists)
            try {
                // spoof this test provider to "overwrite" the legitimate one with the same name
                locationManager.addTestProvider(
                        provider,
                        locationProviderInfo.getRequiresNetwork(),
                        locationProviderInfo.getRequiresSatellite(),
                        locationProviderInfo.getRequiresCell(),
                        locationProviderInfo.getHasMonetaryCost(),
                        locationProviderInfo.getSupportsAltitude(),
                        locationProviderInfo.getSupportsSpeed(),
                        locationProviderInfo.getSupportsBearing(),
                        locationProviderInfo.getPowerRequirement(),
                        locationProviderInfo.getAccuracy());
            } catch( IllegalArgumentException e ) {
                // there was an existing test provider
            }

            // create a new DatabaseHandler
            DatabaseHandler handler = new DatabaseHandler(context, false);

            // get the foremost subscription (with the lowest combined minTime and minDistance)
            Subscription foremostSubscription = handler.getForemostSubscription(provider);

            // send the foremost subscription for this provider back to the client
            if( foremostSubscription != null )
                sendMessage( Utility.buildSubscribeResponse(foremostSubscription, false) );
        }
    }

    private void handleProviderStatus(LocationProviderStatus locationProviderStatus) {
        // get provider name
        String provider = locationProviderStatus.getProvider();

        if( validProvider(provider) ) {
            // construct a Bundle from the Protobuf message
            Bundle extras = Utility.getBundle(locationProviderStatus);

            try {
                // spoof the status to the test provider
                locationManager.setTestProviderStatus(
                        provider,
                        locationProviderStatus.getStatus(),
                        extras,
                        System.currentTimeMillis());
            } catch( IllegalArgumentException e ) {
                Log.e(TAG, "Error setting test provider status: " + e.getMessage());
            }
        }
    }

    private void handleProviderEnabled(LocationProviderEnabled locationProviderEnabled) {
        // get provider name
        String provider = locationProviderEnabled.getProvider();

        if( validProvider(provider) ) {
            try {
                // spoof the enabled/disabled to the test provider
                locationManager.setTestProviderEnabled(
                        provider,
                        locationProviderEnabled.getEnabled());
            } catch( IllegalArgumentException e ) {
                Log.e(TAG, "Error setting test provider enabled: " + e.getMessage());
            }
        }
    }

    private void handleUpdate(LocationUpdate locationUpdate) {
        // get provider name
        String provider = locationUpdate.getProvider();

        if( validProvider(provider) ) {
            // construct a Location from the Protobuf message
            Location location = Utility.getLocation(locationUpdate);

            try {
            // spoof the update to the test provider
                locationManager.setTestProviderLocation(provider, location);
            } catch( IllegalArgumentException e ) {
                Log.e(TAG, "Error setting test provider location: " + e.getMessage());
            }
        }
    }

    private boolean validProvider(String provider) {
        return provider != null
                && provider.length() > 0
                && !provider.equals(LocationManager.PASSIVE_PROVIDER);
    }

    protected void sendMessage(Response response) {
        Log.d(TAG, "Sending Response to Event Server");
        try {
            sendQueue.put(response);
        } catch( InterruptedException e ) {
            Log.d(TAG, "sendQueue.put() interrupted: " + e.getMessage());
        }
    }
}
