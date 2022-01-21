#include <time.h>
#include <stdlib.h>
#include <math.h>

double randfrom(double min, double max) {
    double range = (max - min);
    double div = RAND_MAX / range;
    return min + (rand() / div);
}

void rand_points(int n, double x[], double y[], int min, int max) {
    srand(time(NULL));
    double *px = x, *py = y;
    for (int i = 0; i < n; i++) {
        *(px + i) = randfrom(min, max);
        *(py + i) = randfrom(min, max);
    }
}

void switch_point(int n, double x[], double y[], int *i_start, int *i_switch) {
    if (n == 0) return;

    double minY = y[0];
    double maxX = x[0];
    *i_start = 0;

    double maxY = y[0];
    double minX = x[0];
    *i_switch = 0;

    for (int i = 1; i < n; i++)
        if (minY >= y[i]) {
            if (minY == y[i] && maxX > x[i]) continue;

            minY = y[i];
            maxX = x[i];
            *i_start = i;
        } else if (maxY <= y[i]) {
            if (maxY == y[i] && minX < x[i]) continue;

            maxY = y[i];
            minX = x[i];
            *i_switch = i;
        }
}

int hull(int n, double x[], double y[], int c[]) {

// (1) Initialisiere das Index-Array mit c[i] = i für i = 0, . . . , n − 1
    for (int i = 0; i < n; i++)
        c[i] = i;

// (2) Für n = 1 endet die Funktion hier bereits mit Rückgabe 1.
    if (n == 1) return 1;

// (3) Bestimme mit der Funktion switch_point den Index i_start des Punktes mit kleinstem y-Wert
//  und den Index i_switch des Punktes mit größtem y-Wert.
    int i_start = 0, i_switch = 0;
    switch_point(n, x, y, &i_start, &i_switch);

// (4) Wenn i_start 6 = 0, vertausche die Punkte c[0] und c[i_start], sodass anschließend
// c[0] = i_start und c[i_start] = 0 gilt.
    if (i_start) {
        c[0] = i_start;
        c[i_start] = 0;
    }

// (5) Initialisiere m mit 1 und eine weitere int-Variable up ebenfalls mit 1
    int m = 1, up = 1;
    double dx, dy;
    double newPhi, newL;
    int currentJ, newJ;


    while (1) { // Unedliche Schleife, bis ein Ergebnis gefunden wurde
        // (6) Durchlaufe in einer (zyklischen) Schleife alle Indizes, currentJ = m, . . . , n − 1, 0.
        double phi = 10, l = -1;
        for (int j = m; j <= n; j++) {
            newJ = j % n;
            // (7) Berechne den Differenzvektor Qj = Pc[newJ] − Pc[m−1] = (dx, dy).
            dx = x[c[newJ]] - x[c[m - 1]];
            dy = y[c[newJ]] - y[c[m - 1]];

            // (8) Berechne mit den Funktionen atan2 und hypot aus math.h den Winkel
            //φj = atan2(dy,dx) ∈ [−π, π] von Qj mit der x-Achse und die Länge `newJ = hypot(dx,dy) von Qj .

            newPhi = atan2(dy, dx);
            newL = hypot(dx, dy);

            //(9) Bestimme so einen Index newJ ∈ {m, . . . , n − 1, 0} mit minimalem, nichtnegativem Winkel φj ≥ 0,
            //wenn up = 1 gilt, bzw. mit minimalem, nichtpositivem Winkel φj ≤ 0 wenn up = 0 gilt. Gibt es
            //mehrere solche Indizes, dann wähle einen von diesen mit größter Länge `newJ .
            if ((dy >= 0 && up) || (dy <= 0 && !up)) {
                if (newL > 0 && (newPhi < phi || (newPhi == phi && newL > l))) {
                    phi = newPhi;
                    l = newL;
                    currentJ = newJ;
                }
            }
        } // for

        // (10) Wenn Pc[newJ] = Pc[0] gilt, ist der Startpunkt wieder erreicht und die Funktion endet mit Rückgabe m.
        if (x[c[currentJ]] == x[c[0]] && y[c[currentJ]] == y[c[0]])
            return m;

        // (11) Wenn m 6 = newJ gilt, vertausche die Indizes c[m] und c[newJ].
        if (m != currentJ) {
            int tmp = c[m];
            c[m] = c[currentJ];
            c[currentJ] = tmp;
        }

        // (12) Wenn c[m] = i_switch erreicht ist, endet die aufsteigende Suche und es wird up = 0 gesetzt.
        if (c[m] == i_switch)
            up = 0;

        // (13) Erhöhe m um Eins und gehe zu (6).
        m++;

    }
}