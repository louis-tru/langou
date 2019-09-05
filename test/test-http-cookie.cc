/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "lutils/http-cookie.h"
#include "langou/sys.h"

using namespace langou;

void test_http_cookie(int argc, char **argv) {
	
	LOG(http_cookie_get("langou.org", "test"));
	
	http_cookie_set("langou.org", "test", "langou.org");
	
	LOG("A, %s", *http_cookie_get("langou.org", "test"));
	
	LOG("B, %s", *http_cookie_get("www.langou.org", "test"));

	http_cookie_set("www.langou.org", "test", "$");

	LOG("B2, %s", *http_cookie_get("www.langou.org", "test"));

	http_cookie_set("langou.org", "test2", "*");
	
	LOG("D, %s", *http_cookie_get("langou.org", "test2"));
	
	LOG("E, %s", *http_cookie_get("www.langou.org", "test2"));
	
	http_cookie_set("langou.org", "test2", "-----------------------------", -1, "/AA");
	
	LOG("F, %s", *http_cookie_get("langou.org", "test2"));
	
	LOG("H, %s", *http_cookie_get("langou.org", "test2", "/AA"));
	
	LOG(http_cookie_get_all_string("www.langou.org", "/AA"));
	
	http_cookie_set_with_expression("langou.org", "test3=HHHH--l; path=/AA; max-age=60");
	
	LOG(http_cookie_get("langou.org", "test3"));
	
	LOG(http_cookie_get("langou.org", "test3", "/AA"));
	
	LOG("http_cookie_get_all_string 1, %s", *http_cookie_get_all_string("www.langou.org", "/AA"));
	LOG("http_cookie_get_all_string 2, %s", *http_cookie_get_all_string("langou.org", "/AA"));
	
	// test delete
	
	http_cookie_delete("langou.org", "test");
	
	LOG(http_cookie_get("langou.org", "test"));
	
	http_cookie_set("langou.org", "test", "langou.org2");
	http_cookie_set("langou.org", "test9", "langou.org3");
	http_cookie_set("langou.org", "test8", "langou.org4");
	http_cookie_set("www.langou.org", "test7", "langou.org5");
	
	LOG("E, %s", *http_cookie_get("langou.org", "test"));

	http_cookie_set("langou.orh", "test--------A", "langou.org%", -1, "KKK/MMM");

	LOG("http_cookie_get_all_string 3, %s", *http_cookie_get_all_string("langou.org"));
	
	http_cookie_delete_all("langou.org");
	
	LOG(http_cookie_get("langou.org", "test"));
	
	http_cookie_set("langou.org", "test", "langou.org");
	
	LOG("F, %s", *http_cookie_get("langou.org", "test--------A", "KKK/MMM", 1));
	
	http_cookie_clear();
	
	LOG(http_cookie_get("langou.org", "test"));
	
	http_cookie_set("langou.org", "test", "END test cookie", sys::time() + 6e7); // 60s expires
	
	LOG(http_cookie_get("langou.org", "test"));
	
}
