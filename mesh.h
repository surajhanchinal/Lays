#pragma once 
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <bits/stdc++.h>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
using namespace std;
struct Vertex{
    Eigen::Vector3f position;
    Eigen::Vector3f normal;
    Eigen::Vector3f color;
    Eigen::Vector3f joint_indices;
    Eigen::Vector3f joint_weights;
};
struct Material{
    string name;
    Eigen::Vector3f color;
    vector<Vertex> vertices;
    uint texID;
};
class Mesh{
    public:
    vector<GLuint> VAOs;
    Mesh(){

    }
    void SetupMesh(){

        for(int i=0;i<materials.size();i++){
            Material mat = materials[i];
            GLuint VAO,VBO;
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glBindVertexArray(VAO);
            VAOs.push_back(VAO);
            // load data into vertex buffers
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            // A great thing about structs is that their memory layout is sequential for all its items.
            // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
            // again translates to 3/2 floats which translates to a byte array.
            
            glBufferData(GL_ARRAY_BUFFER, mat.vertices.size()*sizeof(Vertex), &mat.vertices[0], GL_STATIC_DRAW); 

            // set the vertex attribute pointers
            // vertex Positions
            glEnableVertexAttribArray(0);	
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
            // vertex normals
            glEnableVertexAttribArray(1);	
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
            // vertex texture coords
            glEnableVertexAttribArray(2);	
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, joint_indices));
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, joint_weights));

            glBindVertexArray(0);
        }
    }

    void Draw(GLuint ShaderID){
        for(int i=0;i<VAOs.size();i++){
            if(!thirdPerson){
                if((materials[i].name == "Shirt-material") ||(materials[i].name == "Pant-material") ||(materials[i].name == "Shoe-material") ||(materials[i].name == "Face-material") ||(materials[i].name == "Hair-material")){
                    continue;
                }
            }
            GLuint color = glGetUniformLocation(ShaderID, "color");
            glUniform3fv(color, 1, materials[i].color.data());
            GLuint vao = VAOs[i];
            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLES,0,materials[i].vertices.size());
            glBindVertexArray(0);
        }
    }
    bool thirdPerson = false;
    string name;
    vector<Eigen::Vector3f> positions;
    vector<Eigen::Vector3f> normals;
    vector<Eigen::Vector3f> colors;
    vector<Eigen::Vector2f> uvs;
    vector<Eigen::Vector3f> weights;
    vector<Eigen::Vector3f> indices;
    vector<Material> materials;
    vector<Eigen::Matrix4f> invBindMatrices;
    vector<string> joints;

};