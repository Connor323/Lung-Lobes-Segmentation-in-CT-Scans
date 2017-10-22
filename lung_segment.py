"""
This script is to segment the lung out of background as a prepocessing for lobes segementation.
"""
import SimpleITK as sitk
import gui

class LungSegment:
    """
    This class is designed for 3D segmentation of lung, including the methods:
    ...
    """
    def __init__(self, img):
        self.img = img
        self.temp_img = None
        self.img_uint8 = None

    def conv_2_uint8(self, WINDOW_LEVEL=(1050,500)):
        """
        Convert original image to 8-bit image
        :param WINDOW_LEVEL: Using an external viewer (ITK-SNAP or 3DSlicer)
                             we identified a visually appealing window-level setting
        :return: None
        """
        # self.img_uint8 = sitk.Cast(self.img,
        #                           sitk.sitkUInt8)
        self.img_uint8 = sitk.Cast(sitk.IntensityWindowing(self.img,
                                  windowMinimum=WINDOW_LEVEL[1] - WINDOW_LEVEL[0] / 2.0,
                                  windowMaximum=WINDOW_LEVEL[1] + WINDOW_LEVEL[0] / 2.0),
                                  sitk.sitkUInt8)

    def regiongrowing(self, seed_pts):
        """
        Implement ConfidenceConnected by SimpleITK tools with given seed points
        :param seed_pts: seed points for region growing [(z,y,x), ...]
        :return: None
        """
        self.temp_img = sitk.ConfidenceConnected(self.img, seedList=seed_pts,
                                                           numberOfIterations=0,
                                                           multiplier=2,
                                                           initialNeighborhoodRadius=1,
                                                           replaceValue=1)

    def image_showing(self, title=''):
        """
        Showing image.
        :return: None
        """
        gui.MultiImageDisplay(image_list=[sitk.LabelOverlay(self.img_uint8, self.temp_img)],
                              title_list=[title])

    def image_closing(self, size=7):
        """
        Implement morphological closing to fix the "holes" inside the image.
        :param size: the size the closing kernel
        :return: None
        """
        closing = sitk.BinaryMorphologicalClosingImageFilter()
        closing.SetForegroundValue(1)
        closing.SetKernelRadius(size)
        self.temp_img = closing.Execute(self.temp_img)