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

import { _CVD, mainScreenScale } from 'quark';
import { Page } from './tool';
import { Navbar } from 'quark/nav';
import {toolbar} from './review';
import { ClickEvent } from 'quark/event';

const px = 1 / mainScreenScale();
const resolve = require.resolve;

export default (self: Page)=>{
	if (!self.isMounted) {
		self.title = 'Nav';
		self.source = resolve(__filename);
		self.backgroundColor = '#333';
		self.navbar = (
			<Navbar backgroundColor="#333" backTextColor="#fff" titleTextColor="#fff">
				<matrix align="rightMiddle" x={-10}>
					<button textFamily="icomoon-ultimate" textColor="#fff" textSize={20} value="\ued63" />
				</matrix>
			</Navbar>
		)
	}

	function hide_show_navbar(e: ClickEvent) {
		self.navbarHidden = !self.navbarHidden;
	}

	function nav_pop(e: ClickEvent) {
		self.collection.pop(true);
	}

	return (
		<free width="match">
			<button class="long_btn2" onClick={hide_show_navbar} value="Hide/Show Navbar" />
			<button class="long_btn2" onClick={nav_pop} value="Nav pop" />

			{toolbar(self)}
		</free>
	)
}