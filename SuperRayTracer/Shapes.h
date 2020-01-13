/*

  Data structures and datatypes for book-keeping of shapes







  */

#ifndef SHAPES_H
	#define SHAPES_H

#include <vector>//for book keeping
#include <math.h>
#include "Settings.h"
#include "readImage_BMP.h"
#include <GL/glut.h>

#define MAX_VERTICIES_ON_POLYGON 10

static int instanceOfVector3 = 0;//need to find memory leaks, not really relevant code
static int instanceOfRay     = 0;
static int instanceOfColor   = 0;

extern unsigned short int imageHolder_red[501][501]; // hold image info
extern unsigned short int imageHolder_green[501][501];
extern unsigned short int imageHolder_blue[501][501];
const unsigned int imageWidth = 128; // image width, number of pixels.
const unsigned int imageHeight = 128; // image height, number of pixels.


class Vector3
{
	public:
		Vector3(){ }// instanceOfVector3++;	}
		Vector3(float x, float y, float z) {this->x = x; this->y = y; this->z = z; }//instanceOfVector3++;}

		//Vector3 * multiply(float b);
		float x,y,z;
		//void translate(float ax, float ay, float az);
		void rotatePointAboutAxis(float angle, float x_rotate,float y_rotate,float z_rotate);//rotate the point about the axis given by 'angle' amount
		void print();

		Vector3 * copy();

		~Vector3(){ }//instanceOfVector3--;}
};

typedef std::vector<Vector3    * > Vector3List;

float angleBetweenRays(Vector3* a, Vector3* m, Vector3* b);
bool inTrangle(Vector3 * retVal,Vector3 * p1,Vector3 * p2,Vector3 * p3);
bool inPolygon(Vector3 ** points, int numPoints, Vector3 * vertex);

//vector related functions
Vector3 add(Vector3 a, Vector3 b);
Vector3 multiply(Vector3 a, float b);
float dotProduct(Vector3 * a, Vector3 *b);
Vector3 * normalToPlane(Vector3 * p1, Vector3 * p2, Vector3 * p3);
float distance(Vector3 * p1, Vector3 * p2);
float distance2(Vector3 * p1, Vector3 * p2);//does not do the sqrt for efficiency
Vector3 * normalizeVector(Vector3 * p);//returns itself for convenience
Vector3 crossProduct(Vector3 a, Vector3 b);
Vector3 subtract(Vector3 a, Vector3 b);

struct Vector2
{
	float x,y;
};

typedef std::vector<Vector2    * > Vector2List;

struct Ray
{
	Vector3 *location;
	Vector3 *direction;

	Ray() { }//instanceOfRay ++;}
	~Ray(){ }//instanceOfRay--;}
};

class ColorRGB
{
	public:
		float red;
		float green;
		float blue;

		ColorRGB() { }//instanceOfColor++;}
		~ColorRGB() { }//instanceOfColor--;}

		ColorRGB *copy()
		{
			ColorRGB * c  = (ColorRGB *)malloc(sizeof(ColorRGB));
			c->blue  = this->blue;
			c->green = this->green;
			c->red   = this->red;

			return c;
		}
};

class SceneObject
{
	public:
		//common properties
		float relfectiveCoefficient;
		float transmissionCoefficient;
		float specular;
		float diffuse;
		float materialConstant;//for refraction

		ColorRGB objectColor;

		bool textured;//if the object is textured, colorAt(Vector3 * intersectionPoint) will have to be called to determin the color.
		GLubyte image[imageWidth][imageHeight][4];

		//virtual methods, all objects need to perform these
		virtual Vector3 *intersectionPoint(Ray * ray, bool furthest) =0;//finds intersection of object and the given array, NULL if no intersection, 'furthest' means take the furthest possible intersection point(used for refraction)
		virtual Vector3 *intersectionPointAssumeIntersection(Ray * ray) =0;//does not do bound check in case of polygon, for all other objects, code is same as intersectionPoint(ray,..)
		virtual bool intersectPyramid(Ray *,Ray *,Ray *,Ray *, float distance, bool doInitialCheck) = 0;//determines whether the object is in the given volume described by 4 rays and a distance, not an exact calculation
		virtual Vector3 normalAt(Vector3 *intersectionPoint)=0;
		virtual ColorRGB colorAt(Vector3 * intersectionPoint)=0;
		void setLighting(float r, float g, float b, float refl, float refr, float materialRefractionConstant, float spec, float diff, char * textureFile)
		{
			relfectiveCoefficient	= refl;
			transmissionCoefficient = refr;
			specular				= spec;
			diffuse					= diff;
			materialConstant		= materialRefractionConstant;

			objectColor.red  =r;
			objectColor.green=g;
			objectColor.blue =b;

			if(textureFile!=NULL)
			{
				if ( !readImage_BMP(textureFile) ) // reading file fails
					exit(-1);

				int i, j;
				for (i = 0; i < imageHeight; i++) 
				{
					for (j = 0; j < imageWidth; j++) 
					{
						image[i][j][0] = (GLubyte) imageHolder_red[i][j];
						image[i][j][1] = (GLubyte) imageHolder_green[i][j];
						image[i][j][2] = (GLubyte) imageHolder_blue[i][j];
						image[i][j][3] = (GLubyte) 255;
					}
				}

				textured = true;
			}
			else
				textured = false;
		}
};


struct ScreenData
{
	private:
		ColorRGB * pixelData;//stores an array of pixel data, ordered by row: pixelData[0] is first row first column, pixelData[1], is 1st row, 2nd column

	public:
		int width;
		int height;

		void init(int width, int height);
		ColorRGB * getPixel(int x, int y);
		void setPixel(int x, int y, ColorRGB color);
		void setPixel(int x, int y, float red, float green, float blue);
};

class Sphere: public SceneObject
{
	public:
		//fields
		Vector3 location;
		float   radius;

		//methods
		Vector3 *intersectionPoint(Ray * ray, bool furthest);
		bool intersectPyramid(Ray *,Ray *,Ray *,Ray *, float distance, bool doInitialCheck);
		Vector3 normalAt(Vector3 *intersectionPoint);
		ColorRGB colorAt(Vector3 * intersectionPoint);
		Vector3 *intersectionPointAssumeIntersection(Ray * ray);
};

class Cylinder: public SceneObject
{
	public:
		//field
		Vector3 p1;
		float radius;

		//For now only use p1,radius and height
		float height;

		//methods
		Vector3 *intersectionPoint(Ray * ray, bool furthest);
		bool intersectPyramid(Ray *,Ray *,Ray *,Ray *, float distance, bool doInitialCheck);
		Vector3 normalAt(Vector3 *intersectionPoint);
		ColorRGB colorAt(Vector3 * intersectionPoint);
		Vector3 *intersectionPointAssumeIntersection(Ray * ray);
};


class FlatCircle: public SceneObject
{
	public:
		Vector3 location;
		float radius;
		Vector3 normal;//needed to indicate which side is face up

		//methods
		Vector3 *intersectionPoint(Ray * ray, bool furthest);
		bool intersectPyramid(Ray *,Ray *,Ray *,Ray *, float distance, bool doInitialCheck);
		Vector3 normalAt(Vector3 *intersectionPoint);
		ColorRGB colorAt(Vector3 * intersectionPoint);
		Vector3 *intersectionPointAssumeIntersection(Ray * ray);
};

class Polygon3: public SceneObject
{
	//Polygons will be one sided because of normal calculations

	public:
		//field
		int numberOfVerticies;
		Vector3 **pointList;
		Vector3 normal;//normal is saved
		float d;//a constant in the equation for a plane that the polygon is in

		float minx, maxx, miny, maxy,minz, maxz;//for quickly checking intersections

		//methods
		Polygon3()
		{
			pointList = (Vector3 **)malloc(sizeof(Vector3*)*MAX_VERTICIES_ON_POLYGON); 
			numberOfVerticies=0;
			objectColor.red  =0.5;
			objectColor.green=0;
			objectColor.blue =0.5;
		}

		Vector3 *intersectionPoint(Ray * ray, bool furthest);
		bool intersectPyramid(Ray *,Ray *,Ray *,Ray *, float distance, bool doInitialCheck);
		Vector3 normalAt(Vector3 *intersectionPoint);
		ColorRGB colorAt(Vector3 * intersectionPoint);
		Vector3 *intersectionPointAssumeIntersection(Ray * ray);

		void addPoint(float x,float y, float z);
		void addPoint(Vector3 * point);
};

typedef std::vector<Polygon3   * > Polygon3List;

class Bottle//DO NOT inherit from SceneObject,needs to be translated to polygons
{
	public:
		Bottle()
		{
			polygonList = new Polygon3List();
		}

		Vector3 location;
		Vector2 bezierControlPoints[4];//describe the curves in 2d, projected off of center, and then rotated around
		float height;//the bezier curve is scaled to be a certain height
		float radius;//the bezier curve is scaled to be a certain width
		float maxRadius;//needed to calculate bounding volume, calculated during 'meshItUp'
		float topRadius;//used to generate the cap of the bottle;

		bool rayThroughBoundingVolume(Ray * ray); //use cylinder as bounding volume

		void meshItUp();//create polygon mesh for the bottle
		Polygon3List * polygonList;//after meshitup is called, the mesh will be converted to polygons and then put in this list

		Cylinder * boundingVolume;
};

struct LightSource
{
	Vector3  location;
	ColorRGB color;
	//float intensity;
};

typedef std::vector<Cylinder   * > CylinderList;
typedef std::vector<Bottle     * > BottleList;
typedef std::vector<Sphere     * > SphereList;
typedef std::vector<LightSource * > LightSourceList;
typedef std::vector<FlatCircle * > FlatCircleList;

struct Polyhedron//NOT inherited from scene object, needs to be translated to polygons
{
	Polygon3List *faces;

	Polyhedron()
	{
		faces = new Polygon3List();
	}

	void addPolygon(Polygon3 * newPolygon);

};

typedef std::vector<Polyhedron * > PolyhedronList;

//this is used for capping the bottle and the cylinder, only those parallel to zx plane
Vector3 * intersectionWithCircle(Ray * ray, float x, float y, float z,float r);
float distanceToIntersection(SceneObject * object, Ray * ray);

#endif