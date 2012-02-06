#pragma once
#include "ElapsedTimeCounter.h"

class RenderWindowTextPainter;

class RenderChannelBase
{
protected:
	struct Parameters
	{
		INT								FontHeight;
		INT								FontWidth;
		UINT							FontWeight;
		char							FontFace[0x100];
		D3DCOLOR						Color;
		RECT							DrawArea;
		UInt32							DrawAreaFlags;
		DWORD							DrawFormat;

		Parameters(INT FontHeight,
			INT FontWidth,
			UINT FontWeight,
			const char* FontFace,
			D3DCOLOR Color,
			RECT* DrawArea,
			DWORD DrawFormat,
			UInt32 DrawAreaFlags);
	};

	LPD3DXFONT						Font;
	RECT							RenderArea;
	bool							Valid;
	Parameters						InputParams;

	RenderChannelBase(INT FontHeight,
		INT FontWidth,
		UINT FontWeight,
		const char* FontFace,
		D3DCOLOR Color,
		RECT* DrawArea,
		DWORD DrawFormat,
		UInt32 DrawAreaFlags = 0);

	virtual void					Render(void* Parameter, LPD3DXSPRITE RenderToSprite) = 0;
	virtual bool					Create();
	virtual void					Release();
	bool							GetIsValid() const;

	friend class					RenderWindowTextPainter;
public:
	enum
	{
		kDrawAreaFlags_RightAligned		= 1 << 0,
		kDrawAreaFlags_BottomAligned	= 1 << 1,
	};

	virtual ~RenderChannelBase();
};

class StaticRenderChannel : public RenderChannelBase
{
public:
	typedef bool					(* RenderHandler)(std::string& RenderedText);		// return false to skip rendering
protected:
	std::string						RenderText;
	RenderHandler					RenderCallback;

	virtual void					Render(void* Parameter, LPD3DXSPRITE RenderToSprite);

	friend class					RenderWindowTextPainter;
public:
	StaticRenderChannel(INT FontHeight,
		INT FontWidth,
		UINT FontWeight,
		const char* FontFace,
		D3DCOLOR Color,
		RECT* DrawArea,
		DWORD DrawFormat,
		UInt32 DrawAreaFlags = 0,
		RenderHandler RenderCallback = NULL);

	virtual ~StaticRenderChannel();
};

class DynamicRenderChannel : public RenderChannelBase
{
protected:
	struct QueueTask
	{
		std::string					Text;
		float						RemainingTime;

		QueueTask(const char* Text, float SecondsToDisplay);
	};

	std::queue<QueueTask*>			RenderQueue;

	virtual void					Render(void* Parameter, LPD3DXSPRITE RenderToSprite);		// parameter's long double* - TimePassedSinceLastUpdate
	virtual void					Release();

	friend class					RenderWindowTextPainter;
public:
	DynamicRenderChannel(INT FontHeight,
		INT FontWidth,
		UINT FontWeight,
		const char* FontFace,
		D3DCOLOR Color,
		RECT* DrawArea,
		DWORD DrawFormat,
		UInt32 DrawAreaFlags = 0);

	virtual ~DynamicRenderChannel();

	bool							Queue(float SecondsToDisplay, const char* Format, ...);
	UInt32							GetQueueSize() const;
};

class RenderWindowTextPainter
{
	static RenderWindowTextPainter*			Singleton;

	RenderWindowTextPainter();

	typedef std::vector<RenderChannelBase*>	RenderChannelListT;
	RenderChannelListT						RegisteredChannels;
	ElapsedTimeCounter						RenderWindowTimeCounter;
	LPD3DXSPRITE							RenderToSprite;
	bool									Enabled;

	bool									LookupRenderChannel(RenderChannelBase* Channel, RenderChannelListT::iterator& MatchIterator);
	bool									ReleaseSprite(bool Recreate = true);
public:
	static RenderWindowTextPainter*			GetSingleton(void);

	void									Render();
	bool									Release(bool Recreate = true);
	bool									RegisterRenderChannel(RenderChannelBase* Channel);		// must be allocated on the heap, painter takes ownership of the pointer, returns true if successful
	bool									UnregisterRenderChannel(RenderChannelBase* Channel);	// releases the registered channel, returns true if successful

	void									SetEnabled(bool State);
	bool									GetEnabled(void) const;
	bool									GetHasActiveTasks(void) const;
	
	void									Deinitialize();
};

#define RENDERTEXT							RenderWindowTextPainter::GetSingleton()