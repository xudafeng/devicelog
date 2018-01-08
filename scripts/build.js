'use strict';

const _ = require('macaca-utils');
const child_process = require('child_process');

if (_.platform.isOSX) {
  child_process.spawnSync('xcodebuild', {
    cwd: process.cwd(),
    stdio: [
      'inherit',
      'inherit',
      'inherit'
    ]
  });
}
