//general includes
#include <vector>
#include <iostream>
#include <libxml++/libxml++.h>
#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkMath.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"

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

using namespace std;

//The point object.
struct Point
{
    double x;
    double y;
    double z;
};

struct Line
{
    int color;
    
};

void print_node(const xmlpp::Node* node)
{
    const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
    cout << node << endl;

    if (node->get_name() == "all")
        {
            cout << node->get_name() << endl;
        }

    if(nodeText)
        {
            cout << "text = \"" << nodeText->get_content() << "\"" << endl;
        }
}

int main (int argc, char* argv[])
{
    string filepath;

    if(argc > 1 )
        filepath = argv[1]; //Allow the user to specify a different XML file to parse.
    else
        filepath = "example.xml";

    try
        {
            xmlpp::DomParser parser;
            parser.set_validate();
            parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
            parser.parse_file(filepath);
            if(parser)
                {
                    //Walk the tree:
                    const xmlpp::Node* pNode = parser.get_document()->get_root_node(); //deleted by DomParser.
                    //std::cout << "zomg?!";
                    print_node(pNode);
                }
        }
    catch(const exception& ex)
        {
            cout << "Exception caught: " << ex.what() << endl;
            return 0;
        }




    vtkSmartPointer<vtkParametricParaboloid> par
        = vtkSmartPointer<vtkParametricParaboloid>::New();
    vtkSmartPointer<vtkParametricFunctionSource> parSrc 
        = vtkSmartPointer<vtkParametricFunctionSource>::New();
    vtkSmartPointer<vtkPolyDataMapper> parMap 
        = vtkSmartPointer<vtkPolyDataMapper>::New();
    vtkSmartPointer<vtkActor> parAct 
        = vtkSmartPointer<vtkActor>::New();


    //par->SetHeight(0.001);
    //par->SetRadius(0.8);
    //par->SetMinimumU(1);
    parSrc->SetParametricFunction(par);
    parSrc->SetScalarModeToNone();
    parMap->SetInputConnection(parSrc->GetOutputPort());
    parMap->SetScalarRange(-1, 1);
    parAct->SetMapper(parMap);
    parAct->GetProperty()->SetOpacity(0.5);
    parAct->GetProperty()->SetColor(1,1,1);
    parAct->GetProperty()->SetDiffuseColor(1,1,1);
    
    //2d array of point objects. Each row is a line.
    Point blah[2][2];
    
    blah[0][0].x = 0;
    blah[0][0].y = 0;
    blah[0][0].z = 0;
    blah[0][1].x = 5;
    blah[0][1].y = 5;
    blah[0][1].z = 5;
    blah[1][0].x = -1;
    blah[1][0].y = -1;
    blah[1][0].z = -1;
    blah[1][1].x = 0;
    blah[1][1].y = -5;
    blah[1][1].z = -5;
    
    //lame c++ caveat... get size of an array is bytesize of array
    //divided by bytesize of array object... fail
    int num_points = sizeof(blah[0]) / sizeof(Point);

    cout << "\n\n\n ohai  " << blah[0][0].x << endl;
    cout << "\n " << blah[0][0].x << endl;
    //cout.flush();

    int i;
    int n;

    vtkSmartPointer<vtkAppendPolyData> glue
        = vtkSmartPointer<vtkAppendPolyData>::New();


    //Loop to build a group of lines using the point array.
    for (i=0; i<2; i++) 
        {
            vtkSmartPointer<vtkPoints> vtk_points
                = vtkSmartPointer<vtkPoints>::New();
            vtk_points->SetNumberOfPoints(num_points);
            vtkSmartPointer<vtkPolyLine> vtk_polyline
                = vtkSmartPointer<vtkPolyLine>::New();
            vtk_polyline->GetPointIds()->SetNumberOfIds(num_points);
            
            for (n=0; n<num_points; n++)
                {
                    vtk_points->InsertPoint(n, blah[i][n].x, blah[i][n].y, blah[i][n].z);
                    vtk_polyline->GetPointIds()->SetId(n, n);
                }
            
            vtkSmartPointer<vtkUnstructuredGrid> vtk_grid
                = vtkSmartPointer<vtkUnstructuredGrid>::New();
            vtk_grid->Allocate(1, 1);
            vtk_grid->InsertNextCell(vtk_polyline->GetCellType(),
                                     vtk_polyline->GetPointIds());
            vtk_grid->SetPoints(vtk_points);
            
            vtkSmartPointer<vtkPolyData> vtk_polydata
                = vtkSmartPointer<vtkPolyData>::New();
            vtk_polydata->SetPoints(vtk_points);
            vtk_polydata->SetLines(vtk_grid->GetCells());
            
            glue->AddInput(vtk_polydata);
        }
    
            
    vtkSmartPointer<vtkDataSetMapper> glue_map
        = vtkSmartPointer<vtkDataSetMapper>::New();
    glue_map->SetInputConnection(glue->GetOutputPort());

    vtkSmartPointer<vtkActor> glue_act
        = vtkSmartPointer<vtkActor>::New();
    glue_act->SetMapper(glue_map);
    glue_act->GetProperty()->SetColor(1, 0, 1);
    

    vtkSmartPointer<vtkRenderer> ren 
        = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renWin 
        = vtkSmartPointer<vtkRenderWindow>::New();
    vtkSmartPointer<vtkRenderWindowInteractor> iren 
        = vtkSmartPointer<vtkRenderWindowInteractor>::New();

    renWin->AddRenderer(ren);
    iren->SetRenderWindow(renWin);

    ren->AddActor(glue_act);
    ren->AddActor(parAct);

    renWin->SetSize(800, 800);
    renWin->SetWindowName("Longcat");
    
    iren->Initialize();
    ren->Render();
    iren->Start();

    return 0;
}
