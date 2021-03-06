#define GL_GLEXT_PROTOTYPES
#include <GL/glew.h>
#include <GL/glu.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "Shader.h"
#include "Texture.h"
#include "Framebuffer.h"
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <unordered_map>
#include <Skull.h>
#include <Ant.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


//#include <QOpenGLDebugLogger>
//#include <QOpenGLContext>
#include "Utilities.h"
#define time(message,timer) glFlush(); glFinish(); qDebug()<<message<<    timer.ntime();

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	640
    Timer timer;
    Timer timer2;
    Timer timer3;
    int bind=0;
    std::unordered_map<std::string,AnimatedObject*> assets;
    std::vector<AnimatedObject*> toyList;
    Object * board;
    Object * ficha1;
    Object * ficha2;
    int chosenToyLeft =0;
    int chosenToyRight=0;
//    AnimatedObject * einstein;
//    Bone * boneDebug;
    Object * origin;
    Object * debugCube1;
    Object * debugCube2;
    Object * debugCube3;
    DebugCube * debugCube;
	Shader *texShader;
	Shader *mixShader;
//	Shader *post_shader;
    Framebuffer* texFramebuffer;
//	Framebuffer* mixFramebuffer;

	GLuint texNames;
	float Xrot;
	float Yrot;
	float Zrot;
	float wXrot;
	float wYrot;
	float wZrot;
	float X0;
	float Y0;
	float lastX;
	float lastY;

    float cameraTheta=3.14/2;
    float cameraPhi=3.14/4;

    int fichaL=1;
    int fichaR=1;

float angle=0;
int an=0;
int dQ=-1;
int f=0;

glm::vec3 cameraPos;
glm::vec3 cameraUp;
glm::mat4 view;
glm::mat4 projection=   glm::perspective(45.0f, 1.0f*1/1, 0.05f, 200.0f);

bool dan= false;

void moveCamera(){
    cameraPos =   glm::vec3(4*cos(cameraTheta)*sin(cameraPhi), 4*sin(cameraTheta)*sin(cameraPhi),4*cos(cameraPhi));
    cameraUp  =   glm::vec3(0.0,0.0,1.0);
    view      =   glm::lookAt(cameraPos, glm::vec3(0.0, 0.0, 0.0), cameraUp);
}

void moveFichas(){
    *(ficha1->model_matrix)=    glm::translate(glm::mat4(1.f),glm::vec3(0,(float)fichaL*0.31-1.4,0));
    *(ficha2->model_matrix)=	glm::translate(glm::mat4(1.f),glm::vec3(0,(float)fichaR*0.31-1.4,0));
}

void initializeGL()
{
	glewInit();
/*    einstein = new AnimatedObject("lowskull");
    einstein->rotate(1,0,0,-90);
    einstein->scale(0.75);
    einstein->traslate(0,0.7,0);*/
    origin = new Object("Origin",false);
    debugCube = new DebugCube(0.2,glm::vec3(0.5,0.5,0.5));
//0, 2, 0
    board =new Object("Board"); board->scale(1.8);// board->rotate(1,0,0,-90);
    ficha1=new Object("Ficha"); ficha1->scale(0.25);/* ficha1->rotate(1,0,0,-90);*/ ficha1->traslate(1.1,0,0.15);
    ficha2=new Object("Ficha"); ficha2->scale(0.25);/* ficha2->rotate(1,0,0,-90);*/ ficha2->traslate(-1.1,0,0.15);
    assets["Ant"]   = new Ant();    assets["Ant"]  ->Import("ant2");     assets["Ant"]  ->setup();
    assets["Skull"] = new Skull();  assets["Skull"]->Import("lowskull"); assets["Skull"]->setup();
    glEnable(GL_DEPTH_TEST);


	texShader	= new Shader("texShader.v","texShader.f");
	mixShader	= new Shader("mixShader.v","mixShader.f");
    texShader	->getProperties("coord3d normal tangent texture weights indices deform m v p m_tr_inv monkeyTex heightTex bones centers parents animation",7,10);
    mixShader	->getProperties("corners colorTex normalTex posTex camera",1,4);

	texFramebuffer = new Framebuffer(SCREEN_WIDTH, SCREEN_HEIGHT,3);
    texFramebuffer->texture_names = {"colorTex","normalTex", "posTex"};


    moveCamera();
    moveFichas();

/*
    QSurfaceFormat format;
    // asks for a OpenGL 3.2 debug context using the Core profile
    format.setMajorVersion(2);
    format.setMinorVersion(1);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DebugContext);
    QOpenGLContext *context = new QOpenGLContext();
    context->setFormat(format);
    context->create();
    qDebug()<<"gl debug extensions?"<<context->hasExtension(QByteArrayLiteral("GL_KHR_debug"));
    QOpenGLDebugLogger *logger = new QOpenGLDebugLogger();
    logger->initialize(); // initializes in the current context, i.e. ctx
*/
}

void texPass();

void paintGL()
{
    glUseProgram(texShader->program);
//	"coord3d normal tangent texture weights indices m v p m_tr_inv monkeyTex heightTex"

    glUniformMatrix4fv(texShader->uniforms["v"], 1, GL_FALSE, glm::value_ptr(view)		);
    glUniformMatrix4fv(texShader->uniforms["p"], 1, GL_FALSE, glm::value_ptr(projection));

	glBindFramebuffer(GL_FRAMEBUFFER, texFramebuffer->fbo);
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT0+1,GL_COLOR_ATTACHMENT0+2};
	glDrawBuffers(3, buffers);
//time("end3:\t", timer)
    texPass();
//time("end3:\t", timer)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(mixShader->program);
    glUniform3fv(mixShader->uniforms["camera"], 1, glm::value_ptr(cameraPos)	);
//time("end3:\t", timer)
    if(dQ == -1)    texFramebuffer->draw(mixShader);
    else            texFramebuffer->draw(mixShader,dQ);
//time("end3:\t", timer)
    for(int i=0;i<toyList.size();i++)
        if(toyList[i]->position->x >8 ||
           toyList[i]->position->x <1)
           toyList.erase(toyList.begin()+i);

//time("end3:\t", timer)
}

void texPass(){

    glClearColor(0, 0, 0, 1.0);
    glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

    texShader->enableAttributes();

    glUniform1i(texShader->uniforms["animation"], 0 );

    board->draw(texShader);
    ficha1->draw(texShader);
    ficha2->draw(texShader);
//  origin->draw(texShader);
//  debugCube->draw(texShader);

    for(auto toy: toyList){
        toy->animate(false);
        toy->draw(texShader);
    }
}
void finalPass(){

}
void mousePressEvent(SDL_MouseButtonEvent event)
{
	X0=event.x;
	Y0=event.y;	
}

void mouseReleaseEvent(SDL_MouseButtonEvent event)
{ 
	lastX=event.x;
	lastY=event.y;

	wYrot =(lastX-X0)/100;
	wXrot =(lastY-Y0)/100;
}

void keyPressEvent(SDL_KeyboardEvent event){

    if(event.keysym.sym==SDLK_KP_2){ if(cameraPhi<3.14/2)  cameraPhi+=0.02;  moveCamera();}
    if(event.keysym.sym==SDLK_KP_8){ if(cameraPhi>=0.02)   cameraPhi-=0.02;  moveCamera();}
    if(event.keysym.sym==SDLK_KP_4){ cameraTheta-=0.02;                      moveCamera();}
    if(event.keysym.sym==SDLK_KP_6){ cameraTheta+=0.02;                      moveCamera();}

    if(event.keysym.sym==SDLK_UP)   {fichaR-= fichaR>1 ? 1:0; moveFichas();}
    if(event.keysym.sym==SDLK_DOWN) {fichaR+= fichaR<8 ? 1:0; moveFichas();}
    if(event.keysym.sym==SDLK_w)    {fichaL-= fichaL>1 ? 1:0; moveFichas();}
    if(event.keysym.sym==SDLK_s)    {fichaL+= fichaL<8 ? 1:0; moveFichas();}

    if(event.keysym.sym==SDLK_LEFT) chosenToyRight=(chosenToyRight+3)%2;
    if(event.keysym.sym==SDLK_RIGHT)chosenToyRight=(chosenToyRight+1)%2;
    if(event.keysym.sym==SDLK_a)    chosenToyLeft =(chosenToyLeft +3)%2;
    if(event.keysym.sym==SDLK_d)    chosenToyLeft =(chosenToyLeft +1)%2;

    if(event.keysym.sym==SDLK_RETURN){
        AnimatedObject* s;
        switch(chosenToyRight){
            case 0: s = new Ant();
                    s->copy(assets["Ant"]);
            break;
            case 1: s = new Skull();
                    s->copy(assets["Skull"]);
            break;
        }

        s->setup(1,fichaR,0);
        toyList.push_back(s);
    }
    if(event.keysym.sym==SDLK_SPACE){
        AnimatedObject* s;
        switch(chosenToyLeft){
            case 0: s = new Ant();
                    s->copy(assets["Ant"]);
            break;
            case 1: s = new Skull();
                    s->copy(assets["Skull"]);
            break;
        }
        s->setup(8,fichaL,1);
        toyList.push_back(s);
    }
    if(event.keysym.sym==SDLK_q) dQ = (dQ>40) ? -1 : dQ+1;
    if(event.keysym.sym==SDLK_l) dan = true;

}


//The window we'll be rendering to
SDL_Window* gWindow = NULL;

void initSDL()
{
	//Initialize SDL
	SDL_Init( SDL_INIT_VIDEO );
	//Use OpenGL 2.1
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );

	gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
    //disable vsync
#if SDL_VERSION_ATLEAST(1,3,0)
SDL_GL_SetSwapInterval(0);
#else /* SDL_VERSION_ATLEAST(1,3,0) */
#ifdef SDL_GL_SWAP_CONTROL
SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0);
#else /* SDL_GL_SWAP_CONTROL */
DEBUG("VSync unsupported on old SDL versions (before 1.2.10).");
#endif /* SDL_GL_SWAP_CONTROL */
#endif /* SDL_VERSION_ATLEAST(1,3,0) */

    //Create context
	if( SDL_GL_CreateContext( gWindow ) != NULL ){
		//Use Vsync
    //	if( SDL_GL_SetSwapInterval( 1 ) < 0 ) printf( "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError() );
	}
}

void close()
{
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	SDL_Quit();
}

int main( int argc, char* args[] )
{
    initSDL();
    initializeGL();

	SDL_Event e;
	bool quit = false;
    Uint32 now      = SDL_GetTicks();
    Uint32 before   = now;
	while( !quit )
	{
		while( SDL_PollEvent( &e ) != 0 )
		{ 
			switch( e.type ){
            case SDL_QUIT: 				quit = true; 				break;
            case SDL_MOUSEBUTTONDOWN:	mousePressEvent(e.button);	break;
            case SDL_MOUSEBUTTONUP:		mouseReleaseEvent(e.button);break;
            case SDL_KEYDOWN:           keyPressEvent(e.key);break;
   //        case SDL_KEYUP:
			}
		}

        paintGL();

		SDL_GL_SwapWindow( gWindow );
         now = SDL_GetTicks();
       //  qDebug()<<"n: "<<now<<"b: "<<before;
         SDL_Delay( std::max((int)(50 - (now-before)),0) );
         now = SDL_GetTicks();
         //qDebug()<<"fps: "<<1000.0/(now-before);
         before = now;

	}
	
	close();

	return 0;
}
