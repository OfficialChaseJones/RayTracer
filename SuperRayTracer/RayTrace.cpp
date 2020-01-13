/*

	RayTrace.cpp




*/
#include "RayTrace.h"
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>

void Scene::init()
{
	//init storage vectors
	cylinders = new CylinderList();
	bottles = new BottleList();
	lightSources = new LightSourceList();
	polygons = new Polygon3List();
	spheres = new SphereList();
	polyhedrons = new PolyhedronList();
	circles = new FlatCircleList();

	bogusTerm.blue = 0.2;
	bogusTerm.red = 0.2;
	bogusTerm.green = 0.9;
}

void Scene::addCylinder(float x, float y, float z, float r, float h)
{
	Cylinder * cylinder = new Cylinder();
	cylinder->height = h;
	cylinder->p1.x = x;
	cylinder->p1.y = y;
	cylinder->p1.z = z;
	cylinder->radius = r;

	cylinder->setLighting(currentObjectColor.red,currentObjectColor.green,currentObjectColor.blue,currentRelfectiveCoefficient,currentTransmissionCoefficient, currentMaterialConstant,	currentSpecular,currentDiffuse, NULL);

	this->cylinders->push_back(cylinder);

	addFlatCircle(x,y,z,r,false);
	addFlatCircle(x,y+h,z,r,true);
}

void Scene::addFlatCircle(float x, float y, float z, float r, bool up)
{
	FlatCircle * circle = new FlatCircle();

	circle->location.x = x;
	circle->location.y = y;
	circle->location.z = z;
	circle->radius = r;

	circle->normal.x = 0;
	circle->normal.z = 0;

	if(up)
		circle->normal.y = 1;
	else
		circle->normal.y = -1;
	
	circle->setLighting(currentObjectColor.red,currentObjectColor.green,currentObjectColor.blue,currentRelfectiveCoefficient,currentTransmissionCoefficient,currentMaterialConstant,	currentSpecular,currentDiffuse, NULL);

	this->circles->push_back(circle);
}

void Scene::addSphere(float x, float y, float z, float r, char *textureFile)
{
	Sphere *sphere = new Sphere();
	sphere->radius = r;
	sphere->location.x = x;
	sphere->location.y = y;
	sphere->location.z = z;
	
	sphere->setLighting(currentObjectColor.red,currentObjectColor.green,currentObjectColor.blue,currentRelfectiveCoefficient,currentTransmissionCoefficient,currentMaterialConstant,	currentSpecular,currentDiffuse, textureFile);

	this->spheres->push_back(sphere);
}

void Scene::addPolygon(Polygon3 * newPolygon)
{
	newPolygon->setLighting(currentObjectColor.red,currentObjectColor.green,currentObjectColor.blue,currentRelfectiveCoefficient,currentTransmissionCoefficient,currentMaterialConstant,	currentSpecular,currentDiffuse, NULL);
	this->polygons->push_back(newPolygon);
}

void Scene::addPolyhedron(Polyhedron * newPolyhedron)
{
	this->polyhedrons->push_back(newPolyhedron);

	//convert to polygons, then add these to the scene
	Polygon3List::iterator polygonIterator;

	for (polygonIterator = newPolyhedron->faces->begin(); polygonIterator != newPolyhedron->faces->end(); polygonIterator++)
	{
		Polygon3 * polygon = *((Polygon3  **)polygonIterator);

		this->polygons->push_back(polygon);
	}

}//addPolyhedron

void Scene::addBottle(Bottle * newBottle)
{
	this->bottles->push_back(newBottle);

	addFlatCircle(newBottle->location.x,newBottle->location.y,newBottle->location.z,newBottle->radius,false);
	addFlatCircle(newBottle->location.x,newBottle->location.y + newBottle->height,newBottle->location.z,newBottle->topRadius,true);

	//We need to iterate through the polygons and set the current lighting properties.
	Polygon3List::iterator polygonIterator;
	for (polygonIterator = newBottle->polygonList->begin(); polygonIterator != newBottle->polygonList->end(); polygonIterator++)
	{
		Polygon3 * polygon = *((Polygon3  **)polygonIterator);
		polygon->setLighting(currentObjectColor.red,currentObjectColor.green,currentObjectColor.blue,currentRelfectiveCoefficient,currentTransmissionCoefficient,currentMaterialConstant,currentSpecular,currentDiffuse, NULL);
	}

}

void Scene::addLightSource(float x, float y, float z, float r, float g, float b)
{
	LightSource * lightSource = new LightSource();

	lightSource->location.x = x;
	lightSource->location.y = y;
	lightSource->location.z = z;
	lightSource->color.red  = r;
	lightSource->color.green= g;
	lightSource->color.blue = b;

	this->lightSources->push_back(lightSource);
}



/*
	Rendering related functions below




  */

void renderScene(struct Scene *scene,struct ScreenData* screenData)
{
	//interface for ray tracer
	//use default depth
	renderScene(scene,screenData, scene->rayTraceDepth);
}

void renderScene(struct Scene *scene,struct ScreenData* screenData, int depth)
{
	//public interface for ray tracer
	Vector3 *leftTopCorner     =getUpperLeft(scene);
	Vector3 *bottomRightCorner =getBottomRight(scene);
	Vector3 * bottomLeftCorner =getBottomLeft(scene);

/*display data
	printf("left top :");
	leftTopCorner->print();
	printf("bottom right :");
	bottomRightCorner->print();
	*/

	printf("Scene: %d x %d\n", screenData->width, screenData->height);

	for(int x=0;x< scene->numPixelsHorizontal;x++)
	{
		if(x%(screenData->width/100)==0)
			printf("%%%f complete\n",100*((float)x)/(screenData->width));

		for(int y=0;y< scene->numPixelsVertical;y++)
		{
			//Memory monitoring to find leaks
			//if((x%50==0)&&(y%50==0))
			//	printf("Vector3s: %d, Rays: %d,  Colors: %d\n", instanceOfVector3,instanceOfRay,instanceOfColor);

			#ifndef ANTI_ALIASING

				float percentToRight = ((float)x)/scene->numPixelsHorizontal;
				float percentDown    =  ((float)y)/scene->numPixelsVertical;

				Ray * rayFromViewPointToXY = calculateRayFromEyePointToPointOnScreen(scene,leftTopCorner,bottomRightCorner,bottomLeftCorner,percentToRight,percentDown);
				ColorRGB color = traceRay(scene,rayFromViewPointToXY,depth, NULL);
				screenData->setPixel(x,y, color);

				delete rayFromViewPointToXY->direction;
				delete rayFromViewPointToXY->location;
				delete rayFromViewPointToXY;

			#endif

			#ifdef ANTI_ALIASING


				ColorRGB finalColor = antiAliasTrace(scene->antialiasDepth,  x,  y, scene, leftTopCorner, bottomRightCorner, bottomLeftCorner, depth);
				screenData->setPixel(x,y, finalColor);


			#endif

		}//for

		//update the window to show partial image
		if(x%2==0)
			glutPostRedisplay();
	}//for

	glutPostRedisplay();//make sure image is displayed

}//render scene

ColorRGB antiAliasTrace(int raysAcross, int x, int y, struct Scene *scene, Vector3 *leftTopCorner, Vector3 *bottomRightCorner,Vector3 * bottomLeftCorner, int depth)
{

	ColorRGB finalColor;// = new ColorRGB();
	finalColor.blue = 0;
	finalColor.red  = 0;
	finalColor.green= 0;

	for(int i =0;i<raysAcross;i++)
	{
		for(int j=0;j<raysAcross;j++)
		{
			float percentToRight =  (((float)(x*raysAcross+i)))/(scene->numPixelsHorizontal*raysAcross);
			float percentDown    =  (((float)(y*raysAcross+j)))/(scene->numPixelsVertical*raysAcross);

			Ray * rayFromViewPointToXY = calculateRayFromEyePointToPointOnScreen(scene,leftTopCorner,bottomRightCorner,bottomLeftCorner,percentToRight,percentDown);
			
			ColorRGB color    = traceRay(scene,rayFromViewPointToXY,depth, NULL);

			finalColor.blue += (color.blue) /(raysAcross*raysAcross);
			finalColor.red  += (color.red)  /(raysAcross*raysAcross);
			finalColor.green+= (color.green)/(raysAcross*raysAcross);

			delete rayFromViewPointToXY->direction;
			delete rayFromViewPointToXY->location;
			delete rayFromViewPointToXY;
			
		}//for
	}//for
	
	return finalColor;
}

Vector3 * getUpperLeft(struct Scene *scene)
{
	Vector3 *leftTopCorner = new Vector3();
	leftTopCorner->x = scene->cameraLookat.x - scene->cameraLocation.x;
	leftTopCorner->y = scene->cameraLookat.y - scene->cameraLocation.y;
	leftTopCorner->z = scene->cameraLookat.z - scene->cameraLocation.z;
	
	//rotate left and right
	leftTopCorner->rotatePointAboutAxis(scene->cameraSpanLeftRight/2,0,1,0);
	//rotate up and down 
	//axis is perpendicular to (0,1,0) and LeftTopCorner
	Vector3 *p1 = new Vector3(0,0,0);
	Vector3 *p2 = new Vector3(0,1,0);
	Vector3 *axisOfRotationUpDown = normalToPlane(p1,p2,leftTopCorner);

	leftTopCorner->rotatePointAboutAxis(scene->cameraSpanUpDown/2,        axisOfRotationUpDown->x,axisOfRotationUpDown->y,axisOfRotationUpDown->z);

	return leftTopCorner;
}

Vector3 * getBottomRight(struct Scene *scene)
{
	Vector3 * bottomRightCorner = new Vector3();
	bottomRightCorner->x = scene->cameraLookat.x - scene->cameraLocation.x;
	bottomRightCorner->y = scene->cameraLookat.y - scene->cameraLocation.y;
	bottomRightCorner->z = scene->cameraLookat.z - scene->cameraLocation.z;
	
	//rotate left and right
	bottomRightCorner->rotatePointAboutAxis(- (scene->cameraSpanLeftRight/2),0,1,0);

	//rotate up and down 
	//axis is perpendicular to (0,1,0) and LeftTopCorner
	Vector3 *p1 = new Vector3(0,0,0);
	Vector3 *p2 = new Vector3(0,1,0);
	Vector3 *axisOfRotationUpDown2 = normalToPlane(p1,p2,bottomRightCorner);

	bottomRightCorner->rotatePointAboutAxis(-(scene->cameraSpanUpDown/2), axisOfRotationUpDown2->x,axisOfRotationUpDown2->y,axisOfRotationUpDown2->z);

	return bottomRightCorner;
}

Vector3 * getBottomLeft(struct Scene *scene)
{
	Vector3 * bottomRightCorner = new Vector3();
	bottomRightCorner->x = scene->cameraLookat.x - scene->cameraLocation.x;
	bottomRightCorner->y = scene->cameraLookat.y - scene->cameraLocation.y;
	bottomRightCorner->z = scene->cameraLookat.z - scene->cameraLocation.z;
	
	//rotate left and right
	bottomRightCorner->rotatePointAboutAxis((scene->cameraSpanLeftRight/2),0,1,0);

	//axis is perpendicular to (0,1,0) and LeftTopCorner
	Vector3 *p1 = new Vector3(0,0,0);
	Vector3 *p2 = new Vector3(0,1,0);
	Vector3 *axisOfRotationUpDown2 = normalToPlane(p1,p2,bottomRightCorner);

	bottomRightCorner->rotatePointAboutAxis(-(scene->cameraSpanUpDown/2), axisOfRotationUpDown2->x,axisOfRotationUpDown2->y,axisOfRotationUpDown2->z);

	return bottomRightCorner;
}

Ray * calculateRayFromEyePointToPointOnScreen(struct Scene *scene, Vector3 *leftTopCorner, Vector3 *bottomRightCorner,Vector3 * bottomLeftCorner,float percentToRight,float percentDown)
{
	Ray * ray = new Ray();//(Ray *) malloc( sizeof(Ray));

	//algorithm:
	//1. find 4 points of screen in world
	//2. use x,y relative to screen width and height to interpolate where the pixel is in world coords
	//3. calc direction to that point
	//      ray begins at that point
	//      direction is the direction just calculated, return

	//From handout 'assume simple camera with window parallel to the x-y axis, and the center of projection the y axis'

	//interpolate exact point based on x,y and above coords.
	//int numPixelsHorizontal;
	//int numPixelsVertical;

	//create frustum

	//find requested ray in relation to leftTopCorner and bottomRightCorner 

	Vector3 *rayDirection = new Vector3();

	rayDirection->x =leftTopCorner->x+ (bottomRightCorner->x-leftTopCorner->x)*percentToRight;
	rayDirection->y =leftTopCorner->y+ (bottomRightCorner->y-leftTopCorner->y)*percentDown;
	//Now contruct formula for a plane, and use above x,y to find z
	Vector3 * normal = normalToPlane(leftTopCorner, bottomRightCorner, bottomLeftCorner);
	//nx + ny + nz + constant = 0
	//constant = -(nx + ny + nz)
	float constant = -(bottomRightCorner->x*normal->x +bottomRightCorner->y*normal->y +bottomRightCorner->z*normal->z);
	//nz = -(nx + ny + constant)
	//OLD: rayDirection->z =leftTopCorner->z+ (bottomRightCorner->z-leftTopCorner->z)*percentDown;
	if(normal->z == 0)
	{
		throw "some error";
	}
	else
	{
		float z = -(rayDirection->x*normal->x +rayDirection->y*normal->y+ constant)/normal->z;
		rayDirection->z = z;
	}

	ray->direction = rayDirection;
	ray->location  = scene->cameraLocation.copy();

	delete normal;

	return ray;
}


ColorRGB traceRay(struct Scene *scene,struct Ray *ray, int depth, SceneObject * objectToExclude)
{
	/*
		determine closest intersection of ray with an object

		if(object hit)
		{
			compute normal at intersection
			return RT_shade(closest object hit, ray, intersection , normal, depth);
		}
		else
			return background color

		objectToExclude is included so that objects don't try to reflect themselves

    */

	if(ray==NULL)
		throw "Ray is null";

	Vector3 intersectionPoint;

	SceneObject * object = closestIntersection(scene,ray, objectToExclude, false, true,&intersectionPoint);//&intersectionPoint);

	if(object !=NULL)
	{
		Vector3 normalAtIntersection= object->normalAt(&intersectionPoint);
	
		ColorRGB retVal = shade(scene, object,ray,&intersectionPoint,&normalAtIntersection, depth);
	
		return retVal;
	}
	else
	{
		//return background color
		return scene->ambientTerm;
	}
	
}//traceRay

ColorRGB shade(struct Scene *scene,SceneObject * object, struct Ray *ray, Vector3 * intersectionPoint, Vector3 * normal, int depth)
{

	/*
	   Color  colorOfRay
	   Ray reflected, refracted, shadow;
	   Color reflectedColor, refractedColor;


		colorOfRay = ambient term;
		for(each light)
		{
			shadow = ray to light from point;
			if(dot product of normal and direction to light is positive)
			{
				compute how much light is blocked by opaque and transparent surfaces
				and use to scale diffuse and specular terms before adding them to color
			}
		}

		NOTE: in the notes, depth is counted forward and I'm counting backward
		if(depth >0)
		{
			if(object is reflective)
			{

				reflectiveRay = ray in reflection direction from point;
				reflectiveColor = trace(reflectiveRay, depth-1);
				scale reflectiveColor by specular coefficient and add to color
			}

			if(object is transparent)
			{
				refractedRay = ray in refraction direction from point;
				if(total internal reflection does not occur)
				{
					refractedColor = trace(refractedRay, depth-1);
					scale refractedColor by transmission coefficent and add to color;
				}

			}
		}
		

		return colorOfRay;
  */ 
	ColorRGB colorOfRay;
	Ray * reflectedRay  = NULL;
	Ray * refractedRay  = NULL;//the ray entering the object
	Ray * refractedRay2  = NULL;//the ray exiting the object
	Ray shadowRay;

	colorOfRay = scene->ambientTerm;

	#ifdef RAY_CAST_INSTEAD

	if(1==1)
	{
		//For testing, just returning the object's color
		colorOfRay.blue =  object->objectColor.blue;
		colorOfRay.red =   object->objectColor.red;
		colorOfRay.green = object->objectColor.green;

		return colorOfRay;
	}

	#endif


	if(scene->lightSources!=NULL)
	{
		LightSourceList::iterator lightIterator;
		LightSourceList * lightSources = scene->lightSources;

		for (lightIterator = lightSources->begin(); lightIterator != lightSources->end(); lightIterator++)
		{
			LightSource * light = *((LightSource  **)lightIterator);

			//shadow = ray to light from point;
			shadowRay.location = intersectionPoint;
			shadowRay.direction= normalizeVector( new Vector3(light->location.x-intersectionPoint->x,light->location.y-intersectionPoint->y,light->location.z-intersectionPoint->z));
			//if(dot product of normal and direction to light is positive)
			float dotProductOfNormalAndDirectionToLight = normal->x * shadowRay.direction->x + normal->y * shadowRay.direction->y +normal->z * shadowRay.direction->z;
			//bool objectIsIluminated = false;
			float illumination = 1;

			if(dotProductOfNormalAndDirectionToLight >0)
			{
				//compute how much light is blocked by opaque and transparent surfaces
				//and use to scale diffuse and specular terms before adding them to color
				Vector3 intersectionPoint2;
				SceneObject *closestIntersectionInFrontOfLight = closestIntersection(scene,&shadowRay, object, false, false, &intersectionPoint2);

				bool stopSearching = false;
				float distanceToLight, distanceToClosestObject;

				Vector3 *newStartingPoint = NULL;

				distanceToLight         = distance2(intersectionPoint, &(light->location));

				while((closestIntersectionInFrontOfLight!=NULL)&&(!stopSearching)&&(illumination!=0))
				{
					//Vector3 *intersectionPoint2 = closestIntersectionInFrontOfLight->intersectionPoint(&shadowRay, false);

					//if(intersectionPoint2 == NULL)
					//	throw "error";
					distanceToClosestObject = distance2(intersectionPoint, &intersectionPoint2);

					//delete intersectionPoint2;

					if(distanceToLight>distanceToClosestObject)
					{
						illumination = illumination * (closestIntersectionInFrontOfLight->transmissionCoefficient);

						if(newStartingPoint!=NULL)
						{
							delete newStartingPoint;
							newStartingPoint = NULL;
						}

						//continue looking behind the transparent object
						Vector3 *newStartingPoint = closestIntersectionInFrontOfLight->intersectionPoint(&shadowRay,true);

						if(newStartingPoint == NULL)
							newStartingPoint = closestIntersectionInFrontOfLight->intersectionPoint(&shadowRay,false);

						if(newStartingPoint!=NULL)
						{
							shadowRay.location = newStartingPoint;
							closestIntersectionInFrontOfLight = closestIntersection(scene,&shadowRay, closestIntersectionInFrontOfLight, false, false, &intersectionPoint2);
							//if(closestIntersectionInFrontOfLight!=NULL)
							//	printf("found secondary shadow");
						}
						else stopSearching = true;
					}
					else stopSearching = true;

				}//while

				if(newStartingPoint!=NULL)
					delete newStartingPoint;
				//*/
			}
			else illumination = 0;


			//if(objectIsIluminated)
			if(illumination>0)
			{
				/*

					Intensity calculation:
					
					adding the term for this light only

					Ia*ka + sum( Ip*kd*(N dot L) + Ip*ks*(N dot H)^specExp )

					Ia is the ambient light intensity, 
					Ip is the intensity of a particular light source, 
					ka, kd, and ks are the material coefficients, and 
					specExp is the phong exponent. 
					(Note that all of the I's and k's are RGB color triples). 
					More generally, this equation can be thought of as:
					H is halfway vector for Viewpoint and light direction?

					Ia = 0
					sum( Ip*kd*(N dot L) + Ip*ks*(N dot H)^specExp )
					this term for 'light'

					Ip*kd*(N dot L) + Ip*ks*(N dot H)^specExp
				*/

				/*
				        V
					<   H
						N

					average N and V
				*/

				Vector3 v=	subtract(*(ray->location), *intersectionPoint);
				Vector3 h;//   = *normal;

				h = multiply(add(v,*normal),0.5);
				normalizeVector(&h);

				float NdotL = dotProductOfNormalAndDirectionToLight;//dotProduct(normal,shadowRay.direction);
				float NdotH = dotProduct(&h,normal) ;
				float Ipr   = light->color.red;
				float Ipg   = light->color.green;
				float Ipb   = light->color.blue;
				float Or    = object->objectColor.red;
				float Ob    = object->objectColor.blue;
				float Og    = object->objectColor.green;

				float kd    = 0.7;//TODO: assign values to objects instead
				float ks    = 0.4;//TODO: assign values to objects instead
				
				int   specExp= 4;
				float powNdotHspecExp = pow(NdotH,specExp) *ks;
				//float m1 = kd*(NdotL);
				//float m1ppow=m1 + (powNdotHspecExp);*/

				/*
				colorOfRay.red  += m1ppow*Ipr;//Ipr*m1 + Ipr*(powNdotHspecExp);
				colorOfRay.green+= m1ppow*Ipg;//*m1 + Ipg*(powNdotHspecExp);
				colorOfRay.blue += m1ppow*Ipb;//*m1 + Ipb*(powNdotHspecExp);
				*/
				
				/*

				Ir =   (Ipr)(kd*Odr(L*N) + ks*Osr*(N*H))

				*/
				
				float term1 = kd*NdotL;
				float term2 = ks*powNdotHspecExp;
				float sum = term1+term2;

				//If object is textured, need to do another function call
				if(object->textured)
				{
					ColorRGB colorAtPoint = object->colorAt(intersectionPoint);

					Or = colorAtPoint.red;
					Og = colorAtPoint.green;
					Ob = colorAtPoint.blue;

				}

				sum = sum*illumination;

				colorOfRay.red	+= Ipr*(sum*Or);
				colorOfRay.green+= Ipg*(sum*Og);
				colorOfRay.blue	+= Ipb*(sum*Ob);
				//colorOfRay.green+= Ipg*(kd*NdotL*Og + ks*powNdotHspecExp*Og);
				//colorOfRay.blue	+= Ipb*(kd*NdotL*Ob + ks*powNdotHspecExp*Ob);
			}//if illumination>0

			delete shadowRay.direction;
		}//for each light
	}//if lightsources

	
	if(depth > 0 )
	{
		//printf("depth %d\n",depth);
		//REFLECTION
		if(object->relfectiveCoefficient > 0.0)
		{
			reflectedRay = reflectiveRayFromIntersection(ray, normal, intersectionPoint);

			if(reflectedRay==NULL)
				throw "big fat error";

			ColorRGB reflectiveColor = traceRay(scene,reflectedRay,depth-1, object);
			//scale reflectiveColor by specular coefficient and add to color
			colorOfRay.red   += object->relfectiveCoefficient * reflectiveColor.red;
			colorOfRay.blue  += object->relfectiveCoefficient * reflectiveColor.blue;
			colorOfRay.green += object->relfectiveCoefficient * reflectiveColor.green;
			
			delete reflectedRay->direction;
			delete reflectedRay->location;
			delete reflectedRay;

		}//if

		//REFRACTION
		if(object->transmissionCoefficient>0.0)
		{
			bool objectClosed = true;
			ColorRGB refractiveColor;

			//get direction of ray as it enters the object
			refractedRay = refractiveRayFromIntersection(ray, normal, intersectionPoint, 1.0,object->materialConstant);

			if(refractedRay==NULL)
				throw "refraction ray not returned";

			//get 2nd intersection point on the object
			
			//Back up refracted ray to avoid floating point errors in my algorithm
			Vector3 *tempLocation  = refractedRay->location;
			refractedRay->location = new Vector3(refractedRay->location->x-refractedRay->direction->x,
												refractedRay->location->y-refractedRay->direction->y,
												refractedRay->location->z-refractedRay->direction->z);
			
			Vector3 *exitPoint = object->intersectionPoint(refractedRay, true);
			Vector3 normalAtExit;

			if(exitPoint!=NULL)
			{
				delete tempLocation;

				normalAtExit= object->normalAt(exitPoint);
				normalAtExit= multiply(normalAtExit,-1);

				refractedRay2= refractiveRayFromIntersection(refractedRay,&normalAtExit,exitPoint,object->materialConstant,1.0);

				if(refractedRay2==NULL)
					throw "refraction ray not returned";

				refractiveColor = traceRay(scene,refractedRay2,depth-1, object);

			}
			else
			{
				/*In this case, the ray entered the object, but did not intersect
				on the way out.  This may be the case for the cylinder where
				the caps are stored as seperate objects or the bottle which is made up
				of tesselated polygons and 2 caps

				So in this case

				Find the closest intersection point (without backface culling!)
				and use that the refract with instead
				
				*/

				//undo operation done earlier
				delete refractedRay->location;
				refractedRay->location = tempLocation;

				Vector3 intersectionPointOfObjectPart2;
				SceneObject * objectPart2 = closestIntersection(scene,refractedRay, object,false, false, &intersectionPointOfObjectPart2);

				if(objectPart2 != NULL)
				{
					exitPoint = intersectionPointOfObjectPart2.copy();//objectPart2->intersectionPoint(refractedRay,false);
					normalAtExit= objectPart2->normalAt(exitPoint);
					normalAtExit        = multiply(normalAtExit,-1);

					//normalizeVector(&normalAtExit);
					refractedRay2= refractiveRayFromIntersection(refractedRay,&normalAtExit,exitPoint,object->materialConstant,1.0);

					if(refractedRay2==NULL)
						throw "refraction ray not returned";

					refractiveColor = traceRay(scene,refractedRay2,depth-1, objectPart2);

				}
				else
					objectClosed = false;
					//throw "error refracting. object not closed";

			}

			if(objectClosed)
			{
				/*normalAtExit        = multiply(normalAtExit,-1);

				refractedRay2= refractiveRayFromIntersection(refractedRay,&normalAtExit,exitPoint,object->materialConstant,1.0);

				if(refractedRay2==NULL)
					throw "refraction ray not returned";

				refractiveColor = traceRay(scene,refractedRay2,depth-1, object);*/

				//scale refractiveColor by transmission coefficient and add to color
				colorOfRay.red   += object->transmissionCoefficient * refractiveColor.red;
				colorOfRay.blue  += object->transmissionCoefficient * refractiveColor.blue;
				colorOfRay.green += object->transmissionCoefficient * refractiveColor.green;

				delete exitPoint;
				delete refractedRay2->direction;
				delete refractedRay2->location;
				delete refractedRay2;
			}
			else
			{
				//throw "object not closed";
			}
			delete refractedRay->direction;
			delete refractedRay->location;
			delete refractedRay;

		}//if

	}//if(depth > 0 )

	
	if(colorOfRay.red>1)
		colorOfRay.red = 1;
	if(colorOfRay.blue>1)
		colorOfRay.blue = 1;
	if(colorOfRay.green>1)
		colorOfRay.green = 1;

	return colorOfRay;
}//shade


SceneObject * closestIntersection(Scene * scene, Ray * ray, SceneObject * objectToIgnore, bool returnFirst, bool backFaceCull, Vector3 *intersection)
{
	//return first is used when the caller just wants to know if there was an intersection, the closest is not needed

	CylinderList * cylinders = scene->cylinders;
	Polygon3List * polygons  = scene->polygons;// includes polygons from the bottle and the polyhedron
	SphereList   * spheres   = scene->spheres;
	BottleList   * bottles   = scene->bottles;
	FlatCircleList* circles  = scene->circles;

	float distanceToClosestIntersection = 99999999;
	SceneObject * closestObject = NULL;

	//find intersections with cylinders
	if(cylinders!=NULL)
	{
		CylinderList::iterator cylinderIterator;

		for (cylinderIterator = cylinders->begin(); cylinderIterator != cylinders->end(); cylinderIterator++)
		{
			Cylinder * cylinder = *((Cylinder  **)cylinderIterator);

			if(cylinder!= objectToIgnore)
			{
				Vector3* intersectionPoint = cylinder->intersectionPoint(ray,false);
				if(intersectionPoint!=NULL)
				{
					float distanceToIntersection = distance2(intersectionPoint, ray->location);
					if(distanceToIntersection < distanceToClosestIntersection)
					{
						closestObject = cylinder;
						distanceToClosestIntersection = distanceToIntersection;
						if(intersection!=NULL)
						{
							intersection->x = intersectionPoint->x;
							intersection->y = intersectionPoint->y;
							intersection->z = intersectionPoint->z;
						}
						if(returnFirst)
						{
							delete intersectionPoint;
							return closestObject;
						}
					}
					delete intersectionPoint;
				}
			}
		}//for
	}


	//find intersection with flat circles
	if(circles!=NULL)
	{
		FlatCircleList::iterator circlesIterator;

		for (circlesIterator = circles->begin(); circlesIterator != circles->end(); circlesIterator++)
		{
			FlatCircle * circle = *((FlatCircle  **)circlesIterator);

			if(circle!= objectToIgnore)
			{
				Vector3* intersectionPoint = circle->intersectionPoint(ray, false);
				if(intersectionPoint!=NULL)
				{
					float distanceToIntersection = distance2(intersectionPoint, ray->location);
					if(distanceToIntersection < distanceToClosestIntersection)
					{
						closestObject = circle;
						distanceToClosestIntersection = distanceToIntersection;
						if(intersection!=NULL)
						{
							intersection->x = intersectionPoint->x;
							intersection->y = intersectionPoint->y;
							intersection->z = intersectionPoint->z;
						}
						if(returnFirst)
						{
							delete intersectionPoint;
							return closestObject;
						}
					}
					delete intersectionPoint;
				}
			}
		}//for
	}

	//find intersections with bottles
	if(bottles!=NULL)
	{
		BottleList::iterator bottleIterator;

		for (bottleIterator = bottles->begin(); bottleIterator != bottles->end(); bottleIterator++)
		{
			Bottle * bottle = *((Bottle  **)bottleIterator);

			//Now iterate through polygons within
			Polygon3List::iterator polygonIterator;

			if(bottle->rayThroughBoundingVolume(ray))
			{
				//bool intersectionFound = false;

				for (polygonIterator = bottle->polygonList->begin(); (polygonIterator != bottle->polygonList->end()); polygonIterator++)
				{

					Polygon3 * polygon = *((Polygon3  **)polygonIterator);
					
					//backface culling
					bool objectFacing =	((polygon->normal.x*ray->direction->x + polygon->normal.y*ray->direction->y +polygon->normal.z*ray->direction->z)>0);

					if(!backFaceCull)
						objectFacing = false;

					if((polygon!= objectToIgnore)&&(!objectFacing))
					{

						Vector3* intersectionPoint = polygon->intersectionPoint(ray, false);
						if(intersectionPoint!=NULL)
						{
							float distanceToIntersection = distance2(intersectionPoint, ray->location);
							if(distanceToIntersection < distanceToClosestIntersection)
							{
								closestObject = polygon;
								distanceToClosestIntersection = distanceToIntersection;
								if(intersection!=NULL)
								{
									intersection->x = intersectionPoint->x;
									intersection->y = intersectionPoint->y;
									intersection->z = intersectionPoint->z;
								}
								if(returnFirst)
								{
									delete intersectionPoint;
									return closestObject;
								}
							}//if
							delete intersectionPoint;
						}//if
					}//if
				}//for
			}//if
		}//for
	}//if

	//find intersections with polygons
	if(polygons!=NULL)
	{
		Polygon3List::iterator polygonIterator;

		for (polygonIterator = polygons->begin(); polygonIterator != polygons->end(); polygonIterator++)
		{
			Polygon3 * polygon = *((Polygon3  **)polygonIterator);
			
			//backface culling
			bool objectFacing =	((polygon->normal.x*ray->direction->x + polygon->normal.y*ray->direction->y +polygon->normal.z*ray->direction->z)>0);

			if(!backFaceCull)
				objectFacing = true;

			if((polygon!= objectToIgnore)&&(!objectFacing))
			{

				Vector3* intersectionPoint = polygon->intersectionPoint(ray,false);
				if(intersectionPoint!=NULL)
				{
					float distanceToIntersection = distance2(intersectionPoint, ray->location);
					if(distanceToIntersection < distanceToClosestIntersection)
					{
						closestObject = polygon;
						distanceToClosestIntersection = distanceToIntersection;
						if(intersection!=NULL)
						{
							intersection->x = intersectionPoint->x;
							intersection->y = intersectionPoint->y;
							intersection->z = intersectionPoint->z;
						}
						if(returnFirst)
						{
							delete intersectionPoint;
							return closestObject;
						}
					}
				}
				delete intersectionPoint;
			}
		}//for
	}

	//find intersections with spheres
	if(spheres!=NULL)
	{
		SphereList::iterator sphereIterator;

		for (sphereIterator = spheres->begin(); sphereIterator != spheres->end(); sphereIterator++)
		{
			Sphere * sphere = *((Sphere  **)sphereIterator);

			if(sphere!= objectToIgnore)
			{

				Vector3* intersectionPoint = sphere->intersectionPoint(ray,false);
				if(intersectionPoint!=NULL)
				{
					float distanceToIntersection = distance2(intersectionPoint, ray->location);
					if(distanceToIntersection < distanceToClosestIntersection)
					{
						closestObject = sphere;
						distanceToClosestIntersection = distanceToIntersection;
						if(intersection!=NULL)
						{
							intersection->x = intersectionPoint->x;
							intersection->y = intersectionPoint->y;
							intersection->z = intersectionPoint->z;
						}
						if(returnFirst)
						{
							delete intersectionPoint;
							return closestObject;
						}
					}
					delete intersectionPoint;					
				}
			}
		}//for
	}

	return closestObject;

}//closestIntersection


Ray * reflectiveRayFromIntersection(Ray * incomingRay, Vector3 * normal, Vector3 * intersectionPoint)
{
	/*
		Phong model


		This is probably wrong, but will be ok for now
		memory leaks galore
		R = (2*N*L)N - L

		r = 2(i * n)n - i

	*/
	Ray * newRay = new Ray();
	Vector3 i(-incomingRay->direction->x,-incomingRay->direction->y,-incomingRay->direction->z);

	Vector3 r= subtract(multiply(*normal,dotProduct(&i,normal) *2), i);

	newRay->direction = r.copy();//normal->copy();
	newRay->location  = intersectionPoint->copy();
//	newRay->location = intersectionPoint;
//	newRay->direction= add(normal->multiply(2*dotProduct(normal,incomingRay->direction)), incomingRay->direction->multiply(-1));

	return newRay;
}

Ray * refractiveRayFromIntersection(Ray * incomingRay, Vector3 * normal, Vector3 * intersectionPoint, float sourceMaterialConstant, float destinationMaterialConstant)
{
	/*
		
	    ->                             ->     ->
		T = (k*cos(thetai) - cos(thetat))*n - k*I

		n is the normal
		I is the oposite of incoming ray
		k = (ni/nt)
		-------------
		unknowns: T, cos(thetai), cos(thetat)

		cos(thetai) = N * I
		cos(thetat) = sqrt((1 - (k^2)(1- (N*I)^2))

		There is something wrong with the code below, or in how I used this function
		I added the line 
			if(k<1) k = 1/k
			as a quick fix, which actuall makes it look nice.

	*/

	float cosThetai, cosThetat, ni,nt, k;

	ni = sourceMaterialConstant;
	nt = destinationMaterialConstant;

	if(nt==0)
		throw "nt, div by zero";

	k =  ni/nt;

	Vector3 I = multiply(*(incomingRay->direction),-1);

	cosThetai = dotProduct(&I, normal);

	if(k>1)
		k = 1/k;

	if(1 - k*k*(1 - cosThetai*cosThetai) <0)
	{
		//constants set too high or low
		//throw "sqrt of neg";

		///send ray right throw
		Ray * newRay = new Ray();
		newRay->location= intersectionPoint->copy();
		newRay->direction= incomingRay->direction->copy();

		printf("sqrt neg\n");
		return newRay;
	}

	cosThetat = sqrt( 1 - k*k*(1 - cosThetai*cosThetai));

	//T = (k*cos(thetai) - cos(thetat))*n - k*I
	Vector3 T;

	T = subtract(multiply(*normal, (k*cosThetai - cosThetat)) , multiply(I, k));

	Ray * newRay = new Ray();
	newRay->location= intersectionPoint->copy();
	newRay->direction= T.copy();//incomingRay->direction->copy();

	//normalizeVector(newRay->direction);

	return newRay;
}