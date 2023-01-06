#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "video_reader/video_reader.h"
#include <iostream>
#include "Texture.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
	std::string url;
	std::cout << "Suppurt: mp4, flv, rtmp, rtsp" << std::endl;
	std::cout << "<example rtsp url> rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mp4" << std::endl;
	std::cout << "Please input your video : ";
	std::cin >> url;
	std::cout << url << std::endl;
Init:

	VideoReaderState* vr_state = new VideoReaderState();
	if (!video_reader_open(vr_state, url.c_str())) {
		printf("Couldn't open video file\n");
		return 1;
	}

	// Allocate frame buffer
	const int frame_width = vr_state->width;
	const int frame_height = vr_state->height;
	uint8_t* frame_data = new uint8_t[frame_width * frame_height * 4];


	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(640, 480, "Video Player", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK) {
		std::cout << "Error init glew API! " << std::endl;
	}

	std::cout << glGetString(GL_VERSION) << std::endl;
	{
		float position[] = {
			-1.0f, -1.0f, 0.0f, 1.0f,
			 1.0f, -1.0f, 1.0f, 1.0f,
			 1.0f,  1.0f, 1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f, 0.0f
		};

		GLuint indices[] = {
			0 , 1 , 2,
			2 , 3 , 0
		};
		

		/* vao && vbo */
		VertexArray va;
		VertexBuffer vb(position, 4 * 4 * sizeof(float));
		VertexBufferLayout layout;
		layout.Push<float>(2);
		layout.Push<float>(2);
		va.AddBuffer(vb, layout);
		IndexBuffer ib(indices, 6);
	

		/* shaderer */
		Shader shader("../res/shaders/Basic.shader");
		shader.Bind();
	
		Player player;
		player.Bind(2);
		shader.SetUniform1i("u_Texture", 2);

		va.UnBind();
		vb.Unbind();
		ib.Unbind();
		shader.UnBind();
		//texture.Unbind();

		Renderer renderer;    
		double pts_in_seconds = 0.0;
		glfwGetTime();


		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{

			renderer.Clear();
			
			shader.Bind();
			renderer.Draw(va, ib, shader);
			
			// read a new frame and load it to
			int64_t pts;
			if (!video_reader_frame(vr_state, frame_data, &pts)) {
				printf("Couldn't load video file\n");
				return 1;
			}
			static bool first_frame = true;
			if (first_frame) {
				glfwSetTime(0.0);
				first_frame = false;
			}

			if (pts_in_seconds != (pts * (double)vr_state->time_base.num / (double)vr_state->time_base.den)) {
				pts_in_seconds = (pts * (double)vr_state->time_base.num / (double)vr_state->time_base.den);
			} else {
				std::cout << "stop stream" << std::endl;
				glfwTerminate();
				delete frame_data;
				video_reader_close(vr_state);
				delete vr_state;
				goto Init;
			}
			
			while (pts_in_seconds > glfwGetTime()) {
				glfwWaitEventsTimeout(pts_in_seconds - glfwGetTime());
			}

			player.show(frame_width, frame_height, frame_data);
		  	player.Bind(2);
			
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}

	delete frame_data;
	video_reader_close(vr_state);
	delete vr_state;
	glfwTerminate();

	return 0;
}
