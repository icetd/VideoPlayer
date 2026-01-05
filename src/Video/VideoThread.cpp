#include "VideoThread.h"

VideoThread::VideoThread() :
	isStartCapture(false),
	isStart(false),
    isDecodeSucceed(false)
{
	m_VideoCapture = std::make_unique<VideoCapture>();
}

VideoThread::~VideoThread()
{
}


void VideoThread::SetOnRenderDataCallback(std::function<void(std::vector<uint8_t>&&)> callback)
{
	OnRenderDataCallback = std::move(callback);
}

void VideoThread::StartPlay()
{
	bool re = false;
	m_VideoCapture->init();
    m_VideoCapture->setReconnectParams(100, 2000);
	re = m_VideoCapture->open(m_Url.c_str());
	if (!re) {
		isStart = false;
		return;
	}
	m_data = (uint8_t*)malloc(0);
	m_Width = m_VideoCapture->getWidth();
	m_Height = m_VideoCapture->getHeight();
	isStartCapture = true;
}

void VideoThread::StopPlay()
{
	isStartCapture = false;
    isDecodeSucceed = false;
	m_VideoCapture->close();
	if (m_data)
		free(m_data);
	m_data = nullptr;
}

void VideoThread::run()
{
	while (!this->isStoped()) {
		if (isStart == true && isStartCapture == false) {
			StartPlay();
		}
		else if (isStart == false && isStartCapture == true) {
			StopPlay();
		}

		if (isStartCapture) {
			int lenght = m_Width * m_Height * 4;
			m_data = (uint8_t*)realloc(m_data, lenght);
			m_pts = 1;
			isDecodeSucceed = m_VideoCapture->decode(m_data, &m_pts);
			if (isDecodeSucceed) {
				OnRenderDataCallback(std::vector<uint8_t>(m_data, m_data + lenght));
			}
            
            if ((m_VideoCapture->getInputType() == "rtsp") |
                (m_VideoCapture->getInputType() == "rtmp") |
                (m_VideoCapture->getInputType() == "http")) {
			    this->sleepMs(10);
            } else {
                this->sleepMs(1000 / m_VideoCapture->getFps() - 5);
            }
		}
		else {
			this->sleepMs(100);
		}
	}
}