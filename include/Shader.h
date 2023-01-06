#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <unordered_map>

struct ShaderProgramSource {
	std::string VertexSource;
	std::string FragmentSource;
};

class Shader
{
public:
	Shader(const std::string& filename);
	~Shader();
	
	void Bind() const;
	void UnBind() const;

	ShaderProgramSource ParseShader(const std::string& filepath);
	unsigned int CompileShader(unsigned int  type, const std::string& source);
    unsigned int  CreateShader(const std::string vertexShader, const std::string fragmentShader);
	

	void SetUniform1i(const std::string& name, int value);
	void SetUniform1f(const std::string& name, float value);
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);

private:
	unsigned int m_RendererID;
	std::string m_Filepath;
	std::unordered_map <std::string, int> m_UniformLocationCache;
	int GetUniformLocation(const std::string& name);
};

#endif
