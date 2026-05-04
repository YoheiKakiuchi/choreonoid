#!/bin/bash

docker run -u luser -v $(pwd)/build00:/userdir -it buildqt:24.04 bash
