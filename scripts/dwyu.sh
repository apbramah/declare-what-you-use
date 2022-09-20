#!/bin/bash

ignore_file=.dwyuignore

function help {
  echo "Usage: $(basename $0) [OPTIONS] <compile_commands_json>"
  echo
  echo OPTIONS:
  grep -P -- "-[^*]\|--.+(?=\))" $0 | sed "s/) #/\n\t/g"

  exit 0
}

POSITIONAL_ARGS=()

while [[ $# -gt 0 ]]; do
  case $1 in
    -k|--keep) # Keep rows matching this regex expression
      keep=$2
      shift # past argument
      shift # past argument
      ;;
    -i|--ignore_file) # Location of .dwyuignore file
      ignore_file=$2
      shift # past argument
      shift # past argument
      ;;
    -h|--help) # Help
      help
      exit 0
      ;;
    -*|--*)
      echo "Unknown option $1"
      help
      exit 1
      ;;
    *)
      POSITIONAL_ARGS+=("$1") # save positional arg
      shift # past argument
      ;;
  esac
done

set -- "${POSITIONAL_ARGS[@]}" # restore positional parameters

compile_commands_file=$1

dwyu ${compile_commands_file} 2>/dev/null | grep -P "${keep}" > actual_unreffed_decls_with_location.txt && \
awk -F$'\t' '{print $1}' actual_unreffed_decls_with_location.txt > actual_unreffed_decls.txt && \
diff actual_unreffed_decls.txt ${ignore_file} > dwyu_diff.txt

if [ $? != 0 ]; then
    echo Remove these from ${ignore_file}:
    cat dwyu_diff.txt | grep -oP "(?<=^> ).*"

    echo Remove these unreferenced declarations from your code:
    cat dwyu_diff.txt | grep -oP "(?<=^< ).*" | grep -f - actual_unreffed_decls_with_location.txt

    exit 1
else
    exit 0
fi
