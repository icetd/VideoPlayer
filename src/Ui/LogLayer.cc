#include "LogLayer.h"

AppLog applog;

void LogLayer::OnAttach()
{
}

void LogLayer::OnUpdate(float ts)
{
    ShowFeedbackLog();
}

void LogLayer::OnDetach()
{
}

void LogLayer::OnUIRender()
{
}

void LogLayer::ShowFeedbackLog()
{
    ImGui::Begin(u8"日志");
    applog.Draw(u8"日志");
	ImGui::End();
}