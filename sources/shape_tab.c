#include "../headers/shape_tab.h"







ShapeTab* ShapeTab_Create(int capacity)
{
    if (capacity < 1)
        Error("Invalid parameter in ShapeTab_Create()");
    
    ShapeTab* shapeTab = (ShapeTab*)malloc(sizeof(ShapeTab));
    if (shapeTab == NULL)
        Error("Malloc error, unable to allocate memory in ShapeTab_Create()");
    
    Shape* shapes = (Shape*)calloc(capacity, sizeof(Shape));
    if (shapes == NULL)
        Error("Calloc error, unable to allocate memory in ShapeTab_Create()");
    
    shapeTab->capacity = capacity;
    shapeTab->size = 0;
    shapeTab->data = shapes;
    shapeTab->averageA = 0;
    shapeTab->averageW = 0;
    shapeTab->averageH = 0;
    shapeTab->stdA = 0;
    shapeTab->stdW = 0;
    shapeTab->stdH = 0;
    shapeTab->tiltedShapes = 0;
    shapeTab->squareShapes = 0;
    return shapeTab;
}


int ShapeTab_Contains(ShapeTab* tab, Shape* shape)
{
    if (tab == NULL || shape == NULL)
        Error("NULL parameter in ShapeTab_Contains()");
    
    for (Shape* current = tab->data->next; current; current = current->next)
    {
        if (current == shape)
            return 1;
    }
    
    return 0;
}


void ShapeTab_AddShape(ShapeTab* tab, Shape* shape)
{
    if (tab == NULL || shape == NULL)
        Error("NULL parameter in ShapeTab_AddShape()");
    
    if (ShapeTab_Contains(tab, shape))
        Error("Attempt to add a shape already contained in ShapeTab_AddShape()");

    if ((double)tab->size * 1.1 >= tab->capacity){
        tab->capacity += 10;
        tab->data = (Shape*)realloc(tab->data, sizeof(Shape) * tab->capacity);
        if (tab->data == NULL)
            Error("Realloc error, unable to reallocate memory in ShapeTab_AddShape()");
    }

    if (shape->isSquare)
        tab->squareShapes += 1;
    if (shape->isTilted)
        tab->tiltedShapes += 1;

    Shape* tmp = tab->data->next;
    tab->data->next = shape;
    shape->next = tmp;
    tab->size += 1;
}


void ShapeTab_RemoveShape(ShapeTab* tab, Shape* shape, int free)
{   
    if (tab == NULL || shape == NULL)
        Error("NULL parameter in ShapeTab_RemoveShape()");
    
    if (shape->isSquare)
        tab->squareShapes -= 1;
    if (shape->isTilted)
        tab->tiltedShapes -= 1;

    Shape* current = tab->data->next;
    Shape* previous = tab->data;
    for (; current && current != shape; current = current->next){
        previous = previous->next;
    }

    if (current == NULL)
        Error("Shape not found in ShapeTab_RemoveShape()");

    previous->next = current->next;
    if (current == tab->grid)
        tab->grid = NULL;    
    if (free != 0)
        Shape_Free(current);
    tab->size --;
}


void ShapeTab_Clear(ShapeTab* tab)
{
    if (tab == NULL)
        Error("NULL parameter in ShapeTab_Clear()");

    for (Shape* current = tab->data->next; current;){
        Shape* tmp = current;
        current = current->next;
        Shape_Free(tmp);
    }

    tab->squareShapes = 0;
    tab->tiltedShapes = 0;
    tab->averageA = 0;
    tab->averageW = 0;
    tab->averageH = 0;
    tab->stdA = 0;
    tab->stdW = 0;
    tab->stdH = 0;
    tab->size = 0;
    tab->data->next = NULL;
}


void ShapeTab_Free(ShapeTab* tab)
{
    if (tab == NULL)
        Error("NULL parameter in ShapeTab_Free()");
        
    Shape* tmp;
    for (Shape* current = tab->data->next; current; current = tmp)
    {
        tmp = current->next;
        Shape_Free(current);
    }
    free(tab->data);
    free(tab);
    tab = NULL;
}


void ShapeTab_RemoveAtIndex(ShapeTab* tab, int index, int free)
{
    if (tab == NULL || index < 0 || index > tab->size)
        Error("Invalid parameter in ShapeTab_RemoveAtIndex()");

    for (Shape* current = tab->data->next; current; current = current->next){
        if (index == 0)
            ShapeTab_RemoveShape(tab, current, free);
        index --;
    }  
}







Shape* ShapeTab_GetBiggestShape(ShapeTab* tab)
{   
    if (tab == NULL)
        Error("NULL parameter in ShapeTab_GetBiggestShape()");

    Shape* max = tab->data->next;
    if (max == NULL)
        Error("Tab parameter do not contains any shape in ShapeTab_GetBiggestShape()");

    for (Shape* current = tab->data->next; current; current = current->next){

         if ((double)current->size > (double)(current->height * current->width) * 0.1 && current->size > max->size)
            max = current;

        /*if ((current->height * current->width > max->height * max->width 
            && (double)current->size > (double)(current->height * current->width) * 0.1) || current->size > max->size)
            max = current;*/
    }
    return max;
}


void ShapeTab_FoundGrid(ShapeTab* tab)
{
    if (tab == NULL)
        Error("NULL parameter in ShapeTab_FoundGrid()");
    tab->grid = ShapeTab_GetBiggestShape(tab);
    if (tab->grid == NULL)
        Error("Unable to found biggest shape in ShapeTab_FoundGrid()");
}


void ShapeTab_RemoveIncludedShape(ShapeTab* tab, Shape* toCheck)
{
    if (tab == NULL || toCheck == NULL)
        Error("NULL parameter in ShapeTab_RemoveIncludedShape()");

    for(Shape* current = tab->data->next; current;){
        Shape* tmp = current;
        current = current->next;
        if (tmp != toCheck && Shape_IsIncluded(toCheck, tmp))
            ShapeTab_RemoveShape(tab, tmp, 1);
    }
}


void ShapeTab_RemoveExcludedShape(ShapeTab* tab, Shape* toCheck)
{
    if (tab == NULL || toCheck == NULL)
        Error("NULL parameter in ShapeTab_RemoveExcludedShape()");

    for(Shape* current = tab->data->next; current;){
        Shape* tmp = current;
        current = current->next;
        if (tmp != toCheck && !Shape_IsIncluded(toCheck, tmp))
            ShapeTab_RemoveShape(tab, tmp, 1);
    }
}


void ShapeTab_KeepOnlyGrid(ShapeTab* tab){
    if (tab == NULL)
        Error("NULL parameter in ShapeTab_KeepOnlyGrid()");
        
    if (tab->grid == NULL)
        ShapeTab_FoundGrid(tab);

    ShapeTab_RemoveIncludedShape(tab, tab->grid);
    ShapeTab_RemoveExcludedShape(tab, tab->grid);
}


void ShapeTab_KeepOnlySquare(ShapeTab* tab)
{
    if (tab == NULL)
        Error("NULL parameter in ShapeTab_KeepOnlySquare()");

    if (tab->grid == NULL)
        ShapeTab_FoundGrid(tab);

    for (Shape* current = tab->data->next; current;){
        Shape* tmp = current;
        current = current->next;
        if (tmp != tab->grid && (!tmp->isSquare || tmp->size < 600))
            ShapeTab_RemoveShape(tab, tmp, 1);
    }   
}


void ShapeTab_KeepOnlyVerified(ShapeTab* tab)
{
    if (tab == NULL)
        Error("NULL parameter in ShapeTab_KeepOnlyVerified()");

    if (tab->grid == NULL)
        ShapeTab_FoundGrid(tab);

    for (Shape* current = tab->data->next; current;)
    {
        Shape* tmp = current;
        current = current->next;

        double heightFactor = tab->grid->height / current->height;
        double widthFactor = tab->grid->width / current->width;

        if (heightFactor < 8 || heightFactor > 17 || widthFactor < 8 || widthFactor > 17)
            ShapeTab_RemoveShape(tab, tmp, 1);
    }
}


int ShapeTab_ContainsSimilar(ShapeTab* tab, Shape* shape)
{
    if (tab == NULL || shape == NULL)
        Error("NULL parameter in ShapeTab_ContainsSimilar()");
    
    if (tab->averageA == 0)
        ShapeTab_ComputeAverage(tab, tab->grid);

    for (Shape* current = tab->data->next; current; current = current->next)
    {
        
        if (shape->averageX > current->extremum[0] && shape->averageX < current->extremum[1]  
        && shape->averageY > current->extremum[2]  && shape->averageY < current->extremum[3]
        && Point_Distance((Point){shape->averageX, shape->averageY}, (Point){current->averageX, current->averageY}) < tab->averageW / 2)
            return 1;
    }
    return 0;
}


void ShapeTab_PasteVerified(ShapeTab* src, ShapeTab* dst)
{
    if (src == NULL || dst == NULL)
        Error("NULL parameter in ShapeTab_PasteVerified()");

    if (src->grid == NULL)
        ShapeTab_FoundGrid(src);

    for(Shape* current = src->data->next; current;){
        Shape* tmp = current;
        current = current->next;
        if (tmp->isSquare && ShapeTab_ContainsSimilar(dst, tmp) == 0 && tmp != src->grid 
        && tmp->height < src->grid->height / 6 && tmp->width < src->grid->width / 6){
            ShapeTab_RemoveShape(src, tmp, 0);
            ShapeTab_AddShape(dst, tmp);
        }
    }
}


void ShapeTab_RemoveInconsistency(ShapeTab* tab, Shape* grid, double factor)
{
    if (tab == NULL || factor < 0)
        Error("Invalid parameter in ShapeTab_RemoveInconsistency()");

    ShapeTab_ComputeAverage(tab, grid);
    ShapeTab_ComputeStandardDeviation(tab, grid);

    ShapeTab_UpdateCountSquaresAndTilted(tab);

    int tilted = tab->tiltedShapes > 10;

    for(Shape* current = tab->data->next; current;){
        Shape* tmp = current;
        current = current->next;
        if (tmp == grid)
            continue;
    
        double dw = fabs((double)tmp->width - tab->averageW) / tab->averageW * 100;
        double dh = fabs((double)tmp->height - tab->averageH) / tab->averageH * 100;
        if (dw > 10 + factor * tab->stdW || dh > 10 + factor * tab->stdH || tmp->isTilted != tilted){
            ShapeTab_RemoveShape(tab, tmp, 1);  
        }
    }
}







void ShapeTab_Print(ShapeTab* tab, Shape* shape)
{
    if (tab == NULL || shape == NULL)
        Error("NULL parameter in ()");

    if (tab->grid == NULL)
        ShapeTab_FoundGrid(tab);

    printf("Sentinel");
    for (Shape* current = tab->data->next; current; current = current->next){
        printf("->");
        if (current == tab->grid)
            printf("GRID");
        else 
            printf("%p", current);
        if(current == shape)
            printf("(CURRENT)");
    }
    printf("->NULL\n\n");
}


void ShapeTab_PrintInfos(ShapeTab* tab)
{
    if (tab == NULL)
        Error("NULL parameter in ShapeTab_PrintInfos()");

    printf("-----SHAPE TAB-----\n");
    printf("SIZE = %d\n", tab->size);
    printf("CAPACITY = %d\n", tab->capacity);
    printf("SQUARES = %d\n", tab->squareShapes);
    printf("TILTED = %d\n", tab->tiltedShapes);
    printf("AREA AVERAGE = %.2lf\n", tab->averageA);
    printf("WIDTH AVERAGE = %.2lf\n", tab->averageW);
    printf("HEIGHT AVERAGE = %.2lf\n", tab->averageH);
    printf("STANDARD AREA DEVIATION = %.2lf %%\n", tab->stdA);
    printf("STANDARD WIDTH DEVIATION = %.2lf %%\n", tab->stdW);
    printf("STANDARD HEIGHT DEVIATION = %.2lf %%\n", tab->stdH);
}


void ShapeTab_PrintShapes(ShapeTab* tab)
{
    if (tab == NULL)
        Error("NULL parameter in ShapeTab_PrintShapes()");

    for (Shape* current = tab->data->next; current; current = current->next)
    {
        Shape_PrintInfos(current);
    }
}







void ShapeTab_ComputeAverage(ShapeTab* tab, Shape* toExclude)
{
    if (tab == NULL)
        Error("NULL parameter in ShapeTab_ComputeStandardDeviation()");

    double sumA = 0;
    double sumH = 0;
    double sumW = 0;
    for(Shape* current = tab->data->next; current; current = current->next)
    {
        if (toExclude && current == toExclude)
            continue;
        sumA += (double)current->size;
        sumH += (double)current->height;
        sumW += (double)current->width;
    }
    tab->averageA = sumA / (double)tab->size;
    tab->averageH = sumH / (double)tab->size;
    tab->averageW = sumW / (double)tab->size;
}

void ShapeTab_ComputeStandardDeviation(ShapeTab* tab, Shape* toExclude)
{
    if (tab == NULL)
        Error("NULL parameter in ShapeTab_ComputeStandardDeviation()");

    double sumA = 0, sumH = 0, sumW = 0;
    for (Shape* current = tab->data->next; current; current = current->next)
    {
        if (toExclude && current == toExclude)
            continue;
        sumA += ((double)current->size - tab->averageA) * ((double)current->size - tab->averageA);
        sumH += ((double)current->height - tab->averageH) * ((double)current->height - tab->averageH);
        sumW += ((double)current->width - tab->averageW) * ((double)current->width - tab->averageW);
    }   
    tab->stdA = sqrt(sumA/(double)tab->size) * 100.0 / tab->averageA;
    tab->stdH = sqrt(sumH/(double)tab->size) * 100.0 / tab->averageH;
    tab->stdW = sqrt(sumW/(double)tab->size) * 100.0 / tab->averageW;
}

void ShapeTab_ComputeDescriptors(ShapeTab* tab)
{
    if (tab == NULL)
        Error("NULL parameter in ShapeTab_ComputeDescriptors()");

    for (Shape* current = tab->data->next; current; current = current->next){
        Shape_ComputeDescriptors(current);
    }
}


void ShapeTab_UpdateIsSquare(ShapeTab* tab)
{
    if (tab == NULL)
        Error("NULL parameter in ShapeTab_UpdateIsSquare()");

    for (Shape* current = tab->data->next; current; current = current->next){
        if (current->size > 0)
            Shape_UpdateIsSquare(current);
    }
}

void ShapeTab_UpdateCountSquaresAndTilted(ShapeTab* tab)
{
    if (tab == NULL)
        Error("NULL parameter in ShapeTab_UpdateIsSquare()");

    tab->tiltedShapes = 0;
    tab->squareShapes = 0;

    for (Shape* current = tab->data->next; current; current = current->next){
        if (current->isSquare)
            tab->squareShapes += 1;
        if (current->isTilted)
            tab->tiltedShapes += 1;
    }
}


double ShapeTab_ComputeAngle(ShapeTab* tab)
{
    if (tab == NULL)
        Error("NULL parameter in ShapeTab_ComputeAngle()");

    double angle = 0;
    if (tab->size == 0)
        return NAN;

    for (Shape* current = tab->data->next; current; current = current->next)
    {
       angle += Shape_ComputeAngle(current);
    }
    return angle / tab->size;
}


void ShapeTab_SetTilted(ShapeTab* tab)
{
    if (tab == NULL)
        Error("NULL parameter in ShapeTab_SetTilted()");

    int count = 0;
    for (Shape* current = tab->data->next; current; current = current->next){
        count += current->isTilted;
    } 

    int tilted = count > tab->size / 2;

    for (Shape* current = tab->data->next; current; current = current->next){
        current->isTilted = tilted;
    }
}
