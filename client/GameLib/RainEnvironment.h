#pragma once

#include "../EterLib/GrpScreen.h"
#include "MapOutdoor.h"

class CRainParticle;

class CRainEnvironment : public CScreen
{
public:
	CRainEnvironment();
	virtual ~CRainEnvironment();

	bool Create();
	void Destroy();

	void Enable();
	void Disable();

	bool m_SendThunder;
	void Update(const D3DXVECTOR3 & c_rv3Pos);
	void Deform();
	int compteurEclaire;
	float lastTime;
	float compteurTemps;
	bool executeEclaire;
	void Render();
	D3DXVECTOR3 randomPositionSonPluie(D3DXVECTOR3 position);
	float feuilleVitesse;


protected:
	void __Initialize();
	bool __CreateBlurTexture();
	bool __CreateGeometry();
	void __BeginBlur();
	void __ApplyBlur();

protected:
	LPDIRECT3DSURFACE8 m_lpOldSurface;
	LPDIRECT3DSURFACE8 m_lpOldDepthStencilSurface;

	LPDIRECT3DTEXTURE8 m_lpRainTexture;
	LPDIRECT3DSURFACE8 m_lpRainRenderTargetSurface;
	LPDIRECT3DSURFACE8 m_lpRainDepthSurface;

	LPDIRECT3DTEXTURE8 m_lpAccumTexture;
	LPDIRECT3DSURFACE8 m_lpAccumRenderTargetSurface;
	LPDIRECT3DSURFACE8 m_lpAccumDepthSurface;

	LPDIRECT3DVERTEXBUFFER8 m_pVB;
	LPDIRECT3DINDEXBUFFER8 m_pIB;

	D3DXVECTOR3 m_v3Center;

	WORD m_wBlurTextureSize;
	CGraphicImageInstance * m_pImageInstance;
	std::vector<CRainParticle*> m_kVct_pkParticleRain;

	DWORD m_dwParticleMaxNum;
	BOOL m_bBlurEnable;

	BOOL m_bRainEnable;
};
