//=========================================================================
// File Name: 	readImage_BMP.cpp
// Purpose: 	read BMP image and store the data in given arrays
//	Copyright:	Yinlong Sun, all rights reserved
//=========================================================================
// File History:
// 2002/10/02: Created
// 2002/10/12: Removed zero handling (no need for RGB type)
//=========================================================================
// Note:
// - Only handle RGB type images, not grayscale!
//=========================================================================
#include <iostream.h>
#include <fstream.h> // file IO classes
#include <stdlib.h> // exit()
#include "readImage_BMP.h"
//=========================================================================
extern unsigned short int imageHolder_red[501][501]; // hold image info
extern unsigned short int imageHolder_green[501][501];
extern unsigned short int imageHolder_blue[501][501];
//=========================================================================
int readImage_BMP(const char fileName_inputImage[])
{
	fstream fin(fileName_inputImage, ios::in | ios::binary); // BMP is binary

  	if(!fin.good()) // open file fails
	{
   			return 0;
	}

	// parameters to read from image file header
	char fileTypeChar1, fileTypeChar2;	//1+1=2 bytes
	int sizeOfFile;							//4 bytes
	int futureUse;								//4 bytes
	int byteOffset;							//4 bytes
	int numOfByteInHeader;					//4 bytes
	int widthOfImage; 						//4 bytes
	int heightOfImage; 						//4 bytes
	short int numOfColorPlane;				//2 bytes
	short int numOfBitPerPixel; // 24 for RGB, 8 for grayscale, 2 bytes
	int typeOfCompress;						//4 bytes
	int sizeOfImage;							//4 bytes
	int horizontalResolution;				//4 bytes
	int verticalResolution;					//4 bytes
	int numOfColorIndex;						//4 bytes
	int numOfColorIndexImportant;			//4 bytes

  	// read parameters from image file header
  	fin.read(reinterpret_cast<char *>(&fileTypeChar1),sizeof fileTypeChar1);
  	fin.read(reinterpret_cast<char *>(&fileTypeChar2),sizeof fileTypeChar2);
	if (fileTypeChar1 != 'B' || fileTypeChar2!= 'M') // not BMP file
   {
   			return 0;
   }

  	fin.read(reinterpret_cast<char *>(&sizeOfFile),sizeof sizeOfFile);
  	fin.read(reinterpret_cast<char *>(&futureUse),sizeof futureUse);
  	fin.read(reinterpret_cast<char *>(&byteOffset),sizeof byteOffset);
  	fin.read(reinterpret_cast<char *>(&numOfByteInHeader),sizeof numOfByteInHeader);
  	fin.read(reinterpret_cast<char *>(&widthOfImage),sizeof widthOfImage);
  	fin.read(reinterpret_cast<char *>(&heightOfImage),sizeof heightOfImage);
  	fin.read(reinterpret_cast<char *>(&numOfColorPlane),sizeof numOfColorPlane);
  	fin.read(reinterpret_cast<char *>(&numOfBitPerPixel),sizeof numOfBitPerPixel);
  	fin.read(reinterpret_cast<char *>(&typeOfCompress),sizeof typeOfCompress);
  	fin.read(reinterpret_cast<char *>(&sizeOfImage),sizeof sizeOfImage);
  	fin.read(reinterpret_cast<char *>(&horizontalResolution),sizeof horizontalResolution);
  	fin.read(reinterpret_cast<char *>(&verticalResolution),sizeof verticalResolution);
  	fin.read(reinterpret_cast<char *>(&numOfColorIndex),sizeof numOfColorIndex);
  	fin.read(reinterpret_cast<char *>(&numOfColorIndexImportant),sizeof numOfColorIndexImportant);

	// parameters to read trailing zero's of each row, and at end of file
	unsigned char ch;
   if (numOfBitPerPixel == 24) // input BMP image is RGB
   {
		/*
   	int numOfValidPixel = widthOfImage * heightOfImage * 3;
   	int imageSizeIncZero = sizeOfFile - byteOffset;
   	int numOfZero = imageSizeIncZero - numOfValidPixel;
		int numOfZeroAtEnd = 2;
		int numOfZeroPerRow = (numOfZero-numOfZeroAtEnd) / heightOfImage;
   	if ((numOfZero-numOfZeroAtEnd) % heightOfImage != 0) // fault handling zero's
   	{
   			return 0;
   	}
		*/

   	// start reading data of RGB image
//		int k = 0; // count pixel
		int i, j;
		fin.seekg(byteOffset); // set cursor to the start of image data
      for (i=0; i<heightOfImage; i++) // loop for rows
   	{
			for (j=0; j<widthOfImage; j++ ) // loop for colomns of each row
      	{
      		fin.read(reinterpret_cast<char *>(&ch),sizeof ch);
      		imageHolder_blue[i][j] = static_cast<unsigned short int>(ch);
//				cout << imageHolder_blue[i][j] << "  ";
         	fin.read(reinterpret_cast<char *>(&ch),sizeof ch);
         	imageHolder_green[i][j] = static_cast<unsigned short int>(ch);
//				cout << imageHolder_green[i][j] << "  ";
         	fin.read(reinterpret_cast<char *>(&ch),sizeof ch);
         	imageHolder_red[i][j] = static_cast<unsigned short int>(ch);
//				cout << imageHolder_red[i][j] << "  ";
//				k++;
      	}
//			cout << endl;
//      	fin.seekg(numOfZeroPerRow, ios::cur); // set cursor numOfZeroPerRow bytes forward
      		// to skip trailing zero's of each row, which are not color values
   	}
//		cout << "Total pixels: " << k << endl;
   }
   else // not RGB image, such as grayscale
   {
   			return 0;
   }

	fin.close();

	return 1;
}
//=========================================================================