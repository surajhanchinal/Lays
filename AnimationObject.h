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

class AnimationObject{

    public:
        AnimationObject(string meshname,string MeshPath){
            name = meshname;
            Parser parser(MeshPath);
            mesh = parser.parseMesh();
            mesh.SetupMesh();
            fpCamera = new Camera(Eigen::Vector3f(1.47,10.7515+4,0.1949),Eigen::Vector3f(0,1,0),45,1920.0f/1022.0f,0.1f,1000.0f,0,1.278);
            model = Eigen::Matrix4f::Identity();
            model.block<3,3>(0,0) = Eigen::AngleAxisf(-M_PI/2, Eigen::Vector3f::UnitX()).toRotationMatrix();
            model.block<3,1>(0,3) = Eigen::Vector3f(0,4,0);
        }
        void addAnimation(string name,string path,int start_frame,float frameTime,Eigen::Vector3f vel){
            Loops.insert(make_pair(name,AnimationLoop(name,path,mesh.joints,start_frame,frameTime,vel)));
        }

        void setAnimation(string name){
            if(!jump_interrupt){
                if(!name.compare("JUMP")){
                    jump_interrupt = true;
                    transition_matrix = lerp_matrix;
                    return;
                }

                if(name.compare(curr_animation)){
                    if(name.compare(next_animation)){
                        next_animation = name;
                        auto itr = Loops.find(curr_animation);
                        auto next_itr = Loops.find(next_animation);
                        next_itr->second.resetTransition();
                        next_itr->second.setTime(itr->second.getTime());
                        transition_matrix = lerp_matrix;
                        in_transition = true;
                    }
                }
            }
        }
        void updateAnimation(float deltatime){
            if(!jump_interrupt){
                if(in_transition){
                    auto itr = Loops.find(next_animation);
                    bool done = false;
                    lerp_matrix = itr->second.transition(transition_matrix,deltatime,done);
                    if(done){
                        itr->second.resetTransition();
                        in_transition = false;
                        curr_animation = next_animation;
                    }
                }  
                else{ 
                    auto itr = Loops.find(curr_animation);
                    lerp_matrix = itr->second.updateAnimation(deltatime);
                }
            }
            else if(jump_interrupt){
                lerp_matrix = jumpHandler(deltatime);
            }
            vector<Eigen::Matrix4f> final_matrix;
            for(int i=0;i<lerp_matrix.size();i++){
                final_matrix.push_back(lerp_matrix[i]*mesh.invBindMatrices[i]);
            }
            GLuint jtmt = glGetUniformLocation(shader, "jointMatrices");
            glUniformMatrix4fv(jtmt,final_matrix.size(),GL_FALSE,reinterpret_cast<GLfloat *>(final_matrix.data()));
            GLuint Matrixm = glGetUniformLocation(shader, "model");
            auto itr = Loops.find(curr_animation);
            this->Move(deltatime,itr->second.getVelocity());
            glUniformMatrix4fv(Matrixm, 1, GL_FALSE, model.data());
            mesh.Draw(shader);
        }

        vector<Eigen::Matrix4f> jumpHandler(float deltatime){
            float acc = -9.8;
            float vel = sqrt(-2*acc);
            jump_time += deltatime;
            float progression = vel*jump_time + (acc*jump_time*jump_time)/2;
            if(progression > 1){
                progression = 1;
            }
            if(progression < 0){
                progression = 0;
                jump_interrupt = false;
                jump_time = 0;
            }
            auto itr = Loops.find("JUMP");
            return itr->second.interpolate(transition_matrix,itr->second.updateAnimation(0),progression);
        }

        void setShader(GLuint shaderID)
        {
            shader = shaderID;
        }
        void Move(float deltatime,Eigen::Vector3f vel){
            out_model.block<3,1>(0,3) += out_model.block<3,3>(0,0)*(deltatime*vel);
        }
        void Rotate(float theta){
            out_model.block<3,3>(0,0) = Eigen::AngleAxis<float>(theta,Eigen::Vector3f(0,1,0)).toRotationMatrix()*out_model.block<3,3>(0,0);
        }
    Camera* fpCamera;
    Eigen::Matrix4f out_model = Eigen::Matrix4f::Identity();
    private:
    Mesh mesh;

    string name;
    vector<Eigen::Matrix4f> lerp_matrix,transition_matrix;
    
    map<string,AnimationLoop> Loops;
    string curr_animation="RUN";
    string next_animation="RUN";
    bool in_transition = false;
    bool jump_interrupt = false;
    float jump_time;
    GLuint shader;
    Eigen::Affine3f aff;
    Eigen::Matrix4f model;
    float angular_vel =  120;
    float theta = 0;
};