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
#define USE_SHADERS 1




GLuint  prog_hdlr,bone_hdlr;
unsigned int VBO,lightVAO;
Camera *camera;
Camera *tpCamera;
Mesh mesh;
Parser *parser;
AnimationObject *object;
ArenaObject *arena;
float deltaTime=0;
GLfloat aspect;
int frame=0,t1,timebase=0;
int t2;
float width=1920,height=1022;
Eigen::Vector3f LightPos(0,10,-10);

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
   

   setUnifs(bone_hdlr);
  GLuint rev_aff_loc = glGetUniformLocation(bone_hdlr, "rev_aff");
  glUniformMatrix4fv(rev_aff_loc, 1, GL_FALSE, object->rev_aff.inverse().matrix().data()); 
   arena->Draw();


   t1=glutGet(GLUT_ELAPSED_TIME);
   frame++;
   if(t1-timebase > 1000){   
     std::cout<<"FPS: "<<frame*1000.0/(t1-timebase)<<std::endl;
		timebase = t1;
		frame = 0;
   }
   glutSwapBuffers();
}




// --------------- keyboard event function ---------------------------------------


void keyUp(unsigned char key, int x, int y){
  if(('w' == key) || ('s' == key)){
    object->setAnimation("REST");
  }
  if(('a' == key) || ('d' == key)){
  object->Rotate(0);
  }
}


void simpleKeyboard(unsigned char key, int x, int y)
{
  if('8' == key){
	  camera->translateCamera(-0.4,0,0);
  }
  if('5' == key){
    camera->translateCamera(0.4,0,0);
  }
  if('7' == key){
    camera->translateCamera(0,0,-0.4);
  }
  if('1' == key){
    camera->translateCamera(0,0,0.4);
  }
  if('4' == key){
    camera->translateCamera(0,0.4,0);
  }
  if('6' == key){
    camera->translateCamera(0,-0.4,0);
  }

  if('w' == key){
    object->setAnimation("WALK");
  }
  if('s' == key){
    object->setAnimation("BACK");
  }
  if('a' == key){
    object->Rotate(1);
  }
  if('d' == key){
    object->Rotate(-1);
  }
  if(32 == key){
    object->setAnimation("JUMP");
  }
  if('x' == key){
    object->setAnimation("SIDE");
  }
  if('k' == key){
    cout<<camera->getEyePosition();
    cout<<camera->phi<<"  "<<camera->theta<<endl;
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
    glutInitWindowPosition(100,100);
    
    glutCreateWindow("ARROW KEYS ROTATE THE TEAPOTS; HOME KEY RESETS");

    glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
    glutReshapeFunc(changeSize);
    glutIdleFunc(vao_display); 
    glutSpecialFunc(specialKeyFunction);
    glutKeyboardFunc(simpleKeyboard);
    glutKeyboardUpFunc(keyUp);

	 glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

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
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,0.0f
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
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);

    object = new AnimationObject("MAN","../csgo/bbup_new2.dae");
    arena = new ArenaObject("ARENA","../csgo/arena.dae");
    arena->setShader(bone_hdlr);
    object->addAnimation("REST","out_running.txt",0,0.15,Eigen::Vector3f(0,0,0));
    object->addAnimation("RUN","out_running.txt",1,0.15,Eigen::Vector3f(0,14,0));
    object->addAnimation("RECOIL","out_recoil.txt",0,0.15,Eigen::Vector3f(0,0,0));
    object->addAnimation("WALK","out_walking.txt",1,0.15,Eigen::Vector3f(0,9,0));
    object->addAnimation("SIDE","out_sideStep.txt",1,0.5,Eigen::Vector3f(9,0,0));
    object->addAnimation("JUMP","out_jump.txt",0,0.15,Eigen::Vector3f(0,14,0));
    object->addAnimation("BACK","out_back.txt",1,0.15,Eigen::Vector3f(0,-9,0));
    object->setShader(prog_hdlr);
    object->setAnimation("REST");
    tpCamera = new Camera(Eigen::Vector3f(11,11,-17),Eigen::Vector3f(0,1,0),45.0f,1920.0f/1022.0f,0.1f,1000.0f,0.1,-0.9);
    camera = object->fpCamera;
    //cout<<camera->getProjectionMatrix()<<endl;
   //cout<<endl;
    //cout<<camera->getViewMatrix()<<endl;
    glutMainLoop();
    return 0;
}
