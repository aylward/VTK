vtk_module(vtkRenderingVolume
  GROUPS
    Rendering
  TEST_DEPENDS
    vtkFiltersModeling
    vtkTestingCore
    vtkTestingRendering
    vtkRenderingVolume${VTK_RENDERING_BACKEND}
    vtkRenderingFreeType
    vtkIOXML
    vtkImagingSources
    vtkImagingGeneral
    vtkImagingHybrid
    vtkInteractionStyle
    vtkIOLegacy
    vtkIOImage
  KIT
    vtkRendering
  DEPENDS
    vtkCommonCore
    vtkCommonExecutionModel
    vtkRenderingCore
  PRIVATE_DEPENDS
    vtkCommonDataModel
    vtkCommonMath
    vtkCommonMisc
    vtkCommonSystem
    vtkCommonTransforms
    vtkIOXML
    vtkImagingCore
  )
