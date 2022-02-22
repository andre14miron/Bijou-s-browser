#include <menu.h>
#include <string.h>
#include <stdlib.h> 
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
        pback[23]='\0';        // delimitam secventa
        for(i=0; i<6; i++)     // cautare culoare
            if(strstr(pback,c[i])!=NULL)
                site->color_background = i; 
    }

    // verificare daca se specifica culoarea textului si aflarea ei
    ptext=strstr(style,"color");
    if(ptext!=NULL && style[ptext-style-1]!='-'){
        ptext[12]='\0';        // delimitam secventa
        for(i=0; i<6; i++)     // cautare culoare
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

/* functia returneaza 0 daca numarul de accesari a primului site este >= decat
al doilea site si 1 in caz contrar */
int compare3 (site_data site1, site_data site2)
{
    if(site1.nr_access >= site2.nr_access) return 0;
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

/* ordoneaza folosind o functie de sortare */
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

/* verifica daca se citeste o secventa, statutul fiind aflat din variabila
'ok_seq' si o memoreaza in variabila 'sequence' */
void read_sequence(char *word, char *sequence,int *ok_seq)
{
    /* inceput de secventa */
    if(word[strlen(word)-1]=='"'){
            strcat(sequence," ");
            strcat(sequence,word);
            int n = strlen(sequence);
            sequence[n-1]='\0';       // elimina '"' din finalul cuvantului
            *ok_seq=-1;               // s-a terminat cautarea secventei
    }
    /* final de secventa */
    else if(word[0]=='"'){
            strcpy(sequence,word+1);  // copiaza primul cuvant fara '"'
            *ok_seq=1;                // incepe cautarea secventei
        }
    /* cuvant cuprins intr-o secventa */
    else if(*ok_seq==1){  
        strcat(sequence," ");
        strcat(sequence,word);
    }
}

/* crearea noii baze de date(site2) dupa filtrarea unei baze de date(site) */
/* filtrarea consta in obtinerea site-urilor ce contin cel putin un cuvant sau 
o secventa, dar sa nu contina cuvintele/secventele precedate de '-' din keys */
void find_sites(char *keys, site_data *site, int n, site_data **site2, int *n2)
{
    /* vector de frecventa a site-urilor (1 daca site-ul respectiv indeplineste
    conditiile) */
    int *fr; 
    fr = (int *) calloc(n,sizeof(int));
    
    /* extragem cuvintele cheie */
    char *p, *sequence; // sequence reprez. o secventa(cuvinte intre ghilimele)
    sequence =(char *) calloc(strlen(keys)+1,sizeof(char)); 
    int i, nr=0;        // nr reprez. numarul de site-uri gasite dupa filtrare
    int ok_seq=0;       // ok_seq reprez. statutul gasirii unei secvente
    char *word;         // word reprez. un cuvant/secventa din text
    
    p=strtok(keys," ");
    while(p){
        word = (char *) calloc(strlen(p)+1,sizeof(char));
        strcpy(word,p);
        read_sequence(word,sequence,&ok_seq); 
        if(ok_seq==-1){
            word = (char *) realloc(word,strlen(sequence)+1);
            strcpy(word,sequence);
            ok_seq=0;
        }

        /* verificare cuvant/secventa cat timp nu s-a gasit o secventa 
        neterminata */
        if(ok_seq==0)
            for(i=0; i<n; i++){ 
                /* verificare daca cuvantul se gaseste in continutul unui site */
                if(find_keyword(word,site[i].content)==1 && word[0]!='-' && fr[i]==0){
                    fr[i]=1; //cuvantul a fost gasit
                    nr++;
                }
                /* verificare daca site-urile nu contin cuvantul precedat de 
                '-' */
                if(find_keyword(word+1,site[i].content)==1 && word[0]=='-'){
                    if(fr[i]!=0) 
                        nr--;
                    fr[i]=-1;
                }
            }
        p=strtok(NULL," ");
        free(word);
    } 
    free(sequence);

    /* crearea noii baze de date(site2) dupa filtrare */
    (*site2) = (site_data *) calloc(nr,sizeof(site_data));
    *n2=nr;
    int j=0;
    for(i=0;i<n;i++)
        // daca un site indeplineste conditia, se adauga la noua baza de date
        if(fr[i]==1)
            (*site2)[j++]=site[i];
    
    free(fr);
}

/* utilizatorul tasteaza ce vrea sa caute si cum doreste sa fie cautarea,
salvat in variabila text, respectiv search_bar */
void C(char *search_type, char *text, WINDOW *win_search)
{
	/* citeste pana se tasteaza 'C' sau 'Q' pentru iesire*/
	int c = getch();
        while( c != 'C' && c!='Q')
        	c = getch();
	
	if(c=='Q') {
		*search_type='Q';
		return;
	}
	
	/* obtinere dimensiunile ecranului */
        int yMax, xMax;
        getmaxyx(stdscr, yMax, xMax);
	
	/* creare fereastra pentru cautare */
	win_search = newwin(3, xMax-12, 2, 5);
	box(win_search, 0, 0);
	refresh();
	wrefresh(win_search);
		
	/* afisarea legendei pentru a informa utilizatorul sa apese enter 
	pentru a incepe cautarea*/
	move(yMax-2,0);
	printw("                                   ");
	move(yMax-1,0);
	printw("Press Enter when you want to search");	
	
	/* citirea textului pe care utilizatorul doreste sa il caute si
	afisarea acestuia */
	c=' ';
	int i=0;
	while(c!='\n'){
		mvwprintw(win_search, 1,i+1,"%c",c);
		// textul se salveaza in  variabila text
		text[i++]=c; 
		refresh();
		wrefresh(win_search);
		c = getch();
	}
	text[i]='\0';
	
	/* afisarea legendei pentru precizare tipului de cautare*/
	move(yMax-3,0);
        printw("Press 'S' for a simple search");
        move(yMax-2, 0);
        printw("Press 'A' for an advanced search");
	move(yMax-1, 0);
	printw("Press 'Q' to quit                       ");

	/* citirea tipului de cautare dorit sau 'Q' pentru iesire*/
	*search_type = getch();
	while(*search_type != 'A' && *search_type != 'S' && *search_type != 'Q')
		*search_type = getch();
	if(*search_type == 'Q') return;

	refresh();
	wrefresh(win_search);

}

/* decodifica culorile */
int colors(enum color c)
{
	switch(c)
	{case 0:
	return 7; // white
	case 1:
	return 0; // black
	case 2:  
	return 1; // red
	case 3:
	return 2; // green
	case 4:
	return 4; // blue
	case 5:
	return 3; // yellow
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
        	parsing_html(&site[n]); /// parsarea elementelor HTML 
        	n++;
    	}
    	fclose(fm);
    	site = (site_data *) realloc(site,n*sizeof(site_data));


	/* initializere ncurses */
	initscr();  
 	cbreak(); 
 	noecho();
	start_color(); 

	/* obtinere dimensiunile ecranului */
	int yMax, xMax;
	getmaxyx(stdscr, yMax, xMax);
	

	ITEM **items; 
 	int c; 
 	MENU *menu; 
 	WINDOW *win_menu, *win_search; 
 	int i; 

 	/* afisare numele motorului de cautare */
	move(0,xMax/2-9);	
	printw("BROWSERUL LUI BIJU");

	/* afisare legenda pentru a incepe cautarea */
	move(yMax-2,0);
	printw("Press 'C' to type");
	move(yMax-1,0);
	printw("Press 'Q' to quit"); 
	
	keypad(stdscr, TRUE); 

	char search_type, t[100], text[100];
	
	/* citirea textului si tipului de cautare */
	C(&search_type, t, win_search);
	strcpy(text,t);

	if(search_type=='Q'){
		for(i=0; i<n; i++){
        		free(site[i].content);
        		free(site[i].code);
    		}
    		free(site); 
		endwin();
		return 0;
	}

	/* realizarea cautarii si salvarea acesteia in noua baza de date*/
	site_data *site2;
    	int n2=0;
	find_sites(t, site, n, &site2, &n2);
	if(search_type == 'S')
		sort(&*site2, compare2, n2); 
	else 
		sort(&*site2, compare3, n2); 
 	
	/* creare obiecte ale meniului */  
 	items = (ITEM **)calloc(3*n2+1, sizeof(ITEM *)); 
	for(i = 0; i < 3*n2; i=i+3){
		items[i] = new_item(site2[i/3].title,"");
		items[i+1] = new_item(site2[i/3].URL,"");
		items[i+2] = new_item(" ","");
	} 
 
	/* creare meniu */ 
 	menu = new_menu((ITEM **)items); 
 	
	/* crearea fereastra care sa fie asociata cu meniul */
 	win_menu = newwin(3*n2+7, 50, 5, 4); 
 	keypad(win_menu, TRUE); 
 

 	/* setare fereastra principala si fereastra secundara */ 
 	set_menu_win(menu, win_menu); 
 	set_menu_sub(menu, derwin(win_menu, 3*n2+3, 48, 4, 1)); 
 	
	/* setare marcajul meniului pe string-ul " * " */ 
 	set_menu_mark(menu, " * "); 
 	
	/* setare bordura la fereastra principala si afisare textul cautat*/ 
 	box(win_menu, 0, 0); 
 	mvwprintw(win_menu, 1, 2,"%s",text); 
 	mvwaddch(win_menu, 2, 0, ACS_LTEE); 
 	mvwhline(win_menu, 2, 1, ACS_HLINE, 48); 
 	mvwaddch(win_menu, 2, 49, ACS_RTEE); 
 
 	/* afisare meniu */ 
 	post_menu(menu); 
 	wrefresh(win_menu);
	
	/* afisare legenda pentru selectarea unui site*/
	move(yMax-3, 0);
        printw("                                 "); 
	move(yMax-2, 0);
	printw("Press 'Enter' to select a site   "); 
	refresh();
 	
	int poz=1;
	while((c = wgetch(win_menu)) != 'Q') 
 	{ 
		switch(c) 
 		{ 
			case KEY_DOWN:
			if(poz!=3*n2-2){
 				menu_driver(menu, REQ_DOWN_ITEM);
				menu_driver(menu, REQ_DOWN_ITEM);
				menu_driver(menu, REQ_DOWN_ITEM);
				poz=poz+3;
			}
			else{
				menu_driver(menu, REQ_DOWN_ITEM);
				menu_driver(menu, REQ_UP_ITEM);
			} 
 			break; 
 		
			case KEY_UP:
			if(poz!=1){ 
 				menu_driver(menu, REQ_UP_ITEM); 
				menu_driver(menu, REQ_UP_ITEM);
				menu_driver(menu, REQ_UP_ITEM);
				poz=poz-3;
			}
 			break; 
		}

		if(c == '\n'){
			// creare fereastra pentru afisarea unui site
			WINDOW * html = newwin(yMax, xMax, 0, 0);
			
			/* afisare titlu cu bold */
			wattron(html,A_BOLD); 
			mvwprintw(html,0,0,site2[poz/3].title);
			printw("%s", site2[poz/3].title);
	 		wattroff(html,A_BOLD);
		
			/* afisare text conform specificatiilor de stil*/
			init_pair(1,colors(site2[poz/3].color_text),colors(site2[poz/3].color_background));
			wattron(html,COLOR_PAIR(1)); 
			mvwprintw(html,1,0,site2[poz/3].content);
			wattroff(html,COLOR_PAIR(1));
		
			refresh();
			wrefresh(html);
		
			/* afisare legenda */
			move(yMax-2, 0);
			printw("Press 'B' to return to the menu");
			move(yMax-1, 0);
			printw("Press 'Q' to quit");
		
			/* citeste pana cand se introduce o tasta valida */
			c = getch();
			int ok=0;
			while(c!='B' && ok==0)
 				if(c=='Q')
					ok=-1;
				else c = getch();
			if(ok==-1) break;
		
			/* stergere site */
			move(0,0);
			for(i=0;i<strlen(site2[poz/3].title);i++)
				printw(" ");
			move(1,0);
			int x=1;
			for(i=0;i<strlen(site2[poz/3].content);i++)
				if(site2[poz/3].content[i]=='\n'){
					x++;
					move(x,0);
				}
				else printw(" ");
		
			delwin(html);
			refresh();
 	
			/* reafisare meniu */ 
 			box(win_menu, 0, 0); 
			mvwprintw(win_menu, 1, 2,"%s",text); 
 			mvwaddch(win_menu, 2, 0, ACS_LTEE); 
 			mvwhline(win_menu, 2, 1, ACS_HLINE, 48); 
 			mvwaddch(win_menu, 2, 49, ACS_RTEE);  
 			post_menu(menu); 
 			wrefresh(win_menu);

			/* afisare legenda */
	  		move(yMax-2, 0);
        		printw("Press 'Enter' to select a site ");
			move(yMax-1, 0);
			printw("Press 'Q' to quit");

			refresh(); 

		}

		if(c == 'Q')
			break;
 	}	 
 
	/* eliberare memorie */ 
 	unpost_menu(menu); 
 	free_menu(menu); 
 	for(i = 0; i < 3*n2; ++i) 
 		free_item(items[i]); 

    	for(i=0; i<n; i++){
        	free(site[i].content);
        	free(site[i].code);
    	}
    	free(site); 
	free(site2);

	endwin();

} 
