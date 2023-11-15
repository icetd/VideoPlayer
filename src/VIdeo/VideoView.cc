#include "VideoView.h"
#include "imgui.h"

VideoView::VideoView() : 
    isStartCapture(false),
    isStart(false),
    m_VideoCapture(nullptr),
    m_TexTure(nullptr)
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
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.1f, 0.1f, 0.1f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
        if (ImGui::Button("start", ImVec2(80, 40))) {
            isStart = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("stop", ImVec2(80, 40))) {
            isStart = false;
        }
        ImGui::PopStyleColor(3);

        ImGui::NewLine();
        ImGui::Text("Player Status:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(255, 0, 0, 255), "%s", (isStart ? "Start" : "Stop"));
        ImGui::NewLine();
	ImGui::TextColored(ImVec4(255, 0, 0, 255),"Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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
            startPlay("rtsp://192.168.2.128:8554/unicast");
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
                std::vector<uint8_t> rgbData(m_data, m_data + length);
                m_FrameBufferList.push_back(std::move(rgbData));
            }
        }
    }

    if (isStartCapture)
        stopPlay();
}

void VideoView::OnRender()
{
    if (!m_FrameBufferList.empty()) {
        m_FrameBuffer = m_FrameBufferList.back();
        m_TexTure->bind(m_width, m_height, m_FrameBuffer.data());
        m_FrameBufferList.pop_back();
    }
}

void VideoView::OnImGuiRender()
{
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    if (isStartCapture)
        ImGui::Image((ImTextureID)(intptr_t)m_TexTure->getId(), ImVec2(viewportSize.x, viewportSize.y), ImVec2(0, 1), ImVec2(1, 0));
    else 
        ImGui::Image((ImTextureID)(intptr_t)0, ImVec2(viewportSize.x, viewportSize.y), ImVec2(0, 1), ImVec2(1, 0));
}

void VideoView::startPlay(const char *url)
{
    bool re = false;
    m_VideoCapture->init();
    re = m_VideoCapture->open(url);
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
    m_data = nullptr;
}

