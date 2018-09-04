#include "StdAfx.h"
#include "RainParticle.h"
#include "..\EterLib\StateManager.h"
#include "MapType.h"
#include "../EterLib/StateManager.h"
#include "../EterLib/Camera.h"
#include "../EterLib/ResourceManager.h"
#include "../EffectLib/EffectManager.h"
#include "RainParticle.h"
#include "MapOutdoor.h"
#include "../gamelib/FlyingObjectManager.h"
#include "MapManager.h"
#include "MapOutdoor.h"
#include "TerrainPatch.h"
#include "../SpeedTreeLib/SpeedTreeForest.h"
#include "../EterLib/GrpBase.h"
#include <stdlib.h>     /* srand, rand */


const float c_fRainDistance = 70000.0f;

std::vector<CRainParticle*> CRainParticle::ms_kVct_RainParticlePool;

void CRainParticle::SetCameraVertex(const D3DXVECTOR3 & rv3Up, const D3DXVECTOR3 & rv3Cross)
{
	m_v3Up = rv3Up*m_fHalfWidth;
	m_v3Cross = rv3Cross*m_fHalfHeight;
}

bool CRainParticle::IsActivate()
{

	return m_bActivate;
}

void CRainParticle::UpdateFoudrePhase3()
{
	/*srand(time(NULL));
	int rand_num = rand() % 100;

	if (rand_num > 80)
	{

		STATEMANAGER.RestoreRenderState(D3DRS_DIFFUSEMATERIALSOURCE);
		STATEMANAGER.RestoreRenderState(D3DRS_SPECULARMATERIALSOURCE);
		STATEMANAGER.RestoreRenderState(D3DRS_AMBIENTMATERIALSOURCE);
		STATEMANAGER.RestoreRenderState(D3DRS_EMISSIVEMATERIALSOURCE);

	}*/

}

const char* CRainParticle::randomSonEclaire()
{
	srand(time(NULL));
	int rand_num = rand() % 7 + 1;
	switch (rand_num)
	{
	case 1:
		return "d:/ymir work/sound/meteo/eclaire1.mp3";
		break;
	case 2:
		return "d:/ymir work/sound/meteo/eclaire2.mp3";
		break;
	case 3:
		return "d:/ymir work/sound/meteo/eclaire3.mp3";
		break;
	case 4:
		return "d:/ymir work/sound/meteo/eclaire4.mp3";
		break;
	case 5:
		return "d:/ymir work/sound/meteo/eclaire5.mp3";
		break;
	case 6:
		return "d:/ymir work/sound/meteo/eclaire6.mp3";
		break;
	case 7:
		return "d:/ymir work/sound/meteo/eclaire7.mp3";
		break;
	default:
		return "d:/ymir work/sound/meteo/eclaire1.mp3";
		break;
	}
	return "d:/ymir work/sound/meteo/eclaire1.mp3";
}

float CRainParticle::randomPositionZEclaire(float z)
{
	srand(time(NULL));
	float rand_num = rand() % 100 + (-100);
	return z + rand_num;
}

float CRainParticle::randomPositionXEclaire(float x)
{
	srand(time(NULL));
	float rand_num = rand() % 100 + (-100);
	return x + rand_num;
}

float CRainParticle::randomPositionYEclaire(float y)
{
	srand(time(NULL));
	float rand_num = rand() % 100 + (-100);
	return y + rand_num;
}

void CRainParticle::ExecutionSonTonnerre(D3DXVECTOR3 m_v3Position, bool ExecuteEclaire)
{

	CSoundManager& rkSndMgr = CSoundManager::Instance();
	float x = randomPositionXEclaire(m_v3Position.x);
	float y = randomPositionYEclaire(m_v3Position.y);
	float z = randomPositionZEclaire(m_v3Position.z);
	srand(time(NULL));
	int rand_num = rand() % 100 + 1;
	if (ExecuteEclaire == true && tempsTotal >= 300.0f)
	{
		tempsTotal = 0;
		float fGroundHeight = CFlyingManager::Instance().GetMapManagerPtr()->GetTerrainHeight(x, y);
		CEffectManager& rkEffMgr = CEffectManager::Instance();
		rkEffMgr.RegisterEffect("d:/ymir work/meteo/effet/foudre_orage.mse");
		DWORD dwEffectID = rkEffMgr.CreateEffect("d:/ymir work/meteo/effet/foudre_orage.mse", D3DXVECTOR3(x, y, fGroundHeight + 5.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f));
		srand(time(NULL));

	}

	
	rkSndMgr.PlaySound3D(x, y, z, randomSonEclaire());

}

void CRainParticle::UpdateFoudrePhase2(D3DXVECTOR3 m_v3Position, bool executeEclaire)
{
	srand(time(NULL));
	int rand_num = rand() % 100;
	srand(time(NULL));
	float intensiter = rand() % 50000 + 20000;
	srand(time(NULL));
	float specular = rand() % 300 + 100;
	if (rand_num > 70 && rand_num < 80)
	{
		
		D3DMATERIAL8 DefaultMat;
		ZeroMemory(&DefaultMat, sizeof(D3DMATERIAL8));

		DefaultMat.Diffuse.r = 1.0f;
		DefaultMat.Diffuse.g = 1.0f;
		DefaultMat.Diffuse.b = 1.0f;
		DefaultMat.Diffuse.a = 1.0f;
		DefaultMat.Ambient.r = 10.0f;
		DefaultMat.Ambient.g = 10.0f;
		DefaultMat.Ambient.b = 10.0f;
		DefaultMat.Ambient.a = 10.0f;
		DefaultMat.Emissive.r = 2.0f;
		DefaultMat.Emissive.g = 2.0f;
		DefaultMat.Emissive.b = 2.0f;
		DefaultMat.Emissive.a = 2.0f;
		srand(time(NULL));
		float light = rand() % 2;
		if (light <= 1)
		{

			DefaultMat.Specular.r = 10.0f * specular;
			DefaultMat.Specular.g = 10.0f * specular;
			DefaultMat.Specular.b = 10.0f * specular;
			DefaultMat.Specular.a = 10.0f * specular;
		}
		/*if (light <= 1)
		{

			DefaultMat.Specular.r = 10.0f * specular;
			DefaultMat.Specular.g = 10.0f * specular;
			DefaultMat.Specular.b = 10.0f * specular;
			DefaultMat.Specular.a = 10.0f * specular;
		}
		else
		{
			DefaultMat.Specular.r = 10.0f * -specular;
			DefaultMat.Specular.g = 10.0f * -specular;
			DefaultMat.Specular.b = 10.0f * -specular;
			DefaultMat.Specular.a = 10.0f * -specular;
		}*/
		DefaultMat.Power = intensiter;

		STATEMANAGER.SetMaterial(&DefaultMat);
		STATEMANAGER.SetRenderState(D3DRS_SPECULARENABLE, TRUE);
		STATEMANAGER.SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
		STATEMANAGER.SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
		STATEMANAGER.SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
		STATEMANAGER.SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
		ExecutionSonTonnerre(m_v3Position, executeEclaire);

	}
	else
	{
		STATEMANAGER.RestoreRenderState(D3DRS_DIFFUSEMATERIALSOURCE);
		STATEMANAGER.RestoreRenderState(D3DRS_SPECULARMATERIALSOURCE);
		STATEMANAGER.RestoreRenderState(D3DRS_AMBIENTMATERIALSOURCE);
		STATEMANAGER.RestoreRenderState(D3DRS_EMISSIVEMATERIALSOURCE);

	}
	
}

void CRainParticle::UpdateFoudrePhase1()
{
	srand(time(NULL));
	int rand_num = rand() % 100;
	srand(time(NULL));
	float intensiter = rand() % 50000 + 20000;
	srand(time(NULL));
	float specular = rand() % 300 + 100;
	srand(time(NULL));
	float ambiant = rand() % 100;
	if (rand_num > 40 && rand_num < 55)
	{

		D3DMATERIAL8 DefaultMat;
		ZeroMemory(&DefaultMat, sizeof(D3DMATERIAL8));

		DefaultMat.Diffuse.r = 1.0f;
		DefaultMat.Diffuse.g = 1.0f;
		DefaultMat.Diffuse.b = 1.0f;
		DefaultMat.Diffuse.a = 1.0f;
		DefaultMat.Ambient.r = 10.0f * ambiant;
		DefaultMat.Ambient.g = 10.0f * ambiant;
		DefaultMat.Ambient.b = 10.0f * ambiant;
		DefaultMat.Ambient.a = 10.0f;
		DefaultMat.Emissive.r = 2.0f;
		DefaultMat.Emissive.g = 2.0f;
		DefaultMat.Emissive.b = 2.0f;
		DefaultMat.Emissive.a = 2.0f;
		srand(time(NULL));
		float light = rand() % 2;
		if (light <= 1)
		{

			DefaultMat.Specular.r = 10.0f * specular;
			DefaultMat.Specular.g = 10.0f * specular;
			DefaultMat.Specular.b = 10.0f * specular;
			DefaultMat.Specular.a = 10.0f * specular;
		}
		/*if (light <= 1)
		{

		DefaultMat.Specular.r = 10.0f * specular;
		DefaultMat.Specular.g = 10.0f * specular;
		DefaultMat.Specular.b = 10.0f * specular;
		DefaultMat.Specular.a = 10.0f * specular;
		}
		else
		{
		DefaultMat.Specular.r = 10.0f * -specular;
		DefaultMat.Specular.g = 10.0f * -specular;
		DefaultMat.Specular.b = 10.0f * -specular;
		DefaultMat.Specular.a = 10.0f * -specular;
		}*/
		DefaultMat.Power = intensiter;

		STATEMANAGER.SetMaterial(&DefaultMat);
		STATEMANAGER.SetRenderState(D3DRS_SPECULARENABLE, TRUE);
		STATEMANAGER.SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
		STATEMANAGER.SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
		STATEMANAGER.SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
		STATEMANAGER.SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
		//ExecutionSonTonnerre(m_v3Position);
	}
	else
	{
		STATEMANAGER.RestoreRenderState(D3DRS_DIFFUSEMATERIALSOURCE);
		STATEMANAGER.RestoreRenderState(D3DRS_SPECULARMATERIALSOURCE);
		STATEMANAGER.RestoreRenderState(D3DRS_AMBIENTMATERIALSOURCE);
		STATEMANAGER.RestoreRenderState(D3DRS_EMISSIVEMATERIALSOURCE);

	}

}

void CRainParticle::Update(float fElapsedTime, const D3DXVECTOR3 & c_rv3Pos, bool executeEclaire)
{


	for (int i = 0; i < 10; i++)
	{
		for (int k = 0; k < 10; k++)
		{
			UpdateFoudrePhase3();

			UpdateFoudrePhase2(c_rv3Pos, executeEclaire);
		}

		UpdateFoudrePhase3();
	}

	m_v3Position += m_v3Velocity * fElapsedTime;

	m_v3Position.x += m_v3Cross.x * sin(m_fcurRadian) / 10.0f;
	m_v3Position.y += m_v3Cross.y * sin(m_fcurRadian) / 10.0f;
	m_fcurRadian += m_fPeriod * fElapsedTime;



	if (m_v3Position.z < c_rv3Pos.z - 500.0f)
		m_bActivate = false;
	else if (abs(m_v3Position.x - c_rv3Pos.x) > c_fRainDistance)
		m_bActivate = false;
	else if (abs(m_v3Position.y - c_rv3Pos.y) > c_fRainDistance)
		m_bActivate = false;
}

void CRainParticle::GetVerticies(SParticleVertex & rv3Vertex1, SParticleVertex & rv3Vertex2,
	SParticleVertex & rv3Vertex3, SParticleVertex & rv3Vertex4)
{
	rv3Vertex1.v3Pos = m_v3Position - m_v3Cross - m_v3Up;
	rv3Vertex1.u = 0.0f;
	rv3Vertex1.v = 0.0f;

	rv3Vertex2.v3Pos = m_v3Position + m_v3Cross - m_v3Up;
	rv3Vertex2.u = 1.0f;
	rv3Vertex2.v = 0.0f;

	rv3Vertex3.v3Pos = m_v3Position - m_v3Cross + m_v3Up;
	rv3Vertex3.u = 0.0f;
	rv3Vertex3.v = 1.0f;

	rv3Vertex4.v3Pos = m_v3Position + m_v3Cross + m_v3Up;
	rv3Vertex4.u = 1.0f;
	rv3Vertex4.v = 1.0f;
}

void CRainParticle::Init(const D3DXVECTOR3 & c_rv3Pos)
{
	float fRot = frandom(0.0f, 36000.0f) / 100.0f;
	float fDistance = frandom(0.0f, c_fRainDistance) / 10.0f;

	m_v3Position.x = c_rv3Pos.x + fDistance*sin((double)D3DXToRadian(fRot));
	m_v3Position.y = c_rv3Pos.y + fDistance*cos((double)D3DXToRadian(fRot));
	m_v3Position.z = c_rv3Pos.z + frandom(1500.0f, 2000.0f);
	m_v3Velocity.x = 0.0f;
	m_v3Velocity.y = 0.0f;
	m_v3Velocity.z = frandom(-50.0f, -200.0f);
	m_fHalfWidth = frandom(50.0f, 80.0f);
	m_fHalfHeight = m_fHalfWidth / 12;
	m_bActivate = true;
	m_bChangedSize = false;

	m_fPeriod = frandom(1.5f, 10.0f);
	m_fcurRadian = frandom(-1.6f, 1.6f);
	m_fAmplitude = frandom(1.0f, 3.0f);

}

CRainParticle * CRainParticle::New()
{
	if (ms_kVct_RainParticlePool.empty())
	{
		return new CRainParticle;
	}

	CRainParticle * pParticle = ms_kVct_RainParticlePool.back();
	ms_kVct_RainParticlePool.pop_back();
	return pParticle;
}

void CRainParticle::Delete(CRainParticle * pSnowParticle)
{
	ms_kVct_RainParticlePool.push_back(pSnowParticle);
}

void CRainParticle::DestroyPool()
{
	stl_wipe(ms_kVct_RainParticlePool);
}

CRainParticle::CRainParticle()
{
}
CRainParticle::~CRainParticle()
{
}
