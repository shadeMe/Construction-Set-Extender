#pragma once

// 280		### partial
class Archive
{
public:
	Archive();
	~Archive();

	// 22+? (nothing beyond 0x22 initialized)
	struct Unk154
	{
		UInt32					unk00;			// initialized to 'BSA'
		UInt32					unk04;			// initialized to 0x67
		UInt32					unk08;			// initalized to 0x24
		UInt32					unk0C;
		UInt32					unk10;
		UInt32					unk14;
		UInt32					unk18;
		UInt32					unk1C;
		UInt16					unk20;			// flags of some sort
	};

	// bases
	BSFile						bsfile;			// 000

	//members
	Unk154						unk154;			// 154
	UInt32						unk176;			// 176
	UInt32						unk194;			// 194	bitfield
	LPCRITICAL_SECTION			archiveCS;		// 200
};

void InitializeArchives();