#pragma once
typedef enum {
	RENDERER_WINDOWMODE_WINDOWED = 0,
	RENDERER_WINDOWMODE_FULLSCREEN,
	RENDERER_WINDOWMODE_BORDERLESS
} RENDERER_WINDOWMODE;

typedef struct
{
	int nWidth;
	int nHeight;
	RENDERER_WINDOWMODE windowMode;
	bool bVsync;
} RENDERER_SETTINGS;

namespace Renderer
{
	extern std::string defaultShaderFilename;
	extern char defaultShader[65536];

	extern int nWidth;
	extern int nHeight;

	bool OpenSetupDialog(RENDERER_SETTINGS * settings);
	bool Open(RENDERER_SETTINGS * settings);

	void StartFrame();
	void EndFrame();
	bool WantsToQuit();

	void RenderFullscreenQuad();

	bool ReloadShader(char * szShaderCode, int nShaderCodeSize, char * szErrorBuffer, int nErrorBufferSize);
	void SetShaderConstant(std::string szConstName, float x);
	void SetShaderConstant(std::string szConstName, float x, float y);

	bool GrabFrame(void * pPixelBuffer); // input buffer must be able to hold w * h * 4 bytes of 0xAABBGGRR data

	enum TEXTURETYPE
	{
		TEXTURETYPE_1D = 1,
		TEXTURETYPE_2D = 2,
	};

	struct Texture
	{
		int width;
		int height;
		TEXTURETYPE type;
	};

	Texture * CreateRGBA8TextureFromFile(char * szFilename);
	Texture * CreateA8TextureFromData(int w, int h, unsigned char * data);
	Texture * Create1DR32Texture(int w);
	bool UpdateR32Texture(Texture * tex, float * data);
	void SetShaderTexture(std::string szTextureName, Texture * tex);
	void BindTexture(Texture * tex); // temporary function until all the quad rendering is moved to the renderer
	void ReleaseTexture(Texture * tex);
	struct Vertex
	{
		Vertex(float _x, float _y, unsigned int _c = 0xFFFFFFFF, float _u = 0.0, float _v = 0.0) :
			x(_x), y(_y), c(_c), u(_u), v(_v) {}
		float x, y;
		float u, v;
		unsigned int c;
	};
	void RenderQuad(const Vertex & a, const Vertex & b, const Vertex & c, const Vertex & d);
	void RenderLine(const Vertex & a, const Vertex & b);
}
