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
#include "Mesh.h";
#include "Functions.h";
#include "Shaders.h";
#include "Lights.h";

using namespace std;
class Texture{
public:
	Texture(GLenum TextureTarget, const std::string& FileName){
		m_textureTarget = TextureTarget;
		m_fileName = FileName;
		m_pImage = NULL;
	}

	bool Load(){
		try{
			m_pImage = new Magick::Image(m_fileName);
			m_pImage->write(&m_blob, "RGBA");
		} catch(Magick::Error& Error){
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

	void Bind(GLenum TextureUnit){
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

class CubemapTexture{
public:

	CubemapTexture(const string& Directory, const string& PosXFilename, const string& NegXFilename, const string& PosYFilename, const string& NegYFilename, const string& PosZFilename, const string& NegZFilename){
		string BaseDir = Directory;

		m_fileNames[0] = BaseDir + PosXFilename;
		m_fileNames[1] = BaseDir + NegXFilename;
		m_fileNames[2] = BaseDir + PosYFilename;
		m_fileNames[3] = BaseDir + NegYFilename;
		m_fileNames[4] = BaseDir + PosZFilename;
		m_fileNames[5] = BaseDir + NegZFilename;

		m_textureObj = 0;
	}

	~CubemapTexture();

	bool Load(){
		glGenTextures(1, &m_textureObj);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureObj);

		Magick::Image* pImage = NULL;
		Magick::Blob blob;

		for(unsigned int i = 0; i < 6; i++){
			pImage = new Magick::Image(m_fileNames[i]);

			try{
				pImage->write(&blob, "RGBA");
			} catch(Magick::Error& Error){
				cout << "Error loading texture '" << m_fileNames[i] << "': " << Error.what() << endl;
				delete pImage;
				return false;
			}

			glTexImage2D(types[i], 0, GL_RGB, pImage->columns(), pImage->rows(), 0, GL_RGBA,
				GL_UNSIGNED_BYTE, blob.data());

			delete pImage;
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		return true;
	}

	void Bind(GLenum TextureUnit){
		glActiveTexture(TextureUnit);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureObj);
	}

private:
	string m_fileNames[6];
	GLuint m_textureObj;
};


class SkyBox{
public:
	SkyBox(){};

	~SkyBox(){};

	bool Init(const string& Directory, const string& PosXFilename, const string& NegXFilename, const string& PosYFilename, const string& NegYFilename, const string& PosZFilename, const string& NegZFilename){
		//glUniform1i(m_textureLocation, 0);
		m_pCubemapTex = new CubemapTexture(Directory,
			PosXFilename,
			NegXFilename,
			PosYFilename,
			NegYFilename,
			PosZFilename,
			NegZFilename);

		if(!m_pCubemapTex->Load()){
			return false;
		}
		//m_pMesh = new Mesh();
		return true;
	}
	void Render();
private:
	CubemapTexture* m_pCubemapTex;
};

class ShadowMapFBO{
public:
	ShadowMapFBO(){
		m_fbo = 0;
		m_shadowMap = 0;
	}

	~ShadowMapFBO(){
		if(m_fbo != 0){
			glDeleteFramebuffers(1, &m_fbo);
		}

		if(m_shadowMap != 0){
			glDeleteFramebuffers(1, &m_shadowMap);
		}
	}

	bool Init(unsigned int WindowWidth, unsigned int WindowHeight){
		glGenFramebuffers(1, &m_fbo);

		glGenTextures(1, &m_shadowMap);
		glBindTexture(GL_TEXTURE_2D, m_shadowMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
			m_shadowMap, 0);

		glDrawBuffer(GL_NONE);

		GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if(Status != GL_FRAMEBUFFER_COMPLETE){
			printf("FB error, status: 0x%x\n", Status);
			return false;
		}

		return true;
	}

	void BindForWriting(){
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
		//glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	}

	void BindForReading(GLenum TextureUnit){
		glActiveTexture(TextureUnit);
		glBindTexture(GL_TEXTURE_2D, m_shadowMap);
	}

public:
	GLuint m_fbo;
	GLuint m_shadowMap;
};
class BillboardList{
public:
	BillboardList();
	~BillboardList();

	bool Init(const std::string& TexFilename);

	void Render(const Matrix4f& VP, const Vector3f& CameraPos);

private:
	void CreatePositionBuffer();

	GLuint m_VB;
	Texture* m_pTexture;
};
void BillboardList::Render(const Matrix4f& VP, const Vector3f& CameraPos){
	m_technique.Enable();
	m_technique.SetVP(VP);
	m_technique.SetCameraPosition(CameraPos);

	m_pTexture->Bind(COLOR_TEXTURE_UNIT);

	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, m_VB);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), 0);   // position

	glDrawArrays(GL_POINTS, 0, NUM_ROWS * NUM_COLUMNS);

	glDisableVertexAttribArray(0);
}


GLuint VAO;
GLuint VBO;
GLuint IBO;
GLuint VBO2;
GLuint IBO2;
GLuint VBO3;
GLuint IBO3;
GLuint VBO4;
GLuint IBO4;
GLuint gWVPLocation;
GLuint m_WorldMatrixLocation;
GLuint m_dirLightColorLocation;
GLuint m_dirLightAmbientIntensityLocation;
GLuint m_dirLightLocationDirection;
GLuint m_dirLightLocationDiffuseIntensity;
GLuint m_eyeWorldPosition;
GLuint m_matSpecularIntensityLocation;
GLuint m_matSpecularPowerLocation;
GLuint m_numPointLightsLocation;
GLuint m_numSpotLightsLocation;
GLuint m_LightWVPLocation; 
GLuint m_shadowMapLocation; 
Texture* pTexture = NULL;
Texture* pNormalMap = NULL;
GLuint gSampler;
glm::fmat4* m_transformation = new glm::fmat4();
glm::fmat4* m_transformationS = new glm::fmat4();
glm::fmat4* World = new glm::fmat4();
glm::fvec3 CameraTarget(0.0f, 0.0f, 1.0f);
glm::fvec3 CameraUp(0.0f, 1.0f, 0.0f);
glm::fvec3 CameraPos_(0.0f, 0.0f, 0.0f);
ShadowMapFBO m_shadowMapFBO;
SpotLight sl[1];
SkyBox* m_pSkyBox;

GLuint ShaderShadowProgram; 
GLuint ShaderProgram; 

GLuint m_WVPLocation;
GLuint m_textureLocation;

GLuint ShaderSkyboxProgram; 

GLuint skybox_WVPLocation; 
GLuint skybox_textureLocation; 

GLuint normalMap; 
Mesh* m_pSphereMesh; 



void render(){

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)32);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO2);

	pNormalMap->Bind(GL_TEXTURE2);
	pTexture->Bind(GL_TEXTURE0);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	/*
	glBindBuffer(GL_ARRAY_BUFFER, VBO3);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)32);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO3);

	pTexture->Bind(GL_TEXTURE0);

	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
	

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)32);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	pNormalMap->Bind(GL_TEXTURE2);
	pTexture->Bind(GL_TEXTURE0);

	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);*/


	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);


	pNormalMap->Bind(GL_TEXTURE2);
	//pTexture->Bind(GL_TEXTURE2);
	m_pSphereMesh->Render();

}
void RenderSceneCB(){
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
	Translate(WorldPos, 0.0f, 0.0f, 5.0f);
	Pers(WorldPers, 1.0f, 50.0f, 1024, 768, 60);
	*World = WorldPos * WorldRot * WorldScl;

	m_shadowMapFBO.BindForWriting();
	glClear(GL_DEPTH_BUFFER_BIT);
	/*** Shadow ***/
	glUseProgram(ShaderShadowProgram);
	SetCamera(sl[0].Position, sl[0].Direction, glm::fvec3(0.0f, 1.0f, 0.0f));
	Translate(CameraPos, -sl[0].Position.x, -sl[0].Position.y, -sl[0].Position.z);
	CameraTransform(sl[0].Direction, glm::fvec3(0.0f, 1.0f, 0.0f), CameraRot);
	*m_transformationS = glm::transpose(WorldPers * glm::transpose(CameraRot) * CameraPos * *World);
	glUniformMatrix4fv(m_WVPLocation, 1, GL_TRUE, (const GLfloat*)m_transformationS);
	glCheckError();
	render();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/*** Main Render ***/
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(ShaderProgram);
	m_shadowMapFBO.BindForReading(GL_TEXTURE1); 
	SetCamera(CameraPos_, CameraTarget, CameraUp);
	Translate(CameraPos, -m_camera.Pos.x, -m_camera.Pos.y, -m_camera.Pos.z);
	CameraTransform(m_camera.Target, m_camera.Up, CameraRot);
	*m_transformation = glm::transpose(WorldPers * glm::transpose(CameraRot) * CameraPos * *World);

	glUniformMatrix4fv(m_WorldMatrixLocation, 1, GL_TRUE, (const GLfloat*)World);
	glUniformMatrix4fv(gWVPLocation, 1, GL_TRUE, (const GLfloat*)m_transformation);
	glUniform3f(m_eyeWorldPosition, m_camera.Pos.x, m_camera.Pos.y, m_camera.Pos.z);

	SetCamera(sl[0].Position, sl[0].Direction, glm::fvec3(0.0f, 1.0f, 0.0f));	
	Translate(CameraPos, -sl[0].Position.x, -sl[0].Position.y, -sl[0].Position.z);	
	CameraTransform(sl[0].Direction, glm::fvec3(0.0f, 1.0f, 0.0f), CameraRot);	
	*m_transformationS = glm::transpose(WorldPers * glm::transpose(CameraRot) * CameraPos * *World); 
	glUniformMatrix4fv(m_LightWVPLocation, 1, GL_TRUE, (const GLfloat*)m_transformationS);	
	glCheckError();

	render();
	m_pSkyBox->Render();

	glutSwapBuffers();
}

void SkyBox::Render(){
	glUseProgram(ShaderSkyboxProgram);

	GLint OldCullFaceMode;
	glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);
	GLint OldDepthFuncMode;
	glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);

	glCullFace(GL_FRONT);
	glDepthFunc(GL_LEQUAL);

	glm::fmat4 WorldScl;
	glm::fmat4 WorldRot;
	glm::fmat4 WorldPos;
	glm::fmat4 CameraPos;
	glm::fmat4 CameraRot;
	glm::fmat4 WorldPers;

	glCheckError();
	static float rotate = 0.0f;
	rotate += 0.01f;

	glm::fvec3 CameraPos_(0.0f, 0.0f, 0.0f);
	glm::fvec3 CameraTarget(0.0f, 0.0f, 1.0f);
	glm::fvec3 CameraUp(0.0f, 1.0f, 0.0f);
	Translate(CameraPos, -m_camera.Pos.x, -m_camera.Pos.y, -m_camera.Pos.z);
	CameraTransform(m_camera.Target, m_camera.Up, CameraRot);
	Scale(WorldScl, 20.0f, 20.0f, 20.0f);
	RotateY(WorldRot, rotate);
	Translate(WorldPos, m_camera.Pos.x, m_camera.Pos.y, m_camera.Pos.z);
	Pers(WorldPers, 1.0f, 50.0f, 1024, 768, 60);
	SetCamera(CameraPos_, CameraTarget, CameraUp);
	Translate(CameraPos, -m_camera.Pos.x, -m_camera.Pos.y, -m_camera.Pos.z);
	CameraTransform(m_camera.Target, m_camera.Up, CameraRot);
	*World = WorldPos * WorldRot * WorldScl;
	*m_transformationS = glm::transpose(WorldPers * glm::transpose(CameraRot) * CameraPos * *World);
	glUniformMatrix4fv(m_WVPLocation, 1, GL_TRUE, (const GLfloat*)m_transformationS);
	glCheckError();
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);


	glBindBuffer(GL_ARRAY_BUFFER, VBO4);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO4);

	m_pCubemapTex->Bind(GL_TEXTURE0);

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glCullFace(OldCullFaceMode);
	glDepthFunc(OldDepthFuncMode);
}

static void AddShader(GLuint ShaderProgram_, const char* pShaderText, GLenum ShaderType){
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

	glAttachShader(ShaderProgram_, ShaderObj);
}

static void CompileShadowShaders(){
	ShaderShadowProgram = glCreateProgram();

	if(ShaderShadowProgram == 0){
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	AddShader(ShaderShadowProgram, shadow_pVS, GL_VERTEX_SHADER);
	AddShader(ShaderShadowProgram, shadow_pFS, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = {0};

	glLinkProgram(ShaderShadowProgram);
	glGetProgramiv(ShaderShadowProgram, GL_LINK_STATUS, &Success);
	if(Success == 0){
		glGetProgramInfoLog(ShaderShadowProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glValidateProgram(ShaderShadowProgram);
	glGetProgramiv(ShaderShadowProgram, GL_VALIDATE_STATUS, &Success);
	if(!Success){
		glGetProgramInfoLog(ShaderShadowProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glUseProgram(ShaderShadowProgram);

	m_WVPLocation = glGetUniformLocation(ShaderShadowProgram, "gWVP");
	assert(m_WVPLocation != 0xFFFFFFFF);
	m_textureLocation = glGetUniformLocation(ShaderShadowProgram, "gShadowMap");
	assert(m_textureLocation != 0xFFFFFFFF);
}

static void CompileSkyboxShaders(){
	ShaderSkyboxProgram = glCreateProgram();

	if(ShaderSkyboxProgram == 0){
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	AddShader(ShaderSkyboxProgram, skybox_pVS, GL_VERTEX_SHADER);
	AddShader(ShaderSkyboxProgram, skybox_pFS, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = {0};

	glLinkProgram(ShaderSkyboxProgram);
	glGetProgramiv(ShaderSkyboxProgram, GL_LINK_STATUS, &Success);
	if(Success == 0){
		glGetProgramInfoLog(ShaderSkyboxProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glValidateProgram(ShaderSkyboxProgram);
	glGetProgramiv(ShaderSkyboxProgram, GL_VALIDATE_STATUS, &Success);
	if(!Success){
		glGetProgramInfoLog(ShaderSkyboxProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glUseProgram(ShaderSkyboxProgram);

	skybox_WVPLocation = glGetUniformLocation(ShaderSkyboxProgram, "gWVP");
	assert(skybox_WVPLocation != 0xFFFFFFFF);
	skybox_textureLocation = glGetUniformLocation(ShaderSkyboxProgram, "gCubemapTexture");
	assert(skybox_textureLocation != 0xFFFFFFFF);
}

static void CompileShaders(){
	ShaderProgram = glCreateProgram();

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

	gWVPLocation = glGetUniformLocation(ShaderProgram, "gWVP");
	assert(gWVPLocation != 0xFFFFFFFF);
	m_WorldMatrixLocation = glGetUniformLocation(ShaderProgram, "gWorld");
	assert(m_WorldMatrixLocation != 0xFFFFFFFF);
	gSampler = glGetUniformLocation(ShaderProgram, "gSampler");
	assert(gSampler != 0xFFFFFFFF);
	normalMap = glGetUniformLocation(ShaderProgram, "gNormalMap");
	assert(normalMap != 0xFFFFFFFF);
	m_dirLightColorLocation = glGetUniformLocation(ShaderProgram, "gDirectionalLight.Color");
	assert(m_dirLightColorLocation != 0xFFFFFFFF);
	m_dirLightAmbientIntensityLocation = glGetUniformLocation(ShaderProgram, "gDirectionalLight.AmbientIntensity");
	assert(m_dirLightAmbientIntensityLocation != 0xFFFFFFFF);
	m_dirLightLocationDirection = glGetUniformLocation(ShaderProgram, "gDirectionalLight.Direction");
	assert(m_dirLightLocationDirection != 0xFFFFFFFF);
	m_dirLightLocationDiffuseIntensity = glGetUniformLocation(ShaderProgram, "gDirectionalLight.DiffuseIntensity");
	assert(m_dirLightLocationDiffuseIntensity != 0xFFFFFFFF);
	m_eyeWorldPosition = glGetUniformLocation(ShaderProgram, "gEyeWorldPos");
	assert(m_eyeWorldPosition != 0xFFFFFFFF);
	m_matSpecularIntensityLocation = glGetUniformLocation(ShaderProgram, "gMatSpecularIntensity");
	assert(m_matSpecularIntensityLocation != 0xFFFFFFFF);
	m_matSpecularPowerLocation = glGetUniformLocation(ShaderProgram, "gSpecularPower");
	assert(m_matSpecularPowerLocation != 0xFFFFFFFF);
	m_numPointLightsLocation = glGetUniformLocation(ShaderProgram, "gNumPointLights");
	assert(m_numPointLightsLocation != 0xFFFFFFFF);
	m_LightWVPLocation = glGetUniformLocation(ShaderProgram, "gLightWVP");
	assert(m_LightWVPLocation != 0xFFFFFFFF);
	m_shadowMapLocation = glGetUniformLocation(ShaderProgram, "gShadowMap");
	assert(m_shadowMapLocation != 0xFFFFFFFF);
	for(unsigned int i = 0; i < MAX_POINT_LIGHTS; i++){
		char Name[128];
		memset(Name, 0, sizeof(Name));
		snprintf(Name, sizeof(Name), "gPointLights[%d].Base.Color", i);
		m_pointLightsLocation[i].Color = glGetUniformLocation(ShaderProgram, Name);
		snprintf(Name, sizeof(Name), "gPointLights[%d].Base.AmbientIntensity", i);
		m_pointLightsLocation[i].AmbientIntensity = glGetUniformLocation(ShaderProgram, Name);
		snprintf(Name, sizeof(Name), "gPointLights[%d].Position", i);
		m_pointLightsLocation[i].Position = glGetUniformLocation(ShaderProgram, Name);
		snprintf(Name, sizeof(Name), "gPointLights[%d].Base.DiffuseIntensity", i);
		m_pointLightsLocation[i].DiffuseIntensity = glGetUniformLocation(ShaderProgram, Name);
		snprintf(Name, sizeof(Name), "gPointLights[%d].Atten.Constant", i);
		m_pointLightsLocation[i].Atten.Constant = glGetUniformLocation(ShaderProgram, Name);
		snprintf(Name, sizeof(Name), "gPointLights[%d].Atten.Linear", i);
		m_pointLightsLocation[i].Atten.Linear = glGetUniformLocation(ShaderProgram, Name);
		snprintf(Name, sizeof(Name), "gPointLights[%d].Atten.Exp", i);
		m_pointLightsLocation[i].Atten.Exp = glGetUniformLocation(ShaderProgram, Name);
	}
	m_numSpotLightsLocation = glGetUniformLocation(ShaderProgram, "gNumSpotLights");
	assert(m_numSpotLightsLocation != 0xFFFFFFFF);
	for(unsigned int i = 0; i < MAX_POINT_LIGHTS; i++){
		char Name[128];
		memset(Name, 0, sizeof(Name));
		snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Base.Color", i);
		m_spotLightsLocation[i].Color = glGetUniformLocation(ShaderProgram, Name);
		snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Base.AmbientIntensity", i);
		m_spotLightsLocation[i].AmbientIntensity = glGetUniformLocation(ShaderProgram, Name);
		snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Position", i);
		m_spotLightsLocation[i].Position = glGetUniformLocation(ShaderProgram, Name);
		snprintf(Name, sizeof(Name), "gSpotLights[%d].Direction", i);
		m_spotLightsLocation[i].Direction = glGetUniformLocation(ShaderProgram, Name);

		snprintf(Name, sizeof(Name), "gSpotLights[%d].Cutoff", i);
		m_spotLightsLocation[i].Cutoff = glGetUniformLocation(ShaderProgram, Name);
		snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Base.DiffuseIntensity", i);
		m_spotLightsLocation[i].DiffuseIntensity = glGetUniformLocation(ShaderProgram, Name);
		snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Constant", i);
		m_spotLightsLocation[i].Atten.Constant = glGetUniformLocation(ShaderProgram, Name);
		snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Linear", i);
		m_spotLightsLocation[i].Atten.Linear = glGetUniformLocation(ShaderProgram, Name);
		snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Exp", i);
		m_spotLightsLocation[i].Atten.Exp = glGetUniformLocation(ShaderProgram, Name);
	}
}

int main(int argc, char** argv){
	initGlutAndMagick(argc, argv);
	glutDisplayFunc(RenderSceneCB);
	glutIdleFunc(RenderSceneCB);

	CompileShadowShaders();
	glUniform1i(m_shadowMapLocation, 0); 
	glCheckError();

	glUniform1i(m_textureLocation, 1);
	glCheckError();

	if(!m_shadowMapFBO.Init(1024, 768)){
		return false;
	}

	CompileSkyboxShaders();
	glUniform1i(skybox_textureLocation, 0); 
	glCheckError();

	CompileShaders();
	glUniform1i(gSampler, 0);
	glCheckError();

	glUniform1i(m_shadowMapLocation, 1); 
	glCheckError();

	glUniform1i(normalMap, 2); 
	glCheckError();

	{

	Vertex Vertices[4] = {
		Vertex(glm::fvec3(-1.0f, -1.0f, 0.5773f), glm::fvec2(0.0f, 0.0f)),
		Vertex(glm::fvec3(0.0f, -1.0f, -1.15475), glm::fvec2(0.5f, 0.0f)),
		Vertex(glm::fvec3(1.0f, -1.0f, 0.5773f),  glm::fvec2(1.0f, 0.0f)),
		Vertex(glm::fvec3(0.0f, 1.0f, 0.0f),      glm::fvec2(0.5f, 1.0f))
	};
	Vertex Vertices2[4] = {
		Vertex(glm::fvec3(-10.0f, -1.0f, 10.0f),	glm::fvec2(0.0f, 0.0f)),
		Vertex(glm::fvec3(10.0f, -1.0f, -10.0f),	glm::fvec2(1.0f, 0.0f)),
		Vertex(glm::fvec3(-10.0f, -1.0f, -10.0f),	glm::fvec2(0.5f, 1.0f)),
		Vertex(glm::fvec3(10.0f, -1.0f, 10.0f),	glm::fvec2(0.5f, 1.0f))
	};
	Vertex Vertices3[4] = {
		Vertex(glm::fvec3(-3.0f, -1.0f, 0.5773f), glm::fvec2(0.0f, 0.0f)),
		Vertex(glm::fvec3(-2.0f, -1.0f, -1.15475), glm::fvec2(0.5f, 0.0f)),
		Vertex(glm::fvec3(-1.0f, -1.0f, 0.5773f),  glm::fvec2(1.0f, 0.0f)),
		Vertex(glm::fvec3(-2.0f, 1.0f, 0.0f),      glm::fvec2(0.5f, 1.0f))
	};
	Vertex Vertices4[8] = {
		Vertex(glm::fvec3(-1.0f, -1.0f, 0.5773f), glm::fvec2(0.0f, 0.0f)),
		Vertex(glm::fvec3(-1.0f, -1.0f, -1.15475), glm::fvec2(0.5f, 0.0f)),
		Vertex(glm::fvec3(-1.0f, 1.0f, 0.5773f),  glm::fvec2(1.0f, 0.0f)),
		Vertex(glm::fvec3(-1.0f, 1.0f, -1.15475),      glm::fvec2(0.5f, 1.0f)),

		Vertex(glm::fvec3(1.0f, -1.0f, 0.5773f), glm::fvec2(0.0f, 0.0f)),
		Vertex(glm::fvec3(1.0f, -1.0f, -1.15475), glm::fvec2(0.5f, 0.0f)),
		Vertex(glm::fvec3(1.0f, 1.0f, 0.5773f),  glm::fvec2(1.0f, 0.0f)),
		Vertex(glm::fvec3(1.0f, 1.0f, -1.15475),      glm::fvec2(0.5f, 1.0f))
	};

	unsigned int Indices[] = {0, 3, 1,
							   1, 3, 2,
							   2, 3, 0,
							   1, 2, 0};
	unsigned int Indices2[] = {0, 1, 2,
							   3, 1, 0};
	unsigned int Indices3[] = {0, 3, 1,
							   1, 3, 2,
							   2, 3, 0,
							   1, 2, 0};
	unsigned int Indices4[] = {	0, 1, 4,
								4, 1, 5, // bottom
								0, 4, 2,
								2, 4, 6, // backward
								4, 5, 7,
								6, 4, 7, // right
								0, 3, 1,
								2, 3, 0, // left
								1, 7, 5,
								1, 3, 7, // forward
								2, 6, 7,
								2, 7, 3  // top
								};

	CalcNormals(Indices, 12, Vertices, 4);
	CalcNormals(Indices3, 12, Vertices3, 4);
	CalcNormals(Indices2, 6, Vertices2, 4);
	CalcNormals(Indices4, 36, Vertices4, 8);
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

	glGenBuffers(1, &VBO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices2), Vertices2, GL_STATIC_DRAW);

	glGenBuffers(1, &IBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices2), Indices2, GL_STATIC_DRAW);

	glGenBuffers(1, &VBO3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices3), Vertices3, GL_STATIC_DRAW);

	glGenBuffers(1, &IBO3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices3), Indices3, GL_STATIC_DRAW);

	glGenBuffers(1, &VBO4);
	glBindBuffer(GL_ARRAY_BUFFER, VBO4);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices4), Vertices4, GL_STATIC_DRAW);

	glGenBuffers(1, &IBO4);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO4);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices4), Indices4, GL_STATIC_DRAW);

	glEnable(GL_DEPTH_TEST);
	}

	sl[0].DiffuseIntensity = 0.8f;
	sl[0].Color = glm::fvec3(1.0f, 1.0f, 1.0f);
	sl[0].Position = glm::fvec3(-8.3f, 3.0f, 5.0f);
	sl[0].AmbientIntensity = 0.3f;
	sl[0].Direction = glm::fvec3(0.8f, -0.3f, 0.0f);
	sl[0].Cutoff = 200.5f;
	glUniform1i(m_numSpotLightsLocation, 1);
	glCheckError();

	for(unsigned int i = 0; i < 1; i++){
		glUniform3f(m_spotLightsLocation[i].Color, sl[i].Color.x, sl[i].Color.y, sl[i].Color.z);
		glUniform1f(m_spotLightsLocation[i].AmbientIntensity, sl[i].AmbientIntensity);
		glUniform1f(m_spotLightsLocation[i].DiffuseIntensity, sl[i].DiffuseIntensity);
		glUniform3f(m_spotLightsLocation[i].Position, sl[i].Position.x, sl[i].Position.y, sl[i].Position.z);
		glm::fvec3 Direction = sl[i].Direction;
		Direction = glm::normalize(Direction);
		glUniform3f(m_spotLightsLocation[i].Direction, Direction.x, Direction.y, Direction.z);
		glUniform1f(m_spotLightsLocation[i].Cutoff, glm::cos(glm::radians(sl[i].Cutoff)));
		glUniform1f(m_spotLightsLocation[i].Atten.Constant, sl[i].Attenuation.Constant);
		glUniform1f(m_spotLightsLocation[i].Atten.Linear, sl[i].Attenuation.Linear);
		glUniform1f(m_spotLightsLocation[i].Atten.Exp, sl[i].Attenuation.Exp);
	}

	m_pSphereMesh = new Mesh();

	if(!m_pSphereMesh->LoadMesh("C:\\box.obj")){
		return false;
	}

	pTexture = new Texture(GL_TEXTURE_2D, "C:\\bricks.jpg");

	if(!pTexture->Load()){
		return 1;
	}
	
	pNormalMap = new Texture(GL_TEXTURE_2D, "C:\\normal_map.jpg");
	//pNormalMap = new Texture(GL_TEXTURE_2D, "C:\\TestNormalMap.png");

	if(!pNormalMap->Load()){
		return 1;
	}

	m_pSkyBox = new SkyBox();

	if(!m_pSkyBox->Init("C:\\",
		"sp3right.jpg",
		"sp3left.jpg",
		"sp3top.jpg",
		"sp3bot.jpg",
		"sp3front.jpg",
		"sp3back.jpg")){
		return false;
	}

	glutMainLoop();
}