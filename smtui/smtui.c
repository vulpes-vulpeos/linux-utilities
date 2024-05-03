// cd /media/internal_hdd/Development/c/smtui
// gcc smtui.c -o smtui -lncursesw -lpulse

#include <ctype.h>
#include <pulse/context.h>
#include <pulse/introspect.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <stdint.h>

#include <pulse/pulseaudio.h>
#include <pulse/def.h>
#include <alsa/asoundlib.h>

#include <ncursesw/ncurses.h>

#define KEY_ESC 27

struct pa_port {
	uint32_t   card_index;
    uint32_t   sink_index;
	char      *card_name;
    char      *sink_name;
	char      *port_name;
    char      *aport_name;
    int        port_index;
	char      *port_desc;
	uint32_t   pprof_ttl;
	char     **port_prof;
	int        is_active;
} typedef pa_port;

struct pa_sink {
    uint32_t   sink_index;
    char      *sink_name;
    uint32_t   card_index;
} typedef pa_sink;

struct pa_card {
    uint32_t   index;
    char      *act_prof;
} typedef pa_card;

struct pa_data {
	int      cards_ttl;
	int      ports_ttl;
    int      sinks_ttl;
    int      cards_ctr;
    int      ports_ctr;
    int      sinks_ctr;
	int      def_sink_ind;
    char    *def_sink_act_port;
    pa_card *cards;
    pa_sink *sinks;
	pa_port *ports;
} typedef pa_data;

struct ncwin_par {
	WINDOW   *win;
	int      l;
	int      c;
	int      w;
	int      h;
	int      foc;
	int      sel;
	int      max_sel;
} typedef ncwin_par;

/////////////////////////
///   Ncurses block   /// 
/////////////////////////
void draw_pwin(ncwin_par *pwin, ncwin_par *lwin,pa_data *data){
	werase(pwin->win);
    wbkgd(pwin->win, COLOR_PAIR(2));
    // Window border
    if (pwin->foc){
        wattron(pwin->win, COLOR_PAIR(6));
    };
    box(pwin->win, 0, 0);
    wattroff(pwin->win, COLOR_PAIR(6));

    // List
    int list_h = pwin->h-2;
    int start_ind = 0, col_pair = 5;
     // Scrolling math
    if (data->ports[lwin->sel-1].pprof_ttl > list_h){
        // Forward scrolling
        if (pwin->sel > list_h){
            start_ind = pwin->sel - list_h;
        };
        // Backward scrolling
        if (pwin->max_sel > pwin->sel && (pwin->max_sel - pwin->sel) == list_h){
            --pwin->max_sel;
        };
        if (pwin->max_sel > pwin->sel && (pwin->max_sel - pwin->sel) < list_h &&
            (pwin->max_sel - list_h) >= 0 ){
            start_ind = pwin->max_sel - list_h;
        };
    };
    for (int i = start_ind; i < ((data->ports[lwin->sel-1].pprof_ttl > list_h) ? list_h+start_ind : data->ports[lwin->sel-1].pprof_ttl); ++i){
    	// Highlighting selected port
        if (i+1 == pwin->sel){ 
            if (!(pwin->foc)){
                // Unfocused selection highlight color
                col_pair = 6;
            };
            wattron(pwin->win, COLOR_PAIR(col_pair));
            mvwhline(pwin->win,i+1-start_ind, 2, ' ', COLS-25);
        };
        mvwprintw(pwin->win, i+1-start_ind, 2, "%s",
        	data->ports[lwin->sel-1].port_prof[i]);
        wattroff(pwin->win, COLOR_PAIR(col_pair));
    };
     // Drawing arrows if number of devices is bigger then list height
    if (data->ports[lwin->sel-1].pprof_ttl > list_h){
        if (start_ind != data->ports[lwin->sel-1].pprof_ttl - list_h){
        	mvwprintw(pwin->win, pwin->h-2, pwin->w-2, "▼");
        };
        if (start_ind != 0){
        	mvwprintw(pwin->win, 1, pwin->w-2, "▲");
        };
    };
    wrefresh(pwin->win);
}

void draw_lwin(ncwin_par *lwin, pa_data *data){
	werase(lwin->win);
    wbkgd(lwin->win, COLOR_PAIR(2));
    // Highlighting window border
    if (lwin->foc){
        wattron(lwin->win, COLOR_PAIR(6));
    };
    box(lwin->win, 0, 0);
    wattroff(lwin->win, COLOR_PAIR(6));

    // List
    int list_h = (lwin->h-2)/2;
    int start_ind = 0, col_pair = 5;
    // Scrolling math
    if (data->ports_ttl > list_h){
        // Forward scrolling
        if (lwin->sel > list_h){
            start_ind = lwin->sel - list_h;
        };
        // Backward scrolling
        if (lwin->max_sel > lwin->sel && (lwin->max_sel - lwin->sel) == list_h){
            --lwin->max_sel;
        };
        if (lwin->max_sel > lwin->sel && (lwin->max_sel - lwin->sel) < list_h &&
            (lwin->max_sel - list_h) >= 0 ){
            start_ind = lwin->max_sel - list_h;
        };
    };
    for (int i = 0, j = start_ind; j < ((data->ports_ttl > list_h) ? list_h+start_ind : data->ports_ttl); i+=2, ++j){
    	// Highlighting selected port
        if (j+1 == lwin->sel){ 
            if (!(lwin->foc)){
                // Unfocused selection highlight color
                col_pair = 6;
            };
            wattron(lwin->win, COLOR_PAIR(col_pair));
            mvwhline(lwin->win,i+1, 2, ' ', COLS-25);
            mvwhline(lwin->win,i+2, 2, ' ', COLS-25);
        };
        mvwprintw(lwin->win, i+1, 2, "(%c) %s", 
        	(data->ports[j].is_active) ? '*' : ' ', data->ports[j].port_desc);
        mvwprintw(lwin->win, i+2, 2, "    [%s]", data->ports[j].card_name);
        wattroff(lwin->win, COLOR_PAIR(col_pair));
    };

    // Drawing scrollbar if number of devices is bigger then list height
    if (data->ports_ttl > list_h){
        int sbar_h = 1, sbar_ind = 0;
        if(lwin->sel == 1){ sbar_ind = 0;
        } else if (lwin->sel == (data->ports_ttl)){
            sbar_ind = (list_h-sbar_h)*2;
        } else { sbar_ind = ((lwin->sel)-1)/2; };

        mvwvline(lwin->win, 1, lwin->w-2, ACS_BOARD, list_h*2);
        wattron(lwin->win, COLOR_PAIR(3));
        mvwvline(lwin->win, 1+sbar_ind, lwin->w-2, ' ', sbar_h*2);
        wattroff(lwin->win, COLOR_PAIR(3));
    };

    wrefresh(lwin->win);
}

void draw_bwin(ncwin_par *bwin, char *btns_arr[]){
	werase(bwin->win);
    wbkgd(bwin->win, COLOR_PAIR(2));
    // Printing buttons
    int row = 0;
    for (int i = 0; i < 3; ++i) {
        // Highlighting selected button
        if (i+1 == bwin->sel && bwin->foc){
            wattron(bwin->win, COLOR_PAIR(5));
        };
        
        if (i == 1){row = bwin->h-3;};
        mvwprintw(bwin->win, row, 1, "%s", btns_arr[i]);
        wattroff(bwin->win, COLOR_PAIR(5));
        ++row;
    };

    wrefresh(bwin->win);
}

void draw_mwin(ncwin_par *mwin, char *app_name){
	wbkgd(stdscr, COLOR_PAIR(1));
    wrefresh(stdscr);

    // Shadow
    werase(stdscr);
    wattron(stdscr, COLOR_PAIR(3));
    mvwvline(stdscr, 2, mwin->w +2 , ACS_BLOCK, mwin->h);
    mvwhline(stdscr, mwin->h +1, 3, ACS_BLOCK, mwin->w);
    wattroff(stdscr, COLOR_PAIR(3));
    wrefresh(stdscr);
    // Window body
    werase(mwin->win);
    wbkgd(mwin->win, COLOR_PAIR(2));
    box(mwin->win, 0, 0);
    // App name
    int app_name_x = (mwin->w - strlen(app_name))/2;
    mvwprintw(mwin->win, 0, app_name_x-1, "┤");
    wattron(mwin->win, COLOR_PAIR(4));
    mvwprintw(mwin->win, 0, app_name_x, "%s", app_name);
    wattroff(mwin->win, COLOR_PAIR(4));
    mvwprintw(mwin->win, 0, app_name_x+strlen(app_name), "├");
    wrefresh(mwin->win);
}

void ncurses_init(){
    setlocale(LC_ALL, "en_US.UTF-8");
    initscr();
    cbreak(); // Disable buffering // Or raw();
    noecho(); // Disable showing of keys pressed
    keypad(stdscr, TRUE); // Enable keys like F1, F2, arrow keys etc.
    curs_set(FALSE); // Don't display a cursor
    set_escdelay (500); // Faster ESC key. 1000 - default

    // Colors section
    start_color();
    			 // Text      Background
    init_pair(1, COLOR_BLACK, COLOR_BLUE);  // Background
    init_pair(2, COLOR_BLACK, COLOR_WHITE); // Window
    init_pair(3, COLOR_BLACK, COLOR_BLACK); // Shadow
    init_pair(4, COLOR_RED,   COLOR_WHITE); // Window title
    init_pair(5, COLOR_WHITE, COLOR_RED);   // Active selection
    init_pair(6, COLOR_WHITE, COLOR_BLUE);  // Nonactive selection
    init_pair(7, COLOR_BLACK, COLOR_RED);   // Dialog background
}

////////////////////////////////
///   Setting output block   /// 
////////////////////////////////
void mute_other_ports(pa_port **ports_arr, int *ports_ttl, int *lwin_sel){
    int err;
    snd_mixer_t *handle;
    
    int sel_port = (*lwin_sel)-1;
    int cur_card_ind = (*ports_arr)[sel_port].card_index;
    char *card_str;
    sprintf(card_str, "hw:%d", cur_card_ind);

    // Open the mixer
    err = snd_mixer_open(&handle, 0);
    if (err < 0) {
        endwin();
        printf("Error opening mixer: %s\n", snd_strerror(err));
        exit(1);
    }
    // Attach to the specified mixer
    err = snd_mixer_attach(handle, card_str);
    if (err < 0) {
        endwin();
        printf("Error attaching mixer: %s\n", snd_strerror(err));
        exit(1);
    }
    // Register the mixer
    err = snd_mixer_selem_register(handle, NULL, NULL);
    if (err < 0) {
        endwin();
        printf("Error registering mixer: %s\n", snd_strerror(err));
        exit(1);
    }
    // Load the mixer
    err = snd_mixer_load(handle);
    if (err < 0) {
        endwin();
        printf("Error loading mixer: %s\n", snd_strerror(err));
        exit(1);
    }
    
    snd_mixer_elem_t *elem;
    snd_mixer_selem_id_t *sid;

    // !!! Needs rework !!!
    for (int port_ctr = 0; port_ctr < (*ports_ttl); ++port_ctr){
        // ignore ports without name
        if ((*ports_arr)[port_ctr].aport_name == NULL){continue;};
        // ignore port if active
        if ((*ports_arr)[port_ctr].is_active == TRUE){continue;};
        // ignore ports from different sound card
        if ((*ports_arr)[port_ctr].card_index != (*ports_arr)[sel_port].card_index){continue;};
        // ignore other type of ports
        if (strcmp((*ports_arr)[port_ctr].aport_name,(*ports_arr)[sel_port].aport_name) != 0){continue;};
        
        snd_mixer_selem_id_set_index(sid, (*ports_arr)[port_ctr].port_index);
        snd_mixer_selem_id_set_name(sid, (*ports_arr)[port_ctr].aport_name);
        elem = snd_mixer_find_selem(handle, sid);
        long min_vol, max_vol;

        if ((err = snd_mixer_selem_get_playback_volume_range(elem, &min_vol, &max_vol)) < 0) {
            endwin();
            printf("Cannot get playback volume range for mixer element: %s | %d\n", (*ports_arr)[port_ctr].aport_name, (*ports_arr)[port_ctr].port_index);
            snd_mixer_close(handle);
            exit(1);
        }
        
        // Mute left and right channel
        if ((err = snd_mixer_selem_set_playback_volume (elem, SND_MIXER_SCHN_FRONT_LEFT, 0)) < 0) {
            endwin();
            printf("Cannot mute front left for mixer element: %s | %d\n", (*ports_arr)[port_ctr].aport_name, (*ports_arr)[port_ctr].port_index);
            snd_mixer_close(handle);
            exit(1);
        }; 
        if ((err = snd_mixer_selem_set_playback_volume (elem, SND_MIXER_SCHN_FRONT_RIGHT, 0)) < 0) {
            endwin();
            printf("Cannot mute front left for mixer element: %s | %d\n", (*ports_arr)[port_ctr].aport_name, (*ports_arr)[port_ctr].port_index);
            snd_mixer_close(handle);
            exit(1);
        }; 
    }
    
    //mvwprintw(stdscr, 0, 0, "%d",sp_ctr);
    //wrefresh(stdscr);

    // Close the mixer
    snd_mixer_close(handle);
}

void pa_state_cb(pa_context *c, void *userdata);

void set_sink(pa_port *sel_port, int *sel_prof){
    // need sel_prof -1
    // Variables for pulseaudio loop
    pa_mainloop *pa_ml = pa_mainloop_new();
    pa_mainloop_api *pa_mlapi = pa_mainloop_get_api(pa_ml);
    pa_context *pa_ctx = pa_context_new(pa_mlapi, "smtui");   

    // Mainloop sequence
    pa_context_connect(pa_ctx, NULL, 0, NULL);
    // Getting state of pulseaudio
    pa_context_state_t state;
    while ((state = pa_context_get_state(pa_ctx)) != PA_CONTEXT_READY) {
        if (state == PA_CONTEXT_FAILED || state == PA_CONTEXT_TERMINATED) {
            pa_context_disconnect(pa_ctx);
            pa_context_unref(pa_ctx);
            pa_mainloop_free(pa_ml);
            return;
        };
        pa_mainloop_iterate(pa_ml, 1, NULL);
    };
    int pa_ready = 0;
    pa_context_set_state_callback(pa_ctx, pa_state_cb, &pa_ready);
    
    // Setting card
    pa_operation *pa_op = pa_context_set_card_profile_by_index(pa_ctx, sel_port->card_index, sel_port->port_prof[(*sel_prof)-1], NULL, NULL);
    while (pa_operation_get_state(pa_op) == PA_OPERATION_RUNNING){pa_mainloop_iterate(pa_ml, 1, NULL);};
    pa_operation_unref(pa_op);
    // Setting sink port
    pa_op = pa_context_set_sink_port_by_index(pa_ctx, sel_port->sink_index, sel_port->port_name, NULL, NULL);
    while (pa_operation_get_state(pa_op) == PA_OPERATION_RUNNING){pa_mainloop_iterate(pa_ml, 1, NULL);};
    pa_operation_unref(pa_op);
    // Setting default sink
    pa_op = pa_context_set_default_sink(pa_ctx, sel_port->sink_name, NULL, NULL);
    while (pa_operation_get_state(pa_op) == PA_OPERATION_RUNNING){pa_mainloop_iterate(pa_ml, 1, NULL);};
    pa_operation_unref(pa_op);
    
    pa_context_disconnect(pa_ctx);
    pa_context_unref(pa_ctx);
    pa_mainloop_free(pa_ml);
}

//////////////////////
///   Data block   ///
//////////////////////
char *g_alsa_port_names[] = {"Unknown", "Aux", "Speaker", "Headphone", "Line Out", "Mic",
                              NULL, NULL, NULL, "SPDIF", "HDMI", NULL,
                              "FM", "Video", NULL, NULL, NULL, NULL, NULL, NULL,
                              "Phone", NULL, NULL};
//https://www.kernel.org/doc/html/v4.15/sound/designs/control-names.html
//https://www.freedesktop.org/software/pulseaudio/doxygen/def_8h.html#a6b86472b4fa68a20d1412a95a8ba83e6
//PA_DEVICE_PORT_TYPE_HEADSET, PA_DEVICE_PORT_TYPE_HANDSET, PA_DEVICE_PORT_TYPE_EARPIECE;
//PA_DEVICE_PORT_TYPE_TV;
//PA_DEVICE_PORT_TYPE_USB, PA_DEVICE_PORT_TYPE_BLUETOOTH, PA_DEVICE_PORT_TYPE_PORTABLE,
//PA_DEVICE_PORT_TYPE_HANDSFREE, PA_DEVICE_PORT_TYPE_CAR, PA_DEVICE_PORT_TYPE_HIFI;
//PA_DEVICE_PORT_TYPE_NETWORK, PA_DEVICE_PORT_TYPE_ANALOG;
void pa_cards_and_ports_data_cb(pa_context *c, const pa_card_info *card_info, int eol, void *userdata) {
    pa_data *data = (pa_data *)userdata;     
	
    if (eol > 0) {return;}; // Positive eol => end of the list

    // Filling card index and active profile
    data->cards[data->cards_ctr].index = card_info->index;
    int str_len = strlen(card_info->active_profile2->name)+1;
    data->cards[data->cards_ctr].act_prof = calloc(str_len, sizeof(char));
    strcpy(data->cards[data->cards_ctr].act_prof, card_info->active_profile2->name);

    ++data->cards_ctr;

    // TODO: Fix use of global value
	for (int for_ctr = 0; for_ctr < (card_info->n_ports); ++for_ctr){
		if (card_info->ports[for_ctr]->available == PA_PORT_AVAILABLE_NO ||
			card_info->ports[for_ctr]->direction == PA_DIRECTION_INPUT){continue;};
		// Card index
		data->ports[data->ports_ctr].card_index = card_info->index;
        // Sink index and name
        for (int for_ctr = 0; for_ctr < data->sinks_ttl; ++for_ctr){
            if (data->ports[data->ports_ctr].card_index == data->sinks[for_ctr].card_index){
                data->ports[data->ports_ctr].sink_index = data->sinks[for_ctr].sink_index;
                int str_len = strlen(data->sinks[for_ctr].sink_name)+1;
                data->ports[data->ports_ctr].sink_name = calloc(str_len, sizeof(char));
                strcpy(data->ports[data->ports_ctr].sink_name, data->sinks[for_ctr].sink_name);

                break;
            };
        };
    	// alsa.card_name
    	int str_len = strlen(pa_proplist_gets(card_info->proplist, "alsa.card_name"))+1;
    	data->ports[data->ports_ctr].card_name = calloc(str_len, sizeof(char));
    	strcpy(data->ports[data->ports_ctr].card_name, pa_proplist_gets(card_info->proplist, "alsa.card_name"));
		// Port name
		str_len = strlen(card_info->ports[for_ctr]->name)+1;
		data->ports[data->ports_ctr].port_name = calloc(str_len, sizeof(char));
    	strcpy(data->ports[data->ports_ctr].port_name, card_info->ports[for_ctr]->name);
        // Port description
    	str_len = strlen(card_info->ports[for_ctr]->description)+1;
    	data->ports[data->ports_ctr].port_desc = calloc(str_len, sizeof(char));
    	strcpy(data->ports[data->ports_ctr].port_desc, card_info->ports[for_ctr]->description);
        // Port alsa control name
        if(card_info->ports[for_ctr]->type < 23 && g_alsa_port_names[card_info->ports[for_ctr]->type]){
            str_len = strlen(g_alsa_port_names[card_info->ports[for_ctr]->type])+1;
            data->ports[data->ports_ctr].aport_name = calloc(str_len, sizeof(char));
            strcpy(data->ports[data->ports_ctr].aport_name, g_alsa_port_names[card_info->ports[for_ctr]->type]);
        };
        // Port index
        str_len = strlen(data->ports[data->ports_ctr].port_name)-1;
        if (isdigit(data->ports[data->ports_ctr].port_name[str_len])){
            data->ports[data->ports_ctr].port_index = atoi(&data->ports[data->ports_ctr].port_name[str_len])-1;
            if (data->ports[data->ports_ctr].port_index < 0) {
                data->ports[data->ports_ctr].port_index = 0;
            };
        } else {
            data->ports[data->ports_ctr].port_index = 0;
        };
		// Number of port profiles
    	data->ports[data->ports_ctr].pprof_ttl = card_info->ports[for_ctr]->n_profiles;
		// Port profiles array
		data->ports[data->ports_ctr].port_prof = calloc(data->ports[data->ports_ctr].pprof_ttl, sizeof(char *));
		for (int pr_ctr = 0; pr_ctr < data->ports[data->ports_ctr].pprof_ttl; ++pr_ctr){
			str_len = strlen(card_info->ports[for_ctr]->profiles2[pr_ctr]->name)+1;
			data->ports[data->ports_ctr].port_prof[pr_ctr] = calloc(str_len, sizeof(char));
			strcpy(data->ports[data->ports_ctr].port_prof[pr_ctr], card_info->ports[for_ctr]->profiles2[pr_ctr]->name);
		};
		// Port activeness
		data->ports[data->ports_ctr].is_active = 0;
    	if (!strcmp(data->ports[data->ports_ctr].port_name, data->def_sink_act_port)){
    		data->ports[data->ports_ctr].is_active = 1;
    	};
    	
    	++data->ports_ctr;
    };
}

void pa_sinks_data_cb(pa_context *c, const pa_sink_info *sink_info, int eol, void *userdata) {
    pa_data *data = (pa_data *)userdata;  
    
    if (eol > 0) {return;}; // Positive eol => end of the list
    data->sinks[data->sinks_ctr].sink_index = sink_info->index;
    data->sinks[data->sinks_ctr].card_index = sink_info->card;

    int str_len = strlen(sink_info->name)+1;
    data->sinks[data->sinks_ctr].sink_name = calloc(str_len, sizeof(char));
    strcpy(data->sinks[data->sinks_ctr].sink_name, sink_info->name);

    ++data->sinks_ctr;
}

void pa_def_sink_data_cb(pa_context *c, const pa_sink_info *sink_info, int eol, void *userdata) {
	pa_data *data = (pa_data *)userdata;  
    
    if (eol > 0) {return;}; // Positive eol => end of the list
    data->def_sink_ind = sink_info->index;
    int str_len = strlen(sink_info->active_port->name)+1;
    data->def_sink_act_port = calloc(str_len, sizeof(char));
    strcpy(data->def_sink_act_port, sink_info->active_port->name);
}

void pa_count_sinks_cb(pa_context *c, const pa_sink_info *sink_info, int eol, void *userdata) {
    pa_data *data = (pa_data *)userdata;  
    
    if (eol > 0) {return;}; // Positive eol => end of the list
    if (sink_info){
        ++(data->sinks_ttl);
    };
}

void pa_count_cards_and_ports(pa_context *c, const pa_card_info *card_info, int eol, void *userdata) {
	pa_data *data = (pa_data *)userdata; 
    
    if (eol > 0) {return;}; // Positive eol => end of the list
    if (card_info) {
    	++(data->cards_ttl);
	   
    	for (int i = 0; i < card_info->n_ports; ++i){
    		if (card_info->ports[i]->available == PA_PORT_AVAILABLE_NO ||
    			card_info->ports[i]->direction == PA_DIRECTION_INPUT){continue;};
    		++(data->ports_ttl);
    	};
    };
}

void pa_state_cb(pa_context *c, void *userdata) {
	pa_context_state_t state;
	int *pa_ready = userdata;

	state = pa_context_get_state(c);
	switch (state) {
		case PA_CONTEXT_FAILED:
		case PA_CONTEXT_TERMINATED:
			*pa_ready = 2;
			break;
		case PA_CONTEXT_READY:
			*pa_ready = 1;
			break;
		default:
			break;
	};
}

int fill_data(pa_data *data) {
    // Variables for pulseaudio loop
    pa_mainloop *pa_ml = pa_mainloop_new();
    pa_mainloop_api *pa_mlapi = pa_mainloop_get_api(pa_ml);
    pa_context *pa_ctx = pa_context_new(pa_mlapi, "smtui");   

    // Mainloop sequence
    pa_context_connect(pa_ctx, NULL, 0, NULL);
    // Getting state of pulseaudio
    pa_context_state_t state;
    while ((state = pa_context_get_state(pa_ctx)) != PA_CONTEXT_READY) {
        if (state == PA_CONTEXT_FAILED || state == PA_CONTEXT_TERMINATED) {
        	pa_context_disconnect(pa_ctx);
            pa_context_unref(pa_ctx);
            pa_mainloop_free(pa_ml);
            return 1;
        };
        pa_mainloop_iterate(pa_ml, 1, NULL);
    };
    int pa_ready = 0;
    pa_context_set_state_callback(pa_ctx, pa_state_cb, &pa_ready);

    // Reseting counters
    data->cards_ctr = 0;
    data->ports_ctr = 0;
    data->sinks_ctr = 0;

    // Getting number of sound cards
    data->cards_ttl = 0;
    data->ports_ttl = 0;
    
    pa_operation *pa_op = pa_context_get_card_info_list(pa_ctx, pa_count_cards_and_ports, data);
    while (pa_operation_get_state(pa_op) == PA_OPERATION_RUNNING){pa_mainloop_iterate(pa_ml, 1, NULL);};
	pa_operation_unref(pa_op);
    
    // Getting number of sinks
    data->sinks_ttl = 0;
    pa_op = pa_context_get_sink_info_list(pa_ctx, pa_count_sinks_cb, data);
    while (pa_operation_get_state(pa_op) == PA_OPERATION_RUNNING){pa_mainloop_iterate(pa_ml, 1, NULL);};
    pa_operation_unref(pa_op);

	// Getting default sink data
	pa_op = pa_context_get_sink_info_by_name(pa_ctx, NULL, pa_def_sink_data_cb, data);
	while (pa_operation_get_state(pa_op) == PA_OPERATION_RUNNING){pa_mainloop_iterate(pa_ml, 1, NULL);};
	pa_operation_unref(pa_op);

    // Getting sinks data
    data->sinks = calloc(data->sinks_ttl, sizeof(pa_sink));
    pa_op = pa_context_get_sink_info_list(pa_ctx, pa_sinks_data_cb, data);
    while (pa_operation_get_state(pa_op) == PA_OPERATION_RUNNING){pa_mainloop_iterate(pa_ml, 1, NULL);};
    pa_operation_unref(pa_op);

	// Getting cards and ports data
    data->cards = calloc(data->cards_ttl, sizeof(pa_card));
	data->ports = calloc(data->ports_ttl, sizeof(pa_port));
	pa_op = pa_context_get_card_info_list(pa_ctx, pa_cards_and_ports_data_cb, data);
    while (pa_operation_get_state(pa_op) == PA_OPERATION_RUNNING){pa_mainloop_iterate(pa_ml, 1, NULL);};
	pa_operation_unref(pa_op);
	
    pa_context_disconnect(pa_ctx);
    pa_context_unref(pa_ctx);
    pa_mainloop_free(pa_ml);
    return 0;
}

int act_prof_index (pa_data *data, ncwin_par *lwin){
    // Getting card index in cards array
    int card_arr_ind = 0;
    for (int ctr = 0; ctr < data->cards_ttl; ++ctr){
        if (data->cards[ctr].index == data->ports[lwin->sel-1].card_index){
            card_arr_ind = ctr;
        };
    }
    // Looping through port profiles
    for (int i = 0; i < data->ports[lwin->sel-1].pprof_ttl; ++i){
        if (!strcmp(data->cards[card_arr_ind].act_prof,
            data->ports[lwin->sel-1].port_prof[i])){
            return i+1;
        };
    };

    return 1;
}

//////////////////////
///   Misc block   ///
//////////////////////
void print_data(pa_data *data) {
    printf("\033[1mNumber of cards: %d\033[0m\n", data->cards_ttl);
    for (int i = 0; i < data->cards_ttl; ++i) {
        printf("Card index: %u\n", data->cards[i].index);
        printf("Card active profile: %s\n", data->cards[i].act_prof);
        printf("\n");
    };

    printf("\033[1mNumber of sinks: %d\033[0m\n", data->sinks_ttl);
    printf("Default sink index: %d\n", data->def_sink_ind);
    printf("Default sink port: %s\n", data->def_sink_act_port);
    puts("Sinks list:");
    for (int i = 0; i < data->sinks_ttl; ++i) {
        printf("Sink index: %d\n", data->sinks[i].sink_index);
        printf("Sink name: %s\n", data->sinks[i].sink_name);
        printf("Sink card index: %d\n", data->sinks[i].card_index);
        printf("\n");
    };

    printf("\033[1mNumber of ports: %d\033[0m\n", data->ports_ttl);
	for (int i = 0; i < data->ports_ttl; ++i) {
		printf("Card index: %u\n", data->ports[i].card_index);
		printf("Card name: %s\n", data->ports[i].card_name);
        printf("Sink index: %u\n", data->ports[i].sink_index);
        printf("Sink name: %s\n", data->ports[i].sink_name);
		printf("Port name: %s\n", data->ports[i].port_name);
        printf("Port alsa name: %s\n", data->ports[i].aport_name);
        printf("Port index: %d\n", data->ports[i].port_index);
		printf("Port desc: %s\n", data->ports[i].port_desc);
		printf("Profiles number: %d\n", data->ports[i].pprof_ttl);
		for (int j = 0; j < data->ports[i].pprof_ttl; ++j) {
			printf("Profile #%d: %s\n", j+1, data->ports[i].port_prof[j]);
		};
		printf("Is active: %s\n", (data->ports[i].is_active) ? "true" : "false");
		printf("\n");
	};
}

void close_app(pa_data *data){
    endwin();

    // Memory cleanup
    /*
    free(data->def_sink_act_port);
    for (int i = 0; i < data->ports_ttl; ++i){
    	free(data->ports[i].card_name);
    	free(data->ports[i].port_name);
    	free(data->ports[i].port_desc);
    	for (int j = 0; j < data->ports[i].pprof_ttl; ++j){
    		free(data->ports[i].port_prof[j]);
    	};
    	free(data->ports[i].port_prof);
    };
    free(data->ports);
    */
    exit(0);
}



int main(int argc, char *argv[]) {
    // Filling sound card data
    pa_data data;

    if (fill_data(&data) < 0) {
		fprintf(stderr, "failed to get cards list\n");
		return 1;
    };
    //print_data(&data);
    //return 0;

    // ncurses
    ncurses_init();
    // Exit if terminal window is too small
    if (COLS < 67 || LINES < 18){endwin();
                                printf("Terminal window should have at least 18 lines and 67 columns.\n");
                                return 0;};
    // Main window
    char *app_name = "SinkManager TUI";
    ncwin_par mwin = {.win = newwin(LINES-3, COLS-4, 1, 2), 
					  .l = 1, .c = 2, .w = COLS-4, .h = LINES-3};
    draw_mwin(&mwin, app_name);
    // Buttons window
    char *btns_arr[4] = {"[Set]", "[Refresh]", "[Exit]"};
	ncwin_par bwin = {.win = newwin(mwin.h-5, 11, mwin.l+3, mwin.w-10), 
					  .l = mwin.l+3, .c = mwin.w-10, .w = 11, .h = mwin.h-4, 
					  .foc = 0, .sel = 1, .max_sel = 1};
	draw_bwin(&bwin, btns_arr);
	// List window
	int lwin_h = (mwin.h/4)*3;
    if (mwin.h-lwin_h-3 <= 3){lwin_h -= 2;};
	if (lwin_h%2 != 0){--lwin_h;};
    ncwin_par lwin = {.win = newwin(lwin_h, mwin.w-bwin.w-3,
    								mwin.l+2, mwin.c+2), 
					  .l = mwin.l+2, .c = mwin.c+2,
					  .w = mwin.w-bwin.w-3, .h = lwin_h, 
					  .foc = 1, .sel = 1, .max_sel = 1};
	draw_lwin(&lwin, &data);
	// Profiles window
    ncwin_par pwin = {.win = newwin(mwin.h-lwin.h-3, mwin.w-bwin.w-3, 
    								lwin.h+3, mwin.c+2), 
					  .l = lwin.h+3, .c = mwin.c+2,
					  .w = mwin.w-bwin.w-3, .h = mwin.h-lwin.h-3, 
					  .foc = 0, .sel = 1, .max_sel = 1};
	// Looking for current active profile
    for (int i = 0; i < data.ports[lwin.sel-1].pprof_ttl; ++i){
        // TODO: Fix to proper card index
    	if (!strcmp(data.cards[0].act_prof,
    		data.ports[lwin.sel-1].port_prof[i])){
    		pwin.sel = i+1;
    		break;
    	} else {pwin.sel = 1;};
    };
    draw_pwin(&pwin, &lwin, &data);

    // Parsing keys input
    int ch = 0;
    //while ((ch = getch()) != 'q') { // 'q' key to quit
    while ((ch = getch())) {
        switch (ch) {
            case KEY_UP:
                if (lwin.foc) {
                    --lwin.sel;
                    if (lwin.sel < 1) {
                        lwin.sel = 1;
                    };
                    draw_lwin(&lwin, &data);
                    pwin.sel = act_prof_index(&data, &lwin);
                    draw_pwin(&pwin, &lwin, &data);
                };
                if (pwin.foc) {
                    --pwin.sel;
                    if (pwin.sel < 1) {
                        pwin.sel = 1;
                    };
                    draw_lwin(&lwin, &data);
                    draw_pwin(&pwin, &lwin, &data);
                };
                if (bwin.foc){
                	--bwin.sel;
                	if (bwin.sel < 1){ bwin.sel = 1;};
                	draw_bwin(&bwin, btns_arr);
                };
                break;
            case KEY_DOWN:
                if (lwin.foc) {
                    ++lwin.sel;
                    if (lwin.sel > data.ports_ttl) {
                        lwin.sel = data.ports_ttl;
                    };
                    if (lwin.max_sel < lwin.sel){
                        lwin.max_sel = lwin.sel;
                    };
                    draw_lwin(&lwin, &data);
                    pwin.sel = act_prof_index(&data, &lwin);
                    draw_pwin(&pwin, &lwin, &data);
                };
                if (pwin.foc) {
                    ++pwin.sel;
                    if (pwin.sel > data.ports[lwin.sel-1].pprof_ttl) {
                        pwin.sel = data.ports[lwin.sel-1].pprof_ttl;
                    };
                    if (pwin.max_sel < pwin.sel){
                        pwin.max_sel = pwin.sel;
                    };
                    draw_lwin(&lwin, &data);
                    draw_pwin(&pwin, &lwin, &data);
                };
                if (bwin.foc){
                	++bwin.sel;
                	if (bwin.sel > 4){ bwin.sel = 4;};
                	draw_bwin(&bwin, btns_arr);
                };
                break;
            case KEY_LEFT:
            	if (bwin.foc) {
            		if (bwin.sel < 3){
	            		lwin.foc = TRUE;
	            		bwin.foc = FALSE;

	            		draw_lwin(&lwin, &data);
	            	} else {
	            		pwin.foc = TRUE;
	            		bwin.foc = FALSE;

	            		draw_pwin(&pwin, &lwin, &data);
	            	};
                    draw_bwin(&bwin, btns_arr);
            	};
                break;
            case KEY_RIGHT:
            	if (lwin.foc) {
            		lwin.foc = FALSE;
            		bwin.foc = TRUE;
            		bwin.sel = 1;

            		draw_lwin(&lwin, &data);
                    draw_bwin(&bwin, btns_arr);
            	};
            	if (pwin.foc){
            		pwin.foc = FALSE;
            		bwin.foc = TRUE;
            		bwin.sel = 1;

            		draw_pwin(&pwin, &lwin, &data);
            		draw_bwin(&bwin, btns_arr);
            	};
                break;
            case KEY_ESC: // Escape button
            	if (lwin.foc) {
            		close_app(&data);
            	};
            	if (pwin.foc) {
            		lwin.foc = TRUE;
            		pwin.foc = FALSE;

            		draw_lwin(&lwin, &data);
                    draw_pwin(&pwin, &lwin, &data);
            	};
            	if (bwin.foc) {
            		if (bwin.sel < 3){
	            		lwin.foc = TRUE;
	            		bwin.foc = FALSE;

	            		draw_lwin(&lwin, &data);
	            	} else {
	            		pwin.foc = TRUE;
	            		bwin.foc = FALSE;

	            		draw_pwin(&pwin, &lwin, &data);
	            	};
                    draw_bwin(&bwin, btns_arr);
            	};
                break;
            case 'q':
                close_app(&data);
                break;
            case '\t':
            	if (lwin.foc) {
            		lwin.foc = FALSE;
            		pwin.foc = TRUE;

            		draw_lwin(&lwin, &data);
                    draw_pwin(&pwin, &lwin, &data);
            	} else if (pwin.foc) {
            		lwin.foc = TRUE;
            		pwin.foc = FALSE;

                    draw_lwin(&lwin, &data);
                    draw_pwin(&pwin, &lwin, &data);
            	};
            	break;
            case KEY_ENTER:
            case '\n':
            	if (lwin.foc) {
            		lwin.foc = FALSE;
            		pwin.foc = TRUE;

            		draw_lwin(&lwin, &data);
                    draw_pwin(&pwin, &lwin, &data);
            	} else if (pwin.foc) {
            		pwin.foc = FALSE;
            		bwin.foc = TRUE;
            		bwin.sel = 1;

                    draw_pwin(&pwin, &lwin, &data);
                    draw_bwin(&bwin, btns_arr);
            	} else if (bwin.foc){
            		//Set button
            		if (bwin.sel == 1){
            			set_sink(&data.ports[lwin.sel-1], &pwin.sel);
                        //mute_other_ports(&data.ports, &data.ports_ttl, &lwin.sel);
            			bwin.sel = 2;
            		};
            		// Refresh button
            		if (bwin.sel == 2){
            			lwin.sel = 1;
            			pwin.sel = act_prof_index(&data, &lwin);

	            		lwin.foc = TRUE;
            			pwin.foc = FALSE;
	            		bwin.foc = FALSE;

            			fill_data(&data);
            			draw_lwin(&lwin, &data);
            			draw_pwin(&pwin, &lwin, &data);
                    	draw_bwin(&bwin, btns_arr);
            		};
            		// Exit button
            		if (bwin.sel == 3){
            			close_app(&data);
            		};
            	};
                break;
            case KEY_RESIZE:
                // Main window
                mwin.h = LINES-3;
                mwin.w = COLS-4;
                wresize(mwin.win, mwin.h, mwin.w);
                // Buttons window
                bwin.h = mwin.h-4;
                wresize(bwin.win, bwin.h, bwin.w);
                bwin.l = mwin.l+3;
                bwin.c = mwin.w-10;
                mvwin(bwin.win, bwin.l, bwin.c);
                // List window
                lwin.h = (mwin.h/4)*3;
                if (lwin.h%2 != 0){--lwin.h;};
                lwin.w = mwin.w-bwin.w-3;
                wresize(lwin.win, lwin.h, lwin.w);
                // Profiles window
                pwin.h = mwin.h-lwin.h-3;
                pwin.w = mwin.w-bwin.w-3;
                wresize(pwin.win, pwin.h, pwin.w);
                pwin.l = lwin.h+3;
                pwin.c = mwin.c+2;
                mvwin(pwin.win, pwin.l, pwin.c);

                draw_mwin(&mwin, app_name);
                draw_lwin(&lwin, &data);
                draw_pwin(&pwin, &lwin, &data);
                draw_bwin(&bwin, btns_arr);
                break;
        };
    };

    close_app(&data);

    return 0;
}
