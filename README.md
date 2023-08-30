# Lung-Lobes-Segmentation-in-CT-Scans

Python and C++ implementation on vessel and fissure segmentation in Lung CT Scans based on [Bianca's work](https://pdfs.semanticscholar.org/ddd9/8a73eab745841ba41a13633be76ab1f9c8b0.pdf)

## Usage
**For lung and pulmonary vessel segmentation**. 

I create a ipython notebook for this part (in lung_segmentation.ipynb except the last two parts).

**For pulmonary fissure segmentation**. 


1. Compile the two C++ files for fissure segmentation in Docker.

```
docker build -t connor323lungseg .
docker run -it -w /workdir -v ${PWD}:/workdir connor323lungseg bash
cmake .
make
```

2. Download and preprocess chest CT series, and name sample series as `ORIGINAL_CT.mhd`.

```
cd bin;bash download.sh;cd ..
python bin/preprocess.py bin/images ORIGINAL_CT.mhd
```

3. Use vector-based region growing for fissure segmention.

```
./vector_region_growing ORIGINAL_CT.mhd FISSURE_EXTRACT.mhd
```
Use intensity-based region growing for removing small regions.
```
./region_growing FISSURE_EXTRACT.mhd FISSURE_EXTRACT_REFINED.mhd
```

4. Running the last two sections in the ipython notebook to generate fissure mask and final lung-vessel-fissure mask.

## Results
The result of lung segmentation:  

![ ](images/lung.png)
----

The result of vessel segmentaton:  

![ ](images/vessel.png)

And in 3D view:  

![ ](images/vessel3d.png)
----

The result of fissure segmentation:   
(From left to right: original, after vector-based region growing, after intensity-based region growing)

![ ](images/fissure.png)

The 3D maps of Lung-Vessel-Fissure for 9 cases:  

![ ](images/maps.png)

## Dataset
The dataset I used is [LOLA11](https://lola11.grand-challenge.org), which contains 55 CT Scans. 
