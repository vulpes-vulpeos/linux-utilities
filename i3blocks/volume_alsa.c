#include <stdio.h>
#include <alsa/asoundlib.h> // volume
#include <math.h>

#define ALSA_CARD "default"
#define ALSA_SELEMNAME "Master"

// TODO add speaker toggle (or do it in separate function? )
int is_speaker(snd_mixer_t *handle, snd_mixer_selem_id_t *sid, snd_mixer_elem_t* elem){
    // TODO add sound card name check?

    int mute_0 = 0, mute_1 = 0;
    // Check Headphone 0 port (Headphones)
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, "Headphone");
    if ((elem = snd_mixer_find_selem(handle, sid)) == NULL) {return 1; };
    if (snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, &mute_0) < 0) { return 1; };
    // Check Headphone 1 port (Speakers)
    snd_mixer_selem_id_set_index(sid, 1);
    snd_mixer_selem_id_set_name(sid, "Headphone");
    if ((elem = snd_mixer_find_selem(handle, sid)) == NULL) {return 1; };
    if (snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, &mute_1) < 0) { return 1; };

    if (mute_0 == 1 && mute_1 == 0) { return 0; };

    return 1;
}

// Returns a volume percentage (0–100) based on exponential perception
// Base = 1.035 worksout of the box for -100dB - 0dB range
int get_vol_perc_exp(long min_db, long max_db, long cur_db) {
    if (cur_db <= min_db) return 0;
    if (cur_db >= max_db) return 100;

    // Tunable base: higher = steeper growth, lower = flatter growth (1.035 default)
    double base = 1.0 + (1.035 - 1.0) * (100.0 / (max_db - min_db));
    double percentage = (pow(base, (double)(cur_db-min_db)) / pow(base, (double)(max_db-min_db))) * 100.0;

    return (int)(percentage + 0.5);  // round to nearest integer
}

// gcc volume_alsa.c -o volume_alsa -lasound -lm
int main() {
    char* button = getenv("BLOCK_BUTTON");
    if (button != NULL) {
        switch (atoi(button)) {
            case 1: // Left click - toggle mute
                system("amixer -q set Master toggle");
                break;
            case 4: // Scroll up - increase volume
                system("amixer -q set Master 2dB+");
                break;
            case 5: // Scroll down - decrease volume
                system("amixer -q set Master 2dB-");
                break;
        };
    };

    long min_db = 0, max_db = 0, cur_db = 0;
    int mute = 0, speaker = 1;

    snd_mixer_t *handle = NULL;
    snd_mixer_selem_id_t *sid = NULL;
    snd_mixer_elem_t* elem;
    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, ALSA_SELEMNAME);

    if (snd_mixer_open(&handle, 0) < 0) { return 0; };
    if (snd_mixer_attach(handle, ALSA_CARD) < 0) {snd_mixer_close(handle); return 0; };
    if (snd_mixer_selem_register(handle, NULL, NULL) < 0) {snd_mixer_close(handle); return 0; };
    if (snd_mixer_load(handle) < 0) {snd_mixer_close(handle); return 0; };
    if ((elem = snd_mixer_find_selem(handle, sid)) == NULL) {snd_mixer_close(handle); return 0; };
    if (snd_mixer_selem_get_playback_dB_range(elem, &min_db, &max_db) < 0) {snd_mixer_close(handle); return 0; };
    if (snd_mixer_selem_get_playback_dB(elem, SND_MIXER_SCHN_FRONT_LEFT, &cur_db) < 0) {snd_mixer_close(handle); return 0; };
    if (snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, &mute) < 0) {snd_mixer_close(handle); return 0; };
    speaker = is_speaker(handle, sid, elem);
    snd_mixer_close(handle);

    printf("%s %lddB%s\n", (speaker) ? "󰓃" : " ", cur_db/100, (mute) ? "" : "[X]");

    // send notifcation
    char cmd_notif[128];
    snprintf(cmd_notif, sizeof(cmd_notif), "notify-send -t 500 -a 'cur_vol_notif' '%s %lddB%s' -h 'int:value:%d'", (speaker) ? "󰓃" : "", cur_db/100, (mute) ? "" : "(muted)", get_vol_perc_exp(min_db, max_db, cur_db));
    system(cmd_notif);
    //system("aplay /home/vulpeos/.sounds/volume.wav");

    return 0;
}
