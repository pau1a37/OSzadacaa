// program simulira rezervaciju stolova u restoranu koristeći dretve

// broj dretvi (gostiju) i broj stolova se unosi s tipkovnice
// svaka dretva svake sekunde pokušava rezervirati jedan nasumični stol

// za zaštitu rezervacije koristi se lamportov algoritam (ulaz u kritični odsječak)
// dretva provjerava je li stol slobodan; ako jest, zauzima ga
// nakon pokušaja rezervacije ispisuje se stanje svih stolova

// ponavlja se dok svi stolovi ne budu zauzeti

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int *ULAZ, *BROJ, *REZ;
int brSt, brDr;
pthread_t *dretve;

// funkcija koja vraća najveći broj u nizu BROJ i nova dretva uzima taj broj
int max() {
    int najv = 0;
    for (int i = 0; i < brDr; i++) {
        if (BROJ[i] > najv) {
            najv = BROJ[i];
        }
    }
    return najv;
}

// provjerava jesu li svi stolovi zauzeti
int sve_zauzeto() {
    for (int i = 0; i < brSt; i++) {
        if (REZ[i] == -1) {
            return 0;
        }
    }
    return 1;
}

// lamportov algoritam za ulazak u kritični odsječak
void udji_u_kriticni_odsjecak(int i) {
    ULAZ[i] = 1;
    BROJ[i] = max() + 1;
    ULAZ[i] = 0;

    for (int j = 0; j < brDr; j++) {
        while (ULAZ[j]) {} // čekaj dok druga dretva bira broj
        while (BROJ[j] != 0 && (BROJ[j] < BROJ[i] || (BROJ[j] == BROJ[i] && j < i))) {
            // čekaj dok druga dretva ima prednost
        }
    }
}

// izlazak iz kritičnog odsječka
void izadji_iz_kriticnog_odsjecka(int i) {
    BROJ[i] = 0;
}

// prikaz trenutnog stanja svih stolova
void prikazi_stanje() {
    for (int i = 0; i < brSt; i++) {
        if (REZ[i] == -1) printf("-");
        else printf("%d", REZ[i] + 1); // dodaj 1 radi lakšeg čitanja
    }
    printf("\n");
}

// funkcija koju izvodi svaka dretva
void *provjeri_stol(void *arg) {
    int id = *((int *)arg);   //br dretve
    free(arg); // oslobodi memoriju za argument

    while (!sve_zauzeto()) {
        sleep(1); // čekaj 1 sekundu

        int stol = rand() % brSt; // odaberi nasumičan stol
        printf("Dretva %d: pokušavam rezervirati stol %d\n", id + 1, stol + 1);

        udji_u_kriticni_odsjecak(id); // ulaz u kritični odsječak

        if (REZ[stol] == -1) {
            REZ[stol] = id;
            printf("Dretva %d: REZERVIRALA stol %d, stanje: ", id + 1, stol + 1);
        } else {
            printf("Dretva %d: NEUSPJELA rezervacija stola %d, stanje: ", id + 1, stol + 1);
        }

        prikazi_stanje(); // ispiši stanje
        izadji_iz_kriticnog_odsjecka(id); // izlaz iz kritičnog odsječka
    }

    pthread_exit(NULL);
}

// svaka dretva pokusava rezervisati 1 slucajni stol, bira slucajni stol zatim pomocu lamportovog algoritma pokusava uci u kriticni odsjek
// u kriticnom odsjeku provjerava da li je stol slobodan, ako jeste oznava ga zauzetim a ako nije slobodan ispisuje da nije uspjela rezervacija
// dretva izlazi iz kriticnog odsjeka i proces se ponavlja dok svi stolovi na budu zauzeti
int main() {
    printf("Unesi broj dretvi: ");
    scanf("%d", &brDr);
    printf("Unesi broj stolova: ");
    scanf("%d", &brSt);

    // alokacija memorije za globalne nizove
    REZ = (int *)malloc(sizeof(int) * brSt);
    BROJ = (int *)malloc(sizeof(int) * brDr);
    ULAZ = (int *)malloc(sizeof(int) * brDr);
    dretve = (pthread_t *)malloc(sizeof(pthread_t) * brDr);

    // inicijalizacija stanja
    for (int i = 0; i < brSt; i++) REZ[i] = -1;
    for (int i = 0; i < brDr; i++) BROJ[i] = ULAZ[i] = 0;

    // stvaranje dretvi
    for (int i = 0; i < brDr; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&dretve[i], NULL, provjeri_stol, id);
    }

    // čekanje da sve dretve završe
    for (int i = 0; i < brDr; i++) {
        pthread_join(dretve[i], NULL);
    }

    // oslobađanje memorije
    free(REZ);
    free(BROJ);
    free(ULAZ);
    free(dretve);

    return 0;
}