Bîrleanu Teodor Matei 324 CA 


                                    README TEMA 2
                            APLICATIE CLIENT-SERVER TCP SI UDP PENTRU
                                    GESTIONAREA MESAJELOR

        Voi prezenta pe scurt implementarea pe care am folosit o pentru procesul
    pentru a implementa aplcatia client-server. Tin sa mentionez faptul ca in 
    realizarea aplicatiei am folosit scheletul laboratorului 7. Am folosit 2 
    sleep-days.
        Ca protocol, am incapsulat mesajele intr o structura de mesaj ce contine
    topic, tipul mesajului, continut, ip ul clientului udp si portul. De asemenea,
    pentru client am retinut id ul, file descriptor ul, topicurile la care este 
    abonat si un "semafor" care imi indica faptul ca un client este conectat sau nu.
        Pentru implementarea clientului am pornit de la scheletul laboratorului 7. Am 
    creat un socket tcp pentru a ma conecta la server si am trimis un mesaj de conectare
    serverului , mesajul avand in campul continut(content) id ul clientului. Pentru 
    multiplexare am folosit functia poll initalizand pentru client cele 2 variante de 
    interactiune: primesc mesaj de la tastatura sau de la server. In cazul primirii unui
    mesaj de la tastatura am impartit in mai multe cazuri: daca primesc 'exit' voi trimite
    un mesaj catre serverul avand tipul 0 si voi incheia programul in client, daca primesc
    un mesaj de subscribe mesajul trimis catre server va avea tipul 1 initializand topicul 
    iar in campul content am pus id ul clientului. Pentru cazul de unsubscribe, am trimis 
    un mesaj cu topicul id ul clientului si cu tipul 2. Daca primesc mesaj de la server,
    tratez cazul in care este exit si daca nu il afisez asa cum este cerut.
        Pentru implementarea serverului am pornit tot de la scheletul laboratorului 7.
    Pentru inceput am creat file descriptorii pentru cei 2 socketi tcp si udp. Pentru 
    multiplexare am folosit functia poll initalizand pentru server cele 3 variante de 
    interactiune : mesaj udp, tcp sau de la tastatura. In aceste  cazuri am implementat
    si functia run_server. Daca primesc mesaj de la tastura de iesire trimit fiecarui 
    client un mesaj unde campul content este reprezentat de mesajul 'exit'. Primesc o noua 
    conexiune ,adaug file descriptorul in vectorii de descriptori si primesc de la subscriber
    mesaj cu id ul urmand verificarea daca exista unul astfel evitant utilizatorii cu id duplicat.
    Daca primesc mesaj de la clientii udp construiesc mesajul structurat de mine cu ajutorul
    mesajului udp primit. Am implementat functia "parse_udp" care face aceasta operatie in care am
    tratat si cele 4 cazuri de continut ale mesajului si manipularea lor (SHORT_REAL, INT, FLOAT, STRING).
    Am tratat si cazul in care primesc mesxaj pe socketii destinati clientilor si am tratat cele 3 cazuri 
    subscribe, unsubscribe, exit. La subscribe / unsubscribe am adaugat/sters din matricea de topicuri 
    aferenta clientul topicul trimis de acesta. La exit am sters file descriptorul clientul din vectorul
    cu file descriptori si l am marcat ca fiind deconectat. De asemenea, am dezactivat algoritmul lui 
    Neagle.
        La terminarea programului am inchis file descriptorii pentru udp si tcp. Pentr tratarea erorilor
    am folosit macro ul DIE din laboratorul 7.


        
