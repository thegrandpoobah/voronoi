
 //****************************************************************************
 //
 //                                                                          
 //****************************************************************************

 void writeHeaderSvg(
                     FILE *filePointer,
                     int width,
                     int height 
                    ) {
  
  if (filePointer) { 
  // fprintf(filePointer, "<svg version=\"1.2\" baseProfile=\"tiny\" width=\"%dpx\" height=\"%dpx\">\n", width, height);
 // }

fprintf(filePointer, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
fprintf(filePointer, "<!-- generated at drububu.com  -->\n");
fprintf(filePointer, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
fprintf(filePointer, "<svg version=\"1.1\" preserveAspectRatio=\"xMinYMin meet\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\" width=\"%dpx\" height=\"%dpx\" viewBox=\"0 0 %d %d\">\n",width, height, width, height );

}

 }


 //****************************************************************************
 //
 //                                                                          
 //****************************************************************************

 void writeFooterSvg(FILE *filePointer) {

  if (filePointer) { 
   fprintf(filePointer, "</svg>\n");
  }
 }


 //****************************************************************************
 //
 //                                                                          
 //****************************************************************************

 void drawLineSVG(
               FILE *filePointer,
               double thickness,
               unsigned long RGBTriplet,
               double xStart,
               double yStart,
               double xEnd,
               double yEnd
              ) {
  int red, green, blue;

  red   = (int)((RGBTriplet>>16)&255);
  green = (int)((RGBTriplet>> 8)&255);
  blue  = (int)((RGBTriplet    )&255);  

  if (filePointer) { 
   fprintf(filePointer, " <path fill=\"none\" stroke=\"RGB(%d,%d,%d)\" stroke-linecap=\"round\" stroke-width=\"%.3f\" d=\"M%.3f,%.3f L%.3f,%.3f\"></path>\n",
    red,
    green,
    blue,
    thickness,
    xStart,
    yStart,
    xEnd,
    yEnd
   );
  }
 }


 //****************************************************************************
 //
 //                                                                          
 //****************************************************************************

 void drawCircleSVG(
                 FILE *filePointer,
                 double xPosition,
                 double yPosition,
                 double radius,
                 unsigned long RGBTriplet
                ) {
  int    red, green, blue;
  double factor;

  red   = (int)((RGBTriplet>>16)&255);
  green = (int)((RGBTriplet>> 8)&255);
  blue  = (int)((RGBTriplet    )&255);  

  factor    = radius * 2.0 * 0.276143;


  if (filePointer) { 
   fprintf(filePointer, " <path fill=\"RGB(%d,%d,%d)\" stroke=\"none\" d=\"M%.3f,%.3f C%.3f,%.3f %.3f,%.3f %.3f,%.3f C%.3f,%.3f %.3f,%.3f %.3f,%.3f C%.3f,%.3f %.3f,%.3f %.3f,%.3f C%.3f,%.3f %.3f,%.3f %.3f,%.3f Z\"></path>\n",
    red,
    green,
    blue,
    xPosition + radius, yPosition,
    xPosition + radius, yPosition-factor, xPosition+factor, yPosition-radius, xPosition       , yPosition-radius,
    xPosition - factor, yPosition-radius, xPosition-radius, yPosition-factor, xPosition-radius, yPosition,
    xPosition - radius, yPosition+factor, xPosition-factor, yPosition+radius, xPosition       , yPosition+radius,
    xPosition + factor, yPosition+radius, xPosition+radius, yPosition+factor, xPosition+radius, yPosition             
   );
  }


 }





