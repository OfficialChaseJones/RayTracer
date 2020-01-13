/*


	These functions are implemented to replace functions in RayTrace.h/.cpp
	to inhance performance



	Need to implement
		'objectInVolume'
	which requires implementing
		'objectInVolume' for each object


*/
#ifndef PERFORMANCE_RAY_TRACE_H
	#define PERFORMANCE_RAY_TRACE_H

#include "RayTrace.h"

#define PIXELS_PER_DIVISION 25


SceneObject ***createInitialObjectIntersectionBuffer(struct Scene *scene,int width, int height);
void fillInitialObjectIntersectionBuffer(struct Scene *scene, SceneObject *** objectBuffer);

void performance_RenderScene(struct Scene *scene,struct ScreenData* screenData);
ColorRGB performance_TraceRay(struct Scene *scene,struct Ray *ray, int depth, SceneObject * object, SceneObject **shadowCache);
ColorRGB performance_shade(struct Scene *scene,SceneObject * object, struct Ray *ray, Vector3 * intersectionPoint, Vector3 * normal, int depth, SceneObject **shadowCache);
ColorRGB performance_AntiAliasTrace(int raysAcross, int x, int y, struct Scene *scene, Vector3 *leftTopCorner, Vector3 *bottomRightCorner,Vector3 * bottomLeftCorner, int depth, SceneObject * object, SceneObject ** shadowCache);

bool objectInVolume(Scene * scene, Ray * rayUpperLeft,Ray *rayUpperRight,Ray *rayBottomLeft,Ray *rayBottomRight,SceneObject * object);



#endif