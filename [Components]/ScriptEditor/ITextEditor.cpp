#include "ITextEditor.h"


namespace cse
{


namespace scriptEditor
{


namespace textEditor
{


TextEditorScriptModifiedEventArgs::TextEditorScriptModifiedEventArgs(bool ModifiedStatus)
{
	this->ModifiedStatus = ModifiedStatus;
}

TextEditorMouseClickEventArgs::TextEditorMouseClickEventArgs(MouseButtons Button, int Clicks, int X, int Y, int ScriptTextOffset)
	: MouseEventArgs(Button, Clicks, X, Y, 0)
{
	this->ScriptTextOffset = ScriptTextOffset;
}

FindReplaceResult::HitData::HitData(UInt32 Line, String^ Text, UInt32 Hits)
	: Line(Line), Text(Text), HitCount(Hits)
{
}

FindReplaceResult::FindReplaceResult()
	: Hits(gcnew List<HitData^>), HasError(false)
{
}

void FindReplaceResult::Add(UInt32 Line, String^ Text, UInt32 HitsInLine)
{
	Text->Replace("\t", "")->Replace("\r\n", "")->Replace("\n", "");
	Hits->Add(gcnew HitData(Line, Text, HitsInLine));
}

int FindReplaceResult::TotalHits::get()
{
	int Count = 0;

	for each (auto Itr in Hits)
		Count += Itr->HitCount;

	return Count;
}


} // namespace textEditor


} // namespace scriptEditor


} // namespace cse