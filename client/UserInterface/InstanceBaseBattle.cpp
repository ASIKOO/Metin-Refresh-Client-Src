#include "StdAfx.h"
#include "InstanceBase.h"
#include "PythonBackground.h"
#include "PythonCharacterManager.h"
#include "../PRTerrainLib/Terrain.h"

float NEW_UnsignedDegreeToSignedDegree(float fUD)
{
	float fSD;
	if (fUD>180.0f)
		fSD=-(360.0f-fUD);
	else if (fUD<-180.0f)
		fSD=+(360.0f+fUD);
	else
		fSD=fUD;

	return fSD;
}

float NEW_GetSignedDegreeFromDirPixelPosition(const TPixelPosition& kPPosDir)
{
	D3DXVECTOR3 vtDir(kPPosDir.x, -kPPosDir.y, kPPosDir.z);
	D3DXVECTOR3 vtDirNormal;
	D3DXVec3Normalize(&vtDirNormal, &vtDir);

	D3DXVECTOR3 vtDirNormalStan(0, -1, 0);
	float fDirRot = D3DXToDegree(acosf(D3DXVec3Dot(&vtDirNormal, &vtDirNormalStan)));

	if (vtDirNormal.x<0.0f)
		fDirRot=-fDirRot;

	return fDirRot;
}

bool CInstanceBase::IsFlyTargetObject()
{
	return m_GraphicThingInstance.IsFlyTargetObject();
}

float CInstanceBase::GetFlyTargetDistance()
{
	return m_GraphicThingInstance.GetFlyTargetDistance();
}

void CInstanceBase::ClearFlyTargetInstance()
{
	m_GraphicThingInstance.ClearFlyTarget();
}

void CInstanceBase::SetFlyTargetInstance(CInstanceBase& rkInstDst)
{
// NOTE : NEW_Attack ¶§ TargetÀ» ¹Ù²Ü¶§ ¿©±â¼­ ¸®ÅÏ µÇ¾î¹ö¸² - [levites]
//	if (isLock())
//		return;

	m_GraphicThingInstance.SetFlyTarget(rkInstDst.GetGraphicThingInstancePtr());
}

void CInstanceBase::AddFlyTargetPosition(const TPixelPosition& c_rkPPosDst)
{
	m_GraphicThingInstance.AddFlyTarget(c_rkPPosDst);
}

void CInstanceBase::AddFlyTargetInstance(CInstanceBase& rkInstDst)
{
	m_GraphicThingInstance.AddFlyTarget(rkInstDst.GetGraphicThingInstancePtr());
}


float CInstanceBase::NEW_GetDistanceFromDestInstance(CInstanceBase& rkInstDst)
{
	TPixelPosition kPPosDst;
	rkInstDst.NEW_GetPixelPosition(&kPPosDst);

	return NEW_GetDistanceFromDestPixelPosition(kPPosDst);
}

float CInstanceBase::NEW_GetDistanceFromDestPixelPosition(const TPixelPosition& c_rkPPosDst)
{
	TPixelPosition kPPosCur;
	NEW_GetPixelPosition(&kPPosCur);

	TPixelPosition kPPosDir;
	kPPosDir=c_rkPPosDst-kPPosCur;

	return NEW_GetDistanceFromDirPixelPosition(kPPosDir);
}

float CInstanceBase::NEW_GetDistanceFromDirPixelPosition(const TPixelPosition& c_rkPPosDir)
{
	return sqrtf(c_rkPPosDir.x*c_rkPPosDir.x+c_rkPPosDir.y*c_rkPPosDir.y);
}

float CInstanceBase::NEW_GetRotation()
{
	float fCurRot=GetRotation();
	return NEW_UnsignedDegreeToSignedDegree(fCurRot);
}

float CInstanceBase::NEW_GetRotationFromDirPixelPosition(const TPixelPosition& c_rkPPosDir)
{
	return NEW_GetSignedDegreeFromDirPixelPosition(c_rkPPosDir);
}

float CInstanceBase::NEW_GetRotationFromDestPixelPosition(const TPixelPosition& c_rkPPosDst)
{
	TPixelPosition kPPosCur;
	NEW_GetPixelPosition(&kPPosCur);

	TPixelPosition kPPosDir;
	kPPosDir=c_rkPPosDst-kPPosCur;

	return NEW_GetRotationFromDirPixelPosition(kPPosDir);
}

float CInstanceBase::NEW_GetRotationFromDestInstance(CInstanceBase& rkInstDst)
{
	TPixelPosition kPPosDst;
	rkInstDst.NEW_GetPixelPosition(&kPPosDst);

	return NEW_GetRotationFromDestPixelPosition(kPPosDst);
}

void CInstanceBase::NEW_GetRandomPositionInFanRange(CInstanceBase& rkInstTarget, TPixelPosition* pkPPosDst)
{
	float fDstDirRot=NEW_GetRotationFromDestInstance(rkInstTarget);	

	float fRot=frandom(fDstDirRot-10.0f, fDstDirRot+10.0f);

	D3DXMATRIX kMatRot;
	D3DXMatrixRotationZ(&kMatRot, D3DXToRadian(-fRot));

	D3DXVECTOR3 v3Src(0.0f, 8000.0f, 0.0f);
	D3DXVECTOR3 v3Pos;
	D3DXVec3TransformCoord(&v3Pos, &v3Src, &kMatRot);

	const TPixelPosition& c_rkPPosCur=NEW_GetCurPixelPositionRef();
	//const TPixelPosition& c_rkPPosFront=rkInstTarget.NEW_GetCurPixelPositionRef();

	pkPPosDst->x=c_rkPPosCur.x+v3Pos.x;
	pkPPosDst->y=c_rkPPosCur.y+v3Pos.y;
	pkPPosDst->z=__GetBackgroundHeight(c_rkPPosCur.x, c_rkPPosCur.y);
}

bool CInstanceBase::NEW_GetFrontInstance(CInstanceBase ** ppoutTargetInstance, float fDistance)
{
	const float HALF_FAN_ROT_MIN = 10.0f;
	const float HALF_FAN_ROT_MAX = 50.0f;
	const float HALF_FAN_ROT_MIN_DISTANCE = 1000.0f;
	const float RPM = (HALF_FAN_ROT_MAX-HALF_FAN_ROT_MIN)/HALF_FAN_ROT_MIN_DISTANCE;

	float fDstRot=NEW_GetRotation();

	std::multimap<float, CInstanceBase*> kMap_pkInstNear;
	{
		CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
		CPythonCharacterManager::CharacterIterator i;
		for(i = rkChrMgr.CharacterInstanceBegin(); i!=rkChrMgr.CharacterInstanceEnd(); ++i)
		{
			CInstanceBase* pkInstEach=*i;
			if (pkInstEach==this)
				continue;

			if (!IsAttackableInstance(*pkInstEach))
				continue;

			if (NEW_GetDistanceFromDestInstance(*pkInstEach) > fDistance)
				continue;

			float fEachInstDistance=min(NEW_GetDistanceFromDestInstance(*pkInstEach), HALF_FAN_ROT_MIN_DISTANCE);
			float fEachInstDirRot=NEW_GetRotationFromDestInstance(*pkInstEach);

			float fHalfFanRot=(HALF_FAN_ROT_MAX-HALF_FAN_ROT_MIN)-RPM*fEachInstDistance+HALF_FAN_ROT_MIN;

			float fMinDstDirRot=fDstRot-fHalfFanRot;
			float fMaxDstDirRot=fDstRot+fHalfFanRot;

			if (fEachInstDirRot>=fMinDstDirRot && fEachInstDirRot<=fMaxDstDirRot)
				kMap_pkInstNear.insert(std::multimap<float, CInstanceBase*>::value_type(fEachInstDistance, pkInstEach));
		}
	}

	if (kMap_pkInstNear.empty())
		return false;

	*ppoutTargetInstance = kMap_pkInstNear.begin()->second;

	return true;
}

// 2004.07.21.levites - ºñÆÄºÎ ´ÙÁß Å¸°Ù Áö¿ø
bool CInstanceBase::NEW_GetInstanceVectorInFanRange(float fSkillDistance, CInstanceBase& rkInstTarget, std::vector<CInstanceBase*>* pkVct_pkInst)
{
	const float HALF_FAN_ROT_MIN = 20.0f;
	const float HALF_FAN_ROT_MAX = 40.0f;
	const float HALF_FAN_ROT_MIN_DISTANCE = 1000.0f;
	const float RPM = (HALF_FAN_ROT_MAX-HALF_FAN_ROT_MIN)/HALF_FAN_ROT_MIN_DISTANCE;

	float fDstDirRot=NEW_GetRotationFromDestInstance(rkInstTarget);

	// 2004.07.24.myevan - ºñÆÄºÎ °¡±îÀÌ ÀÖ´Â ÀûºÎÅÍ °ø°Ý
	std::multimap<float, CInstanceBase*> kMap_pkInstNear;
	{
		CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
		CPythonCharacterManager::CharacterIterator i;
		for(i = rkChrMgr.CharacterInstanceBegin(); i!=rkChrMgr.CharacterInstanceEnd(); ++i)
		{
			CInstanceBase* pkInstEach=*i;
			if (pkInstEach==this)
				continue;

			// 2004.07.25.myevan - ÀûÀÎ °æ¿ì¸¸ Ãß°¡ÇÑ´Ù
			if (!IsAttackableInstance(*pkInstEach))
				continue;

			// 2004.07.21.levites - ºñÆÄºÎ ´ÙÁß Å¸°Ù Áö¿ø
			if (m_GraphicThingInstance.IsClickableDistanceDestInstance(pkInstEach->m_GraphicThingInstance, fSkillDistance))
			{
				float fEachInstDistance=min(NEW_GetDistanceFromDestInstance(*pkInstEach), HALF_FAN_ROT_MIN_DISTANCE);
				float fEachInstDirRot=NEW_GetRotationFromDestInstance(*pkInstEach);

				float fHalfFanRot=(HALF_FAN_ROT_MAX-HALF_FAN_ROT_MIN)-RPM*fEachInstDistance+HALF_FAN_ROT_MIN;

				float fMinDstDirRot=fDstDirRot-fHalfFanRot;
				float fMaxDstDirRot=fDstDirRot+fHalfFanRot;

				if (fEachInstDirRot>=fMinDstDirRot && fEachInstDirRot<=fMaxDstDirRot)
					kMap_pkInstNear.insert(std::multimap<float, CInstanceBase*>::value_type(fEachInstDistance, pkInstEach));
			}
		}
	}

	{
		std::multimap<float, CInstanceBase*>::iterator i=kMap_pkInstNear.begin();
		for (i=kMap_pkInstNear.begin(); i!=kMap_pkInstNear.end(); ++i)
			pkVct_pkInst->push_back(i->second);
	}

	if (pkVct_pkInst->empty())
		return false;

	return true;
}

bool CInstanceBase::NEW_GetInstanceVectorInCircleRange(float fSkillDistance, std::vector<CInstanceBase*>* pkVct_pkInst)
{
	std::multimap<float, CInstanceBase*> kMap_pkInstNear;

	{
		CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
		CPythonCharacterManager::CharacterIterator i;
		for(i = rkChrMgr.CharacterInstanceBegin(); i!=rkChrMgr.CharacterInstanceEnd(); ++i)
		{
			CInstanceBase* pkInstEach=*i;

			// ÀÚ½ÅÀÎ °æ¿ì Ãß°¡ÇÏÁö ¾Ê´Â´Ù
			if (pkInstEach==this)
				continue;

			// ÀûÀÎ °æ¿ì¸¸ Ãß°¡ÇÑ´Ù
			if (!IsAttackableInstance(*pkInstEach))
				continue;

			if (m_GraphicThingInstance.IsClickableDistanceDestInstance(pkInstEach->m_GraphicThingInstance, fSkillDistance))
			{
				float fEachInstDistance=NEW_GetDistanceFromDestInstance(*pkInstEach);
				kMap_pkInstNear.insert(make_pair(fEachInstDistance, pkInstEach));
			}
		}
	}

	{
		std::multimap<float, CInstanceBase*>::iterator i=kMap_pkInstNear.begin();
		for (i=kMap_pkInstNear.begin(); i!=kMap_pkInstNear.end(); ++i)
			pkVct_pkInst->push_back(i->second);
	}

	if (pkVct_pkInst->empty())
		return false;

	return true;
}

BOOL CInstanceBase::NEW_IsClickableDistanceDestPixelPosition(const TPixelPosition& c_rkPPosDst)
{
	float fDistance=NEW_GetDistanceFromDestPixelPosition(c_rkPPosDst);

	if (fDistance>150.0f)
		return FALSE;

	return TRUE;
}

BOOL CInstanceBase::NEW_IsClickableDistanceDestInstance(CInstanceBase& rkInstDst)
{
	float fDistance = 150.0f;

	if (IsBowMode())
		fDistance = __GetBowRange();

	if (rkInstDst.IsNPC())
		fDistance = 500.0f;

	if (rkInstDst.IsResource())
		fDistance = 100.0f;

	return m_GraphicThingInstance.IsClickableDistanceDestInstance(rkInstDst.m_GraphicThingInstance, fDistance);
}

bool CInstanceBase::NEW_UseSkill(UINT uSkill, UINT uMot, UINT uMotLoopCount, bool isMovingSkill)
{
	if (IsDead())
		return false;

	if (IsStun())
		return false;

	if (IsKnockDown())
		return false;

	if (isMovingSkill)
	{
		if (!IsWalking())
			StartWalking();

		m_isGoing = TRUE;
	}
	else
	{
		if (IsWalking())
			EndWalking();

		m_isGoing = FALSE;
	}

	float fCurRot=m_GraphicThingInstance.GetTargetRotation();
	SetAdvancingRotation(fCurRot);

	m_GraphicThingInstance.InterceptOnceMotion(CRaceMotionData::NAME_SKILL + uMot, 0.1f, uSkill, 1.0f);

	m_GraphicThingInstance.__OnUseSkill(uMot, uMotLoopCount, isMovingSkill);

	if (uMotLoopCount > 0)
		m_GraphicThingInstance.SetMotionLoopCount(uMotLoopCount);

	return true;
}

void CInstanceBase::NEW_Attack()
{
	float fDirRot=GetRotation();
	NEW_Attack(fDirRot);
}

void CInstanceBase::NEW_Attack(float fDirRot)
{
	if (IsDead())
		return;

	if (IsStun())
		return;

	if (IsKnockDown())
		return;

	if (IsUsingSkill())
		return;
	
	if (IsWalking())
		EndWalking();

	m_isGoing = FALSE;

	if (IsPoly())
	{
		InputNormalAttack(fDirRot);
	}
	else
	{
		if (m_kHorse.IsMounting())
		{
			InputComboAttack(fDirRot);
		}
		else
		{
			InputComboAttack(fDirRot);
		}
	}
}


void CInstanceBase::NEW_AttackToDestPixelPositionDirection(const TPixelPosition& c_rkPPosDst)
{
	float fDirRot=NEW_GetRotationFromDestPixelPosition(c_rkPPosDst);
	
	NEW_Attack(fDirRot);
}

bool CInstanceBase::NEW_AttackToDestInstanceDirection(CInstanceBase& rkInstDst, IFlyEventHandler* pkFlyHandler)
{
	return NEW_AttackToDestInstanceDirection(rkInstDst);
}

bool CInstanceBase::NEW_AttackToDestInstanceDirection(CInstanceBase& rkInstDst)
{
	TPixelPosition kPPosDst;
	rkInstDst.NEW_GetPixelPosition(&kPPosDst);
	NEW_AttackToDestPixelPositionDirection(kPPosDst);

	return true;
}

void CInstanceBase::AttackProcess()
{
	if (!m_GraphicThingInstance.CanCheckAttacking())
		return;
   
	CInstanceBase * pkInstLast = NULL;
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	CPythonCharacterManager::CharacterIterator i = rkChrMgr.CharacterInstanceBegin();
	while (rkChrMgr.CharacterInstanceEnd()!=i)
	{
		CInstanceBase* pkInstEach=*i;
		++i;

		// ¼­·Î°£ÀÇ InstanceType ºñ±³
		if (!IsAttackableInstance(*pkInstEach))
			continue;

		if (pkInstEach!=this)
		{
			if (CheckAttacking(*pkInstEach))
			{
				pkInstLast=pkInstEach;
			}
		}
	}

	if (pkInstLast)
	{
		m_dwLastDmgActorVID=pkInstLast->GetVirtualID();
	}
}

void CInstanceBase::InputNormalAttack(float fAtkDirRot)
{
	m_GraphicThingInstance.InputNormalAttackCommand(fAtkDirRot);
}

void CInstanceBase::InputComboAttack(float fAtkDirRot)
{
	m_GraphicThingInstance.InputComboAttackCommand(fAtkDirRot);
	__ComboProcess();
}

void CInstanceBase::RunNormalAttack(float fAtkDirRot)
{
	EndGoing();
	m_GraphicThingInstance.NormalAttack(fAtkDirRot);
}

void CInstanceBase::RunComboAttack(float fAtkDirRot, DWORD wMotionIndex)
{
	EndGoing();
	m_GraphicThingInstance.ComboAttack(wMotionIndex, fAtkDirRot);
}

// ¸®ÅÏ°ª TRUE°¡ ¹«¾ùÀÎ°¡°¡ ÀÖ´Ù
BOOL CInstanceBase::CheckAdvancing()
{
#ifdef __MOVIE_MODE__
	if (IsMovieMode())
		return FALSE;
#endif
	if (!__IsMainInstance() && !IsAttacking())
	{
		if (IsPC() && IsWalking())
		{
			CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
			for(CPythonCharacterManager::CharacterIterator i = rkChrMgr.CharacterInstanceBegin(); i!=rkChrMgr.CharacterInstanceEnd();++i)
			{
				CInstanceBase* pkInstEach=*i;
				if (pkInstEach==this)
					continue;
				if (!pkInstEach->IsDoor())
					continue;

#ifdef ENABLE_NAGE_SYSTEM
				if (m_GraphicThingInstance.m_isNage)
				{
					if (m_GraphicThingInstance.hauteurEau + 5.0f <= pkInstEach->m_GraphicThingInstance.NEW_GetCurPixelPositionRef().z)
					{
						m_GraphicThingInstance.m_isNage = false;
						return FALSE;
					}
				}
#endif
#ifdef ENABLE_VOL_SYSTEM

				if (m_GraphicThingInstance.GetPosition().z <= pkInstEach->m_GraphicThingInstance.NEW_GetCurPixelPositionRef().z)
				{
					return false;
				}
#endif
				if (m_GraphicThingInstance.TestActorCollision(pkInstEach->GetGraphicThingInstanceRef()))
				{
					BlockMovement();
					return true;
				}
			}				
		}
		return FALSE;
	}

	if (m_GraphicThingInstance.CanSkipCollision())
	{
		//Tracenf("%x VID %d Ãæµ¹ ½ºÅµ", ELTimer_GetMSec(), GetVirtualID());
		return FALSE;
	}


	BOOL bUsingSkill = m_GraphicThingInstance.IsUsingSkill();

	m_dwAdvActorVID = 0;
	UINT uCollisionCount=0;

	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	for(CPythonCharacterManager::CharacterIterator i = rkChrMgr.CharacterInstanceBegin(); i!=rkChrMgr.CharacterInstanceEnd();++i)
	{
		CInstanceBase* pkInstEach=*i;
		if (pkInstEach==this)
			continue;

		CActorInstance& rkActorSelf=m_GraphicThingInstance;
		CActorInstance& rkActorEach=pkInstEach->GetGraphicThingInstanceRef();

#ifdef ENABLE_NAGE_SYSTEM
		if (rkActorSelf.m_isNage == true
#ifdef ENABLE_VOL_SYSTEM
			|| rkActorSelf.m_isVol == true
#endif
			)
		{
			if (rkActorSelf.m_isNage)
			{

				if (rkActorSelf.hauteurEau + 2.0f < rkActorEach.NEW_GetCurPixelPositionRef().z)
				{
					//rkActorSelf.m_isNage = false;
					return FALSE;
				}
			}
#ifdef ENABLE_VOL_SYSTEM
			if (rkActorSelf.m_isVol)
			{
				if (rkActorSelf.hauteurVolZ >rkActorEach.NEW_GetCurPixelPositionRef().z) // Si je vol au dessus
				{
					return FALSE;
				}
			}
			else
			{
				if (rkActorSelf.GetPosition().z < rkActorEach.NEW_GetCurPixelPositionRef().z)
				{
					return FALSE;
				}
			}
#endif

		}
#endif
		//NOTE : SkilÀ» ¾²´õ¶óµµ Door Type°ú´Â CollisionÃ¼Å© ÇÑ´Ù.
		if( bUsingSkill && !rkActorEach.IsDoor() )
			continue;
			
		// ¾ÕÀ¸·Î ÀüÁøÇÒ¼ö ÀÖ´Â°¡?
		if (rkActorSelf.TestActorCollision(rkActorEach))
		{
			
#ifdef ENABLE_VOL_SYSTEM
			if (rkActorSelf.m_isVol)
			{
				if (rkActorSelf.hauteurVolZ > rkActorEach.NEW_GetCurPixelPositionRef().z || rkActorSelf.hauteurVolZ < rkActorEach.NEW_GetCurPixelPositionRef().z) // Si je vol au dessus ou en dessous
				{
					return FALSE;
				}
			}
			else
			{

#endif
				if (rkActorSelf.GetPosition().z < rkActorEach.NEW_GetCurPixelPositionRef().z)
				{
					return FALSE;
				}
				uCollisionCount++;
				if (uCollisionCount == 2)
				{
					rkActorSelf.BlockMovement();
					return TRUE;
				}
				rkActorSelf.AdjustDynamicCollisionMovement(&rkActorEach);

				if (rkActorSelf.TestActorCollision(rkActorEach))
				{
					rkActorSelf.BlockMovement();
					return TRUE;
				}
				else
				{
					
					NEW_MoveToDestPixelPositionDirection(NEW_GetDstPixelPositionRef());
				}
#ifdef ENABLE_VOL_SYSTEM
			}
#endif
		}
	}

	// ¸Ê¼Ó¼º Ã¼Å©
	CPythonBackground& rkBG=CPythonBackground::Instance();
	const D3DXVECTOR3 & rv3Position = m_GraphicThingInstance.GetPosition();
	const D3DXVECTOR3 & rv3MoveDirection = m_GraphicThingInstance.GetMovementVectorRef();

	// NOTE : ¸¸¾à ÀÌµ¿ °Å¸®°¡ Å©´Ù¸é ÂÉ°³¼­ ±¸°£ º°·Î ¼Ó¼ºÀ» Ã¼Å©ÇØ º»´Ù
	//        ÇöÀç ¼³Á¤ÇØ ³õÀº 10.0f´Â ÀÓÀÇÀÇ °Å¸® - [levites]
	int iStep = int(D3DXVec3Length(&rv3MoveDirection) / 10.0f);
	D3DXVECTOR3 v3CheckStep = rv3MoveDirection / float(iStep);
	D3DXVECTOR3 v3CheckPosition = rv3Position;
	for (int j = 0; j < iStep; ++j)
	{
		v3CheckPosition += v3CheckStep;

		// Check
		if (rkBG.isAttrOn(v3CheckPosition.x, -v3CheckPosition.y, CTerrainImpl::ATTRIBUTE_BLOCK))
		{
#ifdef ENABLE_VOL_SYSTEM
			if (m_GraphicThingInstance.m_isVol == false)
			{

#endif
				BlockMovement();

#ifdef ENABLE_VOL_SYSTEM
			}
#endif
			//return TRUE;
		}
#ifdef ENABLE_VOL_SYSTEM
		else
		{
			float HauteurZsolCheck = CPythonBackground::Instance().GetTerrainHeight(v3CheckPosition.x, v3CheckPosition.y); // Récupère la hauteur du sol
			float HauteurZsolJoueur = CPythonBackground::Instance().GetTerrainHeight(rv3Position.x, rv3Position.y); // Récupère la hauteur du sol

			if (HauteurZsolCheck > HauteurZsolJoueur + 45.0f && m_GraphicThingInstance.m_isVol == false && m_GraphicThingInstance.m_isOnBuilding == false) // Si la hauteur du sol prédit est plus haute que celle du joueur on bloque.
			{
				BlockMovement();
				return TRUE;
			}
		}
#endif
	}

	// Check
	D3DXVECTOR3 v3NextPosition = rv3Position + rv3MoveDirection;
	if (rkBG.isAttrOn(v3NextPosition.x, -v3NextPosition.y, CTerrainImpl::ATTRIBUTE_BLOCK))
	{
#ifdef ENABLE_VOL_SYSTEM
		if (m_GraphicThingInstance.m_isVol == false)
		{

#endif
			BlockMovement();
			return TRUE;
#ifdef ENABLE_VOL_SYSTEM
		}
#endif
	}
#ifdef ENABLE_VOL_SYSTEM
	else
	{
		if (v3NextPosition.z > rv3Position.z + 100.0f && m_GraphicThingInstance.m_isVol == false && m_GraphicThingInstance.m_isOnBuilding == false) // Si la hauteur du sol prédit est plus haute que celle du joueur on bloque.
		{
			BlockMovement();
			return TRUE;
		}
	}
#endif



	return FALSE;
}

BOOL CInstanceBase::CheckAttacking(CInstanceBase& rkInstVictim)
{
	if (IsInSafe())
		return FALSE;

	if (rkInstVictim.IsInSafe())
		return FALSE;

#ifdef __MOVIE_MODE__
	return FALSE;
#endif

	if (!m_GraphicThingInstance.AttackingProcess(rkInstVictim.m_GraphicThingInstance))
		return FALSE;

	return TRUE;
}

BOOL CInstanceBase::isNormalAttacking()
{
	return m_GraphicThingInstance.isNormalAttacking();
}

BOOL CInstanceBase::isComboAttacking()
{
	return m_GraphicThingInstance.isComboAttacking();
}

BOOL CInstanceBase::IsUsingSkill()
{
	return m_GraphicThingInstance.IsUsingSkill();
}

BOOL CInstanceBase::IsUsingMovingSkill()
{
	return m_GraphicThingInstance.IsUsingMovingSkill();
}

BOOL CInstanceBase::CanCancelSkill()
{
	return m_GraphicThingInstance.CanCancelSkill();
}

BOOL CInstanceBase::CanAttackHorseLevel()
{
	if (!IsMountingHorse())
		return FALSE;

	return m_kHorse.CanAttack();
}

bool CInstanceBase::IsAffect(UINT uAffect)
{
	return m_kAffectFlagContainer.IsSet(uAffect);
}

MOTION_KEY CInstanceBase::GetNormalAttackIndex()
{
	return m_GraphicThingInstance.GetNormalAttackIndex();
}

DWORD CInstanceBase::GetComboIndex()
{
	return m_GraphicThingInstance.GetComboIndex();
}

float CInstanceBase::GetAttackingElapsedTime()
{
	return m_GraphicThingInstance.GetAttackingElapsedTime();
}

void CInstanceBase::ProcessHitting(DWORD dwMotionKey, CInstanceBase * pVictimInstance)
{
	assert(!"-_-" && "CInstanceBase::ProcessHitting");
	//m_GraphicThingInstance.ProcessSucceedingAttacking(dwMotionKey, pVictimInstance->m_GraphicThingInstance);
}

void CInstanceBase::ProcessHitting(DWORD dwMotionKey, BYTE byEventIndex, CInstanceBase * pVictimInstance)
{
	assert(!"-_-" && "CInstanceBase::ProcessHitting");
	//m_GraphicThingInstance.ProcessSucceedingAttacking(dwMotionKey, byEventIndex, pVictimInstance->m_GraphicThingInstance);
}

void CInstanceBase::GetBlendingPosition(TPixelPosition * pPixelPosition)
{
	m_GraphicThingInstance.GetBlendingPosition(pPixelPosition);
}

void CInstanceBase::SetBlendingPosition(const TPixelPosition & c_rPixelPosition)
{
	m_GraphicThingInstance.SetBlendingPosition(c_rPixelPosition);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void CInstanceBase::Revive()
{
	m_isGoing=FALSE;
	m_GraphicThingInstance.Revive();

	__AttachHorseSaddle();
}

void CInstanceBase::Stun()
{
	NEW_Stop();
	m_GraphicThingInstance.Stun();

	__AttachEffect(EFFECT_STUN);
}

void CInstanceBase::Die()
{
	__DetachHorseSaddle();

	if (IsAffect(AFFECT_SPAWN))
		__AttachEffect(EFFECT_SPAWN_DISAPPEAR);

	// 2004.07.25.ÀÌÆåÆ® ¾ÈºÙ´Â ¹®Á¦ÇØ°á
	////////////////////////////////////////
	__ClearAffects();
	////////////////////////////////////////

	OnUnselected();
	OnUntargeted();

	m_GraphicThingInstance.Die();
}

void CInstanceBase::Hide()
{
	m_GraphicThingInstance.SetAlphaValue(0.0f);
	m_GraphicThingInstance.BlendAlphaValue(0.0f, 0.1f);
}

void CInstanceBase::Show()
{
	m_GraphicThingInstance.SetAlphaValue(1.0f);
	m_GraphicThingInstance.BlendAlphaValue(1.0f, 0.1f);
}