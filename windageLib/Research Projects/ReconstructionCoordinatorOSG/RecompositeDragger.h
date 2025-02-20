/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/
//osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#ifndef OSGMANIPULATOR_RECOMPOSITER_DRAGGER
#define OSGMANIPULATOR_RECOMPOSITER_DRAGGER 1

#include <osgManipulator/TranslateAxisDragger>
#include <osgManipulator/TrackballDragger>
#include "ScaleUniformDragger.h"

namespace osgManipulator {

/**
 * Dragger for performing scaling on all 3 axes.
 */
class RecompositeDragger : public CompositeDragger
{
    public:
        RecompositeDragger();

        /** Setup default geometry for dragger. */
        void setupDefaultGeometry();

    protected:

        virtual ~RecompositeDragger();

        osg::ref_ptr< TranslateAxisDragger >	tDragger;
        osg::ref_ptr< TrackballDragger >		rDragger;
        osg::ref_ptr< ScaleUniformDragger >		sDragger;
};


}

#endif
