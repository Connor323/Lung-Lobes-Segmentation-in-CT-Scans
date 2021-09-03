# https://stackoverflow.com/questions/60024404/how-to-change-the-orientation-of-simpleitk-image-in-python

import argparse
import os,sys
import numpy as np
import SimpleITK as sitk
import pydicom
from scipy import ndimage
from skimage import measure

# ref https://gist.github.com/mrajchl/ccbd5ed12eb68e0c1afc5da116af614a
def resample_img(itk_image, out_spacing=[1.0, 1.0, 1.0], is_label=False):
    
    # Resample images to out_spacing with SimpleITK
    original_spacing = itk_image.GetSpacing()
    original_size = itk_image.GetSize()

    out_size = [
        int(np.round(original_size[0] * (original_spacing[0] / out_spacing[0]))),
        int(np.round(original_size[1] * (original_spacing[1] / out_spacing[1]))),
        int(np.round(original_size[2] * (original_spacing[2] / out_spacing[2])))]

    resample = sitk.ResampleImageFilter()
    resample.SetOutputSpacing(out_spacing)
    resample.SetSize(out_size)
    resample.SetOutputDirection(itk_image.GetDirection())
    resample.SetOutputOrigin(itk_image.GetOrigin())
    resample.SetTransform(sitk.Transform())
    resample.SetDefaultPixelValue(itk_image.GetPixelIDValue())

    if is_label:
        resample.SetInterpolator(sitk.sitkNearestNeighbor)
    else:
        resample.SetInterpolator(sitk.sitkBSpline)

    return resample.Execute(itk_image)

if __name__ == "__main__":


    parser = argparse.ArgumentParser()
    parser.add_argument('input_file', type=str,help='input file path hello')
    parser.add_argument('output_file', type=str,help='out file path')    
    args = parser.parse_args()
    
    target_spacing = (1.,1.,1.)

    if os.path.isdir(args.input_file):
        mydir = os.path.abspath(args.input_file)
        dicom_names = [os.path.join(mydir,x) for x in os.listdir(mydir)]
        mylist = [(int(pydicom.dcmread(x).InstanceNumber),x) for x in dicom_names]
        mylist = [x for x in sorted(mylist,key=lambda x:x[0])]
        dicom_names = [x[1] for x in mylist]
        reader = sitk.ImageSeriesReader()
        reader.SetFileNames(dicom_names)
    else:
        reader= sitk.ImageFileReader()
        reader.SetFileName(args.input_file)

    img = reader.Execute()
    print('original:')
    print('GetSize',img.GetSize())
    print('GetSpacing',img.GetSpacing())
    print('GetOrigin',img.GetOrigin())
    print('GetDirection',img.GetDirection())
    
    # resamling image to isotropic    
    img = resample_img(img,out_spacing=target_spacing)
    

    origin = (0.,0.,0.)
    direction = (1.,0.,0.,0.,1.,0.,0.,0.,1.)
    
    # disabled intensity rescaling
    # vector_region_growing.cxx is already scaling. 
    #arr = sitk.GetArrayFromImage(img).astype(float)
    #img = sitk.GetImageFromArray(arr)

    img.SetSpacing(target_spacing)
    img.SetOrigin(origin)
    img.SetDirection(direction)

    print('after:')
    print('GetSize',img.GetSize())
    print('GetSpacing',img.GetSpacing())
    print('GetOrigin',img.GetOrigin())
    print('GetDirection',img.GetDirection())

    writer = sitk.ImageFileWriter()
    writer.SetFileName(args.output_file)
    writer.Execute(img)

    print('done')
