#ifndef VIDEO_VIEW_H
#define VIDER_VIEW_H

#include <memory>
#include <vector>
#include <string>
#include <mutex>
#include "Texuture.h"
#include "VideoCapture.h"
#include "VideoPackage.h"
#include "MThread.h"
#include "TimeStamp.h"
#include "Utils.h"

class VideoView : public MThread
{
public:
    VideoView();
    ~VideoView();
    void OnUpdate();
    void run() override;
    void stopPlay();

private:
    void OnRender();
    void OnImGuiRender();
    void startPlay();

    void saveFrameAsJPEG(const std::vector<uint8_t> &frameData, int width, int height, const std::string &filename);
    void ShowImagePopup(const char *popup_title, const std::string &path, bool *is_open);

    std::unique_ptr<Texture> m_TexTure;
    std::unique_ptr<VideoCapture> m_VideoCapture;
    std::unique_ptr<VideoPackage> m_VideoPackage;
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
    bool isShowPicture;
    bool isSave;
    std::string cur_picture_path;
    char m_url[128] = "";

    AppLog m_log;
};

#endif