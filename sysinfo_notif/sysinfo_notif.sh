# !/bin/bash
NOTIF_ID=2346
TITLE=" "

# Time and date
TIME=$(date '+%H:%M')
CONTENT="                     <span size='26pt'><b>${TIME}</b></span>\n"
DATE=$(date '+%a, %d/%m/%Y')
CONTENT="${CONTENT}                    <span size='11pt'>${DATE}</span>\n"
# Separator
CONTENT="${CONTENT} -------------------------------------------------------------------- \n"
CONTENT="${CONTENT}<span font='DejaVuSansM Nerd Font' size='10pt'><b>  CPU   TEMP   RAM     VOL   KBD</b>\n"

# CPU usage percent
read IDLE_I TOTAL_I <<< $(head -n 1 /proc/stat | awk '{idle = $5 + $6; total = $2 + $3 + $4 + idle + $7 + $8 + $9;} END {print idle, total;}')
sleep 0.3
read IDLE_II TOTAL_II <<< $(head -n 1 /proc/stat | awk '{idle = $5 + $6; total = $2 + $3 + $4 + idle + $7 + $8 + $9;} END {print idle, total;}')
CPU_USAGE=$(echo "scale=0; (100 * (($TOTAL_II - $TOTAL_I) - ($IDLE_II - $IDLE_I)) / ($TOTAL_II - $TOTAL_I))" | bc)
CONTENT="${CONTENT}  $(printf "%-6s" "$CPU_USAGE%")"

# CPU temperature
CPU_TEMP=$(( $(cat /sys/class/thermal/thermal_zone0/temp) / 1000 ))
THRESHOLD=80
if [ "$CPU_TEMP" -gt "$THRESHOLD" ]; then
    CONTENT="${CONTENT}$(printf "<span foreground='#e7666a'>%-8s</span>" "$CPU_TEMP°C")"
else 
    CONTENT="${CONTENT}$(printf "%-8s" "$CPU_TEMP°C")"
fi

#RAM
RAM_USE=$(free -h -L | awk '{print $6}')
CONTENT="${CONTENT}$(printf "%-8s" "${RAM_USE}B")"

# Volume variables
VOLUME=$(amixer -D default get Master | awk -F"[][]" '/Left:/ { print $2 }')
MUTE=$(amixer -D default get Master | awk -F"[][]" '/Left:/ { print $4 }')
if [[ "$MUTE" != "on" ]]; then VOLUME="${VOLUME}(X)"; fi
CONTENT="${CONTENT}$(printf "%-6s" "${VOLUME}")"

# Keyboard layout
# Input identifiers: swaymsg -t get_inputs
KBD_LAYOUT=$(swaymsg --raw -t get_inputs | jq -r '.[] | select(.identifier == "***************") | .xkb_active_layout_name' | head -n 1)
CONTENT="${CONTENT}${KBD_LAYOUT:0:2}</span>"

# Currently playing
if playerctl status &>/dev/null; then
    CONTENT="${CONTENT}\n -------------------------------------------------------------------- \n"
    CONTENT="${CONTENT}  <b>$(playerctl metadata -f "{{trunc(title,34)}}")</b>\n"
    CONTENT="${CONTENT}  $(playerctl metadata -f "{{artist}} - {{album}}" | sed 's/\(.\{40\}\).*/\1.../')"
    # removing separator if album field is empty
    if [[ "${CONTENT:${#CONTENT} - 1}" == " " ]]; then
        CONTENT="${CONTENT:: -3}"
    fi
fi

# Additional "invisible" line
CONTENT="${CONTENT}\n<span foreground='#5b5950'>-------</span>"
#echo "$CONTENT"
notify-send -t 3000 -a "cur_vol_notif" -r "$NOTIF_ID" -i noicon "$TITLE" "$CONTENT"
exit 0
