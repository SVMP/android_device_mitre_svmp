package org.mitre.svmp.locationhelper;

import android.util.Log;
import org.mitre.svmp.protocol.SVMPProtocol.Response;
import java.util.concurrent.BlockingQueue;

/**
 * @author Joe Portner
 */
public class NettyClientSender extends Thread {
    private static final String TAG = NettyClientSender.class.getName();

    private BlockingQueue<Response> sendQueue;
    private NettyClientHandler handler;
    private boolean interrupt = false;

    public NettyClientSender(BlockingQueue<Response> sendQueue, NettyClientHandler handler) {
        this.sendQueue = sendQueue;
        this.handler = handler;
    }

    public void run() {
        while( !interrupt) {
            try {
                // wait and take the next available response from the send queue
                Response response = sendQueue.take();
                if( response != null )
                    handler.sendResponse(response);
            } catch( InterruptedException e ) {
                Log.d(TAG, "NettyClientSender.sendQueue.take() interrupted exception: " + e.getMessage());
            }
        }
    }

    public void setInterrupt(boolean interrupt) {
        this.interrupt = interrupt;
    }
}
