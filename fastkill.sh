#!/bin/bash

#
# Author: Joshua Inscoe
# Last modified: November 17, 2016
#
# File: fastkill.sh
#



script="$0"
default_pname='annoy'


get_program_info()
{
    pname=$1
    pinfo=$(ps -ef | grep "$pname" | grep -v "$script" | grep -v 'grep')
    echo "$pinfo"
}

get_program_pids()
{
    pname=$1
    lines=$(get_program_info "$pname")
    pidsf=$(echo "$lines" | sed 's/^ *//g' | cut -d' ' -f 2 | tr '\n' ' ')
    echo "$pidsf"
}


case $# in
    0)
        pname="$default_pname"
        ;;

    1)
        pname="$1"
        ;;

    *)
        printf "usage: %s [program-name]\n" "$default_pname"
        exit 1
        ;;
esac

proginfo="$(get_program_info "$pname")"
progpids="$(get_program_pids "$pname")"

if [ -z "$proginfo" ]
then
    printf "No processes matching the program name, '$pname', were found.\n"
    exit 0
fi

clear

printf "The following processes were found.\n\n"
printf "$proginfo\n\n"
printf "This command can be used to terminate the above processes:\n"
printf "kill -KILL $progpids\n\n"

loop=1

while [ $loop -eq 1 ]
do
    read -r -p 'Execute command? [Y/n]: ' answer
    answer="$(echo "$answer" | sed 's/^ *//g' | sed 's/ *$//g')"

    case "$answer" in
        'Y'|'y')
            loop=0
            termproc=1
            ;;

        'N'|'n')
            loop=0
            termproc=0
            ;;
    esac
done

printf "\n"

if [ $termproc -eq 1 ]
then
    printf "Terminating processes...\n"
    kill -KILL $progpids
fi
