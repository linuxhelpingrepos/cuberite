
// DistortedHeightmap.h

// Declares the cDistortedHeightmap class representing the height and composition generator capable of overhangs





#pragma once

#include "ComposableGenerator.h"
#include "HeiGen.h"





#define NOISE_SIZE_Y (257 + 32)





class cDistortedHeightmap :
	public cTerrainHeightGen,
	public cTerrainCompositionGen
{
public:
	/// Structure used for storing block patterns for columns
	struct sBlockInfo
	{
		BLOCKTYPE  BlockType;
		NIBBLETYPE BlockMeta;
	} ;

	cDistortedHeightmap(int a_Seed, cBiomeGenPtr a_BiomeGen);
	
protected:
	typedef cChunkDef::BiomeMap BiomeNeighbors[3][3];

	// Linear upscaling step sizes, must be divisors of cChunkDef::Width and cChunkDef::Height, respectively:
	static const int INTERPOL_X = 8;
	static const int INTERPOL_Y = 4;
	static const int INTERPOL_Z = 8;

	// Linear upscaling buffer dimensions, calculated from the step sizes:
	static const int DIM_X = 1 +  (17 / INTERPOL_X);
	static const int DIM_Y = 1 + (257 / INTERPOL_Y);
	static const int DIM_Z = 1 +  (17 / INTERPOL_Z);

	cPerlinNoise m_NoiseDistortX;
	cPerlinNoise m_NoiseDistortZ;
	cNoise       m_OceanFloorSelect;  ///< Used for selecting between dirt and sand on the ocean floor
	cNoise       m_MesaFloor;         ///< Used for the floor of the clay blocks in mesa biomes

	int m_SeaLevel;
	NOISE_DATATYPE m_FrequencyX;
	NOISE_DATATYPE m_FrequencyY;
	NOISE_DATATYPE m_FrequencyZ;

	int m_CurChunkX;
	int m_CurChunkZ;
	NOISE_DATATYPE m_DistortedHeightmap[17 * 257 * 17];

	/** The bime generator to query for biomes. */
	cBiomeGenPtr m_BiomeGen;

	/** The generator that provides the base heightmap (before distortion). */
	cTerrainHeightGenPtr m_UnderlyingHeiGen;

	/** Cache for m_UnderlyingHeiGen. */
	cHeiGenCache  m_HeightGen;
	
	/// Heightmap for the current chunk, before distortion (from m_HeightGen). Used for optimization.
	cChunkDef::HeightMap m_CurChunkHeights;
	
	// Per-biome terrain generator parameters:
	struct sGenParam
	{
		NOISE_DATATYPE m_DistortAmpX;
		NOISE_DATATYPE m_DistortAmpZ;
	} ;
	static const sGenParam m_GenParam[256];
	
	// Distortion amplitudes for each direction, before linear upscaling
	NOISE_DATATYPE m_DistortAmpX[DIM_X * DIM_Z];
	NOISE_DATATYPE m_DistortAmpZ[DIM_X * DIM_Z];
	
	/// True if Initialize() has been called. Used to initialize-once even with multiple init entrypoints (HeiGen / CompoGen)
	bool m_IsInitialized;
	
	/// The vertical pattern to be used for mesa biomes. Seed-dependant.
	/// One Height of pattern and one Height of stone to avoid checking pattern dimensions
	sBlockInfo m_MesaPattern[2 * cChunkDef::Height];
	

	/// Initializes m_MesaPattern with a reasonable pattern of stained clay / hardened clay, based on the seed
	void InitMesaPattern(int a_Seed);
	
	/// Unless the LastChunk coords are equal to coords given, prepares the internal state (noise arrays, heightmap)
	void PrepareState(int a_ChunkX, int a_ChunkZ);
	
	/// Generates the m_DistortedHeightmap array for the current chunk
	void GenerateHeightArray(void);
	
	/// Calculates the heightmap value (before distortion) at the specified (floating-point) coords
	int GetHeightmapAt(NOISE_DATATYPE a_X, NOISE_DATATYPE a_Z);
	
	/// Updates m_DistortAmpX/Z[] based on m_CurChunkX and m_CurChunkZ
	void UpdateDistortAmps(void);
	
	/// Calculates the X and Z distortion amplitudes based on the neighbors' biomes
	void GetDistortAmpsAt(BiomeNeighbors & a_Neighbors, int a_RelX, int a_RelZ, NOISE_DATATYPE & a_DistortAmpX, NOISE_DATATYPE & a_DistortAmpZ);
	
	/// Reads the settings from the ini file. Skips reading if already initialized
	void Initialize(cIniFile & a_IniFile);
	
	/// Composes a single column in a_ChunkDesc. Chooses what to do based on the biome in that column
	void ComposeColumn(cChunkDesc & a_ChunkDesc, int a_RelX, int a_RelZ);
	
	/// Fills the specified column with the specified pattern; restarts the pattern when air is reached,
	/// switches to ocean floor pattern if ocean is reached. Always adds bedrock at the very bottom.
	void FillColumnPattern(cChunkDesc & a_ChunkDesc, int a_RelX, int a_RelZ, const sBlockInfo * a_Pattern);
	
	/// Fills the specified column with mesa pattern, based on the column height
	void FillColumnMesa(cChunkDesc & a_ChunkDesc, int a_RelX, int a_RelZ);
	
	/// Returns the pattern to use for an ocean floor in the specified column
	const sBlockInfo * ChooseOceanFloorPattern(int a_RelX, int a_RelZ);
	
	
	// cTerrainHeightGen overrides:
	virtual void GenHeightMap(int a_ChunkX, int a_ChunkZ, cChunkDef::HeightMap & a_HeightMap) override;
	virtual void InitializeHeightGen(cIniFile & a_IniFile) override;

	// cTerrainCompositionGen overrides:
	virtual void ComposeTerrain(cChunkDesc & a_ChunkDesc) override;
	virtual void InitializeCompoGen(cIniFile & a_IniFile) override;
} ;
