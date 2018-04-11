//
// Created by saisai on 2018/4/11 0011.
//

#include <jni.h>
#include "include/libavcodec/avcodec.h"
#include "include/libavformat/avformat.h"
#include <android/log.h>
#include "include/libavutil/mathematics.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "include/libavutil/avutil.h"
#include "include/libavutil/imgutils.h"
#include "include/libswscale/swscale.h"

JNIEXPORT jint JNICALL Java_com_shunyi_ffmpegmodule_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject jOB);

AVOutputFormat *ofmt_a = NULL, *ofmt_v = NULL;
AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx_a = NULL, *ofmt_ctx_v = NULL;
AVPacket pkt;
int ret, i;
int videoindex = -1, audioindex = -1;
int frame_index = 0;

const char *in_filename = "in.ts";
const char *out_filename_v = "out.h264";
const char *out_filename_a = "out.aac";

jint Java_com_shunyi_ffmpegmodule_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject jOB){
   int version=avcodec_version();
    return version;
}


jint init_demuxer(){

    av_register_all();
    if (avformat_open_input(&ifmt_ctx, in_filename, 0, 0) < 0)
        return -1;
    if (avformat_find_stream_info(ifmt_ctx, 0) < 0)
        return -2;
    avformat_alloc_output_context2(&ofmt_ctx_v, NULL, NULL, out_filename_v);
    if (!ofmt_ctx_v) return -3;
    ofmt_v = ofmt_ctx_v->oformat;

    avformat_alloc_output_context2(&ofmt_ctx_a, NULL, NULL, out_filename_a);
    if (!ofmt_ctx_a) return -4;
    ofmt_a = ofmt_ctx_a->oformat;

    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVFormatContext *ofmt_ctx;
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVStream *out_stream = NULL;
        if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
            out_stream = avformat_new_stream(ofmt_ctx_v, in_stream->codec->codec);
            ofmt_ctx = ofmt_ctx_v;
        }
        else if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioindex = i;
            out_stream = avformat_new_stream(ofmt_ctx_a, in_stream->codec->codec);
            ofmt_ctx = ofmt_ctx_a;
        }
        else {
            break;
        }
        if (!out_stream) return -5;
        if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0)
            return -6;
        out_stream->codec->codec_tag = 0;
        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }
    __android_log_print(ANDROID_LOG_ERROR,"jni","================Input===============");

    av_dump_format(ifmt_ctx, 0, in_filename, 0);
    __android_log_print(ANDROID_LOG_ERROR,"jni","================Output===============");
    av_dump_format(ofmt_ctx_v, 0, out_filename_v, 1);
    __android_log_print(ANDROID_LOG_ERROR,"jni","++++++++++++++++++++++++++++++++++++");
    av_dump_format(ofmt_ctx_a, 0, out_filename_a, 1);
    __android_log_print(ANDROID_LOG_ERROR,"jni","====================================");
    if (!(ofmt_v->flags & AVFMT_NOFILE)) {
        if (avio_open(&ofmt_ctx_v->pb, out_filename_v, AVIO_FLAG_WRITE) < 0)
            return -7;
    }
    if (!(ofmt_a->flags & AVFMT_NOFILE)) {
        if (avio_open(&ofmt_ctx_a->pb, out_filename_a, AVIO_FLAG_WRITE) < 0)
            return -8;
    }

    if (avformat_write_header(ofmt_ctx_v, NULL) < 0)
        return -9;
    if (avformat_write_header(ofmt_ctx_a, NULL) < 0)
        return -10;
    return 0;
}

jint Java_com_shunyi_ffmpegmodule_MainActivity_main(
        JNIEnv *env,
        jobject jOB,jstring filename,jstring filename_v,jstring filename_a,jobject surface){

    __android_log_print(ANDROID_LOG_ERROR,"main","进来了");
    //获取native window
    ANativeWindow *nativeWindow=ANativeWindow_fromSurface(env,surface);
    __android_log_print(ANDROID_LOG_ERROR,"main","surface");

    in_filename=(*env)->GetStringUTFChars(env,filename,0);
    out_filename_v=(*env)->GetStringUTFChars(env,filename_v,0);
    out_filename_a=(*env)->GetStringUTFChars(env,filename_a,0);

//    if ((ret = init_demuxer()) < 0)
////        errReport("init_demuxer", ret);
//        __android_log_print(ANDROID_LOG_ERROR,"jni","error = %d",ret);
//    while (1) {
//        AVFormatContext *ofmt_ctx;
//        AVStream *in_stream, *out_stream;
//        if (av_read_frame(ifmt_ctx, &pkt) < 0)
//            break;
//        in_stream = ifmt_ctx->streams[pkt.stream_index];
//        if (pkt.stream_index == videoindex) {
//            out_stream = ofmt_ctx_v->streams[0];
//            ofmt_ctx = ofmt_ctx_v;
////            printf("\nv#s:%d\tp:%lld", pkt.size, pkt.pts);
//            __android_log_print(ANDROID_LOG_ERROR,"jni","pkt.size = %d  pts = %lld",pkt.size, pkt.pts);
//        }
//        else if (pkt.stream_index == audioindex) {
//            out_stream = ofmt_ctx_a->streams[0];
//            ofmt_ctx = ofmt_ctx_a;
//            __android_log_print(ANDROID_LOG_ERROR,"jni","pkt.size = %d  pts = %lld",pkt.size, pkt.pts);
//        }
//        else {
//            continue;
//        }
//        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
//        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
//        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
//        pkt.pos = -1;
//        pkt.stream_index = 0;
//
//        if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
//            __android_log_print(ANDROID_LOG_ERROR,"jni","Error muxing packet");
//
//            break;
//        }
//        //printf("Write %8d frames to output file\n",frame_index);
//        av_free_packet(&pkt);
//        frame_index++;
//    }
//    av_write_trailer(ofmt_ctx_a);
//    av_write_trailer(ofmt_ctx_v);
//    avformat_close_input(&ifmt_ctx);
//    if (ofmt_ctx_a && !(ofmt_a->flags & AVFMT_NOFILE))
//        avio_close(ofmt_ctx_a->pb);
//    if (ofmt_ctx_v && !(ofmt_v->flags & AVFMT_NOFILE))
//        avio_close(ofmt_ctx_v->pb);
//    avformat_free_context(ofmt_ctx_a);
//    avformat_free_context(ofmt_ctx_v);
//
//    if (ret < 0 && ret != AVERROR_EOF) {
//        __android_log_print(ANDROID_LOG_ERROR,"jni","Error occurred");
//        return -1;
//    }
//    printf("successed.\n");
//    __android_log_print(ANDROID_LOG_ERROR,"jni","successed");
//    return 0;

    av_register_all();
    __android_log_print(ANDROID_LOG_ERROR,"main","1");
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    // Open video file
    if (avformat_open_input(&pFormatCtx, in_filename, NULL, NULL) != 0) {

//        LOGD("Couldn't open file:%s\n", file_name);
        __android_log_print(ANDROID_LOG_ERROR,"main","Couldn't open file:%s",in_filename);
        return -1; // Couldn't open file
    }
    __android_log_print(ANDROID_LOG_ERROR,"main","2");

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
//        LOGD("Couldn't find stream information.");
        __android_log_print(ANDROID_LOG_ERROR,"main","Couldn't find stream information");
        return -1;
    }
    __android_log_print(ANDROID_LOG_ERROR,"main","3");

    // Find the first video stream
    int videoStream = -1, i;
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO
            && videoStream < 0) {
            videoStream = i;
        }
    }
    __android_log_print(ANDROID_LOG_ERROR,"main","4");
    if (videoStream == -1) {
//        LOGD("Didn't find a video stream.");
        __android_log_print(ANDROID_LOG_ERROR,"main","Didn't find a video stream");
        return -1; // Didn't find a video stream
    }
    __android_log_print(ANDROID_LOG_ERROR,"main","5");

    // Get a pointer to the codec context for the video stream
    AVCodecContext *pCodecCtx = pFormatCtx->streams[videoStream]->codec;

    // Find the decoder for the video stream
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
//        LOGD("Codec not found.");
        __android_log_print(ANDROID_LOG_ERROR,"main","Codec not found");
        return -1; // Codec not found
    }
    __android_log_print(ANDROID_LOG_ERROR,"main","6");

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
//        LOGD("Could not open codec.");
        __android_log_print(ANDROID_LOG_ERROR,"main","Could not open codec");
        return -1; // Could not open codec
    }
    __android_log_print(ANDROID_LOG_ERROR,"main","7");

    // 获取native window
//    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);

    // 获取视频宽高
    int videoWidth = pCodecCtx->width;
    int videoHeight = pCodecCtx->height;

    __android_log_print(ANDROID_LOG_ERROR,"main","videoWidth = %d   videoHeight = %d",videoWidth,videoHeight);

    // 设置native window的buffer大小,可自动拉伸
    ANativeWindow_setBuffersGeometry(nativeWindow, videoWidth, videoHeight,
                                     WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer windowBuffer;
    __android_log_print(ANDROID_LOG_ERROR,"main","8");

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
//        LOGD("Could not open codec.");
        __android_log_print(ANDROID_LOG_ERROR,"main","Could not open codec");
        return -1; // Could not open codec
    }
    __android_log_print(ANDROID_LOG_ERROR,"main","9");

    // Allocate video frame
    AVFrame *pFrame = av_frame_alloc();

    // 用于渲染
    AVFrame *pFrameRGBA = av_frame_alloc();
    if (pFrameRGBA == NULL || pFrame == NULL) {
//        LOGD("Could not allocate video frame.");
        __android_log_print(ANDROID_LOG_ERROR,"main","Could not allocate video frame");
        return -1;
    }

    // Determine required buffer size and allocate buffer
    // buffer中数据就是用于渲染的,且格式为RGBA
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, pCodecCtx->width, pCodecCtx->height,
                                            1);
    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, buffer, AV_PIX_FMT_RGBA,
                         pCodecCtx->width, pCodecCtx->height, 1);

    // 由于解码出来的帧格式不是RGBA的,在渲染之前需要进行格式转换
    struct SwsContext *sws_ctx = sws_getContext(pCodecCtx->width,
                                                pCodecCtx->height,
                                                pCodecCtx->pix_fmt,
                                                pCodecCtx->width,
                                                pCodecCtx->height,
                                                AV_PIX_FMT_RGBA,
                                                SWS_BILINEAR,
                                                NULL,
                                                NULL,
                                                NULL);

    int frameFinished;
    AVPacket packet;
    while (av_read_frame(pFormatCtx, &packet) >= 0) {
        // Is this a packet from the video stream?
        if (packet.stream_index == videoStream) {

            // Decode video frame
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

            // 并不是decode一次就可解码出一帧
            if (frameFinished) {

                // lock native window buffer
                ANativeWindow_lock(nativeWindow, &windowBuffer, 0);

                // 格式转换
                sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data,
                          pFrame->linesize, 0, pCodecCtx->height,
                          pFrameRGBA->data, pFrameRGBA->linesize);

                // 获取stride
                uint8_t *dst = (uint8_t *) windowBuffer.bits;
                int dstStride = windowBuffer.stride * 4;
                uint8_t *src = (pFrameRGBA->data[0]);
                int srcStride = pFrameRGBA->linesize[0];

                // 由于window的stride和帧的stride不同,因此需要逐行复制
                int h;
                for (h = 0; h < videoHeight; h++) {
                    memcpy(dst + h * dstStride, src + h * srcStride, srcStride);
                }

                ANativeWindow_unlockAndPost(nativeWindow);
            }

        }else{

        }
        av_packet_unref(&packet);
    }

    av_free(buffer);
    av_free(pFrameRGBA);

    // Free the YUV frame
    av_free(pFrame);

    // Close the codecs
    avcodec_close(pCodecCtx);

    // Close the video file
    avformat_close_input(&pFormatCtx);
    return 0;
}
