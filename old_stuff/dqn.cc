///////////////////////////////
// File: basic.cc
// Desc: minimal controller example
// Created: 2011.10.19
// Author: Richard Vaughan <vaughan@sfu.ca>
// License: GPL
/////////////////////////////////

#include "stage.hh"
#include "map"
#include "math.h"
#include "set"
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace Stg;
using namespace std;

enum RobotState
{
    UNKNOWN  = 0,
    SEARCHING = 1,
    HOMING = 2
};

#define VEL 0.9
#define PI 3.14159265359

template <typename T> int sign(T val) {
    return (T(0) < val) - (val < T(0));
}

typedef struct
{
    ModelPosition* position;
    ModelRanger* laser;
    Pose target;
    Mat laserMap;
} robot_t;

/** this is added as a callback to a ranger model, and is called
    whenever the model is updated by Stage.
*/

void execAction( robot_t* robot, int forward, int turn)
{
  robot->position->SetSpeed( 0.3*forward, 0, 0.06*turn);
}

int LaserUpdate( ModelRanger* mod, robot_t* robot)
{
    const std::vector<meters_t>& scan = robot->laser->GetSensors()[0].ranges;
    int sample_count = scan.size();
    if( sample_count < 1)
        return 0;
    else
    {
	robot->laserMap = Scalar( 128, 128, 128);
	int x = 60, y = 60, rad = 60;
	int dir = 180*robot->position->GetGlobalPose().a/PI;
  for( int ind = 0; ind < 360; ind++)
	{
    int i = ind + dir;
    for( int j = 0; j < rad; j++)
      if( scan[ind]*10>=j)
        robot->laserMap.data[ 120 *(x+ int( j*cos( PI*i/180))) + y + int( j*sin( PI*i/180))] = 255;
    if( scan[ind] < 6)
      robot->laserMap.data[ 120 *(x+ int( 10*scan[ind]*cos( PI*i/180))) + y + int( 10*scan[ind]*sin( PI*i/180))] = 0;
	}
	imshow( "Laser", robot->laserMap);
//  execAction( robot, -1);
	waitKey(1);
    }
    return 0;
}

/** Stage calls this when the model starts up 
 */
extern "C" int Init( Model* mod )
{
    robot_t* robot = new robot_t;
    robot->position = (ModelPosition*)mod;

    std::cout << "Initing";
    robot->laser = (ModelRanger*)mod->GetUnusedModelOfType("ranger") ;
    assert( robot->laser );
    robot->laser->AddCallback( Model::CB_UPDATE, (model_callback_t)LaserUpdate, robot );
    robot->laser->Subscribe();
    robot->position->Subscribe();
    robot->laserMap.create( 120, 120, CV_8U);
    namedWindow( "Laser", WINDOW_AUTOSIZE);

    return 0; //ok
}
