PROJECT (test)

FIND_PACKAGE(VTK  REQUIRED)
INCLUDE(${VTK_USE_FILE})

INCLUDE_DIRECTORIES("/home/tron/devel/vtkMy", "/home/tron/devel/vtkMy/bin", "/home/tron/devel/vtkMy/Common", "/usr/include/libxml++-2.6", "/usr/lib/libxml++-2.6/include", "/usr/include/glibmm-2.4", "/usr/include/glib-2.0", "/usr/lib/glib-2.0/include", "/usr/lib/glibmm-2.4/include")

LINK_DIRECTORIES("/home/tron/devel/vtkMy/bin")

ADD_EXECUTABLE(test test.cxx)
TARGET_LINK_LIBRARIES(test
    vtkCommon vtkGraphics
    vtkHybrid vtkIO vtkRendering
    vtkmyCommon xml++-2.6)
