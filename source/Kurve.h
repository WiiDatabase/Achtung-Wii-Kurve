#include "math.h"
#include "constants.h"
class Kurve
{
//Public:
public:

// Contructor
Kurve(unsigned short, unsigned short, unsigned int, unsigned int, unsigned int);

// Variables
short x;
short y;
float direction; // in degrees
bool hole;
bool dead;
bool deactivated;
bool ready;

// Methods
void iterate();
void turnLeft();
void turnRight();
void modifyX(float);
void modifyY(float);


// Private:
private:

// Variables
float realX;
float realY;
float angularVelocity; // in radians
int holeSize;
int holeSeparation;
int holeCounter;
int timeToReady;

// Methods
void adjustDirection();
};

