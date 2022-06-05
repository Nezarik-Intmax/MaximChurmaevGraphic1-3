﻿#include <iostream>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "GL/glew.h";
#include "GL/glut.h";
#include "glm/glm.hpp";
#include "GL/freeglut.h";
#include <Magick++.h>;
#include <string>;

GLuint VBO;
GLuint IBO;


GLuint gWVPLocation;
class Texture
{
public:
	Texture(GLenum TextureTarget, const std::string& FileName)
	{
		m_textureTarget = TextureTarget;
		m_fileName = FileName;
		m_pImage = NULL;
	}

	bool Load()
	{
		try {
			m_pImage = new Magick::Image(m_fileName);
			m_pImage->write(&m_blob, "RGBA");
		}
		catch (Magick::Error& Error) {
			std::cout << "Error loading texture '" << m_fileName << "': " << Error.what() << std::endl;
			return false;
		}

		glGenTextures(1, &m_textureObj);
		glBindTexture(m_textureTarget, m_textureObj);
		glTexImage2D(m_textureTarget, 0, GL_RGB, m_pImage->columns(), m_pImage->rows(), -0.5, GL_RGBA, GL_UNSIGNED_BYTE, m_blob.data());
		glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		return true;
	}

	void Bind(GLenum TextureUnit)
	{
		glActiveTexture(TextureUnit);
		glBindTexture(m_textureTarget, m_textureObj);
	}
private:
	std::string m_fileName;
	GLenum m_textureTarget;
	GLuint m_textureObj;
	Magick::Image* m_pImage;
	Magick::Blob m_blob;
};
Texture* pTexture = NULL;
GLuint gSampler;



struct Vertex
{
	glm::fvec3 m_pos;
	glm::fvec2 m_tex;

	Vertex() {}

	Vertex(glm::fvec3 pos, glm::fvec2 tex)
	{
		m_pos = pos;
		m_tex = tex;
	}
};


/*static const char* pVS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
layout (location = 0) in vec3 Position;                                             \n\
                                                                                    \n\
uniform mat4 gWorld;                                                                \n\
                                                                                    \n\
out vec4 Color;                                                                     \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    gl_Position = gWorld * vec4(Position, 1.0);                                     \n\
    Color = vec4(1.0, 1.0, 1.0, 1.0);												\n\
}";*/

static const char* pVS = "                                                          \n\
#version 330                                                                        \n\
layout(location = 0) in vec3 Position;												\n\
layout(location = 1) in vec2 TexCoord;												\n\
																					\n\
uniform mat4 gWVP;																	\n\
																					\n\
out vec2 TexCoord0;																	\n\
																					\n\
void main()																			\n\
{																					\n\
	gl_Position = gWVP * vec4(Position, 1.0);										\n\
	TexCoord0 = TexCoord;															\n\
};";
static const char* pFS = "                                                          \n\
#version 330                                                                        \n\
																					\n\
in vec2 TexCoord0;																	\n\
																					\n\
out vec4 FragColor;																	\n\
																					\n\
uniform sampler2D gSampler;															\n\
																					\n\
void main()																			\n\
{																					\n\
	FragColor = texture2D(gSampler, TexCoord0.st);									\n\
};";

/*static const char* pFS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
in vec4 Color;                                                                      \n\
                                                                                    \n\
out vec4 FragColor;                                                                 \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    FragColor = Color;                                                              \n\
}";*/

float Scale = 0.0f;
#define M_PI 3.14159265358979323846
#define ToRadian(x) ((x) * M_PI / 180.0f)
#define ToDegree(x) ((x) * 180.0f / M_PI)
void InitPers(glm::fmat4& m, float zNear, float zFar, float width, float height, float fov){
	const float ar = width / height;
	const float zRange = zNear - zFar;
	const float tanHalfFOV = tanf(ToRadian(fov / 2.0));

	m[0][0] = 1.0f / (tanHalfFOV * ar);
	m[0][1] = 0.0f;
	m[0][2] = 0.0f;
	m[0][3] = 0.0f;

	m[1][0] = 0.0f;
	m[1][1] = 1.0f / tanHalfFOV;
	m[1][2] = 0.0f;
	m[1][3] = 0.0f;

	m[2][0] = 0.0f;
	m[2][1] = 0.0f;
	m[2][2] = (-zNear - zFar) / zRange;
	m[2][3] = 2.0f * zFar * zNear / zRange;

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 1.0f;
	m[3][3] = 0.0f;

}
void RenderSceneCB(){
	glClear(GL_COLOR_BUFFER_BIT);
	Vertex Vertices[4] = {
		Vertex(glm::fvec3(-1.0f, -1.0f, 0.5773f), glm::fvec2(0.0f, 0.0f)),
		Vertex(glm::fvec3(0.0f, -1.0f, -1.15475), glm::fvec2(0.5f, 0.0f)),
		Vertex(glm::fvec3(1.0f, -1.0f, 0.5773f),  glm::fvec2(1.0f, 0.0f)),
		Vertex(glm::fvec3(0.0f, 1.0f, 0.0f),      glm::fvec2(0.5f, 1.0f))
	};
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	unsigned int Indices[] = {0, 3, 1,
							  1, 3, 2,
							  2, 3, 0,
							  1, 2, 0};

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

	Scale += 0.001f;
	glm::fmat4 WorldPos;
	WorldPos[0][0] = 1.0f; WorldPos[0][1] = 0.0f; WorldPos[0][2] = 0.0f; WorldPos[0][3] = 0.0f;//sinf(ToRadian(Scale));
	WorldPos[1][0] = 0.0f; WorldPos[1][1] = 1.0f; WorldPos[1][2] = 0.0f; WorldPos[1][3] = 0.0f;
	WorldPos[2][0] = 0.0f; WorldPos[2][1] = 0.0f; WorldPos[2][2] = 1.0f; WorldPos[2][3] = 5.0f;
	WorldPos[3][0] = 0.0f; WorldPos[3][1] = 0.0f; WorldPos[3][2] = 0.0f; WorldPos[3][3] = 1.0f;
	glm::fmat4 WorldRot;
	WorldRot[0][0] = cosf(ToRadian(Scale));		   WorldRot[0][1] = 0.0f;		  WorldRot[0][2] = -sinf(ToRadian(Scale));  WorldRot[0][3] = 0.0f;
	WorldRot[1][0] = 0.0f;						   WorldRot[1][1] = 1.0f;		  WorldRot[1][2] = 0.0f;					WorldRot[1][3] = 0.0f;
	WorldRot[2][0] = sinf(ToRadian(Scale));        WorldRot[2][1] = 0.0f;         WorldRot[2][2] = cosf(ToRadian(Scale));   WorldRot[2][3] = 0.0f;
	WorldRot[3][0] = 0.0f;						   WorldRot[3][1] = 0.0f;         WorldRot[3][2] = 0.0f;					WorldRot[3][3] = 1.0f;
	glm::fmat4 WorldScl;
	WorldScl[0][0] = 1.0f;		  WorldScl[0][1] = 0.0f;        WorldScl[0][2] = 0.0f;        WorldScl[0][3] = 0.0f;
	WorldScl[1][0] = 0.0f;        WorldScl[1][1] = 1.0f;		WorldScl[1][2] = 0.0f;        WorldScl[1][3] = 0.0f;
	WorldScl[2][0] = 0.0f;        WorldScl[2][1] = 0.0f;        WorldScl[2][2] = 1.0f;		  WorldScl[2][3] = 0.0f;
	WorldScl[3][0] = 0.0f;        WorldScl[3][1] = 0.0f;        WorldScl[3][2] = 0.0f;        WorldScl[3][3] = 1.0f;
	glm::fmat4 WorldPers;
	InitPers(WorldPers, 1.0f, 100.0f, 1024, 768, 30);
	glm::fmat4* m_transformation = new glm::fmat4(glm::transpose(glm::transpose(WorldPers) * glm::transpose(WorldPos) * glm::transpose(WorldRot) * glm::transpose(WorldScl)));

	glUniformMatrix4fv(gWVPLocation, 1, GL_TRUE, (const GLfloat*)m_transformation);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
	pTexture->Bind(GL_TEXTURE0);

	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glutSwapBuffers();
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType){
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}

	const GLchar* p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = strlen(pShaderText);
	glShaderSource(ShaderObj, 1, p, Lengths);
	glCompileShader(ShaderObj);
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}

	glAttachShader(ShaderProgram, ShaderObj);
}

static void CompileShaders(){
	GLuint ShaderProgram = glCreateProgram();

	if (ShaderProgram == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	AddShader(ShaderProgram, pVS, GL_VERTEX_SHADER);
	AddShader(ShaderProgram, pFS, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glUseProgram(ShaderProgram);

	gWVPLocation = glGetUniformLocation(ShaderProgram, "gWVP");
	assert(gWVPLocation != 0xFFFFFFFF);
	gSampler = glGetUniformLocation(ShaderProgram, "gSampler");
	assert(gSampler != 0xFFFFFFFF);
}
int main(int argc, char** argv){
	Magick::InitializeMagick(*argv);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(1024, 768);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Tutorial 16");
	glutDisplayFunc(RenderSceneCB);
	glutIdleFunc(RenderSceneCB);

	GLenum res = glewInit();
	if(res != GLEW_OK){
		fprintf(stderr, "Error: '%s' \n", glewGetErrorString(res));
		return 1;
	}
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	CompileShaders();
	glUniform1i(gSampler, 0);

	pTexture = new Texture(GL_TEXTURE_2D, "C:\\test.jpg");

	if (!pTexture->Load()) {
		return 1;
	}
	glutMainLoop();
}