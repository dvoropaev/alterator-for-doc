#!/bin/fish

function _completion_actl
    set app_name alteratorctl
    set options ""
    set tokens (commandline -opc)

    if [ $tokens[-1] = $app_name ] || [ $tokens[-1] = actl ]
        set options "$($app_name --help |
            grep -o -- "--[a-zA-Z0-9\-]*" |
            sort)"
        string join '' -- $options "$($app_name --modules)"
        echo "$options"
    else
        set options "$($tokens --help |
                sed '/alteratorctl/d' |
                awk '/^ /' |
                sed '/^  -/d' |
                grep -Eo "^.{30}" |
                awk '{ print $1 }' |
                sort)"
        string join '' -- $options "$($tokens --help |
            grep -o -- "--[a-zA-Z0-9\-]*" |
            sort)"
        echo "$options"
    end
end
complete -f -a "(_completion_actl)" -c alteratorctl
complete -f -a "(_completion_actl)" -c actl
