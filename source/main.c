#include <stdio.h>
#include <user_mem.h>
#include <stdlib.h>
#include <string.h>
#include <orbisdev.h>
#include <orbislink.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <signal.h>
#include <debugnet.h>
#include <fcntl.h>
#include <orbisNfs.h>
#include <sqlite3.h>
#include <raylib.h>

#define ATTR_ORBISGL_WIDTH 1920 
#define ATTR_ORBISGL_HEIGHT 1080


OrbisPadConfig *confPad;
bool flag=true;

#define BROWSER_BACKGROUND_FILE_PATH "system/textures/msxorbis.png"
#define FILE_ICON_PATH "system/textures/file_icon.png"
#define FOLDER_ICON_PATH "system/textures/folder_icon.png"
#define SETTINGS_BACKGROUND_FILE_PATH "system/textures/settings.png"
#define CREDITS_BACKGROUND_FILE_PATH "system/textures/credits.png"
enum SCREEN_STATUS 
{
	SCREEN_EMU,
	SCREEN_BROWSER,
	SCREEN_SETTINGS,
	SCREEN_CREDITS,
	
};
Vector2 pos;

char path[256];
int notSelected=1;
int flagfolder=0;
int screenStatus=1;
Font font;
float posy=0.0;//-21.0;
bool l1flag=false;
bool r1flag=false;

#define NUM_MODELS  8      // Parametric 3d shapes to generate

Texture2D browserTexture;
Texture2D folderTexture;
Texture2D fileTexture;
Texture2D settingsTexture;
Texture2D creditsTexture;
	

OrbisNfsBrowserListEntry *currentEntry;



void getSizeString(char string[8],char string1[3], uint64_t size) 
{
	double double_size = (double)size;

	int i = 0;
	static char *units[] = { "B ", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
	while (double_size >= 1024.0) {
		double_size /= 1024.0;
		i++;
	}
	//char string1[16];
	snprintf(string, 8, "%.*f",(i == 0) ? 0 : 2, double_size);
	snprintf(string1, 3, "%s",units[i]);
	//debugNetPrintf(DEBUG,"%s %d\n",string,strlen(string));
	//debugNetPrintf(DEBUG,"%s %d\n",string1,strlen(string1));

	//snprintf(string, 16, "%.*f %s",  2, double_size, units[i]);

}

void selected_entry(char *tmp)
{
	int i = orbisNfsBrowserGetRelPos();
	currentEntry = orbisNfsBrowserListGetNthEntry(i);
	debugNetPrintf(DEBUGNET_DEBUG,"orbisNfsBrowserGetRelPos: %d, '%s'\n", i, currentEntry->dir->name);
	if(tmp)
		strcpy(tmp, currentEntry->dir->name);
}

void updateController()
{
	int ret;
	unsigned int buttons=0;
	ret=orbisPadUpdate();
	if(ret==0)
	{
		if(orbisPadGetButtonPressed(ORBISPAD_L2|ORBISPAD_R2) || orbisPadGetButtonHold(ORBISPAD_L2|ORBISPAD_R2))
		{
			debugNetPrintf(DEBUGNET_DEBUG,"Combo L2R2 pressed\n");
			buttons=orbisPadGetCurrentButtonsPressed();
			buttons&= ~(ORBISPAD_L2|ORBISPAD_R2);
			orbisPadSetCurrentButtonsPressed(buttons);
		}
		if(orbisPadGetButtonPressed(ORBISPAD_L1|ORBISPAD_R1) )
		{
			debugNetPrintf(DEBUGNET_DEBUG,"Combo L1R1 pressed\n");
			buttons=orbisPadGetCurrentButtonsPressed();
			buttons&= ~(ORBISPAD_L1|ORBISPAD_R1);
			orbisPadSetCurrentButtonsPressed(buttons);
		}
		if(orbisPadGetButtonPressed(ORBISPAD_L1|ORBISPAD_R2) || orbisPadGetButtonHold(ORBISPAD_L1|ORBISPAD_R2))
		{
			debugNetPrintf(DEBUGNET_DEBUG,"Combo L1R2 pressed\n");
			buttons=orbisPadGetCurrentButtonsPressed();
			buttons&= ~(ORBISPAD_L1|ORBISPAD_R2);
			orbisPadSetCurrentButtonsPressed(buttons);
		}
		if(orbisPadGetButtonPressed(ORBISPAD_L2|ORBISPAD_R1) || orbisPadGetButtonHold(ORBISPAD_L2|ORBISPAD_R1) )
		{
			debugNetPrintf(DEBUGNET_DEBUG,"Combo L2R1 pressed\n");
			buttons=orbisPadGetCurrentButtonsPressed();
			buttons&= ~(ORBISPAD_L2|ORBISPAD_R1);
			orbisPadSetCurrentButtonsPressed(buttons);
		}
		if(orbisPadGetButtonPressed(ORBISPAD_UP))// || orbisPadGetButtonHold(ORBISPAD_UP))
		{
			debugNetPrintf(DEBUGNET_DEBUG,"Up pressed\n");
			switch(screenStatus)
            {
            	case SCREEN_BROWSER: orbisNfsBrowserEntryUp(); selected_entry(NULL); break;
            }
		}
		if(orbisPadGetButtonPressed(ORBISPAD_DOWN))// || orbisPadGetButtonHold(ORBISPAD_DOWN))
		{
			debugNetPrintf(DEBUGNET_DEBUG,"Down pressed\n");
			switch(screenStatus)
            {
            	case SCREEN_BROWSER: orbisNfsBrowserEntryDown(); selected_entry(NULL); break;
            }
		}
		if(orbisPadGetButtonPressed(ORBISPAD_RIGHT))// || orbisPadGetButtonHold(ORBISPAD_RIGHT))
		{
			debugNetPrintf(DEBUGNET_DEBUG,"Right pressed\n");
		}
		if(orbisPadGetButtonPressed(ORBISPAD_LEFT))// || orbisPadGetButtonHold(ORBISPAD_LEFT))
		{
			debugNetPrintf(DEBUGNET_DEBUG,"Left pressed\n");
		}
		if(orbisPadGetButtonPressed(ORBISPAD_TRIANGLE))
		{
			debugNetPrintf(DEBUGNET_DEBUG,"Triangle pressed exit\n");
			//flag=0;
			screenStatus=SCREEN_CREDITS;
		}
		if(orbisPadGetButtonPressed(ORBISPAD_CIRCLE))
		{
			debugNetPrintf(DEBUGNET_DEBUG,"Circle pressed\n");  
			//screenStatus=SCREEN_CREDITS;        
		}
		if(orbisPadGetButtonPressed(ORBISPAD_CROSS))
		{
			debugNetPrintf(DEBUGNET_DEBUG,"Cross pressed\n");
			notSelected=0;
			debugNetPrintf(DEBUGNET_DEBUG,"cross level=%d base=%d rel=%d\n",orbisNfsBrowserGetDirLevel(),orbisNfsBrowserGetBasePos(),orbisNfsBrowserGetRelPos());
			
			//entry=orbisNfsBrowserListGetNthEntry(orbisNfsBrowserGetBasePos()+orbisNfsBrowserGetRelPos());					
			currentEntry=orbisNfsBrowserListGetNthEntry(orbisNfsBrowserGetBasePos()+orbisNfsBrowserGetRelPos());

			if(currentEntry!=NULL)
			{
				
				debugNetPrintf(DEBUGNET_INFO,"cross current entry %s customtype=%d\n",currentEntry->dir->name,currentEntry->dir->customtype);
				
				switch(currentEntry->dir->customtype)
				{	
					case FILE_TYPE_FOLDER:
						notSelected=1;
						//if(strcmp(currentEntry->dir->name, ".") == 0)
						//{
						//	flagfolder=1;
						//}
						//else
						//{
							//if(strcmp(currentEntry->dir->name, ".")!=0)
							//{
								debugNetPrintf(DEBUGNET_DEBUG,"cross selected folder level=%d base=%d rel=%d\n",orbisNfsBrowserGetDirLevel(),orbisNfsBrowserGetBasePos(),orbisNfsBrowserGetRelPos());
								
								//orbisFileBrowserDirLevelUp(currentEntry->dir->name);
								//debugNetPrintf(DEBUG,"cross selected folder level=%d base=%d rel=%d\n",orbisFileBrowserGetDirLevel(),orbisFileBrowserGetBasePos(),orbisFileBrowserGetRelPos());
								flagfolder=1;
								//}
						//}
						break;
					case FILE_TYPE_GAME_ROM:
						
						//if(extension==FILE_TYPE_GAME_ROM)
						//{
							sprintf(path,"%s/%s",orbisNfsBrowserGetListPath(),currentEntry->dir->name);
							debugNetPrintf(DEBUGNET_DEBUG,"cross selected entry game %s\n",path);
							
							debugNetPrintf(DEBUGNET_DEBUG,"change cart\n");
							//actionCartInsertFromHost(path,0);

							//screenStatus=SCREEN_EMU;
							//LoadCart(path,slot,MAP_GUESS);
							//}
						//else
						//{
						//	debugNetPrintf(INFO,"wrong extension choose the right one\n");
							//}
						break;
					case FILE_TYPE_GAME_DSK:
						//if(extension==FILE_TYPE_GAME_DSK)
						//{
						
							sprintf(path,"%s/%s",orbisNfsBrowserGetListPath(),currentEntry->dir->name);
							//LoadFileDrive(path,drive);
							debugNetPrintf(DEBUGNET_DEBUG,"change disk\n");
							//actionDiskInsertFromHost(path,0);
							//screenStatus=SCREEN_EMU;

							//}
						//else
						//{
						//	debugNetPrintf(INFO,"wrong extension choose the right one\n");
							//}
						break;
					case FILE_TYPE_CAS:
					//	if(extension==FILE_TYPE_CAS)
					//	{
							sprintf(path,"%s/%s",orbisNfsBrowserGetListPath(),currentEntry->dir->name);
							//ChangeTape(path);
							debugNetPrintf(DEBUGNET_DEBUG,"change cas\n");
							
					//	}
					//	else
					//	{
					//		debugNetPrintf(INFO,"wrong extension choose the right one\n");
					//	}
						break;
					default:
						debugNetPrintf(DEBUGNET_DEBUG,"wrong extension come back\n");
						//comeBack=1;
						break;
				}
			}

		}
		if(orbisPadGetButtonPressed(ORBISPAD_SQUARE))
		{
			debugNetPrintf(DEBUGNET_DEBUG,"Square pressed\n");
			screenStatus=SCREEN_BROWSER;
		}
		if(orbisPadGetButtonPressed(ORBISPAD_L1))
		{
			debugNetPrintf(DEBUGNET_DEBUG,"L1 pressed %d\n");
			l1flag=1;
			posy=posy-1.0;
		}
		if(orbisPadGetButtonPressed(ORBISPAD_L2))
		{
			debugNetPrintf(DEBUGNET_DEBUG,"L2 pressed %d\n");
		}
		if(orbisPadGetButtonPressed(ORBISPAD_R1))
		{
			posy=posy+1.0;
			debugNetPrintf(DEBUGNET_DEBUG,"R1 pressed\n");
			r1flag=1;
		}
		if(orbisPadGetButtonPressed(ORBISPAD_R2))
		{
			debugNetPrintf(DEBUGNET_DEBUG,"R2 pressed\n");
		}
	}
}

void finishApp()
{
	//orbisAudioFinish();
	//orbisKeyboardFinish();
	//orbisGlFinish();
	orbisPadFinish();
	orbisNfsFinish();
	//ps4LinkFinish();
}



bool initApp()
{
	//orbisNfsInit(NFSEXPORT);
	//orbisFileInit();
	int ret=initOrbisLinkApp();

	sceSystemServiceHideSplashScreen();
	
	
	confPad=orbisPadGetConf(); 
	
	
	return true;
}
void DrawTextXY(Font f,const char *text, int posX, int posY, int fontSize, Color color)     // Draw text (using default font)
{
	Vector2 pos={(float)posX,(float)posY};
	int defaultFontSize = 10;   // Default Font chars height in pixel
	if (fontSize < defaultFontSize) fontSize = defaultFontSize;
	float spacing = fontSize/defaultFontSize;
	DrawTextEx(f,text,pos,(float)fontSize,spacing,color);
	
}
void DrawTextScreen(char const *msg)
{	
	DrawTextXY(font,msg,30,46+posy,22,YELLOW);
	DrawTextXY(font,"Back",68,1013+posy,22,RAYWHITE);
	DrawTextXY(font,"Browser",232,1013+posy,22,RAYWHITE);
	DrawTextXY(font,"Credits",431,1013+posy,22,RAYWHITE);
	DrawTextXY(font,"Select",630,1013+posy,22,RAYWHITE);
}
void DrawTextureSpecial(Texture2D texture,int x, int y,int w, int h)
{
	Rectangle sourceRec = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
	Rectangle destRec = { (float)x, (float)y, (float)w, (float)h };
	Vector2 origin = { 0.0f, 0.0f };
	DrawTexturePro(texture, sourceRec, destRec, origin, 0, WHITE);
}
void browserDrawText(void)
{
	int  i=0;
	char dateString[20]; 
	char sizeString[8];
	char unitString[3];
	char mypath[256];
	Color color;
	if(flagfolder==1 && currentEntry)
	{
		if(strcmp(currentEntry->dir->name,".")!=0)
		{
			debugNetPrintf(DEBUGNET_DEBUG,"go to new directory %s\n",currentEntry->dir->name);
			//char rootpath[256];
			//sprintf(rootpath,"%s/%s",orbisNfsBrowserGetListPath(),currentEntry->dir->name);
			//debugNetPrintf(DEBUG,"go to new directory %s\n",rootpath);
			if(strcmp(currentEntry->dir->name,"..")!=0)
			{
				orbisNfsBrowserDirLevelUp(currentEntry->dir->name);
			}
			else
			{
				orbisNfsBrowserDirLevelDown();
			}
			debugNetPrintf(DEBUGNET_DEBUG,"after orbisNfsBrowserDirLevelUp\n");
		}
		else
		{
			orbisNfsBrowserListRefresh();
		}
		flagfolder=0;
	}
	OrbisNfsBrowserListEntry *entry=orbisNfsBrowserListGetNthEntry(orbisNfsBrowserGetBasePos());
	
	/// header
	//color = (vec4){{ 1., .6, .1, 1. }};
	char *aux=orbisNfsBrowserGetListPath();
	if(aux[0]=='.' && strlen(aux)<=2)
	{
		snprintf(mypath,256,"%s","nfs:/");
	}
	else
	{
		char *aux1=orbisNfsBrowserGetListPath();
		snprintf(mypath,256,"%s%s","nfs:/",&aux1[2]);
	}
	DrawTextXY(font,mypath,30,52+76-30+2-19,22,RED);

	/// iterate entries	
	while(entry && i<MAX_ENTRIES)
	{
		if(entry->dir->customtype==FILE_TYPE_FOLDER)
		{
			if(folderTexture.id!=0)
			{
				//orbisGlDraw(programTextureId,folderTextureId,30,90+i*20);
				DrawTextureSpecial(folderTexture,30,90+i*30+1+67-30-17,20,20);
	
			}
			//sprintf(sizeString,"%s","FOLDER");
			getSizeString(sizeString,unitString,entry->dir->size);
		}
		else
		{
			if(fileTexture.id!=0)
			{
				//orbisGlDraw(programTextureId,fileTextureId,30,90+i*20+2);	
				DrawTextureSpecial(fileTexture,30,90+i*30+1+67-30-17,20,20);

			}
			getSizeString(sizeString,unitString,entry->dir->size);
		}
		//debugNetPrintf(DEBUG,("%s %d\n",entry->name,entry->type);

		// this color mark has to be done in render pass!
		if(i==orbisNfsBrowserGetRelPos())
		{
			color = YELLOW; // selected entry

		}
		else
		{
			color = RAYWHITE;
		}
		//print_text(50+posx,90+i*20+posy,entry->dir->name);
		DrawTextXY(font,entry->dir->name,60,90+i*30+1+67-30-19,22,color);
		
		sprintf(dateString,"%02d/%02d/%04d %02d:%02d %s",
		entry->dir->mtime.day,
		entry->dir->mtime.month,
		entry->dir->mtime.year,
		entry->dir->mtime.hour>12?entry->dir->mtime.hour-12:entry->dir->mtime.hour,
		entry->dir->mtime.minute,
		entry->dir->mtime.hour>=12? "PM" : "AM");	
		DrawTextXY(font,dateString,740+960+8-15-35,90+i*30+1+67-30-19,22,color);
		//orbisGlTextDraw(" hello world",740+960+8-15-35,90+i*30+1+67-30,color);

		DrawTextXY(font,unitString,740+960-8-15-35-22*2,90+i*30+1+67-30-19,22,color);
		DrawTextXY(font,sizeString,740+960-8-15-35-22*7,90+i*30+1+67-30-19,22,color);
				
		entry=entry->next;	
		i++;			
			
	}
}
int main(int argc, char *argv[])
{

	const int screenWidth = ATTR_ORBISGL_WIDTH;
    const int screenHeight = ATTR_ORBISGL_HEIGHT;


	//init liborbis libraries
	flag=initApp();
	if(flag)
	{	
		debugNetPrintf(DEBUGNET_INFO,"[ORBISGL] raylib fself sample for Playstation 4 Retail fucking yeah!!!\n");

		//wrapper values are ignored
		InitWindow(screenWidth, screenHeight,"raylib [models] example - mesh generation");
	}

	browserTexture=LoadTexture(BROWSER_BACKGROUND_FILE_PATH);
	SetTextureFilter(browserTexture, TEXTURE_FILTER_BILINEAR);
	folderTexture=LoadTexture(FOLDER_ICON_PATH);
	SetTextureFilter(folderTexture, TEXTURE_FILTER_BILINEAR);
	debugNetPrintf(DEBUGNET_INFO,"[ORBISGL] id=%d width=%d height=%d %s\n",folderTexture.id,folderTexture.width,folderTexture.height,FOLDER_ICON_PATH);

	fileTexture=LoadTexture(FILE_ICON_PATH);
	SetTextureFilter(fileTexture, TEXTURE_FILTER_BILINEAR);
	debugNetPrintf(DEBUGNET_INFO,"[ORBISGL] id=%d width=%d height=%d %s\n",fileTexture.id,fileTexture.width,fileTexture.height,FILE_ICON_PATH);

	settingsTexture=LoadTexture(SETTINGS_BACKGROUND_FILE_PATH);
	creditsTexture=LoadTexture(CREDITS_BACKGROUND_FILE_PATH);
	SetTextureFilter(creditsTexture, TEXTURE_FILTER_BILINEAR);
	debugNetPrintf(DEBUGNET_INFO,"[ORBISGL] textures loaded\n");

	font=LoadFont("system/fonts/Tahoma_bold.ttf");
	SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
	debugNetPrintf(DEBUGNET_INFO,"[ORBISGL] font loaded\n");

	SetTargetFPS(60);// Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

	while(flag)
	{

		updateController();
		BeginDrawing();

			ClearBackground(RAYWHITE);
			switch(screenStatus)
			{
				case SCREEN_CREDITS:
				{
					ClearBackground(BLUE);                          // Set background color (framebuffer clear color)

					if(creditsTexture.id>0) DrawTextureSpecial(creditsTexture,0,0,ATTR_ORBISGL_WIDTH,ATTR_ORBISGL_HEIGHT);
					//DrawTexture(folderTexture);
					DrawTextXY(font,"libOrbisNfs is a homebrew library based in libnfs and it is part of LIBORBIS for PlayStation 4\nLIBORBIS was created while i was working in MSX emulator for firmware 1.76 so more than 4 years now\nIt is open source and it can be ported easily to others sdks\nSource code of LIBORBIS is available at:\nhttps://github.com/orbisdev/orbisdevsdk_liborbis\nThis software have been done using the following open source tools:\n- ORBISDEV SDK\n- Open Source Toolchain based on LLVM/Clang and gnu binutils compiled on MacOS\n- Gimp for all graphic stuff\nSpecial thanks goes to:\n- ORBISDEV team :P less than 2 months to have a full open solution end to end and with CI ready\n- Hitodama the one and only\n- flatz for fself signing, patching and gl stuff\n- maxtoN for liborbispkg tool to create pkg\n- All ps3dev and ps2dev old comrades\nTo openorbis team thanks to make us finish this, we did not think in do our own tools 2 months ago. Congrats for all your work paella and msx time :P !!!!",60,90+1+67,22,RAYWHITE);
					//DrawTextXY(font,"LIBORBIS was created while i was working in MSX emulator for firmware 1.76 so more than 4 years now",60,90+1*30+1+67,22,RAYWHITE);
					DrawTextScreen("Credits");

					break;
				}
				case SCREEN_BROWSER:
				{
					if(browserTexture.id>0)  DrawTextureSpecial(browserTexture,0,0,ATTR_ORBISGL_WIDTH,ATTR_ORBISGL_HEIGHT);
					browserDrawText();
					DrawTextScreen("Browser");
					break;
				}
			}
		EndDrawing();
		//----------------------------------------------------------------------------------
	}

	// De-Initialization
	//--------------------------------------------------------------------------------------
	UnloadTexture(browserTexture); // Unload texture
	UnloadTexture(folderTexture); // Unload texture
	UnloadTexture(fileTexture); // Unload texture
	UnloadTexture(settingsTexture); // Unload texture
	UnloadTexture(creditsTexture); // Unload texture

	CloseWindow();
	
	finishApp();
	sleep(3);
	exit(EXIT_SUCCESS);
}
