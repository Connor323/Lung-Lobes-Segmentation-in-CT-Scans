#include <iostream>
#include <string>
#include "stdlib.h"

#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImportImageFilter.h"

const unsigned short dimension = 3;

typedef float floatVoxelType;
typedef itk::Image<floatVoxelType, dimension> ImageType;
typedef itk::Point< float, ImageType::ImageDimension > PointType;
typedef itk::ImageRegionIterator<ImageType> OutputIterType;

class Regiongrowing {
public:
    ImageType::Pointer OutputImage;

    unsigned int xs;
    unsigned int ys;
    unsigned int zs;

    Regiongrowing(const ImageType::Pointer& image, float thresh_val) {
        // This method is to compute the 3D region growing
        // :params image: the 3D lung mask image
        // :params thresh_val: the threshold value for region growing
        OutputImage = ImageType::New();
        OutputImage->SetRegions( image->GetLargestPossibleRegion() );
        OutputImage->Allocate(true); // initialize buffer to zero

        OutputIterType OutputIter(OutputImage,image->GetLargestPossibleRegion());

        int shape[3];
        xs = image->GetLargestPossibleRegion().GetSize()[0];  
        ys = image->GetLargestPossibleRegion().GetSize()[1];  
        zs = image->GetLargestPossibleRegion().GetSize()[2];  

        // compute region growing 
        float label = 1;
        for (int z = 0; z < zs; z++){
          for (int y = 0; y < ys; y++){
            for (int x = 0; x < xs; x++){
              PointType point;
              point[0] = x;  
              point[1] = y;  
              point[2] = z;   
              ImageType::IndexType out_index;
  
              image->TransformPhysicalPointToIndex( point, out_index );
              ImageType::PixelType p_vec = image->GetPixel(out_index);
              if (OutputImage->GetPixel(out_index) == 0 && p_vec != 0){
                int volume = 1;
                int *vol_point = &volume;
                this->region_growing(image, point, label, thresh_val, vol_point);
                label = label + 1;
              }
            }
          }
        }
    }
    void region_growing(const ImageType::Pointer& , PointType, float, float, int *);
};
void Regiongrowing::region_growing(const ImageType::Pointer& image, PointType point, float label, float thresh_val, int *vol_point){
  //This function is to compute region growing.
  // :params image: the image for region growing
  // :params point: the point for region growing
  // :params label: point label
  // :params thresh_val: the threshold value
  // :params vol_point: the volume for given label to limit the number of recursive.
  float x = point[0]; float y = point[1]; float z = point[2];

  PointType neighbors[6];
  neighbors[0][0] = x+1; neighbors[1][0] = x-1; neighbors[2][0] = x; neighbors[3][0] = x; neighbors[4][0] = x; neighbors[5][0] = x;    
  neighbors[0][1] = y; neighbors[1][1] = y; neighbors[2][1] = y+1; neighbors[3][1] = y-1; neighbors[4][1] = y; neighbors[5][1] = y;   
  neighbors[0][2] = z; neighbors[1][2] = z; neighbors[2][2] = z; neighbors[3][2] = z; neighbors[4][2] = z+1; neighbors[5][2] = z-1;   

  ImageType::IndexType p_index;
  image->TransformPhysicalPointToIndex( point, p_index );
    ImageType::PixelType p = image->GetPixel( p_index );
    
  for (int i = 0; i < 6; i++){
    if (neighbors[i][0] >= 0 && neighbors[i][0] < xs &&
        neighbors[i][1] >= 0 && neighbors[i][1] < ys &&
        neighbors[i][2] >= 0 && neighbors[i][2] < zs){
      ImageType::IndexType out_index;
      image->TransformPhysicalPointToIndex( neighbors[i], out_index );
      if (OutputImage->GetPixel(out_index) == 0){
        ImageType::PixelType np = image->GetPixel( out_index );
        float diff = std::abs(np - p);
        if (np > 0 && *vol_point < 20000){ 
          OutputImage->SetPixel( out_index, label);
          *vol_point = *vol_point + 1;
          this->region_growing(image, neighbors[i], label, thresh_val, vol_point);

        } 
      }
    }  
  }
}



int main( int argc, char * argv[] )
{
  time_t tStart = clock();

  if( argc < 2 ) {
      std::cerr << "Usage: " << std::endl;
      std::cerr << argv[0] << "  inputImageFile  outputImageFile" << std::endl;
      return EXIT_FAILURE;
  }
    
  typedef itk::ImageFileReader< ImageType >  readerType;

  float thresh = 0.1;

  // Read Image
  readerType::Pointer reader = readerType::New();
  reader->SetFileName( argv[1] );
  reader->Update();

  // Compute eigenvalues
  std::cout << "  Region Growing " << std::endl;
  Regiongrowing rg = Regiongrowing::Regiongrowing( reader->GetOutput(), thresh );

  std::cout << "   Saving Image..." << std::endl;
  typedef itk::ImageFileWriter < ImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( argv[2] );
  writer->SetInput( rg.OutputImage );

  writer->Update();
  printf("Time taken: %.2fs\n", (float)(clock() - tStart)/CLOCKS_PER_SEC);

  return EXIT_SUCCESS;
}