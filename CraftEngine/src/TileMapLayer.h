#pragma once

#include "x-gpu-ifc.h"
#include "x-BitmapData.h"

#include "Entity.h"
#include "UniformMeshes.h"

class ViewCamera
{
public:
    EntityGid_t             m_gid;
    float4                  m_Eye;
    float4                  m_At;
    float4                  m_Up;           // X is angle.  Y is just +/- (orientation)? Z is unused?
    float2                  m_frustrum_in_tiles;
    float2                  m_tile_size_pix;
    float                   m_aspect;
    GPU_ViewCameraConsts    m_Consts;

    ViewCamera() {
    }

    void            InitScene       ();
    void            UpdateFrustrum  ();
    void            SetEyeAt        (const float2& xy);
    float4          ClientToWorld   (const int2& clientPosInPix);

    virtual void Tick();

};

enum class TerrainClass
{
    Empty,
    Water,
    Sandy,
    Grassy,
};

struct TileMapItem {
    int         tile_below;         // should always be a Solid (no edge/cornering tiles allowed)
    int         tile_above;         // can be edges, corners, etc.  No Solids allowed.
};

struct TerrainMapItem
{
    TerrainClass   class_below;        // tile classification below-ground
    TerrainClass   class_above;        // tile classification above-ground
};

class OpenWorldEnviron
{
public:
    EntityGid_t m_gid;

public:
    void InitScene();
    virtual void Tick();
};


class TileMapLayer
{
public:
    struct GPU_TileMapConstants
    {
        vFloat2 TileAlignedDisp;
        vInt2   SrcTexSizeInTiles;
        vInt2   SrcTexTileSizePix;
        vInt2   SrcTexBorderPix;
        vInt2   ViewMeshSize;
    };

public:
    EntityGid_t             m_gid;

    // No effort is made here to break out shared resources, such as tile or layout.
    // A meaningful advantage would only become worthwhile if 20 or more tilemap layers
    // are being used during a scene.  Seems highly unlikely --jstine

    struct {
        GPU_InputDesc           layout_tilemap;
        GPU_TextureResource2D   tex_floor;
        GPU_VertexBuffer        mesh_tile;
        GPU_DynVsBuffer         mesh_worldViewTileID;
        GPU_TileMapConstants    consts;
    } gpu;


    int2    m_setCount;

    float2  TileAlignedDisp;
    int2    ViewMeshSize;
    int     ViewInstanceCount;
    int     ViewVerticiesCount;

    int     m_data_offset_uv;
    int     m_edge_tile;
    bool    m_enableDraw;

public:
    TileMapLayer();

    void        SetDataOffsetUV     (int offset_in_words);
    void        PopulateUVs         (const void* terrain_data, int stride_in_words, const int2& viewport_offset);
    void        PopulateUVs         (const void* terrain_data, int stride_in_words);
    void        InitScene           (const char* script_objname);
    void        SetSourceTexture    (const xBitmapDataRO& srctex, const int2& setCount);
    void        SetSourceTexture    (const TextureAtlas&  atlas);
    void        CenterViewOn        (const float2& dest);

    template<typename T> void PopulateUVs (const T* terrain_data, const int2& viewport_offset);
    template<typename T> void PopulateUVs (const T* terrain_data);

    virtual void Tick();
    virtual void Draw() const;
};

template<typename T> inline void TileMapLayer::PopulateUVs (const T* terrain_data, const int2& viewport_offset)
{
    int struct_size_words = sizeof(T)/4;
    PopulateUVs(terrain_data, struct_size_words);
}

template<typename T> inline void TileMapLayer::PopulateUVs (const T* terrain_data)
{
    int struct_size_words = sizeof(T)/4;
    PopulateUVs(terrain_data, struct_size_words);
}

inline void TileMapLayer::SetDataOffsetUV(int offset_in_words)
{
    m_data_offset_uv = offset_in_words;
}

extern ViewCamera           g_ViewCamera;
extern TileMapLayer         g_GroundLayerAbove;
extern TileMapLayer         g_GroundLayerBelow;

extern GPU_ShaderVS         g_ShaderVS_Tiler;
extern GPU_ShaderFS         g_ShaderFS_Tiler;
extern GPU_ConstantBuffer   g_cnstbuf_TileMap;

static const int TileSizeX = 8;
static const int TileSizeY = 8;

// TODO: Make this dynamic ... and pair with TerrtainMapItem

static const int WorldSizeX     = 1024;
static const int WorldSizeY     = 1024;


extern void WorldMap_Procgen();
