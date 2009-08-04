#ifndef __RT3D_H
#define __RT3D_H

//general includes
#include <boost/regex.hpp>
#include <iostream>
#include <libxml++/libxml++.h>
#include <sstream>
#include <vector>
#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkMath.h"
#include "vtkPostScriptWriter.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderLargeImage.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkTIFFWriter.h"

//parametric includes
#include "vtkParametricParaboloid.h"
#include "vtkParametricFunction.h"
#include "vtkParametricFunctionSource.h"
#include "vtkPolyDataMapper.h"

//line includes
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyLine.h"
#include "vtkAppendPolyData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkDataSetMapper.h"

//For making ints to strings!
//what the fuck am I using this for?
//maybe saving multiple images?
template <class T>
inline std::string to_string (const T& t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

struct Point
{
    double x;
    double y;
    double z;
};

struct Line
{
    //code 0-6 to be colors. r0 o1 y2 g3 b4 i5 v6
    int color;
    std::vector<Point> points;
};

#endif
