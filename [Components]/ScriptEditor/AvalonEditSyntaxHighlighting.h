#pragma once

namespace cse
{


namespace scriptEditor
{


namespace textEditor
{


namespace avalonEdit
{


// in retrospect, directly using AvalonEdit's XSHD interfaces would have taken less effort
ref class XSHDColor;

typedef ICSharpCode::AvalonEdit::Highlighting::IHighlightingDefinition		AvalonEditHighlightingDefinition;

// base interfaces
ref class IXSHDElement
{
public:
	virtual String^				Serialize() = 0;

	String^						SerializeNameProperty(String^ Name);
	String^						SerializeFontWeightProperty(bool Bold);
	String^						SerializeColorProperty(Color Foreground, Color Background);
	String^						SerializeNamedColorProperty(XSHDColor^ Color);
};
public interface class IXSHDPropertyName
{
	property String^		Name;
};
public interface class IXSHDPropertyValue
{
	property String^		Value;
};
public interface class IXSHDPropertyColor
{
	property Color			Foreground;
	property Color			Background;
};
public interface class IXSHDPropertyFontWeight
{
	property bool			Bold;
};

// elements
ref class XSHDColor : public IXSHDElement, public IXSHDPropertyName, public IXSHDPropertyColor, public IXSHDPropertyFontWeight
{
	String^					_Name;
	Color					_Foreground;
	Color					_Background;
	bool					_Bold;
public:

	XSHDColor(String^ Name, Color Foreground, Color Background, bool Bold) :
		_Name(Name), _Foreground(Foreground), _Background(Background), _Bold(Bold) {}

		virtual String^			Serialize() override;

		property String^		Name
		{
			virtual String^ get() { return _Name; }
			virtual void set(String^ value) { _Name = value; }
		}
		property Color			Foreground
		{
			virtual Color get() { return _Foreground; }
			virtual void set(Color value) { _Foreground = value; }
		}
		property Color			Background
		{
			virtual Color get() { return _Background; }
			virtual void set(Color value) { _Background = value; }
		}
		property bool			Bold
		{
			virtual bool get() { return _Bold; }
			virtual void set(bool value) { _Bold = value; }
		}
};

ref class XSHDWord : public IXSHDElement, public IXSHDPropertyValue
{
	String^					_Value;
public:
	XSHDWord(String^ Value) : _Value(Value) {}

	virtual String^			Serialize() override;

	property String^		Value
	{
		virtual String^ get() { return _Value; }
		virtual void set(String^ value) { _Value = value; }
	}
};

ref class XSHDKeywords : public IXSHDElement, public IXSHDPropertyColor, public IXSHDPropertyFontWeight
{
	XSHDColor^						NamedColor;
	LinkedList<XSHDWord^>^			Words;

	Color							_Foreground;
	Color							_Background;
	bool							_Bold;
public:
	XSHDKeywords(XSHDColor^ NamedColor, Color Foreground, Color Background, bool Bold) : Words(gcnew LinkedList<XSHDWord^>()), NamedColor(NamedColor), _Foreground(Foreground), _Background(Background), _Bold(Bold) {}

	virtual String^			Serialize() override;

	void					AddWord(XSHDWord^ Word);

	property Color			Foreground
	{
		virtual Color get() { return _Foreground; }
		virtual void set(Color value) { _Foreground = value; }
	}
	property Color			Background
	{
		virtual Color get() { return _Background; }
		virtual void set(Color value) { _Background = value; }
	}
	property bool			Bold
	{
		virtual bool get() { return _Bold; }
		virtual void set(bool value) { _Bold = value; }
	}
};

ref class XSHDBegin : public IXSHDElement, public IXSHDPropertyValue
{
	XSHDColor^					NamedColor;

	String^						_Value;
public:
	XSHDBegin(XSHDColor^ NamedColor, String^ Value) : NamedColor(NamedColor), _Value(Value) {}

	virtual String^			Serialize() override;

	property String^		Value
	{
		virtual String^ get() { return _Value; }
		virtual void set(String^ value) { _Value = value; }
	}
};

ref class XSHDEnd : public IXSHDElement, public IXSHDPropertyValue
{
	String^						_Value;
public:
	XSHDEnd(String^ Value) : _Value(Value) {}

	virtual String^			Serialize() override;

	property String^		Value
	{
		virtual String^ get() { return _Value; }
		virtual void set(String^ value) { _Value = value; }
	}
};

ref class XSHDRuleset;

ref class XSHDSpan : public IXSHDElement
{
	XSHDColor^						NamedColor;
	XSHDRuleset^					Ruleset;
	bool							MultiLine;

	LinkedList<IXSHDElement^>^		Children;
public:
	XSHDSpan(XSHDColor^ NamedColor, XSHDRuleset^ Ruleset, bool MultiLine) : Children(gcnew LinkedList<IXSHDElement^>()), NamedColor(NamedColor), Ruleset(Ruleset), MultiLine(MultiLine) {}

	virtual String^					Serialize() override;

	void							AddChild(IXSHDElement^ Child);
};

ref class XSHDRuleset : public IXSHDElement, public IXSHDPropertyName
{
	LinkedList<IXSHDElement^>^		Children;

	String^							_Name;
public:
	XSHDRuleset(String^ Name) : Children(gcnew LinkedList<IXSHDElement^>()), _Name(Name) {}

	virtual String^					Serialize() override;

	void							AddChild(IXSHDElement^ Child);

	property String^		Name
	{
		virtual String^ get() { return _Name; }
		virtual void set(String^ value) { _Name = value; }
	}
};

ref class XSHDRule : public IXSHDElement, public IXSHDPropertyValue
{
	XSHDColor^					NamedColor;

	String^						_Value;
public:
	XSHDRule(XSHDColor^ NamedColor, String^ Value) : NamedColor(NamedColor), _Value(Value) {}

	virtual String^			Serialize() override;

	property String^		Value
	{
		virtual String^ get() { return _Value; }
		virtual void set(String^ value) { _Value = value; }
	}
};

ref class XSHDArbitrary : public IXSHDElement, public IXSHDPropertyValue
{
	String^						_Value;
public:
	XSHDArbitrary(String^ Value) : _Value(Value) {}

	virtual String^			Serialize() override;

	property String^		Value
	{
		virtual String^ get() { return _Value; }
		virtual void set(String^ value) { _Value = value; }
	}
};

// definition manager
ref class AvalonEditXSHDManager
{
protected:
	List<String^>^											GetKeywordList(void);
	List<String^>^											GetBlockTypeList(void);

	String^													CommentMarkerRuleset;
	String^													StableDefinitions;			// XML of the base highlighting defs, updated
	XSHDColor^												LocalVarsColor;
public:
	void													UpdateBaseDefinitions(void);
	AvalonEditHighlightingDefinition^						GenerateHighlightingDefinition(List<String^>^ LocalVariables);

	AvalonEditXSHDManager();
	~AvalonEditXSHDManager();
};


} // namespace avalonEdit


} // namespace textEditor


} // namespace scriptEditor


} // namespace cse