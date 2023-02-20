Crestus Adelin 336CA

Functiile de tip previous si succesor returneaza vecinii unui nod din cadrul inelului.

Ring era functia de la lab prin care trimitem mesaje in cadrul inelului, fara erori de conexiune.

ring_def: este un fel de alg inel adaptat, in care verificam daca nodurile intermediare sunt intr-un vector, si de asemenea putem adauga informatie pe parcurs

In main:

Coordonatorii citesc din fisiere si isi aloca vectorii de workeri.
In functie de ce parametru 2 primim, trimitem pe inel numarul de workeri al fiecarui coordonator.
Apoi trimitem vectorul de workeri in sine.

Practic clusters_workers este o matrice in care la linia i avem workerii coordonatorului i.

Pentru a transmite nr de workeri at cand er_com = 1(0 nu comunica direct cu 1) sau er_com = 2 (nici 0 nici 2 nu comunica cu 1) am folosit Hearbeat. At cand trimitem de la 0 spre 1 sau spre 2,
fiecare nod isi adauga nr de workeri, iar cand a ajuns la capat, acesta trimite informatia inapoi.

Apoi trimitem din fiecare coordonator vectorul workers spre ceilalti Coordonatori.

Trimitem topologia si spre workeri, si o receptionam in acestia.

Procesul 0 genereaza vectorul si calculeaza cum trb sa redistribuie calculele.

Trimitem bucati din vector spre Coordonatori, iar acestia il impart workerilor.

Workerii fac inmultirea cu 5, si transmit valorile inapoi spre coordonator, care face merge, apoi procesul 0, face merge pe toti vectorii.