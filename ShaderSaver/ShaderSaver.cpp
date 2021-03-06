// ShaderSaver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <assert.h>
#include "SDL.h"
#include "GL\glew.h"
#include "Renderer.h"
#include "jsonxx.h"
#include "Timer.h"
#include "FFT.h"
#include <fstream>

using namespace std;

void ReplaceTokens(std::string &sDefShader, const char * sTokenBegin, const char * sTokenName, const char * sTokenEnd, std::vector<std::string> &tokens)
{
	if (sDefShader.find(sTokenBegin) != std::string::npos
		&& sDefShader.find(sTokenName) != std::string::npos
		&& sDefShader.find(sTokenEnd) != std::string::npos
		&& sDefShader.find(sTokenBegin) < sDefShader.find(sTokenName)
		&& sDefShader.find(sTokenName) < sDefShader.find(sTokenEnd))
	{
		int nTokenStart = sDefShader.find(sTokenBegin) + strlen(sTokenBegin);
		std::string sTextureToken = sDefShader.substr(nTokenStart, sDefShader.find(sTokenEnd) - nTokenStart);

		std::string sFinalShader;
		sFinalShader = sDefShader.substr(0, sDefShader.find(sTokenBegin));

		for (int i = 0; i < tokens.size(); i++)
		{
			std::string s = sTextureToken;
			while (s.find(sTokenName) != std::string::npos)
			{
				s.replace(s.find(sTokenName), strlen(sTokenName), tokens[i], 0, std::string::npos);
			}
			sFinalShader += s;
		}
		sFinalShader += sDefShader.substr(sDefShader.find(sTokenEnd) + strlen(sTokenEnd), std::string::npos);
		sDefShader = sFinalShader;
	}
}

// Initialization routine.
void setup(void)
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 100.0, 0.0, 100.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// Routine to output interaction instructions to the C++ window.
void printInteraction(void)
{
	cout << "Interaction:" << endl;
	cout << "Press +/- to increase/decrease the number of vertices on the circle." << endl;
	cout << "Press esc to quit." << endl;
}

void update(bool *isClosed) {
	SDL_Event e;

	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_QUIT || e.type == SDL_KEYDOWN || e.type == SDL_MOUSEBUTTONDOWN || (e.type == SDL_MOUSEMOTION && (abs(e.motion.xrel) > 1 || abs(e.motion.yrel) > 1)))
			*isClosed = true;
	}
}

int main(int argc, char *argv[])
{
	string title = "Square";
	if (argc < 2) {
		//return 1;
	}

	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "/C") {
			//configure
			title = "Configure";
			//if (!Renderer::OpenSetupDialog(&settings))
				//return -1;
		}
		else if (arg == "/P") {
			if (i + 1 < argc) { // Make sure we aren't at the end of argv!
				//preview
				title = "Preview";
								//destination = argv[i++]; // Increment 'i' so we don't get the argument as the next argv[i].
			}
			else { // Uh-oh, there was no argument to the destination option.
				std::cerr << "--destination option requires one argument." << std::endl;
				return 1;
			}
		}
		else if (arg == "/S") {
			//Run
			title = "Run";
		}
	}

	string line;
	string file;

	ifstream myfile("config.json");
	if (myfile.is_open())
	{
		while (getline(myfile, line))
		{
			file += line + '\n';
		}
		myfile.close();
	}

	else cout << "Unable to open config file" << endl;

	jsonxx::Object options;

	options.parse(file);

	RENDERER_SETTINGS settings;
	settings.bVsync = false;
#ifdef _DEBUG
	settings.nWidth = 1280;
	settings.nHeight = 720;
	settings.windowMode = RENDERER_WINDOWMODE_WINDOWED;
#else
	settings.nWidth = 1920;
	settings.nHeight = 1080;
	settings.windowMode = RENDERER_WINDOWMODE_FULLSCREEN;
	if (options.has<jsonxx::Object>("window"))
	{
		if (options.get<jsonxx::Object>("window").has<jsonxx::Number>("width"))
			settings.nWidth = options.get<jsonxx::Object>("window").get<jsonxx::Number>("width");
		if (options.get<jsonxx::Object>("window").has<jsonxx::Number>("height"))
			settings.nHeight = options.get<jsonxx::Object>("window").get<jsonxx::Number>("height");
		if (options.get<jsonxx::Object>("window").has<jsonxx::Boolean>("fullscreen"))
			settings.windowMode = options.get<jsonxx::Object>("window").get<jsonxx::Boolean>("fullscreen") ? RENDERER_WINDOWMODE_FULLSCREEN : RENDERER_WINDOWMODE_WINDOWED;
	}	
#endif

	bool isClosed = false;

	if (!Renderer::Open(&settings))
	{
		printf("Renderer::Open failed\n");
		return -1;
	}

	if (!FFT::Open())
	{
		printf("FFT::Open() failed, continuing anyway...\n");
		//return -1;
	}

	std::map<std::string, Renderer::Texture*> textures;

	float fFFTSmoothingFactor = 0.9f; // higher value, smoother FFT
	float fFFTSlightSmoothingFactor = 0.6f; // higher value, smoother FFT

	if (!options.empty())
	{
		if (options.has<jsonxx::Object>("rendering"))
		{
			if (options.get<jsonxx::Object>("rendering").has<jsonxx::Number>("fftSmoothFactor"))
				fFFTSmoothingFactor = options.get<jsonxx::Object>("rendering").get<jsonxx::Number>("fftSmoothFactor");
		}

		if (options.has<jsonxx::Object>("textures"))
		{
			printf("Loading textures...\n");
			std::map<std::string, jsonxx::Value*> tex = options.get<jsonxx::Object>("textures").kv_map();
			for (std::map<std::string, jsonxx::Value*>::iterator it = tex.begin(); it != tex.end(); it++)
			{
				char * fn = (char*)it->second->string_value_->c_str();
				printf("* %s...\n", fn);
				Renderer::Texture * tex = Renderer::CreateRGBA8TextureFromFile(fn);
				if (!tex)
				{
					printf("Renderer::CreateRGBA8TextureFromFile(%s) failed\n", fn);
					return -1;
				}
				textures.insert(std::make_pair(it->first, tex));
			}
		}
	}

	Renderer::Texture * texFFT = Renderer::Create1DR32Texture(FFT_SIZE);
	Renderer::Texture * texFFTSmoothed = Renderer::Create1DR32Texture(FFT_SIZE);
	Renderer::Texture * texFFTIntegrated = Renderer::Create1DR32Texture(FFT_SIZE);

	bool shaderInitSuccessful = false;
	char szShader[65535];
	char szError[4096];
#pragma warning(suppress: 4996)
	FILE * f = fopen(Renderer::defaultShaderFilename.c_str(), "rb");
	if (f)
	{
		printf("Loading last shader...\n");

		memset(szShader, 0, 65535);
		int n = fread(szShader, 1, 65535, f);
		fclose(f);
		if (Renderer::ReloadShader(szShader, strlen(szShader), szError, 4096))
		{
			printf("Last shader works fine.\n");
			shaderInitSuccessful = true;
		}
		else {
			printf("Shader error:\n%s\n", szError);
		}
	}
	if (!shaderInitSuccessful)
	{
		printf("No valid last shader found, falling back to default...\n");

		std::string sDefShader = Renderer::defaultShader;
		std::vector<std::string> tokens;
		for (std::map<std::string, Renderer::Texture*>::iterator it = textures.begin(); it != textures.end(); it++)
			tokens.push_back(it->first);
		ReplaceTokens(sDefShader, "{%textures:begin%}", "{%textures:name%}", "{%textures:end%}", tokens);

		tokens.clear();

#pragma warning(suppress: 4996)
		strncpy(szShader, sDefShader.c_str(), 65535);
		if (!Renderer::ReloadShader(szShader, strlen(szShader), szError, 4096))
		{
			printf("Default shader compile failed:\n");
			puts(szError);
			assert(0);
		}
	}

	static float fftData[FFT_SIZE];
	memset(fftData, 0, sizeof(float) * FFT_SIZE);
	static float fftDataSmoothed[FFT_SIZE];
	memset(fftDataSmoothed, 0, sizeof(float) * FFT_SIZE);


	static float fftDataSlightlySmoothed[FFT_SIZE];
	memset(fftDataSlightlySmoothed, 0, sizeof(float) * FFT_SIZE);
	static float fftDataIntegrated[FFT_SIZE];
	memset(fftDataIntegrated, 0, sizeof(float) * FFT_SIZE);

	bool bShowGui = false;
	Timer::Start();
	float fNextTick = 0.1;
	while (!isClosed)
	{
		float time = Timer::GetTime() / 1000.0; // seconds
		Renderer::StartFrame();

		Renderer::SetShaderConstant(string("fGlobalTime"), time);
		Renderer::SetShaderConstant(string("v2Resolution"), settings.nWidth, settings.nHeight);

		if (FFT::GetFFT(fftData))
		{
			Renderer::UpdateR32Texture(texFFT, fftData);

			const static float maxIntegralValue = 1024.0f;
			for (int i = 0; i < FFT_SIZE; i++)
			{
				fftDataSmoothed[i] = fftDataSmoothed[i] * fFFTSmoothingFactor + (1 - fFFTSmoothingFactor) * fftData[i];

				fftDataSlightlySmoothed[i] = fftDataSlightlySmoothed[i] * fFFTSlightSmoothingFactor + (1 - fFFTSlightSmoothingFactor) * fftData[i];
				fftDataIntegrated[i] = fftDataIntegrated[i] + fftDataSlightlySmoothed[i];
				if (fftDataIntegrated[i] > maxIntegralValue) {
					fftDataIntegrated[i] -= maxIntegralValue;
				}
			}

			Renderer::UpdateR32Texture(texFFTSmoothed, fftDataSmoothed);
			Renderer::UpdateR32Texture(texFFTIntegrated, fftDataIntegrated);
		}

		Renderer::SetShaderTexture(string("texFFT"), texFFT);
		Renderer::SetShaderTexture(string("texFFTSmoothed"), texFFTSmoothed);
		Renderer::SetShaderTexture(string("texFFTIntegrated"), texFFTIntegrated);

		for (std::map<std::string, Renderer::Texture*>::iterator it = textures.begin(); it != textures.end(); it++)
		{
			Renderer::SetShaderTexture((char*)it->first.c_str(), it->second);
		}

		Renderer::RenderFullscreenQuad();

		Renderer::EndFrame();

		update(&isClosed);
	}

	FFT::Close();

	Renderer::ReleaseTexture(texFFT);
	Renderer::ReleaseTexture(texFFTSmoothed);
	for (std::map<std::string, Renderer::Texture*>::iterator it = textures.begin(); it != textures.end(); it++)
	{
		Renderer::ReleaseTexture(it->second);
	}

	Renderer::WantsToQuit();

	return 0;
}