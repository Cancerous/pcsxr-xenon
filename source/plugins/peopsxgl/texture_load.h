
// texture.cpp
unsigned char * CheckTextureInSubSCache(int TextureMode, uint32_t GivenClutId, unsigned short *pCache);
void CompressTextureSpace(void);


void LoadSubTexturePageSort(int pageid, int mode, short cx, short cy);
void LoadPackedSubTexturePageSort(int pageid, int mode, short cx, short cy);
void DefineSubTextureSort(void);

GpuTex * Fake15BitTexture(void);


