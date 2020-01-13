/*

	implementation of shape stuff defined in Shapes.h





*/

#include "Shapes.h"


void Polyhedron::addPolygon(Polygon3 * newPolygon)
{
	this->faces->push_back(newPolygon);
}

bool Bottle::rayThroughBoundingVolume(Ray * ray)
{
	//returns true if the ray passes near the bottle
	//false if there is no possible way that the ray intersects the bottle
	
/*	float p0z = (ray->location->z - location.z);
	float p0x = (ray->location->x - location.x);
	float Vz  = ray->direction->z;
	float Vx  = ray->direction->x;
	float r   = this->maxRadius;

	//at2 + bt + c = 0
	float a = Vz*Vz + Vx*Vx;
	float b = 2*p0z*Vz + 2*p0x*Vx;
	float c = p0z*p0z + p0x*p0x - r*r;

	float det = b*b - 4*a*c;

	if (det<0)
		return false;
	
	//solve for t
	float sqrtdet = sqrt(det);
	float t1 = (-b + sqrtdet)/(2*a);
	float t2 = (-b - sqrtdet)/(2*a);

	//we need a t that is positve, and such that y in the proper bounts
	float y1 = ray->location->y + t1*ray->direction->y;
	float y2 = ray->location->y + t2*ray->direction->y;

	bool y1Valid = ((y1<this->location.y + this->height) && (y1>this->location.y) && (t1 >0));
	bool y2Valid = ((y2<this->location.y + this->height) && (y2>this->location.y) && (t2 >0));

	return ((y1Valid)||(y2Valid));

*/
	Vector3* intersection = boundingVolume->intersectionPoint(ray, false);

	if(intersection==NULL)
	{
		return false;
	}
	else
	{
		delete intersection;
		return true;
	}
}


void Bottle::meshItUp()
{
	// use bezier points to create a list of points on a curve
	//      project these off the side, 
	//      then rotate these around the center

	//1. Step one, use bezierControlPoints to create bezierPoints
	Vector2 * bezierPoints = new Vector2[NUM_BEZIER_SAMPLES];
	Vector2 p1 = this->bezierControlPoints[0];
	Vector2 p2 = this->bezierControlPoints[1];
	Vector2 p3 = this->bezierControlPoints[2];
	Vector2 p4 = this->bezierControlPoints[3];

	//Calculate Cx and Cy
	for(int i= 0; i <NUM_BEZIER_SAMPLES; i++)
	{
		float t =((float)i)/NUM_BEZIER_SAMPLES;

		float x,y;

		float t3 = t*t*t;
		float t2 = t*t;

		float Cx1 = -p1.x +3*p2.x -3*p3.x + p4.x;
		float Cx2 = 3*p1.x -6*p2.x +3*p3.x;
		float Cx3 = -3*p1.x +3*p2.x;
		float Cx4 = p1.x ;

		float Cy1 = -p1.y +3*p2.y -3*p3.y + p4.y;
		float Cy2 = 3*p1.y -6*p2.y +3*p3.y;
		float Cy3 = -3*p1.y +3*p2.y;
		float Cy4 = p1.y ;

		x = Cx1*t3 + Cx2*t2 + Cx3*t+Cx4;
		y = Cy1*t3 + Cy2*t2 + Cy3*t+Cy4;
		
		bezierPoints[i].x = x;
		bezierPoints[i].y = y;
	}

	/*2. Step two: scale and then project off the side
		
		need to scale/translate such that 
		
		bezierPoints[0].y = radius
		bezierPoints[0].x = 0
		bezuerPoints[NUM_BEZIER_SAMPLES-1].x = height

	*/
	//translate
	for(i =1;i<NUM_BEZIER_SAMPLES;i++)
		bezierPoints[i].x = bezierPoints[i].x - bezierPoints[0].x;

	bezierPoints[0].x = 0;
	//scale the x coord ST bezuerPoints[NUM_BEZIER_SAMPLES-1].x = height
	//                     o * m = height
	float multX = this->height /bezierPoints[NUM_BEZIER_SAMPLES-1].x;
	for(i =0;i<NUM_BEZIER_SAMPLES;i++)
		bezierPoints[i].x = bezierPoints[i].x * multX;
	float multY = this->radius /bezierPoints[0].y;
	for(i =0;i<NUM_BEZIER_SAMPLES;i++)
		bezierPoints[i].y = bezierPoints[i].y * multY;

	/*
		convert to real world coords located at center
		
	*/
	Vector3 * strip = new Vector3[NUM_BEZIER_SAMPLES];
	maxRadius = 0;//used in bounding volume calculation

	for(i =0;i<NUM_BEZIER_SAMPLES;i++)
	{
		strip[i].x = 0;
		strip[i].y = bezierPoints[i].x;
		strip[i].z = bezierPoints[i].y;

		if(bezierPoints[i].y>maxRadius)
			maxRadius = bezierPoints[i].y;
	}

	topRadius = bezierPoints[NUM_BEZIER_SAMPLES-1].y;

	/*
		now generate polygons using the strip by rotating the points on the strip

		axis of rotation is located at origin,

		translate at the end
	*/

	for(int r = 0 ; r < ROTATIONS; r ++)
	//for(int r = 0 ; r < 3; r ++)
	{
		//for(int v =0; v< 5; v++)
		for(int v =0; v< NUM_BEZIER_SAMPLES -1; v++)
		{
			/*Take 2 points, copy them twice
			  rotate 2 by r, other by r+1
			  translate, then add polygon
			
			   May have to change order to deal with one sidedness of polygon

			*/
			float angle1,angle2;
			Vector3 p1,p2,p3,p4;
			p1 = strip[v];
			p2 = strip[v+1];
			p3 = p1;
			p4 = p2;

			//rotate
			angle1 = 2*PI*(((float)r)/ROTATIONS);
			angle2 = 2*PI*(((float)r+1)/ROTATIONS);

			if(r+1==ROTATIONS)
				angle2 = 0;

			p1.rotatePointAboutAxis(angle1,0,1,0);
			p2.rotatePointAboutAxis(angle1,0,1,0);
			p3.rotatePointAboutAxis(angle2,0,1,0);
			p4.rotatePointAboutAxis(angle2,0,1,0);

			//now create polygon and add
			Polygon3 * polygon = new Polygon3();
			polygon->addPoint(p1.x+location.x,p1.y+location.y,p1.z+location.z);
			polygon->addPoint(p2.x+location.x,p2.y+location.y,p2.z+location.z);
			polygon->addPoint(p4.x+location.x,p4.y+location.y,p4.z+location.z);
			polygon->addPoint(p3.x+location.x,p3.y+location.y,p3.z+location.z);

			this->polygonList->push_back(polygon);

		}//for
	}//for


	delete[] strip;
	delete[] bezierPoints;


	//create bounding volume
	boundingVolume = new Cylinder();
	boundingVolume->p1 = location;
	boundingVolume->radius= maxRadius;
	boundingVolume->height= height;

}//meshItUp

Vector3 crossProduct(Vector3 a, Vector3 b)
{
	Vector3 ret(a.y*b.z - a.z*b.y,a.z*b.x - a.x*b.z,a.x*b.y - a.y*b.x);

//	ret.x = a.y*b.z - a.z*b.y;
//	ret.y = a.z*b.x - a.x*b.z;
//	ret.z = a.x*b.y - a.y*b.x;

	return ret;
}

bool inPolygon(Vector3 ** points, int numPoints, Vector3 * vertex)
{
	//same algorithm as in triangle, make sure vertex is on the same side
	//of each edge

	float s3x,s3y,s3z,s1x,s1y,s1z;

	//v1 = 0
	int v2,v3;
	v2 = 1;
	v3 = 2;

	//Check first sides first
	float v2x = points[v2]->x;
	float v2y = points[v2]->y;
	float v2z = points[v2]->z;

	s3x  = points[0]->x - v2x;//points[v2]->x;
	s3y  = points[0]->y - v2y;//points[v2]->y;
	s3z  = points[0]->z - v2z;//points[v2]->z;

	s1x  = points[v3]->x - v2x;//points[v2]->x;
	s1y  = points[v3]->y - v2y;//points[v2]->y;
	s1z  = points[v3]->z - v2z;//points[v2]->z;

	float s2x  = vertex->x - v2x;//points[v2]->x;
	float s2y  = vertex->y - v2y;//points[v2]->y;
	float s2z  = vertex->z - v2z;//points[v2]->z;

	if ((s1y*s2z - s1z*s2y)*(s1y*s3z - s1z*s3y)+(s1z*s2x - s1x*s2z)*(s1z*s3x - s1x*s3z)+(s1x*s3y - s1y*s3x)*(s1x*s2y - s1y*s2x) < 0)
		return false;

	//Then loop
	for(int v1 =1;v1<numPoints;v1++)
	{
		//	function SameSide(p1,p2, p3,p4)
		//      (DotProduct(CrossProduct(p4-p3, p1-p3), CrossProduct(p4-p3, p2-p3)) >= 0)

		v2 = v1+1;

		if(v1+2==numPoints)
			v3 = 0;
		else
			if(v1+1==numPoints)
		{
			v3 = 1;
			v2 = 0;
		}
		else
			v3 = v1+2;

		/*Check that vertex same side of (v1)-(v2) as (v2,v3)

  			function SameSide(p1,p2, p3,p4)
				(DotProduct(CrossProduct(p4-p3, p1-p3), CrossProduct(p4-p3, p2-p3)) >= 0)
			
  			function SameSide(p1,p3, p4,p5)
				(DotProduct(CrossProduct(p5-p4, p1-p4), CrossProduct(p5-p4, p3-p4)) >= 0)

			this means s3 on the next is the opposite of s1 on the previous


			function PointInTriangle(i, p1,p2,p3)
				if SameSide(i,p1, p2,p3) and SameSide(i,p2, p1,p3)
					and SameSide(i,p3, p1,p2) then return true


		*/

		//if (!SameSide(vertex, points[v1],points[v2],points[v3]))
		//		return false;

		//13 seconds

		//Vector3 s1 = subtract(*points[v3],*points[v2]);
		//Vector3 s2 = subtract(*vertex,    *points[v2]);
		//Vector3 s3 = subtract(*points[v1],*points[v2]);

		float v2x = points[v2]->x;
		float v2y = points[v2]->y;
		float v2z = points[v2]->z;

		s3x = -s1x;
		s3y = -s1y;
		s3z = -s1z;

		s1x  = points[v3]->x - v2x;//points[v2]->x;
		s1y  = points[v3]->y - v2y;//points[v2]->y;
		s1z  = points[v3]->z - v2z;//points[v2]->z;

		float s2x  = vertex->x - v2x;//points[v2]->x;
		float s2y  = vertex->y - v2y;//points[v2]->y;
		float s2z  = vertex->z - v2z;//points[v2]->z;

		//if(!(dotProduct(&crossProduct(s1,s2),&crossProduct(s1,s3))>0))
		//	return false;

		//if ((s1.y*s2.z - s1.z*s2.y)*(s1.y*s3.z - s1.z*s3.y)+(s1.z*s2.x - s1.x*s2.z)*(s1.z*s3.x - s1.x*s3.z)+(s1.x*s3.y - s1.y*s3.x)*(s1.x*s2.y - s1.y*s2.x) < 0)
		//	return false;

		if ((s1y*s2z - s1z*s2y)*(s1y*s3z - s1z*s3y)+(s1z*s2x - s1x*s2z)*(s1z*s3x - s1x*s3z)+(s1x*s3y - s1y*s3x)*(s1x*s2y - s1y*s2x) < 0)
			return false;

	}//for

	return true;

}//inPolygon


bool inTrangle(Vector3 * intersection,Vector3 * p1,Vector3 * p2,Vector3 * p3)
{
	/*
	     -->       -->        -->
		(V0P) = a (V0V1) + b (V0V2)
    */

	/*
	function SameSide(p1,p2, p3,p4)
		(DotProduct(CrossProduct(p4-p3, p1-p3), CrossProduct(p4-p3, p2-p3)) >= 0)
	

	function PointInTriangle(i, p1,p2,p3)
		if SameSide(i,p1, p2,p3) and SameSide(i,p2, p1,p3)
			and SameSide(i,p3, p1,p2) then return true
		else return false
	*/
	
	//Vector3 cp1,cp2;//cross products
	Vector3 s1,s2,s3;

	s1.x = p3->x - p2->x;
	s1.y = p3->y - p2->y;
	s1.z = p3->z - p2->z;

	s2.x = intersection->x - p2->x;
	s2.y = intersection->y - p2->y;
	s2.z = intersection->z - p2->z;

	s3.x = p1->x - p2->x;
	s3.y = p1->y - p2->y;
	s3.z = p1->z - p2->z;

	/*
	//cp1 = s1 x s2
	cp1.x=s1.y*s2.z - s1.z*s2.y;
	cp1.y=s1.z*s2.x - s1.x*s2.z;
	cp1.z=s1.x*s2.y - s1.y*s2.x;

	//cp2 = s1 x s3
	cp2.x=s1.y*s3.z - s1.z*s3.y;
	cp2.y=s1.z*s3.x - s1.x*s3.z;
	cp2.z=s1.x*s3.y - s1.y*s3.x;
	*/
	//positive1 = (dotProduct(&crossProduct(s1, s2), &crossProduct(s1, s3)) >= 0);
	//positive1 = (dotProduct(&cp1, &cp2) >= 0);
	//if (cp1.x*cp2.x+cp1.y*cp2.y+cp1.z*cp2.z < 0)
	//	return false;

	if ((s1.y*s2.z - s1.z*s2.y)*(s1.y*s3.z - s1.z*s3.y)+(s1.z*s2.x - s1.x*s2.z)*(s1.z*s3.x - s1.x*s3.z)+(s1.x*s3.y - s1.y*s3.x)*(s1.x*s2.y - s1.y*s2.x) < 0)
		return false;


	s1.x = p3->x - p1->x;
	s1.y = p3->y - p1->y;
	s1.z = p3->z - p1->z;

	s2.x = intersection->x - p1->x;
	s2.y = intersection->y - p1->y;
	s2.z = intersection->z - p1->z;

	s3.x = p2->x - p1->x;
	s3.y = p2->y - p1->y;
	s3.z = p2->z - p1->z;

	/*cp1 = s1 x s2
	cp1.x=s1.y*s2.z - s1.z*s2.y;
	cp1.y=s1.z*s2.x - s1.x*s2.z;
	cp1.z=s1.x*s2.y - s1.y*s2.x;

	//cp2 = s1 x s3
	cp2.x=s1.y*s3.z - s1.z*s3.y;
	cp2.y=s1.z*s3.x - s1.x*s3.z;
	cp2.z=s1.x*s3.y - s1.y*s3.x;
	*/
	//positive2 = (dotProduct(&crossProduct(subtract(pp4,pp3), subtract(pp1,pp3)), &crossProduct(subtract(pp4,pp3), subtract(pp2,pp3))) >= 0);
	//positive2 = (dotProduct(&crossProduct(s1, s2), &crossProduct(s1, s3)) >= 0);
	//if (cp1.x*cp2.x+cp1.y*cp2.y+cp1.z*cp2.z < 0)
	//	return false;

	if ((s1.y*s2.z - s1.z*s2.y)*(s1.y*s3.z - s1.z*s3.y)+(s1.z*s2.x - s1.x*s2.z)*(s1.z*s3.x - s1.x*s3.z)+(s1.x*s2.y - s1.y*s2.x)*(s1.x*s3.y - s1.y*s3.x) < 0)
		return false;

	//return (positive1==positive2);

	//positive3 = (dotProduct(&crossProduct(subtract(pp4,pp3), subtract(pp1,pp3)), &crossProduct(subtract(pp4,pp3), subtract(pp2,pp3))) >= 0);

	/*cp1 = s3 x s2
	cp1.x=s3.y*s2.z - s3.z*s2.y;
	cp1.y=s3.z*s2.x - s3.x*s2.z;
	cp1.z=s3.x*s2.y - s3.y*s2.x;

	//cp2 = s3 x s1
	cp2.x=s3.y*s1.z - s3.z*s1.y;
	cp2.y=s3.z*s1.x - s3.x*s1.z;
	cp2.z=s3.x*s1.y - s3.y*s1.x;
	*/
	//positive3 = (dotProduct(&crossProduct(s3, s2), &crossProduct(s3, s1)) >= 0);
	//if (cp1.x*cp2.x+cp1.y*cp2.y+cp1.z*cp2.z < 0)
	//	return false;

	return ((s3.y*s2.z - s3.z*s2.y)*(s3.y*s1.z - s3.z*s1.y)+(s3.z*s2.x - s3.x*s2.z)*(s3.z*s1.x - s3.x*s1.z)+(s3.x*s2.y - s3.y*s2.x)*(s3.x*s1.y - s3.y*s1.x) >=0);

	//return ((positive1==positive2)&&(positive1==positive3));

}//inTrangle


float angleBetweenRays(Vector3* a, Vector3* m, Vector3* b)
{
	
	//	m is the shared middle point

	Vector3 v1, v2;

	v1.x = a->x - m->x;
	v1.y = a->y - m->y;
	v1.z = a->z - m->z;

	v2.x = b->x - m->x;
	v2.y = b->y - m->y;
	v2.z = b->z - m->z;

	
	//	dot product = cos(angle) |v1| |v2|
	//	v1 * v2 = cos(angle) |v1| |v2|


	float sq1,sq2;

	sq1 = v1.x*v1.x + v1.y*v1.y + v1.z*v1.z;
	sq2 = v2.x*v2.x + v2.y*v2.y + v2.z*v2.z;

	float  magnitudes  = sqrt(sq1)*sqrt(sq2);
	float  dotProductV1V2= dotProduct(&v1, &v2);

	float cosAngle = (dotProductV1V2/magnitudes);

	return acos(cosAngle);
}

Vector3 * normalizeVector(Vector3 * p)//returns itself for convenience
{
	float d = p->x*p->x + p->y*p->y + p->z*p->z;
	d = sqrt(d);

	p->x = p->x/d;
	p->y = p->y/d;
	p->z = p->z/d;

	return p;
}

Vector3 add(Vector3 a, Vector3 b)
{
	Vector3 c;// = new Vector3();//(Vector3 *)malloc(sizeof(Vector3));
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;

	return c;
}

Vector3 subtract(Vector3 a, Vector3 b)
{
	Vector3 c;// = new Vector3();//(Vector3 *)malloc(sizeof(Vector3));
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;

	return c;
}


Vector3 * Vector3::copy()
{
	Vector3 *c = new Vector3();//(Vector3 *)malloc(sizeof(Vector3));

	c->x = this->x;
	c->y = this->y;
	c->z = this->z;

	return c;
}

void Vector3::print()
{

	printf("Vector ( %f , %f , %f )\n", x,y,z);
}

float dotProduct(Vector3 * a, Vector3 *b)
{
	return (a->x*b->x)+(a->y*b->y)+(a->z*b->z);
}

float distance(Vector3 * p1, Vector3 * p2)
{
	float sq = (p1->x-p2->x)*(p1->x-p2->x) +(p1->y-p2->y)*(p1->y-p2->y)+(p1->z-p2->z)*(p1->z-p2->z);

	return sqrt(sq);
}

float distance2(Vector3 * p1, Vector3 * p2)
{
	return (p1->x-p2->x)*(p1->x-p2->x) +(p1->y-p2->y)*(p1->y-p2->y)+(p1->z-p2->z)*(p1->z-p2->z);
}

Vector3 multiply(Vector3 a, float b)
{
	Vector3 c;// = new Vector3();//(Vector3 *)malloc(sizeof(Vector3));
	c.x = a.x * b;
	c.y = a.y * b;
	c.z = a.z * b;

	return c;
}

/*
Vector3 * Vector3::multiply(float b)
{
	this->x = this->x*b;
	this->y = this->y*b;
	this->z = this->z*b;

	return this;
}

void Vector3::translate(float ax, float ay, float az)
{
	this->x+=ax;
	this->y+=ay;
	this->z+=az;
}*/

void Vector3::rotatePointAboutAxis(float angle, float x_rotate,float y_rotate,float z_rotate)
{
    float newX, newY, newZ;
	float cosAngle, sinAngle;

	cosAngle = cos(angle);
	sinAngle = sin(angle);

	/*
		R =  [ r11 r21 r31
		       r12 r22 r32
			   r13 r23 r33]
  */

	float r11 = (cosAngle + x_rotate*x_rotate*(1-cosAngle));
	float r12 = x_rotate*y_rotate*(1-cosAngle) - z_rotate*sinAngle;
	float r13 = x_rotate*z_rotate*(1-cosAngle) + y_rotate*sinAngle;

	float r21 = x_rotate*y_rotate*(1-cosAngle) + z_rotate*sinAngle;
	float r22 = (cosAngle + y_rotate*y_rotate*(1-cosAngle));
	float r23 = y_rotate*z_rotate*(1-cosAngle) - x_rotate*sinAngle;

	float r31 = x_rotate*z_rotate*(1-cosAngle) - y_rotate*sinAngle;
	float r32 = y_rotate*z_rotate*(1-cosAngle) + x_rotate*sinAngle;
	float r33 = (cosAngle + z_rotate*z_rotate*(1-cosAngle));
	
	newX = r11*this->x + r21*this->y + r31*this->z;
	newY = r12*this->x + r22*this->y + r32*this->z;
	newZ = r13*this->x + r23*this->y + r33*this->z;

	this->x = newX;
	this->y = newY;
	this->z = newZ;

}//rotatePointAboutAxes

Vector3 * normalToPlane(Vector3 * p1, Vector3 * p2, Vector3 * p3)
{
	Vector3 *normal = new Vector3();

	float x1 = p1->x;
	float y1 = p1->y;
	float z1 = p1->z;
	float x2 = p2->x;
	float y2 = p2->y;
	float z2 = p2->z;
	float x3 = p3->x;
	float y3 = p3->y;
	float z3 = p3->z;

	normal->x = (z1 + z2)*(y1 - y2) + (z2 + z3)*(y2 - y3) + (z3 + z1)*(y3 - y1);
	normal->y = (z1 - z2)*(x1 + x2) + (z2 - z3)*(x2 + x3) + (z3 - z1)*(x3 + x1);
	normal->z = (y1 + y2)*(x1 - x2) + (y2 + y3)*(x2 - x3) + (y3 + y1)*(x3 - x1);

	return normal;
}//normalToPlane

//Basic functions for manipulating the screen data
void ScreenData::init(int width, int height)
{
	this->pixelData = (ColorRGB *)malloc(sizeof(ColorRGB)*width*height);
	this->width	 = width;
	this->height = height;

}

ColorRGB * ScreenData::getPixel(int x, int y)
{
	//(0,0) would be pixelData[0]
	//(1,0) would be pixelData[1]
	//(0,1) would be pixelData[width]

	if ((x>this->width)||(x<0))
		return NULL;

	if ((y>this->height)||(y<0))
		return NULL;

	return this->pixelData+x+y*(this->width);
}
void ScreenData::setPixel(int x, int y, ColorRGB color)
{
	ColorRGB * pixelColor = this->getPixel(x,y);

	//if(color ==NULL)
	//	throw "color is null";

	pixelColor->blue  = color.blue;
	pixelColor->red   = color.red;
	pixelColor->green = color.green;
}

void ScreenData::setPixel(int x, int y, float red, float green, float blue)
{
	ColorRGB * pixelColor = this->getPixel(x,y);

	pixelColor->blue  = blue;
	pixelColor->red   = red;
	pixelColor->green = green;
}


ColorRGB Sphere::colorAt(Vector3 * intersectionPoint)
{
	ColorRGB colorAt;

	//from GLubyte image[imageWidth][imageHeight][4];

	//need to find u and v 
	int u = 0;
	int v = 0;

	//scale u from the height of it
	//and   v frin the angle
	float x,y;//relative coords

	y = intersectionPoint->y  -  this->location.y;
	x = intersectionPoint->x  -  this->location.y;

	u = ((y+ this->radius)/(this->radius*2))*imageHeight;
	v = ((x+ this->radius)/(this->radius*2))*imageWidth;

	#ifndef TEXTURE_ANTIALIASING

	colorAt.red = ((float)(image[u][v][2]))/256;
	colorAt.blue= ((float)(image[u][v][0]))/256;
	colorAt.green=((float)(image[u][v][1]))/256;
	
	#endif

	#ifdef TEXTURE_ANTIALIASING

	colorAt.red = ((float)(image[u][v][2]))/1024;
	colorAt.blue= ((float)(image[u][v][0]))/1024;
	colorAt.green=((float)(image[u][v][1]))/1024;

	colorAt.red  += ((float)(image[u+1][v][2]))/1024;
	colorAt.blue += ((float)(image[u+1][v][0]))/1024;
	colorAt.green+= ((float)(image[u+1][v][1]))/1024;

	colorAt.red  += ((float)(image[u][v+1][2]))/1024;
	colorAt.blue += ((float)(image[u][v+1][0]))/1024;
	colorAt.green+= ((float)(image[u][v+1][1]))/1024;

	colorAt.red  += ((float)(image[u+1][v+1][2]))/1024;
	colorAt.blue += ((float)(image[u+1][v+1][0]))/1024;
	colorAt.green+= ((float)(image[u+1][v+1][1]))/1024;

	#endif

	return colorAt;
}//Sphere::colorAt

Vector3 * Sphere::intersectionPoint(Ray * ray, bool furthest)
{
	/*
		Let the ray be P(t) = Q + tV
		Let the sphere be (x-d)2 + (y-e)2 + (z-f)2 = r2
		
		x2 + y2 + z2 + 2xd + 2yd + 2zd - d2 - e2 - f2 - r2 = 0
		x = r->l->x + r->d->x
	
		(d,e,f) is sphere center

		(Qx + tVx - d)2 +(Qy + tVy - e)2 + (Qz + tVz - f)2 = r2

		(Qx -d + tVx)2 +(Qy  -e + tVy)2 + (Qz -f + tVz)2 = r2

		((Qx-d) + tVx)2 +((Qy -e) + tVy)2 + ((Qz-f) + tVz)2 = r2

		(Qx-d)2 + (tVx)2 + 2*(Qx-d)*(tVx) +.....  = r2

		((Qx-d)2 + (Qy-e)2 + (Qz-f)2 -r2) 
		                   + 2*((Qx-d)*Vx + (Qy-e)*Vy + (Qz-f)*Vz)*t
						                            + (Vx2 + Vy2 + Vz2) * t2 = 0
	*/
	
	float d,e,f, Vx, Vy, Vz, Qx, Qy, Qz,r;
	d = this->location.x;
	e = this->location.y;
	f = this->location.z;
	Vx= ray->direction->x;
	Vy= ray->direction->y;
	Vz= ray->direction->z;
	Qx= ray->location->x;
	Qy= ray->location->y;
	Qz= ray->location->z;
	r = this->radius;

	float a,b,c;//variables in quadratic equation to solve for t

	/*a = ray->direction->x*ray->direction->x + ray->direction->y*ray->direction->y  + ray->direction->z*ray->direction->z;
    b = 2*ray->direction->x*(ray->location->x - d) +2*ray->direction->y*(ray->location->y - e) + 2*ray->direction->z*(ray->location->z - f);
	c = (ray->location->x - d)*(ray->location->x - d) +
		(ray->location->y - e)*(ray->location->y - e) +
		(ray->location->z - f)*(ray->location->z - f)
		-this->radius*this->radius;
	*/

	a  = Vx*Vx + Vy*Vy + Vz*Vz;
	b  = 2*((Qx-d)*Vx + (Qy-e)*Vy + (Qz-f)*Vz);
	c  = (Qx-d)*(Qx-d) + (Qy-e)*(Qy-e) + (Qz-f)*(Qz-f) -r*r;

	/*
	at2 + bt + c = 0
	
	  t = (-b +/- sqrt(b*b - 4ac))/2*a
	*/

	float determinant = b*b - 4*a*c;

	if(determinant< 0)
	{

		//no intersection
		return NULL;
	}
	else
	{
		//t is incorrect
		float sqrtdet = sqrt(determinant);
		float t1 = (-b + sqrtdet)/(2*a);
		float t2 = (-b - sqrtdet)/(2*a);
		float t;

		if((t1<0)&&(t2<0))
			return NULL;

		//If only one point is valid, then our refraction algorithm requires that
		//we return a null here
		if((furthest)&&(  (t1<0) || (t2<0)))
			return NULL;


		if(t1<0)
			t = t2;
		else
		{
			if(t2<0)
				t = t1;
			else
			{
				if(furthest)
				{
					//choose larger
					if(t1<t2)
						t = t2;
					else
						t = t1;

				}
				else
				{
					//choose smaller
					if(t1<t2)
						t = t1;
					else
						t = t2;
				}
			}
		}

		/*
		Vector3 * intersectionPoint = new Vector3();
		intersectionPoint ->x = (ray->location->x) + t*(ray->direction->x);
		intersectionPoint ->y = (ray->location->y) + t*(ray->direction->y);
		intersectionPoint ->z = (ray->location->z) + t*(ray->direction->z);
		*/
		Vector3 * intersectionPoint = new Vector3((ray->location->x) + t*(ray->direction->x),
												(ray->location->y) + t*(ray->direction->y),
												(ray->location->z) + t*(ray->direction->z));

		return intersectionPoint;
	}//else
	
}//Sphere::intersectionPoint

Vector3  Sphere::normalAt(Vector3 *intersectionPoint)
{
	//from center to intersection point
	Vector3 normal;
	normal.x = (intersectionPoint->x - this->location.x);
	normal.y = (intersectionPoint->y - this->location.y);
	normal.z = (intersectionPoint->z - this->location.z);

	normalizeVector(&normal);

	return normal;

/*	return 
		normalizeVector(
		new Vector3(
		(intersectionPoint->x - this->location.x),
		(intersectionPoint->y - this->location.y),
		(intersectionPoint->z - this->location.z)));
*/
}//normalAt

ColorRGB Cylinder::colorAt(Vector3 * intersectionPoint)
{
	//texture mapping not implemented
	ColorRGB colorAt;

	colorAt.red = 0;
	colorAt.blue= 0;
	colorAt.green=0;

	return colorAt;
}

Vector3 * Cylinder::intersectionPoint(Ray * ray, bool furthest)
{
	/*

		Use distance from the line.
		If the 2 intersections are between two end caps(planes), then return

	formula for the line

    Vp = (V1-V2)
	P0 = V1
	P  = P0 + sVp

	distance between two lines

	Vq = ray->direction
	Q0 = ray->location
	Q  = Q0 + tVq

	formula stolen from a book for s and t	

    den = (Vp2)(Vq2) - (VpVq)2
	s =( (Q0 - P0)Vp * (Vq Vq) + (P0 - Q0)Vq *(Vp Vq) )/ den
	t =( (Q0 - P0)Vp * (Vp Vq) + (P0 - Q0)Vq *(Vp Vp) )/ den

  */

	
	/*

	For now, I'm just using simple cylinders where the bottom is parralell to xz plane

	p1.y < y < p1.y + height
	(z - p1.z)2 + (x-p1.x)2 = r2

	p = P0 + Vt

	(p0z + Vz*t - p1.z)2 + (p0x + Vx*t -p1.x)2 = r2
	((p0z - p1.z)2 + (p0x - p1.x)2) + (Vz2 + Vx2)t2 + (2*(p0z -p1.z)*Vz + 2*(p0x - p1.x)*Vx)*t = r2

  */

	//First check the caps

	float p0z = (ray->location->z - p1.z);
	float p0x = (ray->location->x - p1.x);
	float Vz  = ray->direction->z;
	float Vx  = ray->direction->x;
	float r   = this->radius;

	//at2 + bt + c = 0
	float a = Vz*Vz + Vx*Vx;
	float b = 2*p0z*Vz + 2*p0x*Vx;
	float c = p0z*p0z + p0x*p0x - r*r;

	float det = b*b - 4*a*c;

	if(det<0)
		return NULL;
	else
	{
		//solve for t
		float sqrtdet = sqrt(det);
		float t1 = (-b + sqrtdet)/(2*a);
		float t2 = (-b - sqrtdet)/(2*a);

		//we need a t that is positve, and such that y in the proper bounts
		float y1 = ray->location->y + t1*ray->direction->y;
		float y2 = ray->location->y + t2*ray->direction->y;

		//make sure the point is in front of the ray and lands within the right y vals
		bool y1Valid = ((y1<this->p1.y + this->height) && (y1>this->p1.y) && (t1 >0));
		bool y2Valid = ((y2<this->p1.y + this->height) && (y2>this->p1.y) && (t2 >0));

		bool useT1;

		if((!y1Valid)&&(!y2Valid))//neither are valid
				return NULL;

		if((y1Valid)&&(y2Valid))//both are valid
		{
			if(!furthest)
				useT1 = (t1<t2);
			else
				useT1 = (t1>t2);
		}
		else
		{
			//If only one point is valid, then our refraction algorithm requires that
			//we return a null here
			if(furthest)
				return NULL;

			useT1 = y1Valid;//only one is valid
		}

		float t;
		if(useT1)
			t = t1;
		else
			t = t2;
		
		Vector3 * intersectionPoint = new Vector3();
		intersectionPoint->x = ray->location->x + ray->direction->x * t;
		intersectionPoint->y = ray->location->y + ray->direction->y * t;
		intersectionPoint->z = ray->location->z + ray->direction->z * t;

		/*
			choose closest of
			intersectionWithCap
			intersectionWithCap2
			intersectionPoint
		*/
		return intersectionPoint;
	}
}//Cylinder::intersectionPoint

Vector3 Cylinder::normalAt(Vector3 *intersectionPoint)
{
	/*return normalizeVector( new Vector3(
		(intersectionPoint->x - this->p1.x),
		(0),
		(intersectionPoint->z - this->p1.z)));
		*/

	Vector3 normal;
	normal.x = (intersectionPoint->x - this->p1.x);
	normal.y = (0);
	normal.z = (intersectionPoint->z - this->p1.z);

	normalizeVector(&normal);

	return normal;

}//Cylinder::normalAt

ColorRGB FlatCircle::colorAt(Vector3 * intersectionPoint)
{
	//texture mapping not implemented
	ColorRGB colorAt;

	colorAt.red = 0;
	colorAt.blue= 0;
	colorAt.green=0;

	return colorAt;
}

Vector3 FlatCircle::normalAt(Vector3 * intersectionPoint)
{
	return normal;
}

Vector3 * FlatCircle::intersectionPoint(Ray * ray, bool furthest)
{
	Vector3 * intersection;
	
	intersection = intersectionWithCircle(ray,location.x,location.y,location.z,radius);		

	return intersection;
}

ColorRGB Polygon3::colorAt(Vector3 * intersectionPoint)
{
	//texture mapping not implemented
	ColorRGB colorAt;

	colorAt.red = 0;
	colorAt.blue= 0;
	colorAt.green=0;

	return colorAt;
}

Vector3 * Polygon3::intersectionPoint(Ray * ray, bool furthest)
{
	/*
		1. need equation for plane containing this polygon

		Ax + By + Cz + d = 0
		(A,B,C) is the normal

		2. determine whether ray actually hits plane & calculate t

		   A*(loc->x + dir->x*t) + B*(loc->y + dir->y*t) + C*(loc->z + dir->z*t) +  d = 0
		   A*(loc->x) + A*(dir->x*t) + B*(loc->y) + B*(dir->y*t) + C*(loc->z) + C*(dir->z*t) +  d = 0
		   A*(loc->x) + B*(loc->y) + C*(loc->z) + d = -(A*(dir->x*t) + B*(dir->y*t) + C*(dir->z*t))
		   A*(loc->x) + B*(loc->y) + C*(loc->z) + d = -t(A*(dir->x) + B*(dir->y) + C*(dir->z))
		   (A*(loc->x) + B*(loc->y) + C*(loc->z) + d)/(A*(dir->x) + B*(dir->y) + C*(dir->z)) = -t

		3.  determine whether point lies within bounds

			check that the dot product is the same for all points
			     (p(i+1) - pi * (int - pi) is the same for all segments

		4. return point
	*/

	//1.
	if(this->numberOfVerticies<3)
		return NULL;

	//do a quick check
	if(
	  ((ray->location->x>maxx)&&(ray->direction->x > 0))||
	  ((ray->location->x<minx)&&(ray->direction->x < 0))||
	  ((ray->location->y>maxy)&&(ray->direction->y > 0))||
	  ((ray->location->y<miny)&&(ray->direction->y < 0))||
	  ((ray->location->z>maxz)&&(ray->direction->z > 0))||
	  ((ray->location->z<minz)&&(ray->direction->z < 0))
	  )
		  return NULL;
		  

	//Vector3 * normal = normalToPlane( pointList[0],pointList[1],pointList[2]);

	float A = normal.x;
	float B = normal.y;
	float C = normal.z;
	//float d = -(A*pointList[0]->x + B*pointList[0]->y + C*pointList[0]->z);
	

	//2.
	Vector3 * dir = ray->direction;
	Vector3 * loc = ray->location;
	
	float det =(A*(dir->x) + B*(dir->y) + C*(dir->z));//determinant

	if(det == 0)
		return NULL;

	float t = -(A*(loc->x) + B*(loc->y) + C*(loc->z) + d)/det;

	if(t<0)
		return NULL;

	Vector3 retVal( loc->x + dir->x * t, loc->y + dir->y * t,loc->z + dir->z * t);

	/*
		Check each triangle, if it is in contained in any, then return the point
	//

	for(int i = 1; i< numberOfVerticies-1; i++)
	{
		//triangle will be 0, i, i+1
		int i2 = i+ 1;

		Vector3 * p1 = pointList[0];
		Vector3 * p2 = pointList[i];
		Vector3 * p3 = pointList[i2];

		if(inTrangle(&retVal,p1,p2,p3))
			return retVal.copy();
	}

	*/
	//quickly check ret value
	if(
		((retVal.x < minx)||(retVal.x> maxx))||
		((retVal.y < miny)||(retVal.y> maxy))||
		((retVal.z < minz)||(retVal.z> maxz))
	  )
		  return NULL;

	if(inPolygon(pointList,this->numberOfVerticies,&retVal))
		return retVal.copy();

	return NULL;


}//Polygon3::intersectionPoint

Vector3 Polygon3::normalAt(Vector3 *intersectionPoint)
{
	//the intersection point here is irrelevant

	/*Vector3 normal = *normalToPlane(pointList[0],pointList[1],pointList[2]);

	normalizeVector(&normal);
*/
	return normal;
}//normalAt



void Polygon3::addPoint(Vector3 * newPoint)
{
	//Adds a point to the polygon, used to set up the scene
	float x = newPoint->x;
	float y = newPoint->y;
	float z = newPoint->z;
	pointList[numberOfVerticies] = newPoint;

	this->numberOfVerticies++;

	if(numberOfVerticies==1)
	{
		minx = x;
		maxx = x;
		miny = y;
		maxy = y;
		minz = z;
		maxz = z;
	}
	else
	{
		if(x>maxx)
			maxx = x;
		if(x<minx)
			minx = x;
		if(y>maxy)
			maxy = y;
		if(y<miny)
			miny = y;
		if(z>maxz)
			maxz = z;
		if(z<minz)
			minz = z;
	}

	if(numberOfVerticies>=3)
	{	
		//precompute the normal
		normal = *normalToPlane(pointList[0],pointList[1],pointList[2]);
		normalizeVector(&normal);
		d = -((normal.x)*pointList[0]->x + (normal.y)*pointList[0]->y + (normal.z)*pointList[0]->z);
	}

}

void Polygon3::addPoint(float x, float y, float z)
{
	//Adds a point to the polygon, used to set up the scene
	Vector3 *newPoint = new Vector3(x,y,z);
	pointList[numberOfVerticies] = newPoint;

	this->numberOfVerticies++;

	if(numberOfVerticies==1)
	{
		minx = x;
		maxx = x;
		miny = y;
		maxy = y;
		minz = z;
		maxz = z;
	}
	else
	{
		if(x>maxx)
			maxx = x;
		if(x<minx)
			minx = x;
		if(y>maxy)
			maxy = y;
		if(y<miny)
			miny = y;
		if(z>maxz)
			maxz = z;
		if(z<minz)
			minz = z;
	}

	if(numberOfVerticies>=3)
	{	
		//precompute the normal
		normal = *normalToPlane(pointList[0],pointList[1],pointList[2]);
		normalizeVector(&normal);
		d = -((normal.x)*pointList[0]->x + (normal.y)*pointList[0]->y + (normal.z)*pointList[0]->z);
	}
}//Polygon3::addPoint

Vector3 * intersectionWithCircle(Ray * ray, float x, float y, float z, float r)
{
	//Used for flat circles, not spheres

	float A = 0;
	float B = 1;
	float C = 0;
	float d = -(A*x + B*y + C*z);

	//2.
	Vector3 * dir = ray->direction;
	Vector3 * loc = ray->location;
	
	float det =(A*(dir->x) + B*(dir->y) + C*(dir->z));//determinant

	if(det == 0)
		return NULL;

	float t = -(A*(loc->x) + B*(loc->y) + C*(loc->z) + d)/det;

	if(t<0)
		return NULL;

	Vector3 retVal( loc->x + dir->x * t, loc->y + dir->y * t,loc->z + dir->z * t);

	//if distance from center to ret val < r, then return retVal

	if(r*r> (retVal.x-x)*(retVal.x-x) + (retVal.y-y)*(retVal.y-y) + (retVal.z-z)*(retVal.z-z))
		return retVal.copy();
	return NULL;
}

