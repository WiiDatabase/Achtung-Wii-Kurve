#include "math.h"
#include "constants.h"
#include <stdlib.h>
#include "Kurve.h"

Kurve::Kurve(unsigned short x0, unsigned short y0, unsigned int turningRadius, unsigned int holeSize0, unsigned int holeSeparation0)
{
	// Positioning
	realX = x = x0;
	realY = y = y0;
	
	// Holes
	holeSize = holeSize0;
	holeSeparation = holeSeparation0;
	holeCounter = holeSize+rand()%holeSeparation;
	hole = false;
	
	// Dead / deactivated / Ready
	dead = false;
	deactivated = false;
	ready = false;
	timeToReady = 60;
	
	// Directioning
	direction = rand()%360;
	angularVelocity = 1/((float)turningRadius);
}

void Kurve::iterate()
{
	// Make ready
	if (timeToReady>0)
	{
		timeToReady--;
		if (timeToReady==0)
		{
			ready=true;
		}
	}
	
	// Iterate if ready
	if (ready==true)
	{
		// Positioning
		realX += cos(direction*PI/180);
		realY += sin(direction*PI/180);
		x=realX;
		y=realY;
		
		// Holing
		holeCounter++;
		if (holeCounter <= holeSize)
		{
			hole = true;
		} else
			hole = false;
		if (holeCounter > holeSize+holeSeparation)
		{
			holeCounter = 0;
		}
	}
}

void Kurve::turnLeft()
{
	direction -= angularVelocity*180/PI;
	adjustDirection();
}

void Kurve::turnRight()
{
	direction += angularVelocity*180/PI;
	adjustDirection();
}

void Kurve::adjustDirection()
{
	if (direction<0)
	{
		direction+=360;
	}

	if (direction>360)
	{
		direction-=360;
	}
}

void Kurve::modifyX(float difference)
{
	realX+=difference;
}

void Kurve::modifyY(float difference)
{
	realY+=difference;
}
