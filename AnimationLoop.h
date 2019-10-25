#pragma once 
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <bits/stdc++.h>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
using namespace std;

class AnimationLoop{

public:
    AnimationLoop(string name,string path,vector<string> joints,int frame_start,float frameTime,Eigen::Vector3f vel){
        this->name = name;
        this->frameTime = frameTime;
        start_frame = frame_start;
        setVelocity(vel);
        cout<<"Loading Animation:  "<<name<<endl;
        ifstream file(path);
        file>>frames;
        for(int k=0;k<frames;k++){
            map<string,Eigen::Matrix4f> jointTFs;
            vector<Eigen::Matrix4f> Frame;
            for(int i=0;i<joints.size();i++){
                string jointname;
                file>>jointname;
                Eigen::Matrix4f jointTransform;
                for(int i=0;i<4;i++){
                    for(int j=0;j<4;j++){
                        float x;
                        file>>x;
                        jointTransform(i,j) = x;
                    }
                }
                jointTFs.insert(make_pair(jointname,jointTransform));
                //cout<<"jointname:   "<<jointname<<"  "<<mesh.joints[i]<<endl;
            }
            for(int i=0;i<joints.size();i++){
                auto itr = jointTFs.find(joints[i]);
                if(itr == jointTFs.end()){
                    cout<<joints[i]<<"  Not found at frame:   "<<k<<endl;
                    continue;
                }
                Frame.push_back(itr->second);
                //cout<<"found: "<<itr->first<<endl;
            }
        
            KeyFrames.push_back(Frame);
        }
    }

    vector<Eigen::Matrix4f> updateAnimation(float deltatime){
        if(!name.compare("REST") or !name.compare("JUMP")){
            return KeyFrames[0];
        }
        time += deltatime;
        float progression = fmod(time,frameTime)/frameTime;
        int frame = (int)(time/frameTime);
        if((frame+1+start_frame) >= frames){
            time -= frameTime*frame;
            frame = start_frame;
        }
        //cout<<frame<<"   time:    "<<time<<"   progression:  "<<progression<<endl;
        vector<Eigen::Matrix4f> result;
        vector<Eigen::Matrix4f> curr_frame = KeyFrames[frame+start_frame];
        vector<Eigen::Matrix4f> next_frame = KeyFrames[frame+1+start_frame];
        return interpolate(curr_frame,next_frame,progression);
    }

    vector<Eigen::Matrix4f> interpolate(vector<Eigen::Matrix4f> curr_frame,vector<Eigen::Matrix4f> next_frame,float progression){
        vector<Eigen::Matrix4f> result;
        for(int i=0;i<curr_frame.size();i++){
            Eigen::Matrix3f curr_rot = curr_frame[i].block<3,3>(0,0);
            Eigen::Matrix3f next_rot = next_frame[i].block<3,3>(0,0);
            Eigen::Vector3f curr_trans = curr_frame[i].block<3,1>(0,3);
            Eigen::Vector3f next_trans = next_frame[i].block<3,1>(0,3);
            Eigen::Quaternionf curr_q(curr_rot);
            curr_q.normalize();
            Eigen::Quaternionf next_q(next_rot);
            next_q.normalize();
            Eigen::Quaternionf slerp_q = curr_q.slerp(progression,next_q);
            Eigen::Vector3f lerp_trans = (1.0f-progression)*curr_trans + progression*next_trans;
            Eigen::Matrix4f result_matrix = Eigen::Matrix4f::Identity();
            result_matrix.block<3,3>(0,0) = slerp_q.toRotationMatrix();
            result_matrix.block<3,1>(0,3) = lerp_trans;
            result.push_back(result_matrix);
        }
        return result;
    }

    vector<Eigen::Matrix4f> transition(vector<Eigen::Matrix4f> start_frame,float delta_time,bool &done){
        vector<Eigen::Matrix4f> my_frame = updateAnimation(0);
        if((tt_time+delta_time) >= transTime){
            done = true;
        }
        else{
            tt_time += delta_time;
        }
        float progression = fmod(tt_time,transTime)/transTime;
        return interpolate(start_frame,my_frame,progression);
    }

    void setTime(float time){
        this->time = fmod(time,frameTime*(frames-start_frame));
    }
    float getTime(){
        return time;
    }

    void reset(){
        time  = 0;
    }
    void resetTransition(){
        tt_time  = 0;
    }
    
    void setVelocity(Eigen::Vector3f vel){
        Velocity = vel;
    }
    Eigen::Vector3f getVelocity(){
        return Velocity;
    }


    public:
        int frames;
        string name;
        vector<vector<Eigen::Matrix4f>> KeyFrames;
        int start_frame = 1;
        float time=0;
        float tt_time=0;
        float frameTime = 0.15;
        float transTime = 0.15;
        Eigen::Vector3f Velocity = Eigen::Vector3f(0,0,1);
};