// ======================================================================
//
// BindableImpl.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_BindableImpl_H
#define INCLUDED_BindableImpl_H

// ======================================================================

namespace DB
{
	class Bindable;
	
	class BindableImpl
	{
	  public:
		BindableImpl(Bindable *owner);
		virtual ~BindableImpl();
		
		virtual bool isNull() const =0;
		virtual void setNull() =0;

	  protected:
		Bindable *m_owner;
	};

}

// ======================================================================

#endif
