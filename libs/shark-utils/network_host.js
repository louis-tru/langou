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

var os = require('os');

function getLocalNetworkHost() {
	var r = [];
	Object.entries(ip()).forEach(([k,v])=>r.push(...v));
	if ( r.length == 0 ) 
		r.push('127.0.0.1');
	return r;
}

function ip() {
	/*
	{ 
	  lo0: 
	 [ { address: '::1', family: 'IPv6', internal: true },
	   { address: '127.0.0.1', family: 'IPv4', internal: true },
	   { address: 'fe80::1', family: 'IPv6', internal: true },
	 ],
	en0: 
	 [ { address: 'fe80::89e:2356:5967:a5c8', family: 'IPv6', internal: false },
	   { address: '192.168.43.132', family: 'IPv4', internal: false },
	 ],
	awdl0: 
	 [ 
	   { address: 'fe80::d9:21ff:fe77:da88', family: 'IPv6', internal: false },
	 ],
	}*/
	
	var ifaces = os.networkInterfaces();
	var address = {}; // '127.0.0.1';
	
	for (var i in ifaces) {
		for (var j of ifaces[i]) {
			if (j.family == 'IPv4' && !j.internal && j.address != '127.0.0.1') {
				var addr = j.address;
				if (!address[i]) {
					address[i] = [];
				}
				address[i].push(addr);
			}
		}
	}
	return address;
}

exports.ip = ip;
exports.getLocalNetworkHost = getLocalNetworkHost;
