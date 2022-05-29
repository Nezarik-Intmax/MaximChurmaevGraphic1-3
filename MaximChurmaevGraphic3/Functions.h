#include <iostream>
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

#define M_PI 3.14159265358979323846
#define ToRadian(x) ((x) * M_PI / 180.0f)
#define ToDegree(x) ((x) * 180.0f / M_PI)
#define MAX_POINT_LIGHTS 3

struct Vertex{
	glm::fvec3 m_pos;
	glm::fvec2 m_tex;
	glm::fvec3 m_normal;
	glm::fvec3 m_tangent;

	Vertex(){}

	Vertex(glm::fvec3 pos, glm::fvec2 tex){
		m_pos = pos;
		m_tex = tex;
		m_normal = glm::fvec3(0.0f, 0.0f, 0.0f);
		m_tangent = glm::fvec3(1.0f, 1.0f, 1.0f);
	}
};
struct{
	glm::fvec3 Pos;
	glm::fvec3 Target;
	glm::fvec3 Up;
} m_camera;


static const GLenum types[6] = {GL_TEXTURE_CUBE_MAP_POSITIVE_X,
								  GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
								  GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
								  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
								  GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
								  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};
GLenum glCheckError_(const char* file, int line){
	GLenum errorCode;
	while((errorCode = glGetError()) != GL_NO_ERROR){
		std::string error;
		switch(errorCode){
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)
void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	void* userParam){
	// ignore non-significant error/warning codes
	if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch(source){
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch(type){
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch(severity){
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}



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
	//m = glm::transpose(m);
}

void CalcNormals(const unsigned int* pIndices, unsigned int IndexCount, Vertex* pVertices, unsigned int VertexCount){
	for(unsigned int i = 0; i < IndexCount; i += 3){
		unsigned int Index0 = pIndices[i];
		unsigned int Index1 = pIndices[i + 1];
		unsigned int Index2 = pIndices[i + 2];
		glm::fvec3 v1 = pVertices[Index1].m_pos - pVertices[Index0].m_pos;
		glm::fvec3 v2 = pVertices[Index2].m_pos - pVertices[Index0].m_pos;
		glm::fvec3 Normal = cross(v1, v2);
		Normal = glm::normalize(Normal);

		pVertices[Index0].m_normal += Normal;
		pVertices[Index1].m_normal += Normal;
		pVertices[Index2].m_normal += Normal;
	}

	for(unsigned int i = 0; i < VertexCount; i++){
		pVertices[i].m_normal = glm::normalize(pVertices[i].m_normal);
	}

	for(unsigned int i = 0; i < IndexCount; i += 3){
		Vertex& v0 = pVertices[pIndices[i]];
		Vertex& v1 = pVertices[pIndices[i + 1]];
		Vertex& v2 = pVertices[pIndices[i + 2]];

		glm::fvec3 Edge1 = v1.m_pos - v0.m_pos;
		glm::fvec3 Edge2 = v2.m_pos - v0.m_pos;

		float DeltaU1 = v1.m_tex.x - v0.m_tex.x;
		float DeltaV1 = v1.m_tex.y - v0.m_tex.y;
		float DeltaU2 = v2.m_tex.x - v0.m_tex.x;
		float DeltaV2 = v2.m_tex.y - v0.m_tex.y;

		float f = 1.0f / (DeltaU1 * DeltaV2 - DeltaU2 * DeltaV1);

		glm::fvec3 Tangent, Bitangent;

		Tangent.x = f * (DeltaV2 * Edge1.x - DeltaV1 * Edge2.x);
		Tangent.y = f * (DeltaV2 * Edge1.y - DeltaV1 * Edge2.y);
		Tangent.z = f * (DeltaV2 * Edge1.z - DeltaV1 * Edge2.z);

		Bitangent.x = f * (-DeltaU2 * Edge1.x - DeltaU1 * Edge2.x);
		Bitangent.y = f * (-DeltaU2 * Edge1.y - DeltaU1 * Edge2.y);
		Bitangent.z = f * (-DeltaU2 * Edge1.z - DeltaU1 * Edge2.z);

		v0.m_tangent += Tangent;
		v1.m_tangent += Tangent;
		v2.m_tangent += Tangent;
	}

	for(unsigned int i = 0; i < VertexCount; i++){
		pVertices[i].m_tangent = glm::normalize(pVertices[i].m_tangent);
	}
}

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
	rx[1][0] = 0.0f;	rx[1][1] = cosf(x);	rx[1][2] = -sinf(x); rx[1][3] = 0.0f;
	rx[2][0] = 0.0f;	rx[2][1] = sinf(x);	rx[2][2] = cosf(x);	rx[2][3] = 0.0f;
	rx[3][0] = 0.0f;	rx[3][1] = 0.0f;	rx[3][2] = 0.0f;	rx[3][3] = 1.0f;
	ry[0][0] = cosf(y); ry[0][1] = 0.0f;	ry[0][2] = -sinf(y); ry[0][3] = 0.0f;
	ry[1][0] = 0.0f;	ry[1][1] = 1.0f;	ry[1][2] = 0.0f;	ry[1][3] = 0.0f;
	ry[2][0] = sinf(y); ry[2][1] = 0.0f;	ry[2][2] = cosf(y);	ry[2][3] = 0.0f;
	ry[3][0] = 0.0f;	ry[3][1] = 0.0f;	ry[3][2] = 0.0f;	ry[3][3] = 1.0f;
	rz[0][0] = cosf(z); rz[0][1] = -sinf(z); rz[0][2] = 0.0f;	rz[0][3] = 0.0f;
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

void initGlutAndMagick(int argc, char** argv){
	Magick::InitializeMagick(*argv);
	glutInit(&argc, argv);

	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_DEBUG);
	glutInitContextVersion(4, 2); // at least 3.2 is required, you can use a higer version when needed
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(1024, 768);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Tutorial 24");

	GLenum res = glewInit();
	if(res != GLEW_OK){
		fprintf(stderr, "Error: '%s' \n", glewGetErrorString(res));
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
}