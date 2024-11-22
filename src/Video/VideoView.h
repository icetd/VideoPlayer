#ifndef VIDEO_VIEW_H
#define VIDER_VIEW_H

#include <memory>
#include <vector>
#include <string>
#include <mutex>
#include "Texuture.h"
#include "VideoCapture.h"
#include "MThread.h"
#include "turbojpeg.h"

class VideoView : public MThread
{
public:
    VideoView();
    ~VideoView();
    void OnUpdate();
    void run() override;
    void stopPlay();
    void saveAsImage();

private:
  
    void OnRender();
    void OnImGuiRender();
    void startPlay();

    
    std::unique_ptr <Texture> m_TexTure;
    std::unique_ptr <VideoCapture> m_VideoCapture;
    uint8_t *m_data;
    int64_t m_pts;
    std::vector<uint8_t> m_FrameBuffer;
    std::vector<std::vector<uint8_t>> m_FrameBufferList;

    std::mutex dataMutex;

    int m_width;
    int m_height;
    bool isStartCapture;
    bool isStart;
    bool isDcodeSucceed;
    char m_url[128] = "";
};

#endif