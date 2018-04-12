package com.shunyi.ffmpegmodule;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.RelativeLayout;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("ffmpegmodule");
    }

    private SurfaceView surfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
//        TextView tv = (TextView) findViewById(R.id.sample_text);
//        tv.setText(stringFromJNI()+"");
        final String dir= Environment.getExternalStorageDirectory().getAbsolutePath()+"/";

        surfaceView = (SurfaceView) findViewById(R.id.surface_view);
        final SurfaceHolder holder = surfaceView.getHolder();
//        holder.addCallback(new SurfaceHolder.Callback() {
//            @Override
//            public void surfaceCreated(final SurfaceHolder holder) {
//                new Thread(new Runnable() {
//                    @Override
//                    public void run() {
//                        main(dir+"/独角戏.avi",
//                                dir+"/三个音轨.h264",
//                                dir+"/三个音轨.aac",
//                                holder);
//
//                    }
//                }).start();
//            }
//
//            @Override
//            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
//
//            }
//
//            @Override
//            public void surfaceDestroyed(SurfaceHolder holder) {
//
//            }
//        });

        findViewById(R.id.btn_start).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        int result = setSource(Environment.getExternalStorageDirectory().getAbsolutePath()+"/三个音轨.ts", holder.getSurface());
                        if(result < 0){
                            Log.e("tag", "mediaPlayer-->setup");
                            return;
                        }
                        play();
                    }
                }).start();
            }
        });

        DisplayMetrics dm = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(dm);
        int mSurfaceViewWidth = dm.widthPixels;
        int mSurfaceViewHeight = dm.heightPixels;

        int h = 1080 * 1080 / (1920-66);
        int margin = (mSurfaceViewHeight - h) / 2;
        RelativeLayout.LayoutParams lp = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.MATCH_PARENT,
                RelativeLayout.LayoutParams.MATCH_PARENT);
        lp.setMargins(0,margin, 0, margin);
        surfaceView.setLayoutParams(lp);

    }


    /**
     * @param sampleRateInHz 当前 声音的频率
     * @param channelsNB 声道个数
     * @return
     */
    public AudioTrack createAudioTrack(int sampleRateInHz,int channelsNB) {

        int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
        //声道布局，默认设置立体声
        int channelConfig;
        if(channelsNB==1){
            channelConfig = AudioFormat.CHANNEL_OUT_MONO;
        }else if(channelsNB==2){
            channelConfig = AudioFormat.CHANNEL_OUT_STEREO;
        }else {
            channelConfig = AudioFormat.CHANNEL_OUT_STEREO;
        }
        int bufferSizeInBytes = AudioTrack.getMinBufferSize(sampleRateInHz, channelConfig, audioFormat);

        AudioTrack audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC,
                sampleRateInHz, channelConfig,
                audioFormat, bufferSizeInBytes,
                AudioTrack.MODE_STREAM);

        return audioTrack;
    }


    public native int setSource(String filePath, Object surface);
    public native int play();
    public native void release();
}
