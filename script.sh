!/bin/bash

#Functie pentru a verifica daca un sir contine caractere non-ASCII
function contineNonASCII {
    if [[ "$1" =~ [^[:ascii:]] ]]; then
        return 0
    else
        return 1
    fi
}

#Functie pentru a verifica daca un sir contine cuvinte cheie asociate fisierelor periculoase
function contineCuvinteCheie {
    cuvinteCheie=("corrupted" "dangerous" "risk" "attack" "malware" "malicious")
    for cuvant in "${cuvinteCheie[@]}"; do
        if [[ $1 == *"$cuvant"* ]]; then
            return 0
        fi
    done
    return 1
}

#Functie pentru a clasifica si afisa rezultatul
function clasificareSiAfisare {
    if [ $linii -lt 3 ] && [ $cuvinte -gt 1000 ] && [ $caractere -gt 2000 ]; then
        echo "$1: Periculos"
    elif contineNonASCII "$1" || contineCuvinteCheie "$1"; then
        echo "$1: Periculos"
    else
        echo "$1: SAFE"
    fi
}

#Verificare daca a fost furnizat un fisier ca argument
if [ $# -ne 1 ]; then
    echo "Utilizare: $0 <cale_fisier>"
    exit 1
fi

#Extragem numele fisierului din calea furnizata
numeFisier=$(basename "$1")

#Numaram liniile, cuvintele si caracterele din fisier
linii=$(wc -l < "$1")
cuvinte=$(wc -w < "$1")
caractere=$(wc -m < "$1")

#Clasificam si afisam rezultatul
clasificareSiAfisare "$numeFisier"
