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

using namespace std;

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
    vector<Point> points;
};

//Helper method to check point validity.
void check_point(Glib::ustring point, const xmlpp::Node* node)
{
    boost::regex re("-?([[:digit:]]*)\\.([[:digit:]]*)|-?([[:digit:]]*)|-?\\.([[:digit:]]*)|-?([[:digit:]]*)\\.");
    boost::smatch wat;
    if (!boost::regex_match(point.raw(), wat, re)) {
        cout << "ERROR: BAD POINT ON LINE: " << node->get_line() << endl;
        throw 1;
    }
}

//do something about this global variable shit, not cool.
Line l;
Point p;
int dumbcount = 0;
int pdumbcount = 0;
int linecount = 0;
int pointcount = 0;


void parse_xml(const xmlpp::Node* node, vector<Line>& rays)
{
    const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);

    //find color of line.
    if (node->get_name().substr(1,5) == "lines") {
        switch(node->get_name()[0]) {
        case 'r' :
            l.color = 0;
            break;
        case 'o' :
            l.color = 1;
            break;
        case 'y' :
            l.color = 2;
            break;
        case 'g' :
            l.color = 3;
            break;
        case 'b' :
            l.color = 4;
            break;
        case 'i' :
            l.color = 5;
            break;
        case 'v' :
            l.color = 6;
            break;
        }
    }

    //old way
//     if (node->get_name() == "rlines") {
//         l.color = 0;
//     } else if (node->get_name() == "olines") {
//         l.color = 1;
//     } else if (node->get_name() == "ylines") {
//         l.color = 2;
//     } else if (node->get_name() == "glines") {
//         l.color = 3;
//     } else if (node->get_name() == "blines") {
//         l.color = 4;
//     } else if (node->get_name() == "ilines") {
//         l.color = 5;
//     } else if (node->get_name() == "vlines") {
//         l.color = 6;
//     } 

    if (node->get_name().substr(0, 4) == "line" && node->get_name() != "lines") {
        rays.push_back(l);
        if (linecount == 0 && dumbcount == 0) {
            dumbcount = 1;
        } else {
            linecount++;
        }
    }

    if (node->get_name().substr(0, 5) == "point") {
        if (pointcount == 0 && pdumbcount ==0) {
            pdumbcount = 1;
        } else {
            pointcount++;
        }
    }

    if (node->get_name() == "x" || node->get_name() == "y" || node->get_name() =="z") {
        xmlpp::Node::NodeList list = node->get_children();
        const xmlpp::TextNode* tnode = dynamic_cast<const xmlpp::TextNode*>(list.front());
        check_point(tnode->get_content(), tnode);
        switch(node->get_name()[0]) {
        case 'x' :
            p.x = atof(tnode->get_content().c_str());
            break;
        case 'y' :
            p.y = atof(tnode->get_content().c_str());
            break;
        case 'z' :
            p.z = atof(tnode->get_content().c_str());
            rays[linecount].points.push_back(p);
            break;
        }
    }

    //the old way
//         if (node->get_name() == "x"){
// //             xmlpp::Node::NodeList list = node->get_children();
// //             const xmlpp::TextNode* tnode = dynamic_cast<const xmlpp::TextNode*>(list.front());
// //             check_point(tnode->get_content(), tnode);
//             p.x = atof(tnode->get_content().c_str());
//         } else if (node->get_name() == "y"){
// //             xmlpp::Node::NodeList list = node->get_children();
// //             const xmlpp::TextNode* tnode = dynamic_cast<const xmlpp::TextNode*>(list.front());
// //             check_point(tnode->get_content(), tnode);
//             p.y = atof(tnode->get_content().c_str());
//         } else if (node->get_name() == "z"){
// //             xmlpp::Node::NodeList list = node->get_children();
// //             const xmlpp::TextNode* tnode = dynamic_cast<const xmlpp::TextNode*>(list.front());
// //             check_point(tnode->get_content(), tnode);
// //             p.z = atof(tnode->get_content().c_str());
//             rays[linecount].points.push_back(p);
//         }


    if(!nodeContent)
        {
            //Recurse through child nodes:
            xmlpp::Node::NodeList list = node->get_children();
            for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
                {
                    //cout << *iter->get_name() << endl;
                    parse_xml(*iter, rays); //recursive
                }
        }
}


int main (int argc, char* argv[])
{
    string filepath;
    vector<Line> rays;
    vector< vtkSmartPointer<vtkAppendPolyData> > pdata;
    vector< vtkSmartPointer<vtkDataSetMapper> > dmap;
    vector< vtkSmartPointer<vtkActor> > vact;

    if(argc > 1 )
        filepath = argv[1]; //Allow the user to specify a different XML file to parse.
    else
        filepath = "test.xml";

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
                    parse_xml(pNode, rays);
                }
        }
    catch(const exception& ex)
        {
            cout << "Exception caught: " << ex.what() << endl;
            return 0;
        }

    //ray loop. (dooo rayyyy egon!)
    //     for(int j=0; j<rays.size(); j++){
    //         cout << "LOOP ON RAY number " << j << endl;
    //         cout << "Ray " << j << " size " << rays[j].points.size() << endl;
    //         cout << "RAY COLOR " << rays[j].color << endl;
    //         for(int k=0; k<rays[j].points.size(); k++)
    //             {
    //                 cout << "ON POINTS " << k << endl;
    //                 cout << rays[j].points[k].x << endl;
    //                 cout << rays[j].points[k].y << endl;
    //                 cout << rays[j].points[k].z << endl;
    //             }
    //     }

    //instantiate a test optic.
    vtkSmartPointer<vtkParametricParaboloid> par
        = vtkSmartPointer<vtkParametricParaboloid>::New();
    vtkSmartPointer<vtkParametricFunctionSource> parSrc 
        = vtkSmartPointer<vtkParametricFunctionSource>::New();
    vtkSmartPointer<vtkPolyDataMapper> parMap 
        = vtkSmartPointer<vtkPolyDataMapper>::New();
    vtkSmartPointer<vtkActor> parAct 
        = vtkSmartPointer<vtkActor>::New();

    par->SetHeight(0.00001);
    par->SetRadius(0.1);
    par->SetMaximumU(0.01);
    parSrc->SetParametricFunction(par);
    parSrc->SetScalarModeToNone();
    
    parMap->SetInputConnection(parSrc->GetOutputPort());
    parMap->SetScalarRange(-1, 1);
    parAct->SetMapper(parMap);
    parAct->GetProperty()->SetOpacity(0.5);
    parAct->GetProperty()->SetColor(1,1,1);
    parAct->RotateY(45);
    parAct->RotateX(60);
    //important for showing backside of parabola.
    parAct->GetProperty()->SetAmbient(0.4);


    for (int i = 0; i < 7; i++) {
        pdata.push_back(vtkSmartPointer<vtkAppendPolyData>::New());
    }

    // vtkSmartPointer<vtkAppendPolyData> rglue
//         = vtkSmartPointer<vtkAppendPolyData>::New();
//     vtkSmartPointer<vtkAppendPolyData> oglue
//         = vtkSmartPointer<vtkAppendPolyData>::New();
//     vtkSmartPointer<vtkAppendPolyData> yglue
//         = vtkSmartPointer<vtkAppendPolyData>::New();
//     vtkSmartPointer<vtkAppendPolyData> gglue
//         = vtkSmartPointer<vtkAppendPolyData>::New();
//     vtkSmartPointer<vtkAppendPolyData> bglue
//         = vtkSmartPointer<vtkAppendPolyData>::New();
//     vtkSmartPointer<vtkAppendPolyData> iglue
//         = vtkSmartPointer<vtkAppendPolyData>::New();
//     vtkSmartPointer<vtkAppendPolyData> vglue
//         = vtkSmartPointer<vtkAppendPolyData>::New();

    //Loop to build a group of lines using the point array.
    for (int i = 0; i < rays.size(); i++) 
        {
            vtkSmartPointer<vtkPoints> vtk_points
                = vtkSmartPointer<vtkPoints>::New();
            vtk_points->SetNumberOfPoints(rays[i].points.size());
            vtkSmartPointer<vtkPolyLine> vtk_polyline
                = vtkSmartPointer<vtkPolyLine>::New();
            vtk_polyline->GetPointIds()->SetNumberOfIds(rays[i].points.size());
            
            for (int n = 0; n < rays[i].points.size(); n++)
                {
                    vtk_points->InsertPoint(n, rays[i].points[n].x, rays[i].points[n].y, rays[i].points[n].z);
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
            
            switch(rays[i].color) 
                {
                case 0 :
                    pdata[0]->AddInput(vtk_polydata);
                    //rglue->AddInput(vtk_polydata);
                    break;
                case 1 :
                    pdata[1]->AddInput(vtk_polydata);
                    //oglue->AddInput(vtk_polydata);
                    break;
                case 2 :
                    pdata[2]->AddInput(vtk_polydata);
                    //yglue->AddInput(vtk_polydata);
                    break;
                case 3 :
                    pdata[3]->AddInput(vtk_polydata);
                    //gglue->AddInput(vtk_polydata);
                    break;
                case 4 :
                    pdata[4]->AddInput(vtk_polydata);
                    //bglue->AddInput(vtk_polydata);
                    break;
                case 5 :
                    pdata[5]->AddInput(vtk_polydata);
                    //iglue->AddInput(vtk_polydata);
                    break;
                case 6 :
                    pdata[6]->AddInput(vtk_polydata);
                    //vglue->AddInput(vtk_polydata);
                    break;
                }
        }

    //We don't need my line objects anymore.
    rays.clear();


    for (int i = 0; i < 7; i++) {
        dmap.push_back(vtkSmartPointer<vtkDataSetMapper>::New());
        dmap[i]->SetInputConnection(pdata[i]->GetOutputPort());
    }

//     vtkSmartPointer<vtkDataSetMapper> rglue_map
//         = vtkSmartPointer<vtkDataSetMapper>::New();
//     rglue_map->SetInputConnection(rglue->GetOutputPort());
//     vtkSmartPointer<vtkDataSetMapper> oglue_map
//         = vtkSmartPointer<vtkDataSetMapper>::New();
//     oglue_map->SetInputConnection(oglue->GetOutputPort());
//     vtkSmartPointer<vtkDataSetMapper> yglue_map
//         = vtkSmartPointer<vtkDataSetMapper>::New();
//     yglue_map->SetInputConnection(yglue->GetOutputPort());
//     vtkSmartPointer<vtkDataSetMapper> gglue_map
//         = vtkSmartPointer<vtkDataSetMapper>::New();
//     gglue_map->SetInputConnection(gglue->GetOutputPort());
//     vtkSmartPointer<vtkDataSetMapper> bglue_map
//         = vtkSmartPointer<vtkDataSetMapper>::New();
//     bglue_map->SetInputConnection(bglue->GetOutputPort());
//     vtkSmartPointer<vtkDataSetMapper> iglue_map
//         = vtkSmartPointer<vtkDataSetMapper>::New();
//     iglue_map->SetInputConnection(iglue->GetOutputPort());
//     vtkSmartPointer<vtkDataSetMapper> vglue_map
//         = vtkSmartPointer<vtkDataSetMapper>::New();
//     vglue_map->SetInputConnection(vglue->GetOutputPort());


    for (int i = 0; i < 7; i++) {
        vact.push_back(vtkSmartPointer<vtkActor>::New());
        vact[i]->SetMapper(dmap[i]);
        vact[i]->GetProperty()->SetOpacity(0.7);
        switch(i) {
        case 0 :
            vact[i]->GetProperty()->SetColor(1, 0, 0);
            break;
        case 1 :
            vact[i]->GetProperty()->SetColor(1, 0.5, 0);
            break;
        case 2 :
            vact[i]->GetProperty()->SetColor(1, 1, 0);
            break;
        case 3 :
            vact[i]->GetProperty()->SetColor(0, 1, 0);
            break;
        case 4 :
            vact[i]->GetProperty()->SetColor(0, 0, 1);
            break;
        case 5 :
            vact[i]->GetProperty()->SetColor(0.3, 0, 0.5);
            break;
        case 6 :
            vact[i]->GetProperty()->SetColor(0.93, 0.5, 0.93);
            break;
        }
    }    

//     vtkSmartPointer<vtkActor> rglue_act
//         = vtkSmartPointer<vtkActor>::New();
//     rglue_act->SetMapper(rglue_map);
//     rglue_act->GetProperty()->SetColor(1, 0, 0);
//     rglue_act->GetProperty()->SetOpacity(0.7);
//     vtkSmartPointer<vtkActor> oglue_act
//         = vtkSmartPointer<vtkActor>::New();
//     oglue_act->SetMapper(oglue_map);
//     oglue_act->GetProperty()->SetColor(1, 0.5, 0);
//     oglue_act->GetProperty()->SetOpacity(0.7);
//     vtkSmartPointer<vtkActor> yglue_act
//         = vtkSmartPointer<vtkActor>::New();
//     yglue_act->SetMapper(yglue_map);
//     yglue_act->GetProperty()->SetColor(1, 1, 0);
//     yglue_act->GetProperty()->SetOpacity(0.7);
//     vtkSmartPointer<vtkActor> gglue_act
//         = vtkSmartPointer<vtkActor>::New();
//     gglue_act->SetMapper(gglue_map);
//     gglue_act->GetProperty()->SetColor(0, 1, 0);
//     gglue_act->GetProperty()->SetOpacity(0.7);
//     vtkSmartPointer<vtkActor> bglue_act
//         = vtkSmartPointer<vtkActor>::New();
//     bglue_act->SetMapper(bglue_map);
//     bglue_act->GetProperty()->SetColor(0, 0, 1);
//     gglue_act->GetProperty()->SetOpacity(0.7);
//     vtkSmartPointer<vtkActor> iglue_act
//         = vtkSmartPointer<vtkActor>::New();
//     iglue_act->SetMapper(iglue_map);
//     iglue_act->GetProperty()->SetColor(0.30, 0, 0.5);
//     iglue_act->GetProperty()->SetOpacity(0.7);
//     vtkSmartPointer<vtkActor> vglue_act
//         = vtkSmartPointer<vtkActor>::New();
//     vglue_act->SetMapper(vglue_map);
//     vglue_act->GetProperty()->SetColor(0.93, 0.5, 0.93);
//     vglue_act->GetProperty()->SetOpacity(0.7);
     
    //red    (1,0,0)
    //orange (1, 0.5, 0)
    //yellow (1, 1, 0)
    //green  (0, 1, 0)
    //blue   (0, 0, 1)
    //indigo (0.30, 0, 0.5);
    //violet (0.93, 0.5, 0.93);

    vtkSmartPointer<vtkRenderer> ren 
        = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renWin 
        = vtkSmartPointer<vtkRenderWindow>::New();
    vtkSmartPointer<vtkRenderWindowInteractor> iren 
        = vtkSmartPointer<vtkRenderWindowInteractor>::New();

    ren->TwoSidedLightingOn();
    ren->LightFollowCameraOn();

    for (int i = 0; i < 7; i++) {
        ren->AddActor(vact[i]);
    }
//     ren->AddActor(rglue_act);
//     ren->AddActor(oglue_act);
//     ren->AddActor(yglue_act);
//     ren->AddActor(gglue_act);
//     ren->AddActor(bglue_act);
//     ren->AddActor(iglue_act);
//     ren->AddActor(vglue_act);
    ren->AddActor(parAct);

    renWin->AddRenderer(ren);
    iren->SetRenderWindow(renWin);

    renWin->SetSize(800, 800);
    renWin->SetWindowName("Longcat");
    
    //renWin->OffScreenRenderingOn();
    
    ren->GetActiveCamera()->SetPosition(1,1,0);
    ren->ResetCamera();
    ren->GetActiveCamera()->Dolly(1.4);
    ren->ResetCameraClippingRange();
    
    vtkSmartPointer<vtkRenderLargeImage> im
        = vtkSmartPointer<vtkRenderLargeImage>::New();
    im->SetInput(ren);
    im->SetMagnification(1);

    vtkSmartPointer<vtkPostScriptWriter> ps_write
        = vtkSmartPointer<vtkPostScriptWriter>::New();
    ps_write->SetInputConnection(im->GetOutputPort());
    ps_write->SetFileName("zomg.ps");
    ps_write->Write();

    vtkSmartPointer<vtkTIFFWriter> tif
        = vtkSmartPointer<vtkTIFFWriter>::New();
    tif->SetInputConnection(im->GetOutputPort());
    tif->SetFileName("zomg.tiff");
    tif->Write();
    
    iren->Initialize();
    ren->Render();
    iren->Start();
    
    return 0;
}
