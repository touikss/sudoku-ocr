#include "../headers/matrix.h"

void Matrix_Print(Shape* mat[][16])
{
    if (mat == NULL)
        Error("NULL parameter in Matrix_Print()");
    printf("----- MATRICE -----\n");
    printf(" ");
    for (int x = 0; x < 16; x ++)
        printf(" %x", x);
    printf("\n");
    for (int i = 0; i < 16; i ++){
        printf("%x|", i);
        for (int j = 0; j < 16; j ++){
            if (mat[i][j] != NULL)
                printf("1");
                //printf("%p", mat[i][j]);
            else    
                printf(" ");
            printf("|");
        }
        printf("\n");
    }
}


int Matrix_Contains(Shape* mat[][16], Shape* shape)
{
    if (mat == NULL)
        Error("NULL parameter in Matrix_Contains()");

    for (int i = 0; i < 16; i ++){
        for (int j = 0; j < 16; j ++){
            if (mat[i][j] == shape)
                return 1;
        }
    }
    return 0;
}


void Matrix_PushRight(Shape* mat[][16], int start)
{
    if (start < 0 || start > 15)
        Error("Invalid parameter in Matrix_PushRight()");
    if (mat == NULL)
        Error("NULL parameter in Matrix_PushRight()");

    for (int i = 15; i >= 0; i --){
        for (int j = 15; j >= start + 1; j --){
            mat[i][j] = mat[i][j - 1];
        }
    }

    for (int j = 0; j < 16; j ++)
        mat[j][start] = NULL;
}


void Matrix_PushDown(Shape* mat[][16], int start)
{
    if (start < 0 || start > 15)
        Error("Invalid parameter in Matrix_PushDown()");
    if (mat == NULL)
        Error("NULL parameter in Matrix_PushDown()");

    for (int i = 15; i >= start + 1; i --){
        for (int j = 0; j < 16 ; j ++){
            mat[i][j] = mat[i - 1][j];
        }
    }
    for (int j = 0; j < 16; j ++)
        mat[start][j] = NULL;
}


void Matrix_Clear(Shape* mat[][16])
{
    if (mat == NULL)
        Error("NULL parameter in Matrix_Clear()");

    for (int i = 0; i < 16; i ++){
        for (int j = 0; j < 16 ; j ++){
            mat[i][j] = NULL;
        }
    }
}


int Matrix_Push(Shape* mat[][16], Shape* shape, int* maxX, int* maxY)
{

    if (mat == NULL || shape == NULL)   
        Error("NULL parameter in Matrix_Push()");

   // if (Matrix_Contains(mat, shape))
     //   return;


    int i = 0;
    int j = 0;
    int isRight = 0;
    int isBelow = 0;
    int found = 0;
    int x = 0;
    int y = 0;
    int lastX = -1;
    int lastY = -1;
    // find x

    for (int x = 0; x < 16; x ++){
        for (y = 0; y < 16; y ++){
            if (!mat[y][x])
                continue;
            else if (x > lastX)
                lastX = x;

            isRight = Shape_IsRight(shape, mat[y][x]);
            if (!isRight){
                if (Shape_IsLeft(shape, mat[y][x])){
                    Matrix_PushRight(mat, x);
                }
                found = 1;
                i = x;
            }
            break;
        }

        if (found)
            break;
    }


  /*  for (int x = 0; x < 16; x ++){
        int ylast = 0;
        int yfirst = 0;
        for (y = 0; y < 16; y ++){
            if (mat[y][x] != NULL){
                if (mat[yfirst][x] == NULL)
                    yfirst = y;
                ylast = y;
            }
        }

        if (!mat[ylast][x])
            continue;
        else if (x > lastX)
            lastX = x;

        isRight = Shape_IsRight(shape, mat[ylast][x]);
        if (!isRight){
            if (Shape_IsLeft(shape, mat[ylast][x])){
                Matrix_PushRight(mat, x);
            }
            found = 1;
            i = x;
            break;
        }
    }*/



    if (!found)
        i = lastX + 1;

    if (i > *maxX)
        *maxX = i;
    
    found = 0;
    // find y
    for (int y = 0; y < 16; y ++){
        int xlast = 0;
        int xfirst = 0;
        for (x = 0; x < 16; x ++){
            if (mat[y][x] != NULL){
                if (mat[y][xfirst] == NULL)
                    xfirst = x;
                xlast = x;
            }
        }

        if (!mat[y][xlast])
            continue;
        else if (y > lastY)
            lastY = y;

        isBelow = Shape_IsBelow(shape, mat[y][xlast]);
        if (!isBelow){
            if (Shape_IsAbove(shape, mat[y][xlast])){
                Matrix_PushDown(mat, y);
            }
            found = 1;
            j = y;
            break;
        }
    }

    if (!found)
        j = lastY + 1;

    if (j > *maxY)
        *maxY = j + 1;

    if (j > 15 || i > 15){
        Matrix_Clear(mat);
        printf("Info : Matrice has been cleared in Matrix_Push().\n");
        return 1;
    }
    //Error("Case detected out of the bounds in Matrix_Push()");

    if (mat[j][i] == NULL || shape->size > mat[j][i]->size)
        mat[j][i] = shape;
    return 0;
}


int Matrix_IsConsistent(ShapeTab* tab, Shape* grid, Shape* mat[][16])
{
    if (tab == NULL)
        Error("Invalid parameter in Matrix_IsConsistent()");

    ShapeTab_ComputeAverage(tab, grid);
    ShapeTab_ComputeStandardDeviation(tab, grid);

    /*if (tab->stdH > 60 || tab->stdW > 60)
        return 0;*/

  /*  if (tab->size < 81 + (grid ? 1 : 0) || tab->size > tab->size < 256 + (grid ? 1 : 0))
        return 0;*/

    int count = 0;
    int maxX = 0, maxY = 0;
    Shape* current = tab->data->next;

    while (tab->size > 0){
        if (current == grid)
            continue;
        Matrix_Push(mat, current, &maxX, &maxY);
        ShapeTab_RemoveShape(tab, current, 0);
        count += 1;
        current = tab->data->next;
    }

    Matrix_Print(mat);

    int hexa = 0;
    for (int i = 0; i < 16; i ++){
        for (int j = 0; j < 16; j ++){
            if (i < 9 && j < 9 && mat[i][j] == NULL)
                return 0;
            if ((i >= 9 || j >= 9) && mat[i][j] != NULL)
                hexa ++;
        }
    }

    return hexa == 0 || hexa == 175;
}


int Matrix_Count(Shape* mat[][16])
{
    int res = 0;
    for (int i = 0; i < 16; i ++){
        for (int j = 0; j < 16; j ++){
            if (mat[i][j])
                res += 1;
        }
    }
    return res;
}

Point FindIntersectionBtwLines(Point A, Point B, Point C, Point D) {
    Point intersection;

    double angle1 = Point_Angle(A, B);
    double angle2 = Point_Angle(C, D);

    if (angle1 == NAN)
        Error("Invalid angle in FindIntersectionBtwLines()");
        
    if (angle2 == NAN)
        Error("Invalid angle in FindIntersectionBtwLines()");

    double m1 = tan(angle1);
    double m2 = tan(angle2);

    double b1 = A.y - m1 * A.x;
    double b2 = C.y - m2 * C.x;

    intersection.x = (b2 - b1) / (m1 - m2);
    intersection.y = m1 * intersection.x + b1;

    return intersection;
}



void Matrix_FindDescriptors(Shape* mat[][16], Shape** min, Shape** max, int row, int col)
{
    if (row > 15 || col > 15)
        Error("Invalid parameter in Matrix_FindDescriptors()");

    *max = NULL;
    *min = NULL;
    Shape* fmin = NULL;

    while(*max == *min && row < 16 && col < 16){
        
        for(int i = 0; i < 16; i ++){
            Shape* current = NULL;

            if (row >= 0)
                current = mat[row][i];
            else 
                current = mat[i][col];

            if (current != NULL){
                *max = current;
                if (*min == NULL)
                    *min = current;
                if (fmin == NULL)
                    fmin = current;
            }
        }

        if (fmin == NULL)
            Error("Matrix is not valid in Matrix_FindDescriptors()");
        
        if (row >= 0)
            row = (row + 1) % 15;
        else 
            col = (col + 1) % 15;
    }

    int dx = fmin->averageX - (*min)->averageX;
    int dy = fmin->averageY - (*min)->averageY;

    if ((dx != 0 || dy != 0) && *min && *max){
        Shape_Translate(*min, dx, dy);
        Shape_Translate(*max, dx, dy);
    }
}


Point Matrix_FindIntersection(Shape* mat[][16], int row, int col)
{
    if (row < 0 || row > 15 || col < 0 || col > 15)
        Error("Invalid parameter in Matrix_GetrowInclination()");

    Shape* min1 = NULL;
    Shape* max1 = NULL;

    Shape* min2 = NULL;
    Shape* max2 = NULL;
    
    Matrix_FindDescriptors(mat, &min1, &max1, row, -1);
    Matrix_FindDescriptors(mat, &min2, &max2, -1, col);

    if (min2 == NULL || max2 == NULL || min1 == NULL || max1 == NULL)
        Error("Not enough case detected in this row or col in Matrix_FindIntersection()");
    if (min2 == max2 || min1 == max1)
        Error("Equals descriptors in Matrix_FindIntersection()");

    Point p = FindIntersectionBtwLines((Point){min1->averageX, min1->averageY}, (Point){max1->averageX, max1->averageY}, 
    (Point){min2->averageX, min2->averageY}, (Point){max2->averageX, max2->averageY});

    return p;
}


int Matrix_Height(Shape* mat[][16])
{
    int height = 0;
    for (int i = 0; i < 16; i ++){
        for (int j = 0; j < 16; j ++){
            if (mat[i][j] != NULL && i > height)
                height = i;
        }
    }
    return height + 1;
}


int Matrix_Width(Shape* mat[][16])
{
    int width = 0;
    for (int i = 0; i < 16; i ++){
        for (int j = 0; j < 16; j ++){
            if (mat[i][j] != NULL && j > width)
                width = j;
        }
    }
    return width + 1;
}



void Matrix_EstimateCase(Shape* mat[][16])
{
    int hexa = 0;
    for (int i = 0; i < 16; i ++){
        for (int j = 0; j < 16; j ++){
            if ((i >= 9 || j >= 9) && mat[i][j] != NULL){
                hexa ++;
                break;
            }
        }
        if (hexa)
            break;
    }

    int size = 9;
    if (hexa)
        size = 16;

    double h = 0;
    double w = 0;
    Shape* tcpy = NULL;

    for (int i = 0; i < size; i ++){
        for (int j = 0; j < size; j ++){
            Shape* current = mat[i][j];
            if (current != NULL){
                tcpy = current;
                h += current->height;
                w += current->width;
            }
        }
    }

    h /= Matrix_Count(mat);
    w /= Matrix_Count(mat);

    for (int i = 0; i < size; i ++){
        for (int j = 0; j < size; j ++){
            if (mat[i][j] == NULL){
                Point p = Matrix_FindIntersection(mat, i, j);
                Shape* s = Shape_Copy(tcpy);
                int dx = p.x - (int)s->averageX;
                int dy = p.y - (int)s->averageY;
                for (unsigned int i = 0; i < s->size; i ++){
                    if (s->points[i].x + dx > 0)
                        s->points[i].x += dx;
                    if (s->points[i].y + dy > 0)
                        s->points[i].y += dy;
                }
                mat[i][j] = s;
                Shape_UpdateAveragePointAndDimension(s);
            }
        }
    }   

}


