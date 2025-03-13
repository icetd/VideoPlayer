#include "VideoView.h"
#include "TimeStamp.h"
#include "imgui.h"
#include "ImGuiFileDialog.h"
#include "jpeglib.h"
#include "Texuture.h"
#include "Utils.h"

#ifdef WIN32
#include <Windows.h>
#else
#include <sys/stat.h>
#endif

#define IMGUI_DEMO_MARKER(section)                                                                   \
    do {                                                                                             \
        if (GImGuiDemoMarkerCallback != NULL)                                                        \
            GImGuiDemoMarkerCallback(__FILE__, __LINE__, section, GImGuiDemoMarkerCallbackUserData); \
    } while (0)

VideoView::VideoView() :
    isStartCapture(false),
    isStart(false),
    isSave(false),
    isDcodeSucceed(false),
    isShowPicture(false),
    m_VideoCapture(nullptr),
    m_TexTure(nullptr),
    m_url("rtsp://192.168.2.128:8554/unicast")
{
    m_TexTure = std::make_unique<Texture>();
    m_VideoCapture = std::make_unique<VideoCapture>();
    m_VideoPackage = std::make_unique<VideoPackage>();
}

VideoView::~VideoView()
{
    if (isStartCapture)
        stopPlay();
}

void VideoView::OnUpdate()
{
    {
        ImGui::Begin("Control");
        ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.2f);
        ImGui::AlignTextToFramePadding();

        static char str1[128] = "";
        ImGui::InputTextWithHint("input url", "set rtsp url here", m_url, IM_ARRAYSIZE(m_url));

        ImGui::NewLine();
        ImVec2 sz = ImVec2(-FLT_MIN, 0.0f);
        if (ImGui::Button(u8"开始", sz)) {
            isStart = true;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_Stationary))
            ImGui::SetTooltip("Start play rtps video.");

        if (ImGui::Button(u8"停止", sz)) {
            isStart = false;
            if (isSave) {
                isSave = false;
                m_VideoPackage->stop();
            }
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_Stationary))
            ImGui::SetTooltip("Stop play rtps video.");

        ImGui::NewLine();
        ImGui::Text("Player Status:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(255, 0, 0, 255), "%s", (isStart ? "Start" : "Stop"));

        ImGui::Separator();
        ImGui::NewLine();
        if (ImGui::Button(u8"拍照", sz) && isDcodeSucceed) {
            saveFrameAsJPEG(m_FrameBuffer, m_VideoCapture->getWidth(), m_VideoCapture->getHeight(),
                            "save/" + TimeStamp::now().toFormattedString(false) + ".jpg");
        }

        if (ImGui::Button(u8"打开保存的照片", sz)) {
            IGFD::FileDialogConfig config;
            config.path = "./save";
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".jpg", config);
        }

        // 根据当前状态显示按钮文本
        if (isSave) {
            if (ImGui::Button(u8"停止录像", sz)) {
                isSave = false;
                m_VideoPackage->stop();
            }
        } else {
            if (ImGui::Button(u8"开始录像", sz) && isDcodeSucceed) {
                isSave = true;
                std::string filename = "save/" + TimeStamp::now().toFormattedString(false) + ".mp4";
                m_VideoPackage->start(filename.c_str(), m_VideoCapture->getWidth(), m_VideoCapture->getHeight());
            }
        }
        // display
        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
            if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
                cur_picture_path = ImGuiFileDialog::Instance()->GetFilePathName();
                isShowPicture = true;
            }
            ImGuiFileDialog::Instance()->Close();
        }

        ShowImagePopup(u8"预览", cur_picture_path, &isShowPicture);

        ImGui::NewLine();
        ImGui::Separator();
        ImGui::ShowStyleSelector("Theme");
        ImGui::NewLine();

        ImGui::TextColored(ImVec4(0, 255, 0, 255), u8"%s", TimeStamp::now().toFormattedString().c_str());

        ImGui::TextColored(ImVec4(0, 255, 0, 255), "Application average %.3f ms/frame (%.1f FPS)",
                           1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::PopItemWidth();
        ImGui::End();
    }

    {
        ImGui::Begin("View");
        OnImGuiRender();
        OnRender();
        ImGui::End();
    }

    {
        ImGui::Begin(u8"日志");
        m_log.Draw(u8"日志");
        ImGui::End();
    }
}

void VideoView::run()
{
    while (!this->isStoped()) {
        if (isStart == true && isStartCapture == false) {
            startPlay();
        } else if (isStart == false && isStartCapture == true) {
            stopPlay();
        }

        if (isStartCapture) {
            int length = m_width * m_height * 4;
            m_data = (uint8_t *)realloc(m_data, length);
            m_pts = 1;
            isDcodeSucceed = m_VideoCapture->decode(m_data, &m_pts);
            if (isDcodeSucceed) {
                std::unique_lock<std::mutex> lock(dataMutex);
                if (m_FrameBufferList.size() > 2) { // set max buffer size
                    m_FrameBufferList.erase(m_FrameBufferList.begin());
                }
                m_FrameBufferList.emplace_back(m_data, m_data + length);
            }
        }
    }

    if (isStartCapture)
        stopPlay();
}

void VideoView::OnRender()
{
    if (!m_FrameBufferList.empty()) {
        dataMutex.lock();
        m_FrameBuffer = m_FrameBufferList.back();
        m_TexTure->bind(m_width, m_height, m_FrameBuffer.data());

        if (isSave)
            m_VideoPackage->saveVideo(m_FrameBuffer.data(), m_VideoCapture->getWidth(), m_VideoCapture->getHeight());
        m_FrameBufferList.pop_back();
        dataMutex.unlock();
    }
}

void VideoView::OnImGuiRender()
{
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    if (isStartCapture)
        ImGui::Image((ImTextureID)(intptr_t)m_TexTure->getId(), ImVec2(viewportSize.x, viewportSize.y), ImVec2(0, 0), ImVec2(1, 1));
    else
        ImGui::Image((ImTextureID)(intptr_t)0, ImVec2(viewportSize.x, viewportSize.y), ImVec2(1, 1), ImVec2(1, 1));
}

void VideoView::startPlay()
{
    bool re = false;
    m_VideoCapture->init();
    re = m_VideoCapture->open(m_url);
    if (!re) {
        isStart = false;
        return;
    }
    m_data = (uint8_t *)malloc(0);
    m_width = m_VideoCapture->getWidth();
    m_height = m_VideoCapture->getHeight();
    isStartCapture = true;
}

void VideoView::stopPlay()
{
    isStartCapture = false;
    m_VideoCapture->close();
    if (m_data)
        free(m_data);
    m_data = nullptr;
}

void createDirectory(const std::string &dirPath)
{
#ifdef WIN32
    // Windows 系统创建目录
    if (!CreateDirectory(dirPath.c_str(), NULL)) {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_ALREADY_EXISTS) {
            fprintf(stderr, "Error creating directory %s\n", dirPath.c_str());
        }
    }
#else
    if (mkdir(dirPath.c_str(), 0755) != 0) {
        if (errno != EEXIST) {
            fprintf(stderr, "Error creating directory %s\n", dirPath.c_str());
        }
    }
#endif
}

void VideoView::saveFrameAsJPEG(const std::vector<uint8_t> &frameData, int width, int height, const std::string &filename)
{
    // 提取文件的目录路径
    size_t pos = filename.find_last_of("/\\");
    std::string dirPath = (pos != std::string::npos) ? filename.substr(0, pos) : "";

    // 如果目录不存在，创建目录
    if (!dirPath.empty()) {
        createDirectory(dirPath);
    }

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    FILE *outfile = fopen(filename.c_str(), "wb");
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
        // 修正倒立问题：从最后一行开始往上写
        row_pointer[0] = &rgbData[(cinfo.image_height - 1 - cinfo.next_scanline) * width * 3];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);

    m_log.AddLog(u8"Saved frame as JPEG: %s\n", filename.c_str());
}

void VideoView::ShowImagePopup(const char *popup_title, const std::string &path, bool *is_open)
{
    if (*is_open) {
        ImGui::OpenPopup(popup_title);
    }

    if (ImGui::BeginPopupModal(popup_title, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        Texture tex(path, "none", false);
        ImVec2 imageSize(640, 480);
        ImGui::Image((ImTextureID)(intptr_t)tex.getId(), imageSize, ImVec2(0, 0), ImVec2(1, 1));

        // 添加按钮，点击后关闭弹窗
        if (ImGui::Button(u8"退出")) {
            *is_open = false; // 更新控制变量
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}