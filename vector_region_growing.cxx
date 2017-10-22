#include <iostream>
#include <string>
#include "stdlib.h"
#include <math.h>  
#include <unordered_set>

#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkHessianRecursiveGaussianImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImportImageFilter.h"
#include "itkMedianImageFilter.h"

const unsigned short dimension = 3;
typedef int VoxelType;
typedef float outVoxelType;
typedef itk::Image<VoxelType, dimension> InputImageType;
typedef itk::Image<outVoxelType, dimension> OutputImageType;
typedef itk::Point< float, InputImageType::ImageDimension > PointType;

typedef float HessianVoxelType;
typedef itk::Image<HessianVoxelType, dimension> HessianInnerImageType;
typedef itk::CastImageFilter<InputImageType, HessianInnerImageType> CastFilterType;

typedef itk::HessianRecursiveGaussianImageFilter<HessianInnerImageType> HFilterType;
typedef itk::Vector<HessianVoxelType, dimension> VecVoxType;
typedef itk::Matrix<HessianVoxelType, dimension, dimension> MatVoxType;
typedef itk::Image<VecVoxType, dimension> VecEigHImageType;
typedef itk::Image<MatVoxType, dimension> MatEigHImageType;

typedef itk::MedianImageFilter<OutputImageType, OutputImageType > MedianImageFilterType;
typedef itk::ImportImageFilter< outVoxelType, dimension >   ImportFilterType;
typedef itk::ImageRegionIterator<OutputImageType> OutputImageIterType;
typedef itk::ImageRegionIterator<VecEigHImageType> OutputIterType;
typedef itk::ImageRegionIterator<InputImageType> InputImageIterType;
typedef itk::SymmetricEigenAnalysis<MatVoxType, VecVoxType, MatVoxType> EigValAnalysisType;
typedef MatEigHImageType::Pointer MatEigHImagePointerType;
typedef MatEigHImageType::RegionType MatRegionType;
typedef MatEigHImageType::PointType MatPointType;
typedef MatEigHImageType::SpacingType MatSpacingType;

typedef VecEigHImageType::Pointer VecEigHImagePointerType;
typedef itk::ImageRegionIteratorWithIndex<HFilterType::OutputImageType> HIterType;

class cmp
{
public:
    bool operator() (  HessianVoxelType a,  HessianVoxelType b )
    {
        return std::abs(a) < std::abs(b) ;
    }
};

class EigValHessian {
public:
    MatRegionType region;
    MatSpacingType spacing;
    MatPointType origin;

    CastFilterType::Pointer CastFilter;
    HFilterType::Pointer HFilter;

    EigValAnalysisType EigValAnalysis;

    VecEigHImagePointerType VecEigHImagePointer;

    OutputImageType::Pointer OutputImage;

    ImportFilterType::Pointer importFilter;

    std::unordered_set<outVoxelType> label_set;

    MedianImageFilterType::Pointer medianFilter;

    EigValHessian(const InputImageType::Pointer& image, float sigma, float alpha, float beta, float gama) {
        // This method is to compute the Hessian matrix by ITK filter: HessianRecursiveGaussianImageFilter
        // Furthermore, calculte the fissure structure measurement with given parameters and then using vector 
        // based region growing method to segment fissures.
        // :params image: the 3D lung mask image
        // :params sigma: the standard deviation for Gaussian function in Hessian matrix
        // :params alpha, beta, gama: parameters for fissure representation
        VecVoxType EigVal;
        MatVoxType EigMat,tmpMat;
        for(int i=0;i<3;i++)
            for(int j=0;j<3;j++)
                EigMat[i][j]=0;

        region=image->GetLargestPossibleRegion();
        spacing=image->GetSpacing();
        origin=image->GetOrigin();

        // initialize the Hassian filter
        CastFilter = CastFilterType::New();
        HFilter = HFilterType::New();
        HFilter->SetSigma(sigma);

        EigValAnalysis.SetDimension(3);
        CastFilter->SetInput(image);
        HFilter->SetInput(CastFilter->GetOutput());

        printf("Processing HFilter\n");
        HFilter->Update();

        VecEigHImagePointer=VecEigHImageType::New();
        VecEigHImagePointer->SetRegions(region);
        VecEigHImagePointer->SetOrigin(origin);
        VecEigHImagePointer->SetSpacing(spacing);
        VecEigHImagePointer->Allocate();

        EigVal[0]=EigVal[1]=EigVal[2]=0;
        VecEigHImagePointer->FillBuffer(EigMat[0]);

        OutputImage = OutputImageType::New();
        OutputImage->SetRegions( region );
        OutputImage->Allocate(true); // initialize buffer to zero

        HIterType HIter(HFilter->GetOutput(),region);

        OutputIterType OutputIter(VecEigHImagePointer,region);

        itk::SymmetricSecondRankTensor<float,3> Tensor;

        bool fissure_cond = true;

        InputImageIterType InputImageIter(image, region);

        // this is the mean and std value for vessel which is compute according to
        // histogram analysis in previous result.
        outVoxelType mean = 198.275350;
        outVoxelType std = 42.571917;

        outVoxelType vessel_thesh = mean - 3 * std;

        for(HIter.GoToBegin(),OutputIter.GoToBegin(),InputImageIter.GoToBegin();
            !HIter.IsAtEnd()&&!OutputIter.IsAtEnd()&&!InputImageIter.IsAtEnd();
            ++HIter,++OutputIter,++InputImageIter){
            Tensor=HIter.Get();
            tmpMat[0][0]=Tensor[0];
            tmpMat[0][1]=Tensor[1];
            tmpMat[1][0]=Tensor[1];
            tmpMat[0][2]=Tensor[2];
            tmpMat[2][0]=Tensor[2];
            tmpMat[1][1]=Tensor[3];
            tmpMat[2][1]=Tensor[4];
            tmpMat[1][2]=Tensor[4];
            tmpMat[2][2]=Tensor[5];

            // compute the eigenvalues given a 3*3 Hessian matrix.
            EigValAnalysis.ComputeEigenValuesAndVectors(tmpMat,EigVal,EigMat);

            // obtain the maximum absolute value for eigenvalues
            HessianVoxelType sortedEigVal[3] = {EigVal[0],EigVal[1],EigVal[2]};

            std::sort(sortedEigVal, sortedEigVal+3, cmp());

            // Compute Structure
            HessianVoxelType theta;
            if (sortedEigVal[2] >= 0){
              theta = 0;
            }else{
              theta = 1;
            }
            HessianVoxelType Fstructure, Fsheet, Sfissure;
            Fstructure = theta * exp(-1*(pow((std::abs(sortedEigVal[2])-alpha)/beta,6)));
            Fsheet = exp(-1*(pow(sortedEigVal[1]/gama,6)));

            Sfissure = Fstructure * Fsheet;

            // Convert to uint8 value
            VoxelType pixel_val = (InputImageIter.Get() + 1024) / 4;

            // Thresholding the result fissure structure measurement
            fissure_cond = Sfissure > 0.8 && pixel_val < vessel_thesh ? true : false;

            // Save the corresponding eigenvector for the maximum eigenvalue
            for (int i = 0; i < 3; i++){
              if (EigVal[i] == sortedEigVal[2] && fissure_cond){
                OutputIter.Set(EigMat[i]);
                break;
              }
            }

        }

        printf("Processing computing eigenvalues and eigenvectors\n");
        VecEigHImagePointer->Update();
        printf("Finish computing eigenvalues and eigenvectors\n");

        // Compute the vector based region growing with given eigenvector.
        const OutputImageType::RegionType region = VecEigHImagePointer->GetBufferedRegion();
        const OutputImageType::SizeType size = region.GetSize();
        const unsigned int xs = size[0];
        const unsigned int ys = size[1];
        const unsigned int zs = size[2];
        float label = 1;
        for (int z = 0; z < zs; z++){
          for (int y = 0; y < ys; y++){
            for (int x = 0; x < xs; x++){
              PointType point;
              point[0] = x;    
              point[1] = y;
              point[2] = z;  
              OutputImageType::IndexType out_index;
              VecEigHImageType::IndexType vec_index;
  
              OutputImage->TransformPhysicalPointToIndex( point, out_index );
              VecEigHImagePointer->TransformPhysicalPointToIndex( point, vec_index );
              VecEigHImageType::PixelType p_vec = VecEigHImagePointer->GetPixel(vec_index);

              if (OutputImage->GetPixel(out_index) == 0 && p_vec[0]+p_vec[1]+p_vec[2] != 0){
                int volume = 1;
                int *vol_point = &volume;
                this->region_growing(point, label, vol_point);
                label = label + 1;
              }
            }
          }
        }
    }
    void region_growing(PointType, float, int *);
};
void EigValHessian::region_growing(PointType point, float label, int *vol_point){
  // This function is to compute the vector based region growing.
  // :params point: the point for region growing
  // :params label: point label
  // :params vol_point: the volume for given label to limit the number of recursive.
  int x = point[0]; int y = point[1]; int z = point[2];
  const OutputImageType::RegionType region = VecEigHImagePointer->GetBufferedRegion();
  const OutputImageType::SizeType size = region.GetSize();
  const unsigned int xs = size[0];
  const unsigned int ys = size[1];
  const unsigned int zs = size[2];

  PointType neighbors[6];
  neighbors[0][0] = x+1; neighbors[1][0] = x-1; neighbors[2][0] = x; neighbors[3][0] = x; neighbors[4][0] = x; neighbors[5][0] = x;    
  neighbors[0][1] = y; neighbors[1][1] = y; neighbors[2][1] = y+1; neighbors[3][1] = y-1; neighbors[4][1] = y; neighbors[5][1] = y;   
  neighbors[0][2] = z; neighbors[1][2] = z; neighbors[2][2] = z; neighbors[3][2] = z; neighbors[4][2] = z+1; neighbors[5][2] = z-1;   
  
  VecEigHImageType::IndexType p_index;
  VecEigHImagePointer->TransformPhysicalPointToIndex( point, p_index );
    VecEigHImageType::PixelType p = VecEigHImagePointer->GetPixel( p_index );
    
  for (int i = 0; i < 6; i++){
    if (neighbors[i][0] >= 0 && neighbors[i][0] < xs &&
        neighbors[i][1] >= 0 && neighbors[i][1] < ys &&
        neighbors[i][2] >= 0 && neighbors[i][2] < zs){
      OutputImageType::IndexType out_index;
      OutputImage->TransformPhysicalPointToIndex( neighbors[i], out_index );
      if (OutputImage->GetPixel(out_index) == 0){
        VecEigHImageType::IndexType vec_index;
        VecEigHImagePointer->TransformPhysicalPointToIndex( neighbors[i], vec_index );
        VecEigHImageType::PixelType np = VecEigHImagePointer->GetPixel( vec_index );
        float product = np[0] * p[0] + np[1] * p[1] + np[2] * p[2];
        if (product > 0.9){
          *vol_point = *vol_point + 1;
          OutputImage->SetPixel( out_index, label);
          this->region_growing(neighbors[i], label, vol_point);
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
    
  typedef itk::ImageFileReader< InputImageType >  readerType;

  float sigma = 0.5;
  float alpha = 90;
  float beta = 120;
  float gama = 100;

  // Read Image
  readerType::Pointer reader = readerType::New();
  reader->SetFileName( argv[1] );
  reader->Update();

  // Compute eigenvalues
  std::cout << "  Compute Eigenvalues " << std::endl;
  EigValHessian eigenvalues = EigValHessian::EigValHessian( reader->GetOutput(), sigma, alpha, beta, gama );

  std::cout << "   Saving Image..." << std::endl;
  typedef itk::ImageFileWriter < OutputImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( argv[2] );
  writer->SetInput( eigenvalues.OutputImage );

  writer->Update();
  printf("Time taken: %.2fs\n", (float)(clock() - tStart)/CLOCKS_PER_SEC);

  return EXIT_SUCCESS;
}