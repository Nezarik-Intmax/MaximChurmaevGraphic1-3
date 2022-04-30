#include <iostream>
#include "GL/glew.h";
#include "GL/glut.h";
#include "glm/glm.hpp";
#include "GL/freeglut.h"

GLuint VBO;
GLuint IBO;
GLuint gWorldLocation;
glm::fmat4* m_transformation = new glm::fmat4();
#define M_PI 3.14159265358979323846
#define ToRadian(x) ((x) * M_PI / 180.0f)
#define ToDegree(x) ((x) * 180.0f / M_PI)

struct{
	glm::fvec3 Pos;
	glm::fvec3 Target;
	glm::fvec3 Up;
} m_camera;
void SetCamera(const glm::fvec3& Pos, const glm::fvec3& Target, const glm::fvec3& Up){
	m_camera.Pos = Pos;
	m_camera.Target = Target;
	m_camera.Up = Up;
}
void CameraTransform(const glm::fvec3& Target, const glm::fvec3& Up, glm::fmat4& m){
	glm::fvec3 N = Target;
	N = glm::normalize(N);
	//N.Normalize();
	glm::fvec3 U = Up;
	U = glm::normalize(U);
	//U.Normalize();
	U = cross(U, Target);
	glm::fvec3 V = cross(N, U);

	m[0][0] = U.x; m[0][1] = U.y; m[0][2] = U.z; m[0][3] = 0.0f;
	m[1][0] = V.x; m[1][1] = V.y; m[1][2] = V.z; m[1][3] = 0.0f;
	m[2][0] = N.x; m[2][1] = N.y; m[2][2] = N.z; m[2][3] = 0.0f;
	m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
}


static const char* pVS = "                                                          \n\
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
    Color = vec4(1.0, 1.0, 1.0, 1.0);                                   \n\
}";

static const char* pFS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
in vec4 Color;                                                                      \n\
                                                                                    \n\
out vec4 FragColor;                                                                 \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    FragColor = Color;                                                              \n\
}";

void Pers(glm::fmat4& m, float zNear, float zFar, float width, float height, float fov){
	const float ar = width / height;
	const float zRange = zNear - zFar;
	const float tanHalfFOV = tanf(ToRadian(fov / 2.0));

	m[0][0] = 1.0f / (tanHalfFOV * ar);	m[1][0] = 0.0f;				 m[2][0] = 0.0f;							m[3][0] = 0.0f;
	m[0][1] = 0.0f;						m[1][1] = 1.0f / tanHalfFOV; m[2][1] = 0.0f;							m[3][1] = 0.0f;
	m[0][2] = 0.0f;						m[1][2] = 0.0f;				 m[2][2] = (-zNear - zFar) / zRange;		m[3][2] = 1.0f;
	m[0][3] = 0.0f;						m[1][3] = 0.0f;				 m[2][3] = 2.0f * zFar * zNear / zRange;	m[3][3] = 0.0f;
	m = glm::transpose(m);
}
void Translate(glm::fmat4& WorldPos, GLfloat x, GLfloat y, GLfloat z){
	WorldPos[0][0] = 1.0f; WorldPos[0][1] = 0.0f; WorldPos[0][2] = 0.0f; WorldPos[0][3] = x;
	WorldPos[1][0] = 0.0f; WorldPos[1][1] = 1.0f; WorldPos[1][2] = 0.0f; WorldPos[1][3] = y;
	WorldPos[2][0] = 0.0f; WorldPos[2][1] = 0.0f; WorldPos[2][2] = 1.0f; WorldPos[2][3] = z;
	WorldPos[3][0] = 0.0f; WorldPos[3][1] = 0.0f; WorldPos[3][2] = 0.0f; WorldPos[3][3] = 1.0f;
	WorldPos = glm::transpose(WorldPos);
}
void RotateY(glm::fmat4& WorldRot, GLfloat y){
	WorldRot[0][0] = cosf(ToRadian(y));	WorldRot[0][1] = 0.0f;	WorldRot[0][2] = -sinf(ToRadian(y));	WorldRot[0][3] = 0.0f;
	WorldRot[1][0] = 0.0f;				WorldRot[1][1] = 1.0f;	WorldRot[1][2] = 0.0f;					WorldRot[1][3] = 0.0f;
	WorldRot[2][0] = sinf(ToRadian(y)); WorldRot[2][1] = 0.0f;  WorldRot[2][2] = cosf(ToRadian(y));		WorldRot[2][3] = 0.0f;
	WorldRot[3][0] = 0.0f;				WorldRot[3][1] = 0.0f;  WorldRot[3][2] = 0.0f;					WorldRot[3][3] = 1.0f;
	WorldRot = glm::transpose(WorldRot);
}
void Rotate(glm::fmat4& WorldRot, GLfloat _x, GLfloat _y, GLfloat _z){
	glm::fmat4 rx, ry, rz;
	const float x = ToRadian(_x);
	const float y = ToRadian(_y);
	const float z = ToRadian(_z);
	rx[0][0] = 1.0f;	rx[0][1] = 0.0f;	rx[0][2] = 0.0f;	rx[0][3] = 0.0f;
	rx[1][0] = 0.0f;	rx[1][1] = cosf(x);	rx[1][2] = -sinf(x);rx[1][3] = 0.0f;
	rx[2][0] = 0.0f;	rx[2][1] = sinf(x);	rx[2][2] = cosf(x);	rx[2][3] = 0.0f;
	rx[3][0] = 0.0f;	rx[3][1] = 0.0f;	rx[3][2] = 0.0f;	rx[3][3] = 1.0f;
	ry[0][0] = cosf(y); ry[0][1] = 0.0f;	ry[0][2] = -sinf(y);ry[0][3] = 0.0f;
	ry[1][0] = 0.0f;	ry[1][1] = 1.0f;	ry[1][2] = 0.0f;	ry[1][3] = 0.0f;
	ry[2][0] = sinf(y); ry[2][1] = 0.0f;	ry[2][2] = cosf(y);	ry[2][3] = 0.0f;
	ry[3][0] = 0.0f;	ry[3][1] = 0.0f;	ry[3][2] = 0.0f;	ry[3][3] = 1.0f;
	rz[0][0] = cosf(z); rz[0][1] = -sinf(z);rz[0][2] = 0.0f;	rz[0][3] = 0.0f;
	rz[1][0] = sinf(z); rz[1][1] = cosf(z); rz[1][2] = 0.0f;	rz[1][3] = 0.0f;
	rz[2][0] = 0.0f;	rz[2][1] = 0.0f;	rz[2][2] = 1.0f;	rz[2][3] = 0.0f;
	rz[3][0] = 0.0f;	rz[3][1] = 0.0f;	rz[3][2] = 0.0f;	rz[3][3] = 1.0f;
	WorldRot = glm::transpose(rz * ry * rx);
}
void Scale(glm::fmat4& WorldScl, GLfloat x, GLfloat y, GLfloat z){
	WorldScl[0][0] = x;		WorldScl[0][1] = 0.0f;	WorldScl[0][2] = 0.0f;	WorldScl[0][3] = 0.0f;
	WorldScl[1][0] = 0.0f;	WorldScl[1][1] = y;		WorldScl[1][2] = 0.0f;	WorldScl[1][3] = 0.0f;
	WorldScl[2][0] = 0.0f;	WorldScl[2][1] = 0.0f;	WorldScl[2][2] = z;		WorldScl[2][3] = 0.0f;
	WorldScl[3][0] = 0.0f;	WorldScl[3][1] = 0.0f;	WorldScl[3][2] = 0.0f;	WorldScl[3][3] = 1.0f;
}
void RenderSceneCB(){
	glClear(GL_COLOR_BUFFER_BIT);
	static float rotate = 0.0f;
	rotate += 0.01f;
	glm::fmat4 WorldScl;
	glm::fmat4 WorldRot;
	glm::fmat4 WorldPos;
	glm::fmat4 CameraPos;
	glm::fmat4 CameraRot;
	glm::fmat4 WorldPers;
	Scale(WorldScl, 1.0f, 1.0f, 1.0f);
	RotateY(WorldRot, rotate);
	Translate(WorldPos, 0, 0, 5.0f);
	Pers(WorldPers, 1.0f, 100.0f, 1024, 768, 30);

	Translate(CameraPos, -m_camera.Pos.x, -m_camera.Pos.y, -m_camera.Pos.z);
	CameraTransform(m_camera.Target, m_camera.Up, CameraRot);

	*m_transformation = glm::transpose(WorldPers * CameraRot * CameraPos * WorldPos * WorldRot * WorldScl);

	glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, (const GLfloat*)m_transformation);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);

	glutSwapBuffers();
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType){
	GLuint ShaderObj = glCreateShader(ShaderType);

	if(ShaderObj == 0){
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
	if(!success){
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}

	glAttachShader(ShaderProgram, ShaderObj);
}

static void CompileShaders(){
	GLuint ShaderProgram = glCreateProgram();

	if(ShaderProgram == 0){
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	AddShader(ShaderProgram, pVS, GL_VERTEX_SHADER);
	AddShader(ShaderProgram, pFS, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = {0};

	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if(Success == 0){
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if(!Success){
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glUseProgram(ShaderProgram);

	gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");
	assert(gWorldLocation != 0xFFFFFFFF);
}
void setTetra(){
	glm::fvec3 Vertices[4];
	Vertices[0] = glm::fvec3(-1.0f, -1.0f, 0.5773f);
	Vertices[1] = glm::fvec3(0.0f, -1.0f, -1.15475);
	Vertices[2] = glm::fvec3(1.0f, -1.0f, 0.5773f);
	Vertices[3] = glm::fvec3(0.0f, 1.0f, 0.0f);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	unsigned int Indices[] = {0, 3, 1,
							  1, 3, 2,
							  2, 3, 0,
							  0, 2, 1};
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
}
int main(int argc, char** argv){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(1024, 768);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Tutorial 13");
	glutDisplayFunc(RenderSceneCB);
	glutIdleFunc(RenderSceneCB);


	GLenum res = glewInit();
	if(res != GLEW_OK){
		fprintf(stderr, "Error: '%s' \n", glewGetErrorString(res));
		return 1;
	}
	glm::fvec3 CameraPos(1.0f, 1.0f, 1.0f);
	glm::fvec3 CameraTarget(0.45f, 0.0f, 1.0f);
	glm::fvec3 CameraUp(0.0f, 1.0f, 0.0f);
	SetCamera(CameraPos, CameraTarget, CameraUp);
	setTetra();
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	CompileShaders();
	glutMainLoop();
}