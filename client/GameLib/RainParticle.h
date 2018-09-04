#pragma once

struct SParticleVertex
{
	D3DXVECTOR3 v3Pos;
	float u, v;
};

struct BlurVertex
{
	D3DXVECTOR3 pos;
	FLOAT       rhw;
	DWORD       color;
	FLOAT		tu, tv;

	static const DWORD FVF;

	BlurVertex(D3DXVECTOR3 p, float w, DWORD c, float u, float v) :pos(p), rhw(w), color(c), tu(u), tv(v) {}
	~BlurVertex(){};
};

class CRainParticle
{
public:
	CRainParticle();
	~CRainParticle();

	static CRainParticle * New();
	static void Delete(CRainParticle * pRainParticle);
	static void DestroyPool();

	void Init(const D3DXVECTOR3 & c_rv3Pos);

	void SetCameraVertex(const D3DXVECTOR3 & rv3Up, const D3DXVECTOR3 & rv3Cross);
	bool IsActivate();

	const char* randomSonEclaire();
	float randomPositionZEclaire(float z);
	float randomPositionXEclaire(float x);
	float randomPositionYEclaire(float x);
	int graphiqueEclaire;
	bool executeEclaire;
	float tempsTotal;
	bool thunderExecuted;
	void ExecutionSonTonnerre(D3DXVECTOR3 m_v3Position, bool executeEclaire);
	void Update(float fElapsedTime, const D3DXVECTOR3 & c_rv3Pos, bool executeEclaire);
	void UpdateFoudrePhase1();
	void UpdateFoudrePhase2(D3DXVECTOR3 m_v3Position, bool executeEclaire);
	void UpdateFoudrePhase3();
	void GetVerticies(SParticleVertex & rv3Vertex1, SParticleVertex & rv3Vertex2,
		SParticleVertex & rv3Vertex3, SParticleVertex & rv3Vertex4);

protected:
	bool m_bActivate;
	bool m_bChangedSize;
	float m_fHalfWidth;
	float m_fHalfHeight;

	D3DXVECTOR3 m_v3Velocity;

	D3DXVECTOR3 m_v3Up;
	D3DXVECTOR3 m_v3Cross;

	float m_fPeriod;
	float m_fcurRadian;
	float m_fAmplitude;
public:
	static std::vector<CRainParticle*> ms_kVct_RainParticlePool;
	bool m_impactPluie;
	D3DXVECTOR3 m_v3Position;

};
