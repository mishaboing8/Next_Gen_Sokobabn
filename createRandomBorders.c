#include <stdio.h>
#include "convex_hull.h"

void makeLines(int fieldLength, char field[fieldLength][fieldLength], int m, double x[], double y[], int c[]);
int createRandomBorders(int fieldLength, char field[fieldLength][fieldLength]){
    int n = 10, m, c[n];
    double x[n], y[n];

    for(int x = 0; x < fieldLength; x++)
        for(int y = 0; y < fieldLength; y++)
            field[x][y] = '0';
    
    rand_points(n, x, y, 0, fieldLength);
    m = hull(n, x, y, c);
    makeLines(fieldLength, field, m, x, y, c);

    for(int x = 0; x < fieldLength; x++){
        for(int y = 0; y < fieldLength; y++){
            printf("%c ", field[x][y] == '0' ? ' ' : '-');
        }
        printf("\n");
    }
}

void makeLines(int fieldLength, char field[fieldLength][fieldLength], int m, double x[], double y[], int c[]){
    for (int i = 0; i < m; i++) {
        double x1 = x[c[i]], y1 = y[c[i]], x2, y2;
        if (i == m - 1) {
            x2 = x[c[0]];
            y2 = y[c[0]];
        } else {
            x2 = x[c[i+1]];
            y2 = y[c[i+1]];
        }
        double xMin = x1 < x2 ? x1 : x2, xMax = x1 < x2 ? x2 : x1;
        double yMin = y1 < y2 ? y1 : y2, yMax = y1 < y2 ? y2 : y1;

        double add = 1.0 / 1000;
        //if x1 and x2 are same it is impossible to devide by 0, but on graph it is just straight vertical line
        if (x1 == x2) {
            for (double y = yMin; y < yMax; y += add)
                field[(int)x1][(int)y] = '|';
        } else{
            for (double x = xMin, y; x < xMax; x += add) {
                y = ((x - x1) * (y2 - y1) / (x2 - x1)) + y1;
                field[(int)x][(int)y] = '|';
            }
        }
    }
}