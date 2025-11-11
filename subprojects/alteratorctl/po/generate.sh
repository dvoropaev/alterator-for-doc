#!/bin/bash

print_help()
{
cat<<EOF
Usage:
    ./generate.sh [OPTION]

Options:
    -h, --help:     Usage help
    -u, --update:   Generate updated .po file by merging with updated .pot file
EOF
}

generate_pot_file()
{
    find . -path ./build -prune -o -regex ".*\.\(c\|h\)$" \
        -exec xgettext --keyword=_ --keyword=N_ -o po/alteratorctl.pot {} +
}

# Parse args
if [[ -n $1 ]]; then
   if [ $1 = "-u" ] || [ $1 = "--update" ]; then
       # Regenerate .pot file
       generate_pot_file
       # Update .po file
       msgmerge --previous --update po/alteratorctl.po po/alteratorctl.pot
   elif [ $1 = "-h" ] || [ $1 = "--help" ]; then
       print_help
   else
       echo "Wrong arguments. Run ./generate.sh -h | --help"
   fi
   exit
fi

#Generate .po file
msginit -i po/alteratorctl -l ru_RU.UTF-8 -o po/alteratorctl.po

# Delete .pot file
rm -f po/alteratorctl.pot
