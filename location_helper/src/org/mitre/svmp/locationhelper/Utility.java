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

import android.location.Location;
import android.os.Bundle;
import android.util.Log;
import org.mitre.svmp.protocol.SVMPProtocol.LocationProviderStatus;
import org.mitre.svmp.protocol.SVMPProtocol.LocationUpdate;
import org.mitre.svmp.protocol.SVMPProtocol.Response;
import org.mitre.svmp.protocol.SVMPProtocol.LocationResponse;
import org.mitre.svmp.protocol.SVMPProtocol.LocationSubscribe;
import org.mitre.svmp.protocol.SVMPProtocol.LocationUnsubscribe;

/**
 * @author Joe Portner
 */
public class Utility {
    private static final String TAG = BackgroundService.class.getName();

    public static Location getLocation(LocationUpdate locationUpdate) {
        Location location = null;

        try {
            location = new Location(locationUpdate.getProvider());

            // get required fields
            location.setLatitude(locationUpdate.getLatitude());
            location.setLongitude(locationUpdate.getLongitude());
            location.setTime(locationUpdate.getTime());

            // get optional fields
            if( locationUpdate.hasAccuracy() )
                location.setAccuracy(locationUpdate.getAccuracy());
            if( locationUpdate.hasAltitude() )
                location.setAltitude(locationUpdate.getAltitude());
            if( locationUpdate.hasBearing() )
                location.setBearing(locationUpdate.getBearing());
            if( locationUpdate.hasSpeed() )
                location.setSpeed(locationUpdate.getSpeed());
        } catch( Exception e ) {
            Log.e(TAG, "Error parsing LocationUpdate: " + e.getMessage());
        }

        return location;
    }

    public static Bundle getBundle(LocationProviderStatus locationProviderStatus) {
        Bundle extras = new Bundle();

        // TODO: populate Bundle with extras from the Tuple list
        /*
        List<LocationProviderStatus.Tuple> tuples = locationProviderStatus.getExtrasList();
        for( LocationProviderStatus.Tuple tuple : tuples ) {
            extras.putString(tuple.getKey(), tuple.getValue());
        }
        */

        return extras;
    }

    public static Response buildSubscribeResponse(Subscription subscription, boolean singleShot) {
        LocationSubscribe.Builder lsBuilder = LocationSubscribe.newBuilder();
        lsBuilder.setType( singleShot
                ? LocationSubscribe.LocationSubscribeType.SINGLE_UPDATE
                : LocationSubscribe.LocationSubscribeType.MULTIPLE_UPDATES );
        lsBuilder.setProvider(subscription.getProvider());
        lsBuilder.setMinTime(subscription.getMinTime());
        lsBuilder.setMinDistance(subscription.getMinDistance());
        LocationSubscribe locationSubscribe = lsBuilder.build();
        return buildResponse( buildLocationResponse(locationSubscribe) );
    }

    public static Response buildUnsubscribeResponse(String provider) {
        LocationUnsubscribe.Builder lsBuilder = LocationUnsubscribe.newBuilder();
        lsBuilder.setProvider(provider);
        LocationUnsubscribe locationUnsubscribe = lsBuilder.build();
        return buildResponse( buildLocationResponse(locationUnsubscribe) );
    }

    private static LocationResponse buildLocationResponse(LocationSubscribe locationSubscribe) {
        LocationResponse.Builder lrBuilder = LocationResponse.newBuilder();
        lrBuilder.setType(LocationResponse.LocationResponseType.SUBSCRIBE);
        lrBuilder.setSubscribe(locationSubscribe);
        return lrBuilder.build();
    }

    private static LocationResponse buildLocationResponse(LocationUnsubscribe locationUnsubscribe) {
        LocationResponse.Builder lrBuilder = LocationResponse.newBuilder();
        lrBuilder.setType(LocationResponse.LocationResponseType.UNSUBSCRIBE);
        lrBuilder.setUnsubscribe(locationUnsubscribe);
        return lrBuilder.build();
    }

    private static Response buildResponse(LocationResponse locationResponse) {
        Response.Builder rBuilder = Response.newBuilder();
        rBuilder.setType(Response.ResponseType.LOCATION);
        rBuilder.setLocationResponse(locationResponse);
        return rBuilder.build();
    }

    public static Response buildHandshakeResponse() {
        Response.Builder rBuilder = Response.newBuilder();
        rBuilder.setType(Response.ResponseType.VMREADY);
        rBuilder.setMessage("LocationHelper-to-EventServer handshake message");
        return rBuilder.build();
    }
}
