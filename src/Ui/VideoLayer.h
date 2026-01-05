#ifndef VIDEO_LAYER_H
#define VIDEO_LAYER_H

#include "../Core/Layer.h"
#include "../Video/VideoThread.h"
#include "../Graphics/Renderer/Texture.h"
#include "../Utils/INIReader.h"
#include "../Video/VideoPackage.h"
#include "LogLayer.h"
#include <mutex>

class VideoLayer : public Layer
{
protected:
	virtual void OnAttach() override;
	virtual void OnUpdate(float ts) override;
	virtual void OnDetach() override;
	virtual void OnUIRender() override;

private:

	void ShowVideoControl();
	void ShowVideo();
	void ShowVideoShow();

	std::unique_ptr<VideoThread> m_VideoThread;
	std::unique_ptr<Texture> m_Texture;
	std::string m_url;
	std::vector<std::vector<uint8_t>> m_dataBufferList;
	std::vector<uint8_t> m_dataBuffer;
	std::mutex mutex_data;
	void OnRenderData(std::vector<uint8_t>&& data);
	void OnRenderVideo();

    std::unique_ptr<VideoThread> m_VideoThreadShow;
	std::unique_ptr<Texture> m_TextureShow;
	std::vector<std::vector<uint8_t>> m_dataBufferListShow;
	std::vector<uint8_t> m_dataBufferShow;
	std::mutex mutex_data_show;
	void OnRenderDataShow(std::vector<uint8_t>&& data);
	void OnRenderVideoShow();

    void saveFrameAsJPEG(const std::vector<uint8_t>& frameData, int width, int height, const std::string& filename);
    std::unique_ptr <VideoPackage> m_VideoPackage;
    bool isSave;

    std::mutex dataMutex;

    std::string cur_picture_path;
    bool isShowPicture;
    void ShowImagePopup(const char* popup_title, const std::string &path, bool* is_open);

    std::string cur_video_path;
    bool isShowVideo;
    void ShowVideoPopup(const char* popup_title, const std::string &path, bool* is_open);

	INIReader *m_config;
    char url_str[128] = "";
};

#endif