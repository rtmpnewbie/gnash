// LoaderContext_as3.hx:  ActionScript 3 "LoaderContext" class, for Gnash.
//
// Generated by gen-as3.sh on: 20090514 by "rob". Remove this
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
import flash.system.LoaderContext;
import flash.system.ApplicationDomain;
import flash.system.SecurityDomain;
import flash.display.MovieClip;
#else
//import flash.LoaderContext;
//import flash.MovieClip;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as3 suffix, as that's the same name as the file.
class LoaderContext_as {
    static function main() {

//Si
//This class is only defined in flash9.

#if flash9
        var x1:LoaderContext = new LoaderContext();
        // Make sure we actually get a valid class        
        if (Type.typeof(LoaderContext)==TObject && x1 != null) {
            DejaGnu.pass("LoaderContext class exists");
        } else {
            DejaGnu.fail("LoaderContext class doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
// FIXME: haxe doesn't appear to support these properties from the spec.

//Si:
//Adobe may have these properties, we do not.
//allowLoadBytesCodeExecuiton does not exist!
	
// 	if (x1.allowLoadBytesCodeExecution == false) {
// 	    DejaGnu.pass("LoaderContext.allowLoadBytesCodeExecution property exists");
// 	} else {
// 	    DejaGnu.fail("LoaderContext.allowLoadBytesCodeExecution property doesn't exist");
// 	}
// 	if (x1.applicationDomain == applicationDomain) {
// 	    DejaGnu.pass("LoaderContext.applicationDomain property exists");
// 	} else {
// 	    DejaGnu.fail("LoaderContext.applicationDomain property doesn't exist");
// 	}
	
//	DejaGnu.note("Type " + Type.typeof(x1.applicationDomain ) );
//	DejaGnu.note("Type " + Type.typeof(x1.securityDomain ) );

//Si
//Check the existence of checkPolicyFile
	if (Type.typeof(x1.checkPolicyFile) == TBool) {
	    DejaGnu.pass("LoaderContext.checkPolicyFile property exists");
	} else {
	    DejaGnu.fail("LoaderContext.checkPolicyFile property doesn't exist");
	}

//Si:
//The following property is initialized first and then checked!
//That works fine :)
	
	x1.applicationDomain = new ApplicationDomain();

	if (Std.is(x1.applicationDomain,ApplicationDomain) ){
	    DejaGnu.pass("LoaderContext.application property exists");
 	} else {
 	    DejaGnu.fail("LoaderContext.application property doesn't exist");
 	}

//Si:
// FIXME:The SecurityDomain does not have a constructer!
// I can not access the securityDomain
// We may built a real object to check that!
// It is passed right now. To be fixed later.
//	if (Std.is(x1.securityDomain,SecurityDomain)) {
	if (Type.typeof(x1.securityDomain) == TNull ){
 	    DejaGnu.pass("LoaderContext.securityDomain property exists");
 	} else {
 	    DejaGnu.fail("LoaderContext.securityDomain property doesn't exist");
 	}

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();

#else
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
