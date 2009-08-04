#include "rt3d.h"

using namespace std;

//do something about this global variable shit, not cool. These all
//exist to help recursive function 'check_point'
Line l;
Point p;
int dumbcount = 0;
int pdumbcount = 0;
int linecount = 0;
int pointcount = 0;

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

    par->SetHeight(1);
    par->SetRadius(4);
    par->SetMaximumU(0.1);
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
            
//             vtkSmartPointer<vtkSmoothPolyDataFilter> res
//                 = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
//             res->SetInput(vtk_polydata);
//             res->SetNumberOfIterations(100);
//             res->BoundarySmoothingOn();

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

    //We don't need my line objects anymore. BALEETED!
    rays.clear();

    for (int i = 0; i < 7; i++) {
        dmap.push_back(vtkSmartPointer<vtkDataSetMapper>::New());
        dmap[i]->SetInputConnection(pdata[i]->GetOutputPort());
    }

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

    //BALEETED!
    pdata.clear();
    dmap.clear();
    vact.clear();

    return 0;
}
