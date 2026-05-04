#!/bin/bash

abs_script=$(readlink -f "$0")
abs_dir=$(dirname ${abs_script})

docker run -u luser -v ${abs_dir}/../:/userdir -it buildqt:24.04 bash
