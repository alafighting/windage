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

#ifndef _AUGMENTED_REALITY_H_
#define _AUGMENTED_REALITY_H_

#include "base.h"
#include "Tracker/Calibration.h"

namespace windage
{
	/**
	 * @brief
	 *		Abstract Class for Augmented Reality
	 * @author
	 *		windage
	 */
	class DLLEXPORT AugmentedReality
	{
	protected:
		Calibration* cameraParameter;	///< camera parameter only read (not-update)

		bool isFlip;
		int imageWidth;		///< input background image width size
		int imageHeight;	///< input background image height size
		int textureWidth;	///< drawing background texture image size (width == height) (*necessary multiple of 4)

		IplImage* textureRepository;	///< background texture storage
		
		virtual void Release() = 0;

	public:
		AugmentedReality();
		virtual ~AugmentedReality();

		/**
		 * @brief
		 *		Attatch Camera Parameter
		 * @remark
		 *		attatch camera paramter only read and update from outside
		 */
		inline void AttatchCameraParameter(Calibration* cameraParameter){this->cameraParameter = cameraParameter;};

		/**
		 * @defgroup AugmentedRealityMethod Augmented Reality Method
		 * @brief
		 *		Augmented Reality Method
		 * @remark
		 *		DrawBackground -> SetProjectionMatrix and ModelViewMatrix -> Draw Virtual Object
		 * @addtogroup AugmentedRealityMethod
		 * @{
		 */

		/**
		 * @brief
		 *		Draw Background Texture
		 * @remark
		 *		draw background image abstract method using input image
		 */
		virtual void DrawBackgroundTexture(IplImage* inputImage) = 0;
		
		/**
		 * @brief
		 *		Set Projection Matrix
		 * @remark
		 *		set projection matrix abstract method using intrinsic matrix
		 */
		virtual void SetProjectionMatrix() = 0;

		/**
		 * @brief
		 *		Set ModelView Matrix
		 * @remark
		 *		set model-view matrix abstract method using extrinsic matrix
		 */
		virtual void SetModelViewMatrix() = 0;
		/** @} */
	};
}

#endif