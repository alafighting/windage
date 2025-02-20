/* ========================================================================
 * PROJECT: windage Library
 * ========================================================================
 * This work is based on the original windage Library developed by
 *   Woonhyuk Baek
 *   Woontack Woo
 *   U-VR Lab, GIST of Gwangju in Korea.
 *   http://windage.googlecode.com/
 *   http://uvr.gist.ac.kr/
 *
 * Copyright of the derived and new portions of this work
 *     (C) 2009 GIST U-VR Lab.
 *
 * This framework is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this framework; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * For further information please contact 
 *   Woonhyuk Baek
 *   <windage@live.com>
 *   GIST U-VR Lab.
 *   Department of Information and Communication
 *   Gwangju Institute of Science and Technology
 *   1, Oryong-dong, Buk-gu, Gwangju
 *   South Korea
 * ========================================================================
 ** @author   Woonhyuk Baek
 * ======================================================================== */

#ifndef _FIND_PROSAC_HOMOGRAPY_H_
#define _FIND_PROSAC_HOMOGRAPY_H_

#include <vector>
#include <cv.h>

#include "base.h"
#include "FindHomography.h"

namespace windage
{
	typedef struct _CompareDistanceLess
	{
		bool operator()(const struct _MatchedPoint& p, const struct _MatchedPoint& q) const
		{
			return p.distance < q.distance;
		}
	}CompareDistanceLess;

	class DLLEXPORT FindPROSACHomography:public FindHomography
	{
	private:
		float confidence;
		int maxIteration;
		double timeout;

	public:
		FindPROSACHomography()
		{
			this->confidence = 0.7f; // constant
			SetTimeout(5.0);

			this->maxIteration = 2000;
			this->reprojectionThreshold = 5.0f;
			for(int i=0; i<9; i++)
				homography[i] = 0.0f;

			this->matchedPoints = NULL;
		}
		~FindPROSACHomography()
		{

		}

		inline void SetMaxIteration(int iteration=1000){this->maxIteration = iteration;};
		inline void SetTimeout(double ms){this->timeout = ms * 1000.0 * cvGetTickFrequency();};

		bool Calculate();
	};
}

#endif