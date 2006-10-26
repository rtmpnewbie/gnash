// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//

#ifndef GNASH_AS_OBJECT_H
#define GNASH_AS_OBJECT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"

#include <cmath>
#include "container.h"
#include "ref_counted.h" // for inheritance 
#include "PropertyList.h"

namespace gnash {

// Forward declarations
class as_function;
class movie;
class as_value;
class as_environment;

/// \brief
/// A generic bag of attributes. Base class for all ActionScript-able objects.
//
/// Base-class for ActionScript script-defined objects.
/// This would likely be ActionScript's 'Object' class.
///
//class as_object : public resource
class DSOEXPORT as_object : public ref_counted
{
	/// Properties of this objects 
	PropertyList _members;

public:

	void dump_members() const;

	/// Reference to this object's '__proto__'
	// TODO: make private (or protected)
	as_object*	m_prototype;

	/// Construct an ActionScript object with no prototype associated.
	as_object()
		:
		//_members(*this),
		m_prototype(NULL)
	{
	}

	/// \brief
	/// Construct an ActionScript object based on the given prototype.
	/// Adds a reference to the prototype, if any.
	as_object(as_object* proto)
		:
		//_members(*this),
		m_prototype(proto)
	{
		if (m_prototype) m_prototype->add_ref();
	}

	/// \brief
	/// Default destructor for ActionScript objects.
	/// Drops reference on prototype member, if any.
	virtual ~as_object()
	{
		if (m_prototype) m_prototype->drop_ref();
	}
	
	/// Return a text representation for this object
	virtual const char* get_text_value() const { return NULL; }

	/// Return the numeric value of this object
	//
	/// The default implementation converts the text value
	/// to a number, override for a more performant implementation
	///
	virtual double get_numeric_value() const {
		return atof(get_text_value());
	}

	/// Set a member value
	virtual void set_member(const tu_stringi& name,
			const as_value& val );

	/// Get a member as_value by name
	//
	/// This is the one to be overridden if you need special
	/// handling of some values.
	///
	virtual bool get_member(const tu_stringi& name, as_value* val);
	

	/// Set member flags (probably used by ASSetPropFlags)
	//
	/// @param setTrue
	///	the set of flags to set
	///
	/// @param setFalse
	///	the set of flags to clear
	///
	/// @return true on success, false on failure
	///	(non-existent or protected member)
	///
	bool set_member_flags(const tu_stringi& name,
			int setTrue, int setFalse=0);

	/// This object is not a movie; no conversion.
	virtual movie*	to_movie() { return NULL; }

	void	clear();

	/// Check whether this object is an instance of the given
	/// as_function constructor
	//
	/// NOTE: built-in classes should NOT be C_FUNCTIONS for this to
	/// work
	///
	bool instanceOf(as_function* ctor);

	/// Set property flags
	//
	/// @param props
	/// @param set_false
	/// @param set_true
	///
	void setPropFlags(as_value& props, int set_false, int set_true);

	/// Copy properties from the given object
	void copyProperties(const as_object& o);

	/// \brief
	/// Enumerate all non-hidden properties pushing
	/// their value to the given as_environment.
	//
	/// The enumeration recurse in prototype.
	/// This implementation will keep track of visited object
	/// to avoid loops in prototype chain. 
	/// NOTE: the MM player just chokes in this case (loop)
	///
	void enumerateProperties(as_environment& env) const;

protected:

	/// Get a member as_value by name
	bool get_member_default(const tu_stringi& name, as_value* val);

	/// Set a member value
	void set_member_default(const tu_stringi& name, const as_value& val);

private:

	/// Set this object's '__proto__' member
	void set_prototype(as_object* proto);

};

} // namespace gnash

#endif // GNASH_AS_OBJECT_H
