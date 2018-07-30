package com.tzutalin.dlibtest;

import android.content.Context;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.media.Image;
import android.media.Image.Plane;
import android.media.ImageReader;
import android.media.ImageReader.OnImageAvailableListener;
import android.os.Handler;
import android.os.Trace;
import android.util.Log;
import android.view.Display;
import android.view.WindowManager;

import com.tzutalin.dlib.Constants;
import com.tzutalin.dlib.FaceDet;
import com.tzutalin.dlib.VisionDetRet;
import com.tzutalin.dlibtest.nativeclass.OpencvNativeClass;

import junit.framework.Assert;

import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.Scalar;

/**
 * Class that takes in preview frames and converts the image to Bitmaps to process with dlib lib.
 */
public class OnGetImageListener implements OnImageAvailableListener {

    private static final int INPUT_SIZE = 320;
    private static final String TAG = OnGetImageListener.class.getName();

    private int mScreenRotation = 90;

    private int mPreviewWdith = 0;
    private int mPreviewHeight = 0;
    private byte[][] mYUVBytes;
    private int[] mRGBBytes = null;
    private Bitmap mRGBframeBitmap = null;
    private Bitmap mCroppedBitmap = null;

    private boolean mIsComputing = false;
    private Handler mInferenceHandler;

    private Context mContext;
    private FaceDet mFaceDet;
    private FloatingCameraWindow mWindow;
    private Paint mFaceLandmardkPaint;

    public void initialize(
            final Context context,
            final Handler handler) {

        this.mContext = context;
        this.mInferenceHandler = handler;
        mFaceDet = new FaceDet(Constants.getFaceShapeModelPath());
        mWindow = new FloatingCameraWindow(mContext);

        mFaceLandmardkPaint = new Paint();
        mFaceLandmardkPaint.setColor(Color.GREEN);
        mFaceLandmardkPaint.setStrokeWidth(2);
        mFaceLandmardkPaint.setStyle(Paint.Style.STROKE);

    }

    public void deInitialize() {
        synchronized (OnGetImageListener.this) {
//            if (mFaceDet != null) {
//                mFaceDet.release();
//            }

            if (mWindow != null) {
                mWindow.release();
            }
        }
    }

    private void drawResizedBitmap(final Bitmap src, final Bitmap dst) {

        Display getOrient = ((WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
        int orientation = Configuration.ORIENTATION_UNDEFINED;
        Point point = new Point();
        getOrient.getSize(point);
        int screen_width = point.x;
        int screen_height = point.y;
        //Log.d(TAG, String.format("screen size (%d,%d)", screen_width, screen_height));
        if (screen_width < screen_height) {
            orientation = Configuration.ORIENTATION_PORTRAIT;
            mScreenRotation = 90;
        } else {
            orientation = Configuration.ORIENTATION_LANDSCAPE;
            mScreenRotation = 0;
        }

        Assert.assertEquals(dst.getWidth(), dst.getHeight());
        final float minDim = Math.min(src.getWidth()/2, src.getHeight()/2);

        final Matrix matrix = new Matrix();

        // We only want the center square out of the original rectangle.
        final float translateX = -Math.max(0, (src.getWidth() - minDim) / 2);
        final float translateY = -Math.max(0, (src.getHeight() - minDim) / 2);
        matrix.preTranslate(translateX, translateY);

        final float scaleFactor = dst.getHeight() / minDim;
        matrix.postScale(scaleFactor, scaleFactor);

        // Rotate around the center if necessary.
        if (mScreenRotation != 0) {
            matrix.postTranslate(-dst.getWidth() / 2.0f, -dst.getHeight() / 2.0f);
            matrix.postRotate(mScreenRotation);
            matrix.postTranslate(dst.getWidth() / 2.0f, dst.getHeight() / 2.0f);
        }

        final Canvas canvas = new Canvas(dst);
        canvas.drawBitmap(src, matrix, null);
    }

    private Mat convertBitmapToMat(Bitmap rgbaImage){

        // convert Java Bitmap into OpenCV Mat
        Mat rgbaMat = new Mat(rgbaImage.getWidth(), rgbaImage.getHeight(), CvType.CV_8UC4, Scalar.all(255));
        Bitmap bm32 = rgbaImage.copy(Config.ARGB_8888,true);
        Utils.bitmapToMat(bm32,rgbaMat);

        //Mat grayMat = new Mat (rgbaImage.getHeight(), rgbaImage.getWidth(), CvType.CV_8UC1, Scalar.all(255));
        //Imgproc.cvtColor(rgbaMat, grayMat, Imgproc.COLOR_RGB2GRAY, 1);

        //Mat rgbMat = new Mat(rgbaImage.getWidth(), rgbaImage.getHeight(), CvType.CV_8UC3, Scalar.all(255));
        //Imgproc.cvtColor(rgbaMat, rgbMat, Imgproc.COLOR_RGB2BGR, 3);

        return rgbaMat;
    }

    private Bitmap converMatToBitmap(Mat rgbaMat){

        Bitmap bmOutput = Bitmap.createBitmap(rgbaMat.width(),rgbaMat.height(), Config.ARGB_8888);
        //Bitmap bmOutput = Bitmap.createBitmap(rgbaMat.rows(),rgbaMat.cols(), Config.ARGB_4444);
        Utils.matToBitmap(rgbaMat,bmOutput);

        return bmOutput;
    }

    @Override
    public void onImageAvailable(final ImageReader reader) {
        Image image = null;
        try {
            image = reader.acquireLatestImage();

            if (image == null) {
                return;
            }

            // No mutex needed as this method is not reentrant.
            if (mIsComputing) {
                image.close();
                return;
            }
            mIsComputing = true;

            Trace.beginSection("imageAvailable");

            final Plane[] planes = image.getPlanes();

            // Initialize the storage bitmaps once when the resolution is known.
            if (mPreviewWdith != image.getWidth() || mPreviewHeight != image.getHeight()) {
                mPreviewWdith = image.getWidth();
                mPreviewHeight = image.getHeight();

                Log.d(TAG, String.format("mPreview: Initializing[mPreview] at size %dx%d", mPreviewWdith, mPreviewHeight));

                mRGBBytes = new int[mPreviewWdith * mPreviewHeight];
                mRGBframeBitmap = Bitmap.createBitmap(mPreviewWdith, mPreviewHeight, Config.ARGB_8888);
                mCroppedBitmap = Bitmap.createBitmap(mPreviewWdith, mPreviewHeight, Config.ARGB_8888);

                Log.d(TAG, String.format("mCroppedBitmap Initializing[mCroppedBitmap] at size %dx%d", mCroppedBitmap.getWidth(), mCroppedBitmap.getHeight()));

                mYUVBytes = new byte[planes.length][];
                for (int i = 0; i < planes.length; ++i) {
                    mYUVBytes[i] = new byte[planes[i].getBuffer().capacity()];
                }
            }

            for (int i = 0; i < planes.length; ++i) {
                planes[i].getBuffer().get(mYUVBytes[i]);
            }

            final int yRowStride = planes[0].getRowStride();
            final int uvRowStride = planes[1].getRowStride();
            final int uvPixelStride = planes[1].getPixelStride();

            // convert image YUV420 to ARGB8888
            ImageUtils.convertYUV420ToARGB8888(
                    mYUVBytes[0],
                    mYUVBytes[1],
                    mYUVBytes[2],
                    mRGBBytes,
                    mPreviewWdith,
                    mPreviewHeight,
                    yRowStride,
                    uvRowStride,
                    uvPixelStride,
                    false);

            image.close();
        } catch (final Exception e) {
            if (image != null) {
                image.close();
            }
            Log.e(TAG, "Exception!", e);
            Trace.endSection();
            return;
        }

        mRGBframeBitmap.setPixels(mRGBBytes, 0, mPreviewWdith, 0, 0, mPreviewWdith, mPreviewHeight);

        //Log.d(TAG, String.format("Initializing[mRGBframeBitmap] at size %dx%d", mRGBframeBitmap.getWidth(), mRGBframeBitmap.getHeight()));

        // you need active the function drawResized
        //drawResizedBitmap(mRGBframeBitmap, mCroppedBitmap);
        mCroppedBitmap = mRGBframeBitmap;

        //Log.d(TAG, String.format("mCroppedBitmap Initializing[mCroppedBitmap] at size %dx%d", mCroppedBitmap.getWidth(), mCroppedBitmap.getHeight()));


        mInferenceHandler.post(
                new Runnable() {
                    @Override
                    public void run() {

                        long startTime = System.currentTimeMillis();

                        synchronized (OnGetImageListener.this) {

                            Mat matInput = convertBitmapToMat(mCroppedBitmap);
                            Mat matOutput = new Mat(mCroppedBitmap.getWidth(), mCroppedBitmap.getWidth(), CvType.CV_8UC1, Scalar.all(255));

                            OpencvNativeClass.covertGray(matInput.getNativeObjAddr(), matOutput.getNativeObjAddr());
                            OpencvNativeClass.FindFeatures(matOutput.getNativeObjAddr());

                            mCroppedBitmap = converMatToBitmap(matOutput);
                            //ImageUtils.saveBitmap(mCroppedBitmap);

                        }
                        long endTime = System.currentTimeMillis();
                        Log.d(TAG,"Time cost: " + String.valueOf((endTime - startTime) / 10000f) + " sec");
                        //Log.d(TAG,"FPS cost: " + String.valueOf(1/((endTime - startTime) / 10000f)) + " framas/sec");

                        //show the image process
                        mWindow.setRGBBitmap(mCroppedBitmap);
                        mIsComputing = false;
                    }
                });

        Trace.endSection();
    }
}
