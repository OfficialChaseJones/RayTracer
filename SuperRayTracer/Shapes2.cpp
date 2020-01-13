/*


	Shapes2.cpp  (Shapes.cpp got too big)


	contains the 'bool intersectPyramid(Ray *,Ray *,Ray *,Ray *, float distance);' function
	for each shape intended to be used in 'PerformanceRayTrace'

	
	the function 'intersectPyramid' does not have to be exact, but when uncertain, MUST return true


*/

#include "Shapes.h"


float distanceToIntersection(SceneObject * object, Ray * ray)
{
	Vector3* intersection = object->intersectionPoint(ray,false);
	if(intersection==NULL)
		return -1;
	else
	{
		float distanceToIntersection = distance(intersection, ray->location);
		delete intersection;
		return distanceToIntersection;
	}

}

bool Sphere::intersectPyramid(Ray * rayUpperLeft,Ray * rayUpperRight,Ray * rayBottomLeft,Ray *rayBottomRight, float heightOfPyramid, bool doInitialCheck)
{
	//1. if any of the rays intersect the sphere 
	//			if doIntitialCheck,return FALSE(because it means that it is behind the object that bounds the pyramid)
	//			else if heightOfPyramid > distance to intersection, return true
	//2. create bounding cube, do intersection with polygons

	float distanceToIntersectionUpperLeft = distanceToIntersection(this,rayUpperLeft);
	if(distanceToIntersectionUpperLeft>=0)
	{
		if(doInitialCheck)
			return false;
		else if (heightOfPyramid > distanceToIntersectionUpperLeft)
			return true;
	}
	float distanceToIntersectionUpperRight = distanceToIntersection(this,rayUpperRight);
	if(distanceToIntersectionUpperRight>=0)
	{
		if(doInitialCheck)
			return false;
		else if (heightOfPyramid > distanceToIntersectionUpperRight)
			return true;
	}
	float distanceToIntersectionBottomLeft = distanceToIntersection(this,rayBottomLeft);
	if(distanceToIntersectionBottomLeft>=0)
	{
		if(doInitialCheck)
			return false;
		else if (heightOfPyramid > distanceToIntersectionBottomLeft)
			return true;
	}
	float distanceToIntersectionBottomRight = distanceToIntersection(this,rayBottomRight);
	if(distanceToIntersectionBottomRight>=0)
	{
		if(doInitialCheck)
			return false;
		else if (heightOfPyramid > distanceToIntersectionBottomRight)
			return true;
	}

	Polygon3 * side1 = new Polygon3();
	Polygon3 * side2 = new Polygon3();
	Polygon3 * side3 = new Polygon3();
	Polygon3 * side4 = new Polygon3();
	Polygon3 * side5 = new Polygon3();
	Polygon3 * side6 = new Polygon3();

	Vector3 p1(location.x - radius,location.y - radius,location.z - radius);
	Vector3 p2(location.x - radius,location.y - radius,location.z + radius);
	Vector3 p3(location.x - radius,location.y + radius,location.z - radius);
	Vector3 p4(location.x - radius,location.y + radius,location.z + radius);
	Vector3 p5(location.x + radius,location.y - radius,location.z - radius);
	Vector3 p6(location.x + radius,location.y - radius,location.z + radius);
	Vector3 p7(location.x + radius,location.y + radius,location.z - radius);
	Vector3 p8(location.x + radius,location.y + radius,location.z + radius);

	//left
	side1->addPoint(&p1);	
	side1->addPoint(&p2);
	side1->addPoint(&p4);
	side1->addPoint(&p3);
	//right
	side2->addPoint(&p5);	
	side2->addPoint(&p6);
	side2->addPoint(&p8);
	side2->addPoint(&p7);
	//top
	side3->addPoint(&p3);
	side3->addPoint(&p4);
	side3->addPoint(&p8);
	side3->addPoint(&p7);
	//bottom
	side4->addPoint(&p1);
	side4->addPoint(&p2);
	side4->addPoint(&p6);
	side4->addPoint(&p5);
	//front
	side5->addPoint(&p1);
	side5->addPoint(&p3);
	side5->addPoint(&p7);
	side5->addPoint(&p5);
	//back
	side6->addPoint(&p2);
	side6->addPoint(&p4);
	side6->addPoint(&p8);
	side6->addPoint(&p6);

	bool intersects = false;

	if((side1->intersectPyramid(rayUpperLeft,rayUpperRight, rayBottomLeft,rayBottomRight, heightOfPyramid,false))||
	  (side2->intersectPyramid(rayUpperLeft,rayUpperRight, rayBottomLeft,rayBottomRight, heightOfPyramid,false))||
	  (side3->intersectPyramid(rayUpperLeft,rayUpperRight, rayBottomLeft,rayBottomRight, heightOfPyramid,false))||
	  (side4->intersectPyramid(rayUpperLeft,rayUpperRight, rayBottomLeft,rayBottomRight, heightOfPyramid,false))||
	  (side5->intersectPyramid(rayUpperLeft,rayUpperRight, rayBottomLeft,rayBottomRight, heightOfPyramid,false))||
	  (side6->intersectPyramid(rayUpperLeft,rayUpperRight, rayBottomLeft,rayBottomRight, heightOfPyramid,false)))
	{
		intersects = true;	
	}

	delete side1;
	delete side2;
	delete side3;
	delete side4;
	delete side5;
	delete side6;

	return intersects;

}//Sphere::intersectPyramid

bool Cylinder::intersectPyramid(Ray * rayUpperLeft,Ray * rayUpperRight,Ray * rayBottomLeft,Ray *rayBottomRight, float heightOfPyramid, bool doInitialCheck)
{
	//1. if any of the rays intersect the Cylinder 
	//			if doIntitialCheck,return FALSE(because it means that it is behind the object that bounds the pyramid)
	//			else if heightOfPyramid > distance to intersection, return true
	//2. Create bounding polyhedron, do intersection

	//same as sphere, only bounding rectangular polyhedron, not a cube

	float distanceToIntersectionUpperLeft = distanceToIntersection(this,rayUpperLeft);
	if(distanceToIntersectionUpperLeft>=0)
	{
		if(doInitialCheck)
			return false;
		else if (heightOfPyramid > distanceToIntersectionUpperLeft)
			return true;
	}
	float distanceToIntersectionUpperRight = distanceToIntersection(this,rayUpperRight);
	if(distanceToIntersectionUpperRight>=0)
	{
		if(doInitialCheck)
			return false;
		else if (heightOfPyramid > distanceToIntersectionUpperRight)
			return true;
	}
	float distanceToIntersectionBottomLeft = distanceToIntersection(this,rayBottomLeft);
	if(distanceToIntersectionBottomLeft>=0)
	{
		if(doInitialCheck)
			return false;
		else if (heightOfPyramid > distanceToIntersectionBottomLeft)
			return true;
	}
	float distanceToIntersectionBottomRight = distanceToIntersection(this,rayBottomRight);
	if(distanceToIntersectionBottomRight>=0)
	{
		if(doInitialCheck)
			return false;
		else if (heightOfPyramid > distanceToIntersectionBottomRight)
			return true;
	}

	Polygon3 * side1 = new Polygon3();
	Polygon3 * side2 = new Polygon3();
	Polygon3 * side3 = new Polygon3();
	Polygon3 * side4 = new Polygon3();
	Polygon3 * side5 = new Polygon3();
	Polygon3 * side6 = new Polygon3();

	
	Vector3 p1(p1.x - radius,p1.y         ,p1.z - radius);
	Vector3 p2(p1.x - radius,p1.y         ,p1.z + radius);
	Vector3 p3(p1.x - radius,p1.y + height,p1.z - radius);
	Vector3 p4(p1.x - radius,p1.y + height,p1.z + radius);
	Vector3 p5(p1.x + radius,p1.y	      ,p1.z - radius);
	Vector3 p6(p1.x + radius,p1.y		  ,p1.z + radius);
	Vector3 p7(p1.x + radius,p1.y + height,p1.z - radius);
	Vector3 p8(p1.x + radius,p1.y + height,p1.z + radius);

	//left
	side1->addPoint(&p1);	
	side1->addPoint(&p2);
	side1->addPoint(&p4);
	side1->addPoint(&p3);
	//right
	side2->addPoint(&p5);	
	side2->addPoint(&p6);
	side2->addPoint(&p8);
	side2->addPoint(&p7);
	//top
	side3->addPoint(&p3);
	side3->addPoint(&p4);
	side3->addPoint(&p8);
	side3->addPoint(&p7);
	//bottom
	side4->addPoint(&p1);
	side4->addPoint(&p2);
	side4->addPoint(&p6);
	side4->addPoint(&p5);
	//front
	side5->addPoint(&p1);
	side5->addPoint(&p3);
	side5->addPoint(&p7);
	side5->addPoint(&p5);
	//back
	side6->addPoint(&p2);
	side6->addPoint(&p4);
	side6->addPoint(&p8);
	side6->addPoint(&p6);

	bool intersects = false;

	if((side1->intersectPyramid(rayUpperLeft,rayUpperRight, rayBottomLeft,rayBottomRight, heightOfPyramid,false))||
	  (side2->intersectPyramid(rayUpperLeft,rayUpperRight, rayBottomLeft,rayBottomRight, heightOfPyramid,false))||
	  (side3->intersectPyramid(rayUpperLeft,rayUpperRight, rayBottomLeft,rayBottomRight, heightOfPyramid,false))||
	  (side4->intersectPyramid(rayUpperLeft,rayUpperRight, rayBottomLeft,rayBottomRight, heightOfPyramid,false))||
	  (side5->intersectPyramid(rayUpperLeft,rayUpperRight, rayBottomLeft,rayBottomRight, heightOfPyramid,false))||
	  (side6->intersectPyramid(rayUpperLeft,rayUpperRight, rayBottomLeft,rayBottomRight, heightOfPyramid,false)))
	{
		intersects = true;	
	}

	delete side1;
	delete side2;
	delete side3;
	delete side4;
	delete side5;
	delete side6;

	return intersects;

}//Cylinder::intersectPyramid

bool Polygon3::intersectPyramid(Ray * rayUpperLeft,Ray * rayUpperRight,Ray * rayBottomLeft,Ray *rayBottomRight, float heightOfPyramid, bool doInitialCheck)
{
	//1. find intersection of rays with plane that polygon rests on
	//2. if any of the rays intersect the polygon 
	//			if doIntitialCheck,return FALSE(because it means that it is behind the object that bounds the pyramid)
	//			else if heightOfPyramid > distance to intersection, return true
	//3. if any of the points on the polygon lie within the quadralateral, return true
	//4. return false;

	float distanceToIntersectionUpperLeft = distanceToIntersection(this,rayUpperLeft);
	if(distanceToIntersectionUpperLeft>=0)
	{
		if(doInitialCheck)
			return false;
		else if (heightOfPyramid > distanceToIntersectionUpperLeft)
			return true;
	}
	float distanceToIntersectionUpperRight = distanceToIntersection(this,rayUpperRight);
	if(distanceToIntersectionUpperRight>=0)
	{
		if(doInitialCheck)
			return false;
		else if (heightOfPyramid > distanceToIntersectionUpperRight)
			return true;
	}
	float distanceToIntersectionBottomLeft = distanceToIntersection(this,rayBottomLeft);
	if(distanceToIntersectionBottomLeft>=0)
	{
		if(doInitialCheck)
			return false;
		else if (heightOfPyramid > distanceToIntersectionBottomLeft)
			return true;
	}
	float distanceToIntersectionBottomRight = distanceToIntersection(this,rayBottomRight);
	if(distanceToIntersectionBottomRight>=0)
	{
		if(doInitialCheck)
			return false;
		else if (heightOfPyramid > distanceToIntersectionBottomRight)
			return true;
	}


	float A = normal.x;
	float B = normal.y;
	float C = normal.z;

	
	float det1 =(A*(rayUpperLeft->direction->x) + B*(rayUpperLeft->direction->y) + C*(rayUpperLeft->direction->z));//determinant
	float det2 =(A*(rayUpperRight->direction->x) + B*(rayUpperRight->direction->y) + C*(rayUpperRight->direction->z));//determinant
	float det3 =(A*(rayBottomLeft->direction->x) + B*(rayBottomLeft->direction->y) + C*(rayBottomLeft->direction->z));//determinant
	float det4 =(A*(rayBottomRight->direction->x) + B*(rayBottomRight->direction->y) + C*(rayBottomRight->direction->z));//determinant

	//NOTE: This may be incorrect
	if((det1 == 0)||(det2 == 0)||(det3 == 0)||(det4 == 0))
		return false;

	float t1 = -(A*(rayUpperLeft->location->x)   + B*(rayUpperLeft->location->y)   + C*(rayUpperLeft->location->z)   + d)/det1;
	float t2 = -(A*(rayUpperRight->location->x)  + B*(rayUpperRight->location->y)  + C*(rayUpperRight->location->z)  + d)/det2;
	float t3 = -(A*(rayBottomLeft->location->x)  + B*(rayBottomLeft->location->y)  + C*(rayBottomLeft->location->z)  + d)/det3;
	float t4 = -(A*(rayBottomRight->location->x) + B*(rayBottomRight->location->y) + C*(rayBottomRight->location->z) + d)/det4;

	if((t1<0)||(t2<0)||(t3<0)||(t4<0))
		return false;


	Vector3 p1( rayUpperLeft->location->x   + rayUpperLeft->direction->x * t1  , rayUpperLeft->location->y   + rayUpperLeft->direction->y * t1  ,rayUpperLeft->location->z   + rayUpperLeft->direction->z * t1);
	Vector3 p2( rayUpperRight->location->x  + rayUpperRight->direction->x * t2 , rayUpperRight->location->y  + rayUpperRight->direction->y * t2 ,rayUpperRight->location->z  + rayUpperRight->direction->z * t2);
	Vector3 p3( rayBottomLeft->location->x  + rayBottomLeft->direction->x * t3 , rayBottomLeft->location->y  + rayBottomLeft->direction->y * t3 ,rayBottomLeft->location->z  + rayBottomLeft->direction->z * t3);
	Vector3 p4( rayBottomRight->location->x + rayBottomRight->direction->x * t4, rayBottomRight->location->y + rayBottomRight->direction->y * t4,rayBottomRight->location->z + rayBottomRight->direction->z * t4);

	if(distance(&p1,rayUpperLeft->location) > heightOfPyramid)
		return false;

	Vector3 ** quadralateral = new Vector3 *[4];
	
	quadralateral[0] = &p1;
	quadralateral[1] = &p2;
	quadralateral[2] = &p4;
	quadralateral[3] = &p3;


	//use inPolygon
	for(int i =0;i<this->numberOfVerticies;i++)
	{
		if(inPolygon(quadralateral,4,pointList[i]))
		{
			delete quadralateral;
			return true;
		}
	}


	delete quadralateral;

	return false;
}//Polygon3::intersectPyramid

bool FlatCircle::intersectPyramid(Ray * rayUpperLeft,Ray * rayUpperRight,Ray * rayBottomLeft,Ray *rayBottomRight, float heightOfPyramid, bool doInitialCheck)
{
	//1. find intersection of rays with plane that circle rests on
	//2. if any of the rays intersect the circle 
	//			if doIntitialCheck,return FALSE(because it means that it is behind the object that bounds the pyramid)
	//			else if heightOfPyramid > distance to intersection, return true
	//3. Create bounding polygon for circle, if any of the bounding points lies within the quadralateral, return true
	//3. return false;

	float distanceToIntersectionUpperLeft = distanceToIntersection(this,rayUpperLeft);
	if(distanceToIntersectionUpperLeft>=0)
	{
		if(doInitialCheck)
			return false;
		else if (heightOfPyramid > distanceToIntersectionUpperLeft)
			return true;
	}
	float distanceToIntersectionUpperRight = distanceToIntersection(this,rayUpperRight);
	if(distanceToIntersectionUpperRight>=0)
	{
		if(doInitialCheck)
			return false;
		else if (heightOfPyramid > distanceToIntersectionUpperRight)
			return true;
	}
	float distanceToIntersectionBottomLeft = distanceToIntersection(this,rayBottomLeft);
	if(distanceToIntersectionBottomLeft>=0)
	{
		if(doInitialCheck)
			return false;
		else if (heightOfPyramid > distanceToIntersectionBottomLeft)
			return true;
	}
	float distanceToIntersectionBottomRight = distanceToIntersection(this,rayBottomRight);
	if(distanceToIntersectionBottomRight>=0)
	{
		if(doInitialCheck)
			return false;
		else if (heightOfPyramid > distanceToIntersectionBottomRight)
			return true;
	}

	/*Create bounding polygon
	
		The circle is flat and parallel to x-z plane

	*/
	
	Polygon3 *boundingPolygon = new Polygon3();

	Vector3 p1(location.x-radius, location.y, location.z-radius);
	Vector3 p2(location.x-radius, location.y, location.z+radius);
	Vector3 p3(location.x+radius, location.y, location.z+radius);
	Vector3 p4(location.x+radius, location.y, location.z-radius);

	boundingPolygon->addPoint(&p1);
	boundingPolygon->addPoint(&p2);
	boundingPolygon->addPoint(&p3);
	boundingPolygon->addPoint(&p4);

	bool intersectsBoundingPolygon = boundingPolygon->intersectPyramid(rayUpperLeft,rayUpperRight, rayBottomLeft,rayBottomRight, heightOfPyramid,false);

	delete boundingPolygon;

	return intersectsBoundingPolygon;
}//FlatCircle::intersectPyramid


Vector3 *Sphere::intersectionPointAssumeIntersection(Ray * ray)
{
	return this->intersectionPoint(ray,false);
}
Vector3 *Cylinder::intersectionPointAssumeIntersection(Ray * ray)
{
	return this->intersectionPoint(ray,false);
}
Vector3 *FlatCircle::intersectionPointAssumeIntersection(Ray * ray)
{
	float A = 0;
	float B = 1;
	float C = 0;
	float d = -(A*location.x + B*location.y + C*location.z);

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

	return retVal.copy();
}
Vector3 *Polygon3::intersectionPointAssumeIntersection(Ray * ray)
{
	//This assumes that the
	float A = normal.x;
	float B = normal.y;
	float C = normal.z;
	
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

	return retVal.copy();
}





