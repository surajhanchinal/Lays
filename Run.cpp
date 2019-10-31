#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <FreeImage.h>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include "AnimationObject.h"
#include "ArenaObject.h"
#include <ncurses.h>
#include "textureManager.h"
#include "Bullet.h"
#define USE_SHADERS 1



float sensitivity = 0.001;
GLuint  prog_hdlr,bone_hdlr,cross_hdlr,bullet_hdlr;
unsigned int VBO,lightVAO;
bool flag=true;
Camera *camera;
Camera *tpCamera;
Mesh mesh;
Parser *parser;
AnimationObject *object;
ArenaObject *arena;
TextureManager texer;
float deltaTime=0;
GLfloat aspect;
int frame=0,t1,timebase=0;
int t2;
float width=1920,height=1022;
int prev_x,prev_y;
bool first_time = true;
char current_click = ' ';
int count1 = 0;
Eigen::Vector3f LightPos(0,30,-10);

void drawCrossHair(){
  float length = 2;
  float length2 = 0.5;
  float offset = 0.008;
  float scale = 0.005;
  glUseProgram(cross_hdlr);
    GLuint Matrixp = glGetUniformLocation(cross_hdlr, "projection");
    GLuint Matrixm = glGetUniformLocation(cross_hdlr, "model");
    Eigen::Matrix4f m = Eigen::Matrix4f::Identity();
    glUniformMatrix4fv(Matrixp, 1, GL_FALSE, camera->getProjectionMatrix().data());    
    m << scale*length2,0,0,0,
         0,scale*length,0,offset,
         0,0,scale,-0.5,
         0,0,0,1;
    glUniformMatrix4fv(Matrixm, 1, GL_FALSE, m.data());
    glBindVertexArray(lightVAO);
    glDrawArrays(GL_TRIANGLES,0,36);

    m << scale*length2,0,0,0,
         0,scale*length,0,-offset,
         0,0,scale,-0.5,
         0,0,0,1;
    glUniformMatrix4fv(Matrixm, 1, GL_FALSE, m.data());
    glBindVertexArray(lightVAO);
    glDrawArrays(GL_TRIANGLES,0,36);

    m << scale*length,0,0,offset,
         0,scale*length2,0,0,
         0,0,scale,-0.5,
         0,0,0,1;
    glUniformMatrix4fv(Matrixm, 1, GL_FALSE, m.data());
    glBindVertexArray(lightVAO);
    glDrawArrays(GL_TRIANGLES,0,36);

    m << scale*length,0,0,-offset,
         0,scale*length2,0,0,
         0,0,scale,-0.5,
         0,0,0,1;
    glUniformMatrix4fv(Matrixm, 1, GL_FALSE, m.data());
    glBindVertexArray(lightVAO);
    glDrawArrays(GL_TRIANGLES,0,36);
}


void setUnifs(GLuint shaderID){
   glUseProgram(shaderID);
   GLuint Matrixp = glGetUniformLocation(shaderID, "projection");
   GLuint Matrixv = glGetUniformLocation(shaderID, "view");
   GLuint ltpos = glGetUniformLocation(shaderID, "LightPos");
   glUniform3fv(ltpos,1,LightPos.data());
   glUniformMatrix4fv(Matrixp, 1, GL_FALSE, camera->getProjectionMatrix().data());
   glUniformMatrix4fv(Matrixv, 1, GL_FALSE, camera->getViewMatrix().data());
}

void vao_display(){
   glViewport(0, 0, 1920, 1022);
   glClearColor(0,0,0,0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glEnable(GL_TEXTURE_2D);
   
   deltaTime = (glutGet(GLUT_ELAPSED_TIME) - t2)/1000.0f;
   t2 = glutGet(GLUT_ELAPSED_TIME);
   setUnifs(prog_hdlr);
   object->updateAnimation(deltaTime);
   object->Fire(deltaTime);

   setUnifs(bone_hdlr);
   arena->Draw(object->out_model.inverse());
    
    /*glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
     GLuint Matrixp = glGetUniformLocation(bone_hdlr, "projection");
    GLuint Matrixm = glGetUniformLocation(bone_hdlr, "model");
    Eigen::Matrix4f m = Eigen::Matrix4f::Identity();
    glUniformMatrix4fv(Matrixp, 1, GL_FALSE, camera->getProjectionMatrix().data());    
    m << 8,0,0,0,
         0,14,0,7,
         0,0,8,0,
         0,0,0,1;
    glUniformMatrix4fv(Matrixm, 1, GL_FALSE, m.data());
    glBindVertexArray(lightVAO);

    glDrawArrays(GL_TRIANGLES,0,36);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);*/
    setUnifs(bullet_hdlr);
    object->DrawBullets();
   drawCrossHair();


   t1=glutGet(GLUT_ELAPSED_TIME);
   frame++;
   if(t1-timebase > 1000){
    //std::cout<<count1<<endl;
    count1 = 0;   
    std::cout<<"FPS: "<<frame*1000.0/(t1-timebase)<<std::endl;
		timebase = t1;
		frame = 0;
   }
   glutSwapBuffers();
}




// --------------- keyboard event function ---------------------------------------

void click(int button,int state,int x,int y){
  if(button == GLUT_LEFT_BUTTON){
    if(state == GLUT_UP){
      object->setAnimation("REST");
      object->stopFire();
      cout<<"release"<<endl;
    }
    if(state == GLUT_DOWN){
      object->setAnimation("RECOIL");
      object->startFire();
      cout<<"catch"<<endl;
    }
  }
}


void look( int x, int y ){
  count1++;
  if((y > 0.8*height) || (y< 0.2*height)){
    glutWarpPointer(x, height/2);
    prev_y = height/2;
  }
  else if((x > 0.9*width) || (x< 0.1*width)){
    glutWarpPointer(width/2, y);
    prev_x = width/2;
  }
  else{
    int deltaX = x - prev_x;
    int deltaY = y - prev_y;
    object->Rotate(-deltaX*sensitivity,-deltaY*sensitivity);
    prev_x = x;
    prev_y = y;
  }
}

void keyUp(unsigned char key, int x, int y){
  if(current_click == key){
    if(('w' == key) || ('s' == key)){
    object->setAnimation("REST");
  }
  if(('a' == key) || ('d' == key) || (32 == key)){
  object->setAnimation("REST");
  }
  }
  
  
}


void simpleKeyboard(unsigned char key, int x, int y)
{

  current_click = key;
  if('8' == key){
	  camera->moveCamera(-0.4,0,0);
  }
  if('5' == key){
    camera->moveCamera(0.4,0,0);
  }
  if('7' == key){
    camera->moveCamera(0,0,-0.4);
  }
  if('1' == key){
    camera->moveCamera(0,0,0.4);
  }
  if('4' == key){
    camera->moveCamera(0,0.4,0);
  }
  if('6' == key){
    camera->moveCamera(0,-0.4,0);
  }

  if('w' == key){
    object->setAnimation("RUN");
  }
  if('s' == key){
    object->setAnimation("BACK");
  }
  if('a' == key){
    object->setAnimation("RIGHT_SIDE");
  }
  if('d' == key){
    object->setAnimation("LEFT_SIDE");
  }
  if(32 == key){
    object->setAnimation("JUMP");
  }
  if('x' == key){
    //object->setAnimation("RECOIL");
    //cout<<endl<<object->fpCamera->getViewMatrix()<<endl;
    object->putBullet();
  }
  if('k' == key){
    //cout<<camera->getEyePosition();
    //cout<<camera->phi<<"  "<<camera->theta<<endl;
    cout<<"Position:  "<<object->getPosition()<<endl;
  }
  if('g' == key){
    camera = tpCamera;
  }
  if('h' == key){
    camera = object->fpCamera;
  }
  
  glutPostRedisplay();
}


void specialKeyFunction(int key, int x, int y) {
        // Change rotation amounts in response to arrow and home keys.
    if ( key == GLUT_KEY_LEFT )
      camera->rotateCamera(0,-0.1);
    else if ( key == GLUT_KEY_RIGHT )
      camera->rotateCamera(0,0.1);
    else if ( key == GLUT_KEY_DOWN)
      camera->rotateCamera(0.1,0);
    else if ( key == GLUT_KEY_UP )
      camera->rotateCamera(-0.1,0);
    glutPostRedisplay();
}


void changeSize(int x, int y){
    cout<<"hello"<<endl;
    if (y == 0 || x == 0) return;

    aspect = (GLfloat)((float)x/y);
    cout<<x<<"  "<<y<<endl;
    //camera->updateAspect(aspect);
    width = x;
    height = y;
    glViewport(0,0,x,y);
} 

#if USE_SHADERS
void printInfoLog(GLuint obj) {
	int log_size = 0;
	int bytes_written = 0;
	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &log_size);
	if (!log_size) return;
	char *infoLog = new char[log_size];
	glGetProgramInfoLog(obj, log_size, &bytes_written, infoLog);
	std::cerr << infoLog << std::endl;
	delete [] infoLog;
}

bool read_n_compile_shader(const char *filename, GLuint &hdlr, GLenum shaderType) {
	std::ifstream is(filename, std::ios::in|std::ios::binary|std::ios::ate);
	if (!is.is_open()) {
		std::cerr << "Unable to open file " << filename << std::endl;
		return false;
	}
	long size = is.tellg();
	char *buffer = new char[size+1];
	is.seekg(0, std::ios::beg);
	is.read (buffer, size);
	is.close();
	buffer[size] = 0;

	hdlr = glCreateShader(shaderType);
	glShaderSource(hdlr, 1, (const GLchar**)&buffer, NULL);
	glCompileShader(hdlr);
	std::cerr << "info log for " << filename << std::endl;
	printInfoLog(hdlr);
	delete [] buffer;
	return true;
}

void setShaders(GLuint &prog_hdlr, const char *vsfile, const char *fsfile) {
	GLuint vert_hdlr, frag_hdlr;
	read_n_compile_shader(vsfile, vert_hdlr, GL_VERTEX_SHADER);
	read_n_compile_shader(fsfile, frag_hdlr, GL_FRAGMENT_SHADER);

	prog_hdlr = glCreateProgram();
	glAttachShader(prog_hdlr, frag_hdlr);
	glAttachShader(prog_hdlr, vert_hdlr);

	glLinkProgram(prog_hdlr);
	std::cerr << "info log for the linked program" << std::endl;
	printInfoLog(prog_hdlr);
}
#endif
// ----------------- main routine -------------------------------------------------

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(1920,1080); 
    glutInitWindowPosition(0,0);
    
    glutCreateWindow("ARROW KEYS ROTATE THE TEAPOTS; HOME KEY RESETS");

    glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
    glutReshapeFunc(changeSize);
    glutIdleFunc(vao_display); 
    glutSpecialFunc(specialKeyFunction);
    glutKeyboardFunc(simpleKeyboard);
    glutKeyboardUpFunc(keyUp);
    glutPassiveMotionFunc(look);
    glutMotionFunc(look);
    glutMouseFunc(click);
    glutSetCursor(GLUT_CURSOR_NONE);
	  glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    #if USE_SHADERS
	glewInit();
	if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader && GL_EXT_geometry_shader4)
		std::cout << "Ready for GLSL - vertex, fragment, and geometry units" << std::endl;
	else {
		std::cout << "No GLSL support" << std::endl;
		exit(1);
	}
	setShaders(prog_hdlr, "shaders/vert.glsl", "shaders/frag.glsl");
  setShaders(bone_hdlr, "shaders/light_vert.glsl", "shaders/light_frag.glsl");
  setShaders(cross_hdlr, "shaders/cross_vert.glsl", "shaders/cross_frag.glsl");
  setShaders(bullet_hdlr, "shaders/bullet_vert.glsl", "shaders/bullet_frag.glsl");
#endif

   float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,0.0f,

        -0.5f, -0.5f, 0.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,0.0f,
         0.5f, -0.5f, 0.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,0.0f,
         0.5f,  0.5f, 0.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,0.0f,
         0.5f,  0.5f, 0.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,0.0f,
        -0.5f,  0.5f, 0.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,0.0f,
        -0.5f, -0.5f, 0.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,0.0f

    };


    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    /* glBindVertexArray(cubeVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2); */

// second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);

    object = new AnimationObject("MAN","../csgo/bbup_new2.dae");
    arena = new ArenaObject("ARENA","../csgo/arena.dae");
    arena->setShader(bone_hdlr);
    object->addAnimation("REST","out_running.txt",0,0.15,Eigen::Vector3f(0,0,0));
    object->addAnimation("RUN","out_running.txt",1,0.15,Eigen::Vector3f(0,0,-50));
    object->addAnimation("RECOIL","out_recoil.txt",0,0.05,Eigen::Vector3f(0,0,0));
    object->addAnimation("WALK","out_walking.txt",1,0.15,Eigen::Vector3f(0,0,-50));
    object->addAnimation("LEFT_SIDE","out_sideStep.txt",1,0.5,Eigen::Vector3f(40,0,0));
    object->addAnimation("RIGHT_SIDE","out_sideStep.txt",1,0.5,Eigen::Vector3f(-40,0,0));
    object->addAnimation("JUMP","out_jump2.txt",0,0.15,Eigen::Vector3f(0,0,-14));
    object->addAnimation("BACK","out_back.txt",1,0.15,Eigen::Vector3f(0,0,25));
    object->setShader(prog_hdlr);
    object->setAnimation("REST");
    object->setArena(arena);
    texer.TextureFromFile("./bullet_hole2.png","bullet");
    object->initBullets(30,bullet_hdlr,lightVAO,texer.getTextureID("bullet"),10);
    tpCamera = new Camera(Eigen::Vector3f(11,11,-17),Eigen::Vector3f(0,1,0),45.0f,1920.0f/1022.0f,0.1f,1000.0f,0.1,-0.9);
    camera = object->fpCamera;
    //cout<<camera->getProjectionMatrix()<<endl;
   //cout<<endl;
    //cout<<camera->getViewMatrix()<<endl;
    glutMainLoop();
    return 0;
}
