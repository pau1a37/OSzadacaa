// program simulira dugotrajnu obradu brojeva – računa kvadrate prirodnih brojeva
// svaki kvadrat se dodaje u obrada.txt, a status.txt sadrži broj do kojeg se došlo

// signal 10 sigusr1 ispisuje trenutno stanje (koji broj se trenutno obrađuje)
// signal 2 sigterm sprema trenutno stanje u status.txt i uredno gasi program
// signal 15 sigint prekida program bez spremanja statusa – u status.txt ostaje nula

// ako se program pokrene i status.txt ima 0, znači da je prethodni prekid bio neuredan
// tada program analizira obrada.txt, uzima zadnji uneseni broj, računa mu korijen i nastavlja odatle

// svaka obrada (računanje kvadrata) traje 2 sekunde (simulacija dugog posla)
// nakon svake obrade broj se povećava, kvadrat se dodaje u obrada.txt, a status.txt se ažurira

// datoteke se otvaraju i zatvaraju po potrebi kako bi se izbjegli problemi kod prekida programa

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// funkcija prekopirana s interneta za korijen
double sqrt(double n) {
    double x = n;
    double y = 1.0;
    double e = 0.000001; // preciznost

    while (x - y > e) {
        x = (x + y) / 2;
        y = n / x;
    }

    return x;
}


// globalne varijable
int statusBr = 0;
int zavrsi = 0;

void sigusr1_handler(int sig) {
    printf("\nTrenutni broj: %d\n", statusBr);
}

void sigterm_handler(int sig) {
    printf("\nSIGTERM primljen, cuvam status i zavrsavam.\n");
    FILE *status = fopen("status.txt", "w");
    if (status) {
        fprintf(status, "%d", statusBr);
        fclose(status);
    }
    exit(0);
}

void sigint_handler(int sig) {
    printf("\nSIGINT primljen, prekid bez cuvanja statusa.\n");
    exit(0);
}

int ucitaj_status() {
    FILE *status = fopen("status.txt", "r");
    int broj = 0;
    if (status) {
        fscanf(status, "%d", &broj);
        fclose(status);
    }
    return broj;
}

int zadnji_broj_iz_obrade() {
    FILE *obrada = fopen("obrada.txt", "r");
    int broj = 0, temp;
    if (obrada) {
        while (fscanf(obrada, "%d", &temp) == 1)
            broj = temp;
        fclose(obrada);
    }
    return (int)sqrt(broj);
}

void obradi_i_upisi() {
    FILE *obrada = fopen("obrada.txt", "a");
    FILE *status = fopen("status.txt", "w");

    if (!obrada || !status) {
        perror("Greska pri otvaranju datoteka");
        exit(1);
    }

    // upis nule = obrada u toku
    fprintf(status, "0\n");
    fflush(status);

    sleep(2); // simulacija obrade

    statusBr++;
    int kvadrat = statusBr * statusBr;

    fprintf(obrada, "%d\n", kvadrat);
    printf("Dodan broj: %d (kvadrat %d)\n", statusBr, kvadrat);

    // upis novog statusa
    freopen("status.txt", "w", status);
    fprintf(status, "%d\n", statusBr);

    fclose(status);
    fclose(obrada);
    sleep(2);
}

// program u beskonacnoj petlji racuna kvadrate prirodnih brojeva, zapisuju se u obrada.txt, i pamti svako trenutno stanje u status.txt kako bi u slucaju prekida mogao nastaviti
int main() {
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigint_handler);

    statusBr = ucitaj_status();

    if (statusBr == 0) {
        statusBr = zadnji_broj_iz_obrade();
        printf("Detektiran prekid ili prvo pokretanje. Nastavljam od %d\n", statusBr);
    } else {
        printf("Ucitana vrijednost iz status.txt: %d\n", statusBr);
    }

    while (1) {
        obradi_i_upisi();
    }

    return 0;
}