/*


settings for the project

  also some constants

*/


#define WINDOW_WIDTH  700
#define WINDOW_HEIGHT 700
#define SCENE_DATA_FILENAME "scene.dat"
//constants
#define MAX_RAY_TRACE_DEPTH 3 // 3 is as large as this should ever be (9 rays per pixel)
//#define RAY_CAST_INSTEAD  //This option will not do lighting effects, just outlines
//#define LIGHTLY_SHADOW
#define ANTI_ALIASING
#define ANTI_ALIAS_DEPTH 1
#define TEXTURE_ANTIALIASING  //Actually just blurs the image to get red of the jaggedness
#define NUM_BEZIER_SAMPLES 20
#define ROTATIONS          40

#define PI 3.1415926535897

