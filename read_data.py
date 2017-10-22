"""
This file is to load the input image and convert to numpy array.
"""
import SimpleITK as sitk
import matplotlib.pyplot as plt

class LoadData:
    """
    This class is designed to load "one" input image.
    """
    def __init__(self, path, name):
        """
        :param path: image derectory
        :param name: image name
        """
        self.img_path = path + name
        self.image = None
        self.slices = None

    def loaddata(self):
        """
        Load image with given image path.
        :return: None
        """
        self.image = sitk.ReadImage(self.img_path)

    def tileimage(self, index1, index2):
        """
        Tile the 3D image into two selected slices for showing.
        :param index1: selected slice 1
        :param index2: selected slice 2
        :return: None
        """
        self.slices = sitk.Tile(self.image[:, :, index1],
                                self.image[:, :, index2],
                                (2, 1, 0))

    def sitk_show(self, title=None, margin=0.0, dpi=40):
        """
        Show the tiled 2D images.
        :param title: Title
        :param margin: Margin
        :param dpi: ???
        :return: None
        """
        nda = sitk.GetArrayFromImage(self.slices)
        figsize = (1 + margin) * nda.shape[0] / dpi, (1 + margin) * nda.shape[1] / dpi
        extent = (0, nda.shape[1], nda.shape[0], 0)
        fig = plt.figure(figsize=figsize, dpi=dpi)
        ax = fig.add_axes([margin, margin, 1 - 2 * margin, 1 - 2 * margin])

        plt.set_cmap("gray")
        ax.imshow(nda, extent=extent, interpolation=None)
        if title:
            plt.title(title)
        plt.show()


def main():
    data_path = "resource/"
    img_name = "lola11-01.mhd"

    # Slice index to visualize with 'sitk_show'
    idxSlice1 = 26
    idxSlice2 = 50

    # int label to assign to the segmented gray matter
    labelGrayMatter = 1

    data = LoadData(data_path, img_name)
    data.loaddata()
    print "after read image..."
    data.tileimage(idxSlice1, idxSlice2)
    data.sitk_show()

    print "after showing image..."
    image_array = sitk.GetArrayFromImage(data.image)
    print "the shape of image is ", image_array.shape

    # output = sitk.DiscreteGaussianFilter(input, 1.0, 5)
    # sitk.Show(image)

if __name__ == "__main__":
    main()