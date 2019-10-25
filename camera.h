#pragma once 
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>

class Camera{

public:

    Camera(const Eigen::Vector3f in_position,const Eigen::Vector3f in_up,float in_fovY, float in_aspect, float in_near, float in_far,float phi,float theta){
        position = in_position;
        up = in_up;
        fovY = in_fovY;
        aspect = in_aspect;
        near = in_near;
        far = in_far;
        this->phi = phi;
        this->theta = theta;
          
        generateLookAt();
        setPerspective();
    }

void generateLookAt()
{
  for(int i=0;i<4;i++){
      for(int j = 0;j<4;j++){
          mViewMatrix(i,j) = 0;
      }
  }
  Eigen::Vector3f Direction = Eigen::Vector3f(1*cos(phi)*cos(theta),1*sin(phi),1*cos(phi)*sin(theta));
  R.col(2) = Direction.normalized();
  if(R.col(2).cwiseAbs() == up.cwiseAbs()){
      R.col(2) = Eigen::Vector3f(0,0,1);
  }
  R.col(0) = up.cross(R.col(2)).normalized();
  R.col(1) = R.col(2).cross(R.col(0));
  mViewMatrix.topLeftCorner<3,3>() = R.transpose();
  mViewMatrix.topRightCorner<3,1>() = -R.transpose() * position;
  mViewMatrix(3,3) = 1.0f;
}



void setPerspective()
{
  float theta = fovY*(M_PI/180.0f)*0.5;
  float range = far - near;
  float invtan = 1./tan(theta);

  for(int i=0;i<4;i++){
      for(int j = 0;j<4;j++){
          mProjectionMatrix(i,j) = 0;
      }
  }
  
  mProjectionMatrix(0,0) = invtan / aspect;
  mProjectionMatrix(1,1) = invtan;
  mProjectionMatrix(2,2) = -(near + far) / range;
  mProjectionMatrix(3,2) = -1;
  mProjectionMatrix(2,3) = -2 * near * far / range;
  mProjectionMatrix(3,3) = 0;


}

void rotateCamera(float inc_phi,float inc_theta){
    phi += inc_phi;
    if(phi > 89.5)
        phi = 89.5;
    if(phi < -89.5)
        phi = -89.5;
    theta += inc_theta;
    theta = fmod(theta,2*M_PI);
    generateLookAt();
}

void translateCamera(float x,float y,float z){
    position += R.col(2)*x;
    position += R.col(0)*y;
    position += R.col(1)*z;
    generateLookAt();
}

void moveCamera(float x,float y,float z){
    position += Eigen::Vector3f(x,y,z);
    generateLookAt();
}

Eigen::Vector3f& getEyePosition(){
    return position;
}

Eigen::Matrix4f& getViewMatrix(){
    return mViewMatrix;
}

Eigen::Matrix4f& getProjectionMatrix(){
    return mProjectionMatrix;
}


void setModel(Eigen::Matrix4f model){
    modelMatrix = model;
}

void updateAspect(float in_aspect){
    aspect = in_aspect;
    setPerspective();
}
float theta,phi;
private:
    Eigen::Matrix4f mProjectionMatrix;
    Eigen::Matrix4f mViewMatrix;
    Eigen::Matrix4f modelMatrix;
    Eigen::Matrix4f LightMatrix;
    Eigen::Vector3f position;
    Eigen::Vector3f up;
    float fovY;
    float aspect;
    float near;
    float far;
    
    Eigen::Matrix3f R;


};