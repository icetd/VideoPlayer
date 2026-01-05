#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>
#include <iostream>

# define M_PI           3.14159265358979323846  /* pi */

class Camera
{
public:

    Camera(glm::vec3 eye, glm::vec3 lookat, glm::vec3 upVector, float width, float height)
        : m_eye(std::move(eye))
        , m_lookAt(std::move(lookat))
        , m_upVector(std::move(upVector))
        , m_width(width)
        , m_height(height)
    {
        m_eyeInitial = m_eye;
        UpdateViewMatrix();
    }

    glm::mat4x4 GetViewMatrix() const { return m_viewMatrix; }
    glm::mat4x4 GetProjMatrix() const { return m_projMatrix; }
    glm::vec3 GetEye() const { return m_eye; }
	glm::vec3 GetUpVector() const { return m_upVector; }
	glm::vec3 GetLookAt() const { return m_lookAt; }
	glm::vec3 GetViewDir() const { return -glm::transpose(m_viewMatrix)[2]; }
	glm::vec3 GetRightVector() const { return glm::transpose(m_viewMatrix)[0]; }
	float GetFOV() const { return m_fov; }

	void SetFOV(float fov)
    { 
        m_fov = fov; 
    }

    void UpdateViewMatrix()
    {
        m_viewMatrix = glm::lookAt(m_eye, m_lookAt, m_upVector);
    }

    void UpdateProjMatrix()
    {
        m_projMatrix = glm::perspective(glm::radians(m_fov), m_width / m_height, 0.1f, 200000.0f );
    }

    void SetCameraView(glm::vec3 eye, glm::vec3 lookat, glm::vec3 up)
    {
        m_eye = std::move(eye);
        m_lookAt = std::move(lookat);
        m_upVector = std::move(up);
        UpdateViewMatrix();
    }

    void ArcBallCamera(float deltaX, float deltaY)
    {
        glm::vec4 position(GetEye().x, GetEye().y, GetEye().z, 1);
        glm::vec4 pivot(GetLookAt().x, GetLookAt().y, GetLookAt().z, 1);
        float deltaAngleX = (2 * M_PI / m_width);
        float deltaAngleY = (M_PI / m_height);
        float xAngle = deltaX * deltaAngleX;
        float yAngle = deltaY * deltaAngleY;

        xAngleSub += xAngle;
        yAngleSub += yAngle;
        
        float cosAngle = glm::dot(GetViewDir(), m_upVector);
        if (cosAngle * glm::sign(yAngle) > 0.99f)
            yAngle = 0;

        glm::mat4x4 rotationMatrixX(1.0f);
        rotationMatrixX = glm::rotate(rotationMatrixX, xAngle, m_upVector);
        position = (rotationMatrixX * (position - pivot)) + pivot;

        glm::mat4x4 rotationMatrixY(1.0f);
        rotationMatrixY = glm::rotate(rotationMatrixY, yAngle, GetRightVector());
        glm::vec3 finalPosition = (rotationMatrixY * (position - pivot)) + pivot;

        SetCameraView(finalPosition, GetLookAt(), m_upVector);
    }

    void PanCamera(glm::vec2 deltaMouse)
    {
        glm::vec3 cameraRight = glm::normalize(glm::cross(m_lookAt - m_eye, m_upVector));
        glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, m_lookAt - m_eye));

        glm::vec3 pan = m_panSpeed * (deltaMouse.x * cameraRight - deltaMouse.y * cameraUp);

        m_eye += pan;
        m_lookAt += pan;

        UpdateViewMatrix();
    }

    void ProcessMouseScroll(float yoffset)
    {
        m_fov -= (float)yoffset;
        if (m_fov < 1.0f)
            m_fov = 1.0f;

        if (m_fov > 45.0f)
            m_fov = 45.0f;
    }

    void Follow(glm::vec3 targetPos, glm::vec3 offset)
    {
        // 计算相机的新位置，使用偏移量
        m_eye = targetPos + offset;
        m_lookAt = targetPos; // 相机始终注视目标
        UpdateViewMatrix();   // 更新视图矩阵
    }

private:
    glm::mat4x4 m_viewMatrix;
    glm::mat4x4 m_projMatrix;
    glm::vec3 m_eye;                // Camera position in 3D
    glm::vec3 m_eyeInitial;         // Camera position in 3D
    glm::vec3 m_lookAt;             // Point that the camera is looking at
    glm::vec3 m_upVector;           // Orientation of the camera
    float m_width;
    float m_height;
    float m_fov = 45;
    float m_panSpeed = 0.05f;

    float xAngleSub = 0.0;
    float yAngleSub = 0.0;
};

#endif