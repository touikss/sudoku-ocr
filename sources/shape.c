#include "../headers/shape.h"


/////////////////////////////////////////////////////////////////////////////////
///////////////////////AUXILARY FUNCTIONS DEFINITION/////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


double CoordinateDistance(int x1, int y1, int x2, int y2) {
    double dx = x2 - x1;
    double dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}


double Point_Distance(Point point1, Point point2)
{
    return (CoordinateDistance(point1.x, point1.y, point2.x, point2.y));
}


double Point_Angle(Point p1, Point p2)
{
    return PointAngle(p1.x, p1.y, p2.x, p2.y);
}



double PointAngle(int x1, int y1, int x2, int y2)
{
    if (y2 - y1 == 0 && x2 - x1 == 0)
        return NAN;
    //Error("NULL vector in PointAngle()");
    //printf("ATAN2 = %lf\n", atan2(y2 - y1, x2 - x1));
    return atan2(y2 - y1, x2 - x1);
}


/////////////////////////////////////////////////////////////////////////////////
////////////////////////////FUNCTIONS DEFINITION/////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


Shape* Shape_Create(int capacity, int maxWidth, int maxHeight)
{
    if (capacity < 1 || maxHeight < 1 || maxHeight < 1)
        Error("Invalid parameter in Shape_Create()");

    Shape* shape = (Shape *)malloc(sizeof(Shape));
    if (shape == NULL)
        Error("Malloc error, unable to allocate memory in Shape_Create()");

    shape->capacity = capacity;
    shape->size = 0;
    shape->height = 0;
    shape->width = 0;
    shape->isSquare = 0;
    shape->isTilted = 0;
    shape->extremum[0] = maxWidth;
    shape->extremum[1] = 0;
    shape->extremum[2] = maxHeight;
    shape->extremum[3] = 0;
    shape->averageX = 0;
    shape->averageY = 0;

    for (int i = 0; i < 8; i ++)
    {
        shape->descriptor[i].x = 0;
        shape->descriptor[i].y = 0;
    }

    shape->points = (Point*)calloc(capacity, sizeof(Point));
    if (shape->points == NULL)
        Error("Calloc error, unable to allocate memory in Shape_Create()");

    return shape;
}


void Shape_Free(Shape* shape)
{
    if (shape == NULL)
        Error("NULL parameter in Shape_Free()");

    if (shape->points != NULL)
        free(shape->points);
    free(shape);
    shape = NULL;
}


void Shape_AddPoint(Shape* shape, unsigned int x, unsigned int y)
{
    if (shape == NULL)
        Error("NULL parameter in Shape_AddPoint");
    
    // Increase capacity if needed
    if (shape->size >= shape->capacity)
    {
        shape->capacity *= 1.5;
        Point* newPoints = (Point *)realloc(shape->points, sizeof(Point) * shape->capacity);
        if (newPoints == NULL) 
            Error("Realloc error, unable to re allocate memory in Shape_AddPoint()");
        shape->points = newPoints;
    }

    int modified = 0;
    shape->averageX = (shape->averageX * shape->size + x ) / (shape->size + 1);
    shape->averageY = (shape->averageY * shape->size + y ) / (shape->size + 1);

    Point point = {x , y};
    if (x <= shape->extremum[0]) {shape->extremum[0] = x; modified = 1; shape->descriptor[0] = point;}
    if (x >= shape->extremum[1]) {shape->extremum[1] = x; modified = 1; shape->descriptor[1] = point;}
    if (y <= shape->extremum[2]) {shape->extremum[2] = y; modified = 1; shape->descriptor[2] = point;}
    if (y >= shape->extremum[3]) {shape->extremum[3] = y; modified = 1; shape->descriptor[3] = point;}

    if (modified)
    {
        shape->height = shape->extremum[1] - shape->extremum[0];
        shape->width = shape->extremum[3] - shape->extremum[2];
    }

    shape->points[shape->size].x = x;
    shape->points[shape->size].y = y;
    shape->size++;
}


int Shape_IsIncluded(Shape* container, Shape* content)
{
    if (container == NULL || content == NULL)
        Error("NULL parameter in Shape_IsIncluded");

    if (container->extremum[0] <= content->extremum[0] &&
    container->extremum[1] >= content->extremum[1] &&
    container->extremum[2] <= content->extremum[2] &&
    container->extremum[3] >= content->extremum[3])
        return 1;

    return 0;
}


void Shape_ComputeDescriptors(Shape* shape)
{
    // Ref points
    Point refPoints[4] = {
        {shape->extremum[0], shape->extremum[2]},
        {shape->extremum[0], shape->extremum[3]},
        {shape->extremum[1], shape->extremum[2]},
        {shape->extremum[1], shape->extremum[3]}
    };

    double refDist[4];
    Point descriptors[4];

    for (int i = 0; i < 4; i ++)
    {
        refDist[i] = Point_Distance(refPoints[i], shape->points[0]);
        descriptors[i].x = shape->points[0].x;
        descriptors[i].y = shape->points[0].y;
    }

    for (size_t i = 1; i < shape->size; i ++){
        for (int j = 0; j < 4; j ++)
        {
            double dist = Point_Distance(refPoints[j], shape->points[i]);
            if (dist < refDist[j]){
                refDist[j] = dist;
                descriptors[j] = shape->points[i];
            }
        }
    }

    for (int i = 4; i < 8; i ++){
        shape->descriptor[i].x = descriptors[i - 4].x;
        shape->descriptor[i].y = descriptors[i - 4].y;
    }
}


double ScaleFactor(double height, double width)
{
    double r1 = 0;
    double r2 = 0;
    if (width != 0)
        r1 = height/ width;
    if (height != 0)
        r2 = width/height;
    if (fabs(1.0-r1) > fabs(1.0-r2)){
        return fabs(1.0-r1);
    }
    return fabs(1.0-r2);
}


void Shape_PrintInfos(Shape* shape)
{
    if (shape == NULL)
        Error("NULL paremeter in Shape_PrintInfos()");
    printf("-----SHAPE-----\n");
    printf("SIZE = %d\n", shape->size);
    printf("IS SQUARE = %s\n", shape->isSquare? "YES" : "NO");
    printf("IS TILTED = %s\n", shape->isTilted? "YES" : "NO");
    printf("CAPACITY = %d\n", shape->capacity);
    printf("AVERAGE : X = %lf, Y = %lf\n", shape->averageX, shape->averageY);
    printf("HEIGHT = %d, WIDTH = %d\n", shape->height, shape->width);
    printf("EXTREMUMS : \n");
    for (int i = 0; i < 4; i ++){
        printf("    %d - %u\n", i, shape->extremum[i]);
    }
    printf("DESCRIPTORS : \n");
    for (int i = 0; i < 8; i ++){
        printf("    %d - (%d, %d)\n", i, shape->descriptor[i].x, shape->descriptor[i].y);
    }
    printf("\n");
}


int AreEquals(double a, double b, double c, double d, double margin) {
    double average = (a + b + c + d) / 4.0;
    
    return fabs(a - average) / average <= margin
        && fabs(b - average) / average <= margin
        && fabs(c - average) / average <= margin
        && fabs(d - average) / average <= margin;
}


void Shape_UpdateIsSquare(Shape* shape)
{
    if (shape == NULL)
        Error("NULL parameter in Shape_UpdateIsSquare()");

    // Check if descriptors are already updated
    for (int i = 0; i < 8; i ++){
        if (shape->descriptor[i].x == 0 && shape->descriptor[i].x == 0){
            Shape_ComputeDescriptors(shape);
            break;
        }
    }

  /*  if (shape->descriptor[0].x == 0 && shape->descriptor[0].y == 0 && shape->descriptor[0].x == 0 && shape->descriptor[0].y == 0){
        Shape_PrintInfos(shape);
        Error("Descriptor undefined in Shape_UpdateIsSquare()");
    }*/

    double dist02 = Point_Distance(shape->descriptor[0], shape->descriptor[2]); // bas gauche bas droite
    double dist21 = Point_Distance(shape->descriptor[2], shape->descriptor[1]); // bas droite haut droite
    double dist13 = Point_Distance(shape->descriptor[1], shape->descriptor[3]); // haut droite haut gauche
    double dist30 = Point_Distance(shape->descriptor[3], shape->descriptor[0]); // bas gauche haut gauche

    double dist45 = Point_Distance(shape->descriptor[4], shape->descriptor[5]); // bas gauche bas droite
    double dist57 = Point_Distance(shape->descriptor[5], shape->descriptor[7]); // bas droite haut droite
    double dist76 = Point_Distance(shape->descriptor[7], shape->descriptor[6]); // haut droite haut gauche
    double dist64 = Point_Distance(shape->descriptor[6], shape->descriptor[4]); // bas gauche haut gauche
    
    int tiltedSquare = AreEquals(dist02, dist13, dist21, dist30, 00.13); // 1 si il est penché et que c'est un carré
    int normalSquare = AreEquals(dist45, dist57, dist64, dist76, 0.13); // 1 si il est pas penché et que c'est un carré

    if (tiltedSquare || normalSquare)
        shape->isSquare = 1;

    if (shape->height < 28 || shape->width < 28 || shape->size < 500)
        shape->isSquare = 0;

    shape->isTilted = tiltedSquare;
}


double crossProduct (int x1, int y1, int x2, int y2) {
    return ((y1 * x2) - (x1 * y2));
}


int PointSideOfSegment(Point A, Point B, Point P) {
   int abX = B.x - A.x;
   int abY = B.y - A.y;
   int apX = P.x - A.x;
   int apY = P.y - A.y;
   return ((apY * abX) - (apX * abY)) < 0;
}


int Shape_IsRight(Shape* src, Shape* cmp)
{
    if (src == NULL || cmp == NULL)
        Error("NULL parameter in Shape_IsRight()");

    if (cmp->isTilted){
        double angle = fabs(Point_Angle(cmp->descriptor[3], cmp->descriptor[1]));

        int x = cmp->descriptor[1].x + (double)cmp->descriptor[1].y / tan(angle);
        int y = cmp->descriptor[3].y + (int)((double)cmp->descriptor[3].x * tan(angle));
        return PointSideOfSegment((Point){x, 0}, (Point){0, y}, src->descriptor[0]);
    }

    if (src->descriptor[5].x > cmp->descriptor[7].x)
        return 1;
    return 0;
}


int Shape_IsLeft(Shape* src, Shape* cmp)
{
    if (src == NULL || cmp == NULL)
        Error("NULL parameter in Shape_IsLeft()");

    if (cmp->isTilted){
        double angle = fabs(Point_Angle(cmp->descriptor[3], cmp->descriptor[1]));
        int x = cmp->descriptor[2].x + (double)cmp->descriptor[2].y / tan(angle);
        int y = cmp->descriptor[0].y + (int)((double)cmp->descriptor[0].x * tan(angle));
        return !PointSideOfSegment((Point){x, 0}, (Point){0, y}, src->descriptor[3]);
    }

    if (src->descriptor[7].x < cmp->descriptor[5].x)
        return 1;
    return 0;
}


int Shape_IsAbove(Shape* src, Shape* cmp)
{
    if (src == NULL || cmp == NULL)
        Error("NULL parameter in Shape_IsAbove()");

    if (cmp->isTilted){
        double angle = fabs(Point_Angle(cmp->descriptor[2], cmp->descriptor[1]));
        int x = cmp->descriptor[2].x - (double)cmp->descriptor[2].y / tan(angle);
        int y = cmp->descriptor[1].y + (int)((double)(2000 - cmp->descriptor[1].x) * tan(angle));
        return PointSideOfSegment((Point){x, 0}, (Point){2000, y}, (Point){src->averageX, src->averageY});
    }

    if (src->descriptor[5]. y < cmp->descriptor[4].y)
        return 1;
    return 0;
}


int Shape_IsBelow(Shape* src, Shape* cmp)
{
    if (src == NULL || cmp == NULL)
        Error("NULL parameter in Shape_IsBelow()");

    if (cmp->isTilted){
        double angle = fabs(Point_Angle(cmp->descriptor[0], cmp->descriptor[3]));
        int x = cmp->descriptor[0].x - (double)cmp->descriptor[0].y / tan(angle);
        int y = cmp->descriptor[3].y + (int)((double)(2000 - cmp->descriptor[3].x) * tan(angle));
        return !PointSideOfSegment((Point){x, 0}, (Point){2000, y}, (Point){src->averageX, src->averageY});
    }

    if (src->descriptor[4].y > cmp->descriptor[5].y)
        return 1;
    return 0;
}


double Shape_ComputeAngle(Shape* shape)
{
    if (shape == NULL)
        Error("NULL parameter in Shape_ComputeAngle()");

    double shapeAngle = 0;

    if (shape->isTilted){
        shapeAngle += fabs(Point_Angle(shape->descriptor[2], shape->descriptor[1]));
        shapeAngle += fabs(Point_Angle(shape->descriptor[0], shape->descriptor[3]));
        shapeAngle += fabs(M_PI_2 - Point_Angle(shape->descriptor[0], shape->descriptor[2]));
        shapeAngle += fabs(M_PI_2 - Point_Angle(shape->descriptor[1], shape->descriptor[3]));
    }
    else {
        shapeAngle += fabs(Point_Angle(shape->descriptor[4], shape->descriptor[6]));
        shapeAngle += fabs(Point_Angle(shape->descriptor[5], shape->descriptor[7]));
        shapeAngle += fabs(M_PI_2 - Point_Angle(shape->descriptor[4], shape->descriptor[5]));
        shapeAngle += fabs(M_PI_2 - Point_Angle(shape->descriptor[6], shape->descriptor[7]));
    }
    return shapeAngle / 4;
}


Shape* Shape_Copy(Shape* tcpy){
    if (tcpy == NULL)
        Error("NULL parameter in Shape_Copy()");

    Shape* cpy = Shape_Create(tcpy->size, tcpy->width, tcpy->height);

    for (unsigned int i = 0; i < tcpy->size; i++){
        Shape_AddPoint(cpy, tcpy->points[i].x, tcpy->points[i].y);
    }

    return cpy;
}


void Shape_UpdateAveragePointAndDimension(Shape* shape){
    if (shape == NULL)
        Error("NULL parameter in Shape_UpdateAveragePointAndDimension()");

    double x = 0;
    double y = 0;

    unsigned int minx = 2000;
    unsigned int maxx = 0;
    unsigned int miny = 2000;
    unsigned int maxy = 0;


    for (unsigned int i = 0; i < shape->size; i++){
        if (shape->points[i].x > maxx)
            maxx = shape->points[i].x;
        else if (shape->points[i].x < minx)
            minx = shape->points[i].x;

        if (shape->points[i].y > maxy)
            maxy = shape->points[i].y;
        else if ( shape->points[i].y < miny)
            miny =  shape->points[i].y;

        x += shape->points[i].x;
        y += shape->points[i].y;
    }

    shape->averageX = x / shape->size;
    shape->averageY = y / shape->size;
    shape->width = maxx - minx;
    shape->height = maxy - miny;
}


void Shape_Translate(Shape* shape, int dx, int dy)
{
    if (shape == NULL)
        Error("NULL parameter in Shape_Translate()");

    for (unsigned int i = 0; i < shape->size; i ++){
        shape->points[i].x += dx;
        shape->points[i].y += dy;
    }

    Shape_UpdateAveragePointAndDimension(shape);
}