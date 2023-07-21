#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <map>
#include <conio.h>
#include <freeglut/GL/freeglut.h>




#include <raaCamera/raaCamera.h>
#include <raaUtilities/raaUtilities.h>
#include <raaMaths/raaMaths.h>
#include <raaMaths/raaVector.h>
#include <raaSystem/raaSystem.h>
#include <raaPajParser/raaPajParser.h>
#include <raaText/raaText.h>

#include "raaConstants.h"
#include "raaParse.h"
#include "raaControl.h"
#include <time.h>

//Key Value pairs for Menus for Display & other
#define DEFAULT_LAYOUT 1 
#define CONTINENT_LAYOUT 2
#define WORLD_SYSTEM_LAYOUT 3
#define RESET_CAMERA 3
#define TOGGLE_HELP 4
#define TOGGLE_STATS 5
#define RESET_ALL_VARS 6
#define RESET_ALL 7
//For user definable parameters 
//Key Value pairs for Arc Length Constant
#define ARC_LENGTH_FACTOR_RESET 4
#define ARC_LENGTH_FACTOR_0 0
#define ARC_LENGTH_FACTOR_1 1
#define ARC_LENGTH_FACTOR_2 2
#define ARC_LENGTH_FACTOR_3 3
//Key Value pairs for Damping value
#define DAMPING_VALUE_RESET 4
#define DAMPING_VALUE_0 0
#define DAMPING_VALUE_1 1
#define DAMPING_VALUE_2 2
#define DAMPING_VALUE_3 3
//Key Value pairs for Coulombs Constant
#define COULOMBS_CONSTANT_RESET 4
#define COULOMBS_CONSTANT_0 0
#define COULOMBS_CONSTANT_1 1
#define COULOMBS_CONSTANT_2 2
#define COULOMBS_CONSTANT_3 3
//Key Value pairs for Coulombs Range of affect
#define COULOMBS_ROA_RESET 4
#define COULOMBS_ROA_0 0
#define COULOMBS_ROA_1 1
#define COULOMBS_ROA_2 2
#define COULOMBS_ROA_3 3
//Key Value pairs for time control
#define TIME_INTERVAL_RESET 2
#define TIME_INTERVAL_MINUS 0
#define TIME_INTERVAL_PLUS 1

//Default, user changeable values
#define DEFAULT_ARC_LENGTH_FACTOR 300
#define DEFAULT_DAMPING_VALUE 0.5
#define DEFAULT_COULOMBS_CONSTANT 50
#define DEFAULT_COULOMBS_ROA 100
#define DEFAULT_TIME_INTERVAL 0.01f

// NOTES
// look should look through the libraries and additional files I have provided to familarise yourselves with the functionallity and code.
// The data is loaded into the data structure, managed by the linked list library, and defined in the raaSystem library.
// You will need to expand the definitions for raaNode and raaArc in the raaSystem library to include additional attributes for the siumulation process
// If you wish to implement the mouse selection part of the assignment you may find the camProject and camUnProject functions usefull


// core system global data
raaCameraInput g_Input; // structure to hadle input to the camera comming from mouse/keyboard events
raaCamera g_Camera; // structure holding the camera position and orientation attributes
raaSystem g_System; // data structure holding the imported graph of data - you may need to modify and extend this to support your functionallity
raaControl g_Control; // set of flag controls used in my implmentation to retain state of key actions


float smallestArc = 10000.0f;  //For normalizing arc length
float biggestArc = 0.0f;
boolean isContinentLayout = false; //Flag for if moving to continent static layout
boolean returnToDefault = false; //Same, but for default layout
boolean isWorldSystemLayout = false;
 //Used for position in linked list for calculating repulsion
int currentPNodePos = 1;
int currentSNodePos = 1;
raaNode* currentNode;
boolean isSimulating = false; //Flags if simulation is running
float systemKineticEnergy = 0; //Energy of system
int frameCount =0; //How many frames nodes have been moved when returning to a static layout
float continentPositioningX = 0.0f; //X,Y & Z co-ords needed for Continent static layout
float continentPositioningY = 0.0f;
float continentPositioningZ[5];
//Just to calculate frame rate
int frameRate = 0;
int lastFrameTime = 0; 
int timeSinceLastFrameRateUpdate = 0;
//For altering ideal Length of arc
float arcLengthFactor = DEFAULT_ARC_LENGTH_FACTOR;
float arcLenghtFactorOptions[4] = { -100,-10,10,100};
//For altering damping value
float dampingValue = DEFAULT_DAMPING_VALUE;
float dampingValueOptions[4] = { -0.1,-0.01,0.01,0.1 };
//For altering Coloumbs constant
float coulombsConstant = DEFAULT_COULOMBS_CONSTANT;
float coulombsConstantOptions[4] = { -10,-1,1,10 };
//For altering Coloumbs range of affect
float coulombsRoA = DEFAULT_COULOMBS_ROA;
float coulombsRoAOptions[4] = { -25,-5,5,25 };
float timeInterval = DEFAULT_TIME_INTERVAL;
//Flags to allow toggling of hud text
boolean displayHelp = true;
boolean displayStats = true;
 

// global var: parameter name for the file to load
const static char csg_acFileParam[] = {"-input"};

// global var: file to load data from
char g_acFile[256];

// core functions -> reduce to just the ones needed by glut as pointers to functions to fulfill tasks
void display(); // The rendering function. This is called once for each frame and you should put rendering code here
void idle(); // The idle function is called at least once per frame and is where all simulation and operational code should be placed
void reshape(int iWidth, int iHeight); // called each time the window is moved or resived
void keyboard(unsigned char c, int iXPos, int iYPos); // called for each keyboard press with a standard ascii key
void keyboardUp(unsigned char c, int iXPos, int iYPos); // called for each keyboard release with a standard ascii key
void sKeyboard(int iC, int iXPos, int iYPos); // called for each keyboard press with a non ascii key (eg shift)
void sKeyboardUp(int iC, int iXPos, int iYPos); // called for each keyboard release with a non ascii key (eg shift)
void mouse(int iKey, int iEvent, int iXPos, int iYPos); // called for each mouse key event
void motion(int iXPos, int iYPos); // called for each mouse motion event

// Non glut functions
void myInit(); // the myinit function runs once, before rendering starts and should be used for setup
void nodeDisplay(raaNode *pNode); // callled by the display function to draw nodes
void arcDisplay(raaArc *pArc); // called by the display function to draw arcs
void buildGrid(); // 


void calcArcIdealLength(raaArc* pArc);
void resetResultantForce(raaNode* pNode);
void calcFromArcs(raaArc* pArc);
void calcFromNodes(raaNode* pNode);
void getArcLengthBounds(raaArc* pArc);
void processMenuEvents(int option);
void menuCreator();
void processDisplayEvents(int option);
void updatePos(raaNode* pNode);
void calcMoveToDefault(raaNode* pNode);
void setDefaultNodePos(raaNode* pNode);
void resetSimulation();
void resetVelocity(raaNode* pNode);
void calcMoveToContinent(raaNode* pNode);
void checkAndApplyRepulsion(raaNode* sNode);
void changeArcLength(int option);
void changeDampingValue(int option);
void processArcOptionEvents(int option);
void processDampingOptionEvents(int option);
void processCoulombsOptionEvents(int option);
void processCoulombsRoAOptionEvents(int option);
void prepareForStaticLayout();
void calcMoveToWorldSystem(raaNode* pNode);
void setStaticLayoutStartVecCoords();
void processTimeOptionEvents(int option);

void updatePos(raaNode* pNode) {
	//Moves the node to the decided static place
	for (int i = 0; i < 3; i++)
	{
		pNode->m_afPosition[i] += pNode->movePerFrame[i];
	}
}
void changeTimeInterval(int option) {
	switch (option) //Check for validity first to prevent divide by 0
	{
	case TIME_INTERVAL_MINUS:
		if (timeInterval >=0.0006f)
		{
			timeInterval /= 1.25;
		}
		
		break;
	case TIME_INTERVAL_PLUS:
		if (timeInterval<=0.024414f)
		{
			timeInterval*=1.25;
		}
		
		break;
	default:
		break;
	}
}
void changeArcLength(int option) {
	arcLengthFactor += arcLenghtFactorOptions[option];
	if (arcLengthFactor < arcLenghtFactorOptions[2])
	{
		arcLengthFactor = arcLenghtFactorOptions[2];
	}
	else if (arcLengthFactor > 10000) {
		arcLengthFactor = 10000;
	}
	visitArcs(&g_System, calcArcIdealLength);
}

void changeDampingValue(int option) {
	dampingValue += dampingValueOptions[option];
	if (dampingValue<0)
	{
		dampingValue = 0;
	}
	else if (dampingValue > 1) {
		dampingValue = 1;
	}
}

void changeCoulombsConstant(int option){
	coulombsConstant += coulombsConstantOptions[option];
	if (coulombsConstant<0)
	{
		coulombsConstant = 0;
	}
	else if (coulombsConstant > 1000) {
		coulombsConstant = 1000;
	}
}
void changeCoulomsRoA(int option) {
	coulombsRoA += coulombsRoAOptions[option];
	if (coulombsRoA<0)
	{
		coulombsRoA = 0;
	}
	else if (coulombsRoA > 10000) {
		coulombsRoA = 10000;
	}
}
void getArcLengthBounds(raaArc* pArc) {
	//Gets the range of arc lengths based on the formula F=-kx, where F is the mass of the 2 nodes. 
	//This calculates the resting length of the spring/arc.
	//However, some are too small or too large for a readable layout. The bounds will be used to normalize this numbers
	float arcLength = ((pArc->m_pNode0->m_fMass + pArc->m_pNode1->m_fMass) / pArc->m_fSpringCoef);
	if (arcLength < smallestArc)
	{
		smallestArc = arcLength;
	}
	if (arcLength > biggestArc) {
		biggestArc = arcLength;
	}
}

void calcArcIdealLength(raaArc* pArc) {
	//Length of arc decided from a factor of mass of connected nodes / SpringCoef, normaized with all other arcs
	float arcLength = ((pArc->m_pNode0->m_fMass + pArc->m_pNode1->m_fMass) / pArc->m_fSpringCoef);
	pArc->m_fIdealLen = (1 + ((arcLength - smallestArc) / (biggestArc - smallestArc))) * arcLengthFactor;
}

void resetResultantForce(raaNode* pNode) {
	//Resets all nodes resultant force to 0, ready for preceeding calculations
	for (int i = 0; i < 3; i++)
	{
		pNode->forceVect[i] = 0.0f;
	
	}
}

void menuCreator() {
	//Sub menu displaying display options
	char textForStrings[128]; //Allows for the creation of a char array, containing variables
	
	
	int displayMenu = glutCreateMenu(processDisplayEvents);
	glutAddMenuEntry("Default", DEFAULT_LAYOUT);
	glutAddMenuEntry("By Continent", CONTINENT_LAYOUT);
	glutAddMenuEntry("By World System", WORLD_SYSTEM_LAYOUT);

	//Sub menu for Arc Length Constant options
	int arcOptionMenu = glutCreateMenu(processArcOptionEvents);
	glutAddMenuEntry("Reset", ARC_LENGTH_FACTOR_RESET);
	sprintf(textForStrings, "%.2fx", arcLenghtFactorOptions[0]);
	glutAddMenuEntry(textForStrings, ARC_LENGTH_FACTOR_0);
	sprintf(textForStrings, "%.2fx", arcLenghtFactorOptions[1]);
	glutAddMenuEntry(textForStrings, ARC_LENGTH_FACTOR_1);
	sprintf(textForStrings, "%.2fx", arcLenghtFactorOptions[2]);
	glutAddMenuEntry(textForStrings, ARC_LENGTH_FACTOR_2);
	sprintf(textForStrings, "%.2fx", arcLenghtFactorOptions[3]);
	glutAddMenuEntry(textForStrings, ARC_LENGTH_FACTOR_3);

	//Sub menu for Damping value option
	int dampingOptionMenu = glutCreateMenu(processDampingOptionEvents);
	glutAddMenuEntry("Reset", DAMPING_VALUE_RESET);
	sprintf(textForStrings, "%.2f", dampingValueOptions[0]);
	glutAddMenuEntry(textForStrings, DAMPING_VALUE_0);
	sprintf(textForStrings, "%.2f", dampingValueOptions[1]);
	glutAddMenuEntry(textForStrings, DAMPING_VALUE_1);
	sprintf(textForStrings, "%.2f", dampingValueOptions[2]);
	glutAddMenuEntry(textForStrings, DAMPING_VALUE_2);
	sprintf(textForStrings, "%.2f", dampingValueOptions[3]);
	glutAddMenuEntry(textForStrings, DAMPING_VALUE_3);

	//Sub menu for Coulombs Constant options
	int coulomsOptionsMenu = glutCreateMenu(processCoulombsOptionEvents);
	glutAddMenuEntry("Reset", COULOMBS_CONSTANT_RESET);
	sprintf(textForStrings, "%.2fx", coulombsConstantOptions[0]);
	glutAddMenuEntry(textForStrings, COULOMBS_CONSTANT_0);
	sprintf(textForStrings, "%.2fx", coulombsConstantOptions[1]);
	glutAddMenuEntry(textForStrings, COULOMBS_CONSTANT_1);
	sprintf(textForStrings, "%.2fx", coulombsConstantOptions[2]);
	glutAddMenuEntry(textForStrings, COULOMBS_CONSTANT_2);
	sprintf(textForStrings, "%.2fx", coulombsConstantOptions[3]);
	glutAddMenuEntry(textForStrings, COULOMBS_CONSTANT_3);

	//Sub menu for Coulombs area of affect options
	int coulomsRoAOptionsMenu = glutCreateMenu(processCoulombsRoAOptionEvents);
	glutAddMenuEntry("Reset", COULOMBS_ROA_RESET); 
	sprintf(textForStrings, "%.2f", coulombsRoAOptions[0]);
	glutAddMenuEntry(textForStrings, COULOMBS_ROA_0);
	sprintf(textForStrings, "%.2f", coulombsRoAOptions[1]);
	glutAddMenuEntry(textForStrings, COULOMBS_ROA_1);
	sprintf(textForStrings, "%.2f", coulombsRoAOptions[2]);
	glutAddMenuEntry(textForStrings, COULOMBS_ROA_2);
	sprintf(textForStrings, "%.2f", coulombsRoAOptions[3]);
	glutAddMenuEntry(textForStrings, COULOMBS_ROA_3);

	//Sub menu for time options
	int timeOptionsMenu = glutCreateMenu(processTimeOptionEvents);
	glutAddMenuEntry("Reset", TIME_INTERVAL_RESET);
	glutAddMenuEntry("+", TIME_INTERVAL_PLUS);
	glutAddMenuEntry("-", TIME_INTERVAL_MINUS);

	//Menu thats displayed on right click
	int mainMenu = glutCreateMenu(processMenuEvents);
	
	glutAddMenuEntry("Toggle Help", TOGGLE_HELP);
	glutAddMenuEntry("Toggle Stats", TOGGLE_STATS);
	glutAddSubMenu("Display:", displayMenu);
	glutAddSubMenu("Arc Length Constant", arcOptionMenu);
	glutAddSubMenu("Damping Value", dampingOptionMenu);
	glutAddSubMenu("Coulombs Constant", coulomsOptionsMenu);
	glutAddSubMenu("Coulombs range of affect", coulomsRoAOptionsMenu);
	glutAddSubMenu("Time Step", timeOptionsMenu);
	glutAddMenuEntry("Reset Camera", RESET_CAMERA);
	glutAddMenuEntry("Reset All Variables", RESET_ALL_VARS);
	glutAddMenuEntry("Reset Everything", RESET_ALL);

	// attach the menu to the right button
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void processTimeOptionEvents(int option) {
	switch (option)
	{
	case TIME_INTERVAL_MINUS:
		changeTimeInterval(TIME_INTERVAL_MINUS);
		break;
	case TIME_INTERVAL_PLUS:
		changeTimeInterval(TIME_INTERVAL_PLUS);
		break;
	case TIME_INTERVAL_RESET:
		timeInterval = DEFAULT_TIME_INTERVAL;
		break;
	default:
		break;
	}
}
void processDampingOptionEvents(int option) {
	switch (option)
	{
	case DAMPING_VALUE_RESET:
		dampingValue = DEFAULT_DAMPING_VALUE;
		break;
	case DAMPING_VALUE_0:
		changeDampingValue(0);
		break;
	case DAMPING_VALUE_1:
		changeDampingValue(1);
		break;
	case DAMPING_VALUE_2:
		changeDampingValue(2);
		break;
	case DAMPING_VALUE_3:
		changeDampingValue(3);
		break;
	default:
		break;
	}
}
void processCoulombsOptionEvents(int option) {
	switch (option)
	{
	case COULOMBS_CONSTANT_RESET:
		coulombsConstant = DEFAULT_COULOMBS_CONSTANT;
		break;
	case COULOMBS_CONSTANT_0:
		changeCoulombsConstant(0);
		break;
	case COULOMBS_CONSTANT_1:
		changeCoulombsConstant(1);
		break;
	case COULOMBS_CONSTANT_2:
		changeCoulombsConstant(2);
		break;
	case COULOMBS_CONSTANT_3:
		changeCoulombsConstant(3);
		break;
	default:
		break;
	}
}
void processCoulombsRoAOptionEvents(int option) {
	switch (option)
	{
	case COULOMBS_ROA_RESET:
		coulombsRoA = DEFAULT_COULOMBS_ROA;
		break;
	case COULOMBS_CONSTANT_0:
		changeCoulomsRoA(0);
		break;
	case COULOMBS_CONSTANT_1:
		changeCoulomsRoA(1);
		break;
	case COULOMBS_CONSTANT_2:
		changeCoulomsRoA(2);
		break;
	case COULOMBS_CONSTANT_3:
		changeCoulomsRoA(3);
		break;
	default:
		break;
	}
}

void processArcOptionEvents(int option) {
	switch (option)
	{
	case ARC_LENGTH_FACTOR_RESET:
		arcLengthFactor = DEFAULT_ARC_LENGTH_FACTOR;
		visitArcs(&g_System, calcArcIdealLength);
	case ARC_LENGTH_FACTOR_0:
		changeArcLength(0);
		break;
	case ARC_LENGTH_FACTOR_1:
		changeArcLength(1);
		break;
	case ARC_LENGTH_FACTOR_2:
		changeArcLength(2);
		break;
	case ARC_LENGTH_FACTOR_3:
		changeArcLength(3);
		break;
	default:
		break;
	}
}
void prepareForStaticLayout() {
	isSimulating = false;
	resetSimulation();
	frameCount = 0;
	systemKineticEnergy = 0;
	returnToDefault = false;
	isContinentLayout = false;
	isWorldSystemLayout = false;
}
void setStaticLayoutStartVecCoords() {
	continentPositioningX = 700.0f; //Depth
	continentPositioningY = 600.0f; //Up Down
	for (int i = 0; i < 6; i++)
	{
		continentPositioningZ[i] = -100.0f;
	}
}

void processDisplayEvents(int option) {
	//Hands displayMenu buttons
	switch (option)
	{
	case DEFAULT_LAYOUT:
		//turn off and reset simulation, calculate distance to move per frame, flag return to Defualt mode
		prepareForStaticLayout();
		visitNodes(&g_System, calcMoveToDefault);
		returnToDefault = true;		
		break;
	case CONTINENT_LAYOUT:
		prepareForStaticLayout();
		setStaticLayoutStartVecCoords();
		visitNodes(&g_System, calcMoveToContinent);
		setStaticLayoutStartVecCoords;
		isContinentLayout = true;
		break;
	case WORLD_SYSTEM_LAYOUT:
		prepareForStaticLayout();
		setStaticLayoutStartVecCoords();
		visitNodes(&g_System, calcMoveToWorldSystem);
		setStaticLayoutStartVecCoords;
		isWorldSystemLayout = true;
	default:
		break;
	}
}
void calcMoveToWorldSystem(raaNode* pNode) {
	int worldSystemNo = pNode->m_uiWorldSystem;
	pNode->movePerFrame[0] = (continentPositioningX - pNode->m_afPosition[0]) / 100; //Depth. Doesnt alter
	
	int selectedRow = (worldSystemNo - 1) * 2;
	float isSecondRow = false;
	if (continentPositioningZ[selectedRow] > 1800)
	{
		selectedRow++;
		isSecondRow = true;
	}
	int yMoveFactor;
	if (isSecondRow)
	{
		yMoveFactor = 25;
	}
	else {
		yMoveFactor = 0;
	}
	pNode->movePerFrame[1] = ((continentPositioningY - (selectedRow * 100)+yMoveFactor - pNode->m_afPosition[1])) / 100;//Up Down. Creates rows of same continent
	pNode->movePerFrame[2] = ((continentPositioningZ[selectedRow]) - pNode->m_afPosition[2]) / 100; //Left Right. Based on position of last drawn on that row
	continentPositioningZ[selectedRow] += 75.0f;
}

void calcMoveToContinent(raaNode* pNode) {
	int continentNo = pNode->m_uiContinent;
	pNode->movePerFrame[0] = (continentPositioningX-pNode->m_afPosition[0])/100; //Depth. Doesnt alter
	pNode->movePerFrame[1] = ((continentPositioningY - ((continentNo - 1) * 100)-pNode->m_afPosition[1]))/100;//Up Down. Creates rows of same continent
	pNode->movePerFrame[2] = (continentPositioningZ[continentNo - 1]-pNode->m_afPosition[2])/100; //Left Right. Based on position of last drawn on that row
	continentPositioningZ[continentNo - 1] += 75.0f;
	
}
void resetSimulation() {
	//Completely resets simulation. Needed for when a static layout is chosen. 
	visitNodes(&g_System, resetResultantForce);
	visitNodes(&g_System, resetVelocity);
}

void resetVelocity(raaNode* pNode) {
	//Removes node velocity. Required when a static layout is chosen
	for (int i = 0; i < 3; i++)
	{
		pNode->veloVect[i] = 0.0f;
	}
}
void calcMoveToDefault(raaNode* pNode) {
	//Works out how much each node must move to reach its original position in 100 frames
	for (int i = 0; i < 3; i++)
	{
		pNode->movePerFrame[i] = ( pNode->defaultPosition[i] - pNode->m_afPosition[i]) / 100.0f;
	}
}

void processMenuEvents(int option) {
	//Handles Main Menu events. 
	switch (option)
	{
	case RESET_CAMERA:
		camReset(g_Camera);
		break;
	case TOGGLE_HELP:
		if (displayHelp)
		{
			displayHelp = false;
		}
		else {
			displayHelp = true;
		}
		break;
	case TOGGLE_STATS:
		if (displayStats) {
			displayStats = false;
		}
		else {
			displayStats = true;
		}
	case RESET_ALL_VARS:
		arcLengthFactor = DEFAULT_ARC_LENGTH_FACTOR;
		visitArcs(&g_System, calcArcIdealLength);
		dampingValue = DEFAULT_DAMPING_VALUE;
		coulombsConstant = DEFAULT_COULOMBS_CONSTANT;
		coulombsRoA = DEFAULT_COULOMBS_ROA;
		timeInterval = DEFAULT_TIME_INTERVAL;
		break;
	case RESET_ALL:
		arcLengthFactor = DEFAULT_ARC_LENGTH_FACTOR;
		visitArcs(&g_System, calcArcIdealLength);
		dampingValue = DEFAULT_DAMPING_VALUE;
		coulombsConstant = DEFAULT_COULOMBS_CONSTANT;
		coulombsRoA = DEFAULT_COULOMBS_ROA;
		timeInterval = DEFAULT_TIME_INTERVAL;
		camReset(g_Camera);
		//turn off and reset simulation, calculate distance to move per frame, flag return to Defualt mode
		isSimulating = false;
		resetSimulation();
		visitNodes(&g_System, calcMoveToDefault);
		returnToDefault = true;
		isContinentLayout = false;
		isWorldSystemLayout = false;
		frameCount = 0;
		systemKineticEnergy = 0;
		break;
	default:
		break;
	}
}

void calculateRepulsion(raaNode* pNode) {
	currentNode = pNode;
	visitNodes(&g_System, checkAndApplyRepulsion);
	currentSNodePos = 1;
	currentPNodePos++;
}
void checkAndApplyRepulsion(raaNode* sNode) {
	if (currentSNodePos>currentPNodePos)
	{
		//Calculate the distance they're away in all axis, including absolute values
		float vectDiffs[3];
		float absVectDiffs[3];
		vectDiffs[0] = currentNode->m_afPosition[0] - sNode->m_afPosition[0];
		vectDiffs[1] = currentNode->m_afPosition[1] - sNode->m_afPosition[1];
		vectDiffs[2] = currentNode->m_afPosition[2] - sNode->m_afPosition[2];
		absVectDiffs[0] = abs(vectDiffs[0]);
		absVectDiffs[1] = abs(vectDiffs[1]);
		absVectDiffs[2] = abs(vectDiffs[2]);
		//Calculate how far away the nodes are from eachoth
		float currentLen = sqrt((vectDiffs[0] * vectDiffs[0]) + (vectDiffs[1] * vectDiffs[1]) + (vectDiffs[2] * vectDiffs[2]));
		if (currentLen < coulombsRoA) //If they're less than this close together
		{
			//Apply repulsion, based on coloumbs law to prevent node overlap
			float force = coulombsConstant * ((currentNode->m_fMass * sNode->m_fMass) / (currentLen * currentLen));
			float sumOfDiffs = absVectDiffs[0] + absVectDiffs[1] + absVectDiffs[2];
			float vectForce[3];
			vectForce[0] = (absVectDiffs[0] / sumOfDiffs) * force;
			vectForce[1] = (absVectDiffs[1] / sumOfDiffs) * force;
			vectForce[2] = (absVectDiffs[2] / sumOfDiffs) * force;
			//Calculate direction force is being applied to each node. (One +, one -)
			for (int z = 0; z < 3; z++)
			{
				if (vectDiffs[z] <= 0) {
					currentNode->forceVect[z] -= vectForce[z];
					sNode->forceVect[z] += vectForce[z];
				}
				else {
					currentNode->forceVect[z] += vectForce[z];
					sNode->forceVect[z] -= vectForce[z];
				}
			}
		}
	}
	currentSNodePos++;
	
}
void setDefaultNodePos(raaNode* pNode) {
	//Fill in data relating to default position of nodes
	for (int i = 0; i < 4; i++)
	{
		pNode->defaultPosition[i] = pNode->m_afPosition[i];
	}
}

void calcFromArcs(raaArc *pArc) {
	//calculate current length of arc (with both directed and absoluted values for differnces in axis)
	raaNode* n1 = pArc->m_pNode0;
	raaNode* n2 = pArc->m_pNode1;
	float vectDiffs[3];
	vectDiffs[0] = n1->m_afPosition[0] - n2->m_afPosition[0];
	vectDiffs[1] = n1->m_afPosition[1] - n2->m_afPosition[1];
	vectDiffs[2] = n1->m_afPosition[2] - n2->m_afPosition[2];
	float absVectDiffs[3];
	absVectDiffs[0] = abs(vectDiffs[0]);
	absVectDiffs[1] = abs(vectDiffs[1]);
	absVectDiffs[2] = abs(vectDiffs[2]);
	float currentLen = sqrt((vectDiffs[0] * vectDiffs[0]) + (vectDiffs[1] * vectDiffs[1]) + (vectDiffs[2] * vectDiffs[2]));
	//Calculate total applied force, based on Hookes law F =-kx, then applied force in each vector direction
	float force = (pArc->m_fSpringCoef * (pArc->m_fIdealLen - currentLen));
	//How much of this force is applied to each unit of length
	float forcePerUnit = (force/currentLen); 
	//Calc applied force in each direction
	float vectForce[3];
	vectForce[0] = absVectDiffs[0] * forcePerUnit;
	vectForce[1] = absVectDiffs[1] * forcePerUnit;
	vectForce[2] = absVectDiffs[2] * forcePerUnit;
	//Calculate direction force is being applied to each node. (One +, one -)
	for (int i = 0; i < 3; i++)
	{
		if (vectDiffs[i] <= 0) {
			n1->forceVect[i] -= vectForce[i];
			n2->forceVect[i] += vectForce[i];
		}
		else {
			n1->forceVect[i] += vectForce[i];
			n2->forceVect[i] -= vectForce[i];
		}
	}	
}

void calcFromNodes(raaNode* pNode) {
	
	//Calculate acceleration, displacement and velocity
	for (int i = 0; i < 3; i++)
	{
		float currentAcc = (pNode->forceVect[i] / pNode->m_fMass) * (1 - dampingValue);
		float displacement = (pNode->veloVect[i]* timeInterval) + (0.5*(currentAcc* timeInterval * timeInterval));
		pNode->m_afPosition[i] += displacement;
		pNode->veloVect[i] = (displacement / timeInterval) *(1-dampingValue);
	}
	//Calculate node velocity, kinetic energy and add to total system energy
	float nodeVelocity = sqrt((pNode->veloVect[0] * pNode->veloVect[0]) + (pNode->veloVect[1] * pNode->veloVect[1]) + (pNode->veloVect[2] * pNode->veloVect[2]));
	float kineticEnergy = 0.5 * pNode->m_fMass * nodeVelocity * nodeVelocity;
	systemKineticEnergy += kineticEnergy;
}
void calculateFrameRate() {
	
	int time = glutGet(GLUT_ELAPSED_TIME);
	timeSinceLastFrameRateUpdate += (time-lastFrameTime);
	
	if (timeSinceLastFrameRateUpdate > 250) {
		frameRate = (1.0f / (time - lastFrameTime)*1000);
		timeSinceLastFrameRateUpdate = 0;
	}
	lastFrameTime = time;
}

void nodeDisplay(raaNode *pNode) // function to render a node (called from display())
{
	float* position = pNode->m_afPosition; 
	unsigned int continent = pNode->m_uiContinent;
	unsigned int world_system = pNode->m_uiWorldSystem;
	char* name = pNode->m_acName;

	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	float afCol[4] = { 0.0f , 0.0f, 0.0f, 1.0f };
	//Pick colour of shape, based on continent of represented country
	switch (continent) {
	case 1: //Africa - Red
		afCol[0] = 1.0f; //Set 'r' in rgba
		break;
	case 2: //Asia - Green
		afCol[1] = 1.0f; //Set 'g' in rgba
		break;
	case 3: //Europe - Blue
		afCol[2] = 1.0f; //'b' in rgba
		break;
	case 4: //North America - Pink
		afCol[0] = 1.0f; // 'r' in rgba
		afCol[2] = 1.0f; // 'b' in rgba
		break;
	case 5: //Oceania - Yellow
		afCol[0] = 1.0f; // 'r' in rgba
		afCol[1] = 1.0f; // 'g' in rgba
		break;
	case 6: //South America - Teal
		afCol[1] = 1.0f; // 'g' in rgba
		afCol[2] = 1.0f; // 'b' in rgba
		break;
	default: //If case not found, black is chosen
		break;
	}
	utilitiesColourToMat(afCol, 2.0f); //Build and set material
	glTranslated(position[0], position[1], position[2]); //Use position array to set origin of shape
	float rad;
	//Pick shape, based on World_System
	switch (world_system)
	{
	case 1:
		rad = mathsRadiusOfSphereFromVolume(pNode->m_fMass);
		glutSolidSphere(rad, 25, 25); //Draw primitive
		glTranslated(0.0f, rad, 0.0f);
		break;
	case 2: //Draw cube
		rad = mathsDimensionOfCubeFromVolume(pNode->m_fMass);
		glutSolidCube(rad);
		glTranslated(0.0f,0.5*rad, 0.0f);
		break;
	case 3:
		rad = mathsRadiusOfIcosahedron(pNode->m_fMass);
		glScalef(rad, rad, rad);
		glutSolidIcosahedron();
		glScalef(1/rad, 1/rad, 1/rad);
		glTranslated(0.0f, rad, 0.0f);
		break;
	default: //If case not found, draw teapot
		rad = mathsDimensionOfCubeFromVolume(pNode->m_fMass);
		glutSolidTeapot(rad);
		break;
	}
	glMultMatrixf(camRotMatInv(g_Camera));
	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	glColor3f(afCol[0], afCol[1], afCol[2]); 
	glTranslated(0.0f, 10.0f, 0.0f);	
	glScalef(25.0f, 25.0f, 1.0f);
	outlinePrint(pNode->m_acName);
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	
	glPopMatrix();
	glPopAttrib();
}

void arcDisplay(raaArc *pArc) // function to render an arc (called from display())
{
	raaNode* m_pNode0 = pArc->m_pNode0; 
	raaNode* m_pNode1 = pArc->m_pNode1;
	float* arcPos0 = m_pNode0->m_afPosition; 
	float* arcPos1 = m_pNode1->m_afPosition;
	
	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);

	glBegin(GL_LINES); //Create line between two points
	glColor3f(1.0f, 0.0f, 0.0f); //Start red
	glVertex3f(arcPos0[0], arcPos0[1], arcPos0[2]);
	glColor3f(0.0f, 1.0f, 0.0f); //Start green
	glVertex3f(arcPos1[0], arcPos1[1], arcPos1[2]);
	glEnd();

	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
}
void addText() {
	glDisable(GL_LIGHTING);	
	char textForStrings[128];
	int statOffset = 30;
	glColor3f(0.0f, 0.0f, 0.0f);
		glRasterPos3f(10, 15, 0);
		sprintf(textForStrings, "Current Frame Rate:%i", frameRate);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)textForStrings);

	if (displayHelp)
	{
		statOffset = 270;
		glColor3f(0.0f, 0.0f, 1.0f);
		glRasterPos3f(10,30, 0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)"Press 'q' to toggle the simulation");
	


		glColor3f(0.0f, 0.0f, 0.0f);
		glRasterPos3f(10, 45, 0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)"Press '1' to toggle Help");

		glColor3f(0.0f, 0.0f, 1.0f);
		glRasterPos3f(10, 60, 0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)"Press '2' to toggle Stats");

		glColor3f(0.0f, 0.0f, 0.0f);
		glRasterPos3f(10, 75, 0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)"Right-click for more options");
		glColor3f(0.0f, 0.0f, 1.0f);
		glRasterPos3f(10, 90, 0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)"Press 'G' to toggle the grid");
		glColor3f(0.0f, 0.0f, 0.0f);
		glRasterPos3f(10, 105, 0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)"Camera Controls: W->Zoom in, S->Zoom out Middle Mouse->Pan");

		glColor3f(0.0f, 0.0f, 1.0f);
		glRasterPos3f(10, 120, 0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)"Change Arc Lengths by a factor using:");
		glRasterPos3f(10, 135, 0);
		sprintf(textForStrings, "T=%.2fx Y=%.2fx U=%.2fx I=%.2fx", arcLenghtFactorOptions[0], arcLenghtFactorOptions[1], arcLenghtFactorOptions[2], arcLenghtFactorOptions[3]);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)textForStrings);

		glColor3f(0.0f, 0.0f, 0.0f);
		glRasterPos3f(10, 150, 0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)"Change Node Damping value using:");
		glRasterPos3f(10, 165, 0);
		sprintf(textForStrings, "H=%.2f J=%.2f K=%.2f L=%.2f", dampingValueOptions[0], dampingValueOptions[1], dampingValueOptions[2], dampingValueOptions[3]);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)textForStrings);

		glColor3f(0.0f, 0.0f, 1.0f);
		glRasterPos3f(10, 180, 0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)"Change Coulombs constant using:");
		glRasterPos3f(10, 195, 0);
		sprintf(textForStrings, "Z=%.2fx X=%.2fx C=%.2fx V=%.2fx", coulombsConstantOptions[0], coulombsConstantOptions[1], coulombsConstantOptions[2], coulombsConstantOptions[3]);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)textForStrings);
	
		glColor3f(0.0f, 0.0f, 0.0f);
		glRasterPos3f(10, 210, 0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)"Change Coulombs range of affect using:");
		glRasterPos3f(10, 225, 0);
		sprintf(textForStrings, "B=%.2f N=%.2f M=%.2f ,=%.2f", coulombsRoAOptions[0], coulombsRoAOptions[1], coulombsRoAOptions[2], coulombsRoAOptions[3]);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)textForStrings);

		glColor3f(0.0f, 0.0f,1.0f);
		glRasterPos3f(10, 240, 0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)"Change Time Step using:");
		glRasterPos3f(10, 255, 0);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)"+=Increase time passed for step, -=Decrease time passed for step");
	}
	if (displayStats)
	{
		glColor3f(0.0f, 0.0f, 0.0f);
		glRasterPos3f(10, statOffset+0, 0);
		sprintf(textForStrings, "System Kinetic Energy:%f", systemKineticEnergy);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)textForStrings);

		glColor3f(0.0f, 0.0f, 1.0f);
		glRasterPos3f(10, statOffset+15, 0);
		sprintf(textForStrings, "Arc Length Constant:%.2fx", arcLengthFactor);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)textForStrings);

		glColor3f(0.0f, 0.0f, 0.0f);
		glRasterPos3f(10, statOffset+30, 0);
		sprintf(textForStrings, "Damping Value:%.2f", dampingValue);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)textForStrings);

		glColor3f(0.0f, 0.0f, 1.0f);
		glRasterPos3f(10, statOffset+45, 0);
		sprintf(textForStrings, "Coulombs Constant:%.2fx", coulombsConstant);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)textForStrings);

		glColor3f(0.0f, 0.0f, 0.0f);
		glRasterPos3f(10, statOffset+60, 0);
		sprintf(textForStrings, "Coulombs range of affect:%.2f", coulombsRoA);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)textForStrings);

		glColor3f(0.0f, 0.0f, 1.0f);
		glRasterPos3f(10, statOffset + 75, 0);
		sprintf(textForStrings, "Time Step:%f",timeInterval);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)textForStrings);
	}
	glEnable(GL_LIGHTING);
}

// draw the scene. Called once per frame and should only deal with scene drawing (not updating the simulator)
void display() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); // clear the rendering buffers

	glLoadIdentity(); // clear the current transformation state
	glMultMatrixf(camObjMat(g_Camera)); // apply the current camera transform

	// draw the grid if the control flag for it is true	
	if (controlActive(g_Control, csg_uiControlDrawGrid)) glCallList(gs_uiGridDisplayList);

	glPushAttrib(GL_ALL_ATTRIB_BITS); // push attribute state to enable constrained state changes
	visitNodes(&g_System, nodeDisplay); // loop through all of the nodes and draw them with the nodeDisplay function
	visitArcs(&g_System, arcDisplay); // loop through all of the arcs and draw them with the arcDisplay function
	glPopAttrib();
	glPopMatrix();
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, 512, 384, 0);

	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glLoadIdentity();

	addText();

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glFlush(); // ensure all the ogl instructions have been processed
	glutSwapBuffers(); // present the rendered scene to the screen
}

// processing of system and camera data outside of the renderng loop
void idle() 
{
	if (isSimulating)
	{
		//Loops through stages per iteration
		systemKineticEnergy = 0;
		visitNodes(&g_System, resetResultantForce);
		visitArcs(&g_System, calcFromArcs);
		visitNodes(&g_System, calculateRepulsion);
		visitNodes(&g_System, calcFromNodes);
		currentPNodePos = 1;
		currentSNodePos = 1;
	}
	else if (returnToDefault || isContinentLayout || isWorldSystemLayout) {
		if (frameCount < 100) //If nodes are still moving to static positions
		{
			frameCount++; //Increment frame count
			visitNodes(&g_System, updatePos); //Move nodes
		}
		else
		{
			//Reset variables, since movement is complete
			frameCount = 0;
			returnToDefault = false;
			isContinentLayout = false;
			isWorldSystemLayout = false;

		}
	}

	calculateFrameRate();
	controlChangeResetAll(g_Control); // re-set the update status for all of the control flags
	camProcessInput(g_Input, g_Camera); // update the camera pos/ori based on changes since last render
	camResetViewportChanged(g_Camera); // re-set the camera's viwport changed flag after all events have been processed
	glutPostRedisplay();// ask glut to update the screen
}

// respond to a change in window position or shape
void reshape(int iWidth, int iHeight)  
{
	glViewport(0, 0, iWidth, iHeight);  // re-size the rendering context to match window
	camSetViewport(g_Camera, 0, 0, iWidth, iHeight); // inform the camera of the new rendering context size
	glMatrixMode(GL_PROJECTION); // switch to the projection matrix stack 
	glLoadIdentity(); // clear the current projection matrix state
	gluPerspective(csg_fCameraViewAngle, ((float)iWidth)/((float)iHeight), csg_fNearClip, csg_fFarClip); // apply new state based on re-sized window
	glMatrixMode(GL_MODELVIEW); // swap back to the model view matrix stac
	glGetFloatv(GL_PROJECTION_MATRIX, g_Camera.m_afProjMat); // get the current projection matrix and sort in the camera model
	glutPostRedisplay(); // ask glut to update the screen
}

// detect key presses and assign them to actions
void keyboard(unsigned char c, int iXPos, int iYPos)
{
	switch(c)
	{
	case 'w':
		camInputTravel(g_Input, tri_pos); // mouse zoom
		break;
	case 's':
		camInputTravel(g_Input, tri_neg); // mouse zoom
		break;
	case 'p':
		camPrint(g_Camera); // print the camera data to the comsole
		break;
	case 'g':
		controlToggle(g_Control, csg_uiControlDrawGrid); // toggle the drawing of the grid
		break;
	case 'q':
		if (isSimulating)
		{
			isSimulating = false;
		}
		else {
			isSimulating = true;
			returnToDefault = false;
			isContinentLayout = false;
			isWorldSystemLayout = false;
			frameCount = 0;
		}
		break;
	case 't':
		changeArcLength(0);
		break;
	case 'y':
		changeArcLength(1);
		break;
	case 'u':
		changeArcLength(2);
		break;
	case 'i':
		changeArcLength(3);
		break;
	case 'h':
		changeDampingValue(0);
		break;
	case 'j':
		changeDampingValue(1);
		break;
	case 'k':
		changeDampingValue(2);
		break;
	case 'l':
		changeDampingValue(3);
		break;
	case 'z':
		changeCoulombsConstant(0);
		break;
	case 'x':
		changeCoulombsConstant(1);
		break;
	case 'c':
		changeCoulombsConstant(2);
		break;
	case 'v':
		changeCoulombsConstant(3);
		break;
	case 'b':
		changeCoulomsRoA(0);
		break;
	case 'n':
		changeCoulomsRoA(1);
		break;
	case 'm':
		changeCoulomsRoA(2);
		break;
	case ',':
		changeCoulomsRoA(3);
		break;
	case '-':
		changeTimeInterval(0);
		break;
	case '=':
		changeTimeInterval(1);
		break;
	case '1':
		processMenuEvents(TOGGLE_HELP);
		break;
	case '2':
		processMenuEvents(TOGGLE_STATS);
		break;
	default:
		break;
	}
}

// detect standard key releases
void keyboardUp(unsigned char c, int iXPos, int iYPos) 
{
	switch(c)
	{
		// end the camera zoom action
		case 'w': 
		case 's':
			camInputTravel(g_Input, tri_null);
			break;
	}
}

void sKeyboard(int iC, int iXPos, int iYPos)
{
	// detect the pressing of arrow keys for ouse zoom and record the state for processing by the camera
	switch(iC)
	{
		case GLUT_KEY_UP:
			camInputTravel(g_Input, tri_pos);
			break;
		case GLUT_KEY_DOWN:
			camInputTravel(g_Input, tri_neg);
			break;
	}
}

void sKeyboardUp(int iC, int iXPos, int iYPos)
{
	// detect when mouse zoom action (arrow keys) has ended
	switch(iC)
	{
		case GLUT_KEY_UP:
		case GLUT_KEY_DOWN:
			camInputTravel(g_Input, tri_null);
			break;
	}
}

void mouse(int iKey, int iEvent, int iXPos, int iYPos)
{
	// capture the mouse events for the camera motion and record in the current mouse input state
	if (iKey == GLUT_LEFT_BUTTON)
	{
		camInputMouse(g_Input, (iEvent == GLUT_DOWN) ? true : false);
		if (iEvent == GLUT_DOWN)camInputSetMouseStart(g_Input, iXPos, iYPos);
	}
	else if (iKey == GLUT_MIDDLE_BUTTON)
	{
		camInputMousePan(g_Input, (iEvent == GLUT_DOWN) ? true : false);
		if (iEvent == GLUT_DOWN)camInputSetMouseStart(g_Input, iXPos, iYPos);
	}
}

void motion(int iXPos, int iYPos)
{
	// if mouse is in a mode that tracks motion pass this to the camera model
	if(g_Input.m_bMouse || g_Input.m_bMousePan) camInputSetMouseLast(g_Input, iXPos, iYPos);
}


void myInit()
{
	// setup my event control structure
	controlInit(g_Control);

	// initalise the maths library
	initMaths();

	// Camera setup
	camInit(g_Camera); // initalise the camera model
	camInputInit(g_Input); // initialise the persistant camera input data 
	camInputExplore(g_Input, true); // define the camera navigation mode

	// opengl setup - this is a basic default for all rendering in the render loop
	glClearColor(csg_afColourClear[0], csg_afColourClear[1], csg_afColourClear[2], csg_afColourClear[3]); // set the window background colour
	glEnable(GL_DEPTH_TEST); // enables occusion of rendered primatives in the window
	glEnable(GL_LIGHT0); // switch on the primary light
	glEnable(GL_LIGHTING); // enable lighting calculations to take place
	glEnable(GL_BLEND); // allows transparency and fine lines to be drawn
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // defines a basic transparency blending mode
	glEnable(GL_NORMALIZE); // normalises the normal vectors used for lighting - you may be able to switch this iff (performance gain) is you normalise all normals your self
	glEnable(GL_CULL_FACE); // switch on culling of unseen faces
	glCullFace(GL_BACK); // set culling to not draw the backfaces of primatives

	// build the grid display list - display list are a performance optimization 
	buildGrid();

	// initialise the data system and load the data file
	initSystem(&g_System);
	parse(g_acFile, parseSection, parseNetwork, parseArc, parsePartition, parseVector);

	//Code that needs to be ran once at initialization 
	visitArcs(&g_System, getArcLengthBounds);
	visitArcs(&g_System, calcArcIdealLength);
	visitNodes(&g_System, setDefaultNodePos);
	menuCreator();

}

int main(int argc, char* argv[])
{  
	// check parameters to pull out the path and file name for the data file
	for (int i = 0; i<argc; i++) if (!strcmp(argv[i], csg_acFileParam)) sprintf_s(g_acFile, "%s", argv[++i]);


	if (strlen(g_acFile)) 
	{ 
		// if there is a data file

		glutInit(&argc, (char**)argv); // start glut (opengl window and rendering manager)

		glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA); // define buffers to use in ogl
		glutInitWindowPosition(csg_uiWindowDefinition[csg_uiX], csg_uiWindowDefinition[csg_uiY]);  // set rendering window position
		glutInitWindowSize(csg_uiWindowDefinition[csg_uiWidth], csg_uiWindowDefinition[csg_uiHeight]); // set rendering window size
		glutCreateWindow("raaAssignment1-2022");  // create rendering window and give it a name

		buildFont(); // setup text rendering (use outline print function to render 3D text

		myInit(); // application specific initialisation

		// provide glut with callback functions to enact tasks within the event loop
		glutDisplayFunc(display);
		glutIdleFunc(idle);
		glutReshapeFunc(reshape);
		glutKeyboardFunc(keyboard);
		glutKeyboardUpFunc(keyboardUp);
		glutSpecialFunc(sKeyboard);
		glutSpecialUpFunc(sKeyboardUp);
		glutMouseFunc(mouse);
		glutMotionFunc(motion);
		glutMainLoop(); // start the rendering loop running, this will only ext when the rendering window is closed 

		killFont(); // cleanup the text rendering process

		return 0; // return a null error code to show everything worked
	}
	else
	{
		// if there isn't a data file 

		printf("The data file cannot be found, press any key to exit...\n");
		_getch();
		return 1; // error code
	}
}

void buildGrid()
{
	if (!gs_uiGridDisplayList) gs_uiGridDisplayList= glGenLists(1); // create a display list

	glNewList(gs_uiGridDisplayList, GL_COMPILE); // start recording display list

	glPushAttrib(GL_ALL_ATTRIB_BITS); // push attrib marker
	glDisable(GL_LIGHTING); // switch of lighting to render lines

	glColor4fv(csg_afDisplayListGridColour); // set line colour

	// draw the grid lines
	glBegin(GL_LINES);
	for (int i = (int)csg_fDisplayListGridMin; i <= (int)csg_fDisplayListGridMax; i++)
	{
		glVertex3f(((float)i)*csg_fDisplayListGridSpace, 0.0f, csg_fDisplayListGridMin*csg_fDisplayListGridSpace);
		glVertex3f(((float)i)*csg_fDisplayListGridSpace, 0.0f, csg_fDisplayListGridMax*csg_fDisplayListGridSpace);
		glVertex3f(csg_fDisplayListGridMin*csg_fDisplayListGridSpace, 0.0f, ((float)i)*csg_fDisplayListGridSpace);
		glVertex3f(csg_fDisplayListGridMax*csg_fDisplayListGridSpace, 0.0f, ((float)i)*csg_fDisplayListGridSpace);
	}
	glEnd(); // end line drawing

	glPopAttrib(); // pop attrib marker (undo switching off lighting)

	glEndList(); // finish recording the displaylist
}