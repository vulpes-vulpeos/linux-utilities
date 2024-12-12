#!/bin/bash

DESKTOP_DIRS=("$HOME/.local/share/applications" "/usr/share/applications" "/usr/local/share/applications")
TERMINAL="kitty"

# Get .desktop files
desktop_files=($(for dir in "${DESKTOP_DIRS[@]}"; do [ -d "$dir" ] && echo "$dir"/*.desktop; done))

# Parse application names and execution commands
apps=()
declare -A app_cmds
declare -A requires_terminal

# Creating array of apps, their exec commands and if they require terminal. Ignore hidden
for file in "${desktop_files[@]}"; do
    [ -f "$file" ] || continue
    grep -qEi "^(Hidden|NoDisplay)=true" "$file" && continue
    name=$(grep -m1 "^Name=" "$file" | cut -d= -f2-)
    exec_cmd=$(grep -m1 "^Exec=" "$file" | cut -d= -f2- | sed -e 's/ *%[fFuUdDnNickvm]//g')
    terminal_needed=$(grep -qi "^Terminal=true" "$file" && echo "true" || echo "false")
    [ -n "$name" ] && [ -n "$exec_cmd" ] && {
        apps+=("$name")
        app_cmds["$name"]="$exec_cmd"
        requires_terminal["$name"]="$terminal_needed"
    }
done

# Sort apps alphabeticaly case insensetive
mapfile -t apps < <(printf '%s\n' "${apps[@]}" | sort -fu)

# Send the list of apps to dmenu
selected_app=$(printf '%s\n' "${apps[@]}" | wmenu -bi -N "#403f3a" -n "#dbdbdb" -S "#9c4f30" -s "#dbdbdb" -p "Search:" -M "#9c4f30" -m "#dbdbdb" )

# Run the selected application if valid
if [ -n "$selected_app" ] && [ -n "${app_cmds["$selected_app"]}" ]; then
    [ "${requires_terminal["$selected_app"]}" = "true" ] && { $TERMINAL -e sh -c "${app_cmds["$selected_app"]}" & } || { eval "${app_cmds["$selected_app"]}" & }
else
    echo "No valid app selected or command missing."
    exit 1
fi
