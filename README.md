# Tema1-APD
Generare paralela de fractali folosind multimile Mandelbrot si Julie

Realizata timp de 4 ore.

Detalii implementare:

Algoritms Julia si Mandelbrot au fost paralelizati la fel, prin paralelizarea for-ului
intern si a transformarii rezultatului din coordonate matematice in coordonate ecran, astfel
am impartit egal coordonatele pentru a nu avea coliziuni intre thread-uri, am folosit o bariera
Dupa iesirea din for-ul intern pentru a nu avea coliziuni.

Pentr-u a rula ambii algoritmi pe aceleasi thread-uri am fauct o functie ajutatoare "run_alg()"
care apeleaza cei doi algoritmi si scrie rezultatul in fisierul de iesire.
Citirea si alocarea tablouluicu rezultat se face in main();
