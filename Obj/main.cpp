#define GLEW_STATIC
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <ext.hpp>

GLuint createProgram(std::string vertexShaderFile, std::string fragmentShaderFile)
{
	// ���_�V�F�[�_�̓ǂݍ���
	std::ifstream vertexIfs(vertexShaderFile, std::ios::binary);
	if (vertexIfs.fail())
	{
		std::cerr << "Error: Can't open source file: " << vertexShaderFile << std::endl;
		return 0;
	}
	auto vertexShaderSource = std::string(std::istreambuf_iterator<char>(vertexIfs), std::istreambuf_iterator<char>());
	if (vertexIfs.fail())
	{
		std::cerr << "Error: could not read source file: " << vertexShaderFile << std::endl;
		return 0;
	}
	GLchar const* vertexShaderSourcePointer = vertexShaderSource.c_str();

	// �t���O�����g�V�F�[�_�̓ǂݍ���
	std::ifstream fragmentIfs(fragmentShaderFile, std::ios::binary);
	if (fragmentIfs.fail())
	{
		std::cerr << "Error: Can't open source file: " << fragmentShaderFile << std::endl;
		return 0;
	}
	auto fragmentShaderSource = std::string(std::istreambuf_iterator<char>(fragmentIfs), std::istreambuf_iterator<char>());
	if (fragmentIfs.fail())
	{
		std::cerr << "Error: could not read source file: " << fragmentShaderFile << std::endl;
		return 0;
	}
	GLchar const* fragmentShaderSourcePointer = fragmentShaderSource.c_str();


	// �v���O�����I�u�W�F�N�g���쐬
	const GLuint program = glCreateProgram();

	GLint status = GL_FALSE;
	GLsizei infoLogLength;

	// ���_�V�F�[�_�̃R���p�C��
	const GLuint vertexShaderObj = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderObj, 1, &vertexShaderSourcePointer, nullptr);
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
	glShaderSource(fragmentShaderObj, 1, &fragmentShaderSourcePointer, nullptr);
	glCompileShader(fragmentShaderObj);
	glAttachShader(program, fragmentShaderObj);

	// ���_�V�F�[�_�̃`�F�b�N
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

	return program;
}

std::vector<std::string> splitString(const std::string& s, char delim)
{
	std::vector<std::string> elems(0);
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

bool loadOBJ(std::string path, std::vector<glm::vec3>& outVertices, std::vector<glm::vec2>& outUVs, std::vector<glm::vec3>& outNormals)
{
	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> tmpVertices;
	std::vector<glm::vec2> tmpUVs;
	std::vector<glm::vec3> tmpNormals;

	std::ifstream ifs(path);
	std::string line;
	if (ifs.fail())
	{
		std::cerr << "Can't open obj file: " << path << std::endl;
		return false;
	}
	while (getline(ifs, line))
	{
		auto col = splitString(line, ' ');

		if (col[0] == "v")
		{
			tmpVertices.emplace_back(std::stof(col[1]), std::stof(col[2]), std::stof(col[3]));
		}
		else if (col[0] == "vt")
		{
			tmpUVs.emplace_back(std::stof(col[1]), std::stof(col[2]));
		}
		else if (col[0] == "vn")
		{
			tmpNormals.emplace_back(std::stof(col[1]), std::stof(col[2]), std::stof(col[3]));
		}
		else if (col[0] == "f")
		{
			auto v1 = splitString(col[1], '/');
			auto v2 = splitString(col[2], '/');
			auto v3 = splitString(col[3], '/');
			vertexIndices.emplace_back(std::stoi(v1[0]));
			vertexIndices.emplace_back(std::stoi(v2[0]));
			vertexIndices.emplace_back(std::stoi(v3[0]));
			uvIndices.emplace_back(std::stoi(v1[1]));
			uvIndices.emplace_back(std::stoi(v2[1]));
			uvIndices.emplace_back(std::stoi(v3[1]));
			normalIndices.emplace_back(std::stoi(v1[2]));
			normalIndices.emplace_back(std::stoi(v2[2]));
			normalIndices.emplace_back(std::stoi(v3[2]));
		}
	}

	for (unsigned int i = 0; i < vertexIndices.size(); i++)
	{
		unsigned int vertexIndex = vertexIndices[i];
		outVertices.emplace_back(tmpVertices[vertexIndex - 1]);
	}
	for (unsigned int i = 0; i < uvIndices.size(); i++)
	{
		unsigned int uvIndex = uvIndices[i];
		outUVs.emplace_back(tmpUVs[uvIndex - 1]);
	}
	for (unsigned int i = 0; i < normalIndices.size(); i++)
	{
		unsigned int normalIndex = normalIndices[i];
		outNormals.emplace_back(tmpNormals[normalIndex - 1]);
	}
}

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

	// Z�e�X�g��L���ɂ���
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	// �w�ʃJ�����O��L���ɂ���
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	const GLuint program = createProgram("shader.vert", "shader.frag");

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	if (!loadOBJ("testMonkey.obj", vertices, uvs, normals))
	{
		std::cerr << "Can't load obj file: test.obj" << std::endl;
		return 1;
	}
		
	// VAO�̍쐬�ƃo�C���h
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// VBO���쐬
	GLuint verticesVBO;
	glGenBuffers(1, &verticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, verticesVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0));

	GLuint uvsVBO;
	glGenBuffers(1, &uvsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, uvsVBO);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0));

	GLuint normalsVBO;
	glGenBuffers(1, &normalsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0));

	// uniform�ϐ��̏ꏊ���擾����
	const GLint ViewLoc = glGetUniformLocation(program, "View");
	const GLint ModelViewLoc = glGetUniformLocation(program, "ModelView");
	const GLint ModelViewITLoc = glGetUniformLocation(program, "ModelViewIT");
	const GLint ProjectionLoc = glGetUniformLocation(program, "Projection");

	glfwSetTime(0.0);

	while (glfwWindowShouldClose(window) == GL_FALSE) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program);

		glm::mat4 Model = glm::rotate(glm::mat4(1), static_cast<float>(glfwGetTime()), glm::vec3(0, 1, 0));
		glm::mat4 View = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glm::mat4 ModelView = View * Model;
		glm::mat4 ModelViewIT = glm::inverseTranspose(ModelView);
		glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 640.f / 480.f, 1.f, 10.f);

		glUniformMatrix4fv(ViewLoc, 1, GL_FALSE, &View[0][0]);
		glUniformMatrix4fv(ModelViewLoc, 1, GL_FALSE, &ModelView[0][0]);
		glUniformMatrix4fv(ModelViewITLoc, 1, GL_FALSE, &ModelViewIT[0][0]);
		glUniformMatrix4fv(ProjectionLoc, 1, GL_FALSE, &Projection[0][0]);

		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &verticesVBO);
	glDeleteBuffers(1, &uvsVBO);
	glDeleteBuffers(1, &normalsVBO);
}
