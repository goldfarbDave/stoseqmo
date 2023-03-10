#!/bin/bash
fail_code=0
for file in $(find . -name "*.hpp"); do
    fl=$(head -n1 $file)
    if [ "${fl}" != "#pragma once" ]; then
        echo "check_headers.sh failed: ${file} does not start with '#pragma once'"
        fail_code=1
    fi
done;
exit $fail_code
