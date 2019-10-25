#pragma once 
#include <bits/stdc++.h>
#include "rapidxml-1.13/rapidxml.hpp"
#include "rapidxml-1.13/rapidxml_utils.hpp"
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include "mesh.h"
using namespace std;
class Parser{

public:
    Parser(string in_path){
        path = in_path;
    }


Mesh parseMesh(){
    ParseGeometry();
    ParseSkin();
    MakeTriangles();
    return mesh;    
}

Mesh getMesh(){
    ParseGeometry();
    MakeTriangles();
    return mesh;    
}

void ParseGeometry() {
    

    rapidxml::file<> xmlFile(path.c_str());
    rapidxml::xml_document<> doc;
    doc.parse<0>(xmlFile.data());    // 0 means default parse flags
    rapidxml::xml_node<> *root = doc.first_node();
    cout<<"root: "<<root->name()<<endl;
    rapidxml::xml_node<> *geometries = root->first_node("library_geometries");
    rapidxml::xml_node<> * geometry_node = geometries->first_node("geometry");

    
    rapidxml::xml_node<> *mesh_node = geometry_node->first_node();
    mesh.name = geometry_node->first_attribute("id")->value();
    cout<<"Mesh Name: "<<mesh.name<<endl;
        
        
        
        for (rapidxml::xml_node<> * source_node = mesh_node->first_node("source"); source_node; source_node = source_node->next_sibling()){
            if(strcmp(source_node->name(),"source"))
                break;
            string source_name = source_node->first_attribute("id")->value();
            source_name = source_name.substr(mesh.name.size()+1);
            
            cout<<"Source Name: "<<source_name<<endl;
            
            rapidxml::xml_node<> *data_node = source_node->first_node();            
            if(!source_name.compare("positions")){
                istringstream iss(data_node->value());
                int count = atoi(data_node->first_attribute("count")->value());
                for(int i=0;i<count/3;i++){
                    float x,y,z;
                    iss>>x>>y>>z;
                    mesh.positions.push_back(Eigen::Vector3f(x,y,z));
                }
            }
            else if(!source_name.compare("normals")){
                istringstream iss(data_node->value());
                int count = atoi(data_node->first_attribute("count")->value());
                for(int i=0;i<count/3;i++){
                    float x,y,z;
                    iss>>x>>y>>z;
                    mesh.normals.push_back(Eigen::Vector3f(x,y,z));
                }
            }
            else if(!source_name.compare("map-0")){
                istringstream iss(data_node->value());
                int count = atoi(data_node->first_attribute("count")->value());
                for(int i=0;i<count/2;i++){
                    float s,t;
                    iss>>s>>t;
                    mesh.uvs.push_back(Eigen::Vector2f(s,t));
                }
            }
            else if(!source_name.compare("colors-Col")){
                istringstream iss(data_node->value());
                int count = atoi(data_node->first_attribute("count")->value());
                for(int i=0;i<count/3;i++){
                    float r,g,b;
                    iss>>r>>g>>b;
                    mesh.colors.push_back(Eigen::Vector3f(r,g,b));
                }
            }
        }


}

void ParseSkin(){
    ///  joint weights and indices
    rapidxml::file<> xmlFile(path.c_str());
    rapidxml::xml_document<> doc;
    doc.parse<0>(xmlFile.data());    // 0 means default parse flags
    rapidxml::xml_node<> *root = doc.first_node();
    rapidxml::xml_node<> *joint_node = root->first_node("library_controllers");
    rapidxml::xml_node<> *skin_node = joint_node->first_node()->first_node();
    rapidxml::xml_node<> * source_node = skin_node->first_node("source");
    int joint_count = atoi(source_node->first_node()->first_attribute("count")->value());
    string names = source_node->first_node()->value();
    istringstream issjointname(names);
    source_node = source_node->next_sibling();
    string matrixes = source_node->first_node()->value();
    istringstream matrices(matrixes);
    

    string jointname;
    
    while(issjointname >> jointname){
        Eigen::Matrix4f jointTransform;
        cout<<jointname<<endl;
        for(int i=0;i<4;i++){
            for(int j=0;j<4;j++){
                float x;
                matrices>>x;
                jointTransform(i,j) = x;
            }
        }
        mesh.invBindMatrices.push_back(jointTransform);
        mesh.joints.push_back(jointname);
    }

    source_node = source_node->next_sibling();
    istringstream issweights(source_node->first_node()->value());
    rapidxml::xml_node<> *vertex_weights = skin_node->first_node("vertex_weights");
    int numVertices = atoi(vertex_weights->first_attribute("count")->value());
    istringstream issvcount(vertex_weights->first_node("vcount")->value());
    istringstream issv(vertex_weights->first_node("v")->value());

    vector<float> jointWeights;
    float weight;
    while(issweights>>weight){
        jointWeights.push_back(weight);
    }
    vector<float> vertexCounts;
    float vcount;
    while(issvcount>>vcount){
        vertexCounts.push_back(vcount);
    }
    for(int i=0;i<numVertices;i++){
        Eigen::Vector3f Weight;
        Eigen::Vector3f Index;
        for(int j=0;j<3;j++){

            if(j < vertexCounts[i]){
            float index_idx,weight_idx;
            issv>>index_idx>>weight_idx;
            Weight[j] = jointWeights[weight_idx];
            Index[j] = index_idx;
            }
            else{
                Weight[j] = 0;
                Index[j] = 0;
            }
        }
        /*for(int m=0;m<3;m++){
            if(Weight[m] < 0.3){
                Weight[m] = 0;
            }
        }*/
        // cout<<"Index:  "<<Weight.x()<<"  "<<Weight.y()<<"  "<<Weight.z()<<"  "<<(Weight.x()+Weight.y()+Weight.z())<<endl;
        Weight = Weight/(Weight.x()+Weight.y()+Weight.z());
        mesh.weights.push_back(Weight);
        mesh.indices.push_back(Index);
    }

}


Mesh MakeTriangles(){

    map<string,Eigen::Vector3f> color_map;
    color_map.insert(make_pair("Hair-material",Eigen::Vector3f(0.7,0.7,0.7)));
    color_map.insert(make_pair("Skin-material",Eigen::Vector3f(0.761,0.401,0.205)));
    color_map.insert(make_pair("gun_barrel-material",Eigen::Vector3f(0.0,0.0,0.0)));
    color_map.insert(make_pair("gun_main-material",Eigen::Vector3f(0.087,0.004,0.0008)));
    color_map.insert(make_pair("gun_main2-material",Eigen::Vector3f(0.024,0.026,0.027)));
    color_map.insert(make_pair("iron_sight-material",Eigen::Vector3f(0.163,0.163,0.163)));
    color_map.insert(make_pair("magazine-material",Eigen::Vector3f(0.055,0.055,0.055)));
    color_map.insert(make_pair("Shoe-material",Eigen::Vector3f(0.046962,0.046962,0.046962)));
    color_map.insert(make_pair("Shirt-material",Eigen::Vector3f(0.8,0.024,0.024)));
    color_map.insert(make_pair("Pant-material",Eigen::Vector3f(0.004,0.019,0.095)));
    color_map.insert(make_pair("Eyebrow-material",Eigen::Vector3f(0.007,0.004,0.003)));
    color_map.insert(make_pair("wall-material",Eigen::Vector3f(0.012,0.044,0.342)));
    color_map.insert(make_pair("floor-material",Eigen::Vector3f(0.8,0.044,0.007)));
    rapidxml::file<> xmlFile(path.c_str());
    rapidxml::xml_document<> doc;
    doc.parse<0>(xmlFile.data());    // 0 means default parse flags
    rapidxml::xml_node<> *root = doc.first_node();
    rapidxml::xml_node<> *geometries = root->first_node("library_geometries");
    rapidxml::xml_node<> * geometry_node = geometries->first_node("geometry");
    rapidxml::xml_node<> *mesh_node = geometry_node->first_node();
    rapidxml::xml_node<> *triangles = mesh_node->first_node("triangles");
    
    
    for (rapidxml::xml_node<> * triangles = mesh_node->first_node("triangles"); triangles; triangles = triangles->next_sibling()){
        if(strcmp(triangles->name(),"triangles"))
            break;
        Material mat;
        mat.name = triangles->first_attribute("material")->value();
        cout<<mat.name<<endl;
        int count = atoi(triangles->first_attribute("count")->value());
        int num_vertices = count*3;
        rapidxml::xml_node<> *p = triangles->first_node("p");
        istringstream iss(p->value());
        
        int attributes[] = {0,0,0,0};
    
        for (rapidxml::xml_node<> * input_node = triangles->first_node("input"); input_node; input_node = input_node->next_sibling()){
            if(strcmp(input_node->name(),"input"))
                break;
            if(!strcmp(input_node->first_attribute("semantic")->value(),"VERTEX"))
                attributes[0] = 1;
            if(!strcmp(input_node->first_attribute("semantic")->value(),"NORMAL"))
                attributes[1] = 1;
            if(!strcmp(input_node->first_attribute("semantic")->value(),"TEXCOORD"))
                attributes[2] = 1;
            if(!strcmp(input_node->first_attribute("semantic")->value(),"COLOR"))
                attributes[3] = 1;
        }
        bool is_character = (mesh.weights.size() != 0);
        for(int i=0;i<num_vertices;i++){
            Vertex v;
            int n;
            if(attributes[0] == 1){ 
                iss>>n;
                v.position = mesh.positions[n];
                if(is_character){
                v.joint_weights = mesh.weights[n];
                v.joint_indices = mesh.indices[n];    
                }
                
            }
            if(attributes[1] == 1){
                iss>>n;
                v.normal = mesh.normals[n];
            }
            if(attributes[2] == 1){
                iss>>n;
                //v.uv = mesh.uvs[n];
            }
            if(attributes[3] == 1){
                iss>>n;
                v.color = mesh.colors[n];
            }
            mat.vertices.push_back(v);
        }
        
        auto it = color_map.find(mat.name);
        if(it == color_map.end()){
            mat.color = Eigen::Vector3f(0,0,1);
        }
        else{
            mat.color = it->second;
        }
        mesh.materials.push_back(mat);
    }
    cout<<"Materials no.:  "<<mesh.materials.size()<<endl;
    return mesh;
}


private:
    string path;
    Mesh mesh;
    
};