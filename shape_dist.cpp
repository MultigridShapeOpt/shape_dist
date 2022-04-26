#include <iostream>
#include <vtk/vtkSmartPointer.h>
#include <vtk/vtkBooleanOperationPolyDataFilter.h>

#include <vtk/vtkNew.h>
#include <vtk/vtkXMLPUnstructuredGridReader.h>
#include <vtk/vtkGenericDataObjectReader.h>
#include <vtk/vtkDataSetSurfaceFilter.h>
#include <vtk/vtkGeometryFilter.h>
#include <vtk/vtkXMLPolyDataWriter.h>
#include <vtk/vtkUnstructuredGrid.h>
#include <vtk/vtkClipPolyData.h>
#include <vtk/vtkSphere.h>
#include <vtk/vtkPointData.h>
#include <vtk/vtkFieldData.h>
#include <vtk/vtkPolyDataNormals.h>
#include <vtk/vtkArrayCalculator.h>
#include <vtk/vtkIntegrateAttributes.h>
#include <vtk/vtkPassArrays.h>
#include <vtk/vtkmCleanGrid.h>

int main(int argc, char* argv[]) {

  if (argc < 3) {
    std::cout << "Call the program as: ./executable file1.vtk file2.vtk\n";
    return 0;
  } else {
    std::cout << "Running with: " << argv[1] << " and " << argv[2] << "\n";
  }
  std::string file1(argv[1]);
  std::string file2(argv[2]);

  // assume that the relevant boundaries of the two shapes are within a sphere of
  // radius 2.0 around the origin
  vtkSmartPointer<vtkSphere> sphere_clip = vtkSmartPointer<vtkSphere>::New();
  sphere_clip->SetRadius(2.0);

  vtkSmartPointer<vtkXMLPUnstructuredGridReader> preader1 = vtkSmartPointer<vtkXMLPUnstructuredGridReader>::New();
  vtkSmartPointer<vtkGenericDataObjectReader> reader1 = vtkSmartPointer<vtkGenericDataObjectReader>::New();
  vtkSmartPointer<vtkmCleanGrid> cleaner1 = vtkSmartPointer<vtkmCleanGrid>::New();
  vtkSmartPointer<vtkDataSetSurfaceFilter> geometry1 = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
  if(file1.find(".pvtu") != std::string::npos) {
    preader1->SetFileName(file1.c_str());
    preader1->Update();
    cleaner1->SetInputData(preader1->GetOutput());
    cleaner1->Update();
    geometry1->SetInputData(cleaner1->GetOutput());
  } else {
    reader1->SetFileName(file1.c_str());
    reader1->Update();
    geometry1->SetInputData(reader1->GetUnstructuredGridOutput());
  }
  geometry1->Update();

  vtkSmartPointer<vtkXMLPUnstructuredGridReader> preader2 = vtkSmartPointer<vtkXMLPUnstructuredGridReader>::New();
  vtkSmartPointer<vtkGenericDataObjectReader> reader2 = vtkSmartPointer<vtkGenericDataObjectReader>::New();
  vtkSmartPointer<vtkmCleanGrid> cleaner2 = vtkSmartPointer<vtkmCleanGrid>::New();
  vtkSmartPointer<vtkDataSetSurfaceFilter> geometry2 = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
  if(file2.find(".pvtu") != std::string::npos) {
    preader2->SetFileName(file2.c_str());
    preader2->Update();
    cleaner2->SetInputData(preader2->GetOutput());
    cleaner2->Update();
    geometry2->SetInputData(cleaner2->GetOutput());
  } else {
    reader2->SetFileName(file2.c_str());
    reader2->Update();
    geometry2->SetInputData(reader2->GetUnstructuredGridOutput());
  }
  geometry2->Update();

  vtkSmartPointer<vtkClipPolyData> clip1 = vtkSmartPointer<vtkClipPolyData>::New();
  clip1->SetInputData(geometry1->GetOutput());
  clip1->SetClipFunction(sphere_clip);
  clip1->SetInsideOut(true);
  clip1->Update();

  vtkSmartPointer<vtkClipPolyData> clip2 = vtkSmartPointer<vtkClipPolyData>::New();
  clip2->SetInputData(geometry2->GetOutput());
  clip2->SetClipFunction(sphere_clip);
  clip2->SetInsideOut(true);
  clip2->Update();

  vtkSmartPointer<vtkPolyDataNormals> normals_before1 = vtkSmartPointer<vtkPolyDataNormals>::New();
  normals_before1->SetInputData(clip1->GetOutput());
  normals_before1->AutoOrientNormalsOn();
  normals_before1->SetComputeCellNormals(true);
  normals_before1->SetComputePointNormals(true);
  normals_before1->Update();

  vtkSmartPointer<vtkPolyDataNormals> normals_before2 = vtkSmartPointer<vtkPolyDataNormals>::New();
  normals_before2->SetInputData(clip2->GetOutput());
  normals_before2->AutoOrientNormalsOn();
  normals_before2->SetComputeCellNormals(true);
  normals_before2->SetComputePointNormals(true);
  normals_before2->Update();

  vtkSmartPointer<vtkBooleanOperationPolyDataFilter> boolIntersection =
                            vtkSmartPointer<vtkBooleanOperationPolyDataFilter>::New();
  boolIntersection->SetInputData( 0, normals_before1->GetOutput() );
  boolIntersection->SetInputData( 1, normals_before2->GetOutput() );
  boolIntersection->SetOperationToIntersection();
  boolIntersection->Update();

  vtkSmartPointer<vtkBooleanOperationPolyDataFilter> boolUnion =
                            vtkSmartPointer<vtkBooleanOperationPolyDataFilter>::New();
  boolUnion->SetInputData( 0, normals_before1->GetOutput() );
  boolUnion->SetInputData( 1, normals_before2->GetOutput() );
  boolUnion->SetOperationToUnion();
  boolUnion->Update();

  vtkSmartPointer<vtkPolyDataNormals> normalsIntersection = vtkSmartPointer<vtkPolyDataNormals>::New();
  normalsIntersection->SetInputData(boolIntersection->GetOutput());
  normalsIntersection->SetComputeCellNormals(true);
  normalsIntersection->SetComputePointNormals(true);
  normalsIntersection->Update();

  vtkSmartPointer<vtkPolyDataNormals> normalsUnion = vtkSmartPointer<vtkPolyDataNormals>::New();
  normalsUnion->SetInputData(boolUnion->GetOutput());
  normalsUnion->SetComputeCellNormals(true);
  normalsUnion->SetComputePointNormals(true);
  normalsUnion->Update();

  vtkSmartPointer<vtkArrayCalculator> calculator = vtkSmartPointer<vtkArrayCalculator>::New();
  calculator->SetInputData(normalsIntersection->GetOutput());
  calculator->AddCoordinateVectorVariable( "coords",  0, 1, 2 );
  calculator->AddVectorVariable("Normals", "Normals");
  calculator->SetResultArrayName("result");
  calculator->SetFunction("1.0/3.0*dot(coords,Normals)");
  calculator->Update();

  vtkSmartPointer<vtkPassArrays> passFilter = vtkSmartPointer<vtkPassArrays>::New();
  passFilter->SetInputData(calculator->GetOutput());
  passFilter->AddArray(vtkDataObject::POINT, "result");
  passFilter->Update();

  vtkSmartPointer<vtkIntegrateAttributes> integration = vtkSmartPointer<vtkIntegrateAttributes>::New();
  integration->SetInputData(passFilter->GetOutput());
  integration->Update();
  double volumeIntersection = 0.0;
  for (size_t i=0; i<integration->GetOutput()->GetPointData()->GetNumberOfArrays(); ++i) {
     if (std::string("result").compare(integration->GetOutput()->GetPointData()->GetArray(i)->GetName()) == 0) {
       volumeIntersection = integration->GetOutput()->GetPointData()->GetArray(i)->GetTuple(0)[0];
     }
  }

  calculator->SetInputData(normalsUnion->GetOutput());
  calculator->AddCoordinateVectorVariable( "coords",  0, 1, 2 );
  calculator->AddVectorVariable("Normals", "Normals");
  calculator->Update();
  passFilter->Update();
  integration->Update();
  double volumeUnion = std::numeric_limits<double>::infinity();
  for (size_t i=0; i<integration->GetOutput()->GetPointData()->GetNumberOfArrays(); ++i) {
     if (std::string("result").compare(integration->GetOutput()->GetPointData()->GetArray(i)->GetName()) == 0) {
       volumeUnion = integration->GetOutput()->GetPointData()->GetArray(i)->GetTuple(0)[0];
     }
  }

  std::cout << "Union - Intersection = "
            << volumeUnion << " - " << volumeIntersection << " = "
            << volumeUnion - volumeIntersection << std::endl;

  //write the polydata to a file
  vtkSmartPointer<vtkXMLPolyDataWriter> writer1 = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  writer1->SetFileName ( "shape_01.vtp" );
  writer1->SetInputData ( normalsIntersection->GetOutput() );
  writer1->Write();

  vtkSmartPointer<vtkXMLPolyDataWriter> writer2 = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  writer2->SetFileName ( "shape_02.vtp" );
  writer2->SetInputData ( normalsUnion->GetOutput() );
  writer2->Write();

  return 0;
}
