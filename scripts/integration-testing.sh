#!/bin/bash

brew install nvm > /dev/null 2>&1
source $(brew --prefix nvm)/nvm.sh
nvm install 8

npm i

npm run lint

npm run test
