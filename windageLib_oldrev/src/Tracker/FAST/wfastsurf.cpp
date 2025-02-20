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

#include "Tracker/FAST/wfastsurf.h"
//#define USE_GAUSSIAN_WEIGHT

const int dx1[] = {3, 3, 2, 1, 0, -1, -2, -3};
const int dx2[] = {-3, -3, -2, -1, 0, 1, 2, 3};
const int dy1[] = {0, 1, 2, 3, 3, 3, 2, 1};
const int dy2[] = {0, -1, -2, -3, -3, -3, -2, -1};

// modified FAST SURF descriptor
void wExtractFASTSURF( const CvArr* _img, const CvArr* _mask,
							CvSeq** _keypoints, CvSeq** _descriptors,
							CvMemStorage* storage, CvSURFParams params,
							int useProvidedKeyPts)
{
	const int PATCH_SZ = 15;
	CvSeq *keypoints = *_keypoints;
    int N = keypoints->total;
	IplImage* image = ((IplImage*)_img);

	// create descriptor storage
	int descriptor_size = 36;
	const int descriptor_data_type = CV_32F;
	CvSeq* descriptors = 0;

	if( _descriptors ) *_descriptors = 0;
	if( _descriptors )
    {
        descriptors = cvCreateSeq( 0, sizeof(CvSeq),
            descriptor_size*CV_ELEM_SIZE(descriptor_data_type), storage );
        cvSeqPushMulti( descriptors, 0, N );
    }

#ifdef USE_GAUSSIAN_WEIGHT
	const float DESC_SIGMA = 3.3f;
	float DW[PATCH_SZ][PATCH_SZ];
    CvMat _DW = cvMat(PATCH_SZ, PATCH_SZ, CV_32F, DW);
	// Gaussian used to weight descriptor samples
    {
    double c2 = 1./(DESC_SIGMA*DESC_SIGMA*2);
    double gs = 0;
    for(int i = 0; i < PATCH_SZ; i++)
    {
        for(int j = 0; j < PATCH_SZ; j++)
        {
            double x = j - (float)(PATCH_SZ-1)/2, y = i - (float)(PATCH_SZ-1)/2;
            double val = exp(-(x*x+y*y)*c2);
            DW[i][j] = (float)val;
            gs += val;
        }
    }
    cvScale( &_DW, &_DW, 1./gs );
    }
#endif

	#pragma omp parallel for schedule(dynamic)
    for(int k = 0; k < N; k++ )
    {
		int i, j;
		CvSURFPoint* kp = (CvSURFPoint*)cvGetSeqElem( keypoints, k );
		int x = cvRound(kp->pt.x);
		int y = cvRound(kp->pt.y);

		// calculate rotation
		int dx = 0;
		int dy = 0;
		for(int i=0; i<8; i++)
		{
			int difference = (int)((unsigned char)image->imageData[(y + dy1[i]) * image->widthStep + (x + dx1[i])] - (unsigned char)image->imageData[(y + dy2[i]) * image->widthStep + (x + dx2[i])]);
			dx += dx1[i] * difference;
			dy += dy1[i] * difference;
		}

        float descriptor_dir = cvFastArctan( (float)dy, (float)dx );
        kp->dir = descriptor_dir;
        descriptor_dir *= (float)(CV_PI/180);

		// extract descriptors
		float sin_dir = sin(-descriptor_dir);
        float cos_dir = cos(-descriptor_dir) ;

		int PATCH[PATCH_SZ+1][PATCH_SZ+1];
		CvMat _patch = cvMat(PATCH_SZ+1, PATCH_SZ+1, CV_8U, PATCH);

        // Nearest neighbour version (faster)
        float win_offset = -(float)(PATCH_SZ-1)/2;
        float start_x = kp->pt.x + win_offset*cos_dir + win_offset*sin_dir;
        float start_y = kp->pt.y - win_offset*sin_dir + win_offset*cos_dir;
        for( i=0; i<PATCH_SZ+1; i++, start_x+=sin_dir, start_y+=cos_dir )
        {
            float pixel_x = start_x;
            float pixel_y = start_y;
            for( j=0; j<PATCH_SZ+1; j++, pixel_x+=cos_dir, pixel_y-=sin_dir )
            {
                x = cvRound( pixel_x );
                y = cvRound( pixel_y );
                x = MAX( x, 0 );
                y = MAX( y, 0 );
                x = MIN( x, image->width-1 );
                y = MIN( y, image->height-1 );
				PATCH[i][j] = (int)(unsigned char)image->imageData[y*image->widthStep + x];
             }
        }

        // Calculate gradients in x and y with wavelets of size 2s
		float DX[PATCH_SZ][PATCH_SZ];
		float DY[PATCH_SZ][PATCH_SZ];
		float dw = 1.0f;
        for( i = 0; i < PATCH_SZ; i++ )
		{
            for( j = 0; j < PATCH_SZ; j++ )
            {
#ifdef USE_GAUSSIAN_WEIGHT
				dw = DW[i][j];
#endif
				DX[i][j] = (float)(PATCH[i][j+1] - PATCH[i][j] + PATCH[i+1][j+1] - PATCH[i+1][j]) * dw;
				DY[i][j] = (float)(PATCH[i+1][j] - PATCH[i][j] + PATCH[i+1][j+1] - PATCH[i][j+1]) * dw;
            }
		}

        // Construct the descriptor
		float* vec = (float*)cvGetSeqElem( descriptors, k );
        for(i=0; i<descriptor_size; i++)
            vec[i] = 0;

		// always 36-bin descriptor
        for(i = 0; i < 3; i++)
		{
            for(j = 0; j < 3; j++)
            {
                for(y = i*5; y < i*5+5; y++)
                {
                    for(x = j*5; x < j*5+5; x++)
                    {
                        vec[0] += DX[y][x];
						vec[1] += DY[y][x];
                        vec[2] += fabs(DX[y][x]);
						vec[3] += fabs(DY[y][x]);
                    }
                }
                vec+=4;
			}
		}
    }

	if( _descriptors )	*_descriptors = descriptors;
	if( _keypoints)		*_keypoints = keypoints;
}
