/*


	These functions are implemented to replace functions in RayTrace.h/.cpp
	to inhance performance





*/

#include "PerformanceRayTrace.h"


SceneObject ***createInitialObjectIntersectionBuffer(struct Scene *scene, int width,int height)
{
	//object*[][]
	SceneObject ***buffer;

	buffer = (SceneObject ***)malloc(sizeof(SceneObject**)*width);

	for(int i =0;i<width;i++)
	{
		buffer[i] = (SceneObject **)malloc(sizeof(SceneObject*)*width);
		for(int j=0;j<height;j++)
			buffer[i][j] = NULL;
	}

	return buffer;
}//createInitialObjectIntersectionBuffer

void fillInitialObjectIntersectionBuffer(struct Scene *scene, SceneObject *** objectBuffer)
{
	Vector3 *leftTopCorner     =getUpperLeft(scene);
	Vector3 *bottomRightCorner =getBottomRight(scene);
	Vector3 * bottomLeftCorner =getBottomLeft(scene);

	int horizontalDivisions = scene->numPixelsHorizontal/PIXELS_PER_DIVISION;
	int verticalDivisions   = scene->numPixelsVertical/PIXELS_PER_DIVISION;
	for(int i=0;i<horizontalDivisions;i++)
	{
		for(int j=0;j<verticalDivisions;j++)
		{
			//need to set objectBuffer[i][j]
			int pixelCoord_x1 = PIXELS_PER_DIVISION * i;
			int pixelCoord_y1 = PIXELS_PER_DIVISION * j;
			int pixelCoord_x2 = PIXELS_PER_DIVISION * (i+1);
			int pixelCoord_y2 = PIXELS_PER_DIVISION * (j+1);
			int raysAcross   = scene->antialiasDepth;

			float percentX1 =  ((float)(pixelCoord_x1))/(scene->numPixelsHorizontal);
			float percentY1 =  ((float)(pixelCoord_y1))/(scene->numPixelsVertical);
			float percentX2 =  (((float)(pixelCoord_x2*raysAcross+raysAcross-1)))/(scene->numPixelsHorizontal*raysAcross);
			float percentY2 =  (((float)(pixelCoord_y2*raysAcross+raysAcross-1)))/(scene->numPixelsVertical*raysAcross);

			//Ray * rayFromViewPointToXY = calculateRayFromEyePointToPointOnScreen(scene,leftTopCorner,bottomRightCorner,bottomLeftCorner,percentToRight,percentDown);
			Ray * rayUpperLeft  = calculateRayFromEyePointToPointOnScreen(scene,leftTopCorner,bottomRightCorner,bottomLeftCorner,percentX1,percentY1);
			Ray * rayUpperRight = calculateRayFromEyePointToPointOnScreen(scene,leftTopCorner,bottomRightCorner,bottomLeftCorner,percentX2,percentY1);
			Ray * rayBottomLeft = calculateRayFromEyePointToPointOnScreen(scene,leftTopCorner,bottomRightCorner,bottomLeftCorner,percentX1,percentY2);
			Ray * rayBottomRight= calculateRayFromEyePointToPointOnScreen(scene,leftTopCorner,bottomRightCorner,bottomLeftCorner,percentX2,percentY2);

			SceneObject * object1 = closestIntersection(scene,rayUpperLeft, NULL, false, true, NULL);
			SceneObject * object2 = closestIntersection(scene,rayUpperRight, NULL, false, true, NULL);
			SceneObject * object3 = closestIntersection(scene,rayBottomLeft, NULL, false, true, NULL);
			SceneObject * object4 = closestIntersection(scene,rayBottomRight, NULL, false, true, NULL);

			

			if((object1!=object2)||(object2!=object3)||(object3!=object4))
				objectBuffer[i][j] = NULL;
			else
			{
				//printf("Doing check %d %d\n",i,j);
				if(object1==NULL)
				{
					printf("big fat error\n");
					objectBuffer[i][j] = NULL;
				}
				else
				{
					//if there nothing in the pyramid volume, then set as object1
					bool anythingInVolume = objectInVolume(scene, rayUpperLeft,rayUpperRight,rayBottomLeft,rayBottomRight, object1);

					if(!anythingInVolume)
					{
						//printf("optimized!");
						objectBuffer[i][j] = object1;
					}
					else
						objectBuffer[i][j] = NULL;
				}
			}
		}//for
	}

	printf("done with initial optimization step.");
}//fillInitialObjectIntersectionBuffer

void performance_RenderScene(struct Scene *scene,struct ScreenData* screenData)
{
	int depth =  scene->rayTraceDepth;
	int horizontalDivisions = scene->numPixelsHorizontal/PIXELS_PER_DIVISION;
	int verticalDivisions   = scene->numPixelsVertical/PIXELS_PER_DIVISION;

	SceneObject *** objectBuffer = createInitialObjectIntersectionBuffer(scene,horizontalDivisions,verticalDivisions);
	fillInitialObjectIntersectionBuffer(scene, objectBuffer);

	Vector3 *leftTopCorner     =getUpperLeft(scene);
	Vector3 *bottomRightCorner =getBottomRight(scene);
	Vector3 * bottomLeftCorner =getBottomLeft(scene);

	printf("Scene: %d x %d\n", screenData->width, screenData->height);

	//allocate space for shadow cache
	SceneObject ** shadowCache = (SceneObject**)malloc(sizeof(SceneObject *) * (20));//lights stored as vector so just assumed less than 20 lights
	for(int i =0;i<20;i++)
		shadowCache[i] = NULL;

	for(int x=0;x< scene->numPixelsHorizontal;x++)
	{
		if(x%(screenData->width/100)==0)
			printf("%%%f complete\n",100*((float)x)/(screenData->width));

		for(int y=0;y< scene->numPixelsVertical;y++)
		{

			ColorRGB finalColor = performance_AntiAliasTrace(scene->antialiasDepth,  x,  y, scene, leftTopCorner, bottomRightCorner, bottomLeftCorner, depth,objectBuffer[x/PIXELS_PER_DIVISION][y/PIXELS_PER_DIVISION], shadowCache);
			screenData->setPixel(x,y, finalColor);

		}

		//update the window to show partial image
		if(x%2==0)
			glutPostRedisplay();
	}//for

	glutPostRedisplay();//make sure image is displayed


}//performance_RenderScene

ColorRGB performance_TraceRay(struct Scene *scene,struct Ray *ray, int depth, SceneObject * object, SceneObject ** shadowCache)
{
	/*
		This is just like traceRay, only the parameter 'object' may be pre-set

	*/
	if(ray==NULL)
		throw "Ray is null";

	Vector3 intersectionPoint;

	if(object==NULL)
		object = closestIntersection(scene,ray, NULL, false, true,&intersectionPoint);//&intersectionPoint);
	else
	{
		//still need to computer intersection point
		//Vector3 * temp = object->intersectionPoint(ray,false);
		Vector3 * temp = object->intersectionPointAssumeIntersection(ray);
		intersectionPoint = *temp;
		delete temp;

		//return scene->ambientTerm;
	}

	if(object !=NULL)
	{
		Vector3 normalAtIntersection= object->normalAt(&intersectionPoint);
	
		//ColorRGB retVal = shade(scene, object,ray,&intersectionPoint,&normalAtIntersection, depth);
		ColorRGB retVal =performance_shade(scene, object,ray,&intersectionPoint,&normalAtIntersection, depth, shadowCache);
	
		return retVal;
	}
	else
	{
		//return background color
		return scene->ambientTerm;
	}
	
}//performance_TraceRay



ColorRGB performance_AntiAliasTrace(int raysAcross, int x, int y, struct Scene *scene, Vector3 *leftTopCorner, Vector3 *bottomRightCorner,Vector3 * bottomLeftCorner, int depth, SceneObject * object, SceneObject ** shadowCache)
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
			
			ColorRGB color    = performance_TraceRay(scene,rayFromViewPointToXY,depth, object, shadowCache);

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




bool objectInVolume(Scene * scene, Ray * rayUpperLeft,Ray *rayUpperRight,Ray *rayBottomLeft,Ray *rayBottomRight,SceneObject * object)
{
	//Cycle through all objects, return true on the first intersection
	//The argument 'object' should be skipped over, and we should only be checking for objects in front of it

	float furthestDistance;//the largest distance between the eye and 'object'
	//or

	CylinderList * cylinders = scene->cylinders;
	Polygon3List * polygons  = scene->polygons;// includes polygons from the bottle and the polyhedron
	SphereList   * spheres   = scene->spheres;
	BottleList   * bottles   = scene->bottles;
	FlatCircleList* circles  = scene->circles;

	//compute furthest distance (which is used to bound the pyramid)
	Vector3 *intersectionUpperLeft   = object->intersectionPoint(rayUpperLeft, false);
	Vector3 *intersectionUpperRight  = object->intersectionPoint(rayUpperRight, false);
	Vector3 *intersectionBottomLeft  = object->intersectionPoint(rayBottomLeft, false);
	Vector3 *intersectionBottomRight = object->intersectionPoint(rayBottomRight, false);

	float distanceUpperLeft  = distance(intersectionUpperLeft,   rayUpperLeft->location);
	float distanceUpperRight = distance(intersectionUpperRight,  rayUpperRight->location);
	float distanceBottomLeft = distance(intersectionBottomLeft,  rayBottomLeft->location);
	float distanceBottomRight= distance(intersectionBottomRight, rayBottomRight->location);

	furthestDistance = distanceUpperLeft;
	if(distanceUpperRight>furthestDistance)
		furthestDistance = distanceUpperRight;
	if(distanceBottomLeft>furthestDistance)
		furthestDistance = distanceBottomLeft;
	if(distanceBottomRight>furthestDistance)
		furthestDistance = distanceBottomRight;

	delete intersectionUpperLeft;
	delete intersectionUpperRight;
	delete intersectionBottomLeft;
	delete intersectionBottomRight;


	/*

		Not that this does not need to be super accurate

		bottle will be treated as cylinder


		Cylinders:
			bounding sphere?

		Polygons:
			to check polygon
			1)For each point on the polygon, check if it is in between a particular polygon on the pyramid
			2)See if any of the rays intersect the polygon, and whether those points are in front of 'object'

		Spheres
			distane  between a point and a line
			if less than radius, check if center is in front of the associated point
			if center is in between polygon, then check distance to center against ray through center toward object

		Circles:
			treat as polygon

	*/

		//find intersections with cylinders
	if(cylinders!=NULL)
	{
		CylinderList::iterator cylinderIterator;

		for (cylinderIterator = cylinders->begin(); cylinderIterator != cylinders->end(); cylinderIterator++)
		{
			Cylinder * cylinder = *((Cylinder  **)cylinderIterator);

			if(cylinder!= object)
			{
				if(cylinder->intersectPyramid(rayUpperLeft,rayUpperRight,rayBottomLeft,rayBottomRight,furthestDistance,true))
				{
					return true;
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

			if(circle!= object)
			{
				if(circle->intersectPyramid(rayUpperLeft,rayUpperRight,rayBottomLeft,rayBottomRight,furthestDistance,true))
				{
					return true;
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
			//Polygon3List::iterator polygonIterator;

			if(bottle->boundingVolume->intersectPyramid(rayUpperLeft,rayUpperRight,rayBottomLeft,rayBottomRight,furthestDistance,true))
			{
				return true;
			}

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
			if(polygon!= object)
			{
				if(polygon->intersectPyramid(rayUpperLeft,rayUpperRight,rayBottomLeft,rayBottomRight,furthestDistance,true))
				{
					return true;
				}
				
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

			if(sphere!= object)
			{
				if(sphere->intersectPyramid(rayUpperLeft,rayUpperRight,rayBottomLeft,rayBottomRight,furthestDistance,true))
				{
					return true;
				}
			}
		}//for
	}

	return false;
}






ColorRGB performance_shade(struct Scene *scene,SceneObject * object, struct Ray *ray, Vector3 * intersectionPoint, Vector3 * normal, int depth, SceneObject **shadowCache)
{


	ColorRGB colorOfRay;
	Ray * reflectedRay  = NULL;
	Ray * refractedRay  = NULL;//the ray entering the object
	Ray * refractedRay2  = NULL;//the ray exiting the object
	Ray shadowRay;

	colorOfRay = scene->ambientTerm;

	if(scene->lightSources!=NULL)
	{
		LightSourceList::iterator lightIterator;
		LightSourceList * lightSources = scene->lightSources;

		int i =0;

		for (lightIterator = lightSources->begin(); lightIterator != lightSources->end(); lightIterator++,i++)
		{
			
			LightSource * light = *((LightSource  **)lightIterator);
			SceneObject * theObjectThatCastAShadowOnTheLastObject = shadowCache[i];

			//shadow = ray to light from point;
			shadowRay.location = intersectionPoint;
			shadowRay.direction= normalizeVector( new Vector3(light->location.x-intersectionPoint->x,light->location.y-intersectionPoint->y,light->location.z-intersectionPoint->z));
			//if(dot product of normal and direction to light is positive)
			float dotProductOfNormalAndDirectionToLight = normal->x * shadowRay.direction->x + normal->y * shadowRay.direction->y +normal->z * shadowRay.direction->z;
			//bool objectIsIluminated = false;
			float illumination = 1;

			if(dotProductOfNormalAndDirectionToLight >0)
			{
				bool optimized = false;

				if(theObjectThatCastAShadowOnTheLastObject!=NULL)
				{
					if(theObjectThatCastAShadowOnTheLastObject == object)
					{
						shadowCache[i] = NULL;
					}
					else
					{
						Vector3 * intersectionWithLastObject = theObjectThatCastAShadowOnTheLastObject->intersectionPoint(&shadowRay,false);
						if(intersectionWithLastObject !=NULL)
						{
							optimized = true;
							delete intersectionWithLastObject;
							illumination = 0;
							//printf("optimized shadow!");

							//return scene->bogusTerm;
						}
					}
				}
				

				if(!optimized)
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

							if(closestIntersectionInFrontOfLight->transmissionCoefficient == 0)
								shadowCache[i] = closestIntersectionInFrontOfLight;

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