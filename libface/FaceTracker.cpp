#include "stdafx.h"
#include "FaceTracker.h"
#include "misc.h"

namespace e
{
    const int kHaarOptions = CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_DO_ROUGH_SEARCH;
    const char* kFaceCascade = "D:\\ThirdParty\\opencv-3.1\\etc\\haarcascades\\haarcascade_frontalface_alt2.xml";

    const int kMaxRetryTimes = 2;
    const float kScaleDefault = 0.25f;
    const float kInvScaleDefault = 1 / kScaleDefault;
    const float kWidenDefault = kScaleDefault * 0.15f;
    const float kScaleFactorDefault = 1.1f;
    const cv::Size kMinSizeDefault(15, 15);
    const cv::Size kMaxSizeDefault(60, 60);

    template<class T> 
    void limit(T & t, T a, T b)
    {
        if (t < a)
            t = a;
        else if (t > b)
            t = b;
    }

    inline int round(float x)
    {
        return (int)(x < 0 ? x - 0.5f : x + 0.5f);
    }

    inline void CheckRect(cv::Rect & rect, int w, int h)
    {
        limit(rect.x, 0, w-1);
        limit(rect.y, 0, h-1);
        limit(rect.width, 0, w - rect.x);
        limit(rect.height, 0, h - rect.y);
    }

    inline void ResizeRect(cv::Rect & rect, float scale)
    {
        rect.x = round(rect.x * scale);
        rect.y = round(rect.y * scale);
        rect.width = round(rect.width * scale);
        rect.height = round(rect.height * scale);
    }

    inline void WidenRect(cv::Rect & rect, int dx, int dy)
    {
        rect.x -= dx;
        rect.y -= dy;
        rect.width += dx * 2;
        rect.height += dy * 2;
    }
    //widen and check
    inline void WidenRect(cv::Rect & rect, int dx, int dy, int w, int h)
    {
        WidenRect(rect, dx, dy);
        CheckRect(rect, w, h);
    }

    CFaceTracker::CFaceTracker(void)
        : m_pImage(NULL)
        , m_pMatDetection(NULL)
        , m_bFaceLocated(false)
        , m_dwLastTime(0)
    {
        m_pFaceCascade = new cv::CascadeClassifier();
        if (!m_pFaceCascade->load(kFaceCascade))
        {
            LOGE(_T("face cascade load data failed!"));
            assert(0);
        }

        const char* pFileName = "f:\\flandmark_model.dat";
        m_pFlandmarkModel = flandmark_init(pFileName);
        if (!m_pFlandmarkModel)
        {
            LOGE(_T("flandmark model init failed!"));
            assert(0);
        }

        m_plfFlandmark = new double[2 * m_pFlandmarkModel->data.options.M];
		if (!m_plfFlandmark)
		{
			assert(0);
		}
    }

    CFaceTracker::~CFaceTracker(void)
    {
        if (m_pImage)
        {
            m_pImage->imageData = NULL;
            delete m_pImage;
            m_pImage = NULL;
        }

        if (m_pFaceCascade)
        {
            delete m_pFaceCascade;
            m_pFaceCascade = 0;
        }

        if (m_plfFlandmark)
        {
            delete[] m_plfFlandmark;
            m_plfFlandmark = NULL;
        }

        if (m_pFlandmarkModel)
        {
            flandmark_free(m_pFlandmarkModel);
            m_pFlandmarkModel = 0;
        }
    }

    void CFaceTracker::SetupMatrix(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
    {
        assert(kScaleDefault <= 1.0f);

        if (m_pImage == NULL)
        {
            m_pImage = new IplImage();
            memset(m_pImage, 0, sizeof(IplImage));
            m_pImage->nSize = sizeof(IplImage);
            m_pImage->depth = 8;
            m_pImage->nChannels = nBitCount / 8;
            m_pImage->width = nWidth;
            m_pImage->height = nHeight;
            m_pImage->imageSize = nSize;
            m_pImage->imageData = (char*)pData;
            m_pImage->widthStep = WidthBytes(nWidth*nBitCount);
        }
        
        if (m_pMatDetection == NULL)
        {
            m_pMatDetection = new cv::Mat();
            assert(m_pMatDetection);
        }

        assert(m_pImage->imageSize == nSize);
        assert(m_pImage->width == nWidth && m_pImage->height == nHeight);
        m_pImage->imageData =(char*)pData;
        cv::resize(cvarrToMat(m_pImage), *m_pMatDetection, cv::Size(), kScaleDefault, kScaleDefault, CV_INTER_LINEAR);
    }

    void CFaceTracker::SetupMatrix(cv::Rect roi, void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
    {
        if (m_pImage == NULL)
        {
            m_pImage = new IplImage();
            m_pImage->depth = 8;
            m_pImage->nChannels = nBitCount / 8;
            m_pImage->width = nWidth;
            m_pImage->height = nHeight;
            m_pImage->imageSize = nSize;
            m_pImage->imageData = (char*)pData;
            m_pImage->widthStep = WidthBytes(nWidth*nBitCount);
        }

        if (m_pMatDetection == NULL)
        {
			m_pMatDetection = new cv::Mat();
            assert(m_pMatDetection);
        }

        assert(m_pImage->imageSize == nSize);
        assert(m_pImage->width == nWidth && m_pImage->height == nHeight);
        m_pImage->imageData = (char*)pData;

        cv::Mat roiMat = cv::Mat(cvarrToMat(m_pImage), roi);
        cv::resize(roiMat, *m_pMatDetection, cv::Size(), kScaleDefault, kScaleDefault, CV_INTER_LINEAR);
    }

    inline void CFaceTracker::DrawRect(cv::Rect rect, int nPenSize, int nColor, void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
    {
        e::DrawRect(rect.x, rect.y, rect.width, rect.height, nPenSize, nColor, pData, nSize, nWidth, nHeight, nBitCount);
    }

    void CFaceTracker::DrawPoint(std::vector<cv::Point>& points, int nPenSize, int nColor, void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
    {
        for (size_t i = 0; i < points.size(); i++)
        {
            e::DrawPoint(points[i].x, points[i].y, nPenSize, nColor, pData, nSize, nWidth, nHeight, nBitCount);
        }
    }

    void CFaceTracker::OnSampleProc(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
    {
        //首次人脸检测
        if (!m_bFaceLocated)
        {
            DWORD dwTime = GetTickCount();
            if (m_dwLastTime != 0 && dwTime - m_dwLastTime < 500) return;
            //adapter and resize 
            SetupMatrix(pData, nSize, nWidth, nHeight, nBitCount);
            std::vector<cv::Rect> faces;
            m_pFaceCascade->detectMultiScale(*m_pMatDetection, faces, kScaleFactorDefault, 2, kHaarOptions, kMinSizeDefault, kMaxSizeDefault);

            for (size_t i = 0; i < faces.size(); i++)
            {
                cv::Rect rect = faces[i];
                ResizeRect(rect, kInvScaleDefault);
                m_prevRect = rect;
                m_faceRect = rect;
                m_bFaceLocated = true;
#if DRAW_FACERECT
                DrawRect(rect, 2, E_RGB(255, 0, 0), pData, nSize, nWidth, nHeight, nBitCount);
#endif
            }

            m_dwLastTime = dwTime;
            static int nTrackTimes = 0;
            LOGD(_T("face tracek restart: %d"), nTrackTimes++);
        }
        else//根据前一帧检测的数据，进行跟踪处理
        {
            int nRetryTimes = 0;
            const float fScale = kScaleDefault;
            const float fInvScale = kInvScaleDefault;
            const float dx = nWidth * kWidenDefault;
            const float dy = nHeight * kWidenDefault;
            cv::Rect searchRect = m_faceRect;
_again:
            //expend the prev face rect to search
            WidenRect(searchRect, round(dx), round(dy), nWidth, nHeight);
#if DRAW_FACERECT
            DrawRect(searchRect, 1, E_RGB(0,0,255), pData, nSize, nWidth, nHeight, nBitCount);
#endif
            SetupMatrix(searchRect, pData, nSize, nWidth, nHeight, nBitCount);
            //setup face size
            std::vector<cv::Rect> faces;
            cv::Size faceSize(round(m_faceRect.width*fScale), round(m_faceRect.height*fScale));
            cv::Size offset(round(faceSize.width*0.25f), round(faceSize.height*0.25f));
            //range of feature size
            m_pFaceCascade->detectMultiScale(*m_pMatDetection, faces, kScaleFactorDefault, 2, kHaarOptions, faceSize-offset, faceSize+offset);

            m_bFaceLocated = false;
            for (size_t i = 0; i < faces.size(); i++)
            {
                cv::Rect rect = faces[i];
                ResizeRect(rect, fInvScale);
                rect.x += searchRect.x;
                rect.y += searchRect.y;
                CheckRect(rect, nWidth, nHeight);
                
                if (m_faceRect.contains(rect.tl()) && m_faceRect.contains(rect.br()))
                {
                    rect = m_faceRect;
                }
                else if (rect.contains(m_faceRect.tl()) && rect.contains(m_faceRect.br()))
                {
                    m_faceRect = rect;
                }

                if (abs(rect.x - m_faceRect.x) >= 5 || abs(rect.y - m_faceRect.y) >= 5)
                {
                    m_faceRect = rect;
                }
                m_prevRect = rect;
                m_bFaceLocated = true;
#if DRAW_FACERECT
                DrawRect(m_faceRect, 2, E_RGB(0,255,0), pData, nSize, nWidth, nHeight, nBitCount);
#endif
                //眼睛、鼻子、嘴巴定位处理
                cv::Mat grayMat;
                cv::Mat faceMat = cv::Mat(cvarrToMat(m_pImage), searchRect);
                cv::cvtColor(faceMat, grayMat, CV_RGB2GRAY);
                const float fMarkScale = kScaleDefault;//45.0f / m_prevRect.width;
                const float fInvMarkScale = 1.0f / fMarkScale;
                cv::resize(grayMat, grayMat, cv::Size(), fScale, fMarkScale, CV_INTER_LINEAR);

                rect = m_prevRect;
                rect.x -= searchRect.x;
                rect.y -= searchRect.y;
                ResizeRect(rect, fMarkScale);
                int bbox[4] = { rect.x, rect.y, rect.x+rect.width, rect.y+rect.height };
                int margin[2] = { 10, 10 };
                double *landmarks = m_plfFlandmark;

                if (!flandmark_detect(&IplImage(grayMat), bbox, m_pFlandmarkModel, landmarks, margin))
                {
// 					cvCircle(m_pSrcMat
// 						, cvPoint(int(landmarks[0] * fInvMarkScale + searchRect.x), int(landmarks[1] * fInvMarkScale + searchRect.y))
// 						, 3, CV_RGB(0, 0, 255)
// 						, CV_FILLED);

                    for (int i = 2; i < 2 * m_pFlandmarkModel->data.options.M; i += 2)
                    {
                        cvCircle(m_pImage
                            , cvPoint(int(landmarks[i] * fInvMarkScale + searchRect.x), int(landmarks[i + 1] * fInvMarkScale + searchRect.y))
                            , 3, CV_RGB(255, 0, 0)
                            , CV_FILLED);
                    }
                    LOGD(_T("flandmark box : %d,%d,%d,%d"), bbox[0], bbox[1], bbox[2]-bbox[0], bbox[3]-bbox[1]);
                }
            }// end for each face

            //跟踪失败，再次检测或结束
            if (!m_bFaceLocated)
            {
                static int nFailedTimes = 0;
                LOGD(_T("face track failed: %d"), nFailedTimes++);
                cv::imwrite("f:\\temp\\loss.bmp", *m_pMatDetection);
                if (++nRetryTimes < kMaxRetryTimes) goto _again;           
#if DRAW_FACERECT
                DrawRect(m_prevRect, 2, E_RGB(0, 255, 252), pData, nSize, nWidth, nHeight, nBitCount);
#endif
            }          
        }
    }
}