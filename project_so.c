#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#define DIM 2048
#define TXT 0644

//Structura pentru informatiile despre un fisier corupt
struct InformatiiCorupt {
    char nume[DIM];
    int estePericulos;
};

//Functie pentru verificarea si mutarea fisierelor suspecte in carantina
void verificareMalware(const char *caleFisier, const char *caleCarantina, int pipe_p) {
    if (caleCarantina == NULL) return;

    char *numeFisier = strrchr(caleFisier, '/');
    if (numeFisier != NULL) numeFisier++;

    char *caleFisierCarantina = malloc(sizeof(char) * DIM);
    if (caleFisierCarantina == NULL) return;

    sprintf(caleFisierCarantina, "%s/%s", caleCarantina, numeFisier);

    rename(caleFisier, caleFisierCarantina);

    //Trimite informatii despre fisierul mutat prin pipe catre procesul parinte
    struct InformatiiCorupt info;
    strcpy(info.nume, numeFisier);
    info.estePericulos = 1; 
    write(pipe_p, &info, sizeof(struct InformatiiCorupt));

    free(caleFisierCarantina);
}     

//Functie pentru verificarea daca un fisier este un director
bool esteDirector(const char *caleFisier) {
    struct stat statFisier;
    if (stat(caleFisier, &statFisier) == -1) {
        perror("Eroare la obtinerea informatiilor despre fisier");
        return false;
    }
    return S_ISDIR(statFisier.st_mode);
}

//Functie pentru citirea informatiilor despre un fisier
char* citireInformatiiFisier(const char *caleFisier) {
    struct stat statFisier;
    if (stat(caleFisier, &statFisier) == -1) {
        perror("Eroare la obtinerea informatiilor despre fisier");
        return NULL;
    }

    char *informatii = (char*)malloc(11 * sizeof(char));
    if (informatii == NULL) {
        perror("Eroare la alocarea memoriei");
        return NULL;
    }

    sprintf(informatii, "%c%c%c%c%c%c%c%c%c%c",
        (statFisier.st_mode & S_IRUSR) ? 'r' : '-',
        (statFisier.st_mode & S_IWUSR) ? 'w' : '-',
        (statFisier.st_mode & S_IXUSR) ? 'x' : '-',
        (statFisier.st_mode & S_IRGRP) ? 'r' : '-',
        (statFisier.st_mode & S_IWGRP) ? 'w' : '-',
        (statFisier.st_mode & S_IXGRP) ? 'x' : '-',
        (statFisier.st_mode & S_IROTH) ? 'r' : '-',
        (statFisier.st_mode & S_IWOTH) ? 'w' : '-',
        (statFisier.st_mode & S_IXOTH) ? 'x' : '-',
        (S_ISDIR(statFisier.st_mode)) ? 'd' : '-'
    );

    return informatii;
}

//Functie pentru analizarea si afisarea informatiilor despre fisierele dintr-un director
int analizareDirector(const char *caleDirector, char *text, const char *caleCarantina, int pipe_p) {
    DIR *director;
    struct dirent *intrare;
    director = opendir(caleDirector);
    if (director == NULL) {
        perror("Eroare la deschiderea directorului");
        return 0;
    }
    while ((intrare = readdir(director)) != NULL) {
        char caleFisier[DIM];
        snprintf(caleFisier, sizeof(caleFisier), "%s/%s", caleDirector, intrare->d_name);
        if (strcmp(intrare->d_name, ".") == 0 || strcmp(intrare->d_name, "..") == 0)
            continue;
        if (!esteDirector(caleFisier)) {
            char *informatii = citireInformatiiFisier(caleFisier);
            if (informatii != NULL && strstr(informatii, "---------") != NULL) {
                verificareMalware(caleFisier, caleCarantina, pipe_p);
                free(informatii);
                continue;
            }
            strcat(text, informatii);
            free(informatii);
        } else {
            analizareDirector(caleFisier, text, caleCarantina, pipe_p);
        }
    }
    closedir(director);
    return 1;
}

//Functie pentru citirea continutului unui fisier
char* citireFisier(const char *caleFisier) {
    int fd = open(caleFisier, O_RDONLY);
    if (fd == -1) {
        perror("Eroare la deschiderea fisierului");
        return NULL;
    }

    off_t DIMFisier = lseek(fd, 0, SEEK_END);
    if (DIMFisier == -1) {
        perror("Eroare la obtinerea dimensiunii fisierului");
        close(fd);
        return NULL;
    }
    lseek(fd, 0, SEEK_SET); 
    char *continut = (char*)malloc((DIMFisier + 1) * sizeof(char)); 
    if (continut == NULL) {
        perror("Eroare la alocarea memoriei");
        close(fd);
        return NULL;
    }

    ssize_t bytesCitite = read(fd, continut, DIMFisier);
    if (bytesCitite == -1) {
        perror("Eroare la citirea din fisier");
        close(fd);
        free(continut);
        return NULL;
    }
    continut[bytesCitite] = '\0';

    close(fd);
    return continut;
}

//Functie pentru verificarea existentei unui fisier
bool existaFisier(const char *caleFisier) {
    return access(caleFisier, F_OK) != -1;
}

//Functie pentru scrierea intr-un fisier
void scrieinFisier(const char *caleFisier, const char *text) {
    int fd = open(caleFisier, O_WRONLY | O_TRUNC, TXT);
    if (fd == -1) {
        perror("Eroare la deschiderea fisierului pentru scriere");
        return;
    }
    if (write(fd, text, strlen(text)) == -1) {
        perror("Eroare la scrierea in fisier");
        close(fd);
        return;
    }
    printf("Snapshot actualizat pentru %s\n", caleFisier);
    close(fd);
}

//Functie pentru crearea unui fisier
void creeazaFisier(const char *caleFisier, const char *text) {
    int fd = open(caleFisier, O_CREAT | O_WRONLY, TXT);
    if (fd == -1) {
        perror("Eroare la crearea fisierului");
        return;
    }
    printf("Snapshot creat cu succes pentru %s\n", caleFisier);
    if (write(fd, text, strlen(text)) == -1) {
        perror("Eroare la scrierea in fisier");
        close(fd);
        return;
    }
    close(fd);
}

//Functie pentru crearea unui snapshot al unui fisier
void creareSnapshot(const char *caleFisier, const char *text) {
    if (existaFisier(caleFisier)) {
        char *continutFisier = citireFisier(caleFisier);
        if (continutFisier != NULL) {
            if (strstr(continutFisier, text) == NULL || strlen(continutFisier) != strlen(text)) {
                scrieinFisier(caleFisier, text);
            } else {
                printf("Nu s-au efectuat modificari pentru %s\n", caleFisier);
            }
            free(continutFisier);
        }
    } else {
        creeazaFisier(caleFisier, text);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 14) {
        fprintf(stderr, "Utilizare: %s cale_fisier ... (-o -> director_output, -s -> carantina)\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *caleOutput = NULL;
    char *caleCarantina = NULL;
    pid_t pid;
    int status = 0;
    int pipefd[2];

    // Crearea pipe-ului
    if (pipe(pipefd) == -1) {
        perror("Eroare la crearea pipe-ului");
        return EXIT_FAILURE;
    }

    for(int i = 1; i < argc; i++) {
        if (!strcmp("-o", argv[i])) {
            if (++i == argc) break;
            caleOutput = argv[i];
            if (caleOutput[strlen(caleOutput) - 1] == '/')
                caleOutput[strlen(caleOutput) - 1] = '\0';
        }
        if (!strcmp("-s", argv[i])) {
            if (++i == argc) break;
            caleCarantina = argv[i];
            if (caleCarantina[strlen(caleCarantina) - 1] == '/')
                caleCarantina[strlen(caleCarantina) - 1] = '\0';
        }
    }

    for (int i = 1; i < argc; i++) {
        if (!strcmp("-o", argv[i]) || !strcmp("-s", argv[i])) {
            i++;
            continue;
        }

        if (argv[i][strlen(argv[i]) - 1] == '/')
            argv[i][strlen(argv[i]) - 1] = '\0';

        char text[DIM];
        strcpy(text, "Cale/Dim/Perm/INode/TimpUMod\n");

        pid = fork(); 
        if (pid == -1) {
            perror("Eroare la fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            close(pipefd[0]); 

            //Verifica directorul si trimite informatiile prin pipe catre procesul parinte
            analizareDirector(argv[i], text, caleCarantina, pipefd[1]);

            close(pipefd[1]); 
            exit(EXIT_SUCCESS);
        }
    }
    
    int pidValoare[argc];
    int valoareIesire[argc];
    
    for (int i = 1; i < argc; i++) {
        if (!strcmp("-o", argv[i]) || !strcmp("-s", argv[i])) {
            i++;
            continue;
        }
        close(pipefd[1]); 

        //Preia informatiile trimise de procesul fiu si decide cum sa gestioneze fiecare fisier
        struct InformatiiCorupt info;
        while (read(pipefd[0], &info, sizeof(struct InformatiiCorupt)) > 0) {
            if (info.estePericulos) {
                printf("Fisierul \"%s\" a fost mutat in carantina.\n", info.nume);
            } else {
                printf("Fisierul \"%s\" este sigur.\n", info.nume);
            }
        }

        close(pipefd[0]); 
        waitpid(pid, &status, 0); 

        if (WIFEXITED(status)) valoareIesire[i - 1] = WEXITSTATUS(status);
    }

    printf("\n");

    for (int i = 1; i < argc; i++) {
        if (!strcmp("-o", argv[i]) || !strcmp("-s", argv[i])) {
            i++;
            continue;
        }
        printf("Procesul cu PID-ul %d s-a incheiat cu codul %d\n", pidValoare[i - 1], valoareIesire[i - 1]);
    }

    return EXIT_SUCCESS;
}
