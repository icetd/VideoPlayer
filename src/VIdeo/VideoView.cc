#include "VideoView.h"
#include "imgui.h"

#define IMGUI_DEMO_MARKER(section)  \
    do { if (GImGuiDemoMarkerCallback != NULL) \
            GImGuiDemoMarkerCallback(__FILE__, __LINE__, section, GImGuiDemoMarkerCallbackUserData); \
    } while (0)

VideoView::VideoView() : 
    isStartCapture(false),
    isStart(false),
    m_VideoCapture(nullptr),
    m_TexTure(nullptr),
    m_url("rtsp://192.168.2.128:8554/unicast")
{
    m_TexTure = std::make_unique<Texture>();
    m_VideoCapture = std::make_unique<VideoCapture>();
}

VideoView::~VideoView()
{
    if(isStartCapture) 
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
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_Stationary))
            ImGui::SetTooltip("Stop play rtps video.");

        ImGui::NewLine();
        ImGui::Text("Player Status:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(255, 0, 0, 255), "%s", (isStart ? "Start" : "Stop"));

        ImGui::Separator();
        ImGui::ShowStyleSelector("Theme");
        ImGui::NewLine();
        
        ImGui::Separator();
        ImGui::NewLine();
        ImGui::NewLine();
	    ImGui::TextColored(ImVec4(0, 255, 0, 255),"Application average %.3f ms/frame (%.1f FPS)", 
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
}

void VideoView::run()
{
    while(!this->isStoped()) {

        if(isStart == true && isStartCapture == false ) {
            startPlay();
        } else if (isStart == false && isStartCapture == true) {
            stopPlay();
        }

        if (isStartCapture)
        {
            int length = m_width * m_height * 4;
            m_data = (uint8_t *)realloc(m_data, length);
            m_pts = 1;
            isDcodeSucceed = m_VideoCapture->decode(m_data, &m_pts);
            if (isDcodeSucceed) {
                dataMutex.lock();
                std::vector<uint8_t> rgbData(m_data, m_data + length);
                m_FrameBufferList.push_back(std::move(rgbData));
                if (m_FrameBufferList.size() > 2) {    // set max buffer size
                    m_FrameBufferList.pop_back();
                }
                dataMutex.unlock();
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
    m_data = (uint8_t*)malloc(0);
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

