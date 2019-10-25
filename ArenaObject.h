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
            
            Eigen::Affine3f aff = Eigen::Affine3f::Identity();
            aff.prerotate(Eigen::AngleAxis<float>(-M_PI/2,Eigen::Vector3f(1,0,0)));
            model = aff.matrix();
        }
        void setShader(GLuint shaderID)
        {
            shader = shaderID;
        }

        void Draw(){
            GLuint Matrixm = glGetUniformLocation(shader, "model");
            glUniformMatrix4fv(Matrixm, 1, GL_FALSE, model.data()); 
            mesh.Draw(shader);
        }

    private:
        Mesh mesh;
        string name;        
        GLuint shader;
        Eigen::Matrix4f model;
        
        
};