package org.mitre.svmp.locationhelper;

import io.netty.channel.ChannelInitializer;
import io.netty.channel.ChannelPipeline;
import io.netty.channel.socket.SocketChannel;
import io.netty.handler.codec.protobuf.ProtobufDecoder;
import io.netty.handler.codec.protobuf.ProtobufEncoder;
import io.netty.handler.codec.protobuf.ProtobufVarint32FrameDecoder;
import io.netty.handler.codec.protobuf.ProtobufVarint32LengthFieldPrepender;
import org.mitre.svmp.protocol.SVMPProtocol.Request;
import java.util.concurrent.BlockingQueue;

/**
 * @author Joe Portner
 */
public class NettyClientInitializer extends ChannelInitializer<SocketChannel> {

    protected BlockingQueue<Request> receiveQueue;
    public NettyClientInitializer(BlockingQueue<Request> receiveQueue) {
        this.receiveQueue = receiveQueue;
    }

    @Override
    public void initChannel(SocketChannel ch) throws Exception {
        // create a new ChannelPipeline to deal with channel data
        ChannelPipeline p = ch.pipeline();

        // add decoders to the pipeline to receive data from the server
        p.addLast("frameDecoder", new ProtobufVarint32FrameDecoder());
        p.addLast("protobufDecoder", new ProtobufDecoder(Request.getDefaultInstance()));

        // add encoders to the pipeline to send data to the server
        p.addLast("frameEncoder", new ProtobufVarint32LengthFieldPrepender());
        p.addLast("protobufEncoder", new ProtobufEncoder());

        // add the handler to the pipeline to construct/send data
        p.addLast("handler", new NettyClientHandler(receiveQueue));
    }
}
