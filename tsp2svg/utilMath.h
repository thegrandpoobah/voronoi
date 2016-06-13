

        /*******************************************************************************/
        /*                                                                             */
        /*******************************************************************************/

        #define PI    3.141592653589793238462643383279502884197169399375




        /*******************************************************************************/
        /*                                                                             */
        /*******************************************************************************/

        struct positionStructure
        {  
         double xPosition;
         double yPosition;
         double zPosition;
        };


        /*******************************************************************************/
        /*                                                                             */
        /*******************************************************************************/

        struct PointStructure {  
         double xPoint;
         double yPoint;
        };


        /*******************************************************************************/
        /*                                                                             */
        /*******************************************************************************/

        struct matrixStructure
        {  
         struct positionStructure origin;

         struct positionStructure xAxis;
         struct positionStructure yAxis;
         struct positionStructure zAxis;
        };





         /******************************************************************************/
         /*                                                                            */
         /*                                                                            */
         /******************************************************************************/

         double calculateDistance(
                                  double xPositionStart,
                                  double yPositionStart,
                                  double zPositionStart,
                                  double xPositionEnd,
                                  double yPositionEnd,
                                  double zPositionEnd
                                 )
         {

          xPositionStart -= xPositionEnd;
          yPositionStart -= yPositionEnd;
          zPositionStart -= zPositionEnd;

          return(sqrt(xPositionStart*xPositionStart+yPositionStart*yPositionStart+zPositionStart*zPositionStart));
         }


         //****************************************************************************
         // calculate angle                                                                          
         // dotproduct is used; length of vector should be equals 1.0 for right
         // calculation of angle                                                                            
         //****************************************************************************

         double calculateAngle(
                               double x0,
                               double y0,
                               double z0,
                               double x1,
                               double y1,
                               double z1
                              )
         {double angle, length;

          length = fabs(x0) + fabs(y0) + fabs(z0);

          if ( length!=1.0 ){

           if ( x0==0.0 ) {
                 if ( y0==0.0 ){ z0 = z0>0.0 ? 1.0 : z0<0.0 ? -1.0 : 0.0; }
            else if ( z0==0.0 ){ y0 = y0>0.0 ? 1.0 : y0<0.0 ? -1.0 : 0.0; }
           } else if ( y0 == 0.0 ) {
                 if ( x0==0.0 ){ z0 = z0>0.0 ? 1.0 : z0<0.0 ? -1.0 : 0.0; }
            else if ( z0==0.0 ){ x0 = x0>0.0 ? 1.0 : x0<0.0 ? -1.0 : 0.0; }
           }


           length = sqrt( x0*x0 + y0*y0 + z0*z0 );

           if( length == 0.0  ) {
            printf("length (almost a) zero... %24.20f (%7.3f %7.3f %7.3f)\n", length, x0, y0, z0);
           } else {
            length = 1.0/length;
            x0 *= length;
            y0 *= length;
            z0 *= length;
           }
          }

          length = fabs(x1) + fabs(y1) + fabs(z1);

          if ( length!=1.0 ){

           if ( x1==0.0 ) {
                 if ( y1==0.0 ){ z1 = z1>0.0 ? 1.0 : z1<0.0 ? -1.0 : 0.0; }
            else if ( z1==0.0 ){ y1 = y1>0.0 ? 1.0 : y1<0.0 ? -1.0 : 0.0; }
           } else if ( y1 == 0.0 ) {
                 if ( x1==0.0 ){ z1 = z1>0.0 ? 1.0 : z1<0.0 ? -1.0 : 0.0; }
            else if ( z1==0.0 ){ x1 = x1>0.0 ? 1.0 : x1<0.0 ? -1.0 : 0.0; }
           }

           length = sqrt( x1*x1 + y1*y1 + z1*z1 );

           if( length == 0.0  ) {
            printf("length (almost b) zero... %24.20f (%7.3f %7.3f %7.3f)\n", length, x1, y1, z1);
           } else {
            length = 1.0/length;
            x1 *= length;
            y1 *= length;
            z1 *= length;
           }
          }

          angle = x0*x1 + y0*y1 + z0*z1;

               if ( angle < -1.0 ){ angle = -1.0; }
          else if ( angle >  1.0 ){ angle =  1.0; }


          return angle; // dot product
         }






        /*******************************************************************************/
        /*                                                                             */
        /*                                                                             */
        /*******************************************************************************/

        double calculateIntersectionLines(
                                          double x0,
                                          double y0,
                                          double x1,
                                          double y1,
                                          double x2,
                                          double y2,
                                          double x3,
                                          double y3
                                         )
        {bool flag;
         double denominator, lambda0, lambda1;


         flag = false;
         denominator = (x1-x0)*(y3-y2)-(y1-y0)*(x3-x2);
  

         if(fabs(denominator)>0.0000000000001)
         {
          lambda0 = ((y0-y2)*(x3-x2)-(x0-x2)*(y3-y2))/denominator;
          lambda1 = ((y0-y2)*(x1-x0)-(x0-x2)*(y1-y0))/denominator;

          if(lambda0>=0.0 && lambda0<=1.0 && lambda1>=0.0 && lambda1<=1.0)
          {
           flag = true;
           // intersection.xPosition = x0+lambda0*(x1-x0);
           // intersection.yPosition = y0+lambda0*(y1-y0);
         }}



         return flag ? lambda0  : -1.0;
        }




        /*******************************************************************************/
        /*                                                                             */
        /* check ifposition is inside polygon                                          */
        /* if position is inside the polygon if the number of intersection is uneven,  */
        /* otherwise the vertex position is outside the polygon                        */
        /*                                                                             */
        /*  +--------+                                                                 */
        /*  |        |                                                                 */
        /*  |   A +--|-------------------+ x+1000.0                                    */
        /*  +--------+                                                                 */
        /*                                                                             */
        /*******************************************************************************/

        bool calculatePointInPolygon(
                                     double xPosition,
                                     double yPosition,
                                     double polygon[],
                                     int    numberOfEntries
                                    )

        {bool   flagIntersection;
         int    a, i0, i1, numberOfIntersections;
         double lambda, xEnd, angle, xMinimum, xMaximum;
         

         flagIntersection = false;


         /******************************************************************************/
         /*                                                                            */
         /******************************************************************************/

         for(a=0; !flagIntersection && a<numberOfEntries; a+=2)
         {
          if(xPosition==polygon[a] && yPosition==polygon[a+1]) flagIntersection = true;

          if(a==0 || polygon[a]<xMinimum) xMinimum = polygon[a];
          if(a==0 || polygon[a]>xMaximum) xMaximum = polygon[a];
         }


         /******************************************************************************/
         /*                                                                            */
         /******************************************************************************/

         if(!flagIntersection)
         {
          numberOfIntersections = 0;

          for(a=0; a<numberOfEntries; a+=2)
          {
           if(xPosition>=xMinimum &&
              xPosition<=xMaximum )
           {

            i0     =  a;
            i1     = (a+2)%numberOfEntries;
            xEnd   = xPosition+1000000.0; 

            lambda = calculateIntersectionLines(
                                                xPosition,
                                                yPosition,
                                                xEnd,
                                                yPosition,
                                                polygon[i0  ],
                                                polygon[i0+1],
                                                polygon[i1  ],
                                                polygon[i1+1]
                                               );


            if(lambda>=0.0 && lambda<1.0)
            {
             numberOfIntersections++;
            }

           }




          }

          /*****************************************************************************/
          /*                                                                           */
          /*****************************************************************************/

          if(numberOfIntersections%2)
          {
           flagIntersection = true;
          }
         }


         return flagIntersection;
        }





        /*******************************************************************************/
        /*                                                                             */
        /*                                                                             */
        /*******************************************************************************/

        double calculateIntersectionLine3DPolygon(
                                                  double xStart,
                                                  double yStart,
                                                  double zStart,
                                                  double xEnd,
                                                  double yEnd,
                                                  double zEnd,
                                                  double polygon[9]
                                                 )
        {double lambda, denominator;
         double a, b, c, d;
         double x0, y0, z0, x1, y1, z1, x2, y2, z2;

         x0 = polygon[0];
         y0 = polygon[1];
         z0 = polygon[2];

         x1 = polygon[3];
         y1 = polygon[4];
         z1 = polygon[5];

         x2 = polygon[6];
         y2 = polygon[7];
         z2 = polygon[8];

         a = y0*(z1-z2)+y1*(z2-z0)+y2*(z0-z1);
         b = z0*(x1-x2)+z1*(x2-x0)+z2*(x0-x1);
         c = x0*(y1-y2)+x1*(y2-y0)+x2*(y0-y1);

         d = -(x0*(y1*z2-y2*z1)+x1*(y2*z0-y0*z2)+x2*(y0*z1-y1*z0));

         /******************************************************************************/
         /*                                                                            */
         /******************************************************************************/

         denominator = a*(xStart-xEnd)+b*(yStart-yEnd)+c*(zStart-zEnd);

         /******************************************************************************/
         /* parallel                                                                   */
         /******************************************************************************/

         if(fabs(denominator)<0.00001)
         {
          lambda = -1.0;
         }

         /******************************************************************************/
         /*                                                                            */
         /******************************************************************************/

         else
         {
          lambda = (a*xStart+b*yStart+c*zStart+d)/denominator;
         }



         return lambda;
        }





        /*******************************************************************************/
        /*                                                                             */
        /*                                                                             */
        /*******************************************************************************/

        bool calculateIntersectionLinePolygon(
                                              double xStart,
                                              double yStart,
                                              double xEnd,
                                              double yEnd,
                                              double polygon[],
                                              int    numberOfEntries
                                             )
        {bool   flagIntersection;
         int    a, i0, i1;
         double lambda, lookupPolygonPositions[80];
     
         /****************************************************************************/
         /* duplicate positions                                                      */
         /****************************************************************************/

         for(a=0; a<numberOfEntries; a++) lookupPolygonPositions[a] = polygon[a]; 
         
         /****************************************************************************/
         /* check if start line is inside polygon                                    */
         /****************************************************************************/

         flagIntersection = calculatePointInPolygon(
                                                    xStart,
                                                    yStart,
                                                    lookupPolygonPositions,
                                                    numberOfEntries
                                                   );

         /****************************************************************************/
         /* check if end line is inside polygon                                      */
         /****************************************************************************/

         if(!flagIntersection)
         {
          flagIntersection = calculatePointInPolygon(
                                                     xEnd,
                                                     yEnd,
                                                     lookupPolygonPositions,
                                                     numberOfEntries
                                                    );

          /****************************************************************************/
          /* check if lines intersect                                                 */
          /****************************************************************************/

          if(!flagIntersection)
          {
           for(a=0; !flagIntersection && a<numberOfEntries; a+=2)
           {
            i0 =  (a/2);
            i1 = ((a/2)+1)%(numberOfEntries/2);

            i0 *= 2;
            i1 *= 2;

            lambda = calculateIntersectionLines(
                                                lookupPolygonPositions[i0  ],
                                                lookupPolygonPositions[i0+1],
                                                lookupPolygonPositions[i1  ],
                                                lookupPolygonPositions[i1+1],
                                                xStart,
                                                yStart,
                                                xEnd,
                                                yEnd


                                               );


            if(lambda>=0.0 && lambda<=1.0) flagIntersection = true;
          }}
         }




         return flagIntersection;
        }



        /*******************************************************************************/
        /*                                                                             */
        /*                                                                             */
        /*******************************************************************************/

        double calculateSurface2DPolygon(
                                         double polygon[],
                                         int    numberOfEntries
                                        )
        {int    a, i0, i1, indexLongestSegment;
         double lambda, angle, length, lengthMaximum, xPosition, yPosition, surface;

         if(numberOfEntries!=6){ printf("\a\n error :: number of entries should be 3\n");}

         /****************************************************************************/
         /*                                                                          */
         /****************************************************************************/

         for(a=0; a<3; a++)
         {
          i0 = a*2;
          i1 = i0+1;

          length = calculateDistance(
                                     polygon[i0],
                                     polygon[i1],
                                     0.0,
                                     polygon[(i0+2)%numberOfEntries],
                                     polygon[(i1+2)%numberOfEntries],
                                     0.0
                                    );

          if(a==0 || length>lengthMaximum)
          {
           indexLongestSegment = a;
           lengthMaximum       = length;
          }
         }

         /****************************************************************************/
         /*                                                                          */
         /****************************************************************************/

         i0 = indexLongestSegment * 2;
         i1 = (i0 + 1) % numberOfEntries;

         angle = calculateAngle(
                                polygon[(i0 + 2) % numberOfEntries] - polygon[i0],
                                polygon[(i1 + 2) % numberOfEntries] - polygon[i1],
                                0.0,
                                polygon[(i0 + 4)%numberOfEntries] - polygon[i0],
                                polygon[(i1 + 4)%numberOfEntries] - polygon[i1],
                                0.0
                               );

         lambda = angle*calculateDistance(
                                          polygon[i0],
                                          polygon[i1], 
                                          0.0,
                                          polygon[(i0+4)%numberOfEntries],
                                          polygon[(i1+4)%numberOfEntries], 
                                          0.0
                                         );

         lambda /= lengthMaximum;

         xPosition = polygon[i0]+lambda*(polygon[(i0+2)%numberOfEntries]-polygon[i0]);
         yPosition = polygon[i1]+lambda*(polygon[(i1+2)%numberOfEntries]-polygon[i1]);
         
         /****************************************************************************/
         /*                                                                          */
         /****************************************************************************/

         surface  = (calculateDistance(
                                       polygon[i0],
                                       polygon[i1],
                                       0.0,
                                       xPosition,
                                       yPosition,
                                       0.0
                                      )*
                     calculateDistance(
                                       polygon[(i0+4)%numberOfEntries],
                                       polygon[(i1+4)%numberOfEntries],
                                       0.0,
                                       xPosition,
                                       yPosition,
                                       0.0
                                      ))/2.0;


         surface += (calculateDistance(
                                       polygon[(i0+2)%numberOfEntries],
                                       polygon[(i1+2)%numberOfEntries],
                                       0.0,
                                       xPosition,
                                       yPosition,
                                       0.0
                                       )*
                    calculateDistance(
                                       polygon[(i0+4)%numberOfEntries],
                                       polygon[(i1+4)%numberOfEntries],
                                       0.0,
                                       xPosition,
                                       yPosition,
                                       0.0
                                      ))/2.0;


         /****************************************************************************/
         /*                                                                          */
         /****************************************************************************/

         return surface;
        }






        /*******************************************************************************/
        /*                                                                             */
        /*                                                                             */
        /*******************************************************************************/

        bool calculateIntersection2DPolygons(
                                             double polygonA[],
                                             int    numberOfEntriesA,
                                             double polygonB[],
                                             int    numberOfEntriesB
                                            )
        {bool   flagIntersection, flag;
         int    a, b, c, i0, i1, i2, i3, entries, numberOfIntersections;
         double xPosition, yPosition, lambda;
         double xIntersection, yIntersection;
         double xMinimumA, yMinimumA, xMaximumA, yMaximumA;
         double xMinimumB, yMinimumB, xMaximumB, yMaximumB;

         /******************************************************************************/
         /*                                                                            */
         /******************************************************************************/

         flagIntersection = false;


         /******************************************************************************/
         /* init minimum/maximum                                                       */
         /******************************************************************************/

         entries = numberOfEntriesA>numberOfEntriesB ? numberOfEntriesA : numberOfEntriesB;

         for(a=0; a<entries; a+=2)
         {
          i0 = a;
          i1 = a+1;

          if(a<numberOfEntriesA)
          {
           if(a==0 || polygonA[i0]<xMinimumA) xMinimumA = polygonA[i0];
           if(a==0 || polygonA[i1]<yMinimumA) yMinimumA = polygonA[i1];
           if(a==0 || polygonA[i0]>xMaximumA) xMaximumA = polygonA[i0];
           if(a==0 || polygonA[i1]>yMaximumA) yMaximumA = polygonA[i1];
          }

          if(a<numberOfEntriesB)
          {
           if(a==0 || polygonB[i0]<xMinimumB) xMinimumB = polygonB[i0];
           if(a==0 || polygonB[i1]<yMinimumB) yMinimumB = polygonB[i1];
           if(a==0 || polygonB[i0]>xMaximumB) xMaximumB = polygonB[i0];
           if(a==0 || polygonB[i1]>yMaximumB) yMaximumB = polygonB[i1];
          }
         }






         /******************************************************************************/
         /* check possibilty for intersection boundingboxes                            */
         /*                                                                            */
         /******************************************************************************/


         if(!(xMaximumA<xMinimumB ||
              xMinimumA>xMaximumB ||
              yMaximumA<yMinimumB ||
              yMinimumA>yMaximumB ))
         {



          /*****************************************************************************/
          /* check if polygons share the same vertice position                         */
          /*                                                                           */
          /*****************************************************************************/

          for(a=0; !flagIntersection && a<numberOfEntriesA; a+=2)
          {
           for(b=0; !flagIntersection && b<numberOfEntriesB; b+=2)
           {
            if(polygonA[a]==polygonB[b] && polygonA[a+1]==polygonB[b+1]) flagIntersection = true;
          }}

          /*****************************************************************************/
          /* check if vertex positions of polygon A are inside polygon B               */
          /*                                                                           */
          /*****************************************************************************/
 
          for(a=0; !flagIntersection && a<numberOfEntriesA; a+=2)
          {
           xPosition = polygonA[a  ];
           yPosition = polygonA[a+1];
 
           /****************************************************************************/
           /* calculate if vertex is inside polygonB                                   */
           /****************************************************************************/
 
           for(b=0, numberOfIntersections=0; b<numberOfEntriesB; b+=2)
           {
            i0 =  b;
            i1 = (b+2)%numberOfEntriesB;
 
            lambda = calculateIntersectionLines(
                                                polygonB[i0  ],
                                                polygonB[i0+1],
                                                polygonB[i1  ],
                                                polygonB[i1+1],
                                                xPosition,
                                                yPosition,
                                                xPosition+1000000.0,
                                                yPosition
                                               );

            if(lambda>=0.0 && lambda<=1.0)  numberOfIntersections++;
           }

           if(numberOfIntersections%2) flagIntersection = true;
          }


          /*****************************************************************************/
          /* check if vertex positions of polygon B are inside polygon A               */
          /*                                                                           */
          /*****************************************************************************/
 
          for(a=0; !flagIntersection && a<numberOfEntriesB; a+=2)
          {
           xPosition = polygonB[a  ];
           yPosition = polygonB[a+1];
 
           /****************************************************************************/
           /* calculate if vertex is inside polygonB                                   */
           /****************************************************************************/
 
           for(b=0, numberOfIntersections=0; b<numberOfEntriesA; b+=2)
           {
            i0 =  b;
            i1 = (b+2)%numberOfEntriesA;
 
            lambda = calculateIntersectionLines(
                                                polygonA[i0  ],
                                                polygonA[i0+1],
                                                polygonA[i1  ],
                                                polygonA[i1+1],
                                                xPosition,
                                                yPosition,
                                                xPosition+1000000.0,
                                                yPosition
                                               );

            if(lambda>=0.0 && lambda<=1.0)  numberOfIntersections++;
           }

           if(numberOfIntersections%2) flagIntersection = true;
          }

          /*****************************************************************************/
          /* check if segments polygon A and B intersect                               */
          /*                                                                           */
          /*****************************************************************************/

          for(a=0; !flagIntersection && a<numberOfEntriesA; a+=2)
          {
           i0 =  a;
           i1 = (a+2)%numberOfEntriesA;

           for(b=0; !flagIntersection && b<numberOfEntriesB; b+=2)
           {
            i2 =  b;
            i3 = (b+2)%numberOfEntriesB;

            lambda = calculateIntersectionLines(
                                                polygonA[i0  ],
                                                polygonA[i0+1],
                                                polygonA[i1  ],
                                                polygonA[i1+1],
                                                polygonB[i2  ],
                                                polygonB[i2+1],
                                                polygonB[i3  ],
                                                polygonB[i3+1]
                                               );

            if(lambda>=0.0 && lambda<=1.0) flagIntersection = true;
         }}}

         /******************************************************************************/
         /*                                                                            */
         /******************************************************************************/

         return flagIntersection;
        }







         /******************************************************************************/
         /*                                                                            */
         /*                                                                            */
         /*                                                                            */
         /*                                                                            */
         /******************************************************************************/

         struct matrixStructure * calculateLocalCoordinateSystem(
                                                                 double x0,
                                                                 double y0,
                                                                 double z0,
                                                                 double x1,
                                                                 double y1,
                                                                 double z1,
                                                                 double x2,
                                                                 double y2,
                                                                 double z2
                                                                )
         {bool   flag;
          double length, angle, lambda;
          struct matrixStructure *matrix;

          /*****************************************************************************/
          /*                                                                           */
          /*****************************************************************************/

          flag = true;



          matrix = new matrixStructure; 

          if(matrix==(struct matrixStructure *)NULL)
          {
           printf(" matrix allocation failed...\n");
           flag = false;
          }

          else 
          {
           /*****************************************************************************/
           /*                                                                           */
           /*****************************************************************************/
   
           matrix->origin.xPosition = x0;
           matrix->origin.yPosition = y0;
           matrix->origin.zPosition = z0;
   
           /*****************************************************************************/
           /*                                                                           */
           /*****************************************************************************/
     
           matrix->xAxis.xPosition = x1-x0;
           matrix->xAxis.yPosition = y1-y0;
           matrix->xAxis.zPosition = z1-z0;
 
           length = calculateDistance(
                                      matrix->xAxis.xPosition,
                                      matrix->xAxis.yPosition,
                                      matrix->xAxis.zPosition,
                                      0.0,
                                      0.0,
                                      0.0
                                     );
  
           if(length<=0.0001)
           {
            // printf("problem: length x-axis equals zero...");
            flag = false;
           }
 
 
           else
           {
            lambda                   = 1.0/length;
            matrix->xAxis.xPosition *= lambda;
            matrix->xAxis.yPosition *= lambda;
            matrix->xAxis.zPosition *= lambda;
  
            /****************************************************************************/
            /* calculate z-Axis polygon (crossproduct)                                  */
            /****************************************************************************/
   
            angle = calculateAngle(
                                   x2-x0,
                                   y2-y0,
                                   z2-z0,
                                   x1-x0,
                                   y1-y0,
                                   z1-z0
                                  );

            if(fabs(angle)>=1.0)
            {
             printf(" no appropiate vertex found... %32.30f\n", fabs(angle));
             flag = false;
            }
 
            else
            {
             matrix->zAxis.xPosition = (y2-y0)*(z1-z0)-(z2-z0)*(y1-y0);
             matrix->zAxis.yPosition = (z2-z0)*(x1-x0)-(x2-x0)*(z1-z0);
             matrix->zAxis.zPosition = (x2-x0)*(y1-y0)-(y2-y0)*(x1-x0); 
 
             length = calculateDistance(
                                        matrix->zAxis.xPosition,
                                        matrix->zAxis.yPosition,
                                        matrix->zAxis.zPosition,
                                        0.0,
                                        0.0,
                                        0.0
                                       );
            
             if(length==0.0)
             {
              // printf("length z-Axis equals to zero...");
              flag = false;
             }
                
             else
             {
              lambda                   = 1.0/length;
              matrix->zAxis.xPosition *= lambda;
              matrix->zAxis.yPosition *= lambda;
              matrix->zAxis.zPosition *= lambda;

              /************************************************************************/
              /* calculate y-Axis (crossproduct of x-Axis and z-Axis)                 */
              /************************************************************************/
    
              matrix->yAxis.xPosition = matrix->zAxis.yPosition*matrix->xAxis.zPosition-matrix->zAxis.zPosition*matrix->xAxis.yPosition;
              matrix->yAxis.yPosition = matrix->zAxis.zPosition*matrix->xAxis.xPosition-matrix->zAxis.xPosition*matrix->xAxis.zPosition;
              matrix->yAxis.zPosition = matrix->zAxis.xPosition*matrix->xAxis.yPosition-matrix->zAxis.yPosition*matrix->xAxis.xPosition; 
   
              length = calculateDistance(
                                         matrix->yAxis.xPosition,
                                         matrix->yAxis.yPosition,
                                         matrix->yAxis.zPosition,
                                         0.0,
                                         0.0,
                                         0.0
                                        );
           
              if(length==0.0)
              {
               printf("lengte y-Axis equals to zero...");
               flag = false; 
              }
 
              else
              {
               lambda                   = 1.0/length;
               matrix->yAxis.xPosition *= lambda;
               matrix->yAxis.yPosition *= lambda;
               matrix->yAxis.zPosition *= lambda;
           
          }}}}}

          /*****************************************************************************/
          /*                                                                           */
          /*****************************************************************************/

          return (flag ? matrix : (struct matrixStructure *)NULL);
         }



         /******************************************************************************/
         /*                                                                            */
         /*                                                                            */
         /*                                                                            */
         /*                                                                            */
         /******************************************************************************/

         struct positionStructure pointToLocalPosition(
                                                       struct matrixStructure *matrix,
                                                       double xPosition,
                                                       double yPosition
                                                      )
         {struct positionStructure position;

          /*****************************************************************************/
          /*                                                                           */
          /*****************************************************************************/

          if(matrix==(struct matrixStructure *)NULL)
          {
           printf("no matrix defined...\n");
          }

          /*****************************************************************************/
          /*                                                                           */
          /*****************************************************************************/

          position.xPosition = matrix->origin.xPosition+(matrix->xAxis.xPosition*xPosition+matrix->yAxis.xPosition*yPosition);
          position.yPosition = matrix->origin.yPosition+(matrix->xAxis.yPosition*xPosition+matrix->yAxis.yPosition*yPosition);
          position.zPosition = matrix->origin.zPosition+(matrix->xAxis.zPosition*xPosition+matrix->yAxis.zPosition*yPosition);

          return position;
         }



         /******************************************************************************/
         /*                                                                            */
         /*                                                                            */
         /*                                                                            */
         /*                                                                            */
         /******************************************************************************/

         struct PointStructure positionToLocalPoint(
                                                    struct matrixStructure *matrix,
                                                    double xPosition,
                                                    double yPosition,
                                                    double zPosition
                                                   )
         {double angleXAxis, angleYAxis, radius;
          struct PointStructure point;

          /*****************************************************************************/
          /*                                                                           */
          /*****************************************************************************/

          if(matrix==(struct matrixStructure *)NULL)
          {
           printf("no matrix defined...\n");
          }

          /*****************************************************************************/
          /*                                                                           */
          /*****************************************************************************/

          else
          {
           xPosition = xPosition-matrix->origin.xPosition;
           yPosition = yPosition-matrix->origin.yPosition;
           zPosition = zPosition-matrix->origin.zPosition;
 
           /****************************************************************************/
           /*                                                                          */
           /****************************************************************************/

           if(xPosition==0.0 && 
              yPosition==0.0 && 
              zPosition==0.0)
           {
            point.xPoint = 0.0;
            point.yPoint = 0.0;
           }

           /****************************************************************************/
           /*                                                                          */
           /****************************************************************************/

           else
           {
            angleXAxis = calculateAngle(
                                        matrix->xAxis.xPosition,
                                        matrix->xAxis.yPosition,
                                        matrix->xAxis.zPosition,
                                        xPosition,
                                        yPosition,
                                        zPosition
                                       );
 
            angleYAxis = calculateAngle(
                                        matrix->yAxis.xPosition,
                                        matrix->yAxis.yPosition,
                                        matrix->yAxis.zPosition,
                                        xPosition,
                                        yPosition,
                                        zPosition
                                       );

            /***************************************************************************/
            /*                                                                         */
            /***************************************************************************/
 
            angleXAxis = acos(angleXAxis)*(180.0/PI);
            angleYAxis = acos(angleYAxis)*(180.0/PI);
  
            if(angleYAxis>90) angleXAxis = 360.0-angleXAxis;
  
            radius = calculateDistance(
                                       xPosition,
                                       yPosition,
                                       zPosition,
                                       0.0,
                                       0.0,
                                       0.0
                                      );
  
            angleXAxis *= PI/180.0;
   
            point.xPoint = cos(angleXAxis)*radius;
            point.yPoint = sin(angleXAxis)*radius;
           }
          }
      

         return point;
        }









        /*******************************************************************************/
        /*                                                                             */
        /* copyright 2002 Arjan Westerdiep                                             */
        /* versie: 0.00                                                                */
        /* gewijzigd: 14-07'02                                                         */
        /*                                                                             */
        /*******************************************************************************/

        long bigEndianLong(
                          long a
                         )
        {char  bufferBigEndian[4];
         char* bufferLittleEndian = (char *)&a;

         bufferBigEndian[0] = bufferLittleEndian[3];
         bufferBigEndian[1] = bufferLittleEndian[2];
         bufferBigEndian[2] = bufferLittleEndian[1];
         bufferBigEndian[3] = bufferLittleEndian[0];

         return *((long *)&bufferBigEndian);
        }




         /******************************************************************************/
         /* copyright 2000 Arjan Westerdiep                                            */
         /* versie: 0.00                                                               */
         /* gewijzigd: 05-06'01                                                        */
         /******************************************************************************/

         double pseudo(void)
         {
          return((double)(rand()%10001)/10000.0);
         }













             

