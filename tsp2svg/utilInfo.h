

        double  calculateAngle(double, double, double, double, double, double);
        double  calculateDistance(double, double, double, double, double, double);




        bool calculatePointInPolygon(double, double, double, double[], int);


        int    lezen_meshpolygoon(double,double,double);
        int    lezen_meshbestand(void);
        double  initialiseren_mesh(int,int,double,double,double);



        bool   calculateIntersection2DPolygons(double[], int, double[], int);



        double  random_getal(void);
        double  pseudo(void);
        double  intensiteit_diffuus(double,double,double,double);
        double  intensiteit_specular(double,double,int);
        double  intensiteit_totaal(double,double,double,double);

        double  initialiseren_mesh(double,double,double);
        int    lezen_meshpolygoon(double,double,double);
        int    lezen_meshbestand(void);

        void   toon_dosvoortgang(double,char[]);
        void   toon_dosheader(char[]);
        void   toon_copyright(char[],char[]);

        void   teken_cylinder(int,int,double,double,double,double,double,double,double,double);


        void schrijven_aiheader(int,FILE *);
        void schrijven_aifooter(FILE *);


