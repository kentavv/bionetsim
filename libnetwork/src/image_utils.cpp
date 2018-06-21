// This file is part of GenoDYN.
//
// GenoDYN is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// GenoDYN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with GenoDYN.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2008 Kent A. Vander Velden

// Copyright (C) 2008 Kent A. Vander Velden

#include <qimage.h>

#include "image_utils.h"

void
cropImageWhiteSpace(QImage &img, const int margin) {
	img.convertDepth(32);

	const int w = img.width();
	const int h = img.height();
	const unsigned int *p = (const unsigned int*)(img.bits());

	int top = -1;
	for(int i=0; i<h; i++) {
		for(int j=0; j<w; j++) {
			if((p[i*w + j] & 0x00ffffff) != 0x00ffffff) {
				top = i;
				break;
			}
		}
		if(top != -1) break;
	}

	if(top == -1) return;

	int bottom = -1;
	for(int i=h-1; i>top; i--) {
		for(int j=0; j<w; j++) {
			if((p[i*w + j] & 0x00ffffff) != 0x00ffffff) {
				bottom = i;
				break;
			}
		}
		if(bottom != -1) break;
	}

	if(bottom == -1) return;

	int left=w, right=0;
	for(int i=top; i<=bottom; i++) {
		for(int j=left; j>=0; j--) {
			if((p[i*w + j] & 0x00ffffff) != 0x00ffffff) {
				left = j;
			}
		}
		for(int j=right; j<w; j++) {
			if((p[i*w + j] & 0x00ffffff) != 0x00ffffff) {
				right = j;
			}
		}
	}

	top    -= margin; if(top < 0) top = 0;
	bottom += margin; if(bottom >= h) bottom = h-1;
	left   -= margin; if(left < 0) left = 0;
	right  += margin; if(right >= w) right = w-1;

	if(left >= right) return;

	img = img.copy(left, top, right-left+1, bottom-top+1);
}
