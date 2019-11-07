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
#include "ArenaObject.h"
#include "Bullet.h"
using namespace std;

class AnimationObject{

    public:
        AnimationObject(string meshname,string MeshPath){
            name = meshname;
            Parser parser(MeshPath);
            mesh = parser.parseMesh();
            mesh.SetupMesh();
            fpCamera = new Camera(Eigen::Vector3f(0.47,10.7515,0.1949+1.0),Eigen::Vector3f(0,1,0),45,1920.0f/1022.0f,0.1f,1000.0f,0,M_PI/2);
            model = Eigen::Matrix4f::Identity();
            model.block<3,3>(0,0) = Eigen::AngleAxisf(-M_PI/2, Eigen::Vector3f::UnitX()).toRotationMatrix();
            model.block<3,1>(0,3) = Eigen::Vector3f(0,0,0);
            out_model.block<3,1>(0,3) = Eigen::Vector3f(50,6,0);
            initHitBoxes();
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
            final_matrix.clear();
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


        void updateAnimation(float deltatime,Eigen::Matrix4f tpmatrix){
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
            final_matrix.clear();
            for(int i=0;i<lerp_matrix.size();i++){
                final_matrix.push_back(lerp_matrix[i]*mesh.invBindMatrices[i]);
            }
            GLuint jtmt = glGetUniformLocation(shader, "jointMatrices");
            glUniformMatrix4fv(jtmt,final_matrix.size(),GL_FALSE,reinterpret_cast<GLfloat *>(final_matrix.data()));
            GLuint Matrixm = glGetUniformLocation(shader, "model");
            auto itr = Loops.find(curr_animation);
            this->Move(deltatime,itr->second.getVelocity());
            Eigen::Matrix4f model2 = Eigen::Matrix4f::Identity();
            model2.block<3,1>(0,3) = Eigen::Vector3f(50,6,0);
            Eigen::Matrix4f new_model = tpmatrix*(out_model*model);
            glUniformMatrix4fv(Matrixm, 1, GL_FALSE, new_model.data());
            mesh.Draw(shader);
        }

        vector<Eigen::Matrix4f> jumpHandler(float deltatime){
            float vel = sqrt(-2*acc);
            jump_time += deltatime;
            float progression = vel*jump_time + (acc*jump_time*jump_time)/2;

            jumpVelocity.y() += acc*deltatime;
            Eigen::Vector3f displacement = jumpVelocity*deltatime;
            this->MoveDisplacement(20*displacement);
            if(progression > 1){
                progression = 1;
            }
            if(progression < 0){
                progression = 0;
                jump_interrupt = false;
                jump_time = 0;
                jumpVelocity = Eigen::Vector3f(0,sqrt(-2*acc*height),0);
            }
            auto itr = Loops.find("JUMP");
            
            return itr->second.interpolate(transition_matrix,itr->second.updateAnimation(0),progression);
        }

        void setShader(GLuint shaderID)
        {
            shader = shaderID;
        }
        void setArena(ArenaObject* arena){
            this->arena = arena;
        }
        void Move(float deltatime,Eigen::Vector3f vel){
            Eigen::Vector3i stop = checkCollision(arena);
            Eigen::Vector3f rot_disp = Eigen::AngleAxis<float>(theta_x,Eigen::Vector3f(0,1,0)).toRotationMatrix()*(deltatime*vel);
            for(int i=0;i<3;i++){
                if(stop[i] == 1){
                    float val = rot_disp[i];
                    rot_disp[i] = max(0.0f,val);
                }
                else if(stop[i] == 2){
                    float val = rot_disp[i];
                    rot_disp[i] = min(0.0f,val);
                }
            }
            out_model.block<3,3>(0,0) = (Eigen::AngleAxis<float>(theta_x+recoilAngle_x,Eigen::Vector3f(0,1,0))*Eigen::AngleAxis<float>(theta_y+recoilAngle_y,Eigen::Vector3f(1,0,0))).toRotationMatrix();
            out_model.block<3,1>(0,3) += rot_disp;
        }
        void MoveDisplacement(Eigen::Vector3f displ){
            out_model.block<3,1>(0,3) += Eigen::AngleAxis<float>(theta_x,Eigen::Vector3f(0,1,0)).toRotationMatrix()*(displ);
            if(out_model(1,3) < 6){
                out_model(1,3) = 6;
            }
        }
        void Rotate(float deltheta_x,float deltheta_y){
            theta_x += deltheta_x;
            theta_y += deltheta_y;
            if(abs(theta_y) > M_PI/6.0f){
                theta_y = (abs(theta_y)/theta_y)*(M_PI/6.0f);
            }
        }

    Eigen::Vector3f getPosition(){
        Eigen::Vector3f position = out_model.block<3,1>(0,3);
        return position;
    }
    bool inRange(float minlim,float maxlim,float arg){
        if((arg>minlim) && (arg<maxlim)){
            return true;
        }
        return false;
    }
    Eigen::Vector3i checkCollision(ArenaObject* arena){
        Eigen::Vector3f position = getPosition();
        Eigen::Vector3i stop = Eigen::Vector3i(0,0,0);
        for(int i=0;i<arena->mesh.materials.size();i++){
        for(int j=0;j<arena->mesh.materials[i].vertices.size()-2;j+=3){
            //Eigen::Matrix3f rot = arena->model.block<3,3>(0,0); 
            Vertex v1 = arena->mesh.materials[i].vertices[j];
            Vertex v2 = arena->mesh.materials[i].vertices[j+1];
            Vertex v3 = arena->mesh.materials[i].vertices[j+2];
            float max_z = max(max(v1.position[2],v2.position[2]),v3.position[2]);
            float max_x = max(max(v1.position[0],v2.position[0]),v3.position[0]);
            float min_z = min(min(v1.position[2],v2.position[2]),v3.position[2]);
            float min_x = min(min(v1.position[0],v2.position[0]),v3.position[0]);
            float my_max_x = position[0] + 3;
            float my_min_x = position[0] - 3;
            float my_max_z = position[2] + 3;
            float my_min_z = position[2] - 3;
            float my_max_y = position[1] + 7;
            float my_min_y = position[1] - 7;
            Eigen::Vector3f avg_position = (v1.position+v2.position+v3.position)/3.0f;
            Eigen::Vector3f avg_normal = (v1.normal+v2.normal+v3.normal)/3.0f;
            if(abs(avg_normal.dot(Eigen::Vector3f(0,1,0))) == 1){
                continue;
                bool is_y_lim = inRange(my_min_y,my_max_y,avg_position.y());
                if(is_y_lim){
                    if(avg_normal.sum() > 0){
                        stop[1] = 1;
                    }
                    else
                    stop[1] = 2;
                }
            }
            
            if(abs(avg_normal.dot(Eigen::Vector3f(1,0,0))) == 1){
                bool is_x_lim = inRange(my_min_x,my_max_x,avg_position.x());
                bool z_min = inRange(min_z,max_z,my_min_z);
                bool z_max = inRange(min_z,max_z,my_max_z);
                if(is_x_lim && z_min && z_max){
                    if(avg_normal.sum() > 0){
                        stop[0] = 1;
                    }
                    else
                    stop[0] = 2;
                }

            }

            if(abs(avg_normal.dot(Eigen::Vector3f(0,0,1))) == 1){
                bool is_z_lim = inRange(my_min_z,my_max_z,avg_position.z());
                bool x_min = inRange(min_x,max_x,my_min_x);
                bool x_max = inRange(min_x,max_x,my_max_x);
                if(is_z_lim && x_min && x_max){
                    if(avg_normal.sum() > 0){
                        stop[2] = 1;
                    }
                    else
                    stop[2] = 2;
                }
            }
        }
            
        }
        return stop;
    }

    bool RayIntersectsTriangle(Eigen::Vector3f rayOrigin, 
                           Eigen::Vector3f rayVector, 
                           vector<Eigen::Vector3f> Triangle,
                           Eigen::Vector3f& outIntersectionPoint,float& t_out)
{
    const float EPSILON = 0.0000001;
    Eigen::Vector3f vertex0 = Triangle[0];
    Eigen::Vector3f vertex1 = Triangle[1];  
    Eigen::Vector3f vertex2 = Triangle[2];
    Eigen::Vector3f edge1, edge2, h, s, q;
    float a,f,u,v;
    edge1 = vertex1 - vertex0;
    edge2 = vertex2 - vertex0;
    h = rayVector.cross(edge2);
    a = edge1.dot(h);
    if (a > -EPSILON && a < EPSILON)
        return false;    // This ray is parallel to this triangle.
    f = 1.0/a;
    s = rayOrigin - vertex0;
    u = f * s.dot(h);
    if (u < 0.0 || u > 1.0)
        return false;
    q = s.cross(edge1);
    v = f * rayVector.dot(q);
    if (v < 0.0 || u + v > 1.0)
        return false;
    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * edge2.dot(q);
    //cout<<"hello:  "<<t<<endl;
    if (t > EPSILON && t < 1/EPSILON) // ray intersection
    {
        outIntersectionPoint = rayOrigin + rayVector * t;
        t_out = t;
        return true;
    }
    else // This means that there is a line intersection but not a ray intersection.
        return false;
}


    bool RayCast(Eigen::Vector3f& intersectionPoint,Eigen::Vector3f& intersectionNormal,float& t){
    
        Eigen::Vector3f Ray = Eigen::Vector3f(0,0,-1);
        Eigen::Matrix4f new_Camera = Eigen::Matrix4f::Identity();
        new_Camera.block<3,1>(0,3) = -fpCamera->getViewMatrix().block<3,1>(0,3);
        Eigen::Matrix4f ReverseMatrix = out_model*new_Camera;
        Eigen::Vector3f RayDir = ReverseMatrix.block<3,3>(0,0)*Ray;
        Eigen::Vector3f RayPos = ReverseMatrix.block<3,1>(0,3);
        bool out = false;
        float mint = 10000000;
        for(int i=0;i<arena->mesh.materials.size();i++){
            for(int j=0;j<arena->mesh.materials[i].vertices.size()-2;j+=3){
                Eigen::Vector3f outIntersectionPoint = Eigen::Vector3f(0,0,0);
                vector<Eigen::Vector3f> triangle;
                triangle.push_back(arena->mesh.materials[i].vertices[j].position);
                triangle.push_back(arena->mesh.materials[i].vertices[j+1].position);
                triangle.push_back(arena->mesh.materials[i].vertices[j+2].position);
                bool inter = RayIntersectsTriangle(RayPos,RayDir,triangle,outIntersectionPoint,t);
                if(inter == true){
                    if(t<mint){
                        out = true;
                        mint = t;
                        intersectionPoint = outIntersectionPoint;
                        intersectionNormal = arena->mesh.materials[i].vertices[j].normal;
                    }
                }
            }
        }
        t = mint;
        return out;
    }

    bool enemyRayCast(float& t){
        Eigen::Vector3f Ray = Eigen::Vector3f(0,0,-1);
        Eigen::Matrix4f new_Camera = Eigen::Matrix4f::Identity();
        new_Camera.block<3,1>(0,3) = -fpCamera->getViewMatrix().block<3,1>(0,3);
        Eigen::Matrix4f ReverseMatrix = out_model*new_Camera;
        Eigen::Vector3f RayDir = ReverseMatrix.block<3,3>(0,0)*Ray;
        Eigen::Vector3f RayPos = ReverseMatrix.block<3,1>(0,3);
        bool out = false;
        float mint = 10000000;
        for(int i=0;i<hitIndexs.size();i++){
            if(enemy->lerp_matrix.size() == 0){
                return false;
            }
            Eigen::Matrix4f trMatrix = enemy->out_model*enemy->model*enemy->lerp_matrix[hitIndexs[i]]*hitBoxes[i];
            for(int k=0;k<36;k+=3){
                vector<Eigen::Vector3f> triangle;
                for(int j=0;j<3;j++){
                    Eigen::Vector4f pos = Eigen::Vector4f(hitVertices[k+j].x(),hitVertices[k+j].y(),hitVertices[k+j].z(),1);
                    Eigen::Vector3f new_pos = (trMatrix*pos).head<3>();
                    triangle.push_back(new_pos);
                    //cout<<new_pos<<endl<<endl;
                }
                Eigen::Vector3f intersectionPoint;
                bool inter = RayIntersectsTriangle(RayPos,RayDir,triangle,intersectionPoint,t);
                if(inter){
                    if(t<mint){
                        out = true;
                        mint = t;
                    }
                }
            }
        }
        t = mint;
        return out;
    }

    void startFire(){
        firing = true;
    }

    void Fire(float deltatime){
        if(firing){
        if(bulletTime >= (1.0f/bps)*bno){
            putBullet();
            bulletsFired += 1;
            recoilAngle_y += 0.7*(M_PI/180.0f);
            recoilAngle_y = fmod(recoilAngle_y,2*M_PI);
            if(bulletsFired >= 10){
                recoilAngle_x += 0.3*(M_PI/180.0f);
                recoilAngle_x = fmod(recoilAngle_x,2*M_PI);
            }
            bno += 1;
            if(bno == bps){
                bno = 0;
                bulletTime -= 1;
            }
        }
        bulletTime += deltatime;
        }
        else{
            bno=0;
            bulletTime = 0;
            bulletsFired = 0;
            theta_x += recoilAngle_x;
            theta_y += recoilAngle_y;
            recoilAngle_y = 0;
            recoilAngle_x = 0;
        }
    }
    void stopFire(){
        firing = false;
    }

    void initBullets(int poolCount,GLuint shader,GLuint vao,GLuint bulletID,int bps){
        this->poolCount = poolCount;
        this->bps = bps;
        for(int i=0;i<poolCount;i++){
            Bullet bullet(vao,shader,bulletID);
            bullets.push_back(bullet);
        }
    }
    void putBullet(){
        Eigen::Vector3f pos;
        Eigen::Vector3f nor;
        float t1,t2;
        bool enemy = enemyRayCast(t1);
        bool out = RayCast(pos,nor,t2);
        if(enemy){
            if(t1 < t2){
                enemyHitCall();
                return;
            }
        }
        if(out){
            bullets[bullet_idx].setPosition(pos,nor);
            bullet_idx = (bullet_idx+1)%this->poolCount;
        }
    }
    void enemyHitCall(){
        cout<<"Enemy hit"<<endl;
    }
    void DrawBullets(){
        for(int i=0;i<poolCount;i++){
            bullets[i].Draw(this->out_model);
        }
    }

    void setThirdPerson(){
        mesh.thirdPerson = true;
    }
    void initEnemy(AnimationObject* object){
        this->enemy = object;
    }


void initHitBoxes(){
    Eigen::Matrix4f m;    
    int i=0;
    //BODY
    i=0;
    hitIndexs.push_back(i);
    m << 3,0,0,0,
    0,4.5,0,1.5,
    0,0,1,0,
    0,0,0,1;
    hitBoxes.push_back(m);
    //SHIN
    i=19;
    hitIndexs.push_back(i);
    i=24;
    hitIndexs.push_back(i);
    m << 0.8,0,0,0,
    0,3.9,0,1.3,
    0,0,1.1,0,
    0,0,0,1;
    hitBoxes.push_back(m);
    hitBoxes.push_back(m);
    //THIGH
    i=18;
    hitIndexs.push_back(i);
    i=23;
    hitIndexs.push_back(i);
    m << 1,0,0,0,
    0,3.8,0,2.2,
    0,0,0.9,0,
    0,0,0,1;
    hitBoxes.push_back(m);
    hitBoxes.push_back(m);
    //FACE
    i=3;
    hitIndexs.push_back(i);
    m << 1.4,0,0,0,
    0,2.4,0,0.2,
    0,0,1.3,-0.3,
    0,0,0,1;
    hitBoxes.push_back(m);
    //U_ARM
    i=4;
    hitIndexs.push_back(i);
    i=11;
    hitIndexs.push_back(i);
    m << 0.7,0,0,0,
    0,3,0,1.3,
    0,0,0.7,0,
    0,0,0,1;
    hitBoxes.push_back(m);
    hitBoxes.push_back(m);
    //L_ARM
    i=5;
    hitIndexs.push_back(i);
    i=12;
    hitIndexs.push_back(i);
    m << 0.5,0,0,0,
    0,2.4,0,1,
    0,0,0.6,0,
    0,0,0,1;
    hitBoxes.push_back(m);
    hitBoxes.push_back(m);
    //SHOE
    i=20;
    hitIndexs.push_back(i);
    i=25;
    hitIndexs.push_back(i);
    m << 2.2,0,0,-1.0,
    0,1,0,0.2,
    0,0,0.6,0.0,
    0,0,0,1;
    hitBoxes.push_back(m);
    hitBoxes.push_back(m);
    hitVertices.push_back(Eigen::Vector3f(-0.5f, -0.5f, -0.5f));
    hitVertices.push_back(Eigen::Vector3f(0.5f, -0.5f, -0.5f));
    hitVertices.push_back(Eigen::Vector3f(0.5f,  0.5f, -0.5f));
    hitVertices.push_back(Eigen::Vector3f(0.5f,  0.5f, -0.5f));
    hitVertices.push_back(Eigen::Vector3f(-0.5f,  0.5f, -0.5f));
    hitVertices.push_back(Eigen::Vector3f(-0.5f, -0.5f, -0.5f));
    hitVertices.push_back(Eigen::Vector3f(-0.5f, -0.5f,  0.5f));
    hitVertices.push_back(Eigen::Vector3f(0.5f, -0.5f,  0.5f));
    hitVertices.push_back(Eigen::Vector3f(0.5f,  0.5f,  0.5f));
    hitVertices.push_back(Eigen::Vector3f(0.5f,  0.5f,  0.5f));
    hitVertices.push_back(Eigen::Vector3f(-0.5f,  0.5f,  0.5f));
    hitVertices.push_back(Eigen::Vector3f(-0.5f, -0.5f,  0.5f));
    hitVertices.push_back(Eigen::Vector3f(-0.5f,  0.5f,  0.5f));
    hitVertices.push_back(Eigen::Vector3f(-0.5f,  0.5f, -0.5f));
    hitVertices.push_back(Eigen::Vector3f(-0.5f, -0.5f, -0.5f));
    hitVertices.push_back(Eigen::Vector3f(-0.5f, -0.5f, -0.5f));
    hitVertices.push_back(Eigen::Vector3f(-0.5f, -0.5f,  0.5f));
    hitVertices.push_back(Eigen::Vector3f(-0.5f,  0.5f,  0.5f));
    hitVertices.push_back(Eigen::Vector3f(0.5f,  0.5f,  0.5f));
    hitVertices.push_back(Eigen::Vector3f(0.5f,  0.5f, -0.5f));
    hitVertices.push_back(Eigen::Vector3f(0.5f, -0.5f, -0.5f));
    hitVertices.push_back(Eigen::Vector3f(0.5f, -0.5f, -0.5f));
    hitVertices.push_back(Eigen::Vector3f(0.5f, -0.5f,  0.5f));
    hitVertices.push_back(Eigen::Vector3f(0.5f,  0.5f,  0.5f));
    hitVertices.push_back(Eigen::Vector3f(-0.5f, -0.5f, -0.5f));
    hitVertices.push_back(Eigen::Vector3f(0.5f, -0.5f, -0.5f));
    hitVertices.push_back(Eigen::Vector3f(0.5f, -0.5f,  0.5f));
    hitVertices.push_back(Eigen::Vector3f(0.5f, -0.5f,  0.5f));
    hitVertices.push_back(Eigen::Vector3f(-0.5f, -0.5f,  0.5f));
    hitVertices.push_back(Eigen::Vector3f(-0.5f, -0.5f, -0.5f));
    hitVertices.push_back(Eigen::Vector3f(-0.5f,  0.5f, -0.5f));
    hitVertices.push_back(Eigen::Vector3f(0.5f,  0.5f, -0.5f));
    hitVertices.push_back(Eigen::Vector3f(0.5f,  0.5f,  0.5f));
    hitVertices.push_back(Eigen::Vector3f(0.5f,  0.5f,  0.5f));
    hitVertices.push_back(Eigen::Vector3f(-0.5f,  0.5f,  0.5f));
    hitVertices.push_back(Eigen::Vector3f(-0.5f,  0.5f, -0.5f));
}
    vector<Eigen::Vector3f> hitVertices;
    vector<Eigen::Matrix4f> hitBoxes;
    vector<int> hitIndexs;
    vector<Eigen::Matrix4f> final_matrix;
    vector<Eigen::Matrix4f> lerp_matrix,transition_matrix;
    Camera* fpCamera;
    Eigen::Matrix4f model;
    Eigen::Matrix4f out_model = Eigen::Matrix4f::Identity();
    private:
    Mesh mesh;
    ArenaObject* arena;
    string name;
    AnimationObject *enemy;
    
    map<string,AnimationLoop> Loops;
    string curr_animation="RUN";
    string next_animation="RUN";
    bool in_transition = false;
    bool jump_interrupt = false;
    float jump_time;
    GLuint shader;
    Eigen::Affine3f aff;
    float angular_vel =  120;
    float theta_x = 0;
    float theta_y = 0;
    float acc = -9.8;
    float real_acc = -5;
    float height = 1;
    vector<Bullet> bullets;
    int bullet_idx = 0;
    int poolCount;
    int bps;
    int bno = 0;
    bool firing = false;
    float bulletTime = 0;
    float recoilAngle_x = 0;
    float recoilAngle_y = 0;
    int bulletsFired = 0;
    Eigen::Vector3f jumpVelocity = Eigen::Vector3f(0,sqrt(-2*acc*height),0);
};