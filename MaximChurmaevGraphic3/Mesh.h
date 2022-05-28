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
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define COLOR_TEXTURE_UNIT GL_TEXTURE0
#define SHADOW_TEXTURE_UNIT GL_TEXTURE1
#define NORMAL_TEXTURE_UNIT GL_TEXTURE2
#define INVALID_OGL_VALUE 0xFFFFFFFF
#define SAFE_DELETE(p) if (p) { delete p; p = NULL; }
class TextureMesh{
public:
    TextureMesh(GLenum TextureMeshTarget, const std::string& FileName){
        m_textureTarget = TextureMeshTarget;
        m_fileName = FileName;
        m_pImage = NULL;
    }

    bool Load(){
        try{
            m_pImage = new Magick::Image(m_fileName);
            m_pImage->write(&m_blob, "RGBA");
        } catch(Magick::Error& Error){
            std::cout << "Error loading TextureMesh '" << m_fileName << "': " << Error.what() << std::endl;
            return false;
        }

        glGenTextures(1, &m_textureObj);
        glBindTexture(m_textureTarget, m_textureObj);
        glTexImage2D(m_textureTarget, 0, GL_RGB, m_pImage->columns(), m_pImage->rows(), -0.5, GL_RGBA, GL_UNSIGNED_BYTE, m_blob.data());
        glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return true;
    }

    void Bind(GLenum TextureMeshUnit){
        glActiveTexture(TextureMeshUnit);
        glBindTexture(m_textureTarget, m_textureObj);
    }
private:
    std::string m_fileName;
    GLenum m_textureTarget;
    GLuint m_textureObj;
    Magick::Image* m_pImage;
    Magick::Blob m_blob;
};

struct VertexMesh{
    glm::fvec3 m_pos;
    glm::fvec2 m_tex;
    glm::fvec3 m_normal;
    glm::fvec3 m_tangent;

    VertexMesh(){}

    VertexMesh(const glm::fvec3& pos, const glm::fvec2& tex, const glm::fvec3& normal, const glm::fvec3& Tangent){
        m_pos = pos;
        m_tex = tex;
        m_normal = normal;
        m_tangent = Tangent;

    }
};


class Mesh{
public:
    Mesh();

    ~Mesh();

    bool LoadMesh(const std::string& Filename);

    void Render();

private:
    bool InitFromScene(const aiScene* pScene, const std::string& Filename);
    void InitMesh(unsigned int Index, const aiMesh* paiMesh);
    bool InitMaterials(const aiScene* pScene, const std::string& Filename);
    void Clear();

#define INVALID_MATERIAL 0xFFFFFFFF

    struct MeshEntry{
        MeshEntry();

        ~MeshEntry();

        bool Init(const std::vector<VertexMesh>& Vertices,
            const std::vector<unsigned int>& Indices);

        GLuint VB;
        GLuint IB;
        unsigned int NumIndices;
        unsigned int MaterialIndex;
    };

    std::vector<MeshEntry> m_Entries;
    std::vector<TextureMesh*> m_Textures;
};

Mesh::MeshEntry::MeshEntry(){
    VB = INVALID_OGL_VALUE;
    IB = INVALID_OGL_VALUE;
    NumIndices = 0;
    MaterialIndex = INVALID_MATERIAL;
};

Mesh::MeshEntry::~MeshEntry(){
    if(VB != INVALID_OGL_VALUE){
        glDeleteBuffers(1, &VB);
    }

    if(IB != INVALID_OGL_VALUE){
        glDeleteBuffers(1, &IB);
    }
}

bool Mesh::MeshEntry::Init(const std::vector<VertexMesh>& Vertices,
    const std::vector<unsigned int>& Indices){
    NumIndices = Indices.size();

    glGenBuffers(1, &VB);
    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexMesh) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &IB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * NumIndices, &Indices[0], GL_STATIC_DRAW);
    return true;
}

Mesh::Mesh(){
}


Mesh::~Mesh(){
    Clear();
}


void Mesh::Clear(){
    for(unsigned int i = 0; i < m_Textures.size(); i++){
        SAFE_DELETE(m_Textures[i]);
    }
}


bool Mesh::LoadMesh(const std::string& Filename){
    // Release the previously loaded mesh (if it exists)
    Clear();

    bool Ret = false;
    Assimp::Importer Importer;

    const aiScene* pScene = Importer.ReadFile(Filename.c_str(), aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace);
    if(pScene){
        Ret = InitFromScene(pScene, Filename);
    } else{
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), Importer.GetErrorString());
    }

    return Ret;
}

bool Mesh::InitFromScene(const aiScene* pScene, const std::string& Filename){
    m_Entries.resize(pScene->mNumMeshes);
    m_Textures.resize(pScene->mNumMaterials);

    // Initialize the meshes in the scene one by one
    for(unsigned int i = 0; i < m_Entries.size(); i++){
        const aiMesh* paiMesh = pScene->mMeshes[i];
        InitMesh(i, paiMesh);
    }

    return InitMaterials(pScene, Filename);
}

void Mesh::InitMesh(unsigned int Index, const aiMesh* paiMesh){
    m_Entries[Index].MaterialIndex = paiMesh->mMaterialIndex;

    std::vector<VertexMesh> Vertices;
    std::vector<unsigned int> Indices;

    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    for(unsigned int i = 0; i < paiMesh->mNumVertices; i++){
        const aiVector3D* pPos = &(paiMesh->mVertices[i]);
        const aiVector3D* pNormal = &(paiMesh->mNormals[i]);
        const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;
        const aiVector3D* pTangent = &(paiMesh->mTangents[i]);

        VertexMesh v(glm::fvec3(pPos->x, pPos->y, pPos->z),
            glm::fvec2(pTexCoord->x, pTexCoord->y),
            glm::fvec3(pNormal->x, pNormal->y, pNormal->z),
            glm::fvec3(pTangent->x, pTangent->y, pTangent->z));

        Vertices.push_back(v);
    }

    for(unsigned int i = 0; i < paiMesh->mNumFaces; i++){
        const aiFace& Face = paiMesh->mFaces[i];
        assert(Face.mNumIndices == 3);
        Indices.push_back(Face.mIndices[0]);
        Indices.push_back(Face.mIndices[1]);
        Indices.push_back(Face.mIndices[2]);
    }

    m_Entries[Index].Init(Vertices, Indices);
}

bool Mesh::InitMaterials(const aiScene* pScene, const std::string& Filename){
    // Extract the directory part from the file name
    std::string::size_type SlashIndex = Filename.find_last_of("/");
    std::string Dir;

    if(SlashIndex == std::string::npos){
        Dir = "C:\\";
    } else if(SlashIndex == 0){
        Dir = "C:\\";
    } else{
        Dir = Filename.substr(0, SlashIndex);
    }

    bool Ret = true;

    // Initialize the materials
    for(unsigned int i = 0; i < pScene->mNumMaterials; i++){
        const aiMaterial* pMaterial = pScene->mMaterials[i];

        m_Textures[i] = NULL;

        if(pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0){
            aiString Path;

            if(pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS){
                std::string FullPath = Dir + Path.data;
                m_Textures[i] = new TextureMesh(GL_TEXTURE_2D, FullPath.c_str());

                if(!m_Textures[i]->Load()){
                    printf("Error loading TextureMesh '%s'\n", FullPath.c_str());
                    delete m_Textures[i];
                    m_Textures[i] = NULL;
                    Ret = false;
                } else{
                    printf("Loaded TextureMesh '%s'\n", FullPath.c_str());
                }
            }
        }
    }

    return Ret;
}

void Mesh::Render(){
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    for(unsigned int i = 0; i < m_Entries.size(); i++){
        glBindBuffer(GL_ARRAY_BUFFER, m_Entries[i].VB);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexMesh), 0);                 // position
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexMesh), (const GLvoid*)12); // TextureMesh coordinate
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexMesh), (const GLvoid*)20); // normal
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexMesh), (const GLvoid*)32); // tangent

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Entries[i].IB);

        const unsigned int MaterialIndex = m_Entries[i].MaterialIndex;

        if(MaterialIndex < m_Textures.size() && m_Textures[MaterialIndex]){
            m_Textures[MaterialIndex]->Bind(COLOR_TEXTURE_UNIT);
        }

        glDrawElements(GL_TRIANGLES, m_Entries[i].NumIndices, GL_UNSIGNED_INT, 0);
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
}
