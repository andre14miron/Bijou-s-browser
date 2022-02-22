#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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

/* roteste la dreapta bitii lui x cu k pozitii */
int rotr(char x, int k)
{
    int i, n=x;
    for(i=0; i<k; i++){
        int p=0;         /// bitul cel mai nesemnificativ
        if(n%2==1) p=1;
        n=n>>1; 
        /* daca p este 1, atunci bitul cel mai semnificativ va fi 1 */
        if(p==1) 
            n=n|(1<<7);
    }
    return n;
}

/* roteste la stanga bitii lui x cu k pozitii */
int rotl(char x, int k)
{
    int i, n=x;
    for(i=0; i<k; i++){
        int p=0;        /// bitul cel mai semnificativ
        if((n>>7)%2==1) 
            p=1;
        n&= ~(1<<7);    /// elimina bitul cel mai semnificativ
        n=n<<1;
        /* daca p este 1, atunci bitul cel mai nesemnificativ va fi 1 */
        if(p==1)
            n=n|(1<<0);
    }
    return n;
}

/* calculare checksum */
int checksum(char *S)
{
    int i, csum=rotl(S[0],0);       // csum reprez. checksum
    for(i=1; i<strlen(S); i++)
        if(i%2==0)
            csum=csum^rotl(S[i],i);
        else
            csum=csum^rotr(S[i],i);

    return csum;
}

/* verifica daca un site(web) se afla in baza de date si daca da, verifica 
daca este un site sigur sau nu */
void verification_site(char *web, site_data *site, int n)
{
    int i;
    /// verifica daca site-ul citit exista in baza de date
    for(i=0; i<n; i++)
        if(strcmp(web,site[i].URL)==0) 
            break; /// a fost gasit

    /// daca a fost parcurs intreaga baza de date, atunci site-ul nu exista 
    if(i==n) 
        printf("Website not found!\n");
    else
        /// daca checksum-ul este identic cu cel oficial, at. este site sigur
        if(checksum(site[i].code)==site[i].checksum)
            printf("Website safe!\n");

        else{
            printf("Malicious website! Official key: %d. ",site[i].checksum); 
            printf("Found key: %d\n", checksum(site[i].code)); 
        } 
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

    /// input reprez. site-ul citit de la tastatura
    char input[50]; 
    while(gets(input))
        /// verificare site
        verification_site(input, site, n); 
       
    /* eliberarea memoriei */
    int i;
    for(i=0; i<n; i++){
        free(site[i].content);
        free(site[i].code);
    }
    free(site);
    
    return 0;
}
