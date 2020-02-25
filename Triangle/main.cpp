#define GLEW_STATIC
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

int main() {
	// GLFW�̏�����
	if (!glfwInit())
		return 1;

	// �I���������̓o�^
	atexit(glfwTerminate);

	// OpenGL Version 4.6 Core Profile��I������
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// �E�B���h�E�̍쐬
	GLFWwindow* const window =
		glfwCreateWindow(640, 480, "Triangle", nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "Can't create GLFW window." << std::endl;
		return 1;
	}

	glfwMakeContextCurrent(window);

	// GLEW�̏�����
	if (glewInit() != GLEW_OK) {
		std::cerr << "Can't initialize GLEW." << std::endl;
		return 1;
	}

	// VSync��҂�
	glfwSwapInterval(1);

	// �w�i�F���w�肷��
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// �V�F�[�_�̃\�[�X
	const GLchar* vertexShaderSource =
		"#version 460\n"
		"layout (location = 0) in vec4 position;\n"
		"layout (location = 1) in vec3 vertexColor;\n"
		"out vec3 color;\n"
		"void main()\n"
		"{\n"
		"  color = vertexColor;\n"
		"  gl_Position = position;\n"
		"}\n";
	const GLchar* fragmentShaderSource =
		"#version 460\n"
		"in vec3 color;\n"
		"layout (location = 0) out vec4 fragment;\n"
		"void main()\n"
		"{\n"
		"  fragment = vec4(color, 1.0);\n"
		"}\n";

	// �v���O�����I�u�W�F�N�g���쐬
	const GLuint program = glCreateProgram();

	GLint status = GL_FALSE;
	GLsizei infoLogLength;

	// ���_�V�F�[�_�̃R���p�C��
	const GLuint vertexShaderObj = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderObj, 1, &vertexShaderSource, nullptr);
	glCompileShader(vertexShaderObj);
	glAttachShader(program, vertexShaderObj);

	// ���_�V�F�[�_�̃`�F�b�N
	glGetShaderiv(vertexShaderObj, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
		std::cerr << "Compile Error in Vertex Shader." << std::endl;
	glGetShaderiv(vertexShaderObj, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 1) {
		std::vector<GLchar> vertexShaderErrorMessage(infoLogLength);
		glGetShaderInfoLog(vertexShaderObj, infoLogLength, nullptr,
			vertexShaderErrorMessage.data());
		std::cerr << vertexShaderErrorMessage.data() << std::endl;
	}

	glDeleteShader(vertexShaderObj);

	// �t���O�����g�V�F�[�_�̃R���p�C��
	const GLuint fragmentShaderObj = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderObj, 1, &fragmentShaderSource, nullptr);
	glCompileShader(fragmentShaderObj);
	glAttachShader(program, fragmentShaderObj);

	// �t���O�����g�V�F�[�_�̃`�F�b�N
	glGetShaderiv(fragmentShaderObj, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
		std::cerr << "Compile Error in Fragment Shader." << std::endl;
	glGetShaderiv(fragmentShaderObj, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 1) {
		std::vector<GLchar> fragmentShaderErrorMessage(infoLogLength);
		glGetShaderInfoLog(fragmentShaderObj, infoLogLength, nullptr,
			fragmentShaderErrorMessage.data());
		std::cerr << fragmentShaderErrorMessage.data() << std::endl;
	}

	glDeleteShader(fragmentShaderObj);

	// �v���O�����̃����N
	glLinkProgram(program);

	// �����N�̃`�F�b�N
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
		std::cerr << "Link Error." << std::endl;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 1) {
		std::vector<GLchar> programLinkErrorMessage(infoLogLength);
		glGetProgramInfoLog(program, infoLogLength, nullptr,
			programLinkErrorMessage.data());
		std::cerr << programLinkErrorMessage.data() << std::endl;
	}

	struct Vertex {
		GLfloat position[3];
		GLfloat color[3];
	};

	// �O�p�`�̒��_���W�ƐF���̔z��
	const Vertex triangleVertices[] = {
		{0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f},
		{-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f},
		{0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f},
	};

	// VAO�̍쐬�ƃo�C���h
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// VBO���쐬
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vertex), triangleVertices,
		GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
		static_cast<Vertex*>(0)->position);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
		static_cast<Vertex*>(0)->color);

	while (glfwWindowShouldClose(window) == GL_FALSE) {
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program);

		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}
