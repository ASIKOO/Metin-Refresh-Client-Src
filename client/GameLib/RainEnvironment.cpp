#include "StdAfx.h"
#include "RainEnvironment.h"

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
#include "Area.h"
#include "AreaTerrain.h"
#include "MapBase.h"
#include "TerrainDecal.h"
#include "../UserInterface/PythonBackground.h"

void CRainEnvironment::Enable()
{
	if (!m_bRainEnable)
	{
		Create();
	}
	m_bRainEnable = TRUE;
}

void CRainEnvironment::Disable()
{
	m_bRainEnable = FALSE;
}

void CRainEnvironment::Update(const D3DXVECTOR3 & c_rv3Pos)
{
	if (!m_bRainEnable)
	{
		if (m_kVct_pkParticleRain.empty())
			return;
	}

	m_v3Center = c_rv3Pos;
}



void CRainEnvironment::Deform()
{
	if (!m_bRainEnable)
	{
		if (m_kVct_pkParticleRain.empty())
			return;
	}

	const D3DXVECTOR3 & c_rv3Pos = m_v3Center;

	static long s_lLastTime = CTimer::Instance().GetCurrentMillisecond();
	long lcurTime = CTimer::Instance().GetCurrentMillisecond();
	float fElapsedTime = float(lcurTime - s_lLastTime) / 15.0f;
	s_lLastTime = lcurTime;

	CCamera * pCamera = CCameraManager::Instance().GetCurrentCamera();
	if (!pCamera)
		return;

	const D3DXVECTOR3 & c_rv3View = pCamera->GetView();

	D3DXVECTOR3 v3ChangedPos = c_rv3View * 3500.0f + c_rv3Pos;
	v3ChangedPos.z = c_rv3Pos.z;

	std::vector<CRainParticle*>::iterator itor = m_kVct_pkParticleRain.begin();
	for (; itor != m_kVct_pkParticleRain.end();)
	{
		CRainParticle * pRain = *itor;

		int rand_num = rand() % 100 + 1;
		if (rand_num >= 75)
		{
			m_SendThunder = true;
		}
		else
		{
			m_SendThunder = true;
		}
		
		pRain->Update(fElapsedTime, v3ChangedPos, m_SendThunder);
		if (m_SendThunder)
		{
			m_SendThunder = false;
		}

		if (!pRain->IsActivate())
		{
			CRainParticle::Delete(pRain);

			itor = m_kVct_pkParticleRain.erase(itor);
		}
		else
		{
			float fGroundHeight = CFlyingManager::Instance().GetMapManagerPtr()->GetTerrainHeight(pRain->m_v3Position.x, pRain->m_v3Position.y);
			long hauteurEau;

			//if (pRain->m_impactPluie == false)
			//{
			bool onWater = false;
			int incrementX = 0;
			for (int i = 0; i < 1000; i++)
			{
				incrementX += 10;
				if (CPythonBackground::Instance().isAttrOn(pRain->m_v3Position.x+incrementX, pRain->m_v3Position.y, CTerrainImpl::ATTRIBUTE_WATER))
				{
					onWater = true;
					CPythonBackground::Instance().GetWaterHeight(int(pRain->m_v3Position.x + incrementX), int(pRain->m_v3Position.y), &hauteurEau);

				}
			}
			if (onWater == false)
			{
				int incrementY = 0;
				for (int i = 0; i < 1000; i++)
				{
					incrementY += 10;
					if (CPythonBackground::Instance().isAttrOn(pRain->m_v3Position.x, pRain->m_v3Position.y+incrementY, CTerrainImpl::ATTRIBUTE_WATER))
					{
						onWater = true;
						CPythonBackground::Instance().GetWaterHeight(int(pRain->m_v3Position.x), int(pRain->m_v3Position.y + incrementY), &hauteurEau);

					}
				}
				if (onWater == false)
				{
					incrementX = 0;
					incrementY = 0;
					for (int i = 0; i < 1000; i++)
					{
						incrementY += 10;
						incrementX += 10;
						if (CPythonBackground::Instance().isAttrOn(pRain->m_v3Position.x+incrementX, pRain->m_v3Position.y + incrementY, CTerrainImpl::ATTRIBUTE_WATER))
						{
							onWater = true;

							CPythonBackground::Instance().GetWaterHeight(int(pRain->m_v3Position.x+incrementX), int(pRain->m_v3Position.y+incrementY), &hauteurEau);
						}
					}
				}
			}
				if (onWater)
				{


					if (hauteurEau > fGroundHeight)
					{
						CEffectManager& rkEffMgr = CEffectManager::Instance();
						rkEffMgr.RegisterEffect("d:/ymir work/effect/etc/fishing/ridack_pluie.mse");
						DWORD dwEffectID = rkEffMgr.CreateEffect("d:/ymir work/effect/etc/fishing/ridack_pluie.mse", D3DXVECTOR3(pRain->m_v3Position.x, pRain->m_v3Position.y, (float)hauteurEau), D3DXVECTOR3(0.0f, 0.0f, 0.0f));
						pRain->m_impactPluie = true;
					}
					else
					{
						CEffectManager& rkEffMgr = CEffectManager::Instance();
						rkEffMgr.RegisterEffect("d:/ymir work/effect/etc/fishing/ridack_pluie2.mse");
						DWORD dwEffectID = rkEffMgr.CreateEffect("d:/ymir work/effect/etc/fishing/ridack_pluie2.mse", D3DXVECTOR3(pRain->m_v3Position.x, pRain->m_v3Position.y, fGroundHeight), D3DXVECTOR3(0.0f, 0.0f, 0.0f));
						pRain->m_impactPluie = true;
					}
				}
				else
				{
					CEffectManager& rkEffMgr = CEffectManager::Instance();
					rkEffMgr.RegisterEffect("d:/ymir work/effect/etc/fishing/ridack_pluie2.mse");
					DWORD dwEffectID = rkEffMgr.CreateEffect("d:/ymir work/effect/etc/fishing/ridack_pluie2.mse", D3DXVECTOR3(pRain->m_v3Position.x, pRain->m_v3Position.y, fGroundHeight), D3DXVECTOR3(0.0f, 0.0f, 0.0f));
					pRain->m_impactPluie = true;
				}
			//}
			++itor;
		}
	}


	if (m_bRainEnable)
	{
		for (int p = 0; p < min(10, m_dwParticleMaxNum - m_kVct_pkParticleRain.size()); ++p)
		{
			CRainParticle * pRainParticle = CRainParticle::New();
			pRainParticle->Init(v3ChangedPos);
			m_kVct_pkParticleRain.push_back(pRainParticle);

		}

	}
}

D3DXVECTOR3 CRainEnvironment::randomPositionSonPluie(D3DXVECTOR3 position)
{
	srand(time(NULL));
	float rand_num_x = rand() % 100;
	srand(time(NULL));
	float rand_num_y = rand() % 100;
	srand(time(NULL));
	float rand_num_z = rand() % 100;
	srand(time(NULL));
	int randomAddition = rand() % 2;

	if (randomAddition <= 1)
	{
		position.x = position.x + rand_num_x;
		position.y = position.y + rand_num_y;
		position.z = position.z + rand_num_z;
	}
	else
	{
		position.x = position.x - rand_num_x;
		position.y = position.y - rand_num_y;
		position.z = position.z - rand_num_z;
	}
	return position;
}

void CRainEnvironment::__BeginBlur()
{
	if (!m_bBlurEnable)
		return;

	ms_lpd3dDevice->GetDepthStencilSurface(&m_lpOldDepthStencilSurface);
	ms_lpd3dDevice->SetRenderTarget(0, m_lpRainRenderTargetSurface);
	ms_lpd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0L);

	STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTALPHA);
}

void CRainEnvironment::__ApplyBlur()
{
	if (!m_bBlurEnable)
		return;

	//			{
	//				STATEMANAGER.SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
	//				STATEMANAGER.SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
	//				STATEMANAGER.SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	//				STATEMANAGER.SetRenderState( D3DRS_COLORVERTEX ,TRUE);
	//				STATEMANAGER.SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE , D3DMCS_COLOR1 );
	//				STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	//				STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	//				STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	//				STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	//				STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	//				STATEMANAGER.SetTextureStageState(0,  D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	//				DWORD	alphaColor = 0xFFFFFF | ((DWORD)(0.6f*255.0f) << 24);
	//
	//				BlurVertex V[4] = { BlurVertex(D3DXVECTOR3(0.0f,0.0f,0.0f),1.0f,		alphaColor, 0,0) ,
	//									BlurVertex(D3DXVECTOR3(wTextureSize,0.0f,0.0f),1.0f,		alphaColor, 1,0) , 
	//									BlurVertex(D3DXVECTOR3(0.0f,wTextureSize,0.0f),1.0f,		alphaColor, 0,1) , 
	//									BlurVertex(D3DXVECTOR3(wTextureSize,wTextureSize,0.0f),1.0f,	alphaColor, 1,1) };
	//				//누적 블러 텍스쳐를 찍는다.
	//				STATEMANAGER.SetTexture(0,m_lpAccumTexture);
	//				STATEMANAGER.SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE|D3DFVF_TEX1 );
	//				STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,V,sizeof(BlurVertex));
	//			}
	//
	//			{
	//				STATEMANAGER.SetRenderTarget(m_lpAccumRenderTargetSurface, m_lpAccumDepthSurface);
	//
	//				BlurVertex V[4] = { BlurVertex(D3DXVECTOR3(0.0f,0.0f,0.0f),1.0f,		0xFFFFFF, 0,0) ,
	//									BlurVertex(D3DXVECTOR3(wTextureSize,0.0f,0.0f),1.0f,		0xFFFFFF, 1,0) , 
	//									BlurVertex(D3DXVECTOR3(0.0f,wTextureSize,0.0f),1.0f,		0xFFFFFF, 0,1) , 
	//									BlurVertex(D3DXVECTOR3(wTextureSize,wTextureSize,0.0f),1.0f,	0xFFFFFF, 1,1) };
	//
	//				STATEMANAGER.SetTexture(0,m_lpRainTexture);
	//				STATEMANAGER.SetRenderState( D3DRS_ALPHABLENDENABLE,   FALSE);
	//				STATEMANAGER.SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE|D3DFVF_TEX1 );
	//				STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,V,sizeof(BlurVertex));
	//			}

	///////////////
	{
		ms_lpd3dDevice->SetRenderTarget(0, m_lpOldSurface);

		STATEMANAGER.SetTexture(0, m_lpRainTexture);
		STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		STATEMANAGER.SetRenderState(D3DRS_WRAP0, D3DWRAPCOORD_0);

		D3DSURFACE_DESC	desc;
		m_lpOldSurface->GetDesc(&desc);
		float sx = (float)desc.Width;
		float sy = (float)desc.Height;
		SAFE_RELEASE(m_lpOldSurface);
		SAFE_RELEASE(m_lpOldDepthStencilSurface);

		BlurVertex V[4] = { BlurVertex(D3DXVECTOR3(0.0f, 0.0f, 0.0f), 1.0f, 0xFFFFFF, 0, 0),
			BlurVertex(D3DXVECTOR3(sx, 0.0f, 0.0f), 1.0f, 0xFFFFFF, 1, 0),
			BlurVertex(D3DXVECTOR3(0.0f, sy, 0.0f), 1.0f, 0xFFFFFF, 0, 1),
			BlurVertex(D3DXVECTOR3(sx, sy, 0.0f), 1.0f, 0xFFFFFF, 1, 1) };

		STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, V, sizeof(BlurVertex));
	}
}

void CRainEnvironment::Render()
{
	if (!m_bRainEnable)
	{
		if (m_kVct_pkParticleRain.empty())
			return;
	}

	__BeginBlur();

	DWORD dwParticleCount = min(m_dwParticleMaxNum, m_kVct_pkParticleRain.size());

	CCamera * pCamera = CCameraManager::Instance().GetCurrentCamera();
	if (!pCamera)
		return;

	const D3DXVECTOR3 & c_rv3Up = pCamera->GetUp();
	const D3DXVECTOR3 & c_rv3Cross = pCamera->GetCross();

	SParticleVertex * pv3Verticies;
	if (SUCCEEDED(m_pVB->Lock(0, sizeof(SParticleVertex)*dwParticleCount * 4, (BYTE**)&pv3Verticies, D3DLOCK_DISCARD)))
	{
		int i = 0;
		std::vector<CRainParticle*>::iterator itor = m_kVct_pkParticleRain.begin();
		for (; i < dwParticleCount && itor != m_kVct_pkParticleRain.end(); ++i, ++itor)
		{
			CRainParticle * pRain = *itor;
			pRain->SetCameraVertex(c_rv3Up, c_rv3Cross);
			pRain->GetVerticies(pv3Verticies[i * 4 + 0],
				pv3Verticies[i * 4 + 1],
				pv3Verticies[i * 4 + 2],
				pv3Verticies[i * 4 + 3]);
		}
		m_pVB->Unlock();
	}

	STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTexture(1, NULL);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	m_pImageInstance->GetGraphicImagePointer()->GetTextureReference().SetTextureStage(0);
	STATEMANAGER.SetIndices(m_pIB, 0);
	STATEMANAGER.SetStreamSource(0, m_pVB, sizeof(SParticleVertex));
	STATEMANAGER.SetVertexShader(D3DFVF_XYZ | D3DFVF_TEX1);
	STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, dwParticleCount * 4, 0, dwParticleCount * 2);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
	__ApplyBlur();
}

bool CRainEnvironment::__CreateBlurTexture()
{
	if (!m_bBlurEnable)
		return true;

	if (!m_bBlurEnable)
		return true;

	if (FAILED(ms_lpd3dDevice->CreateTexture(m_wBlurTextureSize, m_wBlurTextureSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_lpRainTexture)))
		return false;
	if (FAILED(m_lpRainTexture->GetSurfaceLevel(0, &m_lpRainRenderTargetSurface)))
		return false;
	//if (FAILED(ms_lpd3dDevice->CreateDepthStencilSurface(m_wBlurTextureSize, m_wBlurTextureSize, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, TRUE, &m_lpRainDepthSurface)))
		//return false;

	if (FAILED(ms_lpd3dDevice->CreateTexture(m_wBlurTextureSize, m_wBlurTextureSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_lpAccumTexture)))
		return false;
	if (FAILED(m_lpAccumTexture->GetSurfaceLevel(0, &m_lpAccumRenderTargetSurface)))
		return false;
	/*if (FAILED(ms_lpd3dDevice->CreateDepthStencilSurface(m_wBlurTextureSize, m_wBlurTextureSize, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, TRUE, &m_lpAccumDepthSurface)))
		return false;*/

	return true;

	return true;
}

bool CRainEnvironment::__CreateGeometry()
{
	if (FAILED(ms_lpd3dDevice->CreateVertexBuffer(sizeof(SParticleVertex)*m_dwParticleMaxNum * 4,
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		D3DFVF_XYZ | D3DFVF_TEX1,
		D3DPOOL_SYSTEMMEM,
		&m_pVB)))
		return false;

	if (FAILED(ms_lpd3dDevice->CreateIndexBuffer(sizeof(WORD)*m_dwParticleMaxNum * 6,
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&m_pIB)))
		return false;

	WORD* dstIndices;
	if (FAILED(m_pIB->Lock(0, sizeof(WORD)*m_dwParticleMaxNum * 6, (BYTE**)&dstIndices, 0)))
		return false;

	const WORD c_awFillRectIndices[6] = { 0, 2, 1, 2, 3, 1, };
	for (int i = 0; i < m_dwParticleMaxNum; ++i)
	{
		for (int j = 0; j < 6; ++j)
		{
			dstIndices[i * 6 + j] = i * 4 + c_awFillRectIndices[j];
		}
	}

	m_pIB->Unlock();
	return true;
}

bool CRainEnvironment::Create()
{
	Destroy();

	if (!__CreateBlurTexture())
		return false;

	if (!__CreateGeometry())
		return false;

	CGraphicImage * pImage = (CGraphicImage *)CResourceManager::Instance().GetResourcePointer("d:/ymir work/special/rain.dds");
	m_pImageInstance = CGraphicImageInstance::New();
	m_pImageInstance->SetImagePointer(pImage);

	return true;
}

void CRainEnvironment::Destroy()
{
	SAFE_RELEASE(m_lpRainTexture);
	SAFE_RELEASE(m_lpRainRenderTargetSurface);
	SAFE_RELEASE(m_lpRainDepthSurface);
	SAFE_RELEASE(m_lpAccumTexture);
	SAFE_RELEASE(m_lpAccumRenderTargetSurface);
	SAFE_RELEASE(m_lpAccumDepthSurface);
	SAFE_RELEASE(m_pVB);
	SAFE_RELEASE(m_pIB);

	stl_wipe(m_kVct_pkParticleRain);
	CRainParticle::DestroyPool();

	if (m_pImageInstance)
	{
		CGraphicImageInstance::Delete(m_pImageInstance);
		m_pImageInstance = NULL;
	}
	__Initialize();
}

void CRainEnvironment::__Initialize()
{
	m_bRainEnable = FALSE;
	m_lpRainTexture = NULL;
	m_lpRainRenderTargetSurface = NULL;
	m_lpRainDepthSurface = NULL;
	m_lpAccumTexture = NULL;
	m_lpAccumRenderTargetSurface = NULL;
	m_lpAccumDepthSurface = NULL;
	m_pVB = NULL;
	m_pIB = NULL;
	m_pImageInstance = NULL;
	m_SendThunder = false;

	m_kVct_pkParticleRain.reserve(m_dwParticleMaxNum);
}

CRainEnvironment::CRainEnvironment()
{
	m_bBlurEnable = FALSE;
	m_dwParticleMaxNum = 500000;
	m_wBlurTextureSize = 512;

	__Initialize();
}
CRainEnvironment::~CRainEnvironment()
{
	Destroy();
}
