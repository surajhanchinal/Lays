#pragma once
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <bits/stdc++.h>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
using namespace std;
class Bullet{

public:
    Bullet(GLuint bullet_vao,GLuint shader,GLuint texID){
        VAO = bullet_vao;
        this->shader = shader;
        this->texID = texID;
    }

    void setPosition(Eigen::Vector3f position,Eigen::Vector3f normal){
        in_use = true;
        Position = position;
        Normal = normal;
    }

    Eigen::Matrix4f makeRotationDir(Eigen::Vector3f direction, const Eigen::Vector3f& up = Eigen::Vector3f(0,1,0))
    {
        direction.normalize();
        if(direction.cwiseAbs().isApprox(up)){
            cout<<"hey hey hey"<<endl;
            Eigen::Matrix4f rot =  Eigen::Matrix4f::Identity();
            rot.block<3,3>(0,0) = Eigen::AngleAxisf(M_PI/2, Eigen::Vector3f::UnitX()).toRotationMatrix();
        }
        Eigen::Vector3f xaxis = up.cross(direction);
        xaxis.normalize();

        Eigen::Vector3f yaxis = direction.cross(xaxis);
        yaxis.normalize();
        Eigen::Matrix4f rot = Eigen::Matrix4f::Identity();
        rot(0,0)= xaxis.x();
        rot(1,0) = yaxis.x();
        rot(2,0) = direction.x();

        rot(0,1) = xaxis.y();
        rot(1,1) = yaxis.y();
        rot(2,1) = direction.y();

        rot(0,2) = xaxis.z();
        rot(1,2) = yaxis.z();
        rot(2,2) = direction.z();
        return rot;
    }

    void Draw(Eigen::Matrix4f out_model){
        if(in_use == true){
            //setUnifs(bullet_hdlr);
            GLuint bMatrixm = glGetUniformLocation(shader, "model");
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(glGetUniformLocation(shader, "texture1"), 0);
            glBindTexture(shader,texID);
            Eigen::Matrix4f m = Eigen::Matrix4f::Identity();
            m.block<3,3>(0,0)=1*m.block<3,3>(0,0);
            m.block<3,1>(0,3) = Position;
            m = out_model.inverse()*m;

            //Eigen::Matrix4f rot = makeRotationDir(Normal);
            //cout<<rot<<endl;
            //rot.block<3,1>(0,3) = rot.block<3,3>(0,0)*Eigen::Vector3f(-0.03,0,0);
            //m = m*rot;
            if(abs(Normal.x()) == 1){
                Eigen::Matrix4f rot = Eigen::Matrix4f::Identity();
                rot.block<3,1>(0,3) = Normal.x()*Eigen::Vector3f(0.03,0,0);
                rot.block<3,3>(0,0) = Eigen::AngleAxisf(-M_PI/2, Eigen::Vector3f::UnitY()).toRotationMatrix();
                m = m*rot;
            }
            else if(abs(Normal.y()) == 1){
                Eigen::Matrix4f rot = Eigen::Matrix4f::Identity();
                rot.block<3,1>(0,3) = Normal.y()*Eigen::Vector3f(0,0.03,0);
                rot.block<3,3>(0,0) = Eigen::AngleAxisf(-M_PI/2, Eigen::Vector3f::UnitX()).toRotationMatrix();
                m = m*rot;
            }
            else if(abs(Normal.z()) == 1){
                Eigen::Matrix4f rot = Eigen::Matrix4f::Identity();
                rot.block<3,1>(0,3) = Normal.z()*Eigen::Vector3f(0,0,0.03);
                m = m*rot;
            }
            glUniformMatrix4fv(bMatrixm, 1, GL_FALSE, m.data());
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES,36,6);
        }
    }

private:
    bool in_use = false;
    Eigen::Vector3f Position;
    Eigen::Vector3f Normal;
    GLuint VAO,shader,texID;

};
