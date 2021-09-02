# one scan from LUNA dataset
if [ ! -f "images.zip" ]; then

	    curl -X GET \
		        https://services.cancerimagingarchive.net/services/v4/TCIA/query/getImage?SeriesInstanceUID=1.3.6.1.4.1.14519.5.2.1.6279.6001.113679818447732724990336702075 \
			    -o images.zip

	        unzip images.zip -d images
fi
