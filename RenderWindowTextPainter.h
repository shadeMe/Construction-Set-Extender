#pragma once

class RenderWindowTextPainter
{
	static RenderWindowTextPainter*		Singleton;

	RenderWindowTextPainter();

	class RenderChannelBase
	{
	protected:
		LPD3DXFONT						Font;
		D3DCOLOR						Color;
		RECT							DrawArea;

		bool							Valid;

		RenderChannelBase(INT FontHeight, INT FontWidth, UINT FontWeight, const char* FontFace, DWORD Color, RECT* DrawArea);
	public:
		virtual void					Render() = 0;
		virtual void					Release()
		{
			if (Valid == false)
				return;

			Font->Release();
		}
		bool							GetIsValid() { return Valid; }
	};

	class StaticRenderChannel : public RenderChannelBase
	{
		std::string						TextToRender;
	public:
		StaticRenderChannel(INT FontHeight,
							INT FontWidth,
							UINT FontWeight,
							const char* FontFace,
							DWORD Color,
							RECT* DrawArea) : RenderChannelBase(FontHeight, FontWidth, FontWeight, FontFace, Color, DrawArea) {}

		virtual void					Render();
		void							Queue(const char* Text);
		UInt32							GetQueueSize() { return (TextToRender.length() < 1); }
	};

	class DynamicRenderChannel : public RenderChannelBase
	{
		long double						TimeLeft;

		struct QueueTask
		{
			std::string					Text;
			long double					RemainingTime;

			QueueTask(const char* Text, long double SecondsToDisplay) :  Text(Text), RemainingTime(SecondsToDisplay) {}
		};

		std::queue<QueueTask*>			DrawQueue;
	public:
		DynamicRenderChannel(INT FontHeight,
							INT FontWidth,
							UINT FontWeight,
							const char* FontFace,
							DWORD Color,
							RECT* DrawArea) : RenderChannelBase(FontHeight, FontWidth, FontWeight, FontFace, Color, DrawArea), TimeLeft(0) {}

		virtual void					Render();
		virtual void					Release();
		void							Queue(const char* Text, long double SecondsToDisplay);
		UInt32							GetQueueSize() { return DrawQueue.size(); }
	};

	StaticRenderChannel*					RenderChannel1;
	DynamicRenderChannel*					RenderChannel2;
	bool									Valid;
public:

	enum
	{
		kRenderChannel_1 = 0,			// static
		kRenderChannel_2				// dynamic
	};

	static RenderWindowTextPainter*		GetSingleton(void);

	bool								Initialize();			// must be called during init
	bool								Recreate();
	void								Render();
	void								Release();
	void								QueueDrawTask(UInt8 Channel, const char* Text, long double SecondsToDisplay);

	UInt32								GetRenderChannelQueueSize(UInt8 Channel);
};

#define RENDERTEXT								RenderWindowTextPainter::GetSingleton()
#define PrintToRender(message, duration)		RENDERTEXT->QueueDrawTask(RenderWindowTextPainter::kRenderChannel_2, message, duration)