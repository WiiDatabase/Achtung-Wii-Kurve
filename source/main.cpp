// include useful headers
#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <grrlib.h>
#include "constants.h"
#include "wpadSticks.h"
#include "math.h"
#include <unistd.h>
#include "timeControl.h"
#include <fat.h>
#include <mxml.h>
#include <fcntl.h>
#include <asndlib.h>
#include <mp3player.h>

// include sound
#include "bg_music_mp3.h"
#include "click_pcm.h"
#include "click_positive_pcm.h"
#include "click_negative_pcm.h"
#include "ouch1_pcm.h"
#include "ouch2_pcm.h"
#include "ouch3_pcm.h"
#include "ouch4_pcm.h"
#include "ouch5_pcm.h"
#include "tick_pcm.h"

// include object classes
#include "Kurve.h"

// include images
#include "gfx/main_font.h"
#include "gfx/player1_point.h"
#include "gfx/player2_point.h"
#include "gfx/player3_point.h"
#include "gfx/player4_point.h"

// Set default settings
static int numberOfAIs;
static int gameSpeed;
static int gameTurningRadius;
static int gameHoleSize;
static int gameHoleSeparation;
static int gameGoalFactor;
static int rumbleIntensity;
static bool boardersOn;
static bool backgroundMusicOn;
static bool soundEffectsOn;

// Remember pressed

// Classic controller / Nunchuk
static bool wiiLeftUpReady[4]={true,true,true,true};
static bool wiiLeftDownReady[4]={true,true,true,true};

static bool wiiRightUpReady[4]={true,true,true,true};
static bool wiiRightDownReady[4]={true,true,true,true};

// Gamecube controller
static bool gcUpReady[4]={true,true,true,true};
static bool gcDownReady[4]={true,true,true,true};

static bool gcCUpReady[4]={true,true,true,true};
static bool gcCDownReady[4]={true,true,true,true};

// Check whether any controller presses up
bool globalUpPressed()
{
	bool returnValue=false;
	for (int i=0;i<4;i++)
	{
		if((WPAD_ButtonsDown(i) & WPAD_BUTTON_UP)||(WPAD_ButtonsDown(i) & WPAD_BUTTON_RIGHT))
		{
			returnValue=true;
		}
		if (PAD_ButtonsDown(i) & PAD_BUTTON_UP)
		{
			returnValue=true;
		}
		if ((WPAD_StickY(i,false)>96)&&(wiiLeftUpReady[i]))
		{
			wiiLeftUpReady[i]=false;
			returnValue=true;
		}
		if ((WPAD_StickY(i,true)>96)&&(wiiRightUpReady[i]))
		{
			wiiRightUpReady[i]=false;
			returnValue=true;
		}
		if ((PAD_StickY(i)>16)&&(gcUpReady[i]))
		{
			gcUpReady[i]=false;
			returnValue=true;
		}
		if((PAD_SubStickY(i)>16)&&(gcCUpReady[i]))
		{
			gcCUpReady[i]=false;
			returnValue=true;
		}
	}
	return returnValue;
}

// Check whether any controller presses down
bool globalDownPressed()
{
	bool returnValue=false;
	for (int i=0;i<4;i++)
	{
		if((WPAD_ButtonsDown(i) & WPAD_BUTTON_DOWN)||(WPAD_ButtonsDown(i) & WPAD_BUTTON_LEFT))
		{
			returnValue=true;
		}
		if (PAD_ButtonsDown(i) & PAD_BUTTON_DOWN)
		{
			returnValue=true;
		}
		if ((WPAD_StickY(i,false)<-96)&&(wiiLeftDownReady[i]))
		{
			wiiLeftDownReady[i]=false;
			returnValue=true;
		}
		if ((WPAD_StickY(i,true)<-96)&&(wiiRightDownReady[i]))
		{
			wiiRightDownReady[i]=false;
			returnValue=true;
		}
		if ((PAD_StickY(i)<-16)&&(gcDownReady[i]))
		{
			gcDownReady[i]=false;
			returnValue=true;
		}
		if((PAD_SubStickY(i)<-16)&&(gcCDownReady[i]))
		{
			gcCDownReady[i]=false;
			returnValue=true;
		}
	}
	return returnValue;
}

// Update
void updateReadiness()
{
	for (int i=0;i<4;i++)
	{
		// Left wii joystick up
		if (WPAD_StickY(i,false)<=96)
		{
			wiiLeftUpReady[i]=true;
		}
		// Left wii joystick down
		if (WPAD_StickY(i,false)>=-96)
		{
			wiiLeftDownReady[i]=true;
		}
		// Right wii joystick up
		if (WPAD_StickY(i,true)<=96)
		{
			wiiLeftUpReady[i]=true;
		}
		// Right wii joystick down
		if (WPAD_StickY(i,true)>=-96)
		{
			wiiLeftDownReady[i]=true;
		}
		// Gamecube joystick up
		if (PAD_StickY(i)<=16)
		{
			gcUpReady[i]=true;
		}
		// Gamecube joystick down
		if (PAD_StickY(i)>=-16)
		{
			gcDownReady[i]=true;
		}
		// C gamecube joystick up
		if (PAD_SubStickY(i)<=16)
		{
			gcCUpReady[i]=true;
		}
		// C gamecube joystick down
		if (PAD_SubStickY(i)>=-16)
		{
			gcCDownReady[i]=true;
		}
	}
}

// Check whether any controller presses a specific button
bool globalButtonPressed(u32 wiimoteButton1, u32 wiimoteButton2, u32 nunchukButton, u32 classicButton, u16 gamecubeButton)
{
	bool returnValue=false;
	for (int i=0;i<4;i++)
	{
		WPADData *data = WPAD_Data(i);
		if (data->exp.type==WPAD_EXP_NUNCHUK)
		{
			if (WPAD_ButtonsDown(i) & nunchukButton)
			{
				returnValue=true;
			}
		}
		if (data->exp.type==WPAD_EXP_CLASSIC)
		{
			if (WPAD_ButtonsDown(i) & nunchukButton)
			{
				returnValue=true;
			}
		}
		if ((WPAD_ButtonsDown(i) & wiimoteButton1)||(WPAD_ButtonsDown(i) & wiimoteButton2))
		{
			returnValue=true;
		}
		if (PAD_ButtonsDown(i) & gamecubeButton)
		{
			returnValue=true;
		}
	}
	return returnValue;
}

// Play a pcm sound
void playPcm(const u8* sound, int length, int volume)
{
	if (soundEffectsOn==true)
	{
		int vol = volume;
		int voice = SND_GetFirstUnusedVoice();
		if(voice >= 1)
		{
			SND_SetVoice(voice, VOICE_STEREO_16BIT, 48000, 0,(u8 *)sound, length, vol, vol, NULL);
		}
	}
}

// Calculate which orientation of the direction is closest to a specific angle
int closestAngle(float curDir, float toDir)
{
	float leftDist=0;
	float rightDist=0;
	if (toDir>curDir)
	{
		leftDist=toDir-curDir;
		rightDist=curDir+360-toDir;
	}
	if (curDir>toDir)
	{
		leftDist=toDir+360-curDir;
		rightDist=curDir-toDir;
	}
	if (rightDist>leftDist)
	{
		return 1;
	}
	if (leftDist>rightDist)
	{
		return -1;
	}
	return 0;
}

// Load the preferences
void loadPreferences()
{
	mxml_node_t *tree;
	mxml_node_t *data;
	FILE *fp = fopen("sd:/apps/Achtung_Wii_Kurve/settings.xml", "rb");
	if (fp == NULL) 
	{
		fclose(fp);
	}
	else {
		fseek(fp , 0, SEEK_END);
		long settings_size = ftell(fp);
		rewind(fp);
		
		if (settings_size > 0)
		{
			tree = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);
			fclose(fp);
			
			data = mxmlFindElement(tree, tree, "settings", NULL, NULL, MXML_DESCEND);
			
			if (mxmlElementGetAttr(data,"numberOfAIs")) {
				numberOfAIs = atoi(mxmlElementGetAttr(data,"numberOfAIs"));
			}
			if (mxmlElementGetAttr(data,"gameSpeed")) {
				gameSpeed = atoi(mxmlElementGetAttr(data,"gameSpeed"));
			}
			if (mxmlElementGetAttr(data,"gameTurningRadius")) {
				gameTurningRadius = atoi(mxmlElementGetAttr(data,"gameTurningRadius"));
			}
			if (mxmlElementGetAttr(data,"gameHoleSize")) {
				gameHoleSize = atoi(mxmlElementGetAttr(data,"gameHoleSize"));
			}
			if (mxmlElementGetAttr(data,"gameHoleSeparation")) {
				gameHoleSeparation = atoi(mxmlElementGetAttr(data,"gameHoleSeparation"));
			}
			if (mxmlElementGetAttr(data,"gameGoalFactor")) {
				gameGoalFactor = atoi(mxmlElementGetAttr(data,"gameGoalFactor"));
			}
			if (mxmlElementGetAttr(data,"boardersOn")) {
				boardersOn = atoi(mxmlElementGetAttr(data,"boardersOn"));
			}
			if (mxmlElementGetAttr(data,"rumbleIntensity")) {
				rumbleIntensity = atoi(mxmlElementGetAttr(data,"rumbleIntensity"));
			}
			if (mxmlElementGetAttr(data,"backgroundMusicOn")) {
				backgroundMusicOn = atoi(mxmlElementGetAttr(data,"backgroundMusicOn"));
			}
			if (mxmlElementGetAttr(data,"soundEffectsOn")) {
				soundEffectsOn = atoi(mxmlElementGetAttr(data,"soundEffectsOn"));
			}
			
			mxmlDelete(data);
			mxmlDelete(tree);
		}
		else {
			fclose(fp);
			unlink("sd:/apps/Achtung_Wii_Kurve/settings.xml");
		}
	}
}

// Save the preferences
void savePreferences() {
	mxml_node_t *xml;
	mxml_node_t *data; 
	xml = mxmlNewXML("1.0");
	
	data = mxmlNewElement(xml, "settings");
	
	char set1[1];
	sprintf(set1, "%i", numberOfAIs);
	mxmlElementSetAttr(data, "numberOfAIs", set1);
	char set2[1];
	sprintf(set2, "%i", gameSpeed);
	mxmlElementSetAttr(data, "gameSpeed", set2);
	char set3[1];
	sprintf(set3, "%i", gameTurningRadius);
	mxmlElementSetAttr(data, "gameTurningRadius", set3);
	char set4[1];
	sprintf(set4, "%i", gameHoleSize);
	mxmlElementSetAttr(data, "gameHoleSize", set4);
	char set5[1];
	sprintf(set5, "%i", gameHoleSeparation);
	mxmlElementSetAttr(data, "gameHoleSeparation", set5);
	char set6[1];
	sprintf(set6, "%i", gameGoalFactor);
	mxmlElementSetAttr(data, "gameGoalFactor", set6);
	char set7[1];
	sprintf(set7, "%i", boardersOn);
	mxmlElementSetAttr(data, "boardersOn", set7);
	char set8[1];
	sprintf(set8, "%i", rumbleIntensity);
	mxmlElementSetAttr(data, "rumbleIntensity", set8);
	char set9[1];
	sprintf(set9, "%i", backgroundMusicOn);
	mxmlElementSetAttr(data, "backgroundMusicOn", set9);
	char set10[1];
	sprintf(set10, "%i", soundEffectsOn);
	mxmlElementSetAttr(data, "soundEffectsOn", set10);
	
	FILE *f;
	f = fopen("sd:/apps/Achtung_Wii_Kurve/settings.xml", "wb");
	
	if (f == NULL) {
		fclose(f);
	}
	else {
		mxmlSaveFile(xml, f, MXML_NO_CALLBACK);
		fclose(f);
		mxmlDelete(data);
		mxmlDelete(xml);
	}
}

void restoreDefaults()
{
	numberOfAIs=0;
	gameSpeed=5;
	gameTurningRadius=16;
	gameHoleSize=8;
	gameHoleSeparation=200;
	gameGoalFactor=10;
	boardersOn=true;
	rumbleIntensity=false;
	backgroundMusicOn=true;
	soundEffectsOn=true;
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	
	// Initiate pads
	WPAD_Init();
	PAD_Init();
	
	WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(WPAD_CHAN_ALL,640, 480);
	
	// wiimote orientation structure
	struct orient_t orient[4];
	
	// wiimote ir structure
	struct ir_t ir[4];
	
	// expansion structure
	struct expansion_t exp[4];

	// Initiate GRRLIB
	GRRLIB_Init();
	
	// Load images
	
	// The font
	GRRLIB_texImg* font = GRRLIB_LoadTexture(main_font);
	GRRLIB_InitTileSet(font, 32, 36, 32);
	
	// Cursors
	GRRLIB_texImg* img_cursor[4];
	img_cursor[0] = GRRLIB_LoadTexture(player1_point);
	img_cursor[1] = GRRLIB_LoadTexture(player2_point);
	img_cursor[2] = GRRLIB_LoadTexture(player3_point);
	img_cursor[3] = GRRLIB_LoadTexture(player4_point);
	
	// Initiate audio
	SND_Init(INIT_RATE_48000);
	SND_Pause(0);
	MP3Player_Init();
	
	// Initiate FAT
	fatInitDefault();
	
	// Set default settings
	restoreDefaults();
	
	// Load Settings
	loadPreferences();
	
	// Global Variables
	State state = STATE_MENU;
	int selectMenu = 0;
	int selectPreferences = 0;
	int score[24];
	bool connected[24];
	for (int i=0;i<24;i++)
	{
		connected[i]=false;
		score[i]=0;
	}
	bool gameStarted=false;
	u8 gameGrid[GRID_WIDTH][GRID_HEIGHT];
	int round = 0;
	int suddenDeath = 0;
	int waitTime=0;
	int rumbleTime[8] = {0,0,0,0,0,0,0,0};
	Kurve* kurve[24];
	for(int i=0;i<24;i++)
	{
		kurve[i]=0;
	}
	u32 last = GFX_GetTime();
	unsigned int fps = 0;
	int backgroundMusicVolume = MIN_MUSIC_VOLUME;
	int musicTimer=0;
	Control controlWiimote[4];
	Control controlNunchuk[4];
	Control controlGamecube[4];
	for (int i=0;i<4;i++)
	{
		controlWiimote[i]=controlNunchuk[i]=controlGamecube[i]=CONTROL_STICK_CLASSIC;
	}
	int irArrayX[4][10];
	int irArrayY[4][10];
	int irArrayCounter=0;
	bool balanceBoardConnected=false;
	int balanceBoardChannel=0;
	int speedCounter=0;
	int lastSelectMenu=0;
	int lastSelectPreferences=0;
	bool customMusicEnabled=false;
	bool gamePaused=false;
	
	// Initialize randomness
	srand(last);
	
	// AI
	int turnTimer[12];
	AI_TurnType turnType[12];
	for (int i=0;i<12;i++)
	{
		turnTimer[i]=0;
		turnType[i]=AI_TURN_LEFT;
	}
	
	// Detect custom music file and put it into buffer
	FILE* musicFile = 0;
	long musicFileSize;
	char* musicBuffer=NULL;
	size_t musicBufferSize=NULL;
	musicFile=fopen("sd:/apps/Achtung_Wii_Kurve/custom_music.mp3", "rb");
	if (musicFile!=NULL)
	{
		customMusicEnabled=true;
		fseek(musicFile, 0 , SEEK_END);
		musicFileSize = ftell(musicFile);
		rewind(musicFile);
		musicBuffer=(char*) malloc(sizeof(char)*musicFileSize);
		musicBufferSize=fread(musicBuffer,1,musicFileSize,musicFile);
		fclose(musicFile);
	}

	// Main loop
	while(1) {
		// Play background music
		if(!MP3Player_IsPlaying())
		{
			if (customMusicEnabled)
			{
				MP3Player_PlayBuffer(musicBuffer,musicBufferSize+48000,NULL);
			}
			else
			{
				MP3Player_PlayBuffer(bg_music_mp3,bg_music_mp3_size+48000,NULL);
			}
		}
		
		// Scan pads
		WPAD_ScanPads();
		PAD_ScanPads();
		updateReadiness();
		for (int i=0;i<4;i++)
		{
			WPAD_Orientation(i, &orient[i]);
			WPAD_IR(i, &ir[i]);
			WPAD_Expansion(i, &exp[i]);
		}
		
		// Exit
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
		{
			// Free memory
			for (int i=0;i<4;i++)
			{
				free(img_cursor[i]);
			}
			free(font);
			if (customMusicEnabled)
			{
				free(musicBuffer);
			}
			GRRLIB_Exit();
			
			// Exit
			exit(0);
		}
		
		// state = menu
		if (state==STATE_MENU)
		{
			// Move Select
			if (globalDownPressed())
			{
				selectMenu++;
				if (selectMenu>MAX_SELECT_MENU)
				{
					selectMenu=0;
				}
			}
			
			if (globalUpPressed())
			{
				selectMenu--;
				if (selectMenu<0)
				{
					selectMenu=MAX_SELECT_MENU;
				}
			}
			
			// Set select by ir
			for (int i=3;i>=0;i--)
			{
				if ((ir[i].y>100)&&(ir[i].y<100+20*(MAX_SELECT_MENU+1))&&(ir[i].x>50)&&(ir[i].x<590))
				{
					selectMenu=(ir[i].y-100)/20;
				}
			}
			
			// Select
			
			// Start
			if ((globalButtonPressed(WPAD_BUTTON_2,WPAD_BUTTON_A,WPAD_NUNCHUK_BUTTON_C,WPAD_CLASSIC_BUTTON_A,PAD_BUTTON_A)&&(selectMenu==SELECT_MENU_START))
			||globalButtonPressed(WPAD_BUTTON_PLUS,0,0,WPAD_CLASSIC_BUTTON_PLUS,PAD_BUTTON_START))
			{
				playPcm(click_positive_pcm,click_positive_pcm_size,255);
				state = STATE_PLAYERS;
				continue;
			}
			
			// Preferences
			if (globalButtonPressed(WPAD_BUTTON_2,WPAD_BUTTON_A,WPAD_NUNCHUK_BUTTON_C,WPAD_CLASSIC_BUTTON_A,PAD_BUTTON_A)&&(selectMenu==SELECT_MENU_PREFERENCES))
			{
				playPcm(click_positive_pcm,click_positive_pcm_size,255);
				state=STATE_PREFERENCES;
				continue;
			}
			
			// Credits
			if (globalButtonPressed(WPAD_BUTTON_2,WPAD_BUTTON_A,WPAD_NUNCHUK_BUTTON_C,WPAD_CLASSIC_BUTTON_A,PAD_BUTTON_A)&&(selectMenu==SELECT_MENU_CREDITS))
			{
				playPcm(click_positive_pcm,click_positive_pcm_size,255);
				state=STATE_CREDITS;
				continue;
			}
			
			// Quit
			if (globalButtonPressed(WPAD_BUTTON_2,WPAD_BUTTON_A,WPAD_NUNCHUK_BUTTON_C,WPAD_CLASSIC_BUTTON_A,PAD_BUTTON_A)&&(selectMenu==SELECT_MENU_QUIT))
			{
				// Free memory
				for (int i=0;i<4;i++)
				{
					free(img_cursor[i]);
				}
				free(font);
				if (customMusicEnabled)
				{
					free(musicBuffer);
				}
				GRRLIB_Exit();
				
				// Exit
				exit(0);
			}
			
			// Play tick sound
			if (lastSelectMenu!=selectMenu)
			{
				playPcm(tick_pcm,tick_pcm_size,255);
				lastSelectMenu=selectMenu;
			}
		}
		
		// state = players
		if (state==STATE_PLAYERS)
		{
			// Back to menu
			if (globalButtonPressed(WPAD_BUTTON_MINUS,0,0,WPAD_CLASSIC_BUTTON_MINUS,PAD_TRIGGER_L))
			{
				playPcm(click_negative_pcm,click_negative_pcm_size,255);
				state=STATE_MENU;
			}
			
			// Start
			bool startable=false;
			int numberOfConnectedPlayers=0;
			for(int i=0;i<12;i++)
			{
				if (connected[i])
				{
					startable=true;
					numberOfConnectedPlayers++;
				}
			}
			if (balanceBoardConnected)
			{
				startable=true;
				numberOfConnectedPlayers++;
			}
			if (numberOfAIs>0)
			{
				startable=true;
			}
			if ((WPAD_ButtonsDown(0) & WPAD_BUTTON_PLUS)&&startable)
			{
				playPcm(click_positive_pcm,click_positive_pcm_size,255);
				state = STATE_GAME;
				gameStarted=false;
				for(int i=0;i<24;i++)
				{
					score[i]=0;
					suddenDeath=0;
				}
				
				// Connect AIs
				for (int i=0;i<12;i++)
				{
					if ((i<numberOfAIs)&&(i+numberOfConnectedPlayers<12))
					{
						connected[i+12]=true;
					} else
					{
						connected[i+12]=false;
					}
				}
				round = 1;
				continue;
			}
			
			// Connect/Disconnect/Change controls
			
			// Connect/Disconnect Balance Board
			bool balanceBoardCheck=false;
			for (int i=0;i<4;i++)
			{
			u32 devtype;
			WPAD_Probe(WPAD_BALANCE_BOARD,&devtype);
			
			if (devtype==WPAD_EXP_WIIBOARD)
			{
				balanceBoardChannel=i;
				balanceBoardCheck=true;
			}
			}
			if (balanceBoardCheck)
			{
				expansion_t balanceBoardExp;
				balanceBoardExp.type = WPAD_BALANCE_BOARD;
				WPAD_Expansion(WPAD_BALANCE_BOARD, &balanceBoardExp);
				float weight=balanceBoardExp.wb.tl+balanceBoardExp.wb.tr+balanceBoardExp.wb.bl+balanceBoardExp.wb.br;
				if (weight>1)
				{
					if (balanceBoardConnected==false)
					{
						playPcm(click_positive_pcm,click_positive_pcm_size,255);
					}
					balanceBoardConnected=true;
				} else
				{
					if (balanceBoardConnected==true)
					{
						playPcm(click_negative_pcm,click_negative_pcm_size,255);
					}
					balanceBoardConnected=false;
				}
			} else
			{
				if (balanceBoardConnected==true)
				{
					playPcm(click_negative_pcm,click_negative_pcm_size,255);
				}
				balanceBoardConnected=false;
			}
			for(int i=0;i<4;i++)
			{
				WPADData *data = WPAD_Data(i);
				
				// Connect Wiimote
				if ((WPAD_ButtonsDown(i) & WPAD_BUTTON_2)
				||(WPAD_ButtonsDown(i) & WPAD_BUTTON_A))
				{
					if (!connected[i])
					{
						playPcm(click_positive_pcm,click_positive_pcm_size,255);
						connected[i]=true;
					}
				}
				
				// Connect Nunchuk / Classic Controller
				if (((WPAD_ButtonsDown(i) & WPAD_CLASSIC_BUTTON_A)&&(data->exp.type==WPAD_EXP_CLASSIC))
				||((WPAD_ButtonsDown(i) & WPAD_NUNCHUK_BUTTON_C)&&(data->exp.type==WPAD_EXP_NUNCHUK)))
				{
					if (!connected[i+4])
					{
						playPcm(click_positive_pcm,click_positive_pcm_size,255);
						connected[i+4]=true;
					}
				}
				
				// Connect Gamecube Controller
				if (PAD_ButtonsDown(i) & PAD_BUTTON_A)
				{
					if (!connected[i+8])
					{
						playPcm(click_positive_pcm,click_positive_pcm_size,255);
						connected[i+8]=true;
					}
				}
				
				// Disconnect Wiimote
				if ((WPAD_ButtonsDown(i) & WPAD_BUTTON_1)
				||(WPAD_ButtonsDown(i) & WPAD_BUTTON_B))
				{
					if (connected[i])
					{
						playPcm(click_negative_pcm,click_negative_pcm_size,255);
						connected[i]=false;
					}
				}
				// Disconnect Nunchuk / Classic Controller
				if (((WPAD_ButtonsDown(i) & WPAD_CLASSIC_BUTTON_B)&&(data->exp.type==WPAD_EXP_CLASSIC))
				||((WPAD_ButtonsDown(i) & WPAD_NUNCHUK_BUTTON_Z)&&(data->exp.type==WPAD_EXP_NUNCHUK))
				||((data->exp.type!=WPAD_EXP_NUNCHUK)&&(data->exp.type!=WPAD_EXP_CLASSIC)))
				{
					if (connected[i+4])
					{
						playPcm(click_negative_pcm,click_negative_pcm_size,255);
						connected[i+4]=false;
					}
				}
				
				// Disconnect Gamecube Controller
				if (PAD_ButtonsDown(i) & PAD_BUTTON_B)
				{
					if (connected[i+8])
					{
						playPcm(click_negative_pcm,click_negative_pcm_size,255);
						connected[i+8]=false;
					}
				}
				
				// Change controls Wiimote
				if (connected[i])
				{
					if ((WPAD_ButtonsDown(i) & WPAD_BUTTON_UP)
					&&(controlWiimote[i]!=CONTROL_TILT))
					{
						playPcm(click_pcm,click_pcm_size,255);
						controlWiimote[i]=CONTROL_TILT;
					}
					if ((WPAD_ButtonsDown(i) & WPAD_BUTTON_DOWN)
					&&(controlWiimote[i]!=CONTROL_POINT))
					{
						playPcm(click_pcm,click_pcm_size,255);
						controlWiimote[i]=CONTROL_POINT;
					}
					if ((WPAD_ButtonsDown(i) & WPAD_BUTTON_RIGHT)
					&&(controlWiimote[i]!=CONTROL_STICK_CLASSIC))
					{
						playPcm(click_pcm,click_pcm_size,255);
						controlWiimote[i]=CONTROL_STICK_CLASSIC;
					}
				} 
				
				// Change controls Nunchuk/Classic Controller
				if (connected[i+4])
				{
					if ((data->exp.type==WPAD_EXP_NUNCHUK)
					&&((WPAD_StickX(i,false)<-96))
					&&(controlNunchuk[i]!=CONTROL_TILT))
					{
						playPcm(click_pcm,click_pcm_size,255);
						controlNunchuk[i]=CONTROL_TILT;
					} else
					if (((WPAD_StickX(i,false)>96)||(WPAD_StickX(i,true)>96)
					||((WPAD_ButtonsDown(i) & WPAD_CLASSIC_BUTTON_RIGHT)&&(data->exp.type==WPAD_EXP_CLASSIC)))
					&&(controlNunchuk[i]!=CONTROL_STICK_SPECIAL))
					{
						playPcm(click_pcm,click_pcm_size,255);
						controlNunchuk[i]=CONTROL_STICK_SPECIAL;
					} else
					if (((WPAD_StickY(i,false)>96)||(WPAD_StickY(i,true)>96)
					||((WPAD_ButtonsDown(i) & WPAD_CLASSIC_BUTTON_UP)&&(data->exp.type==WPAD_EXP_CLASSIC)))
					&&(controlNunchuk[i]!=CONTROL_STICK_CLASSIC))
					{
						playPcm(click_pcm,click_pcm_size,255);
						controlNunchuk[i]=CONTROL_STICK_CLASSIC;
					}
				}
				
				// Change controls Gamecube Controller
				if (connected[i+8])
				{
					if (((PAD_StickX(i)>16)
					||(PAD_SubStickX(i)>16)
					||(PAD_ButtonsDown(i) & PAD_BUTTON_RIGHT))
					&&(controlGamecube[i]!=CONTROL_STICK_SPECIAL))
					{
						playPcm(click_pcm,click_pcm_size,255);
						controlGamecube[i]=CONTROL_STICK_SPECIAL;
					} else
					if (((PAD_StickY(i)>16)
					||(PAD_SubStickY(i)>16)
					||(PAD_ButtonsDown(i) & PAD_BUTTON_UP))
					&&(controlGamecube[i]!=CONTROL_STICK_CLASSIC))
					{
						playPcm(click_pcm,click_pcm_size,255);
						controlGamecube[i]=CONTROL_STICK_CLASSIC;
					}
				}
			}
		}
		
		// state = game
		if (state==STATE_GAME)
		{	
			// Pause / Unpause
			if (globalButtonPressed(WPAD_BUTTON_PLUS,0,0,WPAD_CLASSIC_BUTTON_PLUS,PAD_BUTTON_START))
			{
				gamePaused=!gamePaused;
				if (gamePaused)
				{
					playPcm(click_positive_pcm,click_positive_pcm_size,255);
				}
				else
				{
					playPcm(click_negative_pcm,click_negative_pcm_size,255);
				}
			}
			
			// If not paused, run the game
			if (!(gamePaused))
			{
				// Go back
				if (globalButtonPressed(WPAD_BUTTON_MINUS,0,0,WPAD_CLASSIC_BUTTON_MINUS,PAD_TRIGGER_L))
				{
					playPcm(click_negative_pcm,click_negative_pcm_size,255);
					waitTime=0;
					state=STATE_PLAYERS;
					continue;
				}
				
				// Initiate gamegrid and kurves
				if (gameStarted==false)
				{
					for(int i=0;i<GRID_WIDTH;i++)
					{
						for(int k=0;k<GRID_HEIGHT;k++)
						{
							gameGrid[i][k]=0;
						}
					}
					for(int i=0;i<24;i++)
					{
						delete kurve[i];
						kurve[i] = new Kurve(10+gameTurningRadius+rand()%(GRID_WIDTH-gameTurningRadius*2-20), 10+gameTurningRadius+rand()%(GRID_HEIGHT-gameTurningRadius*2-20),gameTurningRadius , gameHoleSize, gameHoleSeparation);
						if (!connected[i])
						{
							(*kurve[i]).dead=(*kurve[i]).deactivated=true;
						}
						if ((balanceBoardConnected==true)&&(i==balanceBoardChannel))
						{
							(*kurve[i]).dead=(*kurve[i]).deactivated=false;
						}
						if (i<12)
						{
							turnTimer[i]=0;
						}
						
						// In case of sudden death
						if ((suddenDeath != 0)&&(score[i] != suddenDeath))
						{
							(*kurve[i]).dead = true;
						}
					}
					gameStarted = true;
				}
				
				// Iterate game
				else
				{
					for(int u=0;u<gameSpeed;u++)
					{
					speedCounter++;
						if(speedCounter>3)
						{
							speedCounter=0;
							int m=0;
							for(int i=0;i<24;i++)
							{
								kurve[i]->iterate();
								if (((*kurve[i]).dead==false)&&((*kurve[i]).hole==false))
								{
									if (((*kurve[i]).x<GRID_WIDTH)
									&&((*kurve[i]).x>=0)
									&&((*kurve[i]).y<GRID_HEIGHT)
									&&((*kurve[i]).y>=0))
									{
										gameGrid[(*kurve[i]).x][(*kurve[i]).y]=m+1;
									}
								}
								if ((*kurve[i]).deactivated==false)
								{
									m++;
								}
								
								if (boardersOn==false)
								{
									if ((*kurve[i]).x<0)
									{
										kurve[i]->modifyX(GRID_WIDTH);
									}
									if ((*kurve[i]).x>GRID_WIDTH)
									{
										kurve[i]->modifyX(-GRID_WIDTH);
									}
									if ((*kurve[i]).y<0)
									{
										kurve[i]->modifyY(GRID_HEIGHT);
									}
									if ((*kurve[i]).y>GRID_HEIGHT)
									{
										kurve[i]->modifyY(-GRID_HEIGHT);
									}
								}
							}
							
							// Check collisions, rumble and add score
							for(int i=0;i<24;i++)
							{
								int r = rand();
								int tempX = (*kurve[(r+i)%24]).x+cos((*kurve[(r+i)%24]).direction*PI/180)*3;
								int tempY = (*kurve[(r+i)%24]).y+sin((*kurve[(r+i)%24]).direction*PI/180)*3;
								bool toCloseOther=false;
								for(int j=0;j<24;j++)
								{
									if (((r+i)%24!=j)&&((*kurve[j]).dead==false))
									{
										if(pow((*kurve[(r+i)%24]).x-(*kurve[j]).x,2)+pow((*kurve[(r+i)%24]).y-(*kurve[j]).y,2)<2)
										{
											toCloseOther=true;
										}
									}
								}
								
								if (
								((*kurve[(r+i)%24]).dead==false)
								&&(
								((tempX<0)&&(boardersOn==true))
								||((tempX>GRID_WIDTH)&&(boardersOn==true))
								||((tempY<0)&&(boardersOn==true))
								||((tempY>GRID_HEIGHT)&&(boardersOn==true))
								||(toCloseOther)
								
								||((gameGrid[tempX][tempY]!=0)&&(tempX>=0)&&(tempX<GRID_WIDTH)&&(tempY>=0)&&(tempY<GRID_HEIGHT))
								||((gameGrid[tempX+1][tempY]!=0)&&(tempX+1>=0)&&(tempX+1<GRID_WIDTH)&&(tempY>=0)&&(tempY<GRID_HEIGHT))
								||((gameGrid[tempX-1][tempY]!=0)&&(tempX-1>=0)&&(tempX-1<GRID_WIDTH)&&(tempY>=0)&&(tempY<GRID_HEIGHT))
								||((gameGrid[tempX][tempY+1]!=0)&&(tempX>=0)&&(tempX<GRID_WIDTH)&&(tempY+1>=0)&&(tempY+1<GRID_HEIGHT))
								||((gameGrid[tempX][tempY-1]!=0)&&(tempX>=0)&&(tempX<GRID_WIDTH)&&(tempY-1>=0)&&(tempY-1<GRID_HEIGHT))
								
								))
								{
									(*kurve[(r+i)%24]).dead=true;
									switch(rand()%5)
									{
										case 0:
											playPcm(ouch1_pcm,ouch1_pcm_size,255);
											break;
										case 1:
											playPcm(ouch2_pcm,ouch2_pcm_size,255);
											break;
										case 2:
											playPcm(ouch3_pcm,ouch3_pcm_size,255);
											break;
										case 3:
											playPcm(ouch4_pcm,ouch4_pcm_size,255);
											break;
										case 4:
											playPcm(ouch5_pcm,ouch5_pcm_size,255);
											break;
									}
									if (rumbleIntensity)
									{
										float setRumbleTime=rumbleIntensity*fps*0.3;
										if ((r+i)%24<4)
										{
											WPAD_Rumble((r+i)%24, true);
											rumbleTime[(r+i)%24]=setRumbleTime;
										}
										if (((r+i)%24>=8)&&((r+i)%24<12))
										{
											PAD_ControlMotor((r+i)%24-8, true);
											rumbleTime[(r+i)%24-4]=setRumbleTime;
										}
									}
									int k = 0;
									for(int j=0;j<24;j++)
									{
										if ((*kurve[j]).dead==false)
										{
											score[j]+=1;
											k++;
										}
									}
									
									// Check for winner
									int n=0;
									for (int j=0;j<24;j++)
									{
										if ((*kurve[j]).deactivated==false)
										{
											n++;
										}
									}
									int hasWon = 0;
									int winValue = (gameGoalFactor)*(n-1);
									for (int j=0;j<24;j++)
									{
										if (score[j] == winValue)
										{
											hasWon++;
										}
										if (score[j] > winValue)
										{
											hasWon = 1;
											winValue = score[j];
										}
									}
									
									if (k==1)
									{
										// One winner
										if (hasWon == 1)
										{
											waitTime = 0;
											state = STATE_WINNER;
											continue;
										}
										
										// Many winners
										if (hasWon >= 1)
										{
											suddenDeath = winValue;
											k=1;
										}
									}
									
									// Start new round
									if ((k==1)||(k==0))
									{
										waitTime=1;
									}
								}
							}
							
							// Start new round
							if (waitTime>0)
							{
								waitTime++;
								if (waitTime>60)
								{
									waitTime=0;
									round++;
									gameStarted=false;
								}
							}
							
							// Check input and control AIs
							for(int i=0;i<24;i++)
							{
								// Balance Board
								if (i<WPAD_MAX_WIIMOTES)
								{
									if ((balanceBoardConnected==true)&&(i==balanceBoardChannel))
									{
										expansion_t balanceBoardExp;
										balanceBoardExp.type=WPAD_BALANCE_BOARD;
										WPAD_Expansion(WPAD_BALANCE_BOARD, &balanceBoardExp);
										float x=balanceBoardExp.wb.x;
										// Sick bug from here... therefore there are questionable boundary values.
										if (x!=13) // seems like when noone is on the BB, x is set to 13
											{
											if (x<-20) // -20 would be -8 if it would have been unbuggy
											{
												kurve[balanceBoardChannel]->turnLeft();
											}
											if (x>30) // 30 would be 8 if it would have been unbuggy
											{
												kurve[balanceBoardChannel]->turnRight();
											}
										}
										// End of sick bug
									}
								}
								
								// Wiimote
								if ((i<4)&&((balanceBoardConnected==false)||(i!=balanceBoardChannel)))
								{
									// Classic Stick
									if (controlWiimote[i]==CONTROL_STICK_CLASSIC)
									{
										// Turn left
										if ((WPAD_ButtonsHeld(i) & WPAD_BUTTON_UP)||(WPAD_ButtonsHeld(i) & WPAD_BUTTON_LEFT))
										{
											kurve[i]->turnLeft();
										}
										
										// Turn Right
										if ((WPAD_ButtonsHeld(i) & WPAD_BUTTON_DOWN)||(WPAD_ButtonsHeld(i) & WPAD_BUTTON_RIGHT))
										{
											kurve[i]->turnRight();
										}
									}
									
									// Tilt
									if (controlWiimote[i]==CONTROL_TILT)
									{
										// Turn left
										if (orient[i].pitch>25)
										{
											kurve[i]->turnLeft();
										}
											
										// Turn Right
										if (orient[i].pitch<-25)
										{
											kurve[i]->turnRight();
										}
									}
									
									// Point
									if (controlWiimote[i]==CONTROL_POINT)
									{
										float toDir=atan2(((ir[i].y-100)/2)-(*kurve[i]).y,((ir[i].x-50)/2)-(*kurve[i]).x)*180/PI;
										if (toDir<0)
										{
											toDir+=360;
										}
										float curDir=(*kurve[i]).direction;
										switch(closestAngle(curDir,toDir))
										{
											// Turn Left
											case -1:
											kurve[i]->turnLeft();
											break;
											// Turn Right
											case 1:
											kurve[i]->turnRight();
											break;
											// Do not turn
											default:
											break;
										}
									}
								}
										
								// Nunchuk / Classic Controller
								if ((i<8)&&(i>=4))
								{
								
									// Classic Stick
									if (controlNunchuk[i-4]==CONTROL_STICK_CLASSIC)
									{
										// Turn left
										if ((WPAD_ButtonsHeld(i-4) & WPAD_CLASSIC_BUTTON_LEFT)||(WPAD_StickX(i-4,false)<-32)||(WPAD_StickX(i-4,true)<-32))
										{
											kurve[i]->turnLeft();
										}
											
										// Turn Right
										if ((WPAD_ButtonsHeld(i-4) & WPAD_CLASSIC_BUTTON_RIGHT)||(WPAD_StickX(i-4,false)>32)||(WPAD_StickX(i-4,true)>32))
										{
											kurve[i]->turnRight();
										}
									}
									
									// Special Stick
									if (controlNunchuk[i-4]==CONTROL_STICK_SPECIAL)
									{
										WPADData *data = WPAD_Data(i-4);
										bool turn=false;
										float curDir=(*kurve[i]).direction;
										float toDir=0;
										if ((data->exp.nunchuk.js.mag>0.5)&&(data->exp.type==WPAD_EXP_NUNCHUK))
										{
											toDir=data->exp.nunchuk.js.ang;
											turn=true;
										} else if ((data->exp.classic.ljs.mag>0.5)&&(data->exp.type==WPAD_EXP_CLASSIC))
										{
											toDir=data->exp.classic.ljs.ang;
											turn=true;
										} else if ((data->exp.classic.rjs.mag>0.5)&&(data->exp.type==WPAD_EXP_CLASSIC))
										{
											toDir=data->exp.classic.rjs.ang;
											turn=true;
										}
										toDir-=90;
										if (toDir<0)
										{
											toDir+=360;
										}
										if (turn)
										{
											switch(closestAngle(curDir,toDir))
											{
												// Turn Left
												case -1:
												kurve[i]->turnLeft();
												break;
												// Turn Right
												case 1:
												kurve[i]->turnRight();
												break;
												// Do not turn
												default:
												break;
											}
										}
									}
									
									// Tilt
									if (controlNunchuk[i-4]==CONTROL_TILT)
									{
										// Turn left
										if (exp[i-4].nunchuk.orient.roll<-35)
										{
											kurve[i]->turnLeft();
										}
											
										// Turn Right
										if (exp[i-4].nunchuk.orient.roll>5)
										{
											kurve[i]->turnRight();
										}
									}
								}
								
								// Gamecube Controller
								if ((i>=8)&&(i<12))
								{
									// Classic stick
									if (controlGamecube[i-8]==CONTROL_STICK_CLASSIC)
									{
										// Turn left
										if ((PAD_ButtonsHeld(i-8) & PAD_BUTTON_LEFT)||(PAD_StickX(i-8)<-4)||(PAD_SubStickX(i-8)<-4))
										{
											kurve[i]->turnLeft();
										}
										
										// Turn Right
										if ((PAD_ButtonsHeld(i-8) & PAD_BUTTON_RIGHT)||(PAD_StickX(i-8)>4)||(PAD_SubStickX(i-8)>4))
										{
											kurve[i]->turnRight();
										}
									}
									
									// Special stick
									if (controlGamecube[i-8]==CONTROL_STICK_SPECIAL)
									{	
										int diffX;
										int diffY;
										bool turn;
										int stickMagSquared=pow(PAD_StickX(i-8),2)+pow(PAD_StickY(i-8),2);
										int subStickMagSquared=pow(PAD_SubStickX(i-8),2)+pow(PAD_SubStickY(i-8),2);
										if ((stickMagSquared>4)||(subStickMagSquared>4))
										{
											if ((PAD_StickX(i-8)==0)&&(PAD_StickY(i-8)==0))
											{
												diffX=PAD_SubStickX(i-8);
												diffY=-PAD_SubStickY(i-8);
											} else
											{
												diffX=PAD_StickX(i-8);
												diffY=-PAD_StickY(i-8);
											}
											turn=true;
										} else
										{
											turn=false;
										}
										if (turn)
										{
											float toDir=atan2(diffY,diffX)*180/PI;
											if (toDir<0)
											{
												toDir+=360;
											}
											float curDir=(*kurve[i]).direction;
											switch(closestAngle(curDir,toDir))
											{
												// Turn Left
												case -1:
												kurve[i]->turnLeft();
												break;
												// Turn Right
												case 1:
												kurve[i]->turnRight();
												break;
												// Do not turn
												default:
												break;
											}
										}
									}
								}
								
								// AIs
								if ((i>=12)&&(connected[i]))
								{
									if (turnTimer[i-12]>0)
									{
										if (turnType[i-12]==AI_TURN_LEFT)
										{
											kurve[i]->turnLeft();
										} else if (turnType[i-12]==AI_TURN_RIGHT)
										{
											kurve[i]->turnRight();
										}
										turnTimer[i-12]--;
									} else
									{
										float checkDirection = (*kurve[i]).direction;
										int tempX;
										int tempY;
										bool hit;
										int count = 0;
										int checkLength=50;
										const int MAX_COUNT=42;
										while (true)
										{
											hit=false;
											for(int j=0;j<=checkLength;j++)
											{
												tempX=(*kurve[i]).x+cos(checkDirection*PI/180)*(j+3);
												tempY=(*kurve[i]).y+sin(checkDirection*PI/180)*(j+3);
												if (boardersOn==false)
												{
													if (tempX<0)
													{
														tempX+=GRID_WIDTH;
													}
													if (tempX>GRID_WIDTH)
													{
														tempX-=GRID_WIDTH;
													}
													if (tempY<0)
													{
														tempY+=GRID_HEIGHT;
													}
													if (tempY>GRID_HEIGHT)
													{
														tempY-=GRID_HEIGHT;
													}
												}
												if 
												(
												(tempX<0)
												||(tempX>GRID_WIDTH)
												||(tempY<0)
												||(tempY>GRID_HEIGHT)
												||(gameGrid[tempX][tempY]!=0)
												||(gameGrid[tempX-1][tempY]!=0)
												||(gameGrid[tempX-2][tempY]!=0)
												||(gameGrid[tempX+1][tempY]!=0)
												||(gameGrid[tempX+2][tempY]!=0)
												||(gameGrid[tempX][tempY-1]!=0)
												||(gameGrid[tempX][tempY-2]!=0)
												||(gameGrid[tempX][tempY+1]!=0)
												||(gameGrid[tempX][tempY+2]!=0)
												)
												{
													hit=true;
													break;
												}
											}
											if ((hit==true)&&(count<MAX_COUNT))
											{
												count++;
												checkDirection+=
												(count*5)
												*(
												-(count%2==0)
												+(count%2!=0)
												);
											} else
											{
												if (count==0)
												{
													turnType[i-12]=AI_DONT_TURN;
													turnTimer[i-12]=1;
													break;
												} else if (count==MAX_COUNT)
												{
													if (checkLength>10)
													{
														checkLength-=10;
													} else
													{
														checkLength-=2;
													}
													if (checkLength==0)
													{
														turnType[i-12]=AI_DONT_TURN;
														turnTimer[i-12]=1;
														break;
													} else
													{
														checkDirection = (*kurve[i]).direction;
														count=0;
													}
												}else if (count%2==0)
												{
													turnType[i-12]=AI_TURN_LEFT;
													turnTimer[i-12]=1;
													break;
												} else
												{
													turnType[i-12]=AI_TURN_RIGHT;
													turnTimer[i-12]=1;
													break;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		
		// state = winner
		if (state==STATE_WINNER)
		{
			// Proceed
			if ((globalButtonPressed(WPAD_BUTTON_2,WPAD_BUTTON_A,WPAD_NUNCHUK_BUTTON_C,WPAD_CLASSIC_BUTTON_A,PAD_BUTTON_A))
			||(globalButtonPressed(WPAD_BUTTON_PLUS,0,0,WPAD_CLASSIC_BUTTON_PLUS,PAD_BUTTON_START)))
			{
				playPcm(click_positive_pcm,click_positive_pcm_size,255);
				state = STATE_MENU;
				continue;
			}
		}
		
		// state = credits
		if (state==STATE_CREDITS)
		{
			// Go back
			if ((globalButtonPressed(WPAD_BUTTON_2,WPAD_BUTTON_A,WPAD_NUNCHUK_BUTTON_C,WPAD_CLASSIC_BUTTON_A,PAD_BUTTON_A))
			||(globalButtonPressed(WPAD_BUTTON_1,WPAD_BUTTON_B,WPAD_NUNCHUK_BUTTON_Z,WPAD_CLASSIC_BUTTON_B,PAD_BUTTON_B))
			||(globalButtonPressed(WPAD_BUTTON_PLUS,0,0,WPAD_CLASSIC_BUTTON_PLUS,PAD_BUTTON_START))
			||(globalButtonPressed(WPAD_BUTTON_MINUS,0,0,WPAD_CLASSIC_BUTTON_MINUS,PAD_TRIGGER_L)))
			{
				playPcm(click_negative_pcm,click_negative_pcm_size,255);
				state = STATE_MENU;
				continue;
			}
		}
		
		// state = preferences
		if(state==STATE_PREFERENCES)
		{
			
			// Move Select
			if (globalDownPressed())
			{
				selectPreferences++;
				if (selectPreferences>MAX_SELECT_PREFERENCES)
				{
					selectPreferences=0;
				}
			}
			
			if (globalUpPressed())
			{
				selectPreferences--;
				if (selectPreferences<0)
				{
					selectPreferences=MAX_SELECT_PREFERENCES;
				}
			}
			
			// Set select by ir
			for (int i=3;i>=0;i--)
			{
				if ((ir[i].y>100)&&(ir[i].y<100+20*(MAX_SELECT_PREFERENCES+1))&&(ir[i].x>50)&&(ir[i].x<590))
				{
					selectPreferences=(ir[i].y-100)/20;
				}
			}
			
			// Select / Adjust
			
			// Adjust number of AIs
			if ((globalButtonPressed(WPAD_BUTTON_2,WPAD_BUTTON_A,WPAD_NUNCHUK_BUTTON_C,WPAD_CLASSIC_BUTTON_A,PAD_BUTTON_A))&&(selectPreferences==SELECT_PREFERENCES_COMPS)&&(numberOfAIs<12))
			{
				playPcm(click_pcm,click_pcm_size,255);
				numberOfAIs++;
			}
			
			if ((globalButtonPressed(WPAD_BUTTON_1,WPAD_BUTTON_B,WPAD_NUNCHUK_BUTTON_Z,WPAD_CLASSIC_BUTTON_B,PAD_BUTTON_B))&&(selectPreferences==SELECT_PREFERENCES_COMPS)&&(numberOfAIs>0))
			{
				playPcm(click_pcm,click_pcm_size,255);
				numberOfAIs--;
			}
			
			// Adjust Speed
			if ((globalButtonPressed(WPAD_BUTTON_2,WPAD_BUTTON_A,WPAD_NUNCHUK_BUTTON_C,WPAD_CLASSIC_BUTTON_A,PAD_BUTTON_A))&&(selectPreferences==SELECT_PREFERENCES_SPEED)&&(gameSpeed<10))
			{
				playPcm(click_pcm,click_pcm_size,255);
				gameSpeed++;
			}
			
			if ((globalButtonPressed(WPAD_BUTTON_1,WPAD_BUTTON_B,WPAD_NUNCHUK_BUTTON_Z,WPAD_CLASSIC_BUTTON_B,PAD_BUTTON_B))&&(selectPreferences==SELECT_PREFERENCES_SPEED)&&(gameSpeed>1))
			{
				playPcm(click_pcm,click_pcm_size,255);
				gameSpeed--;
			}
			
			// Adjust Turning Radius
			if ((globalButtonPressed(WPAD_BUTTON_2,WPAD_BUTTON_A,WPAD_NUNCHUK_BUTTON_C,WPAD_CLASSIC_BUTTON_A,PAD_BUTTON_A))&&(selectPreferences==SELECT_PREFERENCES_TURNING_RADIUS)&&(gameTurningRadius<40))
			{
				playPcm(click_pcm,click_pcm_size,255);
				gameTurningRadius+=2;
			}
			
			if ((globalButtonPressed(WPAD_BUTTON_1,WPAD_BUTTON_B,WPAD_NUNCHUK_BUTTON_Z,WPAD_CLASSIC_BUTTON_B,PAD_BUTTON_B))&&(selectPreferences==SELECT_PREFERENCES_TURNING_RADIUS)&&(gameTurningRadius>4))
			{
				playPcm(click_pcm,click_pcm_size,255);
				gameTurningRadius-=2;
			}
			
			// Adjust Hole Size
			if ((globalButtonPressed(WPAD_BUTTON_2,WPAD_BUTTON_A,WPAD_NUNCHUK_BUTTON_C,WPAD_CLASSIC_BUTTON_A,PAD_BUTTON_A))&&(selectPreferences==SELECT_PREFERENCES_HOLE_SIZE)&&(gameHoleSize<25))
			{
				playPcm(click_pcm,click_pcm_size,255);
				gameHoleSize++;
			}
			
			if ((globalButtonPressed(WPAD_BUTTON_1,WPAD_BUTTON_B,WPAD_NUNCHUK_BUTTON_Z,WPAD_CLASSIC_BUTTON_B,PAD_BUTTON_B))&&(selectPreferences==SELECT_PREFERENCES_HOLE_SIZE)&&(gameHoleSize>4))
			{
				playPcm(click_pcm,click_pcm_size,255);
				gameHoleSize--;
			}
			
			// Adjust Hole Separation
			if ((globalButtonPressed(WPAD_BUTTON_2,WPAD_BUTTON_A,WPAD_NUNCHUK_BUTTON_C,WPAD_CLASSIC_BUTTON_A,PAD_BUTTON_A))&&(selectPreferences==SELECT_PREFERENCES_HOLE_SEPARATION)&&(gameHoleSeparation<750))
			{
				playPcm(click_pcm,click_pcm_size,255);
				gameHoleSeparation+=25;
			}
			
			if ((globalButtonPressed(WPAD_BUTTON_1,WPAD_BUTTON_B,WPAD_NUNCHUK_BUTTON_Z,WPAD_CLASSIC_BUTTON_B,PAD_BUTTON_B))&&(selectPreferences==SELECT_PREFERENCES_HOLE_SEPARATION)&&(gameHoleSeparation>25))
			{
				playPcm(click_pcm,click_pcm_size,255);
				gameHoleSeparation-=25;
			}
			
			// Adjust Goal Factor
			if ((globalButtonPressed(WPAD_BUTTON_2,WPAD_BUTTON_A,WPAD_NUNCHUK_BUTTON_C,WPAD_CLASSIC_BUTTON_A,PAD_BUTTON_A))&&(selectPreferences==SELECT_PREFERENCES_GOAL_FACTOR)&&(gameGoalFactor<30))
			{
				playPcm(click_pcm,click_pcm_size,255);
				gameGoalFactor+=1;
			}
			
			if ((globalButtonPressed(WPAD_BUTTON_1,WPAD_BUTTON_B,WPAD_NUNCHUK_BUTTON_Z,WPAD_CLASSIC_BUTTON_B,PAD_BUTTON_B))&&(selectPreferences==SELECT_PREFERENCES_GOAL_FACTOR)&&(gameGoalFactor>1))
			{
				playPcm(click_pcm,click_pcm_size,255);
				gameGoalFactor-=1;
			}
			
			// Toggle Boarders
			if (((globalButtonPressed(WPAD_BUTTON_2,WPAD_BUTTON_A,WPAD_NUNCHUK_BUTTON_C,WPAD_CLASSIC_BUTTON_A,PAD_BUTTON_A))
			||(globalButtonPressed(WPAD_BUTTON_1,WPAD_BUTTON_B,WPAD_NUNCHUK_BUTTON_Z,WPAD_CLASSIC_BUTTON_B,PAD_BUTTON_B)))
			&&(selectPreferences==SELECT_PREFERENCES_BOARDERS))
			{
				playPcm(click_pcm,click_pcm_size,255);
				boardersOn=1-boardersOn;
			}
			
			// Adjust Rumble
			if ((globalButtonPressed(WPAD_BUTTON_2,WPAD_BUTTON_A,WPAD_NUNCHUK_BUTTON_C,WPAD_CLASSIC_BUTTON_A,PAD_BUTTON_A))&&(selectPreferences==SELECT_PREFERENCES_RUMBLE)&&(rumbleIntensity<3))
			{
				playPcm(click_pcm,click_pcm_size,255);
				rumbleIntensity+=1;
			}
			
			if ((globalButtonPressed(WPAD_BUTTON_1,WPAD_BUTTON_B,WPAD_NUNCHUK_BUTTON_Z,WPAD_CLASSIC_BUTTON_B,PAD_BUTTON_B))&&(selectPreferences==SELECT_PREFERENCES_RUMBLE)&&(rumbleIntensity>0))
			{
				playPcm(click_pcm,click_pcm_size,255);
				rumbleIntensity-=1;
			}
			
			// Toggle Sound Effects
			if (((globalButtonPressed(WPAD_BUTTON_2,WPAD_BUTTON_A,WPAD_NUNCHUK_BUTTON_C,WPAD_CLASSIC_BUTTON_A,PAD_BUTTON_A))
			||(globalButtonPressed(WPAD_BUTTON_1,WPAD_BUTTON_B,WPAD_NUNCHUK_BUTTON_Z,WPAD_CLASSIC_BUTTON_B,PAD_BUTTON_B)))
			&&(selectPreferences==SELECT_PREFERENCES_SOUND_EFFECTS))
			{
				playPcm(click_pcm,click_pcm_size,255);
				soundEffectsOn=1-soundEffectsOn;
			}
			
			// Toggle Background Music
			if (((globalButtonPressed(WPAD_BUTTON_2,WPAD_BUTTON_A,WPAD_NUNCHUK_BUTTON_C,WPAD_CLASSIC_BUTTON_A,PAD_BUTTON_A))
			||(globalButtonPressed(WPAD_BUTTON_1,WPAD_BUTTON_B,WPAD_NUNCHUK_BUTTON_Z,WPAD_CLASSIC_BUTTON_B,PAD_BUTTON_B)))
			&&(selectPreferences==SELECT_PREFERENCES_BACKGROUND_MUSIC))
			{
				playPcm(click_pcm,click_pcm_size,255);
				backgroundMusicOn=1-backgroundMusicOn;
			}
			
			// Save Preferences
			if ((globalButtonPressed(WPAD_BUTTON_2,WPAD_BUTTON_A,WPAD_NUNCHUK_BUTTON_C,WPAD_CLASSIC_BUTTON_A,PAD_BUTTON_A))
			&&(selectPreferences==SELECT_PREFERENCES_SAVE_PREFERENCES))
			{
				playPcm(click_pcm,click_pcm_size,255);
				savePreferences();
			}
			
			// Load Preferences
			if ((globalButtonPressed(WPAD_BUTTON_2,WPAD_BUTTON_A,WPAD_NUNCHUK_BUTTON_C,WPAD_CLASSIC_BUTTON_A,PAD_BUTTON_A))
			&&(selectPreferences==SELECT_PREFERENCES_LOAD_PREFERENCES))
			{
				playPcm(click_pcm,click_pcm_size,255);
				loadPreferences();
			}
			
			// Restore Defaults
			if ((globalButtonPressed(WPAD_BUTTON_2,WPAD_BUTTON_A,WPAD_NUNCHUK_BUTTON_C,WPAD_CLASSIC_BUTTON_A,PAD_BUTTON_A))
			&&(selectPreferences==SELECT_PREFERENCES_RESTORE_DEFAULTS))
			{
				playPcm(click_pcm,click_pcm_size,255);
				restoreDefaults();
			}
			
			// Go Back
			if (((globalButtonPressed(WPAD_BUTTON_2,WPAD_BUTTON_A,WPAD_NUNCHUK_BUTTON_C,WPAD_CLASSIC_BUTTON_A,PAD_BUTTON_A))
			&&(selectPreferences==SELECT_PREFERENCES_BACK))
			||(globalButtonPressed(WPAD_BUTTON_MINUS,0,0,WPAD_CLASSIC_BUTTON_MINUS,PAD_TRIGGER_L)))
			{
				playPcm(click_negative_pcm,click_negative_pcm_size,255);
				state=STATE_MENU;
			}
			
			// Play tick sound
			if (lastSelectPreferences!=selectPreferences)
			{
				playPcm(tick_pcm,tick_pcm_size,255);
				lastSelectPreferences=selectPreferences;
			}
		}
		
		// Draw
		
		// Clear screen
		GRRLIB_FillScreen(C_BLACK);
		
		// state = menu
		if (state==STATE_MENU)
		{
			// Headline
			GRRLIB_Printf(50,50,font,C_LIME,0.75,"ACHTUNG WII KURVE!!");
			
			// Version
			GRRLIB_Printf(510,35,font,C_WHITE,0.4,"V. 1.3");
			
			// Declare select holder
			int selectHolder;
			
			// Selectables
			selectHolder=SELECT_MENU_START;
			GRRLIB_Printf(60,100+20*selectHolder,font,C_WHITE*(selectMenu==selectHolder)+C_GRAY*(selectMenu!=selectHolder),0.5,"START");
			selectHolder=SELECT_MENU_PREFERENCES;
			GRRLIB_Printf(60,100+20*selectHolder,font,C_WHITE*(selectMenu==selectHolder)+C_GRAY*(selectMenu!=selectHolder),0.5,"PREFERENCES");
			selectHolder=SELECT_MENU_CREDITS;
			GRRLIB_Printf(60,100+20*selectHolder,font,C_WHITE*(selectMenu==selectHolder)+C_GRAY*(selectMenu!=selectHolder),0.5,"CREDITS");
			selectHolder=SELECT_MENU_QUIT;
			GRRLIB_Printf(60,100+20*selectHolder,font,C_WHITE*(selectMenu==selectHolder)+C_GRAY*(selectMenu!=selectHolder),0.5,"QUIT");
			
			// Control info
			GRRLIB_Printf(50,380,font,C_GRAY,0.5,"NAVIGATE:          D-PAD");
			GRRLIB_Printf(50,400,font,C_GRAY,0.5,"SELECT:            2/A");
		}
		
		// state = preferences
		if (state==STATE_PREFERENCES)
		{
			// Hold strings
			
			// Boarders
			const char* stringHolderBoardersOn="";
			if (boardersOn==false)
			{
				stringHolderBoardersOn="OFF";
			}else
			{
				stringHolderBoardersOn="ON";
			}
			
			// Rumble
			const char* stringHolderRumble= "";
			if (rumbleIntensity==0)
			{
				stringHolderRumble="OFF";
			}else if (rumbleIntensity==1)
			{
				stringHolderRumble="WEAK";
			}
			else if (rumbleIntensity==2)
			{
				stringHolderRumble="MEDIUM";
			}
			else if (rumbleIntensity==3)
			{
				stringHolderRumble="STRONG";
			}
			
			// Sound effects
			const char* stringHolderSoundEffects= "";
			if (soundEffectsOn==false)
			{
				stringHolderSoundEffects="OFF";
			}else
			{
				stringHolderSoundEffects="ON";
			}
			
			// Music
			const char* stringHolderBackgroundMusic= "";
			if (backgroundMusicOn==false)
			{
				stringHolderBackgroundMusic="OFF";
			}else
			{
				stringHolderBackgroundMusic="ON";
			}
			
			//Headline
			GRRLIB_Printf(50,50,font,C_LIME,0.75,"PREFERENCES");
			
			// Control info
			GRRLIB_Printf(50,380,font,C_GRAY,0.5,"NAVIGATE:          D-PAD");
			GRRLIB_Printf(50,400,font,C_GRAY,0.5,"SELECT / ADJUST:   2/A/1/B");
			
			// Declare select holder
			int selectHolder;
			
			// Selectables
			selectHolder=SELECT_PREFERENCES_COMPS;
			GRRLIB_Printf(60,100+20*selectHolder,font,C_WHITE*(selectPreferences==selectHolder)+C_GRAY*(selectPreferences!=selectHolder),0.5,"COMPS:                %i",numberOfAIs);
			selectHolder=SELECT_PREFERENCES_SPEED;
			GRRLIB_Printf(60,100+20*selectHolder,font,C_WHITE*(selectPreferences==selectHolder)+C_GRAY*(selectPreferences!=selectHolder),0.5,"SPEED:                %i",gameSpeed);
			selectHolder=SELECT_PREFERENCES_TURNING_RADIUS;
			GRRLIB_Printf(60,100+20*selectHolder,font,C_WHITE*(selectPreferences==selectHolder)+C_GRAY*(selectPreferences!=selectHolder),0.5,"TURNING RADIUS:       %i",gameTurningRadius);
			selectHolder=SELECT_PREFERENCES_HOLE_SIZE;
			GRRLIB_Printf(60,100+20*selectHolder,font,C_WHITE*(selectPreferences==selectHolder)+C_GRAY*(selectPreferences!=selectHolder),0.5,"HOLE SIZE:            %i",gameHoleSize);
			selectHolder=SELECT_PREFERENCES_HOLE_SEPARATION;
			GRRLIB_Printf(60,100+20*selectHolder,font,C_WHITE*(selectPreferences==selectHolder)+C_GRAY*(selectPreferences!=selectHolder),0.5,"HOLE SEPARATION:      %i",gameHoleSeparation);
			selectHolder=SELECT_PREFERENCES_GOAL_FACTOR;
			GRRLIB_Printf(60,100+20*selectHolder,font,C_WHITE*(selectPreferences==selectHolder)+C_GRAY*(selectPreferences!=selectHolder),0.5,"GOAL FACTOR:          %i",gameGoalFactor);
			selectHolder=SELECT_PREFERENCES_BOARDERS;
			GRRLIB_Printf(60,100+20*selectHolder,font,C_WHITE*(selectPreferences==selectHolder)+C_GRAY*(selectPreferences!=selectHolder),0.5,"BOARDERS:             %s",stringHolderBoardersOn);
			selectHolder=SELECT_PREFERENCES_RUMBLE;
			GRRLIB_Printf(60,100+20*selectHolder,font,C_WHITE*(selectPreferences==selectHolder)+C_GRAY*(selectPreferences!=selectHolder),0.5,"RUMBLE:               %s",stringHolderRumble);
			selectHolder=SELECT_PREFERENCES_SOUND_EFFECTS;
			GRRLIB_Printf(60,100+20*selectHolder,font,C_WHITE*(selectPreferences==selectHolder)+C_GRAY*(selectPreferences!=selectHolder),0.5,"SOUND EFFECTS:        %s",stringHolderSoundEffects);
			selectHolder=SELECT_PREFERENCES_BACKGROUND_MUSIC;
			GRRLIB_Printf(60,100+20*selectHolder,font,C_WHITE*(selectPreferences==selectHolder)+C_GRAY*(selectPreferences!=selectHolder),0.5,"BACKGROUND MUSIC:     %s",stringHolderBackgroundMusic);
			selectHolder=SELECT_PREFERENCES_SAVE_PREFERENCES;
			GRRLIB_Printf(60,100+20*selectHolder,font,C_WHITE*(selectPreferences==selectHolder)+C_GRAY*(selectPreferences!=selectHolder),0.5,"SAVE PREFERENCES");
			selectHolder=SELECT_PREFERENCES_LOAD_PREFERENCES;
			GRRLIB_Printf(60,100+20*selectHolder,font,C_WHITE*(selectPreferences==selectHolder)+C_GRAY*(selectPreferences!=selectHolder),0.5,"LOAD PREFERENCES");
			selectHolder=SELECT_PREFERENCES_RESTORE_DEFAULTS;
			GRRLIB_Printf(60,100+20*selectHolder,font,C_WHITE*(selectPreferences==selectHolder)+C_GRAY*(selectPreferences!=selectHolder),0.5,"RESTORE DEFAULTS");
			selectHolder=SELECT_PREFERENCES_BACK;
			GRRLIB_Printf(60,100+20*selectHolder,font,C_WHITE*(selectPreferences==selectHolder)+C_GRAY*(selectPreferences!=selectHolder),0.5,"BACK");
		}
		
		// state = players
		if (state==STATE_PLAYERS)
		{
			GRRLIB_Printf(50,50,font,C_LIME,0.75,"CONNECT PLAYERS"); //Players
			int j=0;
			for(int i=0;i<12;i++)
			{
				if ((connected[i])&&((i!=balanceBoardChannel)||(balanceBoardConnected==false)))
				{
					const char* stringHolder = "";
					WPADData *data;
					if ((i>=4)&&(i<8))
					{
						data = WPAD_Data(i-4);
					} else
					{
						data = WPAD_Data(0);
					}
					switch (i)
					{
					case 0:
						if ((balanceBoardConnected==false)||(balanceBoardChannel!=0))
						{
							stringHolder = "WIIMOTE 1";
						}
						break;
					case 1:
						if ((balanceBoardConnected==false)||(balanceBoardChannel!=1))
						{
							stringHolder = "WIIMOTE 2";
						}
						break;
					case 2:
						if ((balanceBoardConnected==false)||(balanceBoardChannel!=2))
						{
							stringHolder = "WIIMOTE 3";
						}
						break;
					case 3:
						if ((balanceBoardConnected==false)||(balanceBoardChannel!=3))
						{
							stringHolder = "WIIMOTE 4";
						}
						break;
					case 4:
						if (data->exp.type==WPAD_EXP_NUNCHUK)
						{
							stringHolder = "NUNCHUK 1";
						} else
						{
							stringHolder = "CLASSIC CONT. 1";
						}
						break;
					case 5:
						if (data->exp.type==WPAD_EXP_NUNCHUK)
						{
							stringHolder = "NUNCHUK 2";
						} else
						{
							stringHolder = "CLASSIC CONT. 2";
						}
						break;
					case 6:
						if (data->exp.type==WPAD_EXP_NUNCHUK)
						{
							stringHolder = "NUNCHUK 3";
						} else
						{
							stringHolder = "CLASSIC CONT. 3";
						}
						break;
					case 7:
						if (data->exp.type==WPAD_EXP_NUNCHUK)
						{
							stringHolder = "NUNCHUK 4";
						} else
						{
							stringHolder = "CLASSIC CONT. 4";
						}
						break;
					case 8:
						stringHolder = "GAMECUBE CONT. 1";
						break;
					case 9:
						stringHolder = "GAMECUBE CONT. 2";
						break;
					case 10:
						stringHolder = "GAMECUBE CONT. 3";
						break;
					case 11:
						stringHolder = "GAMECUBE CONT. 4";
						break;
					}
					GRRLIB_Printf(60,100+j*17,font,COLOR_ARRAY[j],0.5,stringHolder);
					Control tempControl=CONTROL_STICK_CLASSIC;
					if (i<4)
					{
						tempControl=controlWiimote[i];
					}
					if ((i>=4)&&(i<8))
					{
						tempControl=controlNunchuk[i-4];
					}
					if (i>=8)
					{
						tempControl=controlGamecube[i-8];
					}
					switch(tempControl)
					{
						case CONTROL_STICK_CLASSIC:
							stringHolder="STICK - CLASSIC";
							if (i<4)
							{
								stringHolder="D-PAD";
							}
							break;
						case CONTROL_STICK_SPECIAL:
							stringHolder="STICK - SPECIAL";
							break;
						case CONTROL_TILT:
							stringHolder="TILT";
							break;
						case CONTROL_POINT:
							stringHolder="POINT";
							break;
						default:
							stringHolder="ERROR";
							break;
					}
					GRRLIB_Printf(380,100+j*17,font,COLOR_ARRAY[j],0.5,stringHolder);
					j++;
				} else
				{
					if ((i==balanceBoardChannel)&&(balanceBoardConnected))
					{
						GRRLIB_Printf(60,100+j*17,font,COLOR_ARRAY[j],0.5,"BALANCE BOARD");
						j++;
					}
				}
			}
			GRRLIB_Printf(50,320,font,C_GRAY,0.5,"START:      +");
			GRRLIB_Printf(50,340,font,C_GRAY,0.5,"BACK:       -");
			GRRLIB_Printf(50,360,font,C_GRAY,0.5,"CONTROLS:   D-PAD/STICKS");
			GRRLIB_Printf(50,380,font,C_GRAY,0.5,"CONNECT:    WM A/2, NC C, CC A, GC A");
			GRRLIB_Printf(50,400,font,C_GRAY,0.5,"DISCONNECT: WM B/1, NC Z, CC B, GC B");
		}
		
		// state = game
		if (state==STATE_GAME)
		{
			
			// Draw gamegrid
			for(int i=0;i<GRID_WIDTH;i++)
			{
				for(int k=0;k<GRID_HEIGHT;k++)
				{
					u32 color = C_BLACK;
					if (gameGrid[i][k]!=0)
					{
						color=COLOR_ARRAY[gameGrid[i][k]-1];
					}
					GRRLIB_Rectangle(50+i*2,100+k*2,1,1,color,false);
				}
			}
			
			if (!boardersOn)
			{
				// Top
				for (int i=0;i<GRID_WIDTH;i++)
				{
					for (int k=20;k<40;k++)
					{
						u32 color = C_BLACK;
						int m=GRID_HEIGHT-41+k;
						if (gameGrid[i][m]!=0)
						{
							color=COLOR_ARRAY[gameGrid[i][m]-1];
						}
						GRRLIB_Rectangle(50+i*2,100-80+k*2,1,1,color-504+(k+1)*12,false);
					}
				}
				
				// Bottom
				for (int i=0;i<GRID_WIDTH;i++)
				{
					for (int k=20;k<40;k++)
					{
						u32 color = C_BLACK;
						int m=40-k;
						if (gameGrid[i][m]!=0)
						{
							color=COLOR_ARRAY[gameGrid[i][m]-1];
						}
						GRRLIB_Rectangle(50+i*2,98+GRID_HEIGHT*2+80-k*2,1,1,color-504+(k+1)*12,false);
					}
				}
				
				// Left
				for (int i=0;i<GRID_HEIGHT;i++)
				{
					for (int k=20;k<40;k++)
					{
						u32 color = C_BLACK;
						int m=GRID_WIDTH-41+k;
						if (gameGrid[m][i]!=0)
						{
							color=COLOR_ARRAY[gameGrid[m][i]-1];
						}
						GRRLIB_Rectangle(50-80+k*2,100+i*2,1,1,color-504+(k+1)*12,false);
					}
				}
				
				// Right
				for (int i=0;i<GRID_HEIGHT;i++)
				{
					for (int k=20;k<40;k++)
					{
						u32 color = C_BLACK;
						int m=40-k;
						if (gameGrid[m][i]!=0)
						{
							color=COLOR_ARRAY[gameGrid[m][i]-1];
						}
						GRRLIB_Rectangle(48+GRID_WIDTH*2+80-k*2,100+i*2,1,1,color-504+(k+1)*12,false);
					}
				}
			}
			
			// Draw borders
			GRRLIB_Rectangle(49,99,GRID_WIDTH*2,GRID_HEIGHT*2,C_WHITE-(1-boardersOn)*192,false);
			GRRLIB_Rectangle(50,100,(GRID_WIDTH-1)*2,(GRID_HEIGHT-1)*2,C_WHITE-(1-boardersOn)*192,false);
			
			// Draw Round
			bool noScore=false;
			if (suddenDeath==0)
			{
				int j = 0;
				for(int i=0;i<24;i++)
				{
					if ((connected[i])||((balanceBoardConnected==true)&&(i==balanceBoardChannel)))
					{
						j++;
					}
				}
				if (j!=1)
				{
					GRRLIB_Printf(50,50,font,C_GRAY,0.6,"ROUND %i          FIRST TO %i", round, (gameGoalFactor)*(j-1));
				} else
				{
					GRRLIB_Printf(50,70,font,C_GRAY,0.6,"PRACTICE         BACK: MINUS");
					noScore=true;
				}
			} else
			{
				GRRLIB_Printf(50,50,font,C_GRAY,0.6,"SUDDEN DEATH!!!");
			}
			
			// Draw score
			if (!noScore)
			{
				GRRLIB_Printf(50,70,font,C_WHITE,0.6,"SCORE:");
				int j=0;
				for(int i=0;i<24;i++)
				{
					float fontSize=0.6;
					int extraYPos=0;
					for (int k=0;k<24;k++)
					{
						if (score[k] >= 100)
						{
							fontSize=0.4;
							extraYPos=5;
						}
					}
					if (connected[i])
					{
						GRRLIB_Printf(175+j*35,70+extraYPos,font,COLOR_ARRAY[j],fontSize,"%i",score[i]);
						j++;
					}
				}
			}
			
			// Draw kurves clearly from start, draw first kurve point and draw IR
			int k=0;
			irArrayCounter++;
			if (irArrayCounter>9)
			{
				irArrayCounter=0;
			}
			for(int i=0;i<24;i++)
			{
				if ((*kurve[i]).deactivated==false)
				{
					if ((*kurve[i]).ready==false)
					{
						if ((*kurve[i]).dead==false)
						{
							GRRLIB_Rectangle((*kurve[i]).x*2+47,(*kurve[i]).y*2+97,6,6,COLOR_ARRAY[k],true);
							for(int j=0;j<3;j++)
							{
								for(int m=0;m<3;m++)
								{
									GRRLIB_Line((*kurve[i]).x*2+49+j,(*kurve[i]).y*2+99+m,(*kurve[i]).x*2+49+12*cos((*kurve[i]).direction*PI/180)+j,(*kurve[i]).y*2+99+12*sin((*kurve[i]).direction*PI/180)+m,COLOR_ARRAY[k]);
								}
							}
						}
					}
					if ((*kurve[i]).dead==false)
					{
						GRRLIB_Rectangle((*kurve[i]).x*2+50,(*kurve[i]).y*2+100,1,1,COLOR_ARRAY[k],false);
					}
					if ((i<4)&&(controlWiimote[i]==CONTROL_POINT)&&(*kurve[i]).dead==false)
					{
						irArrayX[i][irArrayCounter]=ir[i].x;
						irArrayY[i][irArrayCounter]=ir[i].y;
						const int CIRCLE_SIZE=3 ;
						for(int n=0;n<10;n++)
						{
							int tempX=irArrayX[i][n];
							int tempY=irArrayY[i][n];
							for (int j=-CIRCLE_SIZE;j<=CIRCLE_SIZE;j++)
							{
								for (int m=-CIRCLE_SIZE;m<=CIRCLE_SIZE;m++)
								{
									GRRLIB_Plot(tempX+j,tempY+m,COLOR_ARRAY[k]-(128+sqrt(pow(j,2)+pow(m,2))*128/(sqrt(2*pow(CIRCLE_SIZE,2)+24))));
								}
							}
						}
					}
					k++;
				}
			}
			if (gamePaused)
			{
				GRRLIB_Printf(230,170,font,C_WHITE,1,"PAUSED");
			}
		}
		
		// state = winner
		if (state==STATE_WINNER)
		{
			// Headline
			GRRLIB_Printf(50,50,font,C_LIME,0.75,"WE HAVE A WINNER!!");
			
			// Sort and draw players score
			int numberOfPlayers=0;
			int playersIndex[12];
			int displayPlayersIndex[12];
			for (int i=0;i<12;i++)
			{
				playersIndex[i]=-1;
				displayPlayersIndex[i]=-1;
			}
			int colorIndex[24];
			for (int i=0;i<12;i++)
			{
				colorIndex[i]=0;
			}
			int playersIndexCount=0;
			for (int i=0;i<24;i++)
			{
				if ((*kurve[i]).deactivated==false)
				{
					playersIndex[playersIndexCount]=i;
					colorIndex[i]=playersIndexCount;
					playersIndexCount++;
					numberOfPlayers++;
				}
			}
			bool playersDone[24];
			for (int i=0;i<24;i++)
			{
				playersDone[i]=false;
			}
			for (int j=0;j<numberOfPlayers;j++)
			{
				int highestScore=0;
				for (int i=0;i<numberOfPlayers;i++)
				{
					if ((score[playersIndex[i]]>=highestScore)&&(!playersDone[playersIndex[i]]))
					{
						highestScore=score[playersIndex[i]];
						displayPlayersIndex[j]=playersIndex[i];
					}
				}
				playersDone[displayPlayersIndex[j]]=true;
			}
			for (int i=0;i<numberOfPlayers;i++)
			{
				GRRLIB_Printf(60,100+i*20,font,COLOR_ARRAY[colorIndex[displayPlayersIndex[i]]],0.5,"SCORE:   %i",score[displayPlayersIndex[i]]);
			}
			
			// Control info
			GRRLIB_Printf(50,400,font,C_GRAY,0.5,"PROCEED:         A/2/PLUS");
		}
		
		// state = credits
		if (state==STATE_CREDITS)
		{
			GRRLIB_Printf(50,50,font,C_LIME,0.75,"CREDITS");
			GRRLIB_Printf(100,100,font,C_WHITE,0.5,"CODING:   PROFETYLEN");
			GRRLIB_Printf(100,140,font,C_WHITE,0.5,"TESTING:  SONY (JH)");
			GRRLIB_Printf(100,160,font,C_WHITE,0.5,"          WAXYPUMPKIN72");
			GRRLIB_Printf(100,200,font,C_WHITE,0.5,"MUSIC:    BENNY HILL THEME");
			GRRLIB_Printf(100,240,font,C_WHITE,0.5,"SPECIAL THANKS TO:");
			GRRLIB_Printf(100,265,font,C_WHITE,0.35," - THE AUTHORS OF THE GAME: AUCHTUNG DIE KURVE");
			GRRLIB_Printf(100,265,font,C_WHITE,0.35," - THE AUTHORS OF DEVKIT PRO");
			GRRLIB_Printf(100,285,font,C_WHITE,0.35," - THE AUTHORS OF THE HOMEBREW CHANNEL");
			GRRLIB_Printf(100,305,font,C_WHITE,0.35," - THE AUTHORS OF GRRLIB");
			GRRLIB_Printf(100,325,font,C_WHITE,0.35," - ALL THE HELPFUL PEOPLE AT THE WIIBREW FORUMS");
			GRRLIB_Printf(50,400,font,C_GRAY,0.5,"BACK:            A/B/2/1/PLUS/MINUS");
		}
		
		// Draw cursrors
		if ((state==STATE_CREDITS)||(state==STATE_MENU)||(state==STATE_PREFERENCES)||(state==STATE_PLAYERS))
		{
			for (int i=3;i>=0;i--)
			{
				if((ir[i].x>0)&&(ir[i].y>0))
				{
					int angle=ir[i].angle;
					GRRLIB_DrawImg (ir[i].x-cos((-angle-45)*PI/180)*CURSOR_IMAGE_HYPOTENUSE,ir[i].y+sin((-angle-45)*PI/180)*CURSOR_IMAGE_HYPOTENUSE,img_cursor[i],angle,1,1,C_WHITE);
				}
			}
		}
		
		// Render
		GRRLIB_Render();
		
		// Set fps
		if (state==STATE_GAME)
		{
			fps = 45;
		} else
		{
			fps = 0;
		}
		
		// Adjust timing
		if (fps!=0)
		{
			while (GFX_GetElapsedTime(last)<1000/((float) fps))
			{
				usleep(50);
			}
			last = GFX_GetTime();
		}
		
		// Adjust Volume
		if (GFX_GetElapsedTime(musicTimer)>5)
		{
			if (((((state==STATE_GAME)||(state==STATE_WINNER))&&!gamePaused)&&(backgroundMusicVolume<MAX_MUSIC_VOLUME))
			&&(backgroundMusicOn))
			{
				backgroundMusicVolume++;
			}
			if ((((state!=STATE_GAME)&&(state!=STATE_WINNER))&&(backgroundMusicVolume<MIN_MUSIC_VOLUME))
			&&(backgroundMusicOn))
			{
				backgroundMusicVolume++;
			}
			if ((backgroundMusicVolume>0)
			&&(((backgroundMusicVolume>MIN_MUSIC_VOLUME)&&(((state!=STATE_GAME)&&(state!=STATE_WINNER))||gamePaused))||(!backgroundMusicOn)))
			{
				backgroundMusicVolume--;
			}
			musicTimer=GFX_GetTime();
			ASND_ChangeVolumeVoice(0, backgroundMusicVolume, backgroundMusicVolume);
		}
		
		// Shut off rumbles
		for(int i=0;i<8;i++)
		{
			if (rumbleTime[i]>0)
			{
				rumbleTime[i]--;
				if (rumbleTime[i]==0)
				{
					if (i<4)
					{
						WPAD_Rumble(i, false);
					} else
					{
						PAD_ControlMotor(i-4, false);
					}
				}
			}
		}
	}
	return 0;
}
