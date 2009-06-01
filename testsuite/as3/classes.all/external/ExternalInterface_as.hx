// ExternalInterface_as.hx:  ActionScript 3 "ExternalInterface" class, for Gnash.
//
// Generated by gen-as3.sh on: 20090515 by "rob". Remove this
// after any hand editing loosing changes.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// This test case must be processed by CPP before compiling to include the
//  DejaGnu.hx header file for the testing framework support.

#if flash9
import flash.external.ExternalInterface;
import flash.display.MovieClip;
#else
import flash.external.ExternalInterface;
import flash.MovieClip;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class ExternalInterface_as {
    static function main() {
	
        // Make sure we actually get a valid class        
        if (flash.external.ExternalInterface != null) {
            DejaGnu.pass("ExternalInterface class exists");
        } else {
            DejaGnu.fail("ExternalInterface class doesn't exist");
        }

// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	
	//check objectID and marshallExceptions only for flashv9
	#if flash9
	if (Type.typeof(flash.external.ExternalInterface.marshallExceptions) == ValueType.TBool) {
	    DejaGnu.pass("ExternalInterface.marshallExceptions property exists");
	} else {
	    DejaGnu.fail("EI.marshallExceptions property doesn't exist");
	}
	
	//this should be changed to check for a proper string returned
	if (Std.is(flash.external.ExternalInterface.objectID, String)) {
	    DejaGnu.pass("ExternalInterface.objectID property exists");
	} else {
	    DejaGnu.fail("ExternalInterface.objectID property doesn't exist");
	}
	#end

	//change this to just false if ExternalInterfacing is not supported
	if (Type.typeof(flash.external.ExternalInterface.available) == ValueType.TBool) {
	    DejaGnu.pass("ExternalInterface.available property exists");
	} else {
	    DejaGnu.fail("ExternalInterface.available property doesn't exist");
	}

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (Type.typeof(flash.external.ExternalInterface.addCallback) == ValueType.TFunction) {
	    DejaGnu.pass("ExternalInterface::addCallback() method exists");
	} else {
	    DejaGnu.fail("ExternalInterface::addCallback() method doesn't exist");
	}
	
	//tests if the EI call function is available
	if (Type.typeof(flash.external.ExternalInterface.call) == ValueType.TFunction) {
	    DejaGnu.pass("ExternalInterface::call() method exists");
	} else {
	    DejaGnu.fail("ExternalInterface::call() method doesn't exist");
	}

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

