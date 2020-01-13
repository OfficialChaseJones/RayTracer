/*

	Ray tracing project
	Chase Jones, CS434


	Due april 12th.
	Started March 13.




	SuperRayTracer.cpp

	main file that opens a window, sets up scene, calls ray tracing code

	Optimizations:
	1. Shadow path caching. each pixel is likely to have the same path to a light as the previous.
			if(previous illumination constant = 0)
					then if previousObjectSHadowing.intersectionAt(newShadowRay)
						this illumination constant = 0
	2. First object intersection

		implement polygon->intersectspyramid()


  */

/*

	File format:
	
	first comes information about the camera and the final rendered image size and quality

	camera x y z x2 y2 z2  //the camera location and direction
	antialias a            //quality of antialiasing 1- none  2-good 3-very high 4...
	size      x y          //size of the final rendered image
	raytracedepth depth 


	then comes any number of 3 types of commands in the script:
	
	[object name] (location.x location.y location.z) [other args]
	AND
	lighting  r g b  [diffuse coefficient] [specular coefficient]
	AND
	light x y z r g b
	
	Ex:
	lighting 0.1 0.5 0.6 0.3 0.9 1.08 0.5 0.6
	sphere 0  0  0  5
	polygon  5
	         0 0 0 
			 1 2 3
			 5 5 5
	cylinder 0 0 0 3 4
	light 5 0 -5 1 0 0
	bottle x y z r h [x y] (4 points for the bezier curve)
	refraction 0.5 1.19
	reflection 0.5

  */

#include <stdlib.h>
#include <GL/glut.h>
#include <stdio.h>
#include <windows.h>
#include "RayTrace.h"
#include "Settings.h"
#include "PerformanceRayTrace.h"

//file string constants
#define DEFAULT_FILENAME "scene.dat"
#define SPHERE   "sphere"
#define POLYGON  "polygon"
#define CYLINDER "cylinder"
#define LIGHTING "lighting"
#define LIGHT    "light"
#define BOTTLE   "bottle"
#define REFRACTION "refraction"
#define REFLECTION "reflection"

void display();//display current image, does not do ray tracing
void reshape(int w, int h);
void initVars(int width, int height);
void render();//sets up scene and calls necesary methods, data stored in 'screenData'
long WINAPI renderWrapper(long lParam);//initiated by a new thread, calls render
void readSceneFromFile(struct Scene * scene, char * filename);//reads scene in from a file
void generateTestScreen();//used as an error message

ScreenData screenData;
char * filename;

//These are necesary to use the image reader provided by the prof
unsigned short int imageHolder_red[501][501]; // hold image info, enough size
unsigned short int imageHolder_green[501][501];
unsigned short int imageHolder_blue[501][501];


int main(int argc, char ** argv)
{
	//filename may be specified by argument
	if(argc>0)
		filename = argv[0];
	else 
		filename = DEFAULT_FILENAME;

	//create window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
	glutInitWindowSize(WINDOW_WIDTH,WINDOW_HEIGHT);
	glutInitWindowPosition(50,50);
	glutCreateWindow(argv[0]);

	//initVars();
	unsigned long iID;//dummy var necesary for thread
	CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)renderWrapper,NULL,0,&iID);
	//render();

	//set event handlers
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	//begin!
	glutMainLoop();

	return 0;
}//main

void initVars(int width, int height)
{
	screenData.init(width, height);
	generateTestScreen();//draws a basic image, which is displayed in the event of an error
}

long WINAPI renderWrapper(long lParam)
{
	//just calls render function, which sets up the scene and runs the ray tracing code
	render();

	return 0;
}

void render()
{

	//1. set up scene
	//2. call methods
	struct Scene * scene;
	scene = new Scene();
	scene->init();

	try
	{
		readSceneFromFile(scene, SCENE_DATA_FILENAME);
		//screenData.init(scene->numPixelsHorizontal, scene->numPixelsVertical);
		//generateTestScreen();
		//renderScene(scene, &screenData);
		performance_RenderScene(scene,&screenData);
	}
	catch(char * exception)
	{
		//Print the error message and draw a pretty screen
		printf(exception);
		generateTestScreen();
	}

	
	//END TEST
}

void generateTestScreen()
{
	//This is just for testing, and will appear if there is some kind of error
	for(int x=0;x< screenData.width;x++)
	{
		for(int y = 0;y<screenData.height;y++)
		{
			screenData.setPixel(x,y,((float)x)/screenData.width,((float)y)/screenData.height,0);
		}

	}
}

void readSceneFromFile(struct Scene * scene, char * filename)
{
	/*

	File format:
	
	first comes information about the camera and the final rendered image size and quality

	camera x y z x2 y2 z2  //the camera location and direction
	antialias a            //quality of antialiasing 1- none  2-good 3-very high 4...
	size      x y          //size of the final rendered image
	raytracedepth depth 


	then comes any number of 3 types of commands in the script:
	
	[object name] (location.x location.y location.z) [other args]
	AND
	lighting  r g b [diffuse coefficient] [specular coefficient]
	AND
	light x y z r g b
	
	Ex:
	lighting 0.1 0.5 0.6 0.3 0.9 1.08 0.5 0.6
	sphere 0  0  0  5
	polygon  5
	         0 0 0 
			 1 2 3
			 5 5 5
	cylinder 0 0 0 3 4
	light 5 0 -5 1 0 0
	bottle x y z r h [x y]* of bezier curve
	refraction 0.5 1.19
	reflection 0.5

  */

	FILE * filePointer = fopen(filename,"r");
	char command[100];//buffer for the commands read in

	if(filePointer==NULL)
		throw "File could not be found or there was an error opening the file.";

	float cameraX,cameraY,cameraZ,cameraX2,cameraY2,cameraZ2;
	int   antialias, sizex, sizey, rayTraceDepth;

	fscanf(filePointer,"%s", command);//burn strings
	fscanf(filePointer,"%f", &cameraX);
	fscanf(filePointer,"%f", &cameraY);
	fscanf(filePointer,"%f", &cameraZ);
	fscanf(filePointer,"%f", &cameraX2);
	fscanf(filePointer,"%f", &cameraY2);
	fscanf(filePointer,"%f", &cameraZ2);

	fscanf(filePointer,"%s", command);//burn strings
	fscanf(filePointer,"%d", &antialias);

	fscanf(filePointer,"%s", command);//burn strings
	fscanf(filePointer,"%d", &sizex);
	fscanf(filePointer,"%d", &sizey);

	fscanf(filePointer,"%s", command);//burn strings
	fscanf(filePointer,"%d", &rayTraceDepth);

	//set properties
	scene->cameraLocation.x = cameraX;
	scene->cameraLocation.y = cameraY;
	scene->cameraLocation.z = cameraZ;
	scene->cameraLookat.x   = cameraX2;
	scene->cameraLookat.y   = cameraY2;
	scene->cameraLookat.z   = cameraZ2;

	scene->ambientTerm.blue  = 0;
	scene->ambientTerm.green = 0;
	scene->ambientTerm.red   = 0;

	scene->numPixelsHorizontal = sizex;
	scene->numPixelsVertical   = sizey;
	
	initVars(sizex, sizey);

	//glutInitWindowSize(sizex,sizey);
	//glutWindowSize(sizex,sizey);
	glutReshapeWindow(sizex,sizey);

	scene->cameraSpanLeftRight = PI/2.2;
	scene->cameraSpanUpDown    = PI/3.5;

	scene->antialiasDepth =  antialias;
	scene->rayTraceDepth  =  rayTraceDepth;

	while((!feof(filePointer))&&(fscanf(filePointer, "%s", command)!=-1))
	{
		//fscanf(filePointer, "%s", command);

		printf("Reading %s\n", command);

		if(!strcmp(command,SPHERE))
		{
			//read in sphere
			float x,y,z,r;
			char * bitmapFile = (char *) malloc( sizeof(char)*100);

			fscanf(filePointer, "%f", &x);
			fscanf(filePointer, "%f", &y);
			fscanf(filePointer, "%f", &z);
			fscanf(filePointer, "%f", &r);
			fscanf(filePointer, "%s", bitmapFile);

			if(!strcmp(bitmapFile, "NULL"))
			{
				free(bitmapFile);
				bitmapFile = NULL;
			}

			scene->addSphere(x,y,z,r, bitmapFile);
		}
		else if(!strcmp(command,POLYGON))
		{
			//read in polygon
			float numPolygons;
			Polygon3 * newPolygon;

			fscanf(filePointer, "%f", &numPolygons);

			newPolygon = new Polygon3();

			for(int i =0;i<numPolygons;i++)
			{
				float x,y,z;

				fscanf(filePointer, "%f", &x);
				fscanf(filePointer, "%f", &y);
				fscanf(filePointer, "%f", &z);

				newPolygon->addPoint(x,y,z);
			}

			scene->addPolygon(newPolygon);
		}
		else if(!strcmp(command,CYLINDER))
		{
			//read in cylinder
			float x,y,z,r,h;

			fscanf(filePointer, "%f", &x);
			fscanf(filePointer, "%f", &y);
			fscanf(filePointer, "%f", &z);
			fscanf(filePointer, "%f", &r);
			fscanf(filePointer, "%f", &h);

			scene->addCylinder(x,y,z,r,h);

		}
		else if(!strcmp(command,LIGHTING))
		{
			//read in lighting
			//this describes the properties of an object, not a light
			float r,g,b;//colors
			float diffuseCoefficient, specularCoefficient;

			fscanf(filePointer, "%f", &r);
			fscanf(filePointer, "%f", &g);
			fscanf(filePointer, "%f", &b);
			fscanf(filePointer, "%f", &diffuseCoefficient);
			fscanf(filePointer, "%f", &specularCoefficient);

			scene->setLighting(r,g,b,specularCoefficient,diffuseCoefficient);
		}
		else if(!strcmp(command,LIGHT))
		{
			float x,y,z,r,g,b;

			fscanf(filePointer, "%f", &x);
			fscanf(filePointer, "%f", &y);
			fscanf(filePointer, "%f", &z);
			fscanf(filePointer, "%f", &r);
			fscanf(filePointer, "%f", &g);
			fscanf(filePointer, "%f", &b);

			scene->addLightSource(x,y,z,r,g,b);
		}
		else if(!strcmp(command,BOTTLE))
		{
			float x,y,z,r,h;
			float bx1,bx2,bx3,bx4;
			float by1,by2,by3,by4;

			fscanf(filePointer, "%f", &x);
			fscanf(filePointer, "%f", &y);
			fscanf(filePointer, "%f", &z);
			fscanf(filePointer, "%f", &r);
			fscanf(filePointer, "%f", &h);
			fscanf(filePointer, "%f", &bx1);
			fscanf(filePointer, "%f", &by1);
			fscanf(filePointer, "%f", &bx2);
			fscanf(filePointer, "%f", &by2);
			fscanf(filePointer, "%f", &bx3);
			fscanf(filePointer, "%f", &by3);
			fscanf(filePointer, "%f", &bx4);
			fscanf(filePointer, "%f", &by4);

			Bottle * newBottle = new Bottle();

			newBottle->bezierControlPoints[0].x = bx1;
			newBottle->bezierControlPoints[0].y = by1;
			newBottle->bezierControlPoints[1].x = bx2;
			newBottle->bezierControlPoints[1].y = by2;
			newBottle->bezierControlPoints[2].x = bx3;
			newBottle->bezierControlPoints[2].y = by3;
			newBottle->bezierControlPoints[3].x = bx4;
			newBottle->bezierControlPoints[3].y = by4;

			newBottle->height = h;
			newBottle->radius = r;
			newBottle->location.x = x;
			newBottle->location.y = y;
			newBottle->location.z = z;

			newBottle->meshItUp();//create polygon mesh

			scene->addBottle(newBottle);
		}
		else if(!strcmp(command,REFRACTION))
		{
			float transmissionCoefficient, materialConstant;
			
			fscanf(filePointer, "%f", &transmissionCoefficient);
			fscanf(filePointer, "%f", &materialConstant);

			scene->setRefraction(transmissionCoefficient,materialConstant);
		}
		else if(!strcmp(command,REFLECTION))
		{
			float reflectiveCoefficient;

			fscanf(filePointer, "%f", &reflectiveCoefficient);

			scene->setReflection(reflectiveCoefficient);
		}
		else throw "unknown symbol found in file";

	}

	//Read in the information
	//fscanf (filePointer, "%f", &f);

	fclose(filePointer);

}


void display()
{
	//stuff
	glClear(GL_COLOR_BUFFER_BIT);
	
	glBegin(GL_POINTS);
	
	for(int x=0;x< screenData.width;x++)
	{
		for(int y = 0;y<screenData.height;y++)
		{
			ColorRGB * colorOfCurrentPixel = screenData.getPixel(x,y);
			glColor3f(colorOfCurrentPixel->red, colorOfCurrentPixel->green, colorOfCurrentPixel->blue);
			glVertex2f(x,y);
		}
	
	}
	glEnd();

	glutSwapBuffers();
}

void reshape(int w, int h)
{
	glViewport(0,0,(GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,w,h,0,-1,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}