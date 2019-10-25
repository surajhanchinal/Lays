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
            fpCamera = new Camera(Eigen::Vector3f(1.34277,11.0314,-0.18954),Eigen::Vector3f(0,1,0),80,1920.0f/1022.0f,0.1f,1000.0f,-0.1,1.278);
            aff = Eigen::Affine3f::Identity();
            aff.prerotate(Eigen::AngleAxis<float>(-M_PI/2,Eigen::Vector3f(1,0,0)));
            rev_aff = Eigen::Affine3f::Identity();
            //rev_aff.prerotate(Eigen::AngleAxis<float>(-M_PI/2,Eigen::Vector3f(1,0,0)));
            rev_aff.translate(Eigen::Vector3f(0,5,2));
            aff.translate(Eigen::Vector3f(0,5,2));
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
            GLuint rev_aff_loc = glGetUniformLocation(shader, "rev_aff");
            auto itr = Loops.find(curr_animation);
            this->Move(deltatime,itr->second.getVelocity());
            glUniformMatrix4fv(Matrixm, 1, GL_FALSE, aff.matrix().data());
            glUniformMatrix4fv(rev_aff_loc, 1, GL_FALSE, rev_aff.inverse().matrix().data()); 
            //cout<<rev_aff.inverse().matrix()*aff.matrix()<<endl;
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
            aff.rotate(Eigen::AngleAxis<float>(dir*deltatime*angular_vel*(M_PI/180.0),Eigen::Vector3f(0,0,1)));
            aff.translate(deltatime*vel);
            rev_aff.rotate(Eigen::AngleAxis<float>(dir*deltatime*angular_vel*(M_PI/180.0),Eigen::Vector3f(0,0,1)));
            rev_aff.translate(deltatime*vel);
        }
        void Rotate(int dir){
            this->dir = dir;
        }
    Camera* fpCamera;
    Eigen::Affine3f rev_aff;
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
    float angular_vel =  90;
    int dir = 0;
};