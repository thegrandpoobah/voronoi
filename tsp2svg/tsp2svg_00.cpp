 
 #include <stdio.h>
 #include <stdlib.h>
 #include <math.h>
 #include <float.h>
 #include <string.h>
 #include <conio.h>
 #include <ctype.h>
 #include <iostream.h>
 #include <time.h>

 #include "../util/utilinfo.h"
 #include "../util/utilmath.h"
 #include "../util/utilsvg.h"


 #define MAXIMUM_NUMBER_OF_POSITIONS  255000
 #define SVG_BORDER 10

 //****************************************************************************
 //
 //****************************************************************************

 unsigned long _numberOfPositions;

 //****************************************************************************
 //
 //****************************************************************************

 struct PositionStructure {
  double xPosition;
  double yPosition;
  double radius;
 };

 struct PositionStructure  *_positionBuffer;

 //****************************************************************************
 //
 //****************************************************************************

 struct BoundingBoxStructure {
  double xMinimum;
  double yMinimum;
  double xMaximum;
  double yMaximum;
  double xOffset;
  double yOffset;
  bool   flagInit;
 };

 struct BoundingBoxStructure _boundingbox;

 bool flagVariableLineWidth = false;
 

 double minimumDistancePoints = -1.0;


 //****************************************************************************
 //
 //****************************************************************************

 void endProgram(char *); 
 void init();
 void readPositions();
 void generateSVG();
 void readSVGLine(char *);


 char fileNameTSP[80];
 char fileNameCYC[80];

 //****************************************************************************
 //
 //                                                                          
 //****************************************************************************

 void main(
           int argc,
           char *argv[]
          )
 {
  system("cls");  

  printf("\n\n");
  printf(" ----------------------------------------------------------------------\n");
  printf(" tsp2svg\n");
  printf(" use 'tsp2svg [filename].svg [filename].cyc'    for constant line width\n");
  printf(" use 'tsp2svg [filename].svg [filename].cyc +w' for variable line width\n");
  printf(" ----------------------------------------------------------------------\n");


  if (argc < 3 ) {
   endProgram(" wrong numer of arguments... use 'tsp2svg [filename].svg [filename].cyc'\n");
  }

  if ( argc == 4 ){
   if( strstr( argv[3], "+w" ) != NULL && strlen( argv[3] ) == strlen( "+w" ) ) {
    flagVariableLineWidth = true;
   }
  }

  printf(" variable line width is%sused.\n", flagVariableLineWidth ? " " : " not " );

  printf(" ----------------------------------------------------------------------\n");


  sprintf(fileNameTSP, "%s", argv[1]);
  sprintf(fileNameCYC, "%s", argv[2]);

  printf(" .svg: %s\n", fileNameTSP);
  printf(" .cyc: %s\n", fileNameCYC);
  printf("\n\n");

  init();

  endProgram(" svg succesfull created (tsp_art.svg)...");
 }

 //****************************************************************************
 //
 //                                                                          
 //****************************************************************************

 void init() {

  printf(" init\n");

  _positionBuffer = (struct PositionStructure *)calloc(MAXIMUM_NUMBER_OF_POSITIONS, sizeof(struct PositionStructure)); 
      
  if (_positionBuffer == (struct PositionStructure *)NULL) {               
   endProgram(" allocation problem...");
  }
  
  

  _numberOfPositions = 0;
  _boundingbox.flagInit = true;
  readPositions();

  printf(" number of positions: %d               \n", _numberOfPositions );


  generateSVG();
 }


 //****************************************************************************
 //
 // positions are derived from content.svg file                                                                         
 // if position is not valid; end program
 // 
 //****************************************************************************

 void readPositions() {
  int  len;
  char line[256];
  FILE *input;

  printf("\n - read positions -----------------------------------------------------\n");

  _numberOfPositions = 0;

  //***************************************************************************
  // open file
  //***************************************************************************

  input = fopen(fileNameTSP, "r");

  printf(" file: %s\n", fileNameTSP );

  if(!input) {
   sprintf(line, " file %s not valid...\n", fileNameTSP);
   endProgram(line); 
  }

  //***************************************************************************
  // read lines
  //***************************************************************************

  while(fgets(line, sizeof(line) , input) != NULL)  {
   if(strstr(line, "circle") != NULL) {
    printf(" %20.20s...                \r", line);
    readSVGLine(line);
   } else {
    // printf("not relevant %0.60s...\n", line);
   }
  }

  //***************************************************************************
  // close file
  //***************************************************************************

  if (input) {
   fclose(input);
  }
 }



 //****************************************************************************
 //
 //                                                                          
 //****************************************************************************

 void readSVGLine(char *line) {
  unsigned int    a, b;
  double xPosition, yPosition, radius;
  char   k, tmp[200];
   
  xPosition = -1.0;
  yPosition = -1.0;
  radius    = -1.0;

  sprintf(tmp, "");

// printf(" %s\n", line );


  for(a=0; a<strlen(line)-4; a+=1) {

   //**************************************************************************
   // get x position
   //**************************************************************************

   if( xPosition==-1.0 && line[a]=='c' && line[a+1]=='x' && line[a+2]=='=' && line[a+3]=='"'){
    for(b=a+4; b<strlen(line); b+=1){
     if(isdigit(line[b]) || line[b]=='.'){
      tmp[b-(a+4)] = line[b];
     } else {
      tmp[b-(a+4)] ='\0';
      xPosition = atof(tmp);
      break;
     }
    }
   }

   //**************************************************************************
   // get y position
   //**************************************************************************

   if( yPosition == -1.0 && line[a]=='c' && line[a+1]=='y' && line[a+2]=='=' && line[a+3]=='"'){
    for(b=a+4; b<strlen(line); b+=1){
     if(isdigit(line[b]) || line[b]=='.'){
      tmp[b-(a+4)] = line[b];
     } else {
      tmp[b-(a+4)] ='\0';
      yPosition = atof(tmp);
      break;
     }
    }
   }

   //**************************************************************************
   // get radius
   //**************************************************************************

   if( radius == -1.0 && line[a]=='r' && line[a+1]=='=' && line[a+2]=='"'){

// printf(" radius!\n");

    tmp[ 0 ] = '\0';


    for( b= a + 3; b < strlen( line ); b += 1 ) {

// printf(" char %c %d %d \n", line[ b ], b - ( a + 3 ), strlen( line ) );

     if( isdigit( line[ b ] ) || line[ b ] == '.' ) {
      tmp[ b - ( a + 3 )     ] = line[ b ];
      tmp[ b - ( a + 3 ) + 1 ] = '\0';
     } else {

//printf(" *\n");
//printf(" **\n");
//printf(" ** >%s<\n", tmp);

      radius = atof( tmp );

// printf(" > %s %7.3f\n", tmp, radius );

      break;
     }




    }


   }
  }

  //***************************************************************************
  // store data if relevant
  //***************************************************************************

  if(xPosition!=-1.0 && yPosition!=-1.0 && radius!=-1.0) {

   printf(" pos.: %7.3f %7.3f / rad.: %7.3f             \r", xPosition, yPosition, radius );

   // mimunimum maximum

   if(_boundingbox.flagInit || xPosition < _boundingbox.xMinimum) { _boundingbox.xMinimum = xPosition; }
   if(_boundingbox.flagInit || xPosition > _boundingbox.xMaximum) { _boundingbox.xMaximum = xPosition; }
   if(_boundingbox.flagInit || yPosition < _boundingbox.yMinimum) { _boundingbox.yMinimum = yPosition; }
   if(_boundingbox.flagInit || yPosition > _boundingbox.yMaximum) { _boundingbox.yMaximum = yPosition; }

   _boundingbox.flagInit = false;

   _positionBuffer[ _numberOfPositions ].xPosition = xPosition;
   _positionBuffer[ _numberOfPositions ].yPosition = yPosition;
   _positionBuffer[ _numberOfPositions ].radius    = radius;
   _numberOfPositions++;

   if (_numberOfPositions >= MAXIMUM_NUMBER_OF_POSITIONS) {
    endProgram("increase buffersize...\n");
   }
  }

  else {
   endProgram("no match svg file...\n");
  }
 }

 //****************************************************************************
 //
 //                                                                          
 //****************************************************************************

 void generateSVG() {
  int      a, i0, i1, entries, index, lookupTour[MAXIMUM_NUMBER_OF_POSITIONS];
  unsigned long counterPositionsTour;
  double   xOffset, yOffset, xCrossProduct, yCrossProduct, lambdaOne, lambdaSecond, factor;
  double   xPosition, yPosition;
  char     line[128],tmp[128], path[5500];
  FILE     *input, *output;

  printf("\n - generate svg -------------------------------------------------------\n");





  //***************************************************************************
  // open file
  //***************************************************************************

  input = fopen( fileNameCYC , "r");

  printf(" file: %s\n", fileNameCYC );

  if (!input) {
   sprintf(tmp, " file %s not valid...\n", fileNameCYC);
   endProgram(tmp); 
  }

  output = fopen("tsp_art.svg", "w");

  if (!output) {
   endProgram("svg problem...");
  }

  //***************************************************************************
  // open file
  //***************************************************************************




  writeHeaderSvg(
                 output,
                 (_boundingbox.xMaximum - _boundingbox.xMinimum) + (SVG_BORDER<<1),
                 (_boundingbox.yMaximum - _boundingbox.yMinimum) + (SVG_BORDER<<1)
                );






  entries = _numberOfPositions;

  xOffset = -_boundingbox.xMinimum;
  yOffset = -_boundingbox.yMinimum;

  for (a=0; a < entries; a += 1) {
   _positionBuffer[a].xPosition += xOffset + SVG_BORDER;
   _positionBuffer[a].yPosition = _positionBuffer[a].yPosition + yOffset + SVG_BORDER;
  }

  //***************************************************************************
  // read tour
  //***************************************************************************
 
  counterPositionsTour = 0;

  while( fgets(line, sizeof(line) , input) != NULL ) {
   int len = strlen(line)-1;
   if(line[len] == '\n') { line[len] = 0; }
 
   index = atoi(line);
   lookupTour[counterPositionsTour] = index;
   counterPositionsTour+=1L;
  }

  //***************************************************************************
  // check
  // tour should have same number of positions
  //***************************************************************************
 
  if (counterPositionsTour != _numberOfPositions) {

   printf(" number of positions in svg: %d\n", _numberOfPositions);
   printf(" number of positions in cyc: %d\n", counterPositionsTour);
   printf(" both positions should be equals...");

   endProgram(" positions don't match...\n");
  }

  //***************************************************************************
  // write svg
  //***************************************************************************

  fprintf(output, " <g fill=\"black\" stroke=\"none\">\n");

  entries = counterPositionsTour;



if( flagVariableLineWidth ){

  for(a=0; a<entries; a+=1) {

   i0 = lookupTour[a];
   i1 = lookupTour[(a+1)%entries];






   fprintf(output, " <circle cx=\"%.3f\" cy=\"%.3f\" r=\"%.3f\" />\n",
    _positionBuffer[ i0 ].xPosition,
    _positionBuffer[ i0 ].yPosition,
    _positionBuffer[ i0 ].radius
   );

   xCrossProduct =  ( _positionBuffer[ i1 ].yPosition - _positionBuffer[ i0 ].yPosition );
   yCrossProduct = -( _positionBuffer[ i1 ].xPosition - _positionBuffer[ i0 ].xPosition );


   //***************************************************************************
   // init line thickness
   //***************************************************************************

   lambdaOne     = _positionBuffer[ i0 ].radius / sqrt( pow( xCrossProduct, 2.0 ) + pow( yCrossProduct, 2.0 ) );
   lambdaSecond  = _positionBuffer[ i1 ].radius / sqrt( pow( xCrossProduct, 2.0 ) + pow( yCrossProduct, 2.0 ) );




   fprintf(output, " <path d=\"M %.3f,%.3f L %.3f,%.3f L %.3f,%.3f L %.3f,%.3f z\"/>\n",
    _positionBuffer[ i0 ].xPosition + lambdaOne    * xCrossProduct,
    _positionBuffer[ i0 ].yPosition + lambdaOne    * yCrossProduct,
    _positionBuffer[ i1 ].xPosition + lambdaSecond * xCrossProduct,
    _positionBuffer[ i1 ].yPosition + lambdaSecond * yCrossProduct,
    _positionBuffer[ i1 ].xPosition - lambdaSecond * xCrossProduct,
    _positionBuffer[ i1 ].yPosition - lambdaSecond * yCrossProduct,
    _positionBuffer[ i0 ].xPosition - lambdaOne    * xCrossProduct,
    _positionBuffer[ i0 ].yPosition - lambdaOne    * yCrossProduct
   );

}

  }


  //***************************************************************************
  //
  //***************************************************************************


else {

  fprintf(output, " <path d=\"");

  for(a=0; a<entries; a+=1) {

   i0 = lookupTour[a];

   fprintf(output, "%c%.3f %.3f ",
    a==0 ? 'M' : 'L',
    _positionBuffer[i0].xPosition,
    _positionBuffer[i0].yPosition
   );

   if( a!=0 && a!=(entries-1) && (entries%40) == 0 ){
    fprintf(output, "\n");
   }
  }


  fprintf(output, "Z\" stroke-width=\"0.25\" stroke=\"black\" fill=\"none\" />\n");

}


  //***************************************************************************
  //
  //***************************************************************************

  fprintf(output, " </g>\n");

  writeFooterSvg(output);

  if (input) {
   fclose(input);
  }

  if (output) {
   fclose(output);
  }
 }



 //****************************************************************************
 //
 //                                                                          
 //****************************************************************************

 void endProgram(char *text) {

  printf("%s\n", text);

  if (_positionBuffer) {
   free(_positionBuffer);
  }

  exit(0);
 }

