// SQLStatement_as.hx:  ActionScript 3 "SQLStatement" class, for Gnash.
//
// Generated by gen-as3.sh on: 20090514 by "rob". Remove this
// after any hand editing loosing changes.
//
//   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
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
import flash.data.SQLStatement;
import flash.display.MovieClip;
#else
import flash.SQLStatement;
import flash.MovieClip;
#end
import flash.Lib;
import Type;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class SQLStatement_as {
    static function main() {
        var x1:SQLStatement = new SQLStatement();

        // Make sure we actually get a valid class        
        if (x1 != null) {
            DejaGnu.pass("SQLStatement class exists");
        } else {
            DejaGnu.fail("SQLStatement lass doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (x1.executing == false) {
	    DejaGnu.pass("SQLStatement::executing property exists");
	} else {
	    DejaGnu.fail("SQLStatement::executing property doesn't exist");
	}
	if (x1.itemClass == Class) {
	    DejaGnu.pass("SQLStatement::itemClass property exists");
	} else {
	    DejaGnu.fail("SQLStatement::itemClass property doesn't exist");
	}
	if (x1.parameters == Object) {
	    DejaGnu.pass("SQLStatement::parameters property exists");
	} else {
	    DejaGnu.fail("SQLStatement::parameters property doesn't exist");
	}
	if (x1.sqlConnection == SQLConnection) {
	    DejaGnu.pass("SQLStatement::sqlConnection property exists");
	} else {
	    DejaGnu.fail("SQLStatement::sqlConnection property doesn't exist");
	}
	if (x1.text == null) {
	    DejaGnu.pass("SQLStatement::text property exists");
	} else {
	    DejaGnu.fail("SQLStatement::text property doesn't exist");
	}

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (x1.SQLStatement == SQLStatement) {
	    DejaGnu.pass("SQLStatement::SQLStatement() method exists");
	} else {
	    DejaGnu.fail("SQLStatement::SQLStatement() method doesn't exist");
	}
	if (x1.cancel == null) {
	    DejaGnu.pass("SQLStatement::cancel() method exists");
	} else {
	    DejaGnu.fail("SQLStatement::cancel() method doesn't exist");
	}
	if (x1.clearParameters == null) {
	    DejaGnu.pass("SQLStatement::clearParameters() method exists");
	} else {
	    DejaGnu.fail("SQLStatement::clearParameters() method doesn't exist");
	}
	if (x1.execute == null) {
	    DejaGnu.pass("SQLStatement::execute() method exists");
	} else {
	    DejaGnu.fail("SQLStatement::execute() method doesn't exist");
	}
	if (x1.getResult == SQLResult) {
	    DejaGnu.pass("SQLStatement::getResult() method exists");
	} else {
	    DejaGnu.fail("SQLStatement::getResult() method doesn't exist");
	}
	if (x1.next == null) {
	    DejaGnu.pass("SQLStatement::next() method exists");
	} else {
	    DejaGnu.fail("SQLStatement::next() method doesn't exist");
	}

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

