//
// Created by saisai on 2018/4/11 0011.
//

#include <jni.h>
#include "include/libavcodec/avcodec.h"
#include "include/libavformat/avformat.h"
#include <android/log.h>
#include "include/libavutil/mathematics.h"

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
        jobject jOB,jstring filename,jstring filename_v,jstring filename_a){

    in_filename=(*env)->GetStringUTFChars(env,filename,0);
    out_filename_v=(*env)->GetStringUTFChars(env,filename_v,0);
    out_filename_a=(*env)->GetStringUTFChars(env,filename_a,0);

    if ((ret = init_demuxer()) < 0)
//        errReport("init_demuxer", ret);
        __android_log_print(ANDROID_LOG_ERROR,"jni","error = %d",ret);
    while (1) {
        AVFormatContext *ofmt_ctx;
        AVStream *in_stream, *out_stream;
        if (av_read_frame(ifmt_ctx, &pkt) < 0)
            break;
        in_stream = ifmt_ctx->streams[pkt.stream_index];
        if (pkt.stream_index == videoindex) {
            out_stream = ofmt_ctx_v->streams[0];
            ofmt_ctx = ofmt_ctx_v;
//            printf("\nv#s:%d\tp:%lld", pkt.size, pkt.pts);
            __android_log_print(ANDROID_LOG_ERROR,"jni","pkt.size = %d  pts = %lld",pkt.size, pkt.pts);
        }
        else if (pkt.stream_index == audioindex) {
            out_stream = ofmt_ctx_a->streams[0];
            ofmt_ctx = ofmt_ctx_a;
            __android_log_print(ANDROID_LOG_ERROR,"jni","pkt.size = %d  pts = %lld",pkt.size, pkt.pts);
        }
        else {
            continue;
        }
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        pkt.stream_index = 0;

        if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
            __android_log_print(ANDROID_LOG_ERROR,"jni","Error muxing packet");

            break;
        }
        //printf("Write %8d frames to output file\n",frame_index);
        av_free_packet(&pkt);
        frame_index++;
    }
    av_write_trailer(ofmt_ctx_a);
    av_write_trailer(ofmt_ctx_v);
    avformat_close_input(&ifmt_ctx);
    if (ofmt_ctx_a && !(ofmt_a->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx_a->pb);
    if (ofmt_ctx_v && !(ofmt_v->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx_v->pb);
    avformat_free_context(ofmt_ctx_a);
    avformat_free_context(ofmt_ctx_v);

    if (ret < 0 && ret != AVERROR_EOF) {
        __android_log_print(ANDROID_LOG_ERROR,"jni","Error occurred");
        return -1;
    }
    printf("successed.\n");
    __android_log_print(ANDROID_LOG_ERROR,"jni","successed");
    return 0;
}
