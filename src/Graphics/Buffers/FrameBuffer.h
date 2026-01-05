#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

class FrameBuffer 
{
public:
    FrameBuffer(float width, float height);
    ~FrameBuffer();
    unsigned int GetFrameTexture();
    void RescaleFrameBuffer(float width, float height);
    void Bind() const;
    void Unbind() const;

private:
    unsigned int m_fbo;
    unsigned int m_rbo;
    unsigned int m_texture;
};

#endif