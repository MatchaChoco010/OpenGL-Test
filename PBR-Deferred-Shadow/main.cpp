#define GLEW_STATIC
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <ext.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

GLuint createProgram(std::string vertexShaderFile, std::string fragmentShaderFile)
{
	// 頂点シェーダの読み込み
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

	// フラグメントシェーダの読み込み
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


	// プログラムオブジェクトを作成
	const GLuint program = glCreateProgram();

	GLint status = GL_FALSE;
	GLsizei infoLogLength;

	// 頂点シェーダのコンパイル
	const GLuint vertexShaderObj = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderObj, 1, &vertexShaderSourcePointer, nullptr);
	glCompileShader(vertexShaderObj);
	glAttachShader(program, vertexShaderObj);

	// 頂点シェーダのチェック
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

	// フラグメントシェーダのコンパイル
	const GLuint fragmentShaderObj = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderObj, 1, &fragmentShaderSourcePointer, nullptr);
	glCompileShader(fragmentShaderObj);
	glAttachShader(program, fragmentShaderObj);

	// 頂点シェーダのチェック
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

	// プログラムのリンク
	glLinkProgram(program);

	// リンクのチェック
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

bool loadOBJ(std::string path, std::vector<glm::vec3>& outVertices, std::vector<glm::vec2>& outUVs, std::vector<glm::vec3>& outNormals, std::vector<glm::vec3>& outTangents)
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

	for (int i = 0; i < outVertices.size(); i += 3)
	{
		auto& v0 = outVertices[i + 0];
		auto& v1 = outVertices[i + 1];
		auto& v2 = outVertices[i + 2];

		auto& uv0 = outUVs[i + 0];
		auto& uv1 = outUVs[i + 1];
		auto& uv2 = outUVs[i + 2];

		auto deltaPos1 = v1 - v0;
		auto deltaPos2 = v2 - v0;

		auto deltaUV1 = uv1 - uv0;
		auto deltaUV2 = uv2 - uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		auto tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;

		outTangents.emplace_back(tangent);
		outTangents.emplace_back(tangent);
		outTangents.emplace_back(tangent);
	}

	return true;
}

GLuint loadTexture(const char* path, const bool sRGB = false)
{
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (!data)
	{
		std::cerr << "Can't load image: " << path << std::endl;
		return 0;
	}
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	if (sRGB)
	{
		if (nrChannels == 4)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
	}
	else
	{
		if (nrChannels == 4)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
	}
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);

	return texture;
}


const float MIN_ISO = 100.0f;
const float MAX_ISO = 6400.0f;
const float MIN_APERTURE = 1.8;
const float MAX_APERTURE = 22.0;
const float MIN_SHUTTER = 1.0f / 4000.0f;
const float MAX_SHUTTER = 1.0f / 30.0f;

inline float Sqr(float x) { return x * x; }

float ComputeISO(float aperture, float shutterSpeed, float ev)
{
	return (Sqr(aperture) * 100.0f) / (shutterSpeed * powf(2.0f, ev));
}

float ComputeEV(float aperture, float shutterSpeed, float iso)
{
	return std::log2((Sqr(aperture) * 100.0f) / (shutterSpeed * iso));
}

float ComputeTargetEV(float averageLuminance)
{
	static const float K = 12.5f;
	return std::log2(averageLuminance * 100.0f / K);
}

void ApplyProgramAuto(float focalLength, float targetEV, float& aperture, float& shutterSpeed, float& iso)
{
	aperture = 4.0f;
	shutterSpeed = 1.0f / (focalLength * 1000.0f);

	iso = std::clamp(ComputeISO(aperture, shutterSpeed, targetEV), MIN_ISO, MAX_ISO);

	float evDiff = targetEV - ComputeEV(aperture, shutterSpeed, iso);
	aperture = std::clamp(aperture * powf(std::sqrt(2.0f), evDiff * 0.5f), MIN_APERTURE, MAX_APERTURE);

	evDiff = targetEV - ComputeEV(aperture, shutterSpeed, iso);
	shutterSpeed = std::clamp(shutterSpeed * powf(2.0f, -evDiff), MIN_SHUTTER, MAX_SHUTTER);
}


int main() {
	glfwSetErrorCallback([](auto id, auto description) { std::cerr << description << std::endl; });
	// GLFWの初期化
	if (!glfwInit())
		return 1;

	// 終了時処理の登録
	atexit(glfwTerminate);

	// OpenGL Version 4.6 Core Profileを選択する
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	const auto width = 640;
	const auto height = 480;

	// ウィンドウの作成
	GLFWwindow* const window =
		glfwCreateWindow(width, height, "PBR Test", nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "Can't create GLFW window." << std::endl;
		return 1;
	}

	glfwMakeContextCurrent(window);

	// GLEWの初期化
	if (glewInit() != GLEW_OK) {
		std::cerr << "Can't initialize GLEW." << std::endl;
		return 1;
	}

	// VSyncを待つ
	glfwSwapInterval(1);

	// Zテストを有効にする
	glEnable(GL_DEPTH_TEST);

	// Stencilテストを有効にする
	glEnable(GL_STENCIL_TEST);

	// 背面カリングを有効にする
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	// testMonkey.objのロード
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;
	if (!loadOBJ("testMonkey.obj", vertices, uvs, normals, tangents))
	{
		std::cerr << "Can't load obj file: testMonkey.obj" << std::endl;
		return 1;
	}
	// testmonkey.objのVAOの作成
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
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
	GLuint tangentsVBO;
	glGenBuffers(1, &tangentsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, tangentsVBO);
	glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(glm::vec3), &tangents[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0));
	glBindVertexArray(0);
	// testMonkey.objのテクスチャの読み込み
	const GLuint albedoMap = loadTexture("albedo.tga", true);
	const GLuint aoMap = loadTexture("ao.tga", true);
	const GLuint metallicMap = loadTexture("metallic.tga");
	const GLuint roughnessMap = loadTexture("roughness.tga");
	const GLuint normalMap = loadTexture("normal.tga");
	const GLuint emissiveMap = loadTexture("emissive.tga", true);

	// floor.objのロード
	std::vector<glm::vec3> floorVertices;
	std::vector<glm::vec2> floorUVs;
	std::vector<glm::vec3> floorNormals;
	std::vector<glm::vec3> floorTangents;
	if (!loadOBJ("floor.obj", floorVertices, floorUVs, floorNormals, floorTangents))
	{
		std::cerr << "Can't load obj file: floor.obj" << std::endl;
		return 1;
	}
	// floor.objのVAOの作成
	GLuint floorVao;
	glGenVertexArrays(1, &floorVao);
	glBindVertexArray(floorVao);
	GLuint floorVerticesVBO;
	glGenBuffers(1, &floorVerticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, floorVerticesVBO);
	glBufferData(GL_ARRAY_BUFFER, floorVertices.size() * sizeof(glm::vec3), &floorVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0));
	GLuint floorUVsVBO;
	glGenBuffers(1, &floorUVsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, floorUVsVBO);
	glBufferData(GL_ARRAY_BUFFER, floorUVs.size() * sizeof(glm::vec2), &floorUVs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0));
	GLuint floorNormalsVBO;
	glGenBuffers(1, &floorNormalsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, floorNormalsVBO);
	glBufferData(GL_ARRAY_BUFFER, floorNormals.size() * sizeof(glm::vec3), &floorNormals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0));
	GLuint floorTangentsVBO;
	glGenBuffers(1, &floorTangentsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, floorTangentsVBO);
	glBufferData(GL_ARRAY_BUFFER, floorTangents.size() * sizeof(glm::vec3), &floorTangents[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0));
	glBindVertexArray(0);
	// testMonkey.objのテクスチャの読み込み
	const GLuint floorAlbedoMap = loadTexture("floorAlbedo.tga", true);
	const GLuint floorAoMap = loadTexture("floorAo.tga", true);
	const GLuint floorMetallicMap = loadTexture("floorMetallic.tga");
	const GLuint floorRoughnessMap = loadTexture("floorRoughness.tga");
	const GLuint floorNormalMap = loadTexture("floorNormal.tga");
	const GLuint floorEmissiveMap = loadTexture("floorEmissive.tga", true);

	// fullscreen meshのVAO作成
	const std::array<glm::vec2, 3> fullscreenMeshVertices = {
		glm::vec2(-1.0, -1.0),
		glm::vec2(3.0, -1.0),
		glm::vec2(-1.0, 3.0),
	};
	const std::array<glm::vec2, 3> fullscreenMeshUVs = {
		glm::vec2(0.0, 0.0),
		glm::vec2(2.0, 0.0),
		glm::vec2(0.0, 2.0)
	};
	GLuint fullscreenMeshVAO;
	glGenVertexArrays(1, &fullscreenMeshVAO);
	glBindVertexArray(fullscreenMeshVAO);
	GLuint fullscreenMeshVerticesVBO;
	glGenBuffers(1, &fullscreenMeshVerticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, fullscreenMeshVerticesVBO);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(glm::vec2), &fullscreenMeshVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0));
	GLuint fullscreenMeshUVsVBO;
	glGenBuffers(1, &fullscreenMeshUVsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, fullscreenMeshUVsVBO);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(glm::vec2), &fullscreenMeshUVs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0));
	glBindVertexArray(0);

	// 半径位置の球のVAOを作成
	const int slices = 8, stacks = 8;
	std::vector<glm::vec3> sphereVertices;
	for (int j = 0; j <= stacks; ++j)
	{
		const float t(static_cast<float>(j) / static_cast<float>(stacks));
		const float y(cos(3.141593f * t)), r(sin(3.141593f * t));
		for (int i = 0; i <= slices; ++i)
		{
			const float s(static_cast<float>(i) / static_cast<float>(slices));
			const float z(r * cos(6.283185f * s)), x(r * sin(6.283185f * s));
			sphereVertices.emplace_back(x, y, z);
		}
	}
	std::vector<GLuint> sphereIndices;
	for (int j = 0; j < stacks; ++j)
	{
		const int k((slices + 1) * j);
		for (int i = 0; i < slices; ++i)
		{
			const GLuint k0(k + i);
			const GLuint k1(k0 + 1);
			const GLuint k2(k1 + slices);
			const GLuint k3(k2 + 1);

			sphereIndices.emplace_back(k0);
			sphereIndices.emplace_back(k2);
			sphereIndices.emplace_back(k3);

			sphereIndices.emplace_back(k0);
			sphereIndices.emplace_back(k3);
			sphereIndices.emplace_back(k1);
		}
	}
	GLuint sphereVAO;
	glGenVertexArrays(1, &sphereVAO);
	glBindVertexArray(sphereVAO);
	GLuint sphereVerticesVBO;
	glGenBuffers(1, &sphereVerticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVerticesVBO);
	glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(glm::vec3), &sphereVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0));
	GLuint sphereIndicesIBO;
	glGenBuffers(1, &sphereIndicesIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIndicesIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(GLuint), &sphereIndices[0], GL_STATIC_DRAW);
	glBindVertexArray(0);

	// shader programを取得しuniform変数の場所を取得する
	const GLuint geometryPassShaderProgram = createProgram("GeometryPass.vert", "GeometryPass.frag");
	const GLuint geometryPassModelITLoc = glGetUniformLocation(geometryPassShaderProgram, "ModelIT");
	const GLuint geometryPassModelViewLoc = glGetUniformLocation(geometryPassShaderProgram, "ModelView");
	const GLuint geometryPassProjectionLoc = glGetUniformLocation(geometryPassShaderProgram, "Projection");
	const GLuint geometryPassProjectionParamsLoc = glGetUniformLocation(geometryPassShaderProgram, "ProjectionParams");
	const GLuint geometryPassAlbedoMapLoc = glGetUniformLocation(geometryPassShaderProgram, "albedoMap");
	const GLuint geometryPassAoMapLoc = glGetUniformLocation(geometryPassShaderProgram, "aoMap");
	const GLuint geometryPassMetallicMapLoc = glGetUniformLocation(geometryPassShaderProgram, "metallicMap");
	const GLuint geometryPassRoughnessMapLoc = glGetUniformLocation(geometryPassShaderProgram, "roughnessMap");
	const GLuint geometryPassNormalMapLoc = glGetUniformLocation(geometryPassShaderProgram, "normalMap");
	const GLuint geometryPassEmissiveMapLoc = glGetUniformLocation(geometryPassShaderProgram, "emissiveMap");
	const GLuint geometryPassEmissiveIntensityLoc = glGetUniformLocation(geometryPassShaderProgram, "emissiveIntensity");

	const GLuint emissiveAndDirectionalLightPassShaderProgram = createProgram("EmissiveAndDirectionalLightPass.vert", "EmissiveAndDirectionalLightPass.frag");
	const GLuint emissiveAndDirectionalLightPassGBuffer0Loc = glGetUniformLocation(emissiveAndDirectionalLightPassShaderProgram, "GBuffer0");
	const GLuint emissiveAndDirectionalLightPassGBuffer1Loc = glGetUniformLocation(emissiveAndDirectionalLightPassShaderProgram, "GBuffer1");
	const GLuint emissiveAndDirectionalLightPassGBuffer2Loc = glGetUniformLocation(emissiveAndDirectionalLightPassShaderProgram, "GBuffer2");
	const GLuint emissiveAndDirectionalLightPassGBuffer3Loc = glGetUniformLocation(emissiveAndDirectionalLightPassShaderProgram, "GBuffer3");
	const GLuint emissiveAndDirectionalLightPassLightDirectionLoc = glGetUniformLocation(emissiveAndDirectionalLightPassShaderProgram, "LightDirection");
	const GLuint emissiveAndDirectionalLightPassLightIntensityLoc = glGetUniformLocation(emissiveAndDirectionalLightPassShaderProgram, "LightIntensity");
	const GLuint emissiveAndDirectionalLightPassLightColorLoc = glGetUniformLocation(emissiveAndDirectionalLightPassShaderProgram, "LightColor");
	const GLuint emissiveAndDirectionalLightPassWorldCameraPosLoc = glGetUniformLocation(emissiveAndDirectionalLightPassShaderProgram, "worldCameraPos");
	const GLuint emissiveAndDirectionalLightPassViewProjectionILoc = glGetUniformLocation(emissiveAndDirectionalLightPassShaderProgram, "ViewProjectionI");
	const GLuint emissiveAndDirectionalLightPassProjectionParamsLoc = glGetUniformLocation(emissiveAndDirectionalLightPassShaderProgram, "ProjectionParams");

	const GLuint punctualLightStencilPassShaderProgram = createProgram("PunctualLightStencilPass.vert", "PunctualLightStencilPass.frag");
	const GLuint punctualLightStencilPassModelViewProjectionLoc = glGetUniformLocation(punctualLightStencilPassShaderProgram, "ModelViewProjection");

	const GLuint pointLightPassShaderProgram = createProgram("PointLightPass.vert", "PointLightPass.frag");
	const GLuint pointLightPassModelViewProjectionLoc = glGetUniformLocation(pointLightPassShaderProgram, "ModelViewProjection");
	const GLuint pointLightPassGBuffer0Loc = glGetUniformLocation(pointLightPassShaderProgram, "GBuffer0");
	const GLuint pointLightPassGBuffer1Loc = glGetUniformLocation(pointLightPassShaderProgram, "GBuffer1");
	const GLuint pointLightPassGBuffer2Loc = glGetUniformLocation(pointLightPassShaderProgram, "GBuffer2");
	const GLuint pointLightPassGBuffer3Loc = glGetUniformLocation(pointLightPassShaderProgram, "GBuffer3");
	const GLuint pointLightPassWorldLightPosition = glGetUniformLocation(pointLightPassShaderProgram, "worldLightPosition");
	const GLuint pointLightPassLightIntensityLoc = glGetUniformLocation(pointLightPassShaderProgram, "LightIntensity");
	const GLuint pointLightPassLightColorLoc = glGetUniformLocation(pointLightPassShaderProgram, "LightColor");
	const GLuint pointLightPassLightRangeLoc = glGetUniformLocation(pointLightPassShaderProgram, "LightRange");
	const GLuint pointLightPassWorldCameraPosLoc = glGetUniformLocation(pointLightPassShaderProgram, "worldCameraPos");
	const GLuint pointLightPassViewProjectionILoc = glGetUniformLocation(pointLightPassShaderProgram, "ViewProjectionI");
	const GLuint pointLightPassProjectionParamsLoc = glGetUniformLocation(pointLightPassShaderProgram, "ProjectionParams");
	const GLuint pointLightPassResolutionLoc = glGetUniformLocation(pointLightPassShaderProgram, "resolution");

	const GLuint spotLightPassShaderProgram = createProgram("SpotLightPass.vert", "SpotLightPass.frag");
	const GLuint spotLightPassModelViewProjectionLoc = glGetUniformLocation(spotLightPassShaderProgram, "ModelViewProjection");
	const GLuint spotLightPassGBuffer0Loc = glGetUniformLocation(spotLightPassShaderProgram, "GBuffer0");
	const GLuint spotLightPassGBuffer1Loc = glGetUniformLocation(spotLightPassShaderProgram, "GBuffer1");
	const GLuint spotLightPassGBuffer2Loc = glGetUniformLocation(spotLightPassShaderProgram, "GBuffer2");
	const GLuint spotLightPassGBuffer3Loc = glGetUniformLocation(spotLightPassShaderProgram, "GBuffer3");
	const GLuint spotLightPassWorldLightPosition = glGetUniformLocation(spotLightPassShaderProgram, "worldLightPosition");
	const GLuint spotLightPassLightIntensityLoc = glGetUniformLocation(spotLightPassShaderProgram, "LightIntensity");
	const GLuint spotLightPassLightColorLoc = glGetUniformLocation(spotLightPassShaderProgram, "LightColor");
	const GLuint spotLightPassLightRangeLoc = glGetUniformLocation(spotLightPassShaderProgram, "LightRange");
	const GLuint spotLightPassLightDirectionLoc = glGetUniformLocation(spotLightPassShaderProgram, "LightDirection");
	const GLuint spotLightPassLightAngleLoc = glGetUniformLocation(spotLightPassShaderProgram, "LightAngle");
	const GLuint spotLightPassLightBlendLoc = glGetUniformLocation(spotLightPassShaderProgram, "LightBlend");
	const GLuint spotLightPassWorldCameraPosLoc = glGetUniformLocation(spotLightPassShaderProgram, "worldCameraPos");
	const GLuint spotLightPassViewProjectionILoc = glGetUniformLocation(spotLightPassShaderProgram, "ViewProjectionI");
	const GLuint spotLightPassProjectionParamsLoc = glGetUniformLocation(spotLightPassShaderProgram, "ProjectionParams");
	const GLuint spotLightPassResolutionLoc = glGetUniformLocation(spotLightPassShaderProgram, "resolution");

	const GLuint logAverageShaderProgram = createProgram("LogAveragePass.vert", "LogAveragePass.frag");
	const GLuint logAveragePassInputTextureLoc = glGetUniformLocation(logAverageShaderProgram, "inputTexture");

	const GLuint postprocessShaderProgram = createProgram("Postprocess.vert", "Postprocess.frag");
	const GLuint postprocessInputTextureLoc = glGetUniformLocation(postprocessShaderProgram, "inputTexture");
	const GLuint postprocessApertureLoc = glGetUniformLocation(postprocessShaderProgram, "aperture");
	const GLuint postprocessShutterSpeedLoc = glGetUniformLocation(postprocessShaderProgram, "shutterSpeed");
	const GLuint postprocessISOLoc = glGetUniformLocation(postprocessShaderProgram, "iso");

	// FBOを作成する
	GLuint GBuffer0ColorBuffer;
	glGenTextures(1, &GBuffer0ColorBuffer);
	glBindTexture(GL_TEXTURE_2D, GBuffer0ColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	GLuint GBuffer1ColorBuffer;
	glGenTextures(1, &GBuffer1ColorBuffer);
	glBindTexture(GL_TEXTURE_2D, GBuffer1ColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	GLuint GBuffer2ColorBuffer;
	glGenTextures(1, &GBuffer2ColorBuffer);
	glBindTexture(GL_TEXTURE_2D, GBuffer2ColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	GLuint GBuffer3ColorBuffer;
	glGenTextures(1, &GBuffer3ColorBuffer);
	glBindTexture(GL_TEXTURE_2D, GBuffer3ColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	GLuint GBufferDepthBuffer;
	glGenRenderbuffers(1, &GBufferDepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, GBufferDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	GLuint GBufferFBO;
	glGenFramebuffers(1, &GBufferFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, GBufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GBuffer0ColorBuffer, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, GBuffer1ColorBuffer, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, GBuffer2ColorBuffer, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, GBuffer3ColorBuffer, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, GBufferDepthBuffer);
	if (GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);  Status != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Framebuffer Error: " << Status << std::endl;
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//if (auto err = glGetError(); err != GL_NO_ERROR)
	//{
	//	std::cerr << err << std::endl;
	//}

	GLuint HDRColorBuffer;
	glGenTextures(1, &HDRColorBuffer);
	glBindTexture(GL_TEXTURE_2D, HDRColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	GLuint HDRDepthBuffer;
	glGenRenderbuffers(1, &HDRDepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, HDRDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	GLuint HDRFBO;
	glGenFramebuffers(1, &HDRFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, HDRFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, HDRColorBuffer, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, HDRDepthBuffer);
	if (GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER); Status != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Framebuffer Error: " << Status << std::endl;
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLuint LogAverageBuffer;
	glGenTextures(1, &LogAverageBuffer);
	glBindTexture(GL_TEXTURE_2D, LogAverageBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	GLuint LogAverageFBO;
	glGenFramebuffers(1, &LogAverageFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, LogAverageFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, LogAverageBuffer, 0);
	if (GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER); Status != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Framebuffer Error: " << Status << std::endl;
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glfwSetTime(0.0);

	float Lavg = 10.0f;
	float deltaTime = 0.0f;
	float prevTime = 0.0;

	while (glfwWindowShouldClose(window) == GL_FALSE) {
		deltaTime = static_cast<float>(glfwGetTime()) - prevTime;
		prevTime = static_cast<float>(glfwGetTime());


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		auto cameraPos = glm::vec3(0, 10, 10);
		auto near = 1.0f;
		auto far = 50.0f;
		auto ProjectionParams = glm::vec2(near, far);
		auto resolution = glm::vec2(width, height);

		auto View = glm::lookAt(cameraPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		auto Projection = glm::perspective(glm::radians(45.0f), 640.f / 480.f, near, far);
		auto ViewProjection = Projection * View;
		auto ViewProjectionI = glm::inverse(ViewProjection);

		auto DirectionalLightDirection = glm::vec3(0, -1, -0.5);
		//auto DirectionalLightIntensity = 100000.0f;
		auto DirectionalLightIntensity = 700.0f;
		auto DirectionalLightColor = glm::vec3(1.0, 1.0, 1.0);


		// Geometry Pass
		glEnable(GL_STENCIL_TEST);

		glStencilFunc(GL_ALWAYS, 128, 128);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(255);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		glUseProgram(geometryPassShaderProgram);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GBufferFBO);

		const GLenum bufs[] = {
			GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2,
			GL_COLOR_ATTACHMENT3
		};
		glDrawBuffers(4, bufs);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(1.0);
		glClearStencil(0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// testMonkey.objの描画
		auto Model = glm::rotate(glm::mat4(1), static_cast<float>(glfwGetTime()), glm::vec3(0, 1, 0));
		Model = glm::translate(Model, glm::vec3(0, 3, 0));
		auto ModelIT = glm::inverseTranspose(Model);
		auto ModelView = View * Model;

		auto emissiveIntensity = 2000.0f;

		glUniformMatrix4fv(geometryPassModelITLoc, 1, GL_FALSE, &ModelIT[0][0]);
		glUniformMatrix4fv(geometryPassModelViewLoc, 1, GL_FALSE, &ModelView[0][0]);
		glUniformMatrix4fv(geometryPassProjectionLoc, 1, GL_FALSE, &Projection[0][0]);
		glUniform2fv(geometryPassProjectionParamsLoc, 1, &ProjectionParams[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, albedoMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, aoMap);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, metallicMap);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, roughnessMap);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, normalMap);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, emissiveMap);

		glUniform1fv(geometryPassEmissiveIntensityLoc, 1, &emissiveIntensity);

		glUniform1i(geometryPassAlbedoMapLoc, 0);
		glUniform1i(geometryPassAoMapLoc, 1);
		glUniform1i(geometryPassMetallicMapLoc, 2);
		glUniform1i(geometryPassRoughnessMapLoc, 3);
		glUniform1i(geometryPassNormalMapLoc, 4);
		glUniform1i(geometryPassEmissiveMapLoc, 5);

		glDepthFunc(GL_LESS);

		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		auto Model1 = glm::translate(Model, glm::vec3(0, 1, 0));
		auto Model1IT = glm::inverseTranspose(Model1);
		auto ModelView1 = View * Model1;
		glUniformMatrix4fv(geometryPassModelITLoc, 1, GL_FALSE, &Model1IT[0][0]);
		glUniformMatrix4fv(geometryPassModelViewLoc, 1, GL_FALSE, &ModelView1[0][0]);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		// floor.objの描画
		auto ModelFloor = glm::mat4(1);
		auto ModelFloorIT = glm::inverseTranspose(ModelFloor);
		auto ModelViewFloor = View * ModelFloor;

		auto emissiveFloorIntensity = 0.0f;

		glUniformMatrix4fv(geometryPassModelITLoc, 1, GL_FALSE, &ModelFloorIT[0][0]);
		glUniformMatrix4fv(geometryPassModelViewLoc, 1, GL_FALSE, &ModelViewFloor[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floorAlbedoMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, floorAoMap);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, floorMetallicMap);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, floorRoughnessMap);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, floorNormalMap);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, floorEmissiveMap);

		glUniform1fv(geometryPassEmissiveIntensityLoc, 1, &emissiveFloorIntensity);

		glBindVertexArray(floorVao);
		glDrawArrays(GL_TRIANGLES, 0, floorVertices.size());


		// Emissive and DirectionalLight Pass
		glBindFramebuffer(GL_READ_FRAMEBUFFER, GBufferDepthBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, HDRDepthBuffer);
		glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);

		glStencilFunc(GL_EQUAL, 128, 128);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glStencilMask(0);

		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		glUseProgram(emissiveAndDirectionalLightPassShaderProgram);
		glUniform3fv(emissiveAndDirectionalLightPassLightDirectionLoc, 1, &DirectionalLightDirection[0]);
		glUniform1fv(emissiveAndDirectionalLightPassLightIntensityLoc, 1, &DirectionalLightIntensity);
		glUniform3fv(emissiveAndDirectionalLightPassLightColorLoc, 1, &DirectionalLightColor[0]);
		glUniform3fv(emissiveAndDirectionalLightPassWorldCameraPosLoc, 1, &cameraPos[0]);
		glUniformMatrix4fv(emissiveAndDirectionalLightPassViewProjectionILoc, 1, GL_FALSE, &ViewProjectionI[0][0]);
		glUniform2fv(emissiveAndDirectionalLightPassProjectionParamsLoc, 1, &ProjectionParams[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, GBuffer0ColorBuffer);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, GBuffer1ColorBuffer);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, GBuffer2ColorBuffer);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, GBuffer3ColorBuffer);

		glUniform1i(emissiveAndDirectionalLightPassGBuffer0Loc, 0);
		glUniform1i(emissiveAndDirectionalLightPassGBuffer1Loc, 1);
		glUniform1i(emissiveAndDirectionalLightPassGBuffer2Loc, 2);
		glUniform1i(emissiveAndDirectionalLightPassGBuffer3Loc, 3);

		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBindFramebuffer(GL_FRAMEBUFFER, HDRFBO);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		//glClearColor(1.0f, 0.3f, 0.3f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindVertexArray(fullscreenMeshVAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);


		// Point Light Pass
		glUseProgram(pointLightPassShaderProgram);

		glUniform3fv(pointLightPassWorldCameraPosLoc, 1, &cameraPos[0]);
		glUniformMatrix4fv(pointLightPassViewProjectionILoc, 1, GL_FALSE, &ViewProjectionI[0][0]);
		glUniform2fv(pointLightPassProjectionParamsLoc, 1, &ProjectionParams[0]);
		glUniform2fv(pointLightPassResolutionLoc, 1, &resolution[0]);

		{
			/*auto pointLightPosition = glm::vec3(-0.0f, 8.0f, 0.0f);
			auto pointLightIntensity = 24000.0f;
			auto pointLightColor = glm::vec3(0.5, 1.0, 1.0);
			auto pointLightRange = 20.0f;

			auto PointLightModel = glm::translate(glm::mat4(1.0), pointLightPosition);
			PointLightModel = glm::scale(PointLightModel, glm::vec3(pointLightRange + 0.1));
			auto PointLightModelViewProjection = Projection * View * PointLightModel;

			glUseProgram(punctualLightStencilPassShaderProgram);

			glUniformMatrix4fv(punctualLightStencilPassModelViewProjectionLoc, 1, GL_FALSE, &PointLightModelViewProjection[0][0]);

			glEnable(GL_DEPTH_TEST);

			glDisable(GL_CULL_FACE);

			glStencilMask(255);
			glClear(GL_STENCIL_BUFFER_BIT);

			glStencilFunc(GL_ALWAYS, 0, 0);
			glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
			glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

			glDrawBuffer(GL_NONE);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

			glBindVertexArray(sphereVAO);
			glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

			glUseProgram(pointLightPassShaderProgram);

			glUniform3fv(pointLightPassWorldCameraPosLoc, 1, &cameraPos[0]);
			glUniformMatrix4fv(pointLightPassViewProjectionILoc, 1, GL_FALSE, &ViewProjectionI[0][0]);
			glUniform2fv(pointLightPassProjectionParamsLoc, 1, &ProjectionParams[0]);
			glUniform2fv(pointLightPassResolutionLoc, 1, &resolution[0]);

			glUniform3fv(pointLightPassWorldLightPosition, 1, &pointLightPosition[0]);
			glUniform1fv(pointLightPassLightIntensityLoc, 1, &pointLightIntensity);
			glUniform3fv(pointLightPassLightColorLoc, 1, &pointLightColor[0]);
			glUniform1fv(pointLightPassLightRangeLoc, 1, &pointLightRange);

			glUniformMatrix4fv(pointLightPassModelViewProjectionLoc, 1, GL_FALSE, &PointLightModelViewProjection[0][0]);

			glUniform1i(pointLightPassGBuffer0Loc, 0);
			glUniform1i(pointLightPassGBuffer1Loc, 1);
			glUniform1i(pointLightPassGBuffer2Loc, 2);
			glUniform1i(pointLightPassGBuffer3Loc, 3);

			glDisable(GL_DEPTH_TEST);

			glStencilFunc(GL_NOTEQUAL, 0, 255);
			glStencilMask(0);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);

			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

			glBindVertexArray(sphereVAO);
			glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

			glCullFace(GL_BACK);*/
		}


		// Spot Light Pass
		glUseProgram(spotLightPassShaderProgram);

		glUniform3fv(spotLightPassWorldCameraPosLoc, 1, &cameraPos[0]);
		glUniformMatrix4fv(spotLightPassViewProjectionILoc, 1, GL_FALSE, &ViewProjectionI[0][0]);
		glUniform2fv(spotLightPassProjectionParamsLoc, 1, &ProjectionParams[0]);
		glUniform2fv(spotLightPassResolutionLoc, 1, &resolution[0]);

		{
			/*auto spotLightPosition = glm::vec3(4.0f, 8.0f, 4.0f);
			auto spotLightIntensity = 6000.0f;
			auto spotLightColor = glm::vec3(1.0, 0.5, 0.5);
			auto spotLightRange = 20.0f;
			auto spotLightDirection = glm::vec3(-1.0, -1.0, -1.0);
			auto spotLightAngle = glm::radians(45.0f);
			auto spotLightBlend = 0.15f;

			auto SpotLightModel = glm::translate(glm::mat4(1.0), spotLightPosition);
			SpotLightModel = glm::scale(SpotLightModel, glm::vec3(spotLightRange + 0.1));
			auto SpotLightModelViewProjection = Projection * View * SpotLightModel;

			glUseProgram(punctualLightStencilPassShaderProgram);

			glUniformMatrix4fv(punctualLightStencilPassModelViewProjectionLoc, 1, GL_FALSE, &SpotLightModelViewProjection[0][0]);

			glEnable(GL_DEPTH_TEST);

			glDisable(GL_CULL_FACE);

			glStencilMask(255);
			glClear(GL_STENCIL_BUFFER_BIT);

			glStencilFunc(GL_ALWAYS, 0, 0);
			glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
			glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

			glDrawBuffer(GL_NONE);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

			glBindVertexArray(sphereVAO);
			glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

			glUseProgram(spotLightPassShaderProgram);

			glUniform3fv(spotLightPassWorldCameraPosLoc, 1, &cameraPos[0]);
			glUniformMatrix4fv(spotLightPassViewProjectionILoc, 1, GL_FALSE, &ViewProjectionI[0][0]);
			glUniform2fv(spotLightPassProjectionParamsLoc, 1, &ProjectionParams[0]);
			glUniform2fv(spotLightPassResolutionLoc, 1, &resolution[0]);

			glUniform3fv(spotLightPassWorldLightPosition, 1, &spotLightPosition[0]);
			glUniform1fv(spotLightPassLightIntensityLoc, 1, &spotLightIntensity);
			glUniform3fv(spotLightPassLightColorLoc, 1, &spotLightColor[0]);
			glUniform1fv(spotLightPassLightRangeLoc, 1, &spotLightRange);
			glUniform3fv(spotLightPassLightDirectionLoc, 1, &spotLightDirection[0]);
			glUniform1fv(spotLightPassLightAngleLoc, 1, &spotLightAngle);
			glUniform1fv(spotLightPassLightBlendLoc, 1, &spotLightBlend);

			glUniformMatrix4fv(spotLightPassModelViewProjectionLoc, 1, GL_FALSE, &SpotLightModelViewProjection[0][0]);

			glUniform1i(spotLightPassGBuffer0Loc, 0);
			glUniform1i(spotLightPassGBuffer1Loc, 1);
			glUniform1i(spotLightPassGBuffer2Loc, 2);
			glUniform1i(spotLightPassGBuffer3Loc, 3);

			glDisable(GL_DEPTH_TEST);

			glStencilFunc(GL_NOTEQUAL, 0, 255);
			glStencilMask(0);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);

			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

			glBindVertexArray(sphereVAO);
			glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

			glCullFace(GL_BACK);*/
		}


		// Calc Log Average
		glDisable(GL_STENCIL_TEST);
		glUseProgram(logAverageShaderProgram);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, HDRColorBuffer);

		glUniform1i(logAveragePassInputTextureLoc, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, LogAverageFBO);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindVertexArray(fullscreenMeshVAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindTexture(GL_TEXTURE_2D, LogAverageBuffer);
		glGenerateMipmap(GL_TEXTURE_2D);

		const int level = static_cast<int>(std::log2(std::max(width, height)));
		float pixel[3];
		glGetTexImage(GL_TEXTURE_2D, level, GL_RGB, GL_FLOAT, &pixel);
		float Lnew = std::expf(pixel[0]);
		Lavg = Lavg + (Lnew - Lavg) * (1 - std::expf(-1 * deltaTime * 1.0));

		const auto targetEV = ComputeTargetEV(Lavg);
		const auto EVcomp = -2.0f;

		float aperture, shutterSpeed, iso;
		ApplyProgramAuto(50, targetEV - EVcomp, aperture, shutterSpeed, iso);

		glBindTexture(GL_TEXTURE_2D, 0);


		// Postprocess
		glDisable(GL_STENCIL_TEST);

		glUseProgram(postprocessShaderProgram);
		glUniform1fv(postprocessApertureLoc, 1, &aperture);
		glUniform1fv(postprocessShutterSpeedLoc, 1, &shutterSpeed);
		glUniform1fv(postprocessISOLoc, 1, &iso);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, HDRColorBuffer);

		glUniform1i(postprocessInputTextureLoc, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindVertexArray(fullscreenMeshVAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);


		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &verticesVBO);
	glDeleteBuffers(1, &uvsVBO);
	glDeleteBuffers(1, &normalsVBO);
	glDeleteVertexArrays(1, &fullscreenMeshVAO);
	glDeleteBuffers(1, &fullscreenMeshVerticesVBO);
	glDeleteBuffers(1, &fullscreenMeshUVsVBO);
	glDeleteProgram(geometryPassShaderProgram);
	glDeleteProgram(emissiveAndDirectionalLightPassShaderProgram);
	glDeleteProgram(postprocessShaderProgram);
	glDeleteTextures(1, &albedoMap);
	glDeleteTextures(1, &metallicMap);
	glDeleteTextures(1, &roughnessMap);
	glDeleteTextures(1, &normalMap);
	glDeleteTextures(1, &emissiveMap);
	glDeleteTextures(1, &GBuffer0ColorBuffer);
	glDeleteFramebuffers(1, &GBufferFBO);
	glDeleteTextures(1, &GBuffer1ColorBuffer);
	glDeleteTextures(1, &GBuffer2ColorBuffer);
	glDeleteTextures(1, &GBuffer3ColorBuffer);
	glDeleteTextures(1, &HDRColorBuffer);
	glDeleteFramebuffers(1, &HDRFBO);
	glDeleteTextures(1, &LogAverageBuffer);
	glDeleteFramebuffers(1, &LogAverageFBO);
}
