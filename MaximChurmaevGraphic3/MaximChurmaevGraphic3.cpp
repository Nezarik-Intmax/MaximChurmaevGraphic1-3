#include <iostream>
#include "GL/glew.h";
#include "GL/glut.h";
#include "glm/glm.hpp";
#include "GL/freeglut.h";
#include <Magick++.h>
GLuint VBO;
GLuint IBO;
float Scale = 0.0f;
float Rotate = 0.0f;
#define M_PI 3.14159265358979323846
#define ToRadian(x) ((x) * M_PI / 180.0f)
#define ToDegree(x) ((x) * 180.0f / M_PI)
void InitPers(glm::mat4x4& m, float zNear, float zFar, float width, float height, float fov) {
	const float ar = width / height;//1024 / 768;// m_persProj.Width / m_persProj.Height;
	//const float zNear = 1.0f;// m_persProj.zNear;
	//const float zFar = 1000.0f;// m_persProj.zFar;
	const float zRange = zNear - zFar;
	//const float tanHalfFOV = tanf(ToRadian(30.0f / 2.0));
	const float tanHalfFOV = tanf(ToRadian(fov / 2.0f));

	/*m[0][0] = 1.0f / (tanHalfFOV * ar);
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
	m[3][3] = 0.0f;*/

	m[0][0] = 1.0f / (tanHalfFOV * ar);
	m[1][0] = 0.0f;
	m[2][0] = 0.0f;
	m[3][0] = 0.0f;

	m[0][1] = 0.0f;
	m[1][1] = 1.0f / tanHalfFOV;
	m[2][1] = 0.0f;
	m[3][1] = 0.0f;

	m[0][2] = 0.0f;
	m[1][2] = 0.0f;
	m[2][2] = (-zNear - zFar) / zRange;
	m[3][2] = 2.0f * zFar * zNear / zRange;

	m[0][3] = 0.0f;
	m[1][3] = 0.0f;
	m[2][3] = 1.0f;
	m[3][3] = 0.0f;

}
/*void InitPerspectiveProj(Matrix4f& m) {
	const float ar = m_persProj.Width / m_persProj.Height;
	const float zNear = m_persProj.zNear;
	const float zFar = m_persProj.zFar;
	const float zRange = zNear - zFar;
	const float tanHalfFOV = tanf(ToRadian(m_persProj.FOV / 2.0));

	m.m[0][0] = 1.0f / (tanHalfFOV * ar);
	m.m[0][1] = 0.0f;
	m.m[0][2] = 0.0f;
	m.m[0][3] = 0.0f;

	m.m[1][0] = 0.0f;
	m.m[1][1] = 1.0f / tanHalfFOV;
	m.m[1][2] = 0.0f;
	m.m[1][3] = 0.0f;

	m.m[2][0] = 0.0f;
	m.m[2][1] = 0.0f;
	m.m[2][2] = (-zNear - zFar) / zRange;
	m.m[2][3] = 2.0f * zFar * zNear / zRange;

	m.m[3][0] = 0.0f;
	m.m[3][1] = 0.0f;
	m.m[3][2] = 1.0f;
	m.m[3][3] = 0.0f;
}*/
void RenderSceneCB() {
	glClear(GL_COLOR_BUFFER_BIT);
	glm::vec3 Vertices[4];
	Vertices[0] = glm::vec3(-1.0f, -1.0f, 0.5773f);
	Vertices[1] = glm::vec3(0.0f, -1.0f, -1.15475);
	Vertices[2] = glm::vec3(1.0f, -1.0f, 0.5773f);
	Vertices[3] = glm::vec3(0.0f, 1.0f, 0.0f);

	Scale = -0.05f;//+= 0.001f;
	Rotate += 0.001f;
	glm::mat4x4 WorldPos;
	WorldPos[0][0] = 1.0f; WorldPos[0][1] = 0.0f; WorldPos[0][2] = 0.0f; WorldPos[0][3] = 0.0f;//sinf(Scale);
	WorldPos[1][0] = 0.0f; WorldPos[1][1] = 1.0f; WorldPos[1][2] = 0.0f; WorldPos[1][3] = 0.0f;
	WorldPos[2][0] = 0.0f; WorldPos[2][1] = 0.0f; WorldPos[2][2] = 1.0f; WorldPos[2][3] = 5.0f;
	WorldPos[3][0] = 0.0f; WorldPos[3][1] = 0.0f; WorldPos[3][2] = 0.0f; WorldPos[3][3] = 1.0f;
	/**/
	glm::mat4x4 WorldRot;
	/*
	WorldRot[0][0] = cosf(Scale); WorldRot[0][1] = -sinf(Scale); WorldRot[0][2] = 0.0f; WorldRot[0][3] = 0.0f;
	WorldRot[1][0] = sinf(Scale); WorldRot[1][1] = cosf(Scale);  WorldRot[1][2] = 0.0f; WorldRot[1][3] = 0.0f;
	WorldRot[2][0] = 0.0f;        WorldRot[2][1] = 0.0f;         WorldRot[2][2] = 1.0f; WorldRot[2][3] = 0.0f;
	WorldRot[3][0] = 0.0f;        WorldRot[3][1] = 0.0f;         WorldRot[3][2] = 0.0f; WorldRot[3][3] = 1.0f;
	*/
	glm::mat4x4 WorldScl;
	WorldScl[0][0] = sinf(Scale); WorldScl[0][1] = 0.0f;        WorldScl[0][2] = 0.0f;        WorldScl[0][3] = 0.0f;
	WorldScl[1][0] = 0.0f;        WorldScl[1][1] = cosf(Scale); WorldScl[1][2] = 0.0f;        WorldScl[1][3] = 0.0f;
	WorldScl[2][0] = 0.0f;        WorldScl[2][1] = 0.0f;        WorldScl[2][2] = sinf(Scale); WorldScl[2][3] = 0.0f;
	WorldScl[3][0] = 0.0f;        WorldScl[3][1] = 0.0f;        WorldScl[3][2] = 0.0f;        WorldScl[3][3] = 1.0f;
	glm::mat4x4 WorldPers;

	WorldRot[0][0] = cosf(Rotate); WorldRot[0][1] = 0.0f; WorldRot[0][2] = -sinf(Rotate); WorldRot[0][3] = 0.0f;
	WorldRot[1][0] = 0.0f; WorldRot[1][1] = 1.0f; WorldRot[1][2] = 0.0f; WorldRot[1][3] = 0.0f;
	WorldRot[2][0] = sinf(Rotate); WorldRot[2][1] = 0.0f; WorldRot[2][2] = cosf(Rotate); WorldRot[2][3] = 0.0f;
	WorldRot[3][0] = 0.0f; WorldRot[3][1] = 0.0f; WorldRot[3][2] = 0.0f; WorldRot[3][3] = 1.0f;
	InitPers(WorldPers, 0.1f, 100.0f, 1024, 768, 120.0f);
	glm::mat4x4 m_transformation = WorldPers * WorldPos/**/ */**/ WorldRot;// *WorldScl;
	/*
	
	*/
	Vertices[0] = m_transformation * glm::vec4(Vertices[0], 1.0f);
	Vertices[1] = m_transformation * glm::vec4(Vertices[1], 1.0f);
	Vertices[2] = m_transformation * glm::vec4(Vertices[2], 1.0f);
	Vertices[3] = m_transformation * glm::vec4(Vertices[3], 1.0f);
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

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
	//glDrawArrays(GL_TRIANGLES, 0, 4);

	glDisableVertexAttribArray(0);

	glutSwapBuffers();
}
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(1024, 768);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Tutorial 06");
	glutDisplayFunc(RenderSceneCB);
	glutIdleFunc(RenderSceneCB);

	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s' \n", glewGetErrorString(res));
		return 1;
	}
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glutMainLoop();
}