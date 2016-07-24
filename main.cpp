#include <opencv2/opencv.hpp>

bool selectObject = false;
int trackObject = 0;
cv::Rect selection;
cv::Mat image;

void onMouse( int event, int x, int y, int, void* ) {
    static cv::Point origin;
    if(selectObject) {
        selection.x = MIN(x, origin.x);
        selection.y = MIN(y, origin.y);
        selection.width = std::abs(x - origin.x);
        selection.height = std::abs(y - origin.y);
        selection &= cv::Rect(0, 0, image.cols, image.rows);
    }
    switch(event) {
        case CV_EVENT_LBUTTONDOWN:
            origin = cv::Point(x, y);
            selection = cv::Rect(x, y, 0, 0);
            selectObject = true;
            break;
        case CV_EVENT_LBUTTONUP:
            selectObject = false;
            if( selection.width > 0 && selection.height > 0 )
                trackObject = -1;
            break;
    }
}

int main( int argc, const char** argv )
{
    //cv::VideoCapture video(0);
    cv::VideoCapture video("video.mp4");
    cv::namedWindow( "CamShift at Shiyanlou", CV_WINDOW_NORMAL ); 	//靠窗口名识别的，若窗口名不一样则会新开一个窗口
    cv::setWindowProperty("CamShift at Shiyanlou", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
    cv::setMouseCallback( "CamShift at Shiyanlou", onMouse, 0 );

    cv::Mat frame, hsv, hue, mask, hist, backproj;
    cv::Rect trackWindow;
    int hsize = 16;
    float hranges[] = {0,180};
    const float* phranges = hranges;

    while(true) {
        video >> frame;
        if( frame.empty() )
            break;

        frame.copyTo(image);

        cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);

        if( trackObject ) {

            cv::inRange(hsv, cv::Scalar(0, 30, 10), cv::Scalar(180, 256, 256), mask);
            int ch[] = {0, 0};
            hue.create(hsv.size(), hsv.depth());
            cv::mixChannels(&hsv, 1, &hue, 1, ch, 1);

            if( trackObject < 0 ) {
                cv::Mat roi(hue, selection), maskroi(mask, selection);
                calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
                normalize(hist, hist, 0, 255, CV_MINMAX);

                trackWindow = selection;
                trackObject = 1;
            }

            calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
            backproj &= mask;
            cv::RotatedRect trackBox = CamShift(backproj, trackWindow, cv::TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));
            if( trackWindow.area() <= 1 ) {
                int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;
                trackWindow = cv::Rect(trackWindow.x - r, trackWindow.y - r,
                                   trackWindow.x + r, trackWindow.y + r) &
                cv::Rect(0, 0, cols, rows);
            }
            ellipse( image, trackBox, cv::Scalar(0,0,255), 3, CV_AA );

        }

        if( selectObject && selection.width > 0 && selection.height > 0 ) {
            cv::Mat roi(image, selection);
            bitwise_not(roi, roi);
        }

        imshow( "CamShift at Shiyanlou", image );
        char c = (char)cv::waitKey(1000/27.0);
        if( c == 27 )
            break;
    }
    cv::destroyAllWindows();
    video.release();
    return 0;
}
