/*

	RayTrace.h

	data types
	and function prototypes for doing rendering calculations




  */
#ifndef RAY_TRACE_H
	#define RAY_TRACE_H

#include <vector>//for book keeping
#include "Shapes.h"
#include "Settings.h"


//describes the geometry and properties of the scene
struct Scene
{
	//Current propreties(given to objects as they are added OpenGL style)
	float currentRelfectiveCoefficient;
	float currentMaterialConstant;
	float currentTransmissionCoefficient;
	float currentSpecular;
	float currentDiffuse;
	ColorRGB currentObjectColor;

	int antialiasDepth;
	int rayTraceDepth;

	//objects and light sources
	BottleList	   * bottles;
	PolyhedronList * polyhedrons;
	LightSourceList* lightSources;
	CylinderList   * cylinders;
	Polygon3List   * polygons; // also includes polygons from the bottle and the polyhedron
	SphereList     * spheres;
	FlatCircleList * circles;

	//Specify pyramid from eyepoint to the screen
	Vector3 cameraLocation;
	Vector3 cameraLookat;///given
	//Vector3 cameraDirectionNormalized;//calculated
	float cameraSpanLeftRight;
	float cameraSpanUpDown;

	int numPixelsHorizontal;
	int numPixelsVertical;

	ColorRGB ambientTerm;
	ColorRGB bogusTerm;

	//methods
	void init();
	void addSphere(float x, float y, float z, float r, char * textureFile);
	void addLightSource(float x, float y, float z, float r, float g, float b);
	void addCylinder(float x, float y, float z, float r, float h);
	void addPolygon(Polygon3 * newPolygon);
	void addPolyhedron(Polyhedron * newPolyhedron);
	void addBottle(Bottle * newBottle);
	void addFlatCircle(float x, float y, float z, float r, bool up);
	void setRefraction(float transmissionCoefficient, float materialConstant)
	{
		currentTransmissionCoefficient = transmissionCoefficient;
		currentMaterialConstant        = materialConstant;
	}
	void setReflection(float reflectionCoefficient)
	{
		currentRelfectiveCoefficient	= reflectionCoefficient;
	}
	void setLighting(float r, float g, float b, float spec, float diff)
	{
		currentSpecular				= spec;
		currentDiffuse				= diff;

		currentObjectColor.red  =r;
		currentObjectColor.green=g;
		currentObjectColor.blue =b;

		//temp
		//if(currentTransmissionCoefficient >0)
		//	throw "refraction not implemented";
	}

};


//Takes a scene and generates the image
void renderScene(struct Scene *scene,struct ScreenData* screenData);
void renderScene(struct Scene *scene,struct ScreenData* screenData, int depth);//user may specify specific depth


//'private' functions
Ray * calculateRayFromEyePointToPointOnScreen(struct Scene *scene, Vector3 *leftTopCorner, Vector3 *bottomRightCorner, Vector3 * bottomLeftCorner,float percentToRight,float percentDown);//used to generate the initial rays
ColorRGB antiAliasTrace(int raysAcross, int x, int y, struct Scene *scene, Vector3 *leftTopCorner, Vector3 *bottomRightCorner,Vector3 * bottomLeftCorner, int depth);
ColorRGB traceRay(struct Scene *scene,struct Ray *ray, int depth, SceneObject * objectToExclude);
ColorRGB shade(struct Scene *scene,SceneObject * object, struct Ray *ray, Vector3 * intersectionPoint, Vector3 * normal, int depth);
/*
	closestIntersection	:
		returnFirst - if true, don't bother getting closest
		backFaceCull- if true, only get polygons facing
		intersection- caller allocates mem for the point and this function will store the intersection point in it
  */
SceneObject * closestIntersection(Scene * scene, Ray * ray, SceneObject * objectToIgnore, bool returnFirst, bool backFaceCull,Vector3 *intersection);

Ray * reflectiveRayFromIntersection(Ray * incomingRay, Vector3 * normal, Vector3 * intersectionPoint);
Ray * refractiveRayFromIntersection(Ray * incomingRay, Vector3 * normal, Vector3 * intersectionPoint, float sourceMaterialConstant, float destinationMaterialConstant);



Vector3 * getUpperLeft(struct Scene *scene);
Vector3 * getBottomRight(struct Scene *scene);
Vector3 * getBottomLeft(struct Scene *scene);
//bool leftSideOfRay(x1,y1,x2,y2, px,py);




#endif











