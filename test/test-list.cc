/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include <quark/util/array.h>
#include <quark/util/list.h>
#include <quark/util/string.h>
#include <map>
#include <quark/util/log.h>

using namespace quark;

class Test{
	String str;
};

void test_list(int argc, char **argv) {
	
	String str = "a,b,c,d,s,d,f,g,gh,t,r,w,cs,dsfsd,fsdf,dsf,sdf,sd";
	
	Array<String> ls = str.split(",");
	
	Qk_LOG("%d", ls.length());
	
	ls.push("ABCD");
	ls.push("ABCD1");
	ls.pop();
	
	Qk_LOG("%d", ls.length());
	
	Qk_LOG(ls[0]);
	
	Qk_LOG(ls.join("-"));
	
	String str2(str);
	
	Qk_LOG(str2.to_string());
	Qk_LOG(str2.copy().collapse());
	
	for (auto i = ls.begin(), end = ls.end(); i != end; i++) {
		Qk_LOG(*i);
	}
	
	Array<int> ls2;
	
	ls2.push(10);
	ls2.push(11);
	ls2.push(12);
	ls2.pop();
	
}
