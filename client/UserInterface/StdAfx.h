#pragma once

#pragma warning(disable:4702)
#pragma warning(disable:4100)
#pragma warning(disable:4201)
#pragma warning(disable:4511)
#pragma warning(disable:4663)
#pragma warning(disable:4018)
#pragma warning(disable:4245)

#if _MSC_VER >= 1400
//if don't use below, time_t is 64bit
#define _USE_32BIT_TIME_T
#endif

#include "../eterLib/StdAfx.h"
#include "../eterPythonLib/StdAfx.h"
#include "../gameLib/StdAfx.h"
#include "../scriptLib/StdAfx.h"
#include "../milesLib/StdAfx.h"
#include "../EffectLib/StdAfx.h"
#include "../PRTerrainLib/StdAfx.h"
#include "../SpeedTreeLib/StdAfx.h"
#include "../TraducteurLib2/Stdafx.h"

#ifndef __D3DRM_H__
#define __D3DRM_H__
#endif

#include <dshow.h>
#include <qedit.h>
#include "Locale.h"
#include "GameType.h"

extern DWORD __DEFAULT_CODE_PAGE__;

#define APP_NAME	"Metin 2"

enum
{
	POINT_MAX_NUM = 255,	
	CHARACTER_NAME_MAX_LEN = 24,
#if defined(LOCALE_SERVICE_JAPAN)
	PLAYER_NAME_MAX_LEN = 16,
#else
	PLAYER_NAME_MAX_LEN = 12,
#endif
};

void initudp();
void initapp();
void initime();
void initsystem();
void initchr();
void initchrmgr();
void initChat();
void initTextTail();
void initime();
void initItem();
void initNonPlayer();
void initnet();
void initPlayer();
void initSectionDisplayer();
void initServerStateChecker();
void initTrade();
void initMiniMap();
void initProfiler();
void initEvent();
void initeffect();
void initsnd();
void initeventmgr();
void initBackground();
void initwndMgr();
void initshop();
void initpack();
void initskill();
#ifdef NEW_PET_SYSTEM
void initskillpet();
#endif
void initfly();
void initquest();
void initsafebox();
void initguild();
void initMessenger();

extern const std::string& ApplicationStringTable_GetString(DWORD dwID);
extern const std::string& ApplicationStringTable_GetString(DWORD dwID, LPCSTR szKey);

extern const char* ApplicationStringTable_GetStringz(DWORD dwID);
extern const char* ApplicationStringTable_GetStringz(DWORD dwID, LPCSTR szKey);

extern void ApplicationSetErrorString(const char* szErrorString);

typedef std::vector<int> ListeVnumMobVec;
extern ListeVnumMobVec ListeVnumMob;
typedef std::vector<float> ListeScaleMobVec;
extern ListeScaleMobVec ListeScaleMob;

typedef std::vector<int> ListeStatutNageVec;
extern ListeStatutNageVec ListeStatutNage;

typedef std::vector<string> ListePseudoNageVec;
extern ListePseudoNageVec ListePseudoNage;



typedef std::vector<string> ListePseudoJoueurTitreVec;
extern ListePseudoJoueurTitreVec ListePseudoJoueurTitre;
typedef std::vector<string> ListeTitreJoueurTitreVec;
extern ListeTitreJoueurTitreVec ListeTitreJoueurTitre;
typedef std::vector<string> ListeCouleur1TitreVec;
extern ListeCouleur1TitreVec ListeCouleur1Titre;
typedef std::vector<string> ListeCouleur2TitreVec;
extern ListeCouleur2TitreVec ListeCouleur2Titre;
typedef std::vector<string> ListeCouleur3TitreVec;
extern ListeCouleur3TitreVec ListeCouleur3Titre;

typedef std::vector<string> ListePseudoVolVec;
extern ListePseudoVolVec ListePseudoVol;
typedef std::vector<string> ListeStatutVolVec;
extern ListeStatutVolVec ListeStatutVol;
typedef std::vector<string> ListeHauteurVolVec;
extern ListeHauteurVolVec ListeHauteurVol;