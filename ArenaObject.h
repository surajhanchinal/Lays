#pragma once 
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <bits/stdc++.h>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include "parser.h"
#include "AnimationLoop.h"
#include "camera.h"
using namespace std;

class ArenaObject{

    public:
        ArenaObject(string meshname,string MeshPath){
            name = meshname;
            Parser parser(MeshPath);
            mesh = parser.getMesh();
            mesh.SetupMesh();
            
            model = Eigen::Matrix4f::Identity();
            //model.block<3,3>(0,0) = Eigen::AngleAxisf(-M_PI/2, Eigen::Vector3f::UnitX()).toRotationMatrix();
        }
        void setShader(GLuint shaderID)
        {
            shader = shaderID;
        }

        void Draw(Eigen::Matrix4f inverse_model){
            GLuint Matrixm = glGetUniformLocation(shader, "model");
            Eigen::Matrix4f new_model = inverse_model*model;
            glUniformMatrix4fv(Matrixm, 1, GL_FALSE, new_model.data()); 
            
            mesh.Draw(shader);
        }
    Mesh mesh;
    Eigen::Matrix4f model;
    private:
        string name;        
        GLuint shader;
        
        
};