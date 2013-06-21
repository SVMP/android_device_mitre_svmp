package org.mitre.svmp.locationhelper;

import android.util.Log;
import io.netty.channel.Channel;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;
import io.netty.channel.MessageList;
import org.mitre.svmp.protocol.SVMPProtocol.Request;
import org.mitre.svmp.protocol.SVMPProtocol.Response;
import java.util.concurrent.BlockingQueue;

/**
 * @author Joe Portner
 */
public class NettyClientHandler extends ChannelInboundHandlerAdapter {
    private static final String TAG = NettyClientHandler.class.getName();

    // Stateful properties
    private volatile Channel channel;
    protected BlockingQueue<Request> receiveQueue;

    public NettyClientHandler(BlockingQueue<Request> receiveQueue) {
        this.receiveQueue = receiveQueue;
    }

    public boolean sendResponse(Response response) {
        boolean success = true;
        try {
            // write the Response to the channel
            channel.write(response);
        } catch( Exception e ) {
            Log.d(TAG, "sendResponse failed: " + e.getMessage());
            success = false;
        }

        return success;
    }

    @Override
    public void channelRegistered(ChannelHandlerContext ctx) throws Exception {
        channel = ctx.channel();
    }

    @Override
    public void messageReceived(ChannelHandlerContext ctx, MessageList<Object> msgs) throws Exception {
        for (int i = 0; i < msgs.size(); i++) {
            receiveQueue.add((Request) msgs.get(i));
        }
        msgs.recycle();
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) throws Exception {
        Log.d(TAG, "Unexpected exception from downstream.", cause);
        ctx.close();
    }
}
