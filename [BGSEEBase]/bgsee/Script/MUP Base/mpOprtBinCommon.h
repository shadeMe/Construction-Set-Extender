/*
			   __________                                 ____  ___
	_____  __ _\______   \_____ _______  ______ __________\   \/  /
   /     \|  |  \     ___/\__  \\_  __ \/  ___// __ \_  __ \     /
  |  Y Y  \  |  /    |     / __ \|  | \/\___ \\  ___/|  | \/     \
  |__|_|  /____/|____|    (____  /__|  /____  >\___  >__| /___/\  \
		\/                     \/           \/     \/           \_/
									   Copyright (C) 2012 Ingo Berg
									   All rights reserved.

  muParserX - A C++ math parser library with array and string support
  Copyright (c) 2012, Ingo Berg
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright notice,
	 this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright notice,
	 this list of conditions and the following disclaimer in the documentation
	 and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef MP_OPRT_BIN_H
#define MP_OPRT_BIN_H

/** \file
	\brief Definitions of classes used as callbacks for standard binary operators.
*/

/** \defgroup binop Binary operator callbacks

  This group lists the objects representing the binary operators of muParserX.
*/

//#include <cmath>
#include "mpIOprt.h"
#include "CodaMUPValue.h"
#include "mpError.h"

namespace bgsee { namespace script { namespace mup {
#define MUP_NAME_PROXY_get_float_type  GetFloat

  //-----------------------------------------------------------------------------------------------
  class OprtStrAdd : public IOprtBin
  {
  public:
	OprtStrAdd();
	virtual void Eval(ptr_val_type& ret, const ptr_val_type *arg, int argc);
	virtual const char_type* GetDesc() const;
	virtual IToken* Clone() const;
  }; 

  //-----------------------------------------------------------------------------------------------
  /** \brief Callback object for testing if two values are equal.
	  \ingroup binop
  */
  class OprtEQ : public IOprtBin    
  {
  public:
	OprtEQ();
	virtual void Eval(ptr_val_type& ret, const ptr_val_type *a_pArg, int);
	virtual const char_type* GetDesc() const;
	virtual IToken* Clone() const;
  };


  //------------------------------------------------------------------------------
  /** \brief Callback object for testing if two values are not equal.
	  \ingroup binop
  */
  class OprtNEQ : public IOprtBin    
  {
  public:
	OprtNEQ();
	virtual void Eval(ptr_val_type& ret, const ptr_val_type *a_pArg, int);
	virtual const char_type* GetDesc() const;
	virtual IToken* Clone() const;
  };

  //------------------------------------------------------------------------------
  /** \brief Callback object class for the "Less than" operator.
	  \ingroup binop
  */
  class OprtLT : public IOprtBin    
  {
  public:
	OprtLT();
	virtual void Eval(ptr_val_type& ret, const ptr_val_type *a_pArg, int);
	virtual const char_type* GetDesc() const;
	virtual IToken* Clone() const;
  };

  //------------------------------------------------------------------------------
  /** \brief Callback object class for the "Greater than" operator.
	  \ingroup binop
  */
  class OprtGT : public IOprtBin    
  {
  public:
	OprtGT();
	virtual void Eval(ptr_val_type& ret, const ptr_val_type *a_pArg, int);
	virtual const char_type* GetDesc() const;
	virtual IToken* Clone() const;
  };

  //------------------------------------------------------------------------------
  /** \brief Callback object class for the "Less or equal" operator.
	  \ingroup binop
  */
  class OprtLE : public IOprtBin    
  {
  public:
	OprtLE();
	virtual void Eval(ptr_val_type& ret, const ptr_val_type *a_pArg, int);
	virtual const char_type* GetDesc() const;
	virtual IToken* Clone() const;
  };

  //------------------------------------------------------------------------------
  /** \brief Callback object class for the "Greater or or equal" operator.
	  \ingroup binop
  */
  class OprtGE : public IOprtBin    
  {
  public:
	OprtGE();
	virtual void Eval(ptr_val_type& ret, const ptr_val_type *a_pArg, int);
	virtual const char_type* GetDesc() const;
	virtual IToken* Clone() const;
  };

  //------------------------------------------------------------------------------
  /** \brief Callback class for a logic and operator.
	  \ingroup binop
  */
  class OprtAnd : public IOprtBin    
  {
  public:
	OprtAnd();
	virtual void Eval(ptr_val_type& ret, const ptr_val_type *a_pArg, int);
	virtual const char_type* GetDesc() const;
	virtual IToken* Clone() const;
  };

  //------------------------------------------------------------------------------
  /** \brief Callback class for a logic or operator.
	  \ingroup binop
  */
  class OprtOr : public IOprtBin    
  {
  public:
	OprtOr();
	virtual void Eval(ptr_val_type& ret, const ptr_val_type *a_pArg, int);
	virtual const char_type* GetDesc() const;
	virtual IToken* Clone() const;
  };

  //------------------------------------------------------------------------------
  /** \brief Callback class for a logical or operator.
	  \ingroup binop
  */
  class OprtLOr : public IOprtBin    
  {
  public:
	OprtLOr(const char_type *szIdent = _T("||"));
	virtual void Eval(ptr_val_type& ret, const ptr_val_type *a_pArg, int);
	virtual const char_type* GetDesc() const;
	virtual IToken* Clone() const;
  };

  //------------------------------------------------------------------------------
  /** \brief Callback class for a logical and operator.
	  \ingroup binop
  */
  class OprtLAnd : public IOprtBin    
  {
  public:
	OprtLAnd(const char_type *szIdent = _T("&&"));
	virtual void Eval(ptr_val_type& ret, const ptr_val_type *a_pArg, int);
	virtual const char_type* GetDesc() const;
	virtual IToken* Clone() const;
  };

  //------------------------------------------------------------------------------
  /** \brief Callback class for the shift left operator.
	  \ingroup binop
  */
  class OprtShl : public IOprtBin    
  {
  public:
	OprtShl();
	virtual void Eval(ptr_val_type& ret, const ptr_val_type *a_pArg, int);
	virtual const char_type* GetDesc() const;
	virtual IToken* Clone() const;
  };

  //------------------------------------------------------------------------------
  /** \brief Callback class for the shift right operator.
	  \ingroup binop
  */
  class OprtShr : public IOprtBin    
  {
  public:
	OprtShr();
	virtual void Eval(ptr_val_type& ret, const ptr_val_type *a_pArg, int);
	virtual const char_type* GetDesc() const;
	virtual IToken* Clone() const;
  };

	class OprtCastToNum : public IOprtInfix
	{
	public:
		OprtCastToNum() : IOprtInfix("(num)", prINFIX) {}

		virtual void				Eval(mup::ptr_val_type &ret, const ptr_val_type *a_pArg, int a_iArgc);
		virtual const char_type*	GetDesc() const;
		virtual IToken*				Clone() const;
	};

	class OprtCastToRef : public IOprtInfix
	{
	public:
		OprtCastToRef() : IOprtInfix("(ref)", prINFIX) {}

		virtual void				Eval(mup::ptr_val_type &ret, const ptr_val_type *a_pArg, int a_iArgc);
		virtual const char_type*	GetDesc() const;
		virtual IToken*				Clone() const;
	};

	class OprtCastToStr : public IOprtInfix
	{
	public:
		OprtCastToStr(const char* ID) : IOprtInfix(ID, prINFIX) {}

		virtual void				Eval(mup::ptr_val_type &ret, const ptr_val_type *a_pArg, int a_iArgc);
		virtual const char_type*	GetDesc() const;
		virtual IToken*				Clone() const;
	};

	class OprtCastToArray : public IOprtInfix
	{
	public:
		OprtCastToArray() : IOprtInfix("(array)", prINFIX) {}

		virtual void				Eval(mup::ptr_val_type &ret, const ptr_val_type *a_pArg, int a_iArgc);
		virtual const char_type*	GetDesc() const;
		virtual IToken*				Clone() const;
	};

	class OprtTypeID : public IOprtInfix
	{
	public:
		OprtTypeID() : IOprtInfix("(typeID)", prINFIX) {}

		virtual void				Eval(mup::ptr_val_type &ret, const ptr_val_type *a_pArg, int a_iArgc);
		virtual const char_type*	GetDesc() const;
		virtual IToken*				Clone() const;
	};
} } }  // namespace mu

#endif