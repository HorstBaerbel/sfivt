#include "imageIO.h"

#include <iostream>
#include <memory.h>


std::vector<uint8_t> ImageIO::loadFile_RGBA32(const std::string & fileName, uint32_t & width, uint32_t & height, bool keepAspectRatio)
{
	std::vector<uint8_t> rawData;
	//check the file signature and deduce its format
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(fileName.c_str(), 0);
	if (fif == FIF_UNKNOWN) {
		//try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilename(fileName.c_str());
	}
	//format ok? check that the plugin has reading capabilities ...
	if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
		//ok, let's load the file
		FIBITMAP * fiBitmap = FreeImage_Load(fif, fileName.c_str());
		if (fiBitmap != nullptr)
		{
			//loaded. convert to 32bit if necessary
			FIBITMAP * fiConverted = nullptr;
			if (FreeImage_GetBPP(fiBitmap) != 32)
			{
				fiConverted = FreeImage_ConvertTo32Bits(fiBitmap);
				if (fiConverted != nullptr)
				{
					//free original bitmap data
					FreeImage_Unload(fiBitmap);
					fiBitmap = fiConverted;
				}
			}
			//convert image to raw RGBA data stream
			if (fiBitmap != nullptr)
			{
				const uint32_t originalWidth = FreeImage_GetWidth(fiBitmap);
				const uint32_t originalHeight = FreeImage_GetHeight(fiBitmap);
				//smart resize image first if needed
				if (fiBitmap != nullptr && (originalWidth != width || originalHeight != height))
				{
					if (keepAspectRatio)
					{
						//make sure the image fits within width x height
						const float originalAspect = (float)originalWidth / (float)originalHeight;
						//check if adjusting the width gives acceptable new height
						if (width / originalAspect <= height)
						{
							//zoom image to make width fit. heigth follows
							const float zoomWidth = (float)width / (float)originalWidth;
							height = zoomWidth * originalHeight;
						}
						//check if adjusting the height gives acceptable new width
						else if (height * originalAspect <= width)
						{
							//zoom image to make height fit. width follows
							const float zoomHeight = (float)height / (float)originalHeight;
							width = zoomHeight * originalWidth;
						}
					}
					//now try to resample image with good filtering
					FIBITMAP * fiScaled = FreeImage_Rescale(fiBitmap, width, height, FILTER_BILINEAR);//CATMULLROM);
					if (fiScaled != nullptr)
					{
						//worked. delete old image and use scaled image in rest of function.
						FreeImage_Unload(fiBitmap);
						fiBitmap = fiScaled;
					}
				}
				//const unsigned int pitch = FreeImage_GetPitch(fiBitmap);
				//loop through scanlines and add all pixel data to the return vector
				//this is necessary, because width*height*bpp might not be == pitch
				unsigned char * tempData = new unsigned char[width * height * 4];
				for (size_t i = 0; i < height; i++)
				{
					const BYTE * scanLine = FreeImage_GetScanLine(fiBitmap, i);
					memcpy(tempData + (i * width * 4), scanLine, width * 4);
				}
				//convert from BGRA to RGBA
				/*for(size_t i = 0; i < width*height; i++)
				{
					RGBQUAD bgra = ((RGBQUAD *)tempData)[i];
					RGBQUAD rgba;
					rgba.rgbBlue = bgra.rgbRed;
					rgba.rgbGreen = bgra.rgbGreen;
					rgba.rgbRed = bgra.rgbBlue;
					rgba.rgbReserved = bgra.rgbReserved;
					((RGBQUAD *)tempData)[i] = rgba;
				}*/
				rawData = std::vector<unsigned char>(tempData, tempData + width * height * 4);
				//free bitmap data
				FreeImage_Unload(fiBitmap);
				delete[] tempData;
			}
		}
		else
		{
			std::cout << "Error - Failed to load image!" << std::endl;
		}
	}
	else
	{
		std::cout << "Error - File type unknown/unsupported!" << std::endl;
	}
	return rawData;
}
