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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.location.LocationManager;
import android.util.Log;
import org.mitre.svmp.protocol.SVMPProtocol.Response;

/**
 * @author Joe Portner
 */
public class LocationBroadcastReceiver extends BroadcastReceiver {
    private static final String TAG = LocationBroadcastReceiver.class.getName();

    public void onReceive(Context context, Intent intent) {
        // check the Intent Action to make sure it is valid (either "start" or "stop")
        boolean start;
        if( (start = intent.getAction().equals( context.getString(R.string.location_helper_start_action)) )
                || !(start = !intent.getAction().equals( context.getString(R.string.location_helper_stop_action) )) ) {
            Log.d(TAG, "Received location subscribe intent broadcast");

            // get extras from Intent
            String provider = intent.getStringExtra("provider");

            // if this broadcast is for a passive provider, return; the LocationManager shouldn't send us broadcasts
            // for passive providers, this is just a failsafe
            if( provider.equals(LocationManager.PASSIVE_PROVIDER) )
                return;

            long minTime = intent.getLongExtra("minTime", 0L);
            float minDistance = intent.getFloatExtra("minDistance", 0.0f);
            boolean singleShot = intent.getBooleanExtra("singleShot", false);

            // TODO: find out if it's better to store one DatabaseHandler for the entire object, or if this approach is fine
            // create a new DatabaseHandler
            DatabaseHandler handler = new DatabaseHandler(context, false);

            // construct Subscription for this request
            Subscription subscription = new Subscription(provider, minTime, minDistance);

            // get the foremost subscription (with the lowest combined minTime and minDistance)
            Subscription foremostSubscription = handler.getForemostSubscription(provider);

            // this Intent directs us to start a new subscription
            if( start ) {
                // if this is not a "SingleShot" subscription, add it to the sorted list of subscriptions
                if( !singleShot )
                    handler.insertSubscription(subscription);

                // if the foremost subscription doesn't satisfy this one, send a message to make a new subscription
                if( foremostSubscription == null || !foremostSubscription.satisfies(subscription) ) {
                    // this is a long-term subscription, get the lowest minTime and minDistance
                    if( !singleShot && foremostSubscription != null ) {
                        if( foremostSubscription.getMinTime() < minTime )
                            minTime = foremostSubscription.getMinTime();
                        if( foremostSubscription.getMinDistance() < minDistance )
                            minDistance = foremostSubscription.getMinDistance();

                        // to avoid making another database call, we'll construct a new subscription here
                        subscription = new Subscription(provider, minTime, minDistance);
                    }

                    // send the message
                    sendMessage( context, Utility.buildSubscribeResponse(subscription, singleShot) );
                }
            }
            // this Intent directs us to stop an existing subscription
            else {
                long result = handler.deleteSubscription(subscription);

                // if the deletion was successful, then we need to reload the foremost subscription to see if it has changed
                if( result > -1 ) {
                    // refresh the foremost subscription
                    foremostSubscription = handler.getForemostSubscription(provider);

                    // if there is no foremost subscription for this provider, send a message to unsubscribe
                    if( foremostSubscription == null )
                        sendMessage( context, Utility.buildUnsubscribeResponse(provider) );
                    // if we have detected a change in the foremost subscription, send a message to make a new subscription
                    // this saves battery life for the handset
                    else if( foremostSubscription.getMinTime() > subscription.getMinTime()
                            || foremostSubscription.getMinDistance() > subscription.getMinDistance() )
                        sendMessage( context, Utility.buildSubscribeResponse(foremostSubscription, false));
                }
            }

            // cleanup
            handler.close();
        }
        // we also receive the BOOT_COMPLETED broadcast intent to start this service as soon as the phone boots up
        else if( intent.getAction().equals("android.intent.action.BOOT_COMPLETED") ) {
            Log.d(TAG, "Received system boot intent broadcast");

            // create a new DatabaseHandler; "true" tells it to drop old subscription data
            // this only happens once, after the VM boots
            DatabaseHandler handler = new DatabaseHandler(context, true);

            // cleanup
            handler.close();

            // we have to send a handshake to the Netty server on the EventServer so it
            // is aware of our socket channel and can push messages to us
            sendMessage(context, Utility.buildHandshakeResponse());
        }
    }

    // send a Response to the EventServer
    private void sendMessage(Context context, Response response) {
        // create an intent to send to the BackgroundService
        Intent intent = new Intent(context, BackgroundService.class);

        // pack the Response in the intent as a byte array
        intent.putExtra("responseData", response.toByteArray());

        // start the BackgroundService if it hasn't been started, and send the intent to it
        context.startService(intent);
    }
}
