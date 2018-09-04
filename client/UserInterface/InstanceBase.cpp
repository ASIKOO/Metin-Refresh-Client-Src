#include "StdAfx.h"
#include "InstanceBase.h"
#include "PythonBackground.h"
#include "PythonNonPlayer.h"
#include "PythonPlayer.h"
#include "PythonCharacterManager.h"
#include "AbstractPlayer.h"
#include "PythonNetworkStream.h"
#include "AbstractApplication.h"
#include "packet.h"
#include "boost/algorithm/string.hpp"


#include "../eterlib/StateManager.h"
#include "../EterLib/Camera.h"
#include "../gamelib/ItemManager.h"

std::map<int, char*> shiningdata;
std::map<int, char*>::iterator shiningit;

BOOL HAIR_COLOR_ENABLE=FALSE;
BOOL USE_ARMOR_SPECULAR=FALSE;
BOOL RIDE_HORSE_ENABLE=TRUE;
const float c_fDefaultRotationSpeed = 1200.0f;
const float c_fDefaultHorseRotationSpeed = 300.0f;


bool IsWall(unsigned race)
{
	switch (race)
	{
		case 14201:
		case 14202:
		case 14203:
		case 14204:
			return true;
			break;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////


CInstanceBase::SHORSE::SHORSE()
{
	__Initialize();
}

CInstanceBase::SHORSE::~SHORSE()
{
	assert(m_pkActor==NULL);
}

void CInstanceBase::SHORSE::__Initialize()
{
	m_isMounting=false;
	m_pkActor=NULL;
}

void CInstanceBase::SHORSE::SetAttackSpeed(UINT uAtkSpd)
{
	if (!IsMounting())
		return;

	CActorInstance& rkActor=GetActorRef();
	rkActor.SetAttackSpeed(uAtkSpd/100.0f);	
}

void CInstanceBase::SHORSE::SetMoveSpeed(UINT uMovSpd)
{	
	if (!IsMounting())
		return;

	CActorInstance& rkActor=GetActorRef();
	rkActor.SetMoveSpeed(uMovSpd/100.0f);
}

void CInstanceBase::SHORSE::Create(const TPixelPosition& c_rkPPos, UINT eRace, UINT eHitEffect)
{
	assert(NULL==m_pkActor && "CInstanceBase::SHORSE::Create - ALREADY MOUNT");

	m_pkActor=new CActorInstance;

	CActorInstance& rkActor=GetActorRef();
	rkActor.SetEventHandler(CActorInstance::IEventHandler::GetEmptyPtr());
	if (!rkActor.SetRace(eRace))
	{
		delete m_pkActor;
		m_pkActor=NULL;
		return;
	}

	rkActor.SetShape(0);
	rkActor.SetBattleHitEffect(eHitEffect);
	rkActor.SetAlphaValue(0.0f);
	rkActor.BlendAlphaValue(1.0f, 0.5f);
	rkActor.SetMoveSpeed(1.0f);
	rkActor.SetAttackSpeed(1.0f);
	rkActor.SetMotionMode(CRaceMotionData::MODE_GENERAL);
	rkActor.Stop();
	rkActor.RefreshActorInstance();

	rkActor.SetCurPixelPosition(c_rkPPos);

	m_isMounting=true;
}

void CInstanceBase::SHORSE::Destroy()
{
	if (m_pkActor)
	{
		m_pkActor->Destroy();
		delete m_pkActor;	
	}	

	__Initialize();
}

CActorInstance& CInstanceBase::SHORSE::GetActorRef()
{
	assert(NULL!=m_pkActor && "CInstanceBase::SHORSE::GetActorRef");
	return *m_pkActor;
}

CActorInstance* CInstanceBase::SHORSE::GetActorPtr()
{
	return m_pkActor;
}

UINT CInstanceBase::SHORSE::GetLevel()
{
	if (m_pkActor)
	{
		DWORD mount = m_pkActor->GetRace();
		switch (mount)
		{
		case 20101:
		case 20102:
		case 20103:
			return 1;
		case 20104:
		case 20105:
		case 20106:
			return 2;
		case 20107:
		case 20108:
		case 20109:
		case 20110: // #0000673: [M2EU] 새로운 탈것 타고 공격 안됨 
		case 20111: // #0000673: [M2EU] 새로운 탈것 타고 공격 안됨 
		case 20112: // #0000673: [M2EU] 새로운 탈것 타고 공격 안됨 
		case 20113: // #0000673: [M2EU] 새로운 탈것 타고 공격 안됨 
		case 20114:
		case 20115:
		case 20116:
		case 20117:
		case 20118:
		case 20120:
		case 20121:
		case 20122:
		case 20123:
		case 20124:
		case 20125:
			return 3;
		case 20229:
			return 2;
		case 20234:
		case 20235:
		case 20236:
		case 20237:
		case 20238:
		case 20243:
		case 20244:
		case 20245:
			return 3;
		}

		// 마운트 확장 시스템용 특수 처리 (20201 ~ 20212 대역을 사용하고 순서대로 4개씩 나눠서 초급, 중급, 고급임)
		//	-- 탈것 레벨을 클라에서 측정하고 공격/스킬 사용가능 여부도 클라에서 처리하는 것 자체에 문제가 있는 듯.. [hyo]
		{
			// 중급 탈것은 레벨2 (공격 가능, 스킬 불가)
			if ((20205 <= mount && 20208 >= mount) ||
				(20214 == mount) || (20217 == mount)			// 난폭한 전갑순순록, 난폭한 전갑암순록
				)
				return 2;

			// 고급 탈것은 레벨3 (공격 가능, 스킬 가능)
			if ((20209 <= mount && 20212 >= mount) ||
				(20215 == mount) || (20218 == mount) || (20119 == mount) ||		// 용맹한 전갑순순록, 용맹한 전갑암순록
				(20219 <= mount && 20222 >= mount) ||
				(20225 <= mount && 20227 >= mount) ||
				(20230 <= mount && 20235 >= mount) || (mount <= 20240 && 20245 >= mount))
				return 3;

			if (20243 == mount || 20244 == mount || 20245 == mount)
				return 3;

		}
	}
	return 0;
}

bool CInstanceBase::SHORSE::IsNewMount()
{
	if (!m_pkActor)
		return false;
	DWORD mount = m_pkActor->GetRace();

	if ((20205 <= mount &&  20208 >= mount) ||
		(20214 == mount) || (20217 == mount)			// ³­ÆøÇÑ Àü°©¼ø¼ø·Ï, ³­ÆøÇÑ Àü°©¾Ï¼ø·Ï
		)
		return true;

	// °í±Þ Å»°Í
	if ((20209 <= mount &&  20212 >= mount) || 
		(20215 == mount) || (20218 == mount) ||			// ¿ë¸ÍÇÑ Àü°©¼ø¼ø·Ï, ¿ë¸ÍÇÑ Àü°©¾Ï¼ø·Ï
		(20220 == mount)
		)
		return true;

	return false;
}

bool CInstanceBase::SHORSE::CanUseSkill()
{
	// ¸¶»ó½ºÅ³Àº ¸»ÀÇ ·¹º§ÀÌ 3 ÀÌ»óÀÌ¾î¾ß¸¸ ÇÔ.
	if (IsMounting())
		return 2 < GetLevel();

	return true;
}

bool CInstanceBase::SHORSE::CanAttack()
{
	if (IsMounting())
		if (GetLevel()<=1)
			return false;

	return true;
}
			
bool CInstanceBase::SHORSE::IsMounting()
{
	return m_isMounting;
}

void CInstanceBase::SHORSE::Deform()
{
	if (!IsMounting())
		return;

	CActorInstance& rkActor=GetActorRef();
	rkActor.INSTANCEBASE_Deform();
}

void CInstanceBase::SHORSE::Render()
{
	if (!IsMounting())
		return;

	CActorInstance& rkActor=GetActorRef();
	rkActor.Render();
}

void CInstanceBase::__AttachHorseSaddle()
{
	if (!IsMountingHorse())
		return;
	m_kHorse.m_pkActor->AttachModelInstance(CRaceData::PART_MAIN, "saddle", m_GraphicThingInstance, CRaceData::PART_MAIN);
}

void CInstanceBase::__DetachHorseSaddle()
{
	if (!IsMountingHorse())
		return;
	m_kHorse.m_pkActor->DetachModelInstance(CRaceData::PART_MAIN, m_GraphicThingInstance, CRaceData::PART_MAIN);
}

//////////////////////////////////////////////////////////////////////////////////////

void CInstanceBase::BlockMovement()
{
	m_GraphicThingInstance.BlockMovement();
}

bool CInstanceBase::IsBlockObject(const CGraphicObjectInstance& c_rkBGObj)
{
	return m_GraphicThingInstance.IsBlockObject(c_rkBGObj);
}

bool CInstanceBase::AvoidObject(const CGraphicObjectInstance& c_rkBGObj)
{
	return m_GraphicThingInstance.AvoidObject(c_rkBGObj);
}

///////////////////////////////////////////////////////////////////////////////////

bool __ArmorVnumToShape(int iVnum, DWORD * pdwShape)
{
	*pdwShape = iVnum;

	/////////////////////////////////////////

	if (0 == iVnum || 1 == iVnum)
		return false;

	if (!USE_ARMOR_SPECULAR)
		return false;

	CItemData * pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(iVnum, &pItemData))
		return false;

	enum
	{
		SHAPE_VALUE_SLOT_INDEX = 3,
	};

	*pdwShape = pItemData->GetValue(SHAPE_VALUE_SLOT_INDEX);

	return true;
}

// 2004.07.05.myevan.±Ã½ÅÅº¿µ ³¢ÀÌ´Â ¹®Á¦
class CActorInstanceBackground : public IBackground
{
	public:
		CActorInstanceBackground() {}
		virtual ~CActorInstanceBackground() {}
		bool IsBlock(int x, int y)
		{
			CPythonBackground& rkBG=CPythonBackground::Instance();
			return rkBG.isAttrOn(x, y, CTerrainImpl::ATTRIBUTE_BLOCK);
		}
};

static CActorInstanceBackground gs_kActorInstBG;

bool CInstanceBase::LessRenderOrder(CInstanceBase* pkInst)
{
	int nMainAlpha=(__GetAlphaValue() < 1.0f) ? 1 : 0;
	int nTestAlpha=(pkInst->__GetAlphaValue() < 1.0f) ? 1 : 0;
	if (nMainAlpha < nTestAlpha)
		return true;
	if (nMainAlpha > nTestAlpha)
		return false;

	if (GetRace()<pkInst->GetRace())
		return true;
	if (GetRace()>pkInst->GetRace())
		return false;

	if (GetShape()<pkInst->GetShape())
		return true;

	if (GetShape()>pkInst->GetShape())
		return false;

	UINT uLeftLODLevel=__LessRenderOrder_GetLODLevel();
	UINT uRightLODLevel=pkInst->__LessRenderOrder_GetLODLevel();
	if (uLeftLODLevel<uRightLODLevel)
		return true;
	if (uLeftLODLevel>uRightLODLevel)
		return false;

	if (m_awPart[CRaceData::PART_WEAPON]<pkInst->m_awPart[CRaceData::PART_WEAPON])
		return true;

	return false;
}

UINT CInstanceBase::__LessRenderOrder_GetLODLevel()
{
	CGrannyLODController* pLODCtrl=m_GraphicThingInstance.GetLODControllerPointer(0);
	if (!pLODCtrl)
		return 0;

	return pLODCtrl->GetLODLevel();
}

bool CInstanceBase::__Background_GetWaterHeight(const TPixelPosition& c_rkPPos, float* pfHeight)
{
	long lHeight;
	if (!CPythonBackground::Instance().GetWaterHeight((int)c_rkPPos.x, (int)c_rkPPos.y, &lHeight))
		return false;

	*pfHeight = float(lHeight);

	return true;
}

bool CInstanceBase::__Background_IsWaterPixelPosition(const TPixelPosition& c_rkPPos)
{
	return CPythonBackground::Instance().isAttrOn(c_rkPPos.x, c_rkPPos.y, CTerrainImpl::ATTRIBUTE_WATER);
}

const float PC_DUST_RANGE = 2000.0f;
const float NPC_DUST_RANGE = 1000.0f;

DWORD CInstanceBase::ms_dwUpdateCounter=0;
DWORD CInstanceBase::ms_dwRenderCounter=0;
DWORD CInstanceBase::ms_dwDeformCounter=0;

CDynamicPool<CInstanceBase> CInstanceBase::ms_kPool;

bool CInstanceBase::__IsInDustRange()
{
	if (!__IsExistMainInstance())
		return false;

	CInstanceBase* pkInstMain=__GetMainInstancePtr();

	float fDistance=NEW_GetDistanceFromDestInstance(*pkInstMain);

	if (IsPC())
	{
		if (fDistance<=PC_DUST_RANGE)
			return true;
	}

	if (fDistance<=NPC_DUST_RANGE)
		return true;

	return false;
}

void CInstanceBase::__EnableSkipCollision()
{
	if (__IsMainInstance())
	{
		TraceError("CInstanceBase::__EnableSkipCollision - ÀÚ½ÅÀº Ãæµ¹°Ë»ç½ºÅµÀÌ µÇ¸é ¾ÈµÈ´Ù!!");
		return;
	}
	m_GraphicThingInstance.EnableSkipCollision();
}

void CInstanceBase::__DisableSkipCollision()
{
	m_GraphicThingInstance.DisableSkipCollision();
}

DWORD CInstanceBase::__GetShadowMapColor(float x, float y)
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	return rkBG.GetShadowMapColor(x, y);
}

float CInstanceBase::__GetBackgroundHeight(float x, float y)
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	return rkBG.GetHeight(x, y);
}

#ifdef __MOVIE_MODE__

BOOL CInstanceBase::IsMovieMode()
{
	if (IsAffect(AFFECT_INVISIBILITY))
		return true;

	return false;
}

#endif

BOOL CInstanceBase::IsInvisibility()
{
	if (IsAffect(AFFECT_INVISIBILITY))
		return true;

	return false;
}

BOOL CInstanceBase::IsParalysis()
{
	return m_GraphicThingInstance.IsParalysis();
}

BOOL CInstanceBase::IsGameMaster()
{
	if (m_kAffectFlagContainer.IsSet(AFFECT_YMIR))
		return true;
	return false;
}


BOOL CInstanceBase::IsSameEmpire(CInstanceBase& rkInstDst)
{
	if (0 == rkInstDst.m_dwEmpireID)
		return TRUE;

	if (IsGameMaster())
		return TRUE;

	if (rkInstDst.IsGameMaster())
		return TRUE;

	if (rkInstDst.m_dwEmpireID==m_dwEmpireID)
		return TRUE;

	return FALSE;
}

DWORD CInstanceBase::GetEmpireID()
{
	return m_dwEmpireID;
}

DWORD CInstanceBase::Get7emeSkill(){
	return m_dw7emeSkill;
}

DWORD CInstanceBase::Get8emeSkill(){
	return m_dw8emeSKill;
}

DWORD CInstanceBase::GetGuildID()
{
	return m_dwGuildID;
}

DWORD CInstanceBase::GetAIFlag()
{
	return m_dwAIFlag;
}

int CInstanceBase::GetAlignment()
{
	return m_sAlignment;
}

UINT CInstanceBase::GetAlignmentGrade()
{
	if (m_sAlignment >= 12000)
		return 0;
	else if (m_sAlignment >= 8000)
		return 1;
	else if (m_sAlignment >= 4000)
		return 2;
	else if (m_sAlignment >= 1000)
		return 3;
	else if (m_sAlignment >= 0)
		return 4;
	else if (m_sAlignment > -4000)
		return 5;
	else if (m_sAlignment > -8000)
		return 6;
	else if (m_sAlignment > -12000)
		return 7;

	return 8;
}

int CInstanceBase::GetAlignmentType()
{
	switch (GetAlignmentGrade())
	{
		case 0:
		case 1:
		case 2:
		case 3:
		{
			return ALIGNMENT_TYPE_WHITE;
			break;
		}

		case 5:
		case 6:
		case 7:
		case 8:
		{
			return ALIGNMENT_TYPE_DARK;
			break;
		}
	}

	return ALIGNMENT_TYPE_NORMAL;
}

BYTE CInstanceBase::GetPKMode()
{
	return m_byPKMode;
}

bool CInstanceBase::IsKiller()
{
	return m_isKiller;
}

bool CInstanceBase::IsPartyMember()
{
	return m_isPartyMember;
}

BOOL CInstanceBase::IsInSafe()
{
	const TPixelPosition& c_rkPPosCur=m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
	if (CPythonBackground::Instance().isAttrOn(c_rkPPosCur.x, c_rkPPosCur.y, CTerrainImpl::ATTRIBUTE_BANPK))
		return TRUE;

	return FALSE;
}

float CInstanceBase::CalculateDistanceSq3d(const TPixelPosition& c_rkPPosDst)
{
	const TPixelPosition& c_rkPPosSrc=m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
	return SPixelPosition_CalculateDistanceSq3d(c_rkPPosSrc, c_rkPPosDst);
}

void CInstanceBase::OnSelected()
{
#ifdef __MOVIE_MODE__
	if (!__IsExistMainInstance())
		return;
#endif

	if (IsStoneDoor())
		return;

	if (IsDead())
		return;

	__AttachSelectEffect();
}

void CInstanceBase::OnUnselected()
{
	__DetachSelectEffect();
}

void CInstanceBase::OnTargeted()
{
#ifdef __MOVIE_MODE__
	if (!__IsExistMainInstance())
		return;
#endif

	if (IsStoneDoor())
		return;

	if (IsDead())
		return;

	__AttachTargetEffect();
}

void CInstanceBase::OnUntargeted()
{
	__DetachTargetEffect();
}

void CInstanceBase::DestroySystem()
{
	ms_kPool.Clear();
}

void CInstanceBase::CreateSystem(UINT uCapacity)
{
	ms_kPool.Create(uCapacity);

	memset(ms_adwCRCAffectEffect, 0, sizeof(ms_adwCRCAffectEffect));

	ms_fDustGap=250.0f;
	ms_fHorseDustGap=500.0f;
}

CInstanceBase* CInstanceBase::New()
{
	return ms_kPool.Alloc();
}

void CInstanceBase::Delete(CInstanceBase* pkInst)
{
	pkInst->Destroy();
	ms_kPool.Free(pkInst);
}

void CInstanceBase::SetMainInstance()
{
	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();

	DWORD dwVID=GetVirtualID();
	rkChrMgr.SetMainInstance(dwVID);

	m_GraphicThingInstance.SetMainInstance();
}

CInstanceBase* CInstanceBase::__GetMainInstancePtr()
{
	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	return rkChrMgr.GetMainInstancePtr();
}

void CInstanceBase::__ClearMainInstance()
{
	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	rkChrMgr.ClearMainInstance();
}

/* ½ÇÁ¦ ÇÃ·¹ÀÌ¾î Ä³¸¯ÅÍÀÎÁö Á¶»ç.*/
bool CInstanceBase::__IsMainInstance()
{
	if (this==__GetMainInstancePtr())
		return true;

	return false;
}

bool CInstanceBase::__IsExistMainInstance()
{
	if(__GetMainInstancePtr())
		return true;
	else
		return false;
}

bool CInstanceBase::__MainCanSeeHiddenThing()
{
	return false;
//	CInstanceBase * pInstance = __GetMainInstancePtr();
//	return pInstance->IsAffect(AFFECT_GAMJI);
}

float CInstanceBase::__GetBowRange()
{
	float fRange = 2500.0f - 100.0f;

	if (__IsMainInstance())
	{
		IAbstractPlayer& rPlayer=IAbstractPlayer::GetSingleton();
		fRange += float(rPlayer.GetStatus(POINT_BOW_DISTANCE));
	}

	return fRange;
}

CInstanceBase* CInstanceBase::__FindInstancePtr(DWORD dwVID)
{
	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	return rkChrMgr.GetInstancePtr(dwVID);
}

bool CInstanceBase::__FindRaceType(DWORD dwRace, BYTE* pbType)
{
	CPythonNonPlayer& rkNonPlayer=CPythonNonPlayer::Instance();
	return rkNonPlayer.GetInstanceType(dwRace, pbType);
}

bool CInstanceBase::Create(const SCreateData& c_rkCreateData)
{
	IAbstractApplication::GetSingleton().SkipRenderBuffering(300);

	SetInstanceType(c_rkCreateData.m_bType);


#ifdef NEW_PET_SYSTEM
	if (!SetRace(c_rkCreateData.m_dwRace))
		return false;

	if (c_rkCreateData.m_dwRace == 34041 || c_rkCreateData.m_dwRace == 34045 || c_rkCreateData.m_dwRace == 34049 || c_rkCreateData.m_dwRace == 34053 || c_rkCreateData.m_dwRace == 34036 || c_rkCreateData.m_dwRace == 34064)
	{
		if (c_rkCreateData.m_dwLevel >= 81)
			SetRace(c_rkCreateData.m_dwRace + 1);
	}
#else
	if (!SetRace(c_rkCreateData.m_dwRace))
		return false;
#endif

	SetVirtualID(c_rkCreateData.m_dwVID);

	if (c_rkCreateData.m_isMain)
		SetMainInstance();

	if (IsGuildWall())
	{
		unsigned center_x;
		unsigned center_y;

		c_rkCreateData.m_kAffectFlags.ConvertToPosition(&center_x, &center_y);
		
		float center_z = __GetBackgroundHeight(center_x, center_y);
		NEW_SetPixelPosition(TPixelPosition(float(c_rkCreateData.m_lPosX), float(c_rkCreateData.m_lPosY), center_z));
	}
	else
	{
		SCRIPT_SetPixelPosition(float(c_rkCreateData.m_lPosX), float(c_rkCreateData.m_lPosY));
	}	

	if (0 != c_rkCreateData.m_dwMountVnum)
		MountHorse(c_rkCreateData.m_dwMountVnum);

	SetArmor(c_rkCreateData.m_dwArmor);

	if (IsPC())
	{
		SetHair(c_rkCreateData.m_dwHair);
		SetAcce(c_rkCreateData.m_dwAcce);
		SetWeapon(c_rkCreateData.m_dwWeapon);
	}

	__Create_SetName(c_rkCreateData);

	m_dwLevel = c_rkCreateData.m_dwLevel;
	IAbstractPlayer& rPlayer = IAbstractPlayer::GetSingleton();

	m_dw7emeSkill = rPlayer.GetStatus(POINT_7EMESKILL);
	m_dw8emeSKill = rPlayer.GetStatus(POINT_8EMESKILL);
	m_dwAIFlag = c_rkCreateData.m_dwAIFlag;
	m_dwGuildID = c_rkCreateData.m_dwGuildID;
	m_dwEmpireID = c_rkCreateData.m_dwEmpireID;

	SetVirtualNumber(c_rkCreateData.m_dwRace);
	SetRotation(c_rkCreateData.m_fRot);

	SetAlignment(c_rkCreateData.m_sAlignment);
#ifdef NEW_PET_SYSTEM
	SetLevelText(c_rkCreateData.m_dwLevel);
#endif
	SetPKMode(c_rkCreateData.m_byPKMode);

	SetMoveSpeed(c_rkCreateData.m_dwMovSpd);
	SetAttackSpeed(c_rkCreateData.m_dwAtkSpd);

#ifdef ENABLE_VOL_SYSTEM
	AileVolCreer = false;
#endif
#ifdef NEW_PET_SYSTEM
	if (m_dwRace == 34041 || m_dwRace == 34045 || m_dwRace == 34049 || m_dwRace == 34053 || m_dwRace == 34036 || m_dwRace == 34064){
		float scale = c_rkCreateData.m_dwLevel * 0.005f + 0.75f;
		for (int i = 0; i < CRaceData::PART_MAX_NUM; i++)
		{
			m_GraphicThingInstance.SetScaleNew(i, scale, scale, scale);
		}
	}
#endif
	
	// NOTE : Dress ¸¦ ÀÔ°í ÀÖÀ¸¸é Alpha ¸¦ ³ÖÁö ¾Ê´Â´Ù.
	if (!IsWearingDress())
	{
		// NOTE : ¹Ýµå½Ã Affect ¼ÂÆÃ À­ÂÊ¿¡ ÀÖ¾î¾ß ÇÔ
		m_GraphicThingInstance.SetAlphaValue(0.0f);
		m_GraphicThingInstance.BlendAlphaValue(1.0f, 0.5f);
	}

	if (!IsGuildWall())
	{
		SetAffectFlagContainer(c_rkCreateData.m_kAffectFlags);
	}	

	// NOTE : ¹Ýµå½Ã Affect ¼ÂÆÃ ÈÄ¿¡ ÇØ¾ß ÇÔ
	AttachTextTail();
	RefreshTextTail();

	if (c_rkCreateData.m_dwStateFlags & ADD_CHARACTER_STATE_SPAWN) 
	{
		if (IsAffect(AFFECT_SPAWN))
			__AttachEffect(EFFECT_SPAWN_APPEAR);

		if (IsPC())
		{
			Refresh(CRaceMotionData::NAME_WAIT, true);
		}
		else
		{
			Refresh(CRaceMotionData::NAME_SPAWN, false);
		}
	}
	else
	{
		Refresh(CRaceMotionData::NAME_WAIT, true);
	}

	__AttachEmpireEffect(c_rkCreateData.m_dwEmpireID);

	RegisterBoundingSphere();

	if (c_rkCreateData.m_dwStateFlags & ADD_CHARACTER_STATE_DEAD)
		m_GraphicThingInstance.DieEnd();

	SetStateFlags(c_rkCreateData.m_dwStateFlags);

	m_GraphicThingInstance.SetBattleHitEffect(ms_adwCRCAffectEffect[EFFECT_HIT]);

	if (!IsPC())
	{
		DWORD dwBodyColor = CPythonNonPlayer::Instance().GetMonsterColor(c_rkCreateData.m_dwRace);
		if (0 != dwBodyColor)
		{
			SetModulateRenderMode();
			SetAddColor(dwBodyColor);
		}
	}

	__AttachHorseSaddle();

	// ±æµå ½Éº¼À» À§ÇÑ ÀÓ½Ã ÄÚµå, ÀûÁ¤ À§Ä¡¸¦ Ã£´Â Áß
	const int c_iGuildSymbolRace = 14200;
	if (c_iGuildSymbolRace == GetRace())
	{
		std::string strFileName = GetGuildSymbolFileName(m_dwGuildID);
		if (IsFile(strFileName.c_str()))
			m_GraphicThingInstance.ChangeMaterial(strFileName.c_str());
	}

	return true;
}


void CInstanceBase::__Create_SetName(const SCreateData& c_rkCreateData)
{
	if (IsGoto())
	{
		SetNameString("", 0);
		return;
	}
	if (IsWarp())
	{
		__Create_SetWarpName(c_rkCreateData);
		return;
	}

	SetNameString(c_rkCreateData.m_stName.c_str(), c_rkCreateData.m_stName.length());
}

void CInstanceBase::__Create_SetWarpName(const SCreateData& c_rkCreateData)
{
	const char * c_szName;
	if (CPythonNonPlayer::Instance().GetName(c_rkCreateData.m_dwRace, &c_szName))
	{
		std::string strName = c_szName;
		int iFindingPos = strName.find_first_of(" ", 0);
		if (iFindingPos > 0)
		{
			strName.resize(iFindingPos);
		}
		SetNameString(strName.c_str(), strName.length());
	}
	else
	{
		SetNameString(c_rkCreateData.m_stName.c_str(), c_rkCreateData.m_stName.length());
	}
}

void CInstanceBase::SetNameString(const char* c_szName, int len)
{
	m_stName.assign(c_szName, len);
}


bool CInstanceBase::SetRace(DWORD eRace)
{
	m_dwRace = eRace;

	if (!m_GraphicThingInstance.SetRace(eRace))
		return false;

	if (!__FindRaceType(m_dwRace, &m_eRaceType))
		m_eRaceType=CActorInstance::TYPE_PC;

	return true;
}

BOOL CInstanceBase::__IsChangableWeapon(int iWeaponID)
{	
	// µå·¹½º ÀÔ°í ÀÖÀ»¶§´Â ºÎÄÉ¿ÜÀÇ Àåºñ´Â ³ª¿ÀÁö ¾Ê°Ô..
	if (IsWearingDress())
	{
		const int c_iBouquets[] =
		{
			50201,	// Bouquet for Assassin
			50202,	// Bouquet for Shaman
			50203,
			50204,
			0, // #0000545: [M2CN] ¿þµù µå·¹½º¿Í Àåºñ Âø¿ë ¹®Á¦
		};

		for (int i = 0; c_iBouquets[i] != 0; ++i)
			if (iWeaponID == c_iBouquets[i])
				return true;

		return false;
	}
	else
		return true;
}

BOOL CInstanceBase::IsWearingDress()
{
	const int c_iWeddingDressShape = 201;
	return c_iWeddingDressShape == m_eShape;
}

BOOL CInstanceBase::IsHoldingPickAxe()
{
	const int c_iPickAxeStart = 29101;
	const int c_iPickAxeEnd = 29110;
	return m_awPart[CRaceData::PART_WEAPON] >= c_iPickAxeStart && m_awPart[CRaceData::PART_WEAPON] <= c_iPickAxeEnd;
}

BOOL CInstanceBase::IsNewMount()
{
	return m_kHorse.IsNewMount();
}

BOOL CInstanceBase::IsMountingHorse()
{
	return m_kHorse.IsMounting();
}

BOOL CInstanceBase::IsMountingHorseOnly()
{
	if (m_kHorse.IsMounting())
	{
		DWORD dwMountVnum = m_kHorse.m_pkActor->GetRace();
		if ((dwMountVnum >= 20101 && dwMountVnum <= 20109) ||
			(dwMountVnum == 20029 || dwMountVnum == 20030))
			return TRUE;
	}

	return FALSE;
}

void CInstanceBase::MountHorse(UINT eRace)
{
	m_kHorse.Destroy();
	m_kHorse.Create(m_GraphicThingInstance.NEW_GetCurPixelPositionRef(), eRace, ms_adwCRCAffectEffect[EFFECT_HIT]);

	SetMotionMode(CRaceMotionData::MODE_HORSE);	
	SetRotationSpeed(c_fDefaultHorseRotationSpeed);

	m_GraphicThingInstance.MountHorse(m_kHorse.GetActorPtr());
	m_GraphicThingInstance.Stop();
	m_GraphicThingInstance.RefreshActorInstance();
}

void CInstanceBase::DismountHorse()
{
	m_kHorse.Destroy();
}

void CInstanceBase::GetInfo(std::string* pstInfo)
{
	char szInfo[256];
	sprintf(szInfo, "Inst - UC %d, RC %d Pool - %d ", 
		ms_dwUpdateCounter, 
		ms_dwRenderCounter,
		ms_kPool.GetCapacity()
	);

	pstInfo->append(szInfo);
}

void CInstanceBase::ResetPerformanceCounter()
{
	ms_dwUpdateCounter=0;
	ms_dwRenderCounter=0;
	ms_dwDeformCounter=0;
}

bool CInstanceBase::NEW_IsLastPixelPosition()
{
	return m_GraphicThingInstance.IsPushing();
}

const TPixelPosition& CInstanceBase::NEW_GetLastPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetLastPixelPositionRef();
}

void CInstanceBase::NEW_SetDstPixelPositionZ(FLOAT z)
{
	m_GraphicThingInstance.NEW_SetDstPixelPositionZ(z);
}

void CInstanceBase::NEW_SetDstPixelPosition(const TPixelPosition& c_rkPPosDst)
{
	m_GraphicThingInstance.NEW_SetDstPixelPosition(c_rkPPosDst);
}

void CInstanceBase::NEW_SetSrcPixelPosition(const TPixelPosition& c_rkPPosSrc)
{
	m_GraphicThingInstance.NEW_SetSrcPixelPosition(c_rkPPosSrc);
}

const TPixelPosition& CInstanceBase::NEW_GetCurPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetCurPixelPositionRef();	
}

const TPixelPosition& CInstanceBase::NEW_GetDstPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetDstPixelPositionRef();
}

const TPixelPosition& CInstanceBase::NEW_GetSrcPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetSrcPixelPositionRef();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void CInstanceBase::OnSyncing()
{
	m_GraphicThingInstance.__OnSyncing();
}

void CInstanceBase::OnWaiting()
{
	m_GraphicThingInstance.__OnWaiting();
}

void CInstanceBase::OnMoving()
{
	m_GraphicThingInstance.__OnMoving();
}

void CInstanceBase::ChangeGuild(DWORD dwGuildID)
{
	m_dwGuildID=dwGuildID;

	DetachTextTail();
	AttachTextTail();
	RefreshTextTail();
}

DWORD CInstanceBase::GetPart(CRaceData::EParts part)
{
	assert(part >= 0 && part < CRaceData::PART_MAX_NUM);
	return m_awPart[part];
}

DWORD CInstanceBase::GetShape()
{
	return m_eShape;
}

bool CInstanceBase::CanAct()
{
	return m_GraphicThingInstance.CanAct();
}

bool CInstanceBase::CanMove()
{
	return m_GraphicThingInstance.CanMove();
}

bool CInstanceBase::CanUseSkill()
{
	if (IsPoly())
		return false;

	if (IsWearingDress())
		return false;

	if (IsHoldingPickAxe())
		return false;

	if (!m_kHorse.CanUseSkill())
		return false;

	if (!m_GraphicThingInstance.CanUseSkill())
		return false;

	return true;
}

bool CInstanceBase::CanAttack()
{
	if (!m_kHorse.CanAttack())
		return false;

	if (IsWearingDress())
		return false;

	if (IsHoldingPickAxe())
		return false;
	
	return m_GraphicThingInstance.CanAttack();
}



bool CInstanceBase::CanFishing()
{
	return m_GraphicThingInstance.CanFishing();
}


BOOL CInstanceBase::IsBowMode()
{
	return m_GraphicThingInstance.IsBowMode();
}

BOOL CInstanceBase::IsHandMode()
{
	return m_GraphicThingInstance.IsHandMode();
}

BOOL CInstanceBase::IsFishingMode()
{
	if (CRaceMotionData::MODE_FISHING == m_GraphicThingInstance.GetMotionMode())
		return true;

	return false;
}

BOOL CInstanceBase::IsFishing()
{
	return m_GraphicThingInstance.IsFishing();
}

BOOL CInstanceBase::IsDead()
{
	return m_GraphicThingInstance.IsDead();
}

BOOL CInstanceBase::IsStun()
{
	return m_GraphicThingInstance.IsStun();
}

BOOL CInstanceBase::IsSleep()
{
	return m_GraphicThingInstance.IsSleep();
}


BOOL CInstanceBase::__IsSyncing()
{
	return m_GraphicThingInstance.__IsSyncing();
}



void CInstanceBase::NEW_SetOwner(DWORD dwVIDOwner)
{
	m_GraphicThingInstance.SetOwner(dwVIDOwner);
}

float CInstanceBase::GetLocalTime()
{
	return m_GraphicThingInstance.GetLocalTime();
}


void CInstanceBase::PushUDPState(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg)
{
}

DWORD	ELTimer_GetServerFrameMSec();

void CInstanceBase::PushTCPStateExpanded(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg, UINT uTargetVID)
{
	SCommand kCmdNew;
	kCmdNew.m_kPPosDst = c_rkPPosDst;
	kCmdNew.m_dwChkTime = dwCmdTime+100;
	kCmdNew.m_dwCmdTime = dwCmdTime;
	kCmdNew.m_fDstRot = fDstRot;
	kCmdNew.m_eFunc = eFunc;
	kCmdNew.m_uArg = uArg;
	kCmdNew.m_uTargetVID = uTargetVID;
	m_kQue_kCmdNew.push_back(kCmdNew);
}

void CInstanceBase::PushTCPState(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg)
{	
	if (__IsMainInstance())
	{
		//assert(!"CInstanceBase::PushTCPState ÇÃ·¹ÀÌ¾î ÀÚ½Å¿¡°Ô ÀÌµ¿ÆÐÅ¶Àº ¿À¸é ¾ÈµÈ´Ù!");
		TraceError("CInstanceBase::PushTCPState ÇÃ·¹ÀÌ¾î ÀÚ½Å¿¡°Ô ÀÌµ¿ÆÐÅ¶Àº ¿À¸é ¾ÈµÈ´Ù!");
		return;
	}

	int nNetworkGap=ELTimer_GetServerFrameMSec()-dwCmdTime;
	
	m_nAverageNetworkGap=(m_nAverageNetworkGap*70+nNetworkGap*30)/100;
	
	/*
	if (m_dwBaseCmdTime == 0)
	{
		m_dwBaseChkTime = ELTimer_GetFrameMSec()-nNetworkGap;
		m_dwBaseCmdTime = dwCmdTime;

		Tracenf("VID[%d] ³×Æ®¿÷°¸ [%d]", GetVirtualID(), nNetworkGap);
	}
	*/

	//m_dwBaseChkTime-m_dwBaseCmdTime+ELTimer_GetServerMSec();

	SCommand kCmdNew;
	kCmdNew.m_kPPosDst = c_rkPPosDst;
	kCmdNew.m_dwChkTime = dwCmdTime+m_nAverageNetworkGap;//m_dwBaseChkTime + (dwCmdTime - m_dwBaseCmdTime);// + nNetworkGap;
	kCmdNew.m_dwCmdTime = dwCmdTime;
	kCmdNew.m_fDstRot = fDstRot;
	kCmdNew.m_eFunc = eFunc;
	kCmdNew.m_uArg = uArg;
	m_kQue_kCmdNew.push_back(kCmdNew);

	//int nApplyGap=kCmdNew.m_dwChkTime-ELTimer_GetServerFrameMSec();

	//if (nApplyGap<-500 || nApplyGap>500)
	//	Tracenf("VID[%d] NAME[%s] ³×Æ®¿÷°¸ [cur:%d ave:%d] ÀÛµ¿½Ã°£ (%d)", GetVirtualID(), GetNameString(), nNetworkGap, m_nAverageNetworkGap, nApplyGap);
}

/*
CInstanceBase::TStateQueue::iterator CInstanceBase::FindSameState(TStateQueue& rkQuekStt, DWORD dwCmdTime, UINT eFunc, UINT uArg)
{
	TStateQueue::iterator i=rkQuekStt.begin();
	while (rkQuekStt.end()!=i)
	{
		SState& rkSttEach=*i;
		if (rkSttEach.m_dwCmdTime==dwCmdTime)
			if (rkSttEach.m_eFunc==eFunc)
				if (rkSttEach.m_uArg==uArg)
					break;
		++i;
	}

	return i;
}
*/

BOOL CInstanceBase::__CanProcessNetworkStatePacket()
{
	if (m_GraphicThingInstance.IsDead())
		return FALSE;
	if (m_GraphicThingInstance.IsKnockDown())
		return FALSE;
	if (m_GraphicThingInstance.IsUsingSkill())
		if (!m_GraphicThingInstance.CanCancelSkill())
			return FALSE;

	return TRUE;
}

BOOL CInstanceBase::__IsEnableTCPProcess(UINT eCurFunc)
{
	if (m_GraphicThingInstance.IsActEmotion())
	{
		return FALSE;
	}

	if (!m_bEnableTCPState)
	{
		if (FUNC_EMOTION != eCurFunc)
		{
			return FALSE;
		}
	}

	return TRUE;
}

void CInstanceBase::StateProcess()
{	
	while (1)
	{
		if (m_kQue_kCmdNew.empty())
			return;	

		DWORD dwDstChkTime = m_kQue_kCmdNew.front().m_dwChkTime;
		DWORD dwCurChkTime = ELTimer_GetServerFrameMSec();	

		if (dwCurChkTime < dwDstChkTime)
			return;

		SCommand kCmdTop = m_kQue_kCmdNew.front();
		m_kQue_kCmdNew.pop_front();	

		TPixelPosition kPPosDst = kCmdTop.m_kPPosDst;
		//DWORD dwCmdTime = kCmdTop.m_dwCmdTime;	
		FLOAT fRotDst = kCmdTop.m_fDstRot;
		UINT eFunc = kCmdTop.m_eFunc;
		UINT uArg = kCmdTop.m_uArg;
		UINT uVID = GetVirtualID();	
		UINT uTargetVID = kCmdTop.m_uTargetVID;

		TPixelPosition kPPosCur;
		NEW_GetPixelPosition(&kPPosCur);

		/*
		if (IsPC())
			Tracenf("%d cmd: vid=%d[%s] func=%d arg=%d  curPos=(%f, %f) dstPos=(%f, %f) rot=%f (time %d)", 
			ELTimer_GetMSec(),
			uVID, m_stName.c_str(), eFunc, uArg, 
			kPPosCur.x, kPPosCur.y,
			kPPosDst.x, kPPosDst.y, fRotDst, dwCmdTime-m_dwBaseCmdTime);
		*/

		TPixelPosition kPPosDir = kPPosDst - kPPosCur;
		float fDirLen = (float)sqrt(kPPosDir.x * kPPosDir.x + kPPosDir.y * kPPosDir.y);

		//Tracenf("°Å¸® %f", fDirLen);

		if (!__CanProcessNetworkStatePacket())
		{
			Lognf(0, "vid=%d ¿òÁ÷ÀÏ ¼ö ¾ø´Â »óÅÂ¶ó ½ºÅµ IsDead=%d, IsKnockDown=%d", uVID, m_GraphicThingInstance.IsDead(), m_GraphicThingInstance.IsKnockDown());
			return;
		}

		if (!__IsEnableTCPProcess(eFunc))
		{
			return;
		}

		switch (eFunc)
		{
			case FUNC_WAIT:
			{
				//Tracenf("%s (%f, %f) -> (%f, %f) ³²Àº°Å¸® %f", GetNameString(), kPPosCur.x, kPPosCur.y, kPPosDst.x, kPPosDst.y, fDirLen);
				if (fDirLen > 1.0f)
				{
					//NEW_GetSrcPixelPositionRef() = kPPosCur;
					//NEW_GetDstPixelPositionRef() = kPPosDst;
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);

					__EnableSkipCollision();

					m_fDstRot = fRotDst;
					m_isGoing = TRUE;

					m_kMovAfterFunc.eFunc = FUNC_WAIT;

					if (!IsWalking())
						StartWalking();

					//Tracen("¸ñÇ¥Á¤Áö");
				}
				else
				{
					//Tracen("ÇöÀç Á¤Áö");

					m_isGoing = FALSE;

					if (!IsWaiting())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					SetAdvancingRotation(fRotDst);
					SetRotation(fRotDst);
				}
				break;
			}

			case FUNC_MOVE:
			{
				//NEW_GetSrcPixelPositionRef() = kPPosCur;
				//NEW_GetDstPixelPositionRef() = kPPosDst;
				NEW_SetSrcPixelPosition(kPPosCur);
				NEW_SetDstPixelPosition(kPPosDst);
				m_fDstRot = fRotDst;
				m_isGoing = TRUE;
				__EnableSkipCollision();
				//m_isSyncMov = TRUE;

				m_kMovAfterFunc.eFunc = FUNC_MOVE;

				if (!IsWalking())
				{
					//Tracen("°È°í ÀÖÁö ¾Ê¾Æ °È±â ½ÃÀÛ");
					StartWalking();
				}
				else
				{
					//Tracen("ÀÌ¹Ì °È´ÂÁß ");
				}
				break;
			}

			case FUNC_COMBO:
			{
				if (fDirLen >= 50.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot=fRotDst;
					m_isGoing = TRUE;
					__EnableSkipCollision();

					m_kMovAfterFunc.eFunc = FUNC_COMBO;
					m_kMovAfterFunc.uArg = uArg;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					//Tracen("´ë±â °ø°Ý Á¤Áö");

					m_isGoing = FALSE;

					if (IsWalking())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					RunComboAttack(fRotDst, uArg);
				}
				break;
			}

			case FUNC_ATTACK:
			{
				if (fDirLen>=50.0f)
				{
					//NEW_GetSrcPixelPositionRef() = kPPosCur;
					//NEW_GetDstPixelPositionRef() = kPPosDst;
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;
					__EnableSkipCollision();
					//m_isSyncMov = TRUE;

					m_kMovAfterFunc.eFunc = FUNC_ATTACK;

					if (!IsWalking())
						StartWalking();

					//Tracen("³Ê¹« ¸Ö¾î¼­ ÀÌµ¿ ÈÄ °ø°Ý");
				}
				else
				{
					//Tracen("³ë¸» °ø°Ý Á¤Áö");

					m_isGoing = FALSE;

					if (IsWalking())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					BlendRotation(fRotDst);

					RunNormalAttack(fRotDst);

					//Tracen("°¡±õ±â ¶§¹®¿¡ ¿öÇÁ °ø°Ý");
				}
				break;
			}

			case FUNC_MOB_SKILL:
			{
				if (fDirLen >= 50.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;
					__EnableSkipCollision();

					m_kMovAfterFunc.eFunc = FUNC_MOB_SKILL;
					m_kMovAfterFunc.uArg = uArg;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					m_isGoing = FALSE;

					if (IsWalking())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					BlendRotation(fRotDst);

					m_GraphicThingInstance.InterceptOnceMotion(CRaceMotionData::NAME_SPECIAL_1 + uArg);
				}
				break;
			}

			case FUNC_EMOTION:
			{
				if (fDirLen>100.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;

					if (__IsMainInstance())
						__EnableSkipCollision();

					m_kMovAfterFunc.eFunc = FUNC_EMOTION;
					m_kMovAfterFunc.uArg = uArg;
					m_kMovAfterFunc.uArgExpanded = uTargetVID;
					m_kMovAfterFunc.kPosDst = kPPosDst;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					__ProcessFunctionEmotion(uArg, uTargetVID, kPPosDst);
				}
				break;
			}

			default:
			{
				if (eFunc & FUNC_SKILL)
				{
					if (fDirLen >= 50.0f)
					{
						//NEW_GetSrcPixelPositionRef() = kPPosCur;
						//NEW_GetDstPixelPositionRef() = kPPosDst;
						NEW_SetSrcPixelPosition(kPPosCur);
						NEW_SetDstPixelPosition(kPPosDst);
						m_fDstRot = fRotDst;
						m_isGoing = TRUE;
						//m_isSyncMov = TRUE;
						__EnableSkipCollision();

						m_kMovAfterFunc.eFunc = eFunc;
						m_kMovAfterFunc.uArg = uArg;

						if (!IsWalking())
							StartWalking();

						//Tracen("³Ê¹« ¸Ö¾î¼­ ÀÌµ¿ ÈÄ °ø°Ý");
					}
					else
					{
						//Tracen("½ºÅ³ Á¤Áö");

						m_isGoing = FALSE;

						if (IsWalking())
							EndWalking();

						SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
						SetAdvancingRotation(fRotDst);
						SetRotation(fRotDst);

						NEW_UseSkill(0, eFunc & 0x7f, uArg&0x0f, (uArg>>4) ? true : false);
						//Tracen("°¡±õ±â ¶§¹®¿¡ ¿öÇÁ °ø°Ý");
					}
				}
				break;
			}
		}
	}
}


void CInstanceBase::MovementProcess()
{
	TPixelPosition kPPosCur;
	NEW_GetPixelPosition(&kPPosCur);

	// ·»´õ¸µ ÁÂÇ¥°èÀÌ¹Ç·Î y¸¦ -È­ÇØ¼­ ´õÇÑ´Ù.

	TPixelPosition kPPosNext;
	{
		const D3DXVECTOR3 & c_rkV3Mov = m_GraphicThingInstance.GetMovementVectorRef();

		kPPosNext.x = kPPosCur.x + (+c_rkV3Mov.x);
		kPPosNext.y = kPPosCur.y + (-c_rkV3Mov.y);
		kPPosNext.z = kPPosCur.z + (+c_rkV3Mov.z);
	}

	TPixelPosition kPPosDeltaSC = kPPosCur - NEW_GetSrcPixelPositionRef();
	TPixelPosition kPPosDeltaSN = kPPosNext - NEW_GetSrcPixelPositionRef();
	TPixelPosition kPPosDeltaSD = NEW_GetDstPixelPositionRef() - NEW_GetSrcPixelPositionRef();

	float fCurLen = sqrtf(kPPosDeltaSC.x * kPPosDeltaSC.x + kPPosDeltaSC.y * kPPosDeltaSC.y);
	float fNextLen = sqrtf(kPPosDeltaSN.x * kPPosDeltaSN.x + kPPosDeltaSN.y * kPPosDeltaSN.y);
	float fTotalLen = sqrtf(kPPosDeltaSD.x * kPPosDeltaSD.x + kPPosDeltaSD.y * kPPosDeltaSD.y);
	float fRestLen = fTotalLen - fCurLen;

	if (__IsMainInstance())
	{
		if (m_isGoing && IsWalking())
		{
			float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(NEW_GetSrcPixelPositionRef(), NEW_GetDstPixelPositionRef());

			SetAdvancingRotation(fDstRot);

			if (fRestLen<=0.0)
			{
				if (IsWalking())
					EndWalking();

				//Tracen("¸ñÇ¥ µµ´Þ Á¤Áö");

				m_isGoing = FALSE;

				BlockMovement();

				if (FUNC_EMOTION == m_kMovAfterFunc.eFunc)
				{
					DWORD dwMotionNumber = m_kMovAfterFunc.uArg;
					DWORD dwTargetVID = m_kMovAfterFunc.uArgExpanded;
					__ProcessFunctionEmotion(dwMotionNumber, dwTargetVID, m_kMovAfterFunc.kPosDst);
					m_kMovAfterFunc.eFunc = FUNC_WAIT;
					return;
				}
			}
		}
	}
	else
	{
		if (m_isGoing && IsWalking())
		{
			float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(NEW_GetSrcPixelPositionRef(), NEW_GetDstPixelPositionRef());

			SetAdvancingRotation(fDstRot);

			// ¸¸¾à ·»ÅÏ½Ã°¡ ´Ê¾î ³Ê¹« ¸¹ÀÌ ÀÌµ¿Çß´Ù¸é..
			if (fRestLen < -100.0f)
			{
				NEW_SetSrcPixelPosition(kPPosCur);

				float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(kPPosCur, NEW_GetDstPixelPositionRef());
				SetAdvancingRotation(fDstRot);
				//Tracenf("VID %d ¿À¹ö ¹æÇâ¼³Á¤ (%f, %f) %f rest %f", GetVirtualID(), kPPosCur.x, kPPosCur.y, fDstRot, fRestLen);			

				// ÀÌµ¿ÁßÀÌ¶ó¸é ´ÙÀ½¹ø¿¡ ¸ØÃß°Ô ÇÑ´Ù
				if (FUNC_MOVE == m_kMovAfterFunc.eFunc)
				{
					m_kMovAfterFunc.eFunc = FUNC_WAIT;
				}
			}
			// µµÂøÇß´Ù¸é...
			else if (fCurLen <= fTotalLen && fTotalLen <= fNextLen)
			{
				if (m_GraphicThingInstance.IsDead() || m_GraphicThingInstance.IsKnockDown())
				{
					__DisableSkipCollision();

					//Tracen("»ç¸Á »óÅÂ¶ó µ¿ÀÛ ½ºÅµ");

					m_isGoing = FALSE;

					//Tracen("Çàµ¿ ºÒ´É »óÅÂ¶ó ÀÌÈÄ µ¿ÀÛ ½ºÅµ");
				}
				else
				{
					switch (m_kMovAfterFunc.eFunc)
					{
						case FUNC_ATTACK:
						{
							if (IsWalking())
								EndWalking();

							__DisableSkipCollision();
							m_isGoing = FALSE;

							BlockMovement();
							SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
							SetAdvancingRotation(m_fDstRot);
							SetRotation(m_fDstRot);

							RunNormalAttack(m_fDstRot);
							break;
						}

						case FUNC_COMBO:
						{
							if (IsWalking())
								EndWalking();

							__DisableSkipCollision();
							m_isGoing = FALSE;

							BlockMovement();
							SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
							RunComboAttack(m_fDstRot, m_kMovAfterFunc.uArg);
							break;
						}

						case FUNC_EMOTION:
						{
							m_isGoing = FALSE;
							m_kMovAfterFunc.eFunc = FUNC_WAIT;
							__DisableSkipCollision();
							BlockMovement();

							DWORD dwMotionNumber = m_kMovAfterFunc.uArg;
							DWORD dwTargetVID = m_kMovAfterFunc.uArgExpanded;
							__ProcessFunctionEmotion(dwMotionNumber, dwTargetVID, m_kMovAfterFunc.kPosDst);
							break;
						}

						case FUNC_MOVE:
						{
							break;
						}

						case FUNC_MOB_SKILL:
						{
							if (IsWalking())
								EndWalking();

							__DisableSkipCollision();
							m_isGoing = FALSE;

							BlockMovement();
							SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
							SetAdvancingRotation(m_fDstRot);
							SetRotation(m_fDstRot);

							m_GraphicThingInstance.InterceptOnceMotion(CRaceMotionData::NAME_SPECIAL_1 + m_kMovAfterFunc.uArg);
							break;
						}

						default:
						{
							if (m_kMovAfterFunc.eFunc & FUNC_SKILL)
							{
								SetAdvancingRotation(m_fDstRot);
								BlendRotation(m_fDstRot);
								NEW_UseSkill(0, m_kMovAfterFunc.eFunc & 0x7f, m_kMovAfterFunc.uArg&0x0f, (m_kMovAfterFunc.uArg>>4) ? true : false);
							}
							else
							{
								//Tracenf("VID %d ½ºÅ³ °ø°Ý (%f, %f) rot %f", GetVirtualID(), NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y, m_fDstRot);

								__DisableSkipCollision();
								m_isGoing = FALSE;

								BlockMovement();
								SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
								SetAdvancingRotation(m_fDstRot);
								BlendRotation(m_fDstRot);
								if (!IsWaiting())
								{
									EndWalking();
								}

								//Tracenf("VID %d Á¤Áö (%f, %f) rot %f IsWalking %d", GetVirtualID(), NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y, m_fDstRot, IsWalking());
							}
							break;
						}
					}

				}
			}

		}
	}

	if (IsWalking() || m_GraphicThingInstance.IsUsingMovingSkill())
	{
		float fRotation = m_GraphicThingInstance.GetRotation();
		float fAdvancingRotation = m_GraphicThingInstance.GetAdvancingRotation();
		int iDirection = GetRotatingDirection(fRotation, fAdvancingRotation);

		if (DEGREE_DIRECTION_SAME != m_iRotatingDirection)
		{
			if (DEGREE_DIRECTION_LEFT == iDirection)
			{
				fRotation = fmodf(fRotation + m_fRotSpd*m_GraphicThingInstance.GetSecondElapsed(), 360.0f);
			}
			else if (DEGREE_DIRECTION_RIGHT == iDirection)
			{
				fRotation = fmodf(fRotation - m_fRotSpd*m_GraphicThingInstance.GetSecondElapsed() + 360.0f, 360.0f);
			}

			if (m_iRotatingDirection != GetRotatingDirection(fRotation, fAdvancingRotation))
			{
				m_iRotatingDirection = DEGREE_DIRECTION_SAME;
				fRotation = fAdvancingRotation;
			}

			m_GraphicThingInstance.SetRotation(fRotation);
		}

		if (__IsInDustRange())
		{ 
			float fDustDistance = NEW_GetDistanceFromDestPixelPosition(m_kPPosDust);
			if (IsMountingHorse())
			{
				if (fDustDistance > ms_fHorseDustGap)
				{
					NEW_GetPixelPosition(&m_kPPosDust);
					__AttachEffect(EFFECT_HORSE_DUST);
				}
			}
			else
			{
				if (fDustDistance > ms_fDustGap)
				{
					NEW_GetPixelPosition(&m_kPPosDust);
					__AttachEffect(EFFECT_DUST);
				}
			}
		}
	}
}

void CInstanceBase::__ProcessFunctionEmotion(DWORD dwMotionNumber, DWORD dwTargetVID, const TPixelPosition & c_rkPosDst)
{
	if (IsWalking())
		EndWalkingWithoutBlending();

	__EnableChangingTCPState();
	SCRIPT_SetPixelPosition(c_rkPosDst.x, c_rkPosDst.y);

	CInstanceBase * pTargetInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwTargetVID);
	if (pTargetInstance)
	{
		pTargetInstance->__EnableChangingTCPState();

		if (pTargetInstance->IsWalking())
			pTargetInstance->EndWalkingWithoutBlending();

		WORD wMotionNumber1 = HIWORD(dwMotionNumber);
		WORD wMotionNumber2 = LOWORD(dwMotionNumber);

		int src_job = RaceToJob(GetRace());
		int dst_job = RaceToJob(pTargetInstance->GetRace());

		NEW_LookAtDestInstance(*pTargetInstance);
		m_GraphicThingInstance.InterceptOnceMotion(wMotionNumber1 + dst_job);
		m_GraphicThingInstance.SetRotation(m_GraphicThingInstance.GetTargetRotation());
		m_GraphicThingInstance.SetAdvancingRotation(m_GraphicThingInstance.GetTargetRotation());

		pTargetInstance->NEW_LookAtDestInstance(*this);
		pTargetInstance->m_GraphicThingInstance.InterceptOnceMotion(wMotionNumber2 + src_job);
		pTargetInstance->m_GraphicThingInstance.SetRotation(pTargetInstance->m_GraphicThingInstance.GetTargetRotation());
		pTargetInstance->m_GraphicThingInstance.SetAdvancingRotation(pTargetInstance->m_GraphicThingInstance.GetTargetRotation());

		if (pTargetInstance->__IsMainInstance())
		{
			IAbstractPlayer & rPlayer=IAbstractPlayer::GetSingleton();
			rPlayer.EndEmotionProcess();
		}
	}

	if (__IsMainInstance())
	{
		IAbstractPlayer & rPlayer=IAbstractPlayer::GetSingleton();
		rPlayer.EndEmotionProcess();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Update & Deform & Render

int g_iAccumulationTime = 0;

#ifdef ENABLE_VOL_SYSTEM // Si le système de vol est activé.

float CInstanceBase::GetHauteurVol(std::string pseudo)
{
	for (int i = 0; i < ListePseudoVol.size(); i++)
	{
		if (ListePseudoVol[i] == pseudo)
		{
			return stof(ListeHauteurVol[i]);
		}
	}

	return 0;
}


void CInstanceBase::SetHauteurVol(std::string pseudo, float valeur)
{
	for (int i = 0; i < ListePseudoVol.size(); i++)
	{
		if (ListePseudoVol[i] == pseudo)
		{
			ostringstream oss;
			oss << valeur;
			ListeHauteurVol[i] = oss.str();
			oss.clear();
		}
	}
}

void CInstanceBase::SetIsVol(std::string pseudo, std::string statut)
{
	for (int i = 0; i < ListePseudoVol.size(); i++)
	{
		if (ListePseudoVol[i] == pseudo)
		{
			ListeStatutVol[i] = statut;
		}
	}
}

bool CInstanceBase::GetIsVol(std::string pseudo)
{
	for (int i = 0; i < ListePseudoVol.size(); i++)
	{
		if (ListePseudoVol[i] == pseudo)
		{
			if (ListeStatutVol[i] == "On")
			{
				return true;
			}
		}
	}
	return false;
}

float CInstanceBase::GetHauteurObjet(TPixelPosition kPPosCur1)
{
	if (CPythonBackground::Instance().m_pkMap != NULL)
	{
		CMapOutdoor& rkMap = CPythonBackground::Instance().GetMapOutdoorRef();
		return rkMap.GetObjectHeight(kPPosCur1.x, kPPosCur1.y);
	}
	else
	{
		return 0.0f;
	}
}

int CInstanceBase::CreationAileVol(float x, float y, float z)
{
	CInstanceBase::SCreateData kCreateData;
	kCreateData.m_bType = CActorInstance::TYPE_OBJECT;
	kCreateData.m_dwLevel = 0;
	kCreateData.m_dw7emeSkill = 0;
	kCreateData.m_dw8emeSkill = 0;
	kCreateData.m_dwGuildID = 0;
	kCreateData.m_dwEmpireID = 0;
	kCreateData.m_dwMountVnum = 0;
	kCreateData.m_dwRace = 0;
	kCreateData.m_fRot = DIR_NORTH;
	kCreateData.m_lPosX = x;
	kCreateData.m_lPosY = y;
	kCreateData.m_stName = "";
	kCreateData.m_dwStateFlags = 0;
	kCreateData.m_dwMovSpd = m_GraphicThingInstance.m_fMovSpd;
	kCreateData.m_dwAtkSpd = m_GraphicThingInstance.m_fAtkSpd;
	kCreateData.m_sAlignment = 0;
	kCreateData.m_byPKMode = 0;
	kCreateData.m_kAffectFlags.Clear();
	kCreateData.m_dwArmor = 0;
	kCreateData.m_dwWeapon = 0;
	kCreateData.m_dwHair = 0;
	kCreateData.m_dwAcce = 0;
	kCreateData.m_dwRace = 99;
	srand(time(NULL));
	int randomVID = rand() % 100000000 + 1;
	while (CPythonCharacterManager::Instance().CheckInstanceVID(randomVID) == false) // Generate a new VID if already exist until he not exist .
	{
		randomVID = rand() % 100000000 + 1;
	}
	kCreateData.m_dwVID = randomVID;

	kCreateData.m_isMain = false;
	CInstanceBase* aile = CPythonCharacterManager::Instance().CreateInstance(kCreateData);
	CPythonCharacterManager::Instance().RegisterInstance(aile->GetVirtualID());
	TPixelPosition kPPosCur1(x, y, z);

	aile->NEW_SetPixelPosition(kPPosCur1); // Change la position du personnage.

	return aile->GetVirtualID();
}

bool CInstanceBase::GetHauteurEauVol(TPixelPosition kPPosCur1, float * z)
{
	bool ok = false;

	if (CPythonBackground::Instance().isAttrOn(kPPosCur1.x, kPPosCur1.y, CTerrainImpl::ATTRIBUTE_WATER))
	{
		const D3DXVECTOR3 & rv3Position = D3DXVECTOR3(kPPosCur1.x, kPPosCur1.y, kPPosCur1.z);
		int incrementX = 0;
		for (int i = 0; i < 1000; i++) // Check X
		{
			if (z <= 0)
			{
				incrementX += 10;
				float x = rv3Position.x + incrementX;
				float y = rv3Position.y;
				long eau;
				if (CPythonBackground::Instance().GetWaterHeight(x, y, &eau))
				{
					if (z <= 0 && ok == false && eau > 0)
					{
						z = (float*)eau;
						ok = true;
					}
				}
			}

		}
		if (z <= 0 && ok == false)
		{
			int incrementY = 0;
			for (int i = 0; i < 1000; i++) // Check Y
			{
				incrementY += 10;
				float x = rv3Position.x;
				float y = rv3Position.y + incrementY;
				long eau;
				if (CPythonBackground::Instance().GetWaterHeight(x, y, &eau))
				{
					if (z <= 0 && ok == false && eau > 0)
					{
						z = (float*)eau;
						ok = true;
						break;
					}
				}
			}
		}
		if (z <= 0 && ok == false)
		{
			int incrementALL = 0;
			for (int i = 0; i < 1000; i++)  // Check X Y
			{
				incrementALL += 10;
				float x = rv3Position.x + incrementALL;
				float y = rv3Position.y + incrementALL;
				long eau;
				if (CPythonBackground::Instance().GetWaterHeight(x, y, &eau))
				{
					if (z <= 0 && ok == false && eau > 0)
					{
						z = (float*)eau;
						ok = true;
						break;
					}
				}
			}
		}

		// Negatif Check

		if (z <= 0 && ok == false)
		{
			incrementX = 0;
			for (int i = 0; i < 1000; i++) // Check X
			{
				if (z <= 0)
				{
					incrementX += 10;
					float x = rv3Position.x - incrementX;
					float y = rv3Position.y;
					long eau;
					if (CPythonBackground::Instance().GetWaterHeight(x, y, &eau))
					{
						if (z <= 0 && ok == false && eau > 0)
						{
							z = (float*)eau;
							ok = true;
						}
					}
				}

			}
		}
		if (z <= 0 && ok == false)
		{
			int incrementY = 0;
			for (int i = 0; i < 1000; i++) // Check Y
			{
				incrementY += 10;
				float x = rv3Position.x;
				float y = rv3Position.y - incrementY;
				long eau;
				if (CPythonBackground::Instance().GetWaterHeight(x, y, &eau))
				{
					if (z <= 0 && ok == false && eau > 0)
					{
						z = (float*)eau;
						ok = true;
						break;
					}
				}
			}
		}
		if (z <= 0 && ok == false)
		{
			int incrementALL = 0;
			for (int i = 0; i < 1000; i++)  // Check X Y
			{
				incrementALL += 10;
				float x = rv3Position.x - incrementALL;
				float y = rv3Position.y - incrementALL;
				long eau;
				if (CPythonBackground::Instance().GetWaterHeight(x, y, &eau))
				{
					if (z <= 0 && ok == false && eau > 0)
					{
						z = (float*)eau;
						ok = true;
						break;
					}
				}
			}
		}
	}
	return ok;
}


void CInstanceBase::UpdateVol()
{
	if (IsPC() && m_dwLevel > 0) // Si c'est un joueur.
	{
		CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();

		float limiteHauteur = 30.0f;
		float limiteHauteurJump = 20.0f;

		TPixelPosition kPPosCur1 = m_GraphicThingInstance.NEW_GetCurPixelPositionRef();

		float HauteurZjoueur = kPPosCur1.z; /// Récupère la hauteur du joueur 
		float HauteurZsol = CPythonBackground::Instance().GetTerrainHeight(kPPosCur1.x, kPPosCur1.y); // Récupère la hauteur du sol
		float HauteurObjet = GetHauteurObjet(kPPosCur1);

		if (HauteurObjet > 0)
		{
			ostringstream oss;
			oss << "/hauteur s ";
			oss << HauteurObjet;
			rkNetStream.SendChatPacket(oss.str().c_str());
			oss.clear();
			oss << "/hauteur o on";
			rkNetStream.SendChatPacket(oss.str().c_str());

		
			m_GraphicThingInstance.m_isOnBuilding = true;

		}
		else
		{
			ostringstream oss;
			oss << "/hauteur s ";
			oss << HauteurZsol;
			rkNetStream.SendChatPacket(oss.str().c_str());
			m_GraphicThingInstance.m_isOnBuilding = false;
		}
#ifdef ENABLE_NAGE_SYSTEM
		if (m_GraphicThingInstance.m_isNage == false)
		{
#endif
			if (IsPC())
			{
				if (m_GraphicThingInstance.m_isVol == false)
				{
					if (__IsMainInstance())
					{
						ostringstream oss;
						oss << "/hauteur v ";
						oss << kPPosCur1.z + 10;
						rkNetStream.SendChatPacket(oss.str().c_str());
						oss.clear();
					}
					if (GetIsVol(GetNameString()))
					{
						m_GraphicThingInstance.m_isTombe = false;
						m_GraphicThingInstance.m_isNage = false;
						kPPosCur1.z = GetHauteurVol(GetNameString());
						NEW_SetPixelPosition(kPPosCur1);
						m_GraphicThingInstance.m_isVol = true;
						m_GraphicThingInstance.hauteurVolZ = kPPosCur1.z;
					}
					else
					{
						if (m_GraphicThingInstance.m_isNage == false && m_GraphicThingInstance.m_isTombe == false)
						{
							m_GraphicThingInstance.hauteurVolZ = HauteurZsol;
						}
						m_GraphicThingInstance.m_isVol = false;
					}
				}
			}
#ifdef ENABLE_NAGE_SYSTEM
		}
#endif
		if (CRaceMotionData::NAME_VOL_START_TOMBER != GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey) && m_GraphicThingInstance.m_isJump)// Désactive le jump manuellement lancé.
		{
			m_GraphicThingInstance.m_isJump = false;
		}
		if (m_GraphicThingInstance.m_isVol) // Si le joueur vol.
		{
			
			if (GetIsVol(GetNameString()) == false)
			{
				m_GraphicThingInstance.m_isVol = false;
				if (m_GraphicThingInstance.m_isOnBuilding && kPPosCur1.z > HauteurObjet)
				{
					m_GraphicThingInstance.m_isTombe = true; // le joueur tombe.
					m_GraphicThingInstance.compteurTomber = 21;
				}
				else if (kPPosCur1.z > HauteurZsol)
				{
					m_GraphicThingInstance.m_isTombe = true; // le joueur tombe.
					m_GraphicThingInstance.compteurTomber = 21;
				}
				else // Check Eau.
				{
					float hauteurEau;
					bool OnEau = GetHauteurEauVol(kPPosCur1, &hauteurEau);
					if (OnEau)
					{
						if (kPPosCur1.z > hauteurEau)
						{
							m_GraphicThingInstance.m_isTombe = true; // le joueur tombe.
							m_GraphicThingInstance.compteurTomber = 21;
						}
					}
				}
				
			}
			else
			{
				if (__IsMainInstance())
				{
					if (m_GraphicThingInstance.IsMoving())
					{
						CCamera* camera = CCameraManager::Instance().GetCurrentCamera();
						float degre = camera->GetPitch();
						float rotation = camera->GetRoll();
						//TraceError("Degrée de camera: %f", degre);
						if (degre > 30)
						{

							m_GraphicThingInstance.hauteurVolZ = m_GraphicThingInstance.hauteurVolZ - 7.5f;
							SetHauteurVol(GetNameString(), m_GraphicThingInstance.hauteurVolZ);
							ostringstream oss;
							oss << "/hauteur v ";
							oss << m_GraphicThingInstance.hauteurVolZ;
							rkNetStream.SendChatPacket(oss.str().c_str());
							kPPosCur1.z = m_GraphicThingInstance.hauteurVolZ;
							NEW_SetPixelPosition(kPPosCur1);
							m_GraphicThingInstance.m_rotZ = degre;
						}
						else if (degre < 0)
						{
							m_GraphicThingInstance.hauteurVolZ = m_GraphicThingInstance.hauteurVolZ + 7.5f;
							SetHauteurVol(GetNameString(), m_GraphicThingInstance.hauteurVolZ);
							int iType = CHAT_TYPE_TALKING;
							ostringstream oss;
							oss << "/hauteur v ";
							oss << m_GraphicThingInstance.hauteurVolZ;
							rkNetStream.SendChatPacket(oss.str().c_str());
							kPPosCur1.z = m_GraphicThingInstance.hauteurVolZ;
							NEW_SetPixelPosition(kPPosCur1);
							m_GraphicThingInstance.m_rotZ = degre;
						}
						else
						{
							m_GraphicThingInstance.m_rotZ = 0;
						}
					}
					else
					{
						m_GraphicThingInstance.m_rotZ = 0;
					}
				}
				else
				{

					float hauteurTmp = GetHauteurVol(GetNameString());

					if (m_GraphicThingInstance.hauteurVolZ != hauteurTmp)
					{
						kPPosCur1.z = hauteurTmp;
						m_GraphicThingInstance.hauteurVolZ = hauteurTmp;
						NEW_SetPixelPosition(kPPosCur1);
					}
					else
					{
						kPPosCur1.z = m_GraphicThingInstance.hauteurVolZ;
						NEW_SetPixelPosition(kPPosCur1);
					}
				}

				if (__IsMainInstance())
				{

					if (kPPosCur1.z <= HauteurZsol)
					{
						m_GraphicThingInstance.m_isVol = false;
						SetIsVol(GetNameString(), "Off");
						ostringstream oss;
						oss << "/hauteur v ";
						oss << HauteurZsol;
						rkNetStream.SendChatPacket("/fly o");
						rkNetStream.SendChatPacket(oss.str().c_str());
						m_GraphicThingInstance.hauteurVolZ = HauteurZsol;

						m_GraphicThingInstance.m_isVol = false;

						SetIsVol(GetNameString(), "Off");
					}
					else if ((kPPosCur1.z+45.0f <= HauteurObjet && m_GraphicThingInstance.m_isOnBuilding))
					{
						m_GraphicThingInstance.m_isVol = false;
						SetIsVol(GetNameString(), "Off");
						ostringstream oss;
						oss << "/hauteur v ";
						oss << HauteurZsol;
						rkNetStream.SendChatPacket("/fly o");
						rkNetStream.SendChatPacket(oss.str().c_str());
						m_GraphicThingInstance.hauteurVolZ = HauteurObjet;

						m_GraphicThingInstance.m_isVol = false;

						SetIsVol(GetNameString(), "Off");
					}
					else // Vérifie que le joueur ce trouve sur l'eau.
					{
						float hauteurEau;
						bool OnEau = GetHauteurEauVol(kPPosCur1, &hauteurEau);
						if (OnEau)
						{
							if (kPPosCur1.z <= hauteurEau)
							{
								m_GraphicThingInstance.m_isVol = false;
								SetIsVol(GetNameString(), "Off");
								ostringstream oss;
								oss << "/hauteur v ";
								oss << HauteurZsol;
								rkNetStream.SendChatPacket("/fly o");
								rkNetStream.SendChatPacket(oss.str().c_str());
								m_GraphicThingInstance.hauteurVolZ = hauteurEau;

								m_GraphicThingInstance.m_isVol = false;

								SetIsVol(GetNameString(), "Off");
							}
						}
					}
				}
				else
				{
					if (GetIsVol(GetNameString()) == false)
					{
						m_GraphicThingInstance.m_isVol = false;
						if (kPPosCur1.z > HauteurZsol)
						{
							m_GraphicThingInstance.m_isTombe = true; // le joueur tombe.
							m_GraphicThingInstance.compteurTomber = 21;
						}
						else if ((kPPosCur1.z + 45.0f <= HauteurObjet && m_GraphicThingInstance.m_isOnBuilding))
						{
							m_GraphicThingInstance.m_isTombe = true; // le joueur tombe.
							m_GraphicThingInstance.compteurTomber = 21;
						}
						else // Vérifie que le joueur ce trouve sur l'eau.
						{
							float hauteurEau;
							bool OnEau = GetHauteurEauVol(kPPosCur1, &hauteurEau);
							if (OnEau)
							{
								if (kPPosCur1.z <= hauteurEau)
								{
									m_GraphicThingInstance.m_isTombe = true; // le joueur tombe.
									m_GraphicThingInstance.compteurTomber = 21;
								}
							}
						}
					}
				}

				if (m_GraphicThingInstance.m_isVol)
				{
					if (AileVolCreer == false)
					{

						m_GraphicThingInstance.m_AileInstanceVol = CreationAileVol(kPPosCur1.x, kPPosCur1.y, kPPosCur1.z);
						AileVolCreer = true;
						CPythonCharacterManager::Instance().UpdateAileVol(m_GraphicThingInstance.m_AileInstanceVol, kPPosCur1, false, true, m_GraphicThingInstance.m_fcurRotation);
					}
					
					if (m_GraphicThingInstance.IsMoving())
					{
						if (AileVolCreer)
						{
							AileVolCreer = CPythonCharacterManager::Instance().UpdateAileVol(m_GraphicThingInstance.m_AileInstanceVol, kPPosCur1, true, true, m_GraphicThingInstance.m_fcurRotation);
						}
						if (m_GraphicThingInstance.isAttacking() == false && m_GraphicThingInstance.IsActEmotion() == false)
						{
							if (CRaceMotionData::NAME_RUN == GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey)) // Si l'animation actuel est celle de tomber.
							{
								SetRotation(45.0f);
								SetLoopMotion(CRaceMotionData::NAME_VOL_MOVE, 0.15f, m_GraphicThingInstance.m_fMovSpd); // Activation de l'animation.
							}
						}
					}
					else
					{
						if (AileVolCreer)
						{
							AileVolCreer = CPythonCharacterManager::Instance().UpdateAileVol(m_GraphicThingInstance.m_AileInstanceVol, kPPosCur1, false, true, m_GraphicThingInstance.m_fcurRotation);
						}
						if (m_GraphicThingInstance.isAttacking() == false && m_GraphicThingInstance.IsActEmotion() == false)
						{

							if (CRaceMotionData::NAME_WAIT == GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey) || CRaceMotionData::NAME_VOL_TOMBER == GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey)) // Si l'animation actuel est celle de tomber.
							{
								SetRotation(90.0f);
								SetLoopMotion(CRaceMotionData::NAME_VOL_WAIT, 0.15f, m_GraphicThingInstance.m_fMovSpd); // Activation de l'animation.
							}
						}
					}
				}
				else
				{
					if (AileVolCreer)
					{

						CPythonCharacterManager::Instance().UpdateAileVol(m_GraphicThingInstance.m_AileInstanceVol, kPPosCur1, false, false, m_GraphicThingInstance.m_fcurRotation);
						CPythonCharacterManager::Instance().DeleteInstance(m_GraphicThingInstance.m_AileInstanceVol);
						AileVolCreer = false;
					}
					if (m_GraphicThingInstance.IsMoving())
					{

						if (CRaceMotionData::NAME_VOL_MOVE == GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey)) // Si l'animation actuel est celle de tomber.
						{
							SetLoopMotion(CRaceMotionData::NAME_RUN, 0.15f, m_GraphicThingInstance.m_fMovSpd); // Activation de l'animation.
						}

					}
					else
					{

						if (CRaceMotionData::NAME_VOL_WAIT == GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey)) // Si l'animation actuel est celle de tomber.
						{
							SetLoopMotion(CRaceMotionData::NAME_WAIT, 0.15f, m_GraphicThingInstance.m_fMovSpd); // Activation de l'animation.
						}

					}
				}
			}
		}


#ifdef ENABLE_NAGE_SYSTEM
		if (m_GraphicThingInstance.m_isNage == false) // Si le joueur ne nage pas.
		{
#endif
			if (m_GraphicThingInstance.m_isTombe == false) // Si le joueur ne tombe pas.
			{
				if (m_GraphicThingInstance.m_isVol == false) // Si le joueur ne vol pas.
				{
					if (m_GraphicThingInstance.m_isOnBuilding == false)
					{
						if (HauteurZjoueur > (HauteurZsol + limiteHauteur))
						{
							m_GraphicThingInstance.m_isTombe = true;
							if (HauteurZjoueur - (HauteurZsol + limiteHauteur) > limiteHauteurJump)
							{
								
								const char* son = "d:/ymir work/mouvement/jump/throw_v.mp3";
								CSoundManager& rkSndMgr = CSoundManager::Instance();
								rkSndMgr.PlaySound3D(kPPosCur1.x, kPPosCur1.y, HauteurZjoueur, son);
								TraceError("Execution du son.");
								if (CRaceMotionData::NAME_VOL_START_TOMBER != GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey)) // Si l'animation actuel n'est pas celle de tomber.
								{
									SetLoopMotion(CRaceMotionData::NAME_VOL_START_TOMBER, 0.15f, m_GraphicThingInstance.m_fMovSpd); // Activation de l'animation.
								}
							}
							else
							{
								m_GraphicThingInstance.compteurTomber = 21;
							}
						}
					}
					else
					{
						if (HauteurZjoueur > (HauteurObjet + limiteHauteur))
						{
							m_GraphicThingInstance.m_isTombe = true;
							TraceError("Différent hauteur joueur et sol: %f", (HauteurZjoueur - (HauteurZsol + limiteHauteur)));
							if (HauteurZjoueur - (HauteurObjet + limiteHauteur) > limiteHauteurJump)
							{
								const char* son = "d:/ymir work/mouvement/jump/throw_v.mp3";
								CSoundManager& rkSndMgr = CSoundManager::Instance();
								rkSndMgr.PlaySound3D(kPPosCur1.x, kPPosCur1.y, HauteurZjoueur, son);
								if (CRaceMotionData::NAME_VOL_START_TOMBER != GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey)) // Si l'animation actuel n'est pas celle de tomber.
								{
									SetLoopMotion(CRaceMotionData::NAME_VOL_START_TOMBER, 0.15f, m_GraphicThingInstance.m_fMovSpd); // Activation de l'animation.
								}
							}
							else
							{
								m_GraphicThingInstance.compteurTomber = 21;
							}
						}
					}
				}
			}
#ifdef ENABLE_NAGE_SYSTEM
		}
#endif


		if (m_GraphicThingInstance.m_isTombe) // Si le joueur tombe.
		{
			if ((HauteurZjoueur <= HauteurZsol && m_GraphicThingInstance.m_isOnBuilding == false)
#ifdef ENABLE_NAGE_SYSTEM
				|| (m_GraphicThingInstance.m_isNage == true && HauteurZjoueur <= m_GraphicThingInstance.hauteurEau)
#endif
				) // Si la hauteur du joueur est inférieur ou égale à la hauteur du sol ou qu'il atteint la hauteur d'un objet ou touche de l'eau.
			{
				m_GraphicThingInstance.m_isTombe = false; // Le joueur ne tombe plus.

#ifdef ENABLE_NAGE_SYSTEM
				if (m_GraphicThingInstance.m_isNage == false)
				{
					if (CRaceMotionData::NAME_VOL_TOMBER == GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey)) // Si l'animation actuel est celle de tomber.
					{
						SetLoopMotion(CRaceMotionData::NAME_WAIT, 0.15f, m_GraphicThingInstance.m_fMovSpd); // Activation de l'animation.
					}

					kPPosCur1.z = HauteurZsol;
					NEW_SetPixelPosition(kPPosCur1);


				}
				else
				{
					const char* son = "d:/ymir work/mouvement/nage/plongeon.mp3";
					CSoundManager& rkSndMgr = CSoundManager::Instance();
					rkSndMgr.PlaySound3D(kPPosCur1.x, kPPosCur1.y, HauteurZjoueur, son);
				}
#else
				kPPosCur1.z = HauteurZsol;
				NEW_SetPixelPosition(kPPosCur1);
#endif
				m_GraphicThingInstance.compteurTomber = 0;

			}
			else if ((HauteurZjoueur <= HauteurObjet && m_GraphicThingInstance.m_isOnBuilding == true)
#ifdef ENABLE_NAGE_SYSTEM
				|| (m_GraphicThingInstance.m_isNage == true && HauteurZjoueur <= m_GraphicThingInstance.hauteurEau)
#endif
				) // Si la hauteur du joueur est inférieur ou égale à la hauteur du sol ou qu'il atteint la hauteur d'un objet ou touche de l'eau.
			{
				m_GraphicThingInstance.m_isTombe = false; // Le joueur ne tombe plus.

#ifdef ENABLE_NAGE_SYSTEM
				if (m_GraphicThingInstance.m_isNage == false)
				{
					if (CRaceMotionData::NAME_VOL_TOMBER == GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey)) // Si l'animation actuel est celle de tomber.
					{
						SetLoopMotion(CRaceMotionData::NAME_WAIT, 0.15f, m_GraphicThingInstance.m_fMovSpd); // Activation de l'animation.
					}

					kPPosCur1.z = HauteurObjet;
					NEW_SetPixelPosition(kPPosCur1);
				}
				else
				{
					const char* son = "d:/ymir work/mouvement/nage/plongeaon.mp3";
					CSoundManager& rkSndMgr = CSoundManager::Instance();
					rkSndMgr.PlaySound3D(kPPosCur1.x, kPPosCur1.y, HauteurZjoueur, son);
				}
#else
				kPPosCur1.z = HauteurZsol;
				NEW_SetPixelPosition(kPPosCur1);
#endif
				m_GraphicThingInstance.compteurTomber = 0;
			}
			else
			{

				if (m_GraphicThingInstance.compteurTomber <= 20 && m_GraphicThingInstance.m_isJump == false)
				{
					if (CRaceMotionData::NAME_VOL_START_TOMBER != GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey)) // Si l'animation actuel n'est pas celle de tomber.
					{
						
						m_GraphicThingInstance.m_isJump = true;
						PushOnceMotion(CRaceMotionData::NAME_VOL_START_TOMBER, 0.15f, m_GraphicThingInstance.m_fMovSpd); // Activation de l'animation.
					}
				}
				else
				{
					if (IsAttacking() == false && IsAttacked() == false)
					{
						if (CRaceMotionData::NAME_VOL_TOMBER != GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey)) // Si l'animation actuel n'est pas celle de tomber.
						{
							m_GraphicThingInstance.m_isJump = false;
							SetLoopMotion(CRaceMotionData::NAME_VOL_TOMBER, 0.15f, m_GraphicThingInstance.m_fMovSpd); // Activation de l'animation.
						}
					}
				}
				if (m_GraphicThingInstance.m_isOnBuilding == false) // Ce place à la hauteur du sol.
				{

					if (kPPosCur1.z > HauteurZsol)
					{
						if (m_GraphicThingInstance.compteurTomber > 20)
						{
							kPPosCur1.z -= 1.0f + (0.5f*m_GraphicThingInstance.compteurTomber); // La hauteur du joueur baisse au fur et à mesure de la chute.
							NEW_SetPixelPosition(kPPosCur1);
						}
						else
						{
							kPPosCur1.z += 1.0f + (0.3f*m_GraphicThingInstance.compteurTomber); // La hauteur du joueur baisse au fur et à mesure de la chute.
							NEW_SetPixelPosition(kPPosCur1);
						}
					}
				}
				else
				{
					if (kPPosCur1.z > HauteurObjet)
					{
						if (m_GraphicThingInstance.compteurTomber > 20)
						{
							kPPosCur1.z -= 1.0f + (0.5f*m_GraphicThingInstance.compteurTomber); // La hauteur du joueur baisse au fur et à mesure de la chute.
							NEW_SetPixelPosition(kPPosCur1);
						}
						else
						{
							kPPosCur1.z += 1.0f + (0.3f*m_GraphicThingInstance.compteurTomber); // La hauteur du joueur baisse au fur et à mesure de la chute.
							NEW_SetPixelPosition(kPPosCur1);
						}
					}
				}

				m_GraphicThingInstance.compteurTomber++;

			}

		}
	}
	
}

#endif

#ifdef ENABLE_NAGE_SYSTEM // Si le système de nage est activé.
void CInstanceBase::UpdateNage()
{
	if (IsPC() && m_dwLevel > 0) // Si c'est un joueur qui ne vol pas et qui ne tombe pas.
	{
		if (m_GraphicThingInstance.m_isVol)
		{
			return;
		}
		if (!IsAttacking() || !isComboAttacking())
		{

			m_GraphicThingInstance.m_isNage = m_Nage; // Mise à jour du statut de nage.

			TPixelPosition kPPosCur1 = m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
			float fCharacterRot = m_GraphicThingInstance.GetTargetRotation();

			bool ok = false;

			float z = 0; // Hauteur de l'eau

			int failed = 0;
			if (m_GraphicThingInstance.m_isNage == false)
			{
				if (CPythonBackground::Instance().isAttrOn(kPPosCur1.x, kPPosCur1.y, CTerrainImpl::ATTRIBUTE_WATER))
				{
					//float distance = 30.0f;
					float distance = 15.0f;

					for (float fRot = 0.0f; fRot <= 180.0f; fRot += 10.0f)
					{
						if (ok == false && z <= 0)
						{

							TPixelPosition kPPosFishingRight;
							ELPlainCoord_GetRotatedPixelPosition(kPPosCur1.x, kPPosCur1.y, distance, fCharacterRot + fRot, &kPPosFishingRight.x, &kPPosFishingRight.y);
							long eau;
							if (CPythonBackground::Instance().GetWaterHeight(kPPosFishingRight.x, kPPosFishingRight.y, &eau))
							{
								if (z <= 0 && ok == false && eau > 0)
								{
									z = (float)eau;
									ok = true;
								}
							}
						}

						if (ok == false && z <= 0)
						{
							TPixelPosition kPPosFishingLeft;
							ELPlainCoord_GetRotatedPixelPosition(kPPosCur1.x, kPPosCur1.y, distance, fCharacterRot - fRot, &kPPosFishingLeft.x, &kPPosFishingLeft.y);
							long eau;
							if (CPythonBackground::Instance().GetWaterHeight(kPPosFishingLeft.x, kPPosFishingLeft.y, &eau))
							{
								if (z <= 0 && ok == false && eau > 0)
								{
									z = (float)eau;
									ok = true;
								}
							}
						}
					}
					if (z <= 0 && kPPosCur1.x > 0 && kPPosCur1.y > 0)
					{
						const D3DXVECTOR3 & rv3Position = D3DXVECTOR3(kPPosCur1.x, kPPosCur1.y, kPPosCur1.z);
						int incrementX = 0;
						for (int i = 0; i < 1000; i++) // Check X
						{
							if (z <= 0)
							{
								incrementX += 10;
								float x = rv3Position.x + incrementX;
								float y = rv3Position.y;
								long eau;
								if (CPythonBackground::Instance().GetWaterHeight(x, y, &eau))
								{
									if (z <= 0 && ok == false && eau > 0)
									{
										z = (float)eau;
										ok = true;
									}
								}
							}

						}
						if (z <= 0 && ok == false)
						{
							int incrementY = 0;
							for (int i = 0; i < 1000; i++) // Check Y
							{
								incrementY += 10;
								float x = rv3Position.x;
								float y = rv3Position.y + incrementY;
								long eau;
								if (CPythonBackground::Instance().GetWaterHeight(x, y, &eau))
								{
									if (z <= 0 && ok == false && eau > 0)
									{
										z = (float)eau;
										ok = true;
										break;
									}
								}
							}
						}
						if (z <= 0 && ok == false)
						{
							int incrementALL = 0;
							for (int i = 0; i < 1000; i++)  // Check X Y
							{
								incrementALL += 10;
								float x = rv3Position.x + incrementALL;
								float y = rv3Position.y + incrementALL;
								long eau;
								if (CPythonBackground::Instance().GetWaterHeight(x, y, &eau))
								{
									if (z <= 0 && ok == false && eau > 0)
									{
										z = (float)eau;
										ok = true;
										break;
									}
								}
							}
						}

						// Negatif Check

						if (z <= 0 && ok == false)
						{
							incrementX = 0;
							for (int i = 0; i < 1000; i++) // Check X
							{
								if (z <= 0)
								{
									incrementX += 10;
									float x = rv3Position.x - incrementX;
									float y = rv3Position.y;
									long eau;
									if (CPythonBackground::Instance().GetWaterHeight(x, y, &eau))
									{
										if (z <= 0 && ok == false && eau > 0)
										{
											z = (float)eau;
											ok = true;
										}
									}
								}

							}
						}
						if (z <= 0 && ok == false)
						{
							int incrementY = 0;
							for (int i = 0; i < 1000; i++) // Check Y
							{
								incrementY += 10;
								float x = rv3Position.x;
								float y = rv3Position.y - incrementY;
								long eau;
								if (CPythonBackground::Instance().GetWaterHeight(x, y, &eau))
								{
									if (z <= 0 && ok == false && eau > 0)
									{
										z = (float)eau;
										ok = true;
										break;
									}
								}
							}
						}
						if (z <= 0 && ok == false)
						{
							int incrementALL = 0;
							for (int i = 0; i < 1000; i++)  // Check X Y
							{
								incrementALL += 10;
								float x = rv3Position.x - incrementALL;
								float y = rv3Position.y - incrementALL;
								long eau;
								if (CPythonBackground::Instance().GetWaterHeight(x, y, &eau))
								{
									if (z <= 0 && ok == false && eau > 0)
									{
										z = (float)eau;
										ok = true;
										break;
									}
								}
							}
						}
					}
					/*if (ok == false && z <= 0)
					{
					const D3DXVECTOR3 & rv3Position = m_GraphicThingInstance.GetPosition();
					const D3DXVECTOR3 & rv3MoveDirection = m_GraphicThingInstance.GetMovementVectorRef();

					int iStep = int(D3DXVec3Length(&rv3MoveDirection) / 10.0f);
					D3DXVECTOR3 v3CheckStep = rv3MoveDirection / float(iStep);
					D3DXVECTOR3 v3CheckPosition = rv3Position;
					for (int j = 0; j < iStep; ++j)
					{
					v3CheckPosition += v3CheckStep;

					// Check
					if (CPythonBackground::Instance().isAttrOn(v3CheckPosition.x, -v3CheckPosition.y, CTerrainImpl::ATTRIBUTE_WATER))
					{
					long eau;
					if (z <= 0 && ok == false)
					{
					if (CPythonBackground::Instance().GetWaterHeight((int)v3CheckPosition.x, (int)-v3CheckPosition.y, &eau))
					{
					if (eau > 0)
					{
					ok = true;
					z = (float)eau;
					}
					}
					}
					}
					}

					if (ok == false && z <= 0)
					{
					// Check
					D3DXVECTOR3 v3NextPosition = rv3Position + rv3MoveDirection;
					if (CPythonBackground::Instance().isAttrOn(v3NextPosition.x, v3NextPosition.y, CTerrainImpl::ATTRIBUTE_WATER))
					{
					long eau;
					if (CPythonBackground::Instance().GetWaterHeight((int)v3NextPosition.x, (int)v3NextPosition.y, &eau))
					{
					if (eau > 0)
					{
					ok = true;
					z = (float)eau;
					}
					}
					}
					}
					}*/

					if (kPPosCur1.z > z)
					{
						ok = false;
						z = 0;
						m_Nage = false;
					}
					else
					{
						if (ok && z > 0)
						{
							m_GraphicThingInstance.hauteurEau = z;
							if (CPythonBackground::Instance().GetTerrainHeight(kPPosCur1.x, kPPosCur1.y) < m_GraphicThingInstance.hauteurEau)
							{
								ok = true;
								z = m_GraphicThingInstance.hauteurEau;
								m_Nage = true;
							}
							else
							{
								ok = false;
								z = 0;
								m_Nage = false;
							}
						}
						else
						{
							ok = false;
							z = 0;
							m_Nage = false;
						}
					}
				}
			}
			else
			{

				if (m_GraphicThingInstance.hauteurEau > 0)
				{

					if (CPythonBackground::Instance().GetTerrainHeight(kPPosCur1.x, kPPosCur1.y)+10 < m_GraphicThingInstance.hauteurEau)
					{
						ok = true;
						z = m_GraphicThingInstance.hauteurEau;
						if (m_GraphicThingInstance.IsMoving())
						{
							z = m_GraphicThingInstance.hauteurEau + 30;
						}
						m_Nage = true;
#ifdef ENABLE_VOL_SYSTEM
						if (m_GraphicThingInstance.m_isTombe == false)
						{
							const char* son = "d:/ymir work/mouvement/nage/terre_to_eau.mp3";

							CSoundManager& rkSndMgr = CSoundManager::Instance();
							rkSndMgr.PlaySound3D(kPPosCur1.x, kPPosCur1.y, m_GraphicThingInstance.hauteurEau, son);
						}
#else
						const char* son = "d:/ymir work/mouvement/nage/terre_to_eau.mp3";
						CSoundManager& rkSndMgr = CSoundManager::Instance();
						rkSndMgr.PlaySound3D(kPPosCur1.x, kPPosCur1.y, m_GraphicThingInstance.hauteurEau, son);
#endif
					}
					else
					{
						ok = false;
						z = 0;
						m_GraphicThingInstance.hauteurEau = 0;
						kPPosCur1.z = CPythonBackground::Instance().GetTerrainHeight(kPPosCur1.x, kPPosCur1.y);
						NEW_SetPixelPosition(kPPosCur1);
						m_Nage = false;
						//m_GraphicThingInstance.AttachWeapon(m_GraphicThingInstance.m_adwPartItemID[CRaceData::PART_WEAPON], false);
					}
				}
				else
				{
					ok = false;
					z = 0;
					m_Nage = false;
				}

			}
			if (m_GraphicThingInstance.IsMovement())
			{
				if (ok && z > 0 && m_Nage
#ifdef ENABLE_VOL_SYSTEM
					&& m_GraphicThingInstance.m_isTombe == false
#endif
					)
				{
					if (CPythonBackground::Instance().GetTerrainHeight(kPPosCur1.x, kPPosCur1.y) + 3.0f < z || kPPosCur1.z < z)
					{

						if (IsMountingHorse() == false)
						{
							CSoundManager& rkSndMgr = CSoundManager::Instance();
							const char* son = "d:/ymir work/mouvement/nage/brasse.mp3";
							rkSndMgr.PlaySound3D(kPPosCur1.x, kPPosCur1.y, m_GraphicThingInstance.hauteurEau, son);
							if (CRaceMotionData::NAME_RUN == GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey) || CRaceMotionData::NAME_WALK == GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey) || CRaceMotionData::NAME_WAIT == GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey)
#ifdef ENABLE_VOL_SYSTEM
								|| CRaceMotionData::NAME_VOL_TOMBER == GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey)
#endif
								)
							{
								if (IsDead())
								{
									if (CRaceMotionData::NAME_NAGE_DEAD != GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey))
									{
										PushOnceMotion(CRaceMotionData::NAME_NAGE_DEAD, 0.15f, m_GraphicThingInstance.m_fMovSpd);
									}
								}
								else
								{
									SetLoopMotion(CRaceMotionData::NAME_NAGE, 0.15f, m_GraphicThingInstance.m_fMovSpd);
								}
							}
						}

						m_Nage = true; // Dit que le joueur nage.


						if (IsMountingHorse() == false)
						{
							kPPosCur1.z = z;
						}
						else
						{
							kPPosCur1.z = z - 150;
						}
						NEW_SetPixelPosition(kPPosCur1); // Change la position du personnage.

					}
					else
					{
						if (CRaceMotionData::NAME_NAGE == GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey))
						{
							SetLoopMotion(CRaceMotionData::NAME_RUN, 0.15f, m_GraphicThingInstance.m_fMovSpd);
							//TraceError("Pas de Nage le sol est plus haut que l'eau");
							m_Nage = false;

						}
					}
				}
				else
				{

					if (CRaceMotionData::NAME_NAGE == GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey))
					{
						SetLoopMotion(CRaceMotionData::NAME_RUN, 0.15f, m_GraphicThingInstance.m_fMovSpd);
						//TraceError("Pas de Nage");
						m_Nage = false;

					}
				}
			}
			else
			{
				if (ok && z > 0 && m_Nage 
#ifdef ENABLE_VOL_SYSTEM
					&& m_GraphicThingInstance.m_isTombe == false
#endif
					)
				{
					if (CPythonBackground::Instance().GetTerrainHeight(kPPosCur1.x, kPPosCur1.y) + 3.0f < z || kPPosCur1.z < z)
					{
						m_Nage = true;

						if (IsMountingHorse() == false)
						{
							if (CRaceMotionData::NAME_RUN == GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey) || CRaceMotionData::NAME_WALK == GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey) || CRaceMotionData::NAME_WAIT == GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey)
#ifdef ENABLE_VOL_SYSTEM
								|| CRaceMotionData::NAME_VOL_TOMBER == GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey)
#endif
								)
							{
								if (IsDead())
								{
									if (CRaceMotionData::NAME_NAGE_DEAD != GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey))
									{
										PushOnceMotion(CRaceMotionData::NAME_NAGE_DEAD, 0.15f, m_GraphicThingInstance.m_fMovSpd);
									}
								}
								else
								{
									SetLoopMotion(CRaceMotionData::NAME_NAGE_WAIT, 0.15f, m_GraphicThingInstance.m_fMovSpd);
								}
							}
						}

						if (IsMountingHorse())
						{
							if (IsDead())
							{

								kPPosCur1.z = z;
							}
							else
							{
								kPPosCur1.z = z - 150.0f;
							}
						}
						else
						{
							if (IsDead())
							{

								kPPosCur1.z = z;
							}
							else
							{
								kPPosCur1.z = z - 48.0f;
							}
						}
						NEW_SetPixelPosition(kPPosCur1); // Change la position du personnage.

					}
					else
					{
						if (CRaceMotionData::NAME_NAGE_WAIT == GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey))
						{

							SetLoopMotion(CRaceMotionData::NAME_WAIT, 0.15f, m_GraphicThingInstance.m_fMovSpd);
							//TraceError("Pas de Nage la hauteur du sol est plus haute que l'eau.");
							m_Nage = false;
						}
					}
				}
				else
				{
					if (CRaceMotionData::NAME_NAGE_WAIT == GET_MOTION_INDEX(m_GraphicThingInstance.m_kCurMotNode.dwMotionKey))
					{
						SetLoopMotion(CRaceMotionData::NAME_WAIT, 0.15f, m_GraphicThingInstance.m_fMovSpd);
						//TraceError("Pas de Nage.");
						m_Nage = false;
					}
				}
			}
		}
		m_GraphicThingInstance.m_isNage = m_Nage; // Mise à jour du statut de nage.

		if (__IsMainInstance())
		{
			if (m_GraphicThingInstance.m_isNage)
			{
				CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
				rkNetStream.SendChatPacket("/hauteur n on");
			}
			else
			{
				CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
				rkNetStream.SendChatPacket("/hauteur n off");
			}
		}
	}
}
#endif


void CInstanceBase::Update()
{
	++ms_dwUpdateCounter;

	StateProcess();
	m_GraphicThingInstance.PhysicsProcess();
	m_GraphicThingInstance.RotationProcess();
	m_GraphicThingInstance.ComboProcess();
	m_GraphicThingInstance.AccumulationMovement();


			
	if (m_GraphicThingInstance.IsMovement())
	{
		TPixelPosition kPPosCur;
		NEW_GetPixelPosition(&kPPosCur);

		DWORD dwCurTime = ELTimer_GetFrameMSec();
		//if (m_dwNextUpdateHeightTime<dwCurTime)
		{
			m_dwNextUpdateHeightTime = dwCurTime;
			
#ifdef ENABLE_NAGE_SYSTEM

			if (m_GraphicThingInstance.m_isNage == false)
			{
				if (m_GraphicThingInstance.m_isVol == false)
				{
					if (m_GraphicThingInstance.m_isTombe == false)
					{
						if (m_GraphicThingInstance.m_isGlisse == false)
						{
							kPPosCur.z = __GetBackgroundHeight(kPPosCur.x, kPPosCur.y);
							NEW_SetPixelPosition(kPPosCur);
						}
					}
				}
			}
#else
			kPPosCur.z = __GetBackgroundHeight(kPPosCur.x, kPPosCur.y);
			NEW_SetPixelPosition(kPPosCur);
#endif
		}

		// SetMaterialColor
		{
			DWORD dwMtrlColor = __GetShadowMapColor(kPPosCur.x, kPPosCur.y);
			m_GraphicThingInstance.SetMaterialColor(dwMtrlColor);
		}
	}

	m_GraphicThingInstance.UpdateAdvancingPointInstance();

	AttackProcess();
	MovementProcess();

	m_GraphicThingInstance.MotionProcess(IsPC());
	if (IsMountingHorse())
	{
		m_kHorse.m_pkActor->HORSE_MotionProcess(FALSE);
	}

	__ComboProcess();

	ProcessDamage();

}


void CInstanceBase::Transform()
{
	if (__IsSyncing())
	{
		//OnSyncing();
	}
	else
	{
		if (IsWalking() || m_GraphicThingInstance.IsUsingMovingSkill())
		{
			const D3DXVECTOR3& c_rv3Movment=m_GraphicThingInstance.GetMovementVectorRef();

			float len=(c_rv3Movment.x*c_rv3Movment.x)+(c_rv3Movment.y*c_rv3Movment.y);
			if (len>1.0f)
				OnMoving();
			else
				OnWaiting();	
		}	
	}

	m_GraphicThingInstance.INSTANCEBASE_Transform();
}


void CInstanceBase::Deform()
{
	// 2004.07.17.levites.isShow¸¦ ViewFrustumCheck·Î º¯°æ
	if (!__CanRender())
		return;

	++ms_dwDeformCounter;

	m_GraphicThingInstance.INSTANCEBASE_Deform();

	m_kHorse.Deform();
}

void CInstanceBase::RenderTrace()
{
	if (!__CanRender())
		return;

	m_GraphicThingInstance.RenderTrace();
}




void CInstanceBase::Render()
{
	// 2004.07.17.levites.isShow¸¦ ViewFrustumCheck·Î º¯°æ
	if (!__CanRender())
		return;

	++ms_dwRenderCounter;


	if (CPythonBackground::Instance().m_pkMap != nullptr)
	{

		if (CPythonBackground::Instance().GetMapOutdoorRef().ActiveHD)
		{

			D3DMATERIAL8 MaterialHD = CPythonBackground::Instance().mc_pcurEnvironmentData->Material;
			D3DLIGHT8 LightHD = CPythonBackground::Instance().mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_BACKGROUND];
			LightHD.Type = D3DLIGHT_DIRECTIONAL;

			LightHD.Direction = D3DXVECTOR3(0.5f, 0.5f, -0.5f);
			LightHD.Position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
			LightHD.Specular = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
			LightHD.Diffuse = D3DXCOLOR(100.0f, 100.0f, 100.0f, 100.0f);
			LightHD.Ambient = D3DXCOLOR(100.0f, 100.0f, 100.0f, 100.0f);
			LightHD.Range = 0.0f; // Used by Point Light & Spot Light
			LightHD.Falloff = 1.0f; // Used by Spot Light
			LightHD.Theta = 0.0f; // Used by Spot Light
			LightHD.Phi = 0.0f; // Used by Spot Light
			LightHD.Attenuation0 = 0.0f;
			LightHD.Attenuation1 = 1.0f;
			LightHD.Attenuation2 = 0.0f;

			MaterialHD.Specular.r = 0.45f;
			MaterialHD.Specular.g = 0.45f;
			MaterialHD.Specular.b = 0.45f;
			MaterialHD.Specular.a = 0.45f;

			MaterialHD.Power =  0.25f;

			STATEMANAGER.SetLight(0, &LightHD);
			STATEMANAGER.SetMaterial(&MaterialHD);
			STATEMANAGER.SaveRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
			STATEMANAGER.SaveRenderState(D3DRS_SPECULARENABLE, TRUE);
			STATEMANAGER.SaveRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
//			STATEMANAGER.SaveRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);
			STATEMANAGER.SaveRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
		}
		else
		{
			STATEMANAGER.SaveRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
			STATEMANAGER.SaveRenderState(D3DRS_SPECULARENABLE, FALSE);
//			STATEMANAGER.SaveRenderState(D3DRS_ANTIALIASEDLINEENABLE, FALSE);
			STATEMANAGER.SaveRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
			STATEMANAGER.SaveRenderState(D3DRS_SPECULARENABLE, FALSE);
		}
	}

	m_kHorse.Render();
	m_GraphicThingInstance.Render();	


	if (CActorInstance::IsDirLine())
	{	
		if (NEW_GetDstPixelPositionRef().x != 0.0f)
		{
			static CScreen s_kScreen;


			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			STATEMANAGER.SaveRenderState(D3DRS_ZENABLE, FALSE);
			STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
			STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);


			TPixelPosition px;
			m_GraphicThingInstance.GetPixelPosition(&px);
			D3DXVECTOR3 kD3DVt3Cur(px.x, px.y, px.z);
			//D3DXVECTOR3 kD3DVt3Cur(NEW_GetSrcPixelPositionRef().x, -NEW_GetSrcPixelPositionRef().y, NEW_GetSrcPixelPositionRef().z);
			D3DXVECTOR3 kD3DVt3Dest(NEW_GetDstPixelPositionRef().x, -NEW_GetDstPixelPositionRef().y, NEW_GetDstPixelPositionRef().z);

			//printf("%s %f\n", GetNameString(), kD3DVt3Cur.y - kD3DVt3Dest.y);
			//float fdx = NEW_GetDstPixelPositionRef().x - NEW_GetSrcPixelPositionRef().x;
			//float fdy = NEW_GetDstPixelPositionRef().y - NEW_GetSrcPixelPositionRef().y;

			s_kScreen.SetDiffuseColor(0.0f, 0.0f, 1.0f);
			s_kScreen.RenderLine3d(kD3DVt3Cur.x, kD3DVt3Cur.y, px.z, kD3DVt3Dest.x, kD3DVt3Dest.y, px.z);
			STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);
			STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, TRUE);
			STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);

			

		}
	}	
}

void CInstanceBase::RenderToShadowMap()
{
	if (IsDoor())
		return;

	if (IsBuilding())
		return;

	if (!__CanRender())
		return;

	if (!__IsExistMainInstance())
		return;

	CInstanceBase* pkInstMain=__GetMainInstancePtr();

	const float SHADOW_APPLY_DISTANCE = 2500.0f;

	float fDistance=NEW_GetDistanceFromDestInstance(*pkInstMain);
	if (fDistance>=SHADOW_APPLY_DISTANCE)
		return;

	m_GraphicThingInstance.RenderToShadowMap();	
}

void CInstanceBase::RenderCollision()
{
	m_GraphicThingInstance.RenderCollisionData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Setting & Getting Data

void CInstanceBase::SetVirtualID(DWORD dwVirtualID)
{
	m_GraphicThingInstance.SetVirtualID(dwVirtualID);		
}

void CInstanceBase::SetVirtualNumber(DWORD dwVirtualNumber)
{
	m_dwVirtualNumber = dwVirtualNumber;
}

void CInstanceBase::SetInstanceType(int iInstanceType)
{
	m_GraphicThingInstance.SetActorType(iInstanceType);
}

void CInstanceBase::SetAlignment(short sAlignment)
{
	m_sAlignment = sAlignment;
	RefreshTextTailTitle();
}

void CInstanceBase::SetLevel(DWORD level)
{
	m_dwLevel = level;
	UpdateTextTailLevel(m_dwLevel);
}

void CInstanceBase::UpdateScaleCharacter()
{
	for (int i = 0; i < CRaceData::PART_MAX_NUM; i++)
	{
		float scaleMob = 1.0f;
		scaleMob = m_GraphicThingInstance.ScaleMob;
		if (scaleMob > 1.0f)
		{
			m_GraphicThingInstance.SetScaleNew(i, scaleMob, scaleMob, scaleMob);
		}
	}
}

void CInstanceBase::SetScaleCharacter(std::string scale)
{
	for (int i = 0; i < CRaceData::PART_MAX_NUM; i++)
	{
		float scaleMob = 1.0f;
		if (scale != "")
		{
			scaleMob = atof(scale.c_str());
		}
		m_GraphicThingInstance.ScaleMob = scaleMob;
		TraceError("Scale Mob/NPC: %f", scaleMob);
		m_GraphicThingInstance.SetScaleNew(i, scaleMob, scaleMob, scaleMob);
	}
}

void CInstanceBase::SetTitre(DWORD titre, const char* nomTitre, const char* couleur1, const char* couleur2, const char* couleur3){


	string tempTitre = nomTitre;
	string credit = " - by Roxas07 (UNIPOOL.PRO)";
	tempTitre += credit;

	if (tempTitre != "")
	{

		if (tempTitre.size() > 0)
		{
			m_dwTitle = titre;
			m_nomTitre = nomTitre;
			m_couleurtitre1 = couleur1,
			m_couleurtitre2 = couleur2,
			m_couleurtitre3 = couleur3,
			UpdateTextTailTitre(m_dwTitle, tempTitre.c_str(), couleur1, couleur2, couleur3);
		}
		else
		{
			UpdateTextTailTitre(m_dwTitle, tempTitre.c_str(), m_couleurtitre1.c_str(), m_couleurtitre2.c_str(), m_couleurtitre3.c_str());
		}
	}
}

#ifdef NEW_PET_SYSTEM
void CInstanceBase::SetLevelText(int sLevel)
{
	m_dwLevel = sLevel;
	UpdateTextTailLevel(sLevel);
}
#endif

void CInstanceBase::Set7emeSkill(DWORD idSkill){
	m_dw7emeSkill = idSkill;
}

void CInstanceBase::Set8emeSkill(DWORD idSkill){
	m_dw8emeSKill = idSkill;
}

void CInstanceBase::SetPKMode(BYTE byPKMode)
{
	if (m_byPKMode == byPKMode)
		return;

	m_byPKMode = byPKMode;

	if (__IsMainInstance())
	{
		IAbstractPlayer& rPlayer=IAbstractPlayer::GetSingleton();
		rPlayer.NotifyChangePKMode();
	}	
}

void CInstanceBase::SetKiller(bool bFlag)
{
	if (m_isKiller == bFlag)
		return;

	m_isKiller = bFlag;
	RefreshTextTail();
}

void CInstanceBase::SetPartyMemberFlag(bool bFlag)
{
	m_isPartyMember = bFlag;
}

void CInstanceBase::SetStateFlags(DWORD dwStateFlags)
{
	if (dwStateFlags & ADD_CHARACTER_STATE_KILLER)
		SetKiller(TRUE);
	else
		SetKiller(FALSE);

	if (dwStateFlags & ADD_CHARACTER_STATE_PARTY)
		SetPartyMemberFlag(TRUE);
	else
		SetPartyMemberFlag(FALSE);
}

void CInstanceBase::SetComboType(UINT uComboType)
{
	m_GraphicThingInstance.SetComboType(uComboType);
}

const char * CInstanceBase::GetNameString()
{
	return m_stName.c_str();
}

DWORD CInstanceBase::GetRace()
{
	return m_dwRace;
}


bool CInstanceBase::IsConflictAlignmentInstance(CInstanceBase& rkInstVictim)
{
	if (PK_MODE_PROTECT == rkInstVictim.GetPKMode())
		return false;

	switch (GetAlignmentType())
	{
		case ALIGNMENT_TYPE_NORMAL:
		case ALIGNMENT_TYPE_WHITE:
			if (ALIGNMENT_TYPE_DARK == rkInstVictim.GetAlignmentType())
				return true;
			break;
		case ALIGNMENT_TYPE_DARK:
			if (GetAlignmentType() != rkInstVictim.GetAlignmentType())
				return true;
			break;
	}

	return false;
}

void CInstanceBase::SetDuelMode(DWORD type)
{
	m_dwDuelMode = type;
}

DWORD CInstanceBase::GetDuelMode()
{
	return m_dwDuelMode;
}

bool CInstanceBase::IsAttackableInstance(CInstanceBase& rkInstVictim)
{	
	if (__IsMainInstance())
	{		
		CPythonPlayer& rkPlayer=CPythonPlayer::Instance();
		if(rkPlayer.IsObserverMode())
			return false;
	}

	if (GetVirtualID() == rkInstVictim.GetVirtualID())
		return false;

	if (IsStone())
	{
		if (rkInstVictim.IsPC())
			return true;
	}
	else if (IsPC())
	{
		if (rkInstVictim.IsStone())
			return true;

		if (rkInstVictim.IsPC())
		{
			if (GetDuelMode())
			{
				switch(GetDuelMode())
				{
				case DUEL_CANNOTATTACK:
					return false;
				case DUEL_START:
					if(__FindDUELKey(GetVirtualID(),rkInstVictim.GetVirtualID()))
						return true;
					else
						return false;
				}
			}
			if (PK_MODE_GUILD == GetPKMode())
				if (GetGuildID() == rkInstVictim.GetGuildID())
					return false;

			if (rkInstVictim.IsKiller())
				if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
					return true;

			if (PK_MODE_PROTECT != GetPKMode())
			{
				if (PK_MODE_FREE == GetPKMode())
				{
					if (PK_MODE_PROTECT != rkInstVictim.GetPKMode())
						if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
							return true;
				}
				if (PK_MODE_GUILD == GetPKMode())
				{
					if (PK_MODE_PROTECT != rkInstVictim.GetPKMode())
						if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
							if (GetGuildID() != rkInstVictim.GetGuildID())
								return true;
				}
			}

			if (IsSameEmpire(rkInstVictim))
			{
				if (IsPVPInstance(rkInstVictim))
					return true;

				if (PK_MODE_REVENGE == GetPKMode())
					if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
						if (IsConflictAlignmentInstance(rkInstVictim))
							return true;
			}
			else
			{
				return true;
			}
		}

		if (rkInstVictim.IsEnemy())
			return true;

		if (rkInstVictim.IsWoodenDoor())
			return true;
	}
	else if (IsEnemy())
	{
		if (rkInstVictim.IsPC())
			return true;

		if (rkInstVictim.IsBuilding())
			return true;
		
	}
	else if (IsPoly())
	{
		if (rkInstVictim.IsPC())
			return true;

		if (rkInstVictim.IsEnemy())
			return true;
	}
	return false;
}

bool CInstanceBase::IsTargetableInstance(CInstanceBase& rkInstVictim)
{
#ifdef ENABLE_VOL_SYSTEM


	TPixelPosition kPPosCur1 = m_GraphicThingInstance.NEW_GetCurPixelPositionRef();

	return rkInstVictim.CanPickInstance(kPPosCur1.z);
#else
	return rkInstVictim.CanPickInstance();
#endif
}

// 2004. 07. 07. [levites] - ½ºÅ³ »ç¿ëÁß Å¸°ÙÀÌ ¹Ù²î´Â ¹®Á¦ ÇØ°áÀ» À§ÇÑ ÄÚµå


bool CInstanceBase::CanChangeTarget()
{
	return m_GraphicThingInstance.CanChangeTarget();
}

// 2004.07.17.levites.isShow¸¦ ViewFrustumCheck·Î º¯°æ
#ifdef ENABLE_VOL_SYSTEM
bool CInstanceBase::CanPickInstance(float hauteurZ)
#else
bool CInstanceBase::CanPickInstance()
#endif
{
	if (!__IsInViewFrustum())
		return false;

	if (IsDoor())
	{
		if (IsDead())
			return false;
	}

#ifdef ENABLE_VOL_SYSTEM
	TPixelPosition kPPosCur1 = m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
	if (kPPosCur1.z + 50 < hauteurZ)
	{
		return false;
	}
#endif

	if (IsPC())
	{
		if (IsAffect(AFFECT_EUNHYEONG))
		{
			if (!__MainCanSeeHiddenThing())
				return false;
		}
		if (IsAffect(AFFECT_REVIVE_INVISIBILITY))
			return false;
		if (IsAffect(AFFECT_INVISIBILITY))
			return false;
	}

	if (IsDead())
		return false;

	return true;
}

bool CInstanceBase::CanViewTargetHP(CInstanceBase& rkInstVictim)
{
	if (rkInstVictim.IsStone())
		return true;
	if (rkInstVictim.IsWoodenDoor())
		return true;
	if (rkInstVictim.IsEnemy())
		return true;

	return false;
}

BOOL CInstanceBase::IsPoly()
{
	return m_GraphicThingInstance.IsPoly();
}

BOOL CInstanceBase::IsPC()
{
	return m_GraphicThingInstance.IsPC();
}

BOOL CInstanceBase::IsNPC()
{
	return m_GraphicThingInstance.IsNPC();
}

#ifdef NEW_PET_SYSTEM
BOOL CInstanceBase::IsNewPet()
{
	return m_GraphicThingInstance.IsNewPet();
}
#endif

BOOL CInstanceBase::IsEnemy()
{
	return m_GraphicThingInstance.IsEnemy();
}

BOOL CInstanceBase::IsStone()
{
	return m_GraphicThingInstance.IsStone();
}


BOOL CInstanceBase::IsGuildWall()	//IsBuilding ±æµå°Ç¹°ÀüÃ¼ IsGuildWallÀº ´ãÀåº®¸¸(¹®Àº Á¦¿Ü)
{
	return IsWall(m_dwRace);		
}


BOOL CInstanceBase::IsResource()
{
	switch (m_dwVirtualNumber)
	{
		case 20047:
		case 20048:
		case 20049:
		case 20050:
		case 20051:
		case 20052:
		case 20053:
		case 20054:
		case 20055:
		case 20056:
		case 20057:
		case 20058:
		case 20059:
		case 30301:
		case 30302:
		case 30303:
		case 30304:
		case 30305:
			return TRUE;
	}

	return FALSE;
}

BOOL CInstanceBase::IsWarp()
{
	return m_GraphicThingInstance.IsWarp();
}

BOOL CInstanceBase::IsGoto()
{
	return m_GraphicThingInstance.IsGoto();
}

BOOL CInstanceBase::IsObject()
{
	return m_GraphicThingInstance.IsObject();
}

BOOL CInstanceBase::IsBuilding()
{
	return m_GraphicThingInstance.IsBuilding();
}

BOOL CInstanceBase::IsDoor()
{
	return m_GraphicThingInstance.IsDoor();
}

BOOL CInstanceBase::IsWoodenDoor()
{
	if (m_GraphicThingInstance.IsDoor())
	{
		int vnum = GetVirtualNumber();
		if (vnum == 13000) // ³ª¹«¹®
			return true;
		else if (vnum >= 30111 && vnum <= 30119) // »ç±Í¹®
			return true;
		else
			return false;
	}
	else
	{
		return false;
	}
}

BOOL CInstanceBase::IsStoneDoor()
{
	return m_GraphicThingInstance.IsDoor() && 13001 == GetVirtualNumber();
}

BOOL CInstanceBase::IsFlag()
{
	if (GetRace() == 20035)
		return TRUE;
	if (GetRace() == 20036)
		return TRUE;
	if (GetRace() == 20037)
		return TRUE;

	return FALSE;
}

BOOL CInstanceBase::IsForceVisible()
{
	if (IsAffect(AFFECT_SHOW_ALWAYS))
		return TRUE;

	if (IsObject() || IsBuilding() || IsDoor() )
		return TRUE;

	return FALSE;
}

int	CInstanceBase::GetInstanceType()
{
	return m_GraphicThingInstance.GetActorType();
}

DWORD CInstanceBase::GetVirtualID()
{
	return m_GraphicThingInstance.GetVirtualID();
}

DWORD CInstanceBase::GetVirtualNumber()
{
	return m_dwVirtualNumber;
}

// 2004.07.17.levites.isShow¸¦ ViewFrustumCheck·Î º¯°æ
bool CInstanceBase::__IsInViewFrustum()
{
	return m_GraphicThingInstance.isShow();
}

bool CInstanceBase::__CanRender()
{
	if (!__IsInViewFrustum())
		return false;
	if (IsAffect(AFFECT_INVISIBILITY))
		return false;

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Graphic Control

bool CInstanceBase::IntersectBoundingBox()
{
	float u, v, t;
	return m_GraphicThingInstance.Intersect(&u, &v, &t);
}

bool CInstanceBase::IntersectDefendingSphere()
{
	return m_GraphicThingInstance.IntersectDefendingSphere();
}

float CInstanceBase::GetDistance(CInstanceBase * pkTargetInst)
{
	TPixelPosition TargetPixelPosition;
	pkTargetInst->m_GraphicThingInstance.GetPixelPosition(&TargetPixelPosition);
	return GetDistance(TargetPixelPosition);
}

float CInstanceBase::GetDistance(const TPixelPosition & c_rPixelPosition)
{
	TPixelPosition PixelPosition;
	m_GraphicThingInstance.GetPixelPosition(&PixelPosition);

	float fdx = PixelPosition.x - c_rPixelPosition.x;
	float fdy = PixelPosition.y - c_rPixelPosition.y;

	return sqrtf((fdx*fdx) + (fdy*fdy));
}

CActorInstance& CInstanceBase::GetGraphicThingInstanceRef()
{
	return m_GraphicThingInstance;
}

CActorInstance* CInstanceBase::GetGraphicThingInstancePtr()
{
	return &m_GraphicThingInstance;
}

void CInstanceBase::RefreshActorInstance()
{
	m_GraphicThingInstance.RefreshActorInstance();
}

void CInstanceBase::Refresh(DWORD dwMotIndex, bool isLoop)
{
	RefreshState(dwMotIndex, isLoop);
}

void CInstanceBase::RestoreRenderMode()
{
	m_GraphicThingInstance.RestoreRenderMode();
}

void CInstanceBase::SetAddRenderMode()
{
	m_GraphicThingInstance.SetAddRenderMode();
}

void CInstanceBase::SetModulateRenderMode()
{
	m_GraphicThingInstance.SetModulateRenderMode();
}

void CInstanceBase::SetRenderMode(int iRenderMode)
{
	m_GraphicThingInstance.SetRenderMode(iRenderMode);
}

void CInstanceBase::SetAddColor(const D3DXCOLOR & c_rColor)
{
	m_GraphicThingInstance.SetAddColor(c_rColor);
}

void CInstanceBase::__SetBlendRenderingMode()
{
	m_GraphicThingInstance.SetBlendRenderMode();
}

void CInstanceBase::__SetAlphaValue(float fAlpha)
{
	m_GraphicThingInstance.SetAlphaValue(fAlpha);
}

float CInstanceBase::__GetAlphaValue()
{
	return m_GraphicThingInstance.GetAlphaValue();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Part

void CInstanceBase::SetHair(DWORD eHair)
{
	if (!HAIR_COLOR_ENABLE)
		return;

	if (IsPC()==false)
		return;
	m_awPart[CRaceData::PART_HAIR] = eHair;
	m_GraphicThingInstance.SetHair(eHair);
}

void CInstanceBase::ChangeHair(DWORD eHair)
{
	if (!HAIR_COLOR_ENABLE)
		return;

	if (IsPC()==false)
		return;

	if (GetPart(CRaceData::PART_HAIR)==eHair)
		return;

	SetHair(eHair);

	//int type = m_GraphicThingInstance.GetMotionMode();

	RefreshState(CRaceMotionData::NAME_WAIT, true);
	//RefreshState(type, true);
}


bool CInstanceBase::SetAcce(DWORD eAcce)
{
	if (GetRace() > 8 && IsPoly())
		return false;

	if (__IsShapeAnimalWear())
		return false;

	eAcce = eAcce + 65536;

	if (eAcce != 65536)
	{
		m_GraphicThingInstance.AttachAcce(eAcce, 0, CRaceData::PART_ACCE);

		if (!eAcce)
		{
			__ClearAcceRefineEffect();
			m_GraphicThingInstance.SetScale(1.0f, 1.0f, 1.0f);
			m_GraphicThingInstance.SetScalePosition(CRaceData::PART_ACCE, 0.f, 0.f, 0.f);
			m_awPart[CRaceData::PART_ACCE] = 0;
			return true;
		}
		else
		{


			CItemData * pItemData;
			if (!CItemManager::Instance().GetItemDataPointer(eAcce, &pItemData))
			{
				__ClearAcceRefineEffect();
				m_GraphicThingInstance.SetScale(1.0f, 1.0f, 1.0f);
				m_GraphicThingInstance.SetScalePosition(CRaceData::PART_ACCE, 0.f, 0.f, 0.f);
				m_awPart[CRaceData::PART_ACCE] = 0;
				return true;
			}
			else
			{


				BYTE byRace = (BYTE)GetRace();
				BYTE byJob = (BYTE)RaceToJob(byRace);
				BYTE bySex = (BYTE)RaceToSex(byRace);

				D3DXVECTOR3 & scalePos = pItemData->GetItemScalePosition(byJob, bySex);



				if (eAcce == 85004 || eAcce == 85008 || eAcce == 85015 || eAcce == 85018 || eAcce == 85024)
				{

					m_acceEffect = EFFECT_REFINED + EFFECT_ACCE_SPECIAL;
					__EffectContainer_AttachEffect(m_acceEffect);

				}

				m_GraphicThingInstance.SetScaleNew(CRaceData::PART_ACCE, pItemData->GetItemScale(byJob, bySex));
				m_GraphicThingInstance.SetScalePosition(CRaceData::PART_ACCE, scalePos);
				m_awPart[CRaceData::PART_ACCE] = eAcce;
			}

		}
	}
	return true;
}

/*void CInstanceBase::SetAcce(DWORD eAcce)
{
	if (IsPC() == false)
		return;

	__ClearAcceRefineEffect();
	float fSpecular = 0.0f;
	int iAcce = eAcce + 65536;
	if (iAcce == 85004 || iAcce == 85008 || iAcce == 85015 || iAcce == 85018)
	{
		fSpecular = 100.0f;
		m_acceEffect = EFFECT_REFINED + EFFECT_ACCE_SPECIAL;
		__EffectContainer_AttachEffect(m_acceEffect);
	}

	m_awPart[CRaceData::PART_ACCE] = iAcce;
	m_GraphicThingInstance.AttachAcce(iAcce, 0, CRaceData::PART_ACCE);

	CItemData * pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(iAcce, &pItemData))
	{

		__ClearAcceRefineEffect();
		m_GraphicThingInstance.SetScale(1.0f, 1.0f, 1.0f);
		m_GraphicThingInstance.SetScalePosition(CRaceData::PART_ACCE, 0.f, 0.f, 0.f);
		m_awPart[CRaceData::PART_ACCE] = 0;
	}
	else
	{
		BYTE byRace = (BYTE)GetRace();
		BYTE byJob = (BYTE)RaceToJob(byRace);
		BYTE bySex = (BYTE)RaceToSex(byRace);

		D3DXVECTOR3 & scalePos = pItemData->GetItemScalePosition(byJob, bySex);
		if (IsMountingHorseOnly() != 0 && byJob != NRaceData::JOB_WOLFMAN)
			scalePos.z += 15.f;

		m_GraphicThingInstance.SetScaleNew(CRaceData::PART_ACCE, pItemData->GetItemScale(byJob, bySex));
		m_GraphicThingInstance.SetScalePosition(CRaceData::PART_ACCE, scalePos);

		m_GraphicThingInstance.SetAcce(iAcce, fSpecular);
	}
}*/

/*bool CInstanceBase::SetAcce(DWORD eAcce)
{

	if (IsPoly())
		return false;

	if (__IsShapeAnimalWear())
		return false;

	//TraceError("eAcce %d",eAcce);
	eAcce = eAcce + 65536;
	if (m_awPart[CRaceData::PART_ACCE] == eAcce)
		return false;

	m_GraphicThingInstance.AttachAcce(eAcce, 0, CRaceData::PART_ACCE);

	DWORD Race = GetRace();
	DWORD Job = RaceToJob(Race);
	DWORD Sex = RaceToSex(Race);

	D3DXVECTOR3 pos;
	D3DXVECTOR3 scale;

	switch (Race)
	{
	case 1: // Guerrier
	{
				switch (Sex)
				{
				case 0: // Homme
				{
							if (eAcce >= 85001 && eAcce <= 85004)
							{
								pos = D3DXVECTOR3(30.0f, 30.0f, 30.0f);
								scale = D3DXVECTOR3(0.78f, 0.83f, 0.78f);
							}
							if (eAcce >= 85005 && eAcce <= 85008)
							{
								pos = D3DXVECTOR3(27.0f, 27.0f, 27.0f);
								scale = D3DXVECTOR3(0.78f, 0.83f, 0.78f);
							}
							if (eAcce >= 85011 && eAcce <= 85014)
							{
								pos = D3DXVECTOR3(15.0f, 15.0f, 15.0f);
								scale = D3DXVECTOR3(0.83f, 0.83f, 0.83f);
							}
							if (eAcce >= 85015 && eAcce <= 85018)
							{
								pos = D3DXVECTOR3(14.0f, 14.0f, 14.0f);
								scale = D3DXVECTOR3(0.83f, 0.83f, 0.83f);
							}
							break;
				}
				case 1: // Femme
				{
							if (eAcce >= 85001 && eAcce <= 85004)
							{
								pos = D3DXVECTOR3(40.0f, 40.0f, 40.0f);
								scale = D3DXVECTOR3(0.70f, 0.70f, 0.70f);
							}
							if (eAcce >= 85005 && eAcce <= 85008)
							{
								pos = D3DXVECTOR3(40.0f, 40.0f, 40.0f);
								scale = D3DXVECTOR3(0.70f, 0.70f, 0.70f);
							}
							if (eAcce >= 85011 && eAcce <= 85014)
							{
								pos = D3DXVECTOR3(26.0f, 26.0f, 26.0f);
								scale = D3DXVECTOR3(0.72f, 0.72f, 0.72f);
							}
							if (eAcce >= 85015 && eAcce <= 85018)
							{
								pos = D3DXVECTOR3(26.0f, 26.0f, 26.0f);
								scale = D3DXVECTOR3(0.72f, 0.75f, 0.72f);
							}
							break;
				}
				}
				break;
	}
	case 2: // Ninja
	{
				switch (Sex)
				{
				case 0: // Homme
				{
							if (eAcce >= 85001 && eAcce <= 85004)
							{
								pos = D3DXVECTOR3(52.0f, 52.0f, 52.0f);
								scale = D3DXVECTOR3(0.62f, 0.68f, 0.62f);
							}
							if (eAcce >= 85005 && eAcce <= 85008)
							{
								pos = D3DXVECTOR3(50.0f, 50.0f, 50.0f);
								scale = D3DXVECTOR3(0.62f, 0.68f, 0.62f);
							}
							if (eAcce >= 85011 && eAcce <= 85014)
							{
								pos = D3DXVECTOR3(26.0f, 26.0f, 26.0f);
								scale = D3DXVECTOR3(0.72f, 0.72f, 0.72f);
							}
							if (eAcce >= 85015 && eAcce <= 85018)
							{
								pos = D3DXVECTOR3(24.0f, 24.0f, 24.0f);
								scale = D3DXVECTOR3(0.73f, 0.73f, 0.73f);
							}
							break;
				}
				case 1: // Femme
				{
							if (eAcce >= 85001 && eAcce <= 85004)
							{
								pos = D3DXVECTOR3(45.0f, 45.0f, 45.0f);
								scale = D3DXVECTOR3(0.62f, 0.62f, 0.62f);
							}
							if (eAcce >= 85005 && eAcce <= 85008)
							{
								pos = D3DXVECTOR3(45.0f, 45.0f, 45.0f);
								scale = D3DXVECTOR3(0.62f, 0.62f, 0.62f);
							}
							if (eAcce >= 85011 && eAcce <= 85014)
							{
								pos = D3DXVECTOR3(38.0f, 38.0f, 38.0f);
								scale = D3DXVECTOR3(0.54f, 0.65f, 0.62f);
							}
							if (eAcce >= 85015 && eAcce <= 85018)
							{
								pos = D3DXVECTOR3(38.0f, 38.0f, 38.0f);
								scale = D3DXVECTOR3(0.54f, 0.65f, 0.62f);
							}
							break;
				}
				}
				break;
	}
	case 3: // Sura
	{
				switch (Sex)
				{
				case 0: // Homme
				{
							if (eAcce >= 85001 && eAcce <= 85004)
							{
								pos = D3DXVECTOR3(40.0f, 40.0f, 40.0f);
								scale = D3DXVECTOR3(0.73f, 0.77f, 0.73f);
							}
							if (eAcce >= 85005 && eAcce <= 85008)
							{
								pos = D3DXVECTOR3(40.0f, 40.0f, 40.0f);
								scale = D3DXVECTOR3(0.73f, 0.77f, 0.73f);
							}
							if (eAcce >= 85011 && eAcce <= 85014)
							{
								pos = D3DXVECTOR3(21.0f, 21.0f, 21.0f);
								scale = D3DXVECTOR3(0.81f, 0.81f, 0.81f);
							}
							if (eAcce >= 85015 && eAcce <= 85018)
							{
								pos = D3DXVECTOR3(14.0f, 14.0f, 14.0f);
								scale = D3DXVECTOR3(0.85f, 0.85f, 0.85f);
							}
							break;
				}
				case 1: // Femme
				{
							if (eAcce >= 85001 && eAcce <= 85004)
							{
								pos = D3DXVECTOR3(40.0f, 40.0f, 40.0f);
								scale = D3DXVECTOR3(0.65f, 0.70f, 0.65f);
							}
							if (eAcce >= 85005 && eAcce <= 85008)
							{
								pos = D3DXVECTOR3(40.0f, 40.0f, 40.0f);
								scale = D3DXVECTOR3(0.65f, 0.70f, 0.65f);
							}
							if (eAcce >= 85011 && eAcce <= 85014)
							{
								pos = D3DXVECTOR3(30.0f, 30.0f, 30.0f);
								scale = D3DXVECTOR3(0.62f, 0.72f, 0.72f);
							}
							if (eAcce >= 85015 && eAcce <= 85018)
							{
								pos = D3DXVECTOR3(45.0f, 45.0f, 45.0f);
								scale = D3DXVECTOR3(0.62f, 0.62f, 0.62f);
							}
							break;
				}
				}
				break;
	}
	case 4: // Shaman
	{
				switch (Sex)
				{
				case 0: // Homme
				{
							if (eAcce >= 85001 && eAcce <= 85004)
							{
								pos = D3DXVECTOR3(30.0f, 30.0f, 30.0f);
								scale = D3DXVECTOR3(0.75f, 0.75f, 0.75f);
							}
							if (eAcce >= 85005 && eAcce <= 85008)
							{
								pos = D3DXVECTOR3(30.0f, 30.0f, 30.0f);
								scale = D3DXVECTOR3(0.75f, 0.75f, 0.75f);
							}
							if (eAcce >= 85011 && eAcce <= 85014)
							{
								pos = D3DXVECTOR3(18.0f, 18.0f, 18.0f);
								scale = D3DXVECTOR3(0.80f, 0.80f, 0.80f);
							}
							if (eAcce >= 85015 && eAcce <= 85018)
							{
								pos = D3DXVECTOR3(20.0f, 20.0f, 20.0f);
								scale = D3DXVECTOR3(0.78f, 0.78f, 0.78f);
							}
							break;
				}
				case 1: // Femme
				{
							if (eAcce >= 85001 && eAcce <= 85004)
							{
								pos = D3DXVECTOR3(43.0f, 43.0f, 43.0f);
								scale = D3DXVECTOR3(0.65f, 0.65f, 0.65f);
							}
							if (eAcce >= 85005 && eAcce <= 85008)
							{
								pos = D3DXVECTOR3(43.0f, 43.0f, 43.0f);
								scale = D3DXVECTOR3(0.65f, 0.65f, 0.65f);
							}
							if (eAcce >= 85011 && eAcce <= 85014)
							{
								pos = D3DXVECTOR3(26.0f, 26.0f, 26.0f);
								scale = D3DXVECTOR3(0.72f, 0.72f, 0.72f);
							}
							if (eAcce >= 85015 && eAcce <= 85018)
							{
								pos = D3DXVECTOR3(45.0f, 45.0f, 45.0f);
								scale = D3DXVECTOR3(0.62f, 0.62f, 0.62f);
							}
							break;
				}
				}
				break;
	}
	case 5: // Lycan
	{
				pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
				scale = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
				break;
	}

	}


	if (IsMountingHorse()) {
		pos = pos + D3DXVECTOR3(0.0f, -3.0f, 15.0f);
	}

	m_GraphicThingInstance.SetScaleNew(CRaceData::PART_ACCE, scale);
	m_GraphicThingInstance.SetScalePosition(CRaceData::PART_ACCE, pos);
	m_awPart[CRaceData::PART_ACCE] = eAcce;




	return true;
}
*/

void CInstanceBase::ChangeAcce(DWORD eAcce)
{
	if (IsPC() == false)
		return;

	SetAcce(eAcce);
}

void CInstanceBase::SetArmor(DWORD dwArmor)
{
	DWORD dwShape;
	if (__ArmorVnumToShape(dwArmor, &dwShape))
	{
		CItemData * pItemData;
		if (CItemManager::Instance().GetItemDataPointer(dwArmor, &pItemData))
		{
			float fSpecularPower=pItemData->GetSpecularPowerf();
			SetShape(dwShape, fSpecularPower);
			__GetRefinedEffect(pItemData);
			return;
		}
		else
			__ClearArmorRefineEffect();
	}

	SetShape(dwArmor);
}

void CInstanceBase::SetShape(DWORD eShape, float fSpecular)
{
	if (IsPoly())
	{
		m_GraphicThingInstance.SetShape(0);	
	}
	else
	{
		m_GraphicThingInstance.SetShape(eShape, fSpecular);		
	}

	m_eShape = eShape;
}



DWORD CInstanceBase::GetWeaponType()
{
	DWORD dwWeapon = GetPart(CRaceData::PART_WEAPON);
	CItemData * pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwWeapon, &pItemData))
		return CItemData::WEAPON_NONE;

	if (pItemData->GetType() == CItemData::ITEM_TYPE_COSTUME)
	{
		if (pItemData->GetSubType() == CItemData::COSTUME_WEAPON_SWORD)
			return CItemData::WEAPON_SWORD;
		else if (pItemData->GetSubType() == CItemData::COSTUME_WEAPON_DAGGER)
			return CItemData::WEAPON_DAGGER;
		else if (pItemData->GetSubType() == CItemData::COSTUME_WEAPON_BOW)
			return CItemData::WEAPON_BOW;
		else if (pItemData->GetSubType() == CItemData::COSTUME_WEAPON_TWO_HANDED)
			return CItemData::WEAPON_TWO_HANDED;
		else if (pItemData->GetSubType() == CItemData::COSTUME_WEAPON_BELL)
			return CItemData::WEAPON_BELL;
		else if (pItemData->GetSubType() == CItemData::COSTUME_WEAPON_FAN)
			return CItemData::WEAPON_FAN;
		else if (pItemData->GetSubType() == CItemData::COSTUME_WEAPON_CLAW)
			return CItemData::WEAPON_CLAW;
		else
			return pItemData->GetWeaponType();
	}

	return pItemData->GetWeaponType();
}

/*
void CInstanceBase::SetParts(const WORD * c_pParts)
{
	if (IsPoly())
		return;

	if (__IsShapeAnimalWear())
		return;

	UINT eWeapon=c_pParts[CRaceData::PART_WEAPON];

	if (__IsChangableWeapon(eWeapon) == false)
			eWeapon = 0;

	if (eWeapon != m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON))
	{
		m_GraphicThingInstance.AttachPart(CRaceData::PART_MAIN, CRaceData::PART_WEAPON, eWeapon);
		m_awPart[CRaceData::PART_WEAPON] = eWeapon;
	}

	__AttachHorseSaddle();
}
*/


struct TAttachingEffect
{
	DWORD dwEffectIndex;
	int iBoneIndex;
	DWORD dwModelIndex;
	D3DXMATRIX matTranslation;
	BOOL isAttaching;

	int iLifeType;
	DWORD dwEndTime;
};

void CInstanceBase::__ClearWeaponRefineEffect()
{

	if (m_swordRefineEffectRight)
	{
		__DetachEffect(m_swordRefineEffectRight);
		m_swordRefineEffectRight = 0;
	}
	if (m_swordRefineEffectLeft)
	{
		__DetachEffect(m_swordRefineEffectLeft);
		m_swordRefineEffectLeft = 0;
	}
	if (DernierIdEffet.size() > 0)
	{
		for (int i = 0; i < DernierIdEffet.size(); i++)
		{
			m_GraphicThingInstance.DettachEffect(DernierIdEffet[i]);
		}
	}
	DernierIdEffet.clear();
}

void CInstanceBase::__ClearAcceRefineEffect()
{
	if (m_acceEffect)
	{
		__EffectContainer_DetachEffect(EFFECT_REFINED + EFFECT_ACCE_SPECIAL);
		m_acceEffect = 0;
	}
}

void CInstanceBase::__ClearArmorRefineEffect()
{
	if (m_armorRefineEffect)
	{
		__DetachEffect(m_armorRefineEffect);
		m_armorRefineEffect = 0;
	}
}


UINT CInstanceBase::__GetRefinedEffect(CItemData* pItem)
{

	DWORD refine = max(pItem->GetRefine() + pItem->GetSocketCount(), CItemData::ITEM_SOCKET_MAX_NUM) - CItemData::ITEM_SOCKET_MAX_NUM;
	switch (pItem->GetType())
	{
	case CItemData::ITEM_TYPE_WEAPON:
		__ClearWeaponRefineEffect();
		if (refine < 7)	//현재 제련도 7 이상만 이펙트가 있습니다.
			return 0;
		switch (pItem->GetSubType())
		{
		case CItemData::COSTUME_WEAPON_DAGGER:
		case CItemData::WEAPON_DAGGER:
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7 + refine - 7;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7_LEFT + refine - 7;
			break;
		case CItemData::COSTUME_WEAPON_FAN:
		case CItemData::WEAPON_FAN:
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FANBELL_REFINED7 + refine - 7;
			break;
		case CItemData::COSTUME_WEAPON_BELL:
		case CItemData::WEAPON_BELL:
		case CItemData::WEAPON_ARROW:
#ifdef ENABLE_QUIVER_SYSTEM
		case CItemData::WEAPON_QUIVER:
#endif
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7 + refine - 7;
			break;
		case CItemData::COSTUME_WEAPON_BOW:
		case CItemData::WEAPON_BOW:
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_BOW_REFINED7 + refine - 7;
			break;
		case CItemData::COSTUME_WEAPON_CLAW:
		case CItemData::WEAPON_CLAW:
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7 + refine - 7;
			m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7_LEFT + refine - 7;
			break;
		default:
			m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_REFINED7 + refine - 7;
		}
		if (m_swordRefineEffectRight)
			m_swordRefineEffectRight = __AttachEffect(m_swordRefineEffectRight);
		if (m_swordRefineEffectLeft)
			m_swordRefineEffectLeft = __AttachEffect(m_swordRefineEffectLeft);
		break;
	case CItemData::ITEM_TYPE_COSTUME:
		__ClearArmorRefineEffect();
		if (pItem->GetSubType() == CItemData::COSTUME_ACCE)
		{

			__AttachEffect(EFFECT_REFINED + EFFECT_BODYARMOR_SPECIAL);
			__AttachEffect(EFFECT_REFINED + EFFECT_BODYARMOR_SPECIAL2);

		}


	case CItemData::ITEM_TYPE_ARMOR:
		__ClearArmorRefineEffect();



		// 갑옷 특화 이펙트
		if (pItem->GetSubType() == CItemData::ARMOR_BODY || pItem->GetSubType() == CItemData::COSTUME_BODY)
		{
			DWORD vnum = pItem->GetIndex();

			if (12010 <= vnum && vnum <= 12049) // Si c'est les armures 90.
			{
				__AttachEffect(EFFECT_REFINED + EFFECT_BODYARMOR_SPECIAL);
				__AttachEffect(EFFECT_REFINED + EFFECT_BODYARMOR_SPECIAL2);
			}
			/*if (pItem->GetType() == CItemData::ITEM_TYPE_COSTUME && pItem->GetSubType() == CItemData::COSTUME_BODY)
			{ // Si c'est un costume.
				__AttachEffect(EFFECT_REFINED + EFFECT_BODYARMOR_SPECIAL);
				__AttachEffect(EFFECT_REFINED + EFFECT_BODYARMOR_SPECIAL2);
			}*/

			// Si c'est une armure diff?ente des 90.
			if (!shiningdata.empty()){
				for (shiningit = shiningdata.begin(); shiningit != shiningdata.end(); shiningit++)
				if (shiningit->first == vnum) {
					std::string substr(shiningit->second);
					std::vector<string> chars;
					boost::split(chars, substr, boost::is_any_of("#"));

					for (std::vector<string>::size_type i = 0; i != chars.size(); i++) {
						__AttachEffectToArmours(chars[i], 1);
					}

				}
			}
		}


		if (refine < 7)	//현재 제련도 7 이상만 이펙트가 있습니다.
			return 0;

		if (pItem->GetSubType() == CItemData::ARMOR_BODY)
		{
			m_armorRefineEffect = EFFECT_REFINED + EFFECT_BODYARMOR_REFINED7 + refine - 7;
			__AttachEffect(m_armorRefineEffect);
		}

		break;
	}
	if (pItem->GetType() == CItemData::ITEM_TYPE_WEAPON)
	{

		int type = 0;
		switch (pItem->GetSubType())
		{
		case CItemData::WEAPON_SWORD:
		case CItemData::WEAPON_FAN:
		case CItemData::WEAPON_BELL:
			if (GetRace() == 0 || GetRace() == 4)
			{
				type = 4;
			}
			else
			{
				type = 2;
			}
			break;
		case CItemData::WEAPON_ARROW:
		case CItemData::WEAPON_BOW:
			type = 5;
			break;
		case CItemData::WEAPON_TWO_HANDED:
			type = 4;
			break;
		default:
			type = 3;
			break;
		}

		if (type == 3) // Double dague, griffe
		{

			if (!shiningdata.empty()){
				for (shiningit = shiningdata.begin(); shiningit != shiningdata.end(); shiningit++)
				if (shiningit->first == pItem->GetIndex()) {
					std::string substr(shiningit->second);
					std::vector<string> chars;
					boost::split(chars, substr, boost::is_any_of("#"));

					for (std::vector<string>::size_type i = 0; i != chars.size(); i++) {
						DernierIdEffet.push_back(__AttachEffectToArmours(chars[i], 2));
					}

				}
			}

			if (!shiningdata.empty()){
				for (shiningit = shiningdata.begin(); shiningit != shiningdata.end(); shiningit++)
				if (shiningit->first == pItem->GetIndex()) {
					std::string substr(shiningit->second);
					std::vector<string> chars;
					boost::split(chars, substr, boost::is_any_of("#"));

					for (std::vector<string>::size_type i = 0; i != chars.size(); i++) {
						DernierIdEffet.push_back(__AttachEffectToArmours(chars[i], 3));
					}

				}
			}
		}
		else
		{
			if (!shiningdata.empty()){
				for (shiningit = shiningdata.begin(); shiningit != shiningdata.end(); shiningit++)
				if (shiningit->first == pItem->GetIndex()) {
					std::string substr(shiningit->second);
					std::vector<string> chars;
					boost::split(chars, substr, boost::is_any_of("#"));

					for (std::vector<string>::size_type i = 0; i != chars.size(); i++) {
						DernierIdEffet.push_back(__AttachEffectToArmours(chars[i], type));
					}

				}
			}
		}
	}
	return 0;
}

bool CInstanceBase::SetWeapon(DWORD eWeapon)
{
	if (IsPoly())
		return false;
	
	if (__IsShapeAnimalWear())
		return false;
	
	if (__IsChangableWeapon(eWeapon) == false)
		eWeapon = 0;

	m_GraphicThingInstance.AttachWeapon(eWeapon, false);
	m_awPart[CRaceData::PART_WEAPON] = eWeapon;
	
	//Weapon Effect
	CItemData * pItemData;
	if (CItemManager::Instance().GetItemDataPointer(eWeapon, &pItemData))
		__GetRefinedEffect(pItemData);
	else
		__ClearWeaponRefineEffect();

	return true;
}

void CInstanceBase::ChangeWeapon(DWORD eWeapon)
{
	if (eWeapon == m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON))
		return;

	if (SetWeapon(eWeapon))
		RefreshState(CRaceMotionData::NAME_WAIT, true);
}

CActorInstance CInstanceBase::GetActorInstance()
{

	return m_GraphicThingInstance;
}

bool CInstanceBase::ChangeArmor(DWORD dwArmor)
{
	DWORD eShape;
	__ArmorVnumToShape(dwArmor, &eShape);

	if (GetShape()==eShape)
		return false;

	CAffectFlagContainer kAffectFlagContainer;
	kAffectFlagContainer.CopyInstance(m_kAffectFlagContainer);

	DWORD dwVID = GetVirtualID();
	DWORD dwRace = GetRace();
	DWORD eHair = GetPart(CRaceData::PART_HAIR);
	DWORD eAcce = GetPart(CRaceData::PART_ACCE);
	DWORD eWeapon = GetPart(CRaceData::PART_WEAPON);
	float fRot = GetRotation();
	float fAdvRot = GetAdvancingRotation();

	if (IsWalking())
		EndWalking();

	// 2004.07.25.myevan.ÀÌÆåÆ® ¾È ºÙ´Â ¹®Á¦
	//////////////////////////////////////////////////////
	__ClearAffects();
	//////////////////////////////////////////////////////

	if (!SetRace(dwRace))
	{
		TraceError("CPythonCharacterManager::ChangeArmor - SetRace VID[%d] Race[%d] ERROR", dwVID, dwRace);
		return false;
	}

	SetArmor(dwArmor);
	SetHair(eHair);
	SetAcce(eAcce);
	SetWeapon(eWeapon);

	SetRotation(fRot);
	SetAdvancingRotation(fAdvRot);

	__AttachHorseSaddle();

	RefreshState(CRaceMotionData::NAME_WAIT, TRUE);

	// 2004.07.25.myevan.ÀÌÆåÆ® ¾È ºÙ´Â ¹®Á¦
	/////////////////////////////////////////////////
	SetAffectFlagContainer(kAffectFlagContainer);
	/////////////////////////////////////////////////

	CActorInstance::IEventHandler& rkEventHandler=GetEventHandlerRef();
	rkEventHandler.OnChangeShape();

	return true;
}

bool CInstanceBase::__IsShapeAnimalWear()
{
	if (100 == GetShape() ||
		101 == GetShape() ||
		102 == GetShape() ||
		103 == GetShape())
		return true;

	return false;
}

DWORD CInstanceBase::__GetRaceType()
{
	return m_eRaceType;
}


void CInstanceBase::RefreshState(DWORD dwMotIndex, bool isLoop)
{
	DWORD dwPartItemID = m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON);

	BYTE byItemType = 0xff;
	BYTE bySubType = 0xff;

	CItemManager & rkItemMgr = CItemManager::Instance();
	CItemData * pItemData;

	if (rkItemMgr.GetItemDataPointer(dwPartItemID, &pItemData))
	{
		byItemType = pItemData->GetType();
		bySubType = pItemData->GetWeaponType();
	}

	if (IsPoly())
	{
		SetMotionMode(CRaceMotionData::MODE_GENERAL);
	}
	else if (IsWearingDress())
	{
		SetMotionMode(CRaceMotionData::MODE_WEDDING_DRESS);
	}
	else if (IsHoldingPickAxe())
	{
		if (m_kHorse.IsMounting())
		{
			SetMotionMode(CRaceMotionData::MODE_HORSE);
		}
		else
		{
			SetMotionMode(CRaceMotionData::MODE_GENERAL);
		}
	}
	else if (CItemData::ITEM_TYPE_ROD == byItemType)
	{
		if (m_kHorse.IsMounting())
		{
			SetMotionMode(CRaceMotionData::MODE_HORSE);
		}
		else
		{
			SetMotionMode(CRaceMotionData::MODE_FISHING);
		}
	}
	else if (m_kHorse.IsMounting())
	{
		switch (bySubType)
		{
		case CItemData::COSTUME_WEAPON_SWORD:
		case CItemData::WEAPON_SWORD:
			SetMotionMode(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD);
			break;
		case CItemData::COSTUME_WEAPON_TWO_HANDED:
		case CItemData::WEAPON_TWO_HANDED:
			SetMotionMode(CRaceMotionData::MODE_HORSE_TWOHAND_SWORD); // Only Warrior
			break;
		case CItemData::COSTUME_WEAPON_DAGGER:
		case CItemData::WEAPON_DAGGER:
			SetMotionMode(CRaceMotionData::MODE_HORSE_DUALHAND_SWORD); // Only Assassin
			break;
		case CItemData::COSTUME_WEAPON_FAN:
		case CItemData::WEAPON_FAN:
			SetMotionMode(CRaceMotionData::MODE_HORSE_FAN); // Only Shaman
			break;
		case CItemData::COSTUME_WEAPON_BELL:
		case CItemData::WEAPON_BELL:
			SetMotionMode(CRaceMotionData::MODE_HORSE_BELL); // Only Shaman
			break;
		case CItemData::COSTUME_WEAPON_BOW:
		case CItemData::WEAPON_BOW:
			SetMotionMode(CRaceMotionData::MODE_HORSE_BOW); // Only Shaman
			break;
		case CItemData::COSTUME_WEAPON_CLAW:
		case CItemData::WEAPON_CLAW:
			SetMotionMode(CRaceMotionData::MODE_HORSE_CLAW); // Only Lycan
			break;


		default:
			SetMotionMode(CRaceMotionData::MODE_HORSE);
			break;
		}
	}
	else
	{
		switch (bySubType)
		{
		case CItemData::COSTUME_WEAPON_SWORD:
		case CItemData::WEAPON_SWORD:
			SetMotionMode(CRaceMotionData::MODE_ONEHAND_SWORD);
			break;
		case CItemData::COSTUME_WEAPON_TWO_HANDED:
		case CItemData::WEAPON_TWO_HANDED:
			SetMotionMode(CRaceMotionData::MODE_TWOHAND_SWORD); // Only Warrior
			break;
		case CItemData::COSTUME_WEAPON_DAGGER:
		case CItemData::WEAPON_DAGGER:
			SetMotionMode(CRaceMotionData::MODE_DUALHAND_SWORD); // Only Assassin
			break;
		case CItemData::COSTUME_WEAPON_BOW:
		case CItemData::WEAPON_BOW:
			SetMotionMode(CRaceMotionData::MODE_BOW); // Only Assassin
			break;
		case CItemData::COSTUME_WEAPON_FAN:
		case CItemData::WEAPON_FAN:
			SetMotionMode(CRaceMotionData::MODE_FAN); // Only Shaman
			break;
		case CItemData::COSTUME_WEAPON_BELL:
		case CItemData::WEAPON_BELL:
			SetMotionMode(CRaceMotionData::MODE_BELL); // Only Shaman
			break;

		case CItemData::COSTUME_WEAPON_CLAW:
		case CItemData::WEAPON_CLAW:
			SetMotionMode(CRaceMotionData::MODE_CLAW);  // Only Lycan
			break;


		case CItemData::WEAPON_ARROW:

		default:
			SetMotionMode(CRaceMotionData::MODE_GENERAL);
			break;
		}
	}

	if (isLoop)
		m_GraphicThingInstance.InterceptLoopMotion(dwMotIndex);
	else
		m_GraphicThingInstance.InterceptOnceMotion(dwMotIndex);

	RefreshActorInstance();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Device

void CInstanceBase::RegisterBoundingSphere()
{
	// Stone ÀÏ °æ¿ì DeforomNoSkin À» ÇÏ¸é
	// ³«ÇÏÇÏ´Â ¾Ö´Ï¸ÞÀÌ¼Ç °°Àº °æ¿ì ¾Ö´Ï¸ÞÀÌ¼ÇÀÌ
	// ¹Ù¿îµå ¹Ú½º¿¡ ¿µÇâÀ» ¹ÌÃÄ ÄÃ¸µÀÌ Á¦´ë·Î ÀÌ·ç¾îÁöÁö ¾Ê´Â´Ù.
	if (!IsStone())
	{
		m_GraphicThingInstance.DeformNoSkin();
	}

	m_GraphicThingInstance.RegisterBoundingSphere();
}

bool CInstanceBase::CreateDeviceObjects()
{
	return m_GraphicThingInstance.CreateDeviceObjects();
}

void CInstanceBase::DestroyDeviceObjects()
{
	m_GraphicThingInstance.DestroyDeviceObjects();
}

void CInstanceBase::Destroy()
{	
	DetachTextTail();
	
	DismountHorse();

	m_kQue_kCmdNew.clear();
	
	__EffectContainer_Destroy();
	__StoneSmoke_Destroy();

	if (__IsMainInstance())
		__ClearMainInstance();	
	
	m_GraphicThingInstance.Destroy();
	
	__Initialize();
}

void CInstanceBase::__InitializeRotationSpeed()
{
	SetRotationSpeed(c_fDefaultRotationSpeed);
}

void CInstanceBase::__Warrior_Initialize()
{
	m_kWarrior.m_dwGeomgyeongEffect=0;
}

void CInstanceBase::__Initialize()
{
	__Warrior_Initialize();
	__StoneSmoke_Inialize();
	__EffectContainer_Initialize();
	__InitializeRotationSpeed();

	SetEventHandler(CActorInstance::IEventHandler::GetEmptyPtr());

	m_kAffectFlagContainer.Clear();
	m_dwAIFlag = 0;
	m_dwLevel = 0;
	m_dwTitle = 0;
	m_dw7emeSkill = 0;
	m_dw8emeSKill = 0;
	m_dwGuildID = 0;
	m_dwEmpireID = 0;

	m_nomTitre = "";
	m_couleurtitre1 = "";
	m_couleurtitre2 = "";
	m_couleurtitre3 = "";
	m_eType = 0;
	m_eRaceType = 0;
	m_eShape = 0;
	m_dwRace = 0;
	m_dwVirtualNumber = 0;

	m_dwBaseCmdTime=0;
	m_dwBaseChkTime=0;
	m_dwSkipTime=0;

	m_GraphicThingInstance.Initialize();

	m_dwAdvActorVID=0;
	m_dwLastDmgActorVID=0;

	m_nAverageNetworkGap=0;
	m_dwNextUpdateHeightTime=0;

	// Moving by keyboard
	m_iRotatingDirection = DEGREE_DIRECTION_SAME;

	// Moving by mouse	
	m_isTextTail = FALSE;
	m_isGoing = FALSE;
	NEW_SetSrcPixelPosition(TPixelPosition(0, 0, 0));
	NEW_SetDstPixelPosition(TPixelPosition(0, 0, 0));

	m_kPPosDust = TPixelPosition(0, 0, 0);


	m_kQue_kCmdNew.clear();

	m_dwLastComboIndex = 0;

	m_swordRefineEffectRight = 0;
	m_swordRefineEffectLeft = 0;
	m_armorRefineEffect = 0;
	m_acceEffect = 0;


	m_sAlignment = 0;
	m_byPKMode = 0;
	m_isKiller = false;
	m_isPartyMember = false;

	m_bEnableTCPState = TRUE;

	m_stName = "";

	memset(m_awPart, 0, sizeof(m_awPart));
	memset(m_adwCRCAffectEffect, 0, sizeof(m_adwCRCAffectEffect));
	//memset(m_adwCRCEmoticonEffect, 0, sizeof(m_adwCRCEmoticonEffect));
	memset(&m_kMovAfterFunc, 0, sizeof(m_kMovAfterFunc));

	m_bDamageEffectType = false;
	m_dwDuelMode = DUEL_NONE;
	m_dwEmoticonTime = 0;
	m_Nage = false;
}

CInstanceBase::CInstanceBase()
{
	__Initialize();
}

CInstanceBase::~CInstanceBase()
{
	Destroy();
}


void CInstanceBase::GetBoundBox(D3DXVECTOR3 * vtMin, D3DXVECTOR3 * vtMax)
{
	m_GraphicThingInstance.GetBoundBox(vtMin, vtMax);
}

