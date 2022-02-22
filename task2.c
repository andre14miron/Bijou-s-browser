#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define INCR 3

enum color { white, black, red, green, blue, yellow };

typedef struct site_data{
    char URL[50];
    int nr_access;
    int checksum;
    int length;
    char title[50];
    char *content;
    char *code;
    enum color color_text;
    enum color color_background;
}site_data;

/* citirea datelor dintr-un fisier f si memorarea lor intr-o structura site de
tip site_data */
void read_site_data(site_data * site, FILE *f)
{
    fscanf(f, "%s", site->URL);         // citire URL
    fscanf(f, "%d", &site->length);     // citire lungimea in octeti a codului
    fscanf(f, "%d", &site->nr_access);  // citire numarul de accesari
    fscanf(f, "%d", &site->checksum);   // citire checksum

    char t[100];      // variabila folosita pentru citirea unei linii
    fgets(t, 100, f); // citeste ce a ramas din prima linie

    /* citire codul HTML */
    site->code = (char *) calloc(site->length+1,sizeof(char));
    while(fgets(t, 100, f) != NULL) 
        strcat(site->code, t);
}

/* identificarea CSS pentru un element */
void CSS(site_data *site, char *style)
{
    char c[6][7]={"white","black","red","green","blue","yellow"}; 
    char *ptext,*pback;
    int i;
    // verificare daca se specifica culoarea fundalului si aflarea ei
    pback=strstr(style,"background-color");
    if(pback!=NULL){
        pback[23]='\0';       // delimitam secventa
        for(i=0; i<6; i++)    // cautare culoare
            if(strstr(pback,c[i])!=NULL)
                site->color_background = i; 
    }

    // verificare daca se specifica culoarea textului si aflarea ei
    ptext=strstr(style,"color");
    if(ptext!=NULL && style[ptext-style-1]!='-'){
        ptext[12]='\0';       // delimitam secventa
        for(i=0; i<6; i++)    // cautare culoare
            if(strstr(ptext,c[i])!=NULL)
                site->color_text = i; 
    }
}

/* obtinere titlu, CSS si continut dupa parsarea codului HTML */ 
void parsing_html(site_data *site)
{
    /// initializam culorile textului si al fundalului
    site->color_text=black;  
    site->color_background=white;

    /// in variabila text se copiaza codul HTML pentru a se efectua parsarea 
    char *text = (char *) calloc(strlen(site->code)+1,sizeof(char));  
    strcpy(text, site->code);

    int i=0; 
    char *p=strtok(text,"<>");
    while(p){
        /// obtinere titlu 
        if(i==3) 
            strcpy(site->title,p);

        /// obtinere CSS 
        if(i==6)
            if(strlen(p)!=1)
                CSS(site,p); 

        /// obtinere continut 
        if(i==7){
            site->content = (char *) calloc(strlen(p)+1,sizeof(char));
            strcpy(site->content,p);
        }
        p=strtok(NULL,"<>");
        i++;
    }
    free(text);
} 

/* returneaza 1 daca caracterul c este litera si 0 in caz contrar */
int is_letter(char c) 
{
    if(c>='a' && c<='z') 
        return 1;
    if(c>='A' && c<='Z')   
        return 1;
    return 0;
}

/* returneaza 1 daca secventa word se afla in sirul text si 0 in caz contrar */
int find_keyword(char *word, char *text)
{
    char *p1=strstr(text,word);
    if(p1!=NULL){
        /* verificare daca secventa nu este precedata de o litera (in cazul
        in care nu este la inceput de text) */
        int k=p1-text-1;
        if(k>0 && is_letter(text[k])==1) 
            return 0;
        
        /* verificare daca secventa nu este urmata de o litera (in cazul in
        care nu este la sfarsit de text) */
        k=p1-text+strlen(word);
        if(k<strlen(text) && is_letter(text[k])==1) 
            return 0;

        return 1;
    }
    return 0;
}

/* compara lexicografic continutul a doua site-uri, iar daca sunt identice, 
criteriul va fi numarul de accesari in ordine descrecatoare */
int compare2(site_data site1, site_data site2)
{       
    if(strcmp(site1.content,site2.content)<0) return 0;
    if(strcmp(site1.content,site2.content)>0) return 1;

    /* site-urile au acelas continut, se aplica al doilea criteriu */
    if(site1.nr_access >= site2.nr_access) 
        return 0;
    return 1;
}

/* interschimba doua variabile de tip site_data */
void swap(site_data *site1, site_data *site2)
{
    site_data aux;
    aux = *site1;
    *site1 = *site2;
    *site2 = aux;
}

/* ordoneaza folosind o functie de comparare */
void sort(site_data *site, int (*compare)(site_data, site_data), int n)
{
    int i, ok;
    /* sortarea se face dupa algoritmul bubble-sort */
    do{
        ok=1;
        for(i=0; i<n-1; i++)
            if(compare(site[i],site[i+1])==1){
                swap(&site[i],&site[i+1]);
                ok=0;
            }
    }while(ok==0);
}

/* crearea noii baze de date(site2) dupa filtrarea unei baze de date(site) */
/* filtrarea consta in obtinerea site-urilor ce contin cel putin un cuvant din
sirul 'keys' */
void find_sites(char *keys, site_data *site, int n, site_data **site2, int *n2)
{
    /* vector de frecventa a site-urilor (1 daca site-ul respectiv indeplineste
    conditia) */
    int *fr; 
    fr = (int *) calloc(n,sizeof(int));
    
    /// extragem cuvintele cheie
    char *p;
    int i, nr=0;         // nr reprez. numarul de site-uri gasite dupa filtrare
    p=strtok(keys," ");
    while(p){
        for(i=0; i<n; i++)
            /* verificare daca cuvantul se gaseste in continutul unui site */
            if(find_keyword(p,site[i].content)==1 && fr[i]==0){ 
                fr[i]=1; //cuvantul a fost gasit
                nr++;
            }
        p=strtok(NULL," ");
    } 

    /* crearea noii baze de date(site2) dupa filtrare */
    (*site2) = (site_data *) calloc(nr,sizeof(site_data));
    *n2=nr;
    int j=0;
    for(i=0; i<n; i++)
        // daca un site indeplineste conditia, se adauga la noua baza de date
        if(fr[i]==1)
            (*site2)[j++]=site[i];
    
    free(fr);
}

int main()
{
    FILE *fm, *f;
    fm = fopen ( "master.txt", "r" );
    
    /// cap reprez. capacitatea curenta a vectorului de structuri
    /// n reprez. numarul site-urilor citite din master.txt
    int cap=INCR, n=0; 
    site_data *site; 
    site = (site_data *) calloc(cap,sizeof(site_data));

    /* citire din master.txt si citirea datelor din fisiere */
    char s[30];
    while ( fscanf(fm,"%s",s) != EOF ){
        f = fopen (s, "r"); 
        
        /// realoca memorie daca nu exista destula memorie
        if(n == cap-1){
            cap = cap + INCR;
            site = (site_data *) realloc(site,cap*sizeof(site_data));
        }
        
        read_site_data(&site[n], f); /// citeste datele din fisier
        fclose(f);
        parsing_html(&site[n]);      /// parsarea elementelor HTML 
        n++;
    }
    fclose(fm);
    site = (site_data *) realloc(site,n*sizeof(site_data));

    char key_words[100];
    scanf("%[^\n]%*c", key_words); /// citeste cuvintele cheie

    /* crearea noii baze de date dupa filtrare */
    site_data *site2;
    int n2=0;
    find_sites(key_words, site, n, &site2, &n2);

    /* sortarea noii baze de date */
    sort(&*site2, compare2, n2); 

    /* afisarea noii baze de date */
    int i;
    for(i=0; i<n2; i++)
       printf("%s\n",site2[i].URL);  

    /* eliberarea memoriei */
    for(i=0; i<n; i++){
        free(site[i].content);
        free(site[i].code);
    }
    free(site);
    free(site2);

    return 0;
}
