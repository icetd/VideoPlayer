#include <Windows.h>
#include "VideoLayer.h"
#include "../Core/Application.h"
#include "../Core/TimeStamp.h"
#include "imgui.h"
#include "jpeglib.h"
#include "../Utils/ImGuiFileDialog.h"
#include "../Utils/CreateFileDir.h"


void VideoLayer::OnAttach()
{
	m_Texture = std::make_unique<Texture>();
	m_VideoThread = std::make_unique<VideoThread>();
	m_VideoThread->SetOnRenderDataCallback(std::bind(&VideoLayer::OnRenderData, this, std::placeholders::_1));
	m_VideoThread->start();
	m_dataBufferList.clear();

    m_TextureShow = std::make_unique<Texture>();
    m_VideoThreadShow = std::make_unique<VideoThread>();
	m_VideoThreadShow->SetOnRenderDataCallback(std::bind(&VideoLayer::OnRenderDataShow, this, std::placeholders::_1));
    m_VideoThreadShow->start();
    m_dataBufferListShow.clear();


	m_config = new INIReader("./configs/url.ini");
	m_url = m_config->Get("VIDEO", "URL_VIDEO", "rtsp://192.168.2.113:8554/unicast");
    strncpy(url_str, m_url.c_str(), m_url.size());
    isSave = false;
    m_VideoPackage = std::make_unique<VideoPackage>();

    std::string dirPath = "save";
    // 如果目录不存在，创建目录
    if (!dirPath.empty()) {
        createDirectory(dirPath);
    }
}

void VideoLayer::OnUpdate(float ts)
{
	ShowVideoControl();
	ShowVideo();
}

void VideoLayer::OnDetach()
{
}

void VideoLayer::OnUIRender()
{
}

void VideoLayer::ShowVideoControl()
{
	ImGui::Begin(u8"视频");
	ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.2f);
	ImGui::AlignTextToFramePadding();

    ImGui::InputTextWithHint("input url", "set rtsp url here", url_str, IM_ARRAYSIZE(url_str));
    m_VideoThread->SetUrl((char *)url_str);
	static int radio_video1 = 0;
	ImGui::RadioButton(u8"打开", &radio_video1, 1);
	ImGui::SameLine(0, 10);
	ImGui::RadioButton(u8"关闭", &radio_video1, 0);
	ImGui::SameLine(0, 20);

	if (m_VideoThread->GetCaptureStatus()) {
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Start");
	} else {
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Stop");
	}
	m_VideoThread->SetStartStatus((int)radio_video1);

	ImGui::Separator();
	ImGui::NewLine();

    ImVec2 sz = ImVec2(-FLT_MIN, 30.0f);

    if (ImGui::Button(u8"拍照", sz) && m_VideoThread->GetDecodeStatus()) {
        saveFrameAsJPEG(m_dataBuffer, m_VideoThread->GetWidth(), m_VideoThread->GetHeight(), 
                        "save/" + TimeStamp::now().toFormattedString(false) + ".jpg");
    }

    ImGui::Separator();
    ImGui::NewLine();
    // 根据当前状态显示按钮文本
    if (isSave) {
        if (ImGui::Button(u8"停止录像", sz)) {
            isSave = false; 
            m_VideoPackage->stop();
        }
    } else {
        if (ImGui::Button(u8"开始录像", sz) && m_VideoThread->GetDecodeStatus()) {
            isSave = true; 
            std::string filename = "save/" + TimeStamp::now().toFormattedString(false) + ".mp4";
            m_VideoPackage->start(filename.c_str(), m_VideoThread->GetWidth(), m_VideoThread->GetHeight());
        }
    }

    ImGui::Separator();
    ImGui::NewLine();

    if (ImGui::Button(u8"预览保存的图片", sz)) {
        IGFD::FileDialogConfig config;
        config.path = "./save";
        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".jpg", config);
    }

    // display
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        { // action if OK
            cur_picture_path = ImGuiFileDialog::Instance()->GetFilePathName();
            isShowPicture = true;
        }
        ImGuiFileDialog::Instance()->Close();
    }

    ShowImagePopup(u8"预览图片", cur_picture_path, &isShowPicture);

    if (ImGui::Button(u8"预览保存的视频", sz)) {
        IGFD::FileDialogConfig config;
        config.path = "./save";
        ImGuiFileDialog::Instance()->OpenDialog("ChooseVideoKey", "Choose File", ".mp4", config);
    }

    // display
    if (ImGuiFileDialog::Instance()->Display("ChooseVideoKey"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        { // action if OK
            cur_video_path = ImGuiFileDialog::Instance()->GetFilePathName();
            isShowVideo = true;
            m_VideoThreadShow->SetUrl(cur_video_path.c_str());
        }
        ImGuiFileDialog::Instance()->Close();
    }

    m_VideoThreadShow->SetStartStatus((int)isShowVideo);
    ShowVideoPopup(u8"预览视频", cur_video_path, &isShowVideo);

    ImGui::TextColored(ImVec4(0.1f, 0.5f, 0.1f, 1.0f), "Application average %.3f ms/frame (%.1f FPS)",
					   1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::PopItemWidth();
	ImGui::End();
}

void VideoLayer::ShowVideo()
{
	ImGui::Begin(u8"图像");
	if (!m_dataBufferList.empty() && m_VideoThread->GetCaptureStatus())
	{
		bool a = m_dataBufferList.empty();
		mutex_data.lock();
		m_dataBuffer = m_dataBufferList.back();
		m_Texture->Bind(m_VideoThread->GetWidth(), m_VideoThread->GetHeight(), m_dataBuffer.data());

        if (isSave)
            m_VideoPackage->saveVideo(m_dataBuffer.data(), m_VideoThread->GetWidth(), m_VideoThread->GetHeight());

		m_dataBufferList.pop_back();
        mutex_data.unlock();
	} else {
	}
	OnRenderVideo();
	ImGui::End();
}

void VideoLayer::ShowVideoShow()
{
	if (!m_dataBufferListShow.empty() && m_VideoThreadShow->GetCaptureStatus()) {
		mutex_data_show.lock();
		m_dataBufferShow = m_dataBufferListShow.back();
        m_TextureShow->Bind(m_VideoThreadShow->GetWidth(), m_VideoThreadShow->GetHeight(), m_dataBufferShow.data());
		m_dataBufferListShow.pop_back();
        mutex_data_show.unlock();
	}
	else {
	}
	OnRenderVideoShow();
}

void VideoLayer::OnRenderData(std::vector<uint8_t>&& data)
{
	mutex_data.lock();
	m_dataBufferList.emplace_back(std::move(data));
	if (m_dataBufferList.size() > 3)
		m_dataBufferList.pop_back();
	mutex_data.unlock();
}


void VideoLayer::OnRenderVideo()
{
	ImVec2 viewportSize = ImGui::GetContentRegionAvail();
	if (m_VideoThread->GetCaptureStatus()) {
		ImGui::Image((ImTextureID)(intptr_t)m_Texture->getID(), ImVec2(viewportSize.x, viewportSize.y), ImVec2(0, 0), ImVec2(1, 1));
	}
	else {
	}
}

void VideoLayer::OnRenderDataShow(std::vector<uint8_t>&& data)
{
	mutex_data_show.lock();
	m_dataBufferListShow.emplace_back(std::move(data));
	if (m_dataBufferListShow.size() > 3)
		m_dataBufferListShow.pop_back();
	mutex_data_show.unlock();
}


void VideoLayer::OnRenderVideoShow()
{
	ImVec2 viewportSize = ImVec2(640, 480);
	if (m_VideoThreadShow->GetCaptureStatus()) {
		ImGui::Image((ImTextureID)(intptr_t)m_TextureShow->getID(), ImVec2(viewportSize.x, viewportSize.y), ImVec2(0, 0), ImVec2(1, 1));
	} 
    else {
	}
}

void VideoLayer::saveFrameAsJPEG(const std::vector<uint8_t>& frameData, int width, int height, const std::string& filename)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    FILE* outfile = fopen(filename.c_str(), "wb");
    if (!outfile) {
        fprintf(stderr, "Error opening output JPEG file %s\n", filename.c_str());
        return;
    }

    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3; // RGB 图像
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 100, TRUE); // 设置质量参数，90 为高质量

    jpeg_start_compress(&cinfo, TRUE);

    if (frameData.size() == 0) 
        return;

    std::unique_lock<std::mutex> lock(dataMutex);
    // 转换 RGBA 到 RGB
    std::vector<uint8_t> rgbData(width * height * 3);
    for (int i = 0; i < width * height; ++i) {
        rgbData[i * 3 + 0] = frameData[i * 4 + 0]; // R
        rgbData[i * 3 + 1] = frameData[i * 4 + 1]; // G
        rgbData[i * 3 + 2] = frameData[i * 4 + 2]; // B
    }

    JSAMPROW row_pointer[1];
    while (cinfo.next_scanline < cinfo.image_height) {
        // 正常写入：从上往下
        row_pointer[0] = &rgbData[cinfo.next_scanline * width * 3];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);

    applog.AddLog(u8"成功保存图片:  %s.\n", filename.c_str());
}

void VideoLayer::ShowImagePopup(const char* popup_title, const std::string &path, bool* is_open)
{
    if (*is_open) {
        ImGui::OpenPopup(popup_title);
    }

    if (ImGui::BeginPopupModal(popup_title, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        Texture tex(path, "none", "");
        ImVec2 imageSize(640, 480);
        ImGui::Image((ImTextureID)(intptr_t)tex.getID(), imageSize, ImVec2(0, 1), ImVec2(1, 0));

        // 添加按钮，点击后关闭弹窗
        if (ImGui::Button(u8"退出")) {
            *is_open = false;  // 更新控制变量
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void VideoLayer::ShowVideoPopup(const char* popup_title, const std::string &path, bool* is_open)
{
    if (*is_open) {
        ImGui::OpenPopup(popup_title);
    }

    if (ImGui::BeginPopupModal(popup_title, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

        ShowVideoShow();
        // 添加按钮，点击后关闭弹窗
        if (ImGui::Button(u8"退出")) {
            *is_open = false;  // 更新控制变量
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}